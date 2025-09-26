/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions andˆ
 * limitations under the License.
 */

#include "CarrierConfig.h"
#include "IJniMtcServiceThread.h"
#include "IMessage.h"
#include "IMtcCallController.h"
#include "IMtcContext.h"
#include "INetworkWatcher.h"
#include "ISession.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "IuMtcService.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/MtcCallStringUtils.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "emergency/EmergencyServiceController.h"
#include "helper/ICallStateProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include "utility/IMessageUtils.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EmergencyServiceController::EmergencyServiceController(
        IN IMtcEmergencyServiceManager& objServiceManager, IN IMtcContext& objContext) :
        m_objServiceManager(objServiceManager),
        m_objContext(objContext),
        m_eState(IEmergencyServiceController::State::IDLE),
        m_eEmergencyCallState(IMtcCall::State::IDLE),
        m_nEmergencyCallKey(IMtcCall::CALL_KEY_INVALID),
        m_ePendingUnavailableAosReasonForHandover(ImsAosReason::NONE)
{
    IMS_TRACE_I("+EmergencyServiceController", 0, 0, 0);
}

PUBLIC VIRTUAL EmergencyServiceController::~EmergencyServiceController()
{
    IMS_TRACE_I("~EmergencyServiceController", 0, 0, 0);
    m_objContext.ReleaseAsyncOperation(this);
    RemoveListeners();
    StopWaitForHandoverToRetryOverImsPdnTimer();
}

PUBLIC VIRTUAL void EmergencyServiceController::Start()
{
    IMS_TRACE_I("Start", 0, 0, 0);
    Start18xWaitingTimer();

    switch (m_eState)
    {
        case IEmergencyServiceController::State::IDLE:
            AddListeners();

            ControlAos(ImsAosControl::REGISTER_START);
            SetState(IEmergencyServiceController::State::OPENING);
            Notify(IuMtcService::EmergencyServiceState::OPENING);
            break;

        // Exceptional cases that the Java layer didn’t receive the notification.
        case IEmergencyServiceController::State::OPENING:
            Notify(IuMtcService::EmergencyServiceState::OPENING);
            break;
        case IEmergencyServiceController::State::OPENED:
            Notify(IuMtcService::EmergencyServiceState::OPENED);
            break;
    }
}

PUBLIC VIRTUAL void EmergencyServiceController::Close()
{
    IMS_TRACE_I("Close", 0, 0, 0);
    Stop18xWaitingTimer();
    StopWaitForHandoverToRetryOverImsPdnTimer();

    ControlAos(ImsAosControl::REGISTER_STOP);
}

PUBLIC VIRTUAL void EmergencyServiceController::OnAosStateChanged(
        IN [[maybe_unused]] IMtcService& objMtcService, IN MtcAosState eState,
        IN IMS_UINT32 eAosReason, IN [[maybe_unused]] IMS_SINT32 nDataFailureReason)
{
    IMS_TRACE_I("OnAosStateChanged :: AosState[%s] Reason[%s]",
            MtcCallStringUtils::ConvertAosState(eState),
            MtcCallStringUtils::ConvertAosReason(eAosReason), 0);

    switch (m_eState)
    {
        case IEmergencyServiceController::State::IDLE:
            break;

        case IEmergencyServiceController::State::OPENING:
            if (eState == MtcAosState::CONNECTED)
            {
                HandleServiceOpened();
            }
            else if (eState == MtcAosState::DISCONNECTED)
            {
                HandleServiceUnavailable(eAosReason);
            }
            break;

        case IEmergencyServiceController::State::OPENED:
            if (eState == MtcAosState::DISCONNECTED)
            {
                Notify(IuMtcService::EmergencyServiceState::IDLE);
                Finish();
            }
            break;
    }
}

PUBLIC VIRTUAL void EmergencyServiceController::OnCallStateChanged(IN CallKey nCallKey,
        IN IMtcCall::State eState, IN Type /* eType */, IN IMS_BOOL bEmergency,
        IN IMS_SINT32 /* nReason */)
{
    if (!bEmergency || !IsCurrentEmergencyCall(nCallKey) ||
            m_eState != IEmergencyServiceController::State::OPENED)
    {
        return;
    }

    switch (eState)
    {
        case IMtcCall::State::IDLE:
            m_nEmergencyCallKey = nCallKey;
            Start18xWaitingTimer();
            break;

        case IMtcCall::State::TERMINATING:
            Stop18xWaitingTimer();
            break;
        default:
            break;
    }

    m_eEmergencyCallState = eState;
}

