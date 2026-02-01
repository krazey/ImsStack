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
#include "INetworkWatcher.h"
#include "IWifiWatcher.h"
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
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosRegStateManager.h"
#include "condition/AosCondition.h"
#include "connection/AosConnector.h"
#include "provider/AosProvider.h"
#include "provider/AosLog.h"
#include "provider/AosUtil.h"
#include "app/AosEApplication.h"

__IMS_TRACE_TAG_AOS__;

#define APPID m_strTag.GetStr()

PUBLIC
AosEApplication::AosEApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
        AosApplication(piAppContext, strAppId),
        m_bKeepEPdnWhenNoPcscf(IMS_FALSE),
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
            InitEmergencyVariable();
            if (IsNotReady())
            {
                return IMS_FALSE;
            }

            if (MaybeRedialOverCrossStack())
            {
                return IMS_TRUE;
            }

            PostMessage(MSG_REG_START, nReason, 0);
            break;

        case ImsAosControl::REGISTER_STOP:
            PostMessage(MSG_REG_STOP, 0, 0);
            break;

        case ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF:
            ProcessFakeRegRequest(IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF);
            break;

        case ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF:
            ProcessFakeRegRequest(IAosRegistration::REASON_FAKE_MODE_SAME_PCSCF);
            break;

        case CMD_ECALL_INIT:  // FALL-THROUGH
        case CMD_ESMS_INIT:
            SetRegBlockInCbm(IMS_FALSE);
            break;

        case ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY:
            PostMessage(MSG_SCSCF_RESTORATION, 0, nReason);
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

PROTECTED void AosEApplication::InitEmergencyVariable()
{
    SetKeepEPdnWhenNoPcscf(IMS_FALSE);
    SetRegBlockInCbm(IMS_FALSE);
}

PROTECTED void AosEApplication::SetKeepEPdnWhenNoPcscf(IN IMS_BOOL bEnable)
{
    m_bKeepEPdnWhenNoPcscf = bEnable;
}

PROTECTED void AosEApplication::SetRegBlockInCbm(IN IMS_BOOL bBlock)
{
    m_bRegBlockInCbm = bBlock;
}

PROTECTED void AosEApplication::ProcessFakeRegRequest(IN IMS_UINT32 nReason)
{
    if (m_piContext->GetConnection()->GetState() == IAosConnection::STATE_ACTIVE)
    {
        A_IMS_TRACE_D(APPID, "PDN is connected, do fake registration", 0, 0, 0);
        m_piRegistration->RequestCmd(IAosRegistration::CMD_FAKE_MODE, nReason);
    }
    else
    {
        A_IMS_TRACE_D(APPID, "PDN is not connected", 0, 0, 0);
        ProcessCleanAll(AosReason::DATA_DISCONNECTED);
    }
}

PROTECTED IMS_BOOL AosEApplication::IsKeepEPdnWhenNoPcscf() const
{
    return m_bKeepEPdnWhenNoPcscf;
}

PROTECTED IMS_BOOL AosEApplication::IsRegBlockInCbm() const
{
    return m_bRegBlockInCbm;
}

