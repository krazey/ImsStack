/*
 * Copyright (C) 2022 The Android Open Source Project
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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"
#include "ImsEventDef.h"
#include "CarrierConfig.h"
#include "ImsAosParameter.h"
#include "AosAppRequestType.h"
#include "AosReason.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosRegStateManager.h"
#include "condition/AosCondition.h"
#include "connection/AosConnector.h"
#include "provider/AosProvider.h"
#include "provider/AosLog.h"
#include "app/AosEApplication.h"

__IMS_TRACE_TAG_AOS__;

#define APPID m_strTag.GetStr()

PUBLIC
AosEApplication::AosEApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
        AosApplication(piAppContext, strAppId),
        m_bRegBlockInCbm(IMS_FALSE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosEApplication = %" PFLS_u "/%" PFLS_x, APPID,
            sizeof(AosEApplication), this);
}

PUBLIC VIRTUAL AosEApplication::~AosEApplication()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosEApplication = %" PFLS_u "/%" PFLS_x, APPID,
            sizeof(AosEApplication), this);
}

PUBLIC VIRTUAL IMS_BOOL AosEApplication::RequestCmd(
        IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(APPID, "RequestCmd :: Cmd (%s)",
            AosProvider::GetLog()->AppRequestToString(nCmdType), 0, 0);
    IMS_BOOL bResult = IMS_TRUE;

    switch (nCmdType)
    {
        case ImsAosControl::REGISTER_START:
            SetRegBlockInCbm(IMS_FALSE);
            if (IsNotReady())
            {
                return IMS_FALSE;
            }

            PostMessage(MSG_REG_START, nReason, 0);
            break;

        case ImsAosControl::REGISTER_STOP:
            PostMessage(MSG_REG_STOP, 0, 0);
            break;

        case ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF:
            if (m_piContext->GetConnection()->GetState() == IAosConnection::STATE_ACTIVE)
            {
                A_IMS_TRACE_D(APPID, "PDN is connected, do fake registration", 0, 0, 0);
                m_piRegistration->RequestCmd(IAosRegistration::CMD_FAKE_MODE,
                        IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF);
            }
            else
            {
                A_IMS_TRACE_D(APPID, "PDN is not connected", 0, 0, 0);
                ProcessCleanAll(AosReason::DATA_DISCONNECTED);
            }
            break;

        case ImsAosControl::RETRY_COUNT_INCREASE:  // FALL-THROUGH
        case ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION:
            AosApplication::RequestCmd(nCmdType);
            break;

        case CMD_ECALL_INIT:  // FALL-THROUGH
        case CMD_ESMS_INIT:
            SetRegBlockInCbm(IMS_FALSE);
            break;

        default:
            bResult = IMS_FALSE;
            break;
    }

    return bResult;
}

PUBLIC VIRTUAL void AosEApplication::GetProperty(
        IN IMS_UINT32 /* nType */, OUT IMS_UINT32& /* nValue */, OUT AString& strValue)
{
    strValue = AString::ConstNull();
}

PROTECTED void AosEApplication::SetRegBlockInCbm(IN IMS_BOOL bBlock)
{
    m_bRegBlockInCbm = bBlock;
}

PROTECTED IMS_BOOL AosEApplication::IsRegBlockInCbm() const
{
    return m_bRegBlockInCbm;
}

PROTECTED VIRTUAL void AosEApplication::ClearConnection()
{
    m_pConnector->Stop();
}