PUBLIC VIRTUAL void EmergencyServiceController::OnRatChanged(IN
        [[maybe_unused]] ServiceType eServiceType,
        IN [[maybe_unused]] IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType)
{
    if (eRatType == INetworkWatcher::RADIOTECH_TYPE_IWLAN)
    {
        return;
    }

    if (!m_objContext.GetPassiveTimerHolder().IsActive(
                IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN))
    {
        return;
    }

    IMS_TRACE_I("Normal service is handed over to WWAN", 0, 0, 0);
    StopWaitForHandoverToRetryOverImsPdnTimer();
    if (IsNetworkRoaming())
    {
        Notify(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                ConvertToUnavailableReason(m_ePendingUnavailableAosReasonForHandover));
        Finish();
    }
    else
    {
        FinishAndRetryOverImsPdn();
    }
}

PUBLIC VIRTUAL void EmergencyServiceController::OnCallSessionReleased(
        IN CallKey nCallKey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished)
{
    if (!bEmergency || !IsCurrentEmergencyCall(nCallKey) ||
            m_eState != IEmergencyServiceController::State::OPENED)
    {
        return;
    }

    m_nEmergencyCallKey = IMtcCall::CALL_KEY_INVALID;

    if (!bEstablished &&
            m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigEmergency::KEY_RELEASE_EMERGENCY_PDN_ON_FAILURE_AFTER_100_BOOL))
    {
        IMtcSession* piMtcSession = m_objContext.GetCallManager()
                                            .GetCallByCallKey(nCallKey)
                                            ->GetCallContext()
                                            .GetSession();
        if (piMtcSession &&
                m_objContext.GetMessageUtils().IsResponseExist(
                        &piMtcSession->GetISession(), SipStatusCode::SC_100))
        {
            Close();
        }
    }
}

PUBLIC void EmergencyServiceController::OnPassiveTimerExpired(IN IPassiveTimerHolder::Type eType)
{
    if (eType == IPassiveTimerHolder::Type::REGISTRATION_TO_18X)
    {
        IMS_TRACE_I("Registration ~ 18x timer expires", 0, 0, 0);

        if (m_nEmergencyCallKey == IMtcCall::CALL_KEY_INVALID)
        {
            ControlAos(ImsAosControl::REGISTER_STOP);
        }
        else
        {
            m_objContext.GetCallController().Terminate(m_nEmergencyCallKey,
                    CallReasonInfo(
                            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
        }
    }
    else if (eType == IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN)
    {
        IMS_TRACE_I("Wait for handover to retry over IMS PDN timer expires", 0, 0, 0);

        Notify(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                ConvertToUnavailableReason(m_ePendingUnavailableAosReasonForHandover));
        Finish();
    }
}

PRIVATE void EmergencyServiceController::HandleServiceOpened()
{
    SetState(IEmergencyServiceController::State::OPENED);
    Notify(IuMtcService::EmergencyServiceState::OPENED);
}

PRIVATE void EmergencyServiceController::HandleServiceUnavailable(IN IMS_UINT32 eAosReason)
{
    SetState(IEmergencyServiceController::State::IDLE);

    switch (GetRetryOverImsPdnAction(eAosReason))
    {
        case RetryOverImsPdnAction::RETRY_IMMEDIATELY:
            FinishAndRetryOverImsPdn();
            break;
        case RetryOverImsPdnAction::RETRY_AFTER_HANDOVER:
            IMS_TRACE_I("Wait for normal service to be handed over to WWAN", 0, 0, 0);
            m_ePendingUnavailableAosReasonForHandover = eAosReason;
            StartWaitForHandoverToRetryOverImsPdnTimer();
            break;
        default:  // RetryOverImsPdnAction::NO_RETRY
            Notify(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    ConvertToUnavailableReason(eAosReason));
            Finish();
            break;
    }
}

PRIVATE void EmergencyServiceController::AddListeners()
{
    m_objContext.GetPassiveTimerHolder().AddListener(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X, this);

    m_objContext.GetCallStateProxy().AddListener(this);

    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (pNormalService)
    {
        pNormalService->AddNetworkWatcherListener(this);
    }

    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pService)
    {
        pService->AddAosStateListener(this);
    }
}

PRIVATE void EmergencyServiceController::RemoveListeners()
{
    m_objContext.GetPassiveTimerHolder().RemoveListener(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X, this);
    m_objContext.GetPassiveTimerHolder().RemoveListener(
            IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN, this);

    m_objContext.GetCallStateProxy().RemoveListener(this);

    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (pNormalService)
    {
        pNormalService->RemoveNetworkWatcherListener(this);
    }

    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pService)
    {
        pService->RemoveAosStateListener(this);
    }
}

