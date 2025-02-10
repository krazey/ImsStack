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
#include "IMtcCallController.h"
#include "IMtcContext.h"
#include "INetworkWatcher.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "emergency/EmergencyServiceController.h"
#include "helper/ICallStateProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EmergencyServiceController::EmergencyServiceController(
        IN IMtcEmergencyServiceManager& objServiceManager, IN IMtcContext& objContext) :
        m_objServiceManager(objServiceManager),
        m_objContext(objContext),
        m_eState(State::IDLE),
        m_eEmergencyCallState(IMtcCall::State::IDLE),
        m_nEmergencyCallKey(IMtcCall::CALL_KEY_INVALID)
{
    IMS_TRACE_I("+EmergencyServiceController", 0, 0, 0);
}

PUBLIC VIRTUAL EmergencyServiceController::~EmergencyServiceController()
{
    IMS_TRACE_I("~EmergencyServiceController", 0, 0, 0);
    m_objContext.ReleaseAsyncOperation(this);
    RemoveListeners();
}

PUBLIC VIRTUAL void EmergencyServiceController::Start()
{
    Start18xWaitingTimer();

    switch (m_eState)
    {
        case State::IDLE:
            AddListeners();

            ControlAos(ImsAosControl::REGISTER_START);
            SetState(State::OPENING);
            Notify(EmergencyServiceState::OPENING);
            break;

        // Exceptional cases that the Java layer didn’t receive the notification.
        case State::OPENING:
            Notify(EmergencyServiceState::OPENING);
            break;
        case State::OPENED:
            Notify(EmergencyServiceState::OPENED);
            break;
    }
}

PUBLIC VIRTUAL void EmergencyServiceController::Close()
{
    Stop18xWaitingTimer();

    ControlAos(ImsAosControl::REGISTER_STOP);
}

PUBLIC VIRTUAL void EmergencyServiceController::OnAosStateChanged(
        IN IMtcService& /* objMtcService */, IN MtcAosState eState, IN IMS_UINT32 eAosReason)
{
    switch (m_eState)
    {
        case State::IDLE:
            break;

        case State::OPENING:
            if (eState == MtcAosState::CONNECTED)
            {
                HandleServiceOpened();
            }
            else if (eState == MtcAosState::DISCONNECTED)
            {
                HandleServiceUnavailable(eAosReason);
            }
            break;

        case State::OPENED:
            if (eState == MtcAosState::DISCONNECTED)
            {
                Notify(EmergencyServiceState::IDLE, static_cast<IMS_SINT32>(eAosReason));
                Finish();
            }
            break;
    }
}

PUBLIC VIRTUAL void EmergencyServiceController::OnCallStateChanged(IN CallKey nCallKey,
        IN IMtcCall::State eState, IN Type /* eType */, IN IMS_BOOL bEmergency,
        IN IMS_SINT32 /* nReason */)
{
    if (!bEmergency || !IsCurrentEmergencyCall(nCallKey) || m_eState != State::OPENED)
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

            if (IsTerminatingCallSetupUnsuccessful(m_eEmergencyCallState) &&
                    m_objContext.GetConfigurationProxy().GetBoolean(ConfigEmergency::
                                    KEY_RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL_BOOL))
            {
                Close();
            }

            m_nEmergencyCallKey = IMtcCall::CALL_KEY_INVALID;
            break;
        default:
            break;
    }

    m_eEmergencyCallState = eState;
}

PUBLIC void EmergencyServiceController::OnPassiveTimerExpired(
        IN IPassiveTimerHolder::Type /* eType */)
{
    IMS_TRACE_I("Registration ~ 18x timer expires", 0, 0, 0);

    if (m_nEmergencyCallKey == IMtcCall::CALL_KEY_INVALID)
    {
        ControlAos(ImsAosControl::REGISTER_STOP);
    }
    else
    {
        m_objContext.GetCallController().Terminate(m_nEmergencyCallKey,
                CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
    }
}

PRIVATE void EmergencyServiceController::HandleServiceOpened()
{
    SetState(State::OPENED);
    Notify(EmergencyServiceState::OPENED);
}

PRIVATE void EmergencyServiceController::HandleServiceUnavailable(IN IMS_UINT32 eAosReason)
{
    SetState(State::IDLE);

    if (IsRetryOverImsPdnRequired(eAosReason))
    {
        FinishAndRetryOverImsPdn();
    }
    else
    {
        Notify(EmergencyServiceState::UNAVAILABLE, eAosReason);
        Finish();
    }
}

PRIVATE void EmergencyServiceController::AddListeners()
{
    m_objContext.GetPassiveTimerHolder().AddListener(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X, this);

    m_objContext.GetCallStateProxy().AddListener(this);

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

    m_objContext.GetCallStateProxy().RemoveListener(this);

    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pService)
    {
        pService->RemoveAosStateListener(this);
    }
}

PRIVATE
void EmergencyServiceController::Notify(
        IN EmergencyServiceState eState, IN IMS_SINT32 eReason) const
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
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
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
void EmergencyServiceController::SetState(IN State eState)
{
    IMS_TRACE_D("SetState :: state[%d]", eState, 0, 0);
    m_eState = eState;
}

PRIVATE
IMS_BOOL EmergencyServiceController::IsTerminatingCallSetupUnsuccessful(
        IN IMtcCall::State eOldState) const
{
    return eOldState == IMtcCall::State::IDLE || eOldState == IMtcCall::State::OUTGOING;
}

PRIVATE IMS_BOOL EmergencyServiceController::IsCurrentEmergencyCall(IN CallKey nCallKey) const
{
    return m_nEmergencyCallKey == IMtcCall::CALL_KEY_INVALID || m_nEmergencyCallKey == nCallKey;
}

PRIVATE
IMS_BOOL EmergencyServiceController::IsRetryOverImsPdnRequired(IN IMS_SINT32 eAosReason) const
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
    {
        return IMS_FALSE;
    }

    if (eAosReason != ImsAosReason::DATA_DISCONNECTED)
    {
        return IMS_FALSE;
    }

    INetworkWatcher* pNetworkWatcher =
            PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_objContext.GetSlotId());
    return pNetworkWatcher->GetRoamingState() == 0;
}

PRIVATE
void EmergencyServiceController::Start18xWaitingTimer()
{
    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
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