PROTECTED VIRTUAL void AosEApplication::ProcessCleanAll(IN IMS_UINT32 nReason /* = 0 */)
{
    CleanAll(nReason);
    Report_StateChanged(IMS_FALSE);
    ProcessStateStart();
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::ProcessMessage(IN IMSMSG& objMsg)
{
    IMS_BOOL bHandled = IMS_TRUE;

    switch (objMsg.nMSG)
    {
        case MSG_REG_START:
            ProcessRegStart(objMsg);
            break;

        case MSG_REG_STOP:
            ProcessRegStop(objMsg);
            break;

        case MSG_DESTROY:
            ProcessDestroy(objMsg);
            break;

        case MSG_RETRY_COUNT_INCREASE:
            ProcessRegRetryCount(objMsg);
            break;

        case MSG_IPCAN_CHANGED:
            if (m_bEpdgEnabled && GET_N_CONFIG(m_nSlotId)->IsKeepERegRetryOnWlanRequired())
            {
                StopTimer(TIMER_APP_CONNECTED);
            }

            if (GET_N_CONFIG(m_nSlotId)->IsEmergencyReregSupportedOnIpcanChange())
            {
                ProcessIpcanChanged(objMsg);
            }
            break;

        default:
            bHandled = IMS_FALSE;
            break;
    }

    return bHandled;
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegStart(IN IMSMSG& objMsg)
{
    IMS_UINT32 nIpcanType = LONG_TO_INT(objMsg.nWparam);

    StopTimer(TIMER_APP_ACTIVATED);

    if (IsOn())
    {
        Report_StateChanged(IMS_FALSE);
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetEmergencyRegistrationTimerMillis() > 0)
    {
        StartTimer(TIMER_APP_CONNECTED,
                GET_N_CONFIG(m_nSlotId)->GetEmergencyRegistrationTimerMillis());
    }

    if (m_pConnector->IsReady())
    {
        if (m_bEpdgEnabled && GET_N_CONFIG(m_nSlotId)->IsKeepERegRetryOnWlanRequired())
        {
            StopTimer(TIMER_APP_CONNECTED);
        }

        SetAppState(STATE_CONNECTING);
        m_piRegistration->Start();
    }
    else
    {
        SetAppState(STATE_READY);

        if (nIpcanType == AosRegType::TYPE_IPCAN_WLAN)
        {
            A_IMS_TRACE_I(APPID, "ProcessRegStart :: nIpcanType(%d) , emergency over WLAN",
                    nIpcanType, 0, 0);

            if (IsWlanEmergencyBlocked())
            {
                A_IMS_TRACE_I(APPID, "ProcessRegStart :: emergency over WLAN is blocked",
                        nIpcanType, 0, 0);
                ProcessCleanAll();
            }
            else
            {
                m_pConnector->Start();
            }
        }
        else
        {
            if (IsRegWaitingRequired())
            {
                ProcessRegStateCheck();
            }
            else
            {
                m_pConnector->Start();
            }
        }
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegStop(IN IMSMSG& /* objMsg */)
{
    CleanAll();
    Report_StateChanged(IMS_FALSE);
    ProcessStateStart();
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::StateNotReady_Condition(IN IMSMSG& /* objMsg */)
{
    if (!IsEmergencyBlocked())
    {
        SetAppState(STATE_READY);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::StateReady_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);

    switch (nType)
    {
        case CONNECTION_ACTIVATED:
            if (m_bEpdgEnabled && GET_N_CONFIG(m_nSlotId)->IsKeepERegRetryOnWlanRequired())
            {
                StopTimer(TIMER_APP_CONNECTED);
            }
            if (!IsRegBlockInCbm())
            {
                if (m_piRegistration->IsInCallbackMode())
                {
                    SetRegBlockInCbm(IMS_TRUE);
                }
                m_piRegistration->Start();
                SetAppState(STATE_CONNECTING);
            }
            else
            {
                if (!m_piRegistration->IsInCallbackMode())
                {
                    ProcessCleanAll(AosReason::DATA_DISCONNECTED);
                }
            }
            break;

        case CONNECTION_DEACTIVATED:
            if (m_piRegistration->IsInCallbackMode())
            {
                m_pConnector->Start();
            }
            else
            {
                ProcessCleanAll(AosReason::DATA_DISCONNECTED);
            }
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::StateReady_Condition(IN IMSMSG& /* objMsg */)
{
    if (IsEmergencyBlocked())
    {
        SetAppState(STATE_NOTREADY);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegFailed_StateUpdating(IN IMS_UINT32 /* nReason */)
{
    ProcessCleanAll(AosReason::REG_FAILURE);
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegFailed_StateConnecting(
        IN IMS_UINT32 /* nReason */)
{
    ProcessCleanAll(AosReason::REG_FAILURE);
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegFailed_StateConnected(IN IMS_UINT32 /* nReason */)
{
    ProcessCleanAll(AosReason::REG_FAILURE);
}

PROTECTED VIRTUAL void AosEApplication::ProcessConnectionUpdated_StateDisconnecting(
        IN IMS_UINT32 /* nReason */)
{
    ProcessCleanAll(AosReason::REG_FAILURE);
}

PROTECTED VIRTUAL void AosEApplication::ProcessConnectionDeactivated(IN IMS_UINT32 /* nReason */)
{
    if (m_piRegistration->IsInCallbackMode())
    {
        CleanAll(AosReason::DATA_CONNECTION_MAINTAIN);
        if (!IsEmergencyBlocked())
        {
            SetAppState(STATE_READY);
        }
    }
    else
    {
        ProcessCleanAll(AosReason::DATA_DISCONNECTED);
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessConnectionUpdated(IN IMS_UINT32 nReason)
{
    switch (GetState())
    {
        case STATE_CONNECTED:  // FALL-THROUGH
        case STATE_UPDATING:
            break;

        default:
            return;
    }

    switch (nReason)
    {
        case AosConnector::REASON_IP_CHANGED:
            if (m_piRegistration->IsInCallbackMode())
            {
                CleanAll(AosReason::DATA_CONNECTION_MAINTAIN);
                if (!IsRegBlockInCbm())
                {
                    SetAppState(STATE_CONNECTING);
                    m_piRegistration->Start();
                    Report_StateChanged(IMS_FALSE);
                    SetRegBlockInCbm(IMS_TRUE);
                    return;
                }
            }
            else
            {
                ProcessCleanAll(AosReason::IP_CHANGED);
            }
            break;

        case AosConnector::REASON_IPCAN_CAT_CHANGED:
            PostMessage(MSG_IPCAN_CHANGED, 0, 0);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegSucceeded(IN IMS_UINT32 nReason)
{
    StopTimer(TIMER_APP_CONNECTED);

    AosApplication::ProcessRegSucceeded(nReason);
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegFailed_Start(IN IMS_UINT32 /* nReason */)
{
    ProcessCleanAll(AosReason::REG_FAILURE);
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegFailed_Update(IN IMS_UINT32 /* nReason */)
{
    ProcessCleanAll(AosReason::REG_FAILURE);
}

PROTECTED VIRTUAL void AosEApplication::ProcessAppActivatedTimerExpired()
{
    StopTimer(TIMER_APP_ACTIVATED);

    if (m_piCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY) != CallState::OFFHOOK)
    {
        ProcessCleanAll();
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessAppConnectedTimerExpired()
{
    StopTimer(TIMER_APP_CONNECTED);

    if ((GetState() == STATE_CONNECTING) &&
            (GET_N_CONFIG(m_nSlotId)->GetPreferredEmergencyRegistration() ==
                    CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK))
    {
        m_piRegistration->RequestCmd(IAosRegistration::CMD_FAKE_MODE);
    }
    else
    {
        ProcessCleanAll(AosReason::DATA_DISCONNECTED);
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessAppTerminatedTimerExpired()
{
    StopTimer(TIMER_APP_TERMINATED);

    if (m_piCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY) == CallState::IDLE)
    {
        ProcessCleanAll();
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessReconfigTimerExpired()
{
    StopTimer(TIMER_RECONFIG_GUARD);

    ResetBlock(BLOCK_SERVICE_CONNECTING);
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegBlockedTimerExpired()
{
    StopTimer(TIMER_REG_BLOCKED);

    if (!m_pConnector->IsReady())
    {
        A_IMS_TRACE_I(APPID, "Emergency call fail since normal registration is not done", 0, 0, 0);
        ProcessCleanAll(AosReason::REG_FAILURE);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::IsEmergencyBlocked()
{
    return m_pCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED);
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::IsWifiConnected()
{
    IWifiWatcher* piWifi = PhoneInfoService::GetPhoneInfoService()->GetWifiWatcher();

    if (piWifi == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (piWifi->GetState() == IWifiWatcher::STATE_CONNECTED);
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::IsWlanEmergencyBlocked()
{
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::IsRegWaitingRequired()
{
    return (GET_N_CONFIG(m_nSlotId)->GetRegTimerForEmcCall() > 0);
}

PROTECTED VIRTUAL void AosEApplication::ProcessRegStateCheck()
{
    StopTimer(TIMER_REG_BLOCKED);

    IAosRegStateManager* piRegState = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (piRegState == IMS_NULL)
    {
        m_pConnector->Start();
        return;
    }

    A_IMS_TRACE_I(
            APPID, "ProcessRegStateCheck :: RegState(%d)", piRegState->GetImsRegState(), 0, 0);

    if (piRegState->GetImsRegState() == IMS_REG_ON)
    {
        m_pConnector->Start();
    }
    else
    {
        StartTimer(TIMER_REG_BLOCKED, GET_N_CONFIG(m_nSlotId)->GetRegTimerForEmcCall());
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessECallStarted()
{
    SetImsCall(IMS_TRUE);
    SetRegBlockInCbm(IMS_FALSE);

    if (m_piAppActivatedTimer != IMS_NULL)
    {
        StopTimer(TIMER_APP_ACTIVATED);
    }

    if (m_piAppTerminatedTimer != IMS_NULL)
    {
        StopTimer(TIMER_APP_TERMINATED);
    }
}

PROTECTED VIRTUAL void AosEApplication::ProcessECallTerminated()
{
    SetImsCall(IMS_FALSE);

    if (m_piRegistration->IsTerminated())
    {
        ProcessCleanAll();
        return;
    }

    if (IsTimerRunning(TIMER_APP_TERMINATED))
    {
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsEmergencyPdnWithEmergencyCallEndReleased() ||
            m_piRegistration->GetMode() == IAosRegistration::MODE_FAKE)
    {
        StartTimer(TIMER_APP_TERMINATED, EPDN_RELEASE_DELAY_TIME_MILLIS);
        return;
    }

    IMS_SINT32 delayTime = GET_N_CONFIG(m_nSlotId)->GetWaitTimeSecForReleaseEPdnAfterECallEnd();
    if (delayTime > 0)
    {
        StartTimer(TIMER_APP_TERMINATED, delayTime * 1000);
        return;
    }

    if (!IsTimerRunning(TIMER_APP_ACTIVATED))
    {
        // clean when e-call is terminated over ePDG
        if (m_piContext->GetConnection()->IsEpdgEnabled())
        {
            ProcessCleanAll();
            return;
        }
    }
}

PROTECTED VIRTUAL void AosEApplication::UpdateRegState()
{
    IMS_UINT32 nOn = IMS_REG_OFF;

    switch (m_nReportState)
    {
        case APP_CONNECTED:  // FALL-THROUGH
        case APP_UPDATING:
            nOn = IMS_REG_ON;
            break;

        default:
            break;
    }

    IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (piRsm != IMS_NULL)
    {
        piRsm->SetEImsRegState(nOn);
    }
}

PROTECTED VIRTUAL IMS_UINT32 AosEApplication::UpdateConnectedServices(
        IN IMS_BOOL /* bEnforceUpdateRegService */)
{
    return 0;
}

PROTECTED VIRTUAL void AosEApplication::Condition_RequestCommand(
        IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(APPID, "Condition_RequestCommand :: cmd(%d), reason(%d)", nCommand, nReason, 0);

    if (nCommand == AosCondition::REQUEST_DESTROY)
    {
        AosApplication::Condition_RequestCommand(nCommand, nReason);
    }
}

PROTECTED VIRTUAL void AosEApplication::CallTracker_StateChanged(
        IN IMS_UINT32 nType, IN CallState eState)
{
    if (nType != IAosCallTracker::TYPE_EMERGENCY)
    {
        return;
    }

    IMS_BOOL bCurrState = (eState > CallState::IDLE) ? IMS_TRUE : IMS_FALSE;

    if (IsImsCall() != bCurrState)
    {
        if (bCurrState)
        {
            ProcessECallStarted();
        }
        else
        {
            ProcessECallTerminated();
        }
    }
}

PROTECTED VIRTUAL void AosEApplication::NConfiguration_NotifyConfigChanged()
{
    AosApplication::NConfiguration_NotifyConfigChanged();

    IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (IsRegWaitingRequired())
    {
        piRsm->SetListener(this);
    }
    else
    {
        piRsm->SetListener(IMS_NULL);
    }
}

PROTECTED VIRTUAL void AosEApplication::RegStateManager_RegStateChanged(IN IMS_UINT32 nState)
{
    if (IsTimerRunning(TIMER_REG_BLOCKED))
    {
        A_IMS_TRACE_I(APPID, "RegStateManager_RegStateChanged :: state(%d)", nState, 0, 0);

        if (nState == IMS_REG_ON)
        {
            StopTimer(TIMER_REG_BLOCKED);
            m_pConnector->Start();
        }
    }
}

PROTECTED VIRTUAL void AosEApplication::Init()
{
    AosApplication::Init();

    if (IsRegWaitingRequired())
    {
        IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
        piRsm->SetListener(this);
    }
}

PROTECTED VIRTUAL void AosEApplication::CleanUp()
{
    IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    piRsm->SetListener(IMS_NULL);

    AosApplication::CleanUp();
}