PRIVATE
void EmergencyServiceController::Notify(IN IuMtcService::EmergencyServiceState eState,
        IN EmergencyServiceUnavailableReason eReason) const
{
    IMS_TRACE_D("Notify :: state=%d, reason=%d", eState, eReason, 0);

    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    IJniMtcServiceThread* pThread = pService ? pService->GetJniServiceThread() : IMS_NULL;
    if (pThread == IMS_NULL)
    {
        return;
    }

    pThread->OnEmergencyServiceChanged(eState, eReason, ServiceType::EMERGENCY);
}

PRIVATE void EmergencyServiceController::ControlAos(IN IMS_UINT32 nType) const
{
    const IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(nType);
}

PRIVATE void EmergencyServiceController::Finish()
{
    Stop18xWaitingTimer();

    m_objContext.RunAsyncOperation(this,
            [&]()
            {
                m_objServiceManager.StopOpen(IMS_FALSE);
            });
}

PRIVATE void EmergencyServiceController::FinishAndRetryOverImsPdn()
{
    m_objContext.RunAsyncOperation(this,
            [&]()
            {
                m_objServiceManager.StartOpen(ServiceType::NORMAL);
            });
}

PRIVATE
void EmergencyServiceController::SetState(IN IEmergencyServiceController::State eState)
{
    IMS_TRACE_D("SetState :: state[%d]", eState, 0, 0);
    m_eState = eState;
}

PRIVATE IMS_BOOL EmergencyServiceController::IsCurrentEmergencyCall(IN CallKey nCallKey) const
{
    return m_nEmergencyCallKey == IMtcCall::CALL_KEY_INVALID || m_nEmergencyCallKey == nCallKey;
}

PRIVATE
EmergencyServiceController::RetryOverImsPdnAction
EmergencyServiceController::GetRetryOverImsPdnAction(IN IMS_SINT32 eAosReason) const
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
    {
        return RetryOverImsPdnAction::NO_RETRY;
    }

    if (eAosReason != ImsAosReason::DATA_DISCONNECTED)
    {
        return RetryOverImsPdnAction::NO_RETRY;
    }

    const IMtcService* pService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (!pService || !pService->IsActive())
    {
        return RetryOverImsPdnAction::NO_RETRY;
    }

    if (pService->IsWlanIpCanType())
    {
        return RetryOverImsPdnAction::RETRY_AFTER_HANDOVER;
    }

    if (IsNetworkRoaming())
    {
        return RetryOverImsPdnAction::NO_RETRY;
    }

    return RetryOverImsPdnAction::RETRY_IMMEDIATELY;
}

PRIVATE
IMS_BOOL EmergencyServiceController::IsNetworkRoaming() const
{
    INetworkWatcher* pNetworkWatcher =
            PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_objContext.GetSlotId());
    return pNetworkWatcher->GetRoamingState() != 0;
}

PRIVATE
void EmergencyServiceController::Start18xWaitingTimer()
{
    const IMtcService* pService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (!pService)
    {
        IMS_TRACE_E(0, "IMtcService is null", 0, 0, 0);
        return;
    }
    IMS_SINT32 nTime = MtcConfigurationResolver::GetRegistrationTo18xTimer(
            m_objContext.GetConfigurationProxy(), pService->IsWlanIpCanType());

    m_objContext.GetPassiveTimerHolder().AddTimer(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X, nTime, IMS_FALSE);
}

PRIVATE
void EmergencyServiceController::Stop18xWaitingTimer()
{
    m_objContext.GetPassiveTimerHolder().RemoveTimer(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X);
}

PRIVATE
void EmergencyServiceController::StartWaitForHandoverToRetryOverImsPdnTimer()
{
    IMS_SINT32 nTime = m_objContext.GetConfigurationProxy().GetInt(
            ConfigEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT);

    m_objContext.GetPassiveTimerHolder().AddTimer(
            IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN, nTime, IMS_FALSE,
            IMS_TRUE);
    m_objContext.GetPassiveTimerHolder().AddListener(
            IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN, this);
}

PRIVATE
void EmergencyServiceController::StopWaitForHandoverToRetryOverImsPdnTimer()
{
    m_objContext.GetPassiveTimerHolder().RemoveTimer(
            IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN);
}

PRIVATE
EmergencyServiceUnavailableReason EmergencyServiceController::ConvertToUnavailableReason(
        IN IMS_UINT32 eAosReason)
{
    if (eAosReason == ImsAosReason::DATA_PERMANENTLY_FAILED)
    {
        return EmergencyServiceUnavailableReason::DATA_PERMANENTLY_FAILED;
    }
    else if (eAosReason == ImsAosReason::NETWORK_ATTACH_REJECTED)
    {
        return EmergencyServiceUnavailableReason::NETWORK_ATTACH_REJECTED;
    }

    return EmergencyServiceUnavailableReason::NONE;
}