PROTECTED IMS_BOOL AosEApplication::IsReleaseEmergencyPdnUponEmergencyCallEnd()
{
    if (GET_N_CONFIG(m_nSlotId)->IsKeepEPdnUponPcscfUnavailable() && IsKeepEPdnWhenNoPcscf())
    {
        A_IMS_TRACE_I(APPID, "Keep EPDN: Wait until data disconnected", 0, 0, 0);
        SetKeepEPdnWhenNoPcscf(IMS_FALSE);
        return IMS_FALSE;
    }

    IAosNetTracker* piNetTracker = m_piContext->GetNetTracker();
    if (GET_N_CONFIG(m_nSlotId)->IsReleaseEPdnUponECallEndIfEAttach() &&
            piNetTracker->IsEmergencyAttach())
    {
        A_IMS_TRACE_D(APPID, "Release EPDN: Emergency Attach", 0, 0, 0);
        return IMS_TRUE;
    }

    // FAKE MODE
    if (m_piRegistration->GetMode() == IAosRegistration::MODE_FAKE)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsReleaseEPdnUponECallEndInFakeMode())
        {
            A_IMS_TRACE_D(APPID, "Release EPDN: Fake Mode", 0, 0, 0);
            return IMS_TRUE;
        }

        // Checks if the fake mode was caused by BLOCK_SUBSCRIBER_INCOMPLETED. If the fake mode was
        // caused by SIP error responses for EIMS registration, then it should follow the carrier's
        // configuration instead of handling EPDN based on the PLMN the UE is currently attached to.
        if (m_piContext->GetBlock()->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
        {
            const ImsVector<AString> plmns =
                    GET_N_CONFIG(m_nSlotId)->GetPlmnsReleaseEPdnUponECallEndInFakeMode();
            if (piNetTracker != IMS_NULL && plmns.Contains(piNetTracker->GetMobileNetworkPlmn()))
            {
                A_IMS_TRACE_D(APPID, "Release EPDN: Fake Mode (Blocked & PLMN Matched)", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    // IPCAN
    const IMS_UINT32 nIpcan =
            GET_N_CONFIG(m_nSlotId)->GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd();
    IMS_BOOL bShouldRelease = IMS_FALSE;

    switch (nIpcan)
    {
        case CarrierConfig::ImsEmergency::IPCAN_CELLULAR:
            bShouldRelease = !m_bEpdgEnabled;
            break;

        case CarrierConfig::ImsEmergency::IPCAN_WLAN:
            bShouldRelease = m_bEpdgEnabled;
            break;

        case CarrierConfig::ImsEmergency::IPCAN_ALL:
            bShouldRelease = IMS_TRUE;
            break;

        default:
            bShouldRelease = IMS_FALSE;
            break;
    }

    if (bShouldRelease)
    {
        A_IMS_TRACE_D(APPID, "Release EPDN: IPCAN Check (IPCAN:%d, EpdgEnabled:%d)", nIpcan,
                m_bEpdgEnabled, 0);
    }

    return bShouldRelease;
}

PROTECTED IMS_BOOL AosEApplication::MaybeRedialOverCrossStack()
{
    const ImsVector<IMS_SINT32>& objCauses =
            GET_N_CONFIG(m_nSlotId)->GetNetworkAttachRejectCausesForCrossStackRedial();
    if (objCauses.IsEmpty())
    {
        return IMS_FALSE;
    }

    IAosNetTracker* piNetTracker = m_piContext->GetNetTracker();
    if (piNetTracker == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const IMS_UINT32 nRejectCause = piNetTracker->GetMobileNetworkRegistrationRejectCause();
    if (objCauses.Contains(nRejectCause))
    {
        A_IMS_TRACE_D(APPID, "Network attach is rejected with cause(%d)", nRejectCause, 0, 0);
        PostMessage(MSG_REG_STOP, AosReason::NETWORK_ATTACH_REJECTED, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED void AosEApplication::UpdateReadyState()
{
    if (!IsEmergencyBlocked())
    {
        SetAppState(STATE_READY);
    }
    else
    {
        SetAppState(STATE_NOTREADY);
    }
}

PROTECTED VIRTUAL void AosEApplication::ClearConnection()
{
    m_pConnector->Stop();
}

PROTECTED VIRTUAL void AosEApplication::ProcessCleanAll(IN IMS_UINT32 nReason /* = 0 */)
{
    if (m_piNetTracker != IMS_NULL)
    {
        m_piNetTracker->RemoveListener(this);
        m_piNetTracker = IMS_NULL;
    }
    CleanAll(nReason);
    UpdateReadyState();
    Report_StateChanged(IMS_FALSE);
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

        case MSG_SCSCF_RESTORATION:
            ProcessScscfRestoration(objMsg);
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

        m_piContext->GetConnection()->SetActivationRequested(IMS_TRUE);
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

PROTECTED VIRTUAL void AosEApplication::ProcessRegStop(IN IMSMSG& objMsg)
{
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nWparam);
    CleanAll(nReason);
    UpdateReadyState();
    Report_StateChanged(IMS_FALSE);
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::StateNotReady_Condition(IN IMSMSG& /* objMsg */)
{
    UpdateReadyState();
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::StateReady_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    switch (nType)
    {
        case CONNECTION_ACTIVATED:
            if (m_bEpdgEnabled && GET_N_CONFIG(m_nSlotId)->IsKeepERegRetryOnWlanRequired())
            {
                StopTimer(TIMER_APP_CONNECTED);
            }
            else if (GET_N_CONFIG(m_nSlotId)->IsStopERegTimerOnEpdnConnected() &&
                    GET_N_CONFIG(m_nSlotId)->GetEmcRegRetryTimerMillis() > 0)
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
                if (nReason == AosConnector::REASON_PERMANENTLY_FAILED)
                {
                    ProcessCleanAll(AosReason::DATA_PERMANENTLY_FAILED);
                }
                else
                {
                    ProcessCleanAll(AosReason::DATA_DISCONNECTED);
                }
            }
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::StateReady_Condition(IN IMSMSG& /* objMsg */)
{
    UpdateReadyState();
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

PROTECTED VIRTUAL void AosEApplication::ProcessRegFailed_StateConnected(IN IMS_UINT32 nReason)
{
    if (nReason == IAosRegistration::REASON_FAILURE_NO_PCSCF_AVAILABLE)
    {
        SetKeepEPdnWhenNoPcscf(IMS_TRUE);
        ProcessCleanAll(AosReason::DATA_CONNECTION_MAINTAIN);
    }
    else
    {
        ProcessCleanAll(AosReason::REG_FAILURE);
    }
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
        ProcessCleanAll(AosReason::DATA_CONNECTION_MAINTAIN);
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
                ProcessCleanAll(AosReason::DATA_CONNECTION_MAINTAIN);
                if (!IsRegBlockInCbm())
                {
                    SetAppState(STATE_CONNECTING);
                    m_piRegistration->Start();
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
        if (GET_N_CONFIG(m_nSlotId)->IsRegTimerForECallTimeoutAsFailure())
        {
            A_IMS_TRACE_I(
                    APPID, "Emergency call fail since normal registration is not done", 0, 0, 0);
            ProcessCleanAll(AosReason::REG_FAILURE);
        }
        else
        {
            A_IMS_TRACE_I(APPID, "E-Call is proceeded even though normal reg is failed", 0, 0, 0);
            m_pConnector->Start();
        }
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
    if (GET_N_CONFIG(m_nSlotId)->GetRegTimerForEmcCall() <= 0)
    {
        return IMS_FALSE;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsRegTimerForECallWithRatCheckEnabled())
    {
        if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() && IsWifiConnected())
        {
            return IMS_TRUE;
        }

        INetworkWatcher* piNw =
                PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotId);
        if (piNw != IMS_NULL)
        {
            return m_pUtil->IsSupportedNetworkTypeForCellular(piNw->GetNetRadioTechType());
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosEApplication::IsECallConnectedNetworkUnavailable()
{
    IMS_BOOL bUnavailable = IMS_FALSE;
    if (!GET_N_CONFIG(m_nSlotId)->IsReleaseEPdnOfUnavailableNetwork())
    {
        A_IMS_TRACE_I(APPID, "Unavailable network check is not necessary", 0, 0, 0);
        return bUnavailable;
    }

    if (m_bEpdgEnabled)
    {
        bUnavailable = !IsWifiConnected();
        A_IMS_TRACE_I(APPID, "WiFi is %s", bUnavailable ? "not available" : "available", 0, 0);
    }
    else
    {
        INetworkWatcher* piNw =
                PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotId);
        if (piNw != IMS_NULL)
        {
            if (piNw->GetNetRadioTechType() == NW_REPORT_RADIO_NOSRV)
            {
                bUnavailable = IMS_TRUE;
            }
        }
        A_IMS_TRACE_I(APPID, "Cellular is %s", bUnavailable ? "not available" : "available", 0, 0);
    }
    return bUnavailable;
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

    if (IsReleaseEmergencyPdnUponEmergencyCallEnd())
    {
        StartTimer(TIMER_APP_TERMINATED, GET_N_CONFIG(m_nSlotId)->GetSipTimerT1());
    }
    else
    {
        if (IsECallConnectedNetworkUnavailable())
        {
            ProcessCleanAll();
        }
        else
        {
            SetNetTrackerListener();
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

PROTECTED VIRTUAL void AosEApplication::Registration_StateChanged(
        IN IMS_UINT32 nResult, IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsEqualOrLessState(STATE_READY) && nResult == IAosRegistration::RESULT_FAILURE)
    {
        if (!m_piRegistration->IsInCallbackMode())
        {
            ProcessCleanAll(AosReason::DATA_DISCONNECTED);
        }
    }

    AosApplication::Registration_StateChanged(nResult, nReason);
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

PROTECTED VIRTUAL void AosEApplication::CallTracker_ECallSessionReleased(IN IMS_BOOL bEstablished)
{
    A_IMS_TRACE_I(APPID, "CallTracker_ECallSessionReleased", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId)->IsKeepEPdnUponPcscfUnavailable() && !bEstablished)
    {
        A_IMS_TRACE_I(
                APPID, "do not release emergency pdn if the session is not established", 0, 0, 0);
        return;
    }

    SetImsCall(IMS_FALSE);

    if ((IsReleaseEmergencyPdnUponEmergencyCallEnd() || m_piRegistration->IsTerminated()))
    {
        ProcessCleanAll();
    }
}

PROTECTED VIRTUAL void AosEApplication::NConfiguration_NotifyConfigChanged()
{
    AosApplication::NConfiguration_NotifyConfigChanged();

    IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (GET_N_CONFIG(m_nSlotId)->GetRegTimerForEmcCall() > 0)
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

PROTECTED VIRTUAL void AosEApplication::NetTracker_StatusChanged()
{
    if (m_pConnector->IsReady() && !IsImsCall() && IsECallConnectedNetworkUnavailable())
    {
        A_IMS_TRACE_I(APPID, "Network is Unavailable", 0, 0, 0);
        ProcessCleanAll();
    }
}

PROTECTED VIRTUAL void AosEApplication::Init()
{
    AosApplication::Init();

    if (GET_N_CONFIG(m_nSlotId)->GetRegTimerForEmcCall() > 0)
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
