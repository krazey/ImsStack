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

#include "IJniMtcServiceThread.h"
#include "IMtcContext.h"
#include "INetworkWatcher.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/EmergencyServiceController.h"
#include "helper/ICallStateProxy.h"
#include "helper/IMtcAosConnector.h"
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
    RemoveListeners();
}

PUBLIC VIRTUAL void EmergencyServiceController::Start()
{
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

    if (IsCallSetupUnsuccessful(eState) &&
            m_objContext.GetConfigurationProxy().Is(
                    Feature::RELEASE_EMERGENCY_PDN_WITH_EMERGENCY_CALL_FAIL))
    {
        Close();
    }

    if (eState == IMtcCall::State::OUTGOING)
    {
        m_nEmergencyCallKey = nCallKey;
    }
    else if (eState == IMtcCall::State::TERMINATING)
    {
        m_nEmergencyCallKey = IMtcCall::CALL_KEY_INVALID;
    }
    m_eEmergencyCallState = eState;
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
    m_objContext.GetCallStateProxy().AddListener(this);

    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pService)
    {
        pService->AddAosStateListener(this);
    }
}

PRIVATE void EmergencyServiceController::RemoveListeners()
{
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
    m_objContext.GetAsyncRunner(
            [&]()
            {
                m_objServiceManager.StopOpen(IMS_FALSE);
            });
}

PRIVATE void EmergencyServiceController::FinishAndRetryOverImsPdn()
{
    m_objContext.GetAsyncRunner(
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
IMS_BOOL EmergencyServiceController::IsCallSetupUnsuccessful(IN IMtcCall::State eState) const
{
    if (eState != IMtcCall::State::TERMINATING)
    {
        return IMS_FALSE;
    }

    return m_eEmergencyCallState == IMtcCall::State::IDLE ||
            m_eEmergencyCallState == IMtcCall::State::OUTGOING;
}

PRIVATE IMS_BOOL EmergencyServiceController::IsCurrentEmergencyCall(IN CallKey nCallKey) const
{
    return m_nEmergencyCallKey == IMtcCall::CALL_KEY_INVALID || m_nEmergencyCallKey == nCallKey;
}

PRIVATE
IMS_BOOL EmergencyServiceController::IsRetryOverImsPdnRequired(IN IMS_SINT32 eAosReason) const
{
    if (!m_objContext.GetConfigurationProxy().Is(Feature::RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
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
