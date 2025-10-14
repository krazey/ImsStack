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
#include "ServiceEvent.h"
#include "ServiceTimer.h"
#include "ServiceUtil.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "CarrierConfig.h"
#include "AosAppRequestType.h"
#include "IAosService.h"
#include "IImsAosInfo.h"
#include "IImsAosMonitor.h"
#include "ImsAosParameter.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "interface/IAosHandle.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosRegStateManager.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "handle/AosFeatureTag.h"
#include "condition/AosCondition.h"
#include "condition/AosECondition.h"
#include "connection/AosConnector.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"
#include "provider/AosLog.h"
#include "provider/AosLocationStarter.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosStaticProfile.h"
#include "app/AosApplication.h"

__IMS_TRACE_TAG_AOS__;

#define APPID m_strTag.GetStr()

BEGIN_STATE_MAP(AosApplication)
STATE_ENTRY(STATE_NOTREADY)
STATE_ENTRY(STATE_READY)
STATE_ENTRY(STATE_CONNECTING)
STATE_ENTRY(STATE_CONNECTED)
STATE_ENTRY(STATE_UPDATING)
STATE_ENTRY(STATE_DISCONNECTING)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(AosApplication, STATE_NOTREADY)
STATE_MSG_ENTRY(MSG_CONDITION, &AosApplication::StateNotReady_Condition)
STATE_MSG_ENTRY(MSG_CONNECTION, &AosApplication::StateNotReady_Connection)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosApplication, STATE_READY)
STATE_MSG_ENTRY(MSG_CONDITION, &AosApplication::StateReady_Condition)
STATE_MSG_ENTRY(MSG_CONNECTION, &AosApplication::StateReady_Connection)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosApplication, STATE_CONNECTING)
STATE_MSG_ENTRY(MSG_CONDITION, &AosApplication::StateConnecting_Condition)
STATE_MSG_ENTRY(MSG_CONNECTION, &AosApplication::StateConnecting_Connection)
STATE_MSG_ENTRY(MSG_REGISTRATION, &AosApplication::StateConnecting_Registration)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosApplication, STATE_CONNECTED)
STATE_MSG_ENTRY(MSG_CONDITION, &AosApplication::StateConnected_Condition)
STATE_MSG_ENTRY(MSG_CONNECTION, &AosApplication::StateConnected_Connection)
STATE_MSG_ENTRY(MSG_REGISTRATION, &AosApplication::StateConnected_Registration)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosApplication, STATE_UPDATING)
STATE_MSG_ENTRY(MSG_CONDITION, &AosApplication::StateUpdating_Condition)
STATE_MSG_ENTRY(MSG_CONNECTION, &AosApplication::StateUpdating_Connection)
STATE_MSG_ENTRY(MSG_REGISTRATION, &AosApplication::StateUpdating_Registration)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosApplication, STATE_DISCONNECTING)
STATE_MSG_ENTRY(MSG_CONDITION, &AosApplication::StateDisconnecting_Condition)
STATE_MSG_ENTRY(MSG_CONNECTION, &AosApplication::StateDisconnecting_Connection)
STATE_MSG_ENTRY(MSG_REGISTRATION, &AosApplication::StateDisconnecting_Registration)
END_STATE_MSG_MAP()

PUBLIC
AosApplication::AosApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
        ImsActivityEx(strAppId),
        m_piContext(piAppContext),
        m_piRegistration(IMS_NULL),
        m_piCallTracker(IMS_NULL),
        m_piNetTracker(IMS_NULL),
        m_pCondition(IMS_NULL),
        m_pConnector(IMS_NULL),
        m_pUtil(IMS_NULL),
        m_piReconfigTimer(IMS_NULL),
        m_piMsgConditionTimer(IMS_NULL),
        m_piRegStopTimer(IMS_NULL),
        m_piRegBlockedTimer(IMS_NULL),
        m_piAppActivatedTimer(IMS_NULL),
        m_piAppConnectedTimer(IMS_NULL),
        m_piAppTerminatedTimer(IMS_NULL),
        m_piPdnBlockedTimer(IMS_NULL),
        m_piImsEstablishmentTimer(IMS_NULL),
        m_piRatBlockTimer(IMS_NULL),
        m_strAppId(strAppId),
        m_nAppType(TYPE_NORMAL),
        m_nOffReason(AosReason::NONE),
        m_nRat(NW_REPORT_RADIO_INVALID),
        m_nBlockedRats(NW_REPORT_RADIO_INVALID),
        m_eRegType(AosRegistrationType::NORMAL),
        m_nReportState(APP_DISCONNECTED),
        m_nRegPending(PENDING_NONE),
        m_nRegisteredRat(NW_REPORT_RADIO_INVALID),
        m_nRecoverReason(0),
        m_nLteAttachState(IMS_LTE_INFO_UNKNOWN),
        m_nLteExtraInfo(IMS_LTE_INFO_EXTRA_NONE),
        m_nVoiceServiceState(IMS_VOICE_SERVICE_OUT_OF_SERVICE),
        m_nSlotId(piAppContext->GetSlotId()),
        m_nDataFailureReason(0),
        m_bConnected(IMS_FALSE),
        m_bRegRecoveryHeld(IMS_FALSE),
        m_bIsImsCall(IMS_FALSE),
        m_bIsPublished(IMS_FALSE),
        m_bIsActivated(IMS_TRUE),
        m_bEpdgEnabled(IMS_FALSE),
        m_bDataRoaming(IMS_FALSE),
        m_bPdnDeactivationRequired(IMS_FALSE)
{
    m_strTag.Sprintf("%d:%s", m_nSlotId, strAppId.GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosApplication = %" PFLS_u "/%" PFLS_x, APPID,
            sizeof(AosApplication), this);
}

PUBLIC VIRTUAL AosApplication::~AosApplication()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosApplication = %" PFLS_u "/%" PFLS_x, APPID,
            sizeof(AosApplication), this);
}

PUBLIC VIRTUAL void AosApplication::Reconfig()
{
    A_IMS_TRACE_I(APPID, "Reconfig", 0, 0, 0);

    if (IsEqualOrLessState(STATE_READY))
    {
        m_pCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    }

    StartTimer(TIMER_RECONFIG_GUARD, RECONFIG_GUARD_TIME_MILLIS);
}

PUBLIC VIRTUAL IMS_BOOL AosApplication::RequestCmd(
        IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(APPID, "RequestCmd :: Cmd (%s)",
            AosProvider::GetLog()->AppRequestToString(nCmdType), 0, 0);

    IMS_BOOL bResult = IMS_TRUE;
    switch (nCmdType)
    {
        case ImsAosControl::REGISTER_START:
            if (m_piRegistration->GetState() == IAosRegistration::STATE_OFFLINE)
            {
                PostMessage(MSG_REG_START, 0, 0);
            }
            else
            {
                bResult = IMS_FALSE;
            }
            break;

        case ImsAosControl::REGISTER_START_WITH_WLAN:
            if (m_piContext->GetConnection()->IsEpdgEnabled() &&
                    m_piRegistration->GetState() == IAosRegistration::STATE_OFFLINE)
            {
                PostMessage(MSG_REG_START, 0, 0);
            }
            else
            {
                bResult = IMS_FALSE;
            }
            break;

        case ImsAosControl::REGISTER_REFRESH:
            // nWparam : bIgnoreRetryTimer , nLparam : bExplicitUpdate
            PostMessage(MSG_REG_UPDATE, 0, 1);
            break;

        case ImsAosControl::REGISTER_STOP:  // FALL-THROUGH
        case ImsAosControl::REGISTER_STOP_BY_ROAMING:
            // TODO : check the roaming operation for REGISTER_STOP_BY_ROAMING
            ProcessDisconnectingState();
            PostMessage(MSG_REG_STOP, 0, 0);
            break;

        case ImsAosControl::REGISTER_REINITIATE:  // FALL-THROUGH
        case ImsAosControl::REGISTER_REINITIATE_BY_CSFB:
            // TODO : check csfb operation for REGISTER_REINITIATE_BY_CSFB
            PostMessage(MSG_REG_RECOVER, 0, 0);
            break;

        case ImsAosControl::PCSCF_NEXT:
            PostMessage(MSG_PCSCF_RECOVER, AosRegRecoveryType::PCSCF_CHANGE, 0);
            break;

        case ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY:
            PostMessage(MSG_SCSCF_RESTORATION, 0, nReason);
            break;

        case ImsAosControl::IPSEC_DISABLED:
            m_piRegistration->RequestCmd(
                    IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_DISABLE);
            break;

        case ImsAosControl::RETRY_COUNT_INCREASE:
            PostMessage(MSG_RETRY_COUNT_INCREASE, RETRY_COUNT_REG_NONE, 0);
            break;

        case ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION:
            PostMessage(MSG_RETRY_COUNT_INCREASE, RETRY_COUNT_REG_RECOVER, 0);
            break;

        case ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION:  // FALL-THROUGH
        case ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION:
            // TODO
            break;

        case ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION:
            PostMessage(MSG_REG_RECOVER, 0, 0);
            break;

        case ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT:
            PostMessage(MSG_PLMN_BLOCK_WITH_TIMEOUT, nReason, 0);
            break;

        default:
            bResult = IMS_FALSE;
            break;
    }

    return bResult;
}

PUBLIC VIRTUAL const AString& AosApplication::GetActivityName()
{
    A_IMS_TRACE_D(APPID, "GetActivityName :: (%s)", GetName().GetStr(), 0, 0);

    return GetName();
}

PUBLIC VIRTUAL void AosApplication::GetProperty(
        IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue)
{
    strValue = AString::ConstNull();

    switch (nType)
    {
        case PROPERTY_REGISTERED_RAT:
            nValue = m_nRegisteredRat;
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL IMS_UINT32 AosApplication::GetAppState()
{
    return GetState();
}

PUBLIC VIRTUAL IMS_UINT32 AosApplication::GetOffReason()
{
    return m_nOffReason;
}

PUBLIC VIRTUAL IMS_BOOL AosApplication::IsActivated()
{
    return m_bIsActivated;
}

PUBLIC VIRTUAL IMS_BOOL AosApplication::IsOn()
{
    IMS_BOOL bConnected = IMS_FALSE;

    switch (GetState())
    {
        case STATE_CONNECTED:  // FALL-THROUGH
        case STATE_UPDATING:
            bConnected = IMS_TRUE;
            break;

        default:
            break;
    }

    return bConnected;
}

PUBLIC VIRTUAL IMS_BOOL AosApplication::IsCrossSimConnected()
{
    return m_pConnector->IsCrossSimConnected();
}

PUBLIC VIRTUAL void AosApplication::SetActivation(IN IMS_BOOL bActivation)
{
    m_bIsActivated = bActivation;
}

PUBLIC VIRTUAL void AosApplication::NotifyEpsFallbackCallState(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_D(APPID, "NotifyEpsFallbackCallState :: %d", nState, 0, 0);

    if (!IsRegTypeNormal())
    {
        return;
    }

    if (nState == IImsAosInfo::EPSFB_CALL_START)
    {
        m_piRegistration->RequestCmd(IAosRegistration::CMD_CLOSE_UNSECURE_TCP_SOCKET, 0);
        CleanAll();
        Report_StateChanged(IMS_FALSE);
        m_pCondition->SetBlock(BLOCK_EPS_FALLBACK_STARTED, IMS_FALSE);
    }
    else if (nState == IImsAosInfo::EPSFB_CALL_FAILED)
    {
        m_pCondition->ResetBlock(BLOCK_EPS_FALLBACK_STARTED);
    }
}

PUBLIC VIRTUAL void AosApplication::NotifyPublishState(IN IMS_BOOL bStart)
{
    SetPublishState(bStart);

    if (!bStart)
    {
        if (m_piRegStopTimer != IMS_NULL)
        {
            PostMessage(MSG_PUB_TERMINATED, 0, 0);
        }
    }
}

PROTECTED
void AosApplication::ClearOffReason()
{
    m_nOffReason = AosReason::NONE;
}

PROTECTED
void AosApplication::ClearPending()
{
    m_nRegPending = PENDING_NONE;
}

PROTECTED
void AosApplication::ClearWifiRegBlock()
{
    if (!IsRegTypeNormal() ||
            GET_N_CONFIG(m_nSlotId)->GetSubConsecutiveRetryCntForRegForbiddenInWifi() <= 0)
    {
        return;
    }

    m_pCondition->ResetBlock(BLOCK_WIFI_REG_FORBIDDEN);
}

PROTECTED
void AosApplication::ClearDataFailureReason()
{
    m_nDataFailureReason = 0;
}

PROTECTED
AosNetworkType AosApplication::GetNetworkTypeForImsRegState() const
{
    if (m_pUtil->IsWifiTest())
    {
        return AosNetworkType::LTE;
    }

    return m_pUtil->GetAosNetworkType(m_piContext->GetNetTracker()->GetNetworkType());
}

PROTECTED
void AosApplication::SetOffReason(IN IMS_UINT32 nReason)
{
    m_nOffReason = nReason;
}

PROTECTED
void AosApplication::SetImsCall(IN IMS_BOOL bActive)
{
    m_bIsImsCall = bActive;
}

PROTECTED
void AosApplication::SetPublishState(IN IMS_BOOL bActive)
{
    m_bIsPublished = bActive;
}

PROTECTED
void AosApplication::SetRegRecoveryHeld(IN IMS_BOOL bHeld)
{
    m_bRegRecoveryHeld = bHeld;
}

PROTECTED
void AosApplication::SetDataFailureReason(IN IMS_SINT32 nDataFailureReason)
{
    m_nDataFailureReason = nDataFailureReason;
}

PROTECTED
void AosApplication::ResetBlock(IN BLOCK_REASON nReason)
{
    if (m_pCondition->IsReasonBlocked(nReason))
    {
        m_pCondition->ResetBlock(nReason);
    }
}

PROTECTED
void AosApplication::NotifyDeregistered(IN AosReasonCode eReason)
{
    if (!IsRegTypeNormal())
    {
        return;
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        A_IMS_TRACE_D(APPID, "NotifyDeregistered :: Reason(%d)", eReason, 0, 0);
        piService->NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL,
                (eReason == AosReasonCode::CLEAR_RAT_BLOCKS) ? AosNetworkType::NONE
                                                             : GetNetworkTypeForImsRegState(),
                eReason, 0);
    }
}

PROTECTED
void AosApplication::NotifyDeregistering()
{
    if (!IsRegTypeNormal())
    {
        return;
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        A_IMS_TRACE_D(APPID, "NotifyDeregistering", 0, 0, 0);
        piService->NotifyDeregistering(IAosRegistration::IMS_REG_TYPE_NORMAL);
    }
}

PROTECTED
void AosApplication::AddRatBlock()
{
    m_nBlockedRats |= m_nRat;
}

PROTECTED
void AosApplication::ClearRatBlocks()
{
    m_nBlockedRats = NW_REPORT_RADIO_INVALID;
}

PROTECTED
void AosApplication::PerformRatBlockActions(IN IMS_BOOL bStart)
{
    A_IMS_TRACE_D(APPID, "PerformRatBlockActions :: %s", _TRACE_B_(bStart), 0, 0);

    if (bStart)
    {
        StartTimer(TIMER_RAT_BLOCK, RAT_BLOCK_TIME_MILLIS);
        AddRatBlock();
        m_pCondition->SetBlock(BLOCK_CELLULAR_RAT_BLOCK);
        CleanAll();
    }
    else
    {
        StopTimer(TIMER_RAT_BLOCK);
        ClearRatBlocks();
        m_pCondition->ResetBlock(BLOCK_CELLULAR_RAT_BLOCK);
    }
}

PROTECTED
IMS_BOOL AosApplication::IsEmergency() const
{
    return (m_nAppType == TYPE_EMERGENCY);
}

PROTECTED
IMS_BOOL AosApplication::IsStateMessage(IN IMS_UINT32 nMsg) const
{
    return (nMsg < MSG_INIT);
}

PROTECTED
IMS_BOOL AosApplication::IsNotReady()
{
    return (GetState() == STATE_NOTREADY);
}

PROTECTED
IMS_BOOL AosApplication::IsEqualOrLessState(IN IMS_UINT32 nState)
{
    return (GetState() <= nState);
}

PROTECTED
IMS_BOOL AosApplication::IsRegRecoveryHeld() const
{
    return m_bRegRecoveryHeld;
}

PROTECTED
IMS_BOOL AosApplication::IsImsCall() const
{
    return m_bIsImsCall;
}

PROTECTED
IMS_BOOL AosApplication::IsPublished() const
{
    return m_bIsPublished;
}

PROTECTED
IMS_BOOL AosApplication::IsAllDetached() const
{
    return (m_pCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
}

PROTECTED
IMS_BOOL AosApplication::IsTimerRunning(IN IMS_UINT32 nType) const
{
    if (nType == TIMER_REG_STOP)
    {
        return (m_piRegStopTimer != IMS_NULL);
    }

    if (nType == TIMER_RECONFIG_GUARD)
    {
        return (m_piReconfigTimer != IMS_NULL);
    }

    if (nType == TIMER_MSG_CONDITION)
    {
        return (m_piMsgConditionTimer != IMS_NULL);
    }

    if (nType == TIMER_REG_BLOCKED)
    {
        return (m_piRegBlockedTimer != IMS_NULL);
    }

    if (nType == TIMER_APP_ACTIVATED)
    {
        return (m_piAppActivatedTimer != IMS_NULL);
    }

    if (nType == TIMER_APP_CONNECTED)
    {
        return (m_piAppConnectedTimer != IMS_NULL);
    }

    if (nType == TIMER_APP_TERMINATED)
    {
        return (m_piAppTerminatedTimer != IMS_NULL);
    }

    if (nType == TIMER_PDN_BLOCKED)
    {
        return (m_piPdnBlockedTimer != IMS_NULL);
    }

    if (nType == TIMER_IMS_ESTABLISHMENT)
    {
        return (m_piImsEstablishmentTimer != IMS_NULL);
    }

    if (nType == TIMER_RAT_BLOCK)
    {
        return (m_piRatBlockTimer != IMS_NULL);
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosApplication::IsRegTypeNormal() const
{
    return (m_eRegType == AosRegistrationType::NORMAL);
}

PROTECTED
IMS_BOOL AosApplication::IsRegStateUpdatedByNrLteRatChange() const
{
    return IsRegTypeNormal();
}

PROTECTED
IMS_BOOL AosApplication::IsRegisteredNetwork(IN IMS_UINT32 nNetworkType) const
{
    AosNetworkType eCurrentNetwork = m_pUtil->GetAosNetworkType(nNetworkType);
    AosNetworkType eImsRegNetwork = m_piRegistration->GetImsRegNetwork();

    A_IMS_TRACE_D(APPID, "IsRegisteredNetwork :: Current Network(%d), Registered Network(%d)",
            eCurrentNetwork, eImsRegNetwork, 0);

    if (eImsRegNetwork == AosNetworkType::NONE)
    {
        return IMS_FALSE;
    }

    return (eImsRegNetwork == eCurrentNetwork);
}

PROTECTED IMS_BOOL AosApplication::IsPdnDisconnectRequired() const
{
    if (IsEmergency())
    {
        return IMS_FALSE;
    }

    if (m_pCondition && m_pCondition->IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED))
    {
        return IMS_TRUE;
    }

    switch (m_nOffReason)
    {
        case AosReason::IMS_DISABLED:  // FALL-THROUGH
        case AosReason::POWER_OFF:     // FALL-THROUGH
        case AosReason::DATA_PERMANENTLY_FAILED:
            return IMS_TRUE;

        default:
            break;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosApplication::IsPlmnBlockRequired() const
{
    if (m_piNetTracker->GetMobileNetworkType() == NW_REPORT_RADIO_LTE &&
            m_nLteAttachState == IMS_LTE_INFO_COMBINED_ATTACHED &&
            m_nLteExtraInfo == IMS_LTE_INFO_EXTRA_NONE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosApplication::IsBlockRat(IN IMS_UINT32 nRat) const
{
    return m_nBlockedRats & nRat;
}

PROTECTED
IMS_BOOL AosApplication::IsReasonBlockedForImsEstablishmentTimer() const
{
    if (m_pCondition->IsReasonBlocked(BLOCK_AC_INCOMPLETED) ||
            m_pCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED) ||
            m_pCondition->IsReasonBlocked(BLOCK_USIM_AUTHENTICATION_FAILED) ||
            m_pCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED) ||
            m_pCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED) ||
            m_pCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED) ||
            m_pCondition->IsReasonBlocked(BLOCK_IMS_DISABLED) ||
            m_pCondition->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED) ||
            m_pCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED) ||
            m_pCondition->IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED) ||
            m_pCondition->IsReasonBlocked(BLOCK_INVALID_CONNECTION))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosApplication::IsImsEstablishmentTimerStopRequired() const
{
    if (m_piContext->GetConnection()->IsEpdgEnabled())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - ePDG enabled", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_piRegistration->IsRegistered())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - Registered", 0, 0, 0);
        return IMS_TRUE;
    }

    if (IsImsCall())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - Ims call is active", 0, 0, 0);
        return IMS_TRUE;
    }

    if (!m_piNetTracker->IsDataIn())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - Data OOS", 0, 0, 0);
        return IMS_TRUE;
    }

    if (!m_pUtil->IsSupportedNetworkTypeForCellular(m_piNetTracker->GetMobileNetworkType()))
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - Not supported mobile network", 0, 0, 0);
        return IMS_TRUE;
    }

    if (!IsPlmnBlockRequired())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - LTE combined attached", 0, 0, 0);
        return IMS_TRUE;
    }

    if (!m_piNetTracker->IsImsVoiceCallSupported())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - VoPS not supported", 0, 0, 0);
        return IMS_TRUE;
    }

    if (IsReasonBlockedForImsEstablishmentTimer())
    {
        A_IMS_TRACE_D(APPID, "Stop ImsEstablishmentTimer - Block reasons exist", 0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosApplication::IsPdnDeactivationRequired() const
{
    return m_bPdnDeactivationRequired;
}

PROTECTED
IMS_SINT32 AosApplication::GetImsEstablishmentTime() const
{
    return (m_piNetTracker->GetMobileNetworkType() == NW_REPORT_RADIO_NR)
            ? GET_N_CONFIG(m_nSlotId)->GetImsEstablishmentTimeForNr()
            : GET_N_CONFIG(m_nSlotId)->GetImsEstablishmentTimeForLte();
}

PROTECTED VIRTUAL void AosApplication::CreateAosCondition()
{
    if (m_nAppType == TYPE_NORMAL)
    {
        m_pCondition = new AosCondition(m_piContext);
    }
    else
    {
        m_pCondition = new AosECondition(m_piContext);
    }

    m_pCondition->SetListener(this);
}

PROTECTED VIRTUAL void AosApplication::CreateAosConnector()
{
    m_pConnector = new AosConnector(m_piContext);
    m_pConnector->SetListener(this);
}

PROTECTED VIRTUAL void AosApplication::CreateAosLocationStarter(
        IN IMS_BOOL bInitiation /* = IMS_TRUE */)
{
    const IAosLocationStarter* piLs = AosProvider::GetInstance()->GetLocationStarter(m_nSlotId);

    if (piLs != IMS_NULL)
    {
        return;
    }

    IAosLocationStarter* piStarter = new AosLocationStarter();
    AosProvider::GetInstance()->SetLocationStarter(piStarter, m_nSlotId);

    if (bInitiation)
    {
        piStarter->Init(m_piContext);
    }
}

PROTECTED VIRTUAL void AosApplication::AddEventListener()
{
    if (m_nAppType == TYPE_NORMAL)
    {
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_REG_CONTROL, this, m_nSlotId);
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_nSlotId);
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
    }
}

PROTECTED VIRTUAL void AosApplication::RemoveEventListener()
{
    if (m_nAppType == TYPE_NORMAL)
    {
        IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
        IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_nSlotId);
        IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_REG_CONTROL, this, m_nSlotId);
    }
}

PROTECTED VIRTUAL void AosApplication::SetNetTrackerListener()
{
    if (m_piNetTracker == IMS_NULL)
    {
        m_piNetTracker = m_piContext->GetNetTracker();
        if (m_piNetTracker != IMS_NULL)
        {
            m_piNetTracker->SetListener(this);
        }
    }
}

PROTECTED VIRTUAL void AosApplication::SetAppType(IN AosRegistrationType eRegType)
{
    m_eRegType = eRegType;

    if (m_eRegType == AosRegistrationType::EMERGENCY)
    {
        m_nAppType = TYPE_EMERGENCY;
    }
    else
    {
        m_nAppType = TYPE_NORMAL;
    }
}

PROTECTED VIRTUAL void AosApplication::SetAppState(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(APPID, "SetAppState :: old (%s) , curr (%s)",
            AosProvider::GetLog()->AppStateToString(GetState()),
            AosProvider::GetLog()->AppStateToString(nState), 0);

    SetState(nState);

    if (m_piRegistration)
    {
        m_piRegistration->SetAppReady((IsUpdateAvailable()) ? IMS_TRUE : IMS_FALSE);
    }
}

PROTECTED VIRTUAL void AosApplication::SetCleanState()
{
    SetAppState((m_pCondition->IsReady()) ? STATE_READY : STATE_NOTREADY);
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsUpdateAvailable()
{
    IMS_BOOL bOk = IMS_FALSE;

    switch (GetState())
    {
        case STATE_CONNECTING:  // FALL-THROUGH
        case STATE_CONNECTED:   // FALL-THROUGH
        case STATE_UPDATING:
            bOk = IMS_TRUE;
            break;

        default:
            break;
    }

    return bOk;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsRegReconfigAvailable() const
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsReconfigHandleChanged() const
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_BOOL bChanged = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (piHandle->IsRegBinded())
        {
            if (piHandle->GetRequestType() == IAosHandle::DETACH)
            {
                A_IMS_TRACE_I(APPID, "IsReconfigHandleChanged :: AppId(%s), SrvId(%s) => DETACH",
                        piHandle->GetAppId().GetStr(), piHandle->GetServiceId().GetStr(), 0);

                bChanged = IMS_TRUE;
                break;
            }
        }
        else
        {
            if (piHandle->GetRequestType() == IAosHandle::ATTACH)
            {
                A_IMS_TRACE_I(APPID, "IsReconfigHandleChanged :: AppId(%s), SrvId(%s) => ATTACH",
                        piHandle->GetAppId().GetStr(), piHandle->GetServiceId().GetStr(), 0);

                bChanged = IMS_TRUE;
                break;
            }
        }

        if (piHandle->GetRequestType() == IAosHandle::ATTACH)
        {
            // check to change the extra feature tag
            AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
            if (!objBindedList.Equals(piHandle->GetFeatureTagList()))
            {
                bChanged = IMS_TRUE;
                break;
            }
        }
    }

    return bChanged;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsRequestCmdHeldByCondition(
        IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason)
{
    if (nReason == AosReason::POWER_OFF)
    {
        return IMS_FALSE;
    }

    if (IsImsCall() && (nCommand == AosCondition::REQUEST_STOP))
    {
        m_pUtil->AddFeature(PENDING_REG_STOP_HELD, m_nRegPending);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsAllHandleDetached() const
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (piHandle->GetRequestType() == IAosHandle::ATTACH && piHandle->IsRegFeatureTagRequired())
        {
            return IMS_FALSE;
        }
    }

    A_IMS_TRACE_I(APPID, "IsAllHandleDetached :: all services are detached", 0, 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsConditionTimerSkippedDueToTimer() const
{
    return IsTimerRunning(TIMER_MSG_CONDITION);
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::IsRegUpdatedByNrLteRatChange() const
{
    ImsVector<IMS_SINT32>& objRegUpdateRats =
            GET_N_CONFIG(m_nSlotId)->GetUpdateRegistrationWithRatChange();

    IMS_BOOL bLte = IMS_FALSE;
    IMS_BOOL bNr = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objRegUpdateRats.GetSize(); i++)
    {
        IMS_SINT32 nRat = objRegUpdateRats.GetAt(i);
        if (nRat == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN)
        {
            bLte = IMS_TRUE;
        }

        if (nRat == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN)
        {
            bNr = IMS_TRUE;
        }
    }

    return (bLte && bNr);
}

PROTECTED VIRTUAL void AosApplication::CleanAll(IN IMS_UINT32 nOffReason /* = AosReason::NONE */)
{
    A_IMS_TRACE_I(APPID, "CleanAll:: nOffReason (%d)", nOffReason, 0, 0);

    SetAppState(STATE_NOTREADY);

    ClearPending();
    SetOffReason(nOffReason);

    ClearTimers();

    if (nOffReason != AosReason::DATA_CONNECTION_MAINTAIN)
    {
        ClearConnection();
    }

    if (m_piRegistration)
    {
        if (nOffReason == AosReason::SERVICE_POLICY &&
                GET_N_CONFIG(m_nSlotId)->IsKeepRegRetryTimerOnAllEnablersDetached() &&
                m_piRegistration->GetState() == IAosRegistration::STATE_REGSTOP &&
                m_piRegistration->IsRetryTimer())
        {
            A_IMS_TRACE_D(APPID, "CleanAll :: Keep registration while stop retry timer is running",
                    0, 0, 0);
        }
        else
        {
            m_piRegistration->Destroy();
        }
    }

    if (IsPdnDisconnectRequired())
    {
        ProcessPdnDisconnect();
        ClearOffReason();
        ClearDataFailureReason();
    }

    if (IsPdnDeactivationRequired())
    {
        m_pConnector->Stop();
        m_bPdnDeactivationRequired = IMS_FALSE;
    }
}

PROTECTED VIRTUAL void AosApplication::ClearConnection() {}

PROTECTED VIRTUAL void AosApplication::ClearConnector()
{
    if (m_pConnector != IMS_NULL)
    {
        m_pConnector->SetListener(IMS_NULL);
        m_pConnector->CleanUp();

        delete m_pConnector;
        m_pConnector = IMS_NULL;
    }
}

PROTECTED VIRTUAL IMS_UINT32 AosApplication::GetReportState()
{
    IMS_UINT32 nReportState = APP_DISCONNECTED;

    switch (GetState())
    {
        case STATE_NOTREADY:  // FALL-THROUGH
        case STATE_READY:     // FALL-THROUGH
        case STATE_CONNECTING:
            break;

        case STATE_CONNECTED:
            nReportState = APP_CONNECTED;
            break;

        case STATE_UPDATING:
            nReportState = APP_UPDATING;
            break;

        case STATE_DISCONNECTING:
            nReportState = APP_DISCONNECTING;
            break;

        default:
            break;
    }

    return nReportState;
}

PROTECTED VIRTUAL IMS_SINT32 AosApplication::GetDataFailureReason() const
{
    return m_nDataFailureReason;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::OnMessage(IN IMSMSG& objMsg)
{
    A_IMS_TRACE_I(APPID, "OnMessage :: state (%s) , message (%s)",
            AosProvider::GetLog()->AppStateToString(GetState()),
            AosProvider::GetLog()->AppMessageToString(objMsg.nMSG), 0);

    if (!IsStateMessage(objMsg.nMSG))
    {
        return ProcessMessage(objMsg);
    }

    // StateMachine
    PreprocessStateMessage(objMsg);

    return OnStateMessage(objMsg);
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::ProcessMessage(IN IMSMSG& objMsg)
{
    IMS_BOOL bHandled = IMS_TRUE;

    switch (objMsg.nMSG)
    {
        case MSG_REG_START:
            ProcessRegStart(objMsg);
            break;

        case MSG_REG_UPDATE:
            ProcessRegUpdate(objMsg);
            break;

        case MSG_REG_STOP:
            ProcessRegStop(objMsg);
            break;

        case MSG_REG_RECONFIG:
            ProcessRegReconfig(objMsg);
            break;

        case MSG_REG_RECOVER:
            ProcessRegRecovery(objMsg);
            break;

        case MSG_IPCAN_CHANGED:
            ProcessIpcanChanged(objMsg);
            break;

        case MSG_PUB_TERMINATED:
            ProcessRegStopTimerExpired();
            break;

        case MSG_DESTROY:
            ProcessDestroy(objMsg);
            break;

        case MSG_IMS_EST_TIMER_CONTROL:
            ProcessImsEstablishmentControl(objMsg);
            break;

        case MSG_REG_EXCHANGE:
            ProcessRegExchange(objMsg);
            break;

        case MSG_AC_CONFIGURED:
            ProcessAutoConfigurationComplete(objMsg);
            break;

        case MSG_PCSCF_RECOVER:
            ProcessPcscfRecovery(objMsg);
            break;

        case MSG_SCSCF_RESTORATION:
            ProcessScscfRestoration(objMsg);
            break;

        case MSG_PLMN_BLOCK_WITH_TIMEOUT:
        {
            IMS_UINT32 nReason = LONG_TO_INT(objMsg.nWparam);
            AosReasonCode eAosReasonCode = AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT;
            if (nReason == AosReason::VOPS_NOT_SUPPORTED)
            {
                eAosReasonCode = AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT_BY_VOPS_NOT_SUPPORTED;
            }
            else if (nReason == AosReason::SSAC_BARRED)
            {
                eAosReasonCode = AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT_BY_SSAC_BARRED;
            }
            ProcessPlmnBlock(eAosReasonCode);
            break;
        }

        case MSG_RETRY_COUNT_INCREASE:
            ProcessRegRetryCount(objMsg);
            break;

        case MSG_OTHERS:
            ProcessOthers(objMsg);
            break;

        default:
            bHandled = IMS_FALSE;
            break;
    }

    return bHandled;
}

PROTECTED VIRTUAL void AosApplication::ProcessRegStart(IN IMSMSG& /* objMsg */)
{
    if (IsUpdateAvailable())
    {
        m_piRegistration->Start();
    }
    else
    {
        if (GetState() == STATE_NOTREADY)
        {
            m_pCondition->PrintBlockReasons();
        }

        Report_StateChanged(IMS_FALSE);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegUpdate(IN IMSMSG& objMsg)
{
    IMS_BOOL bIgnoreRetryTimer = (LONG_TO_INT(objMsg.nWparam) > 0);
    IMS_BOOL bExplicitUpdate = (LONG_TO_INT(objMsg.nLparam) > 0);

    if (IsUpdateAvailable())
    {
        m_piRegistration->Update(bIgnoreRetryTimer, bExplicitUpdate);
    }
    else
    {
        Report_StateChanged(IMS_FALSE);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegStop(IN IMSMSG& /* objMsg */)
{
    if (GetState() == STATE_DISCONNECTING)
    {
        if (IsTimerRunning(TIMER_REG_STOP) ||
                (m_piRegistration->GetState() == IAosRegistration::STATE_DEREGISTERING))
        {
            A_IMS_TRACE_I(APPID, "ProcessRegStop :: ignore due to processing", 0, 0, 0);
            return;
        }

        NotifyDeregistering();

        if (!IsPublished())
        {
            m_piRegistration->Stop();
        }
        else
        {
            StartTimer(TIMER_REG_STOP, REG_STOP_WAITING_TIME_MILLIS);
        }
    }
    else
    {
        CleanAll(GetOffReason());
        Report_StateChanged();
        ProcessStateStart(APP_START_WAITING_TIME_MILLIS);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegReconfig(IN IMSMSG& /* objMsg */)
{
    if (IsAllDetached())
    {
        ResetBlock(BLOCK_SERVICE_CONNECTING);
        m_pCondition->ResetBlock(BLOCK_ENABLER_DETACHED);
        return;
    }

    if (IsUpdateAvailable())
    {
        if (IsRegReconfigAvailable())
        {
            m_piRegistration->Reconfig();
        }
        else
        {
            Report_StateChanged(IMS_FALSE);
        }
    }

    ResetBlock(BLOCK_SERVICE_CONNECTING);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegRecovery(IN IMSMSG& objMsg)
{
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nWparam);
    A_IMS_TRACE_I(APPID, "ProcessRegRecovery :: reason (%d)", nReason, 0, 0);

    if (IsRegRecoveryHeld())
    {
        A_IMS_TRACE_I(APPID, "ProcessRegRecovery :: recover is held", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_REG_RECOVERY_HELD, m_nRegPending);
        m_nRecoverReason = nReason;
        return;
    }

    switch (GetState())
    {
        case STATE_CONNECTING:  // FALL-THROUGH
        case STATE_CONNECTED:   // FALL-THROUGH
        case STATE_UPDATING:
        {
            IMS_UINT32 nAosReason = AosReason::NONE;
            if (nReason == AosRegRecoveryType::PCSCF_CHANGE ||
                    nReason == AosRegRecoveryType::KEEP_DATA_CONNECTION)
            {
                nAosReason = AosReason::DATA_CONNECTION_MAINTAIN;
            }
            CleanAll(nAosReason);
            SetAppState(STATE_CONNECTING);
            m_piRegistration->Start();
            Report_StateChanged(IMS_FALSE);
            break;
        }
        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessIpcanChanged(IN IMSMSG& /* objMsg */)
{
    // In case handover is successful, wifi-connection will be newly connected later.
    if (!m_piContext->GetConnection()->IsEpdgEnabled())
    {
        ClearWifiRegBlock();
    }

    if (IsUpdateAvailable())
    {
        if (IsTimerRunning(TIMER_RECONFIG_GUARD))
        {
            A_IMS_TRACE_I(APPID, "ProcessIpcanChanged :: ipcan is held", 0, 0, 0);
            m_pUtil->RemoveFeature(PENDING_REG_UPDATE_HELD, m_nRegPending);
            m_pUtil->AddFeature(PENDING_IPCAN_HELD, m_nRegPending);
        }
        else
        {
            m_piRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);
        }
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessDestroy(IN IMSMSG& /* objMsg */)
{
    if (!IsNotReady())
    {
        CleanAll();
        Report_StateChanged();
        ProcessStateStart();
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessImsEstablishmentControl(IN IMSMSG& /* objMsg */)
{
    if (!IsRegTypeNormal())
    {
        return;
    }

    IMS_SINT32 nEstTime = GetImsEstablishmentTime() - 1;

    if (nEstTime <= 0)
    {
        return;
    }

    if (IsOn() || IsTimerRunning(TIMER_IMS_ESTABLISHMENT))
    {
        return;
    }

    if (!IsPlmnBlockRequired() || !m_piNetTracker->IsImsVoiceCallSupported())
    {
        return;
    }

    if (IsReasonBlockedForImsEstablishmentTimer())
    {
        return;
    }

    A_IMS_TRACE_D(APPID, "ProcessImsEstablishmentControl :: ims est time (%d sec)", nEstTime, 0, 0);

    StartTimer(TIMER_IMS_ESTABLISHMENT, nEstTime * 1000);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegExchange(IN IMSMSG& /* objMsg */) {}

PROTECTED VIRTUAL void AosApplication::ProcessAutoConfigurationComplete(IN IMSMSG& /* objMsg */) {}

PROTECTED VIRTUAL void AosApplication::ProcessPcscfRecovery(IN IMSMSG& objMsg)
{
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nWparam);
    A_IMS_TRACE_I(APPID, "ProcessPcscfRecovery :: reason (%d)", nReason, 0, 0);

    if (IsRegRecoveryHeld())
    {
        A_IMS_TRACE_I(APPID, "ProcessPcscfRecovery :: recover is held", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_REG_RECOVERY_HELD, m_nRegPending);
        m_nRecoverReason = nReason;
        return;
    }

    const AStringArray& objCurrPcscfs = m_piContext->GetPcscf()->GetPcscfs();
    IMS_SINT32 nNextAt = m_piContext->GetPcscf()->GetNextPcscfIndex();
    IMS_SINT32 nCount = objCurrPcscfs.GetCount();

    if (nNextAt >= 0 && nCount > 1)
    {
        AStringArray objUpdatePcscfs;

        for (IMS_SINT32 nAt = 0; nAt < nCount; ++nAt)
        {
            IMS_SINT32 nCurrAt = nNextAt + nAt;

            if ((nCurrAt) > (nCount - 1))
            {
                nCurrAt = nCurrAt - nCount;
            }

            objUpdatePcscfs.AddElement(objCurrPcscfs.GetElementAt(nCurrAt));
        }
        m_piContext->GetPcscf()->UpdatePcscfs(objUpdatePcscfs);
    }

    m_nRecoverReason = 0;
    m_piRegistration->Destroy();
    m_piRegistration->RequestCmd(IAosRegistration::CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED);
    PostMessage(MSG_REG_RECOVER, nReason, 0);
}

PROTECTED VIRTUAL void AosApplication::ProcessScscfRestoration(IN IMSMSG& objMsg)
{
    /* Abnormal network connection error between P-CSCF and S-CSCF has been detected.
     * The case is explained in 3GPP 24.229 5.1.2A.1.6.
     * It is sure that current P-CSCF is no longer available for any IMS services.
     * It will conduct a new registration regardless of RegRecoverHold scheme.
     */

    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nUnavailableTimeForCurrentPcscf = LONG_TO_INT(objMsg.nLparam);
    A_IMS_TRACE_I(APPID,
            "ProcessScscfRestoration :: reason (%d), nUnavailableTimeForCurrentPcscf (%d)", nReason,
            nUnavailableTimeForCurrentPcscf, 0);

    SetOffReason(AosReason::INITIAL_REG_REQUESTED);

    m_piRegistration->RequestCmd(
            IAosRegistration::CMD_SCSCF_RESTORATION, nUnavailableTimeForCurrentPcscf);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegRetryCount(IN IMSMSG& objMsg)
{
    IMS_BOOL bSupported =
            GET_N_CONFIG(m_nSlotId)->IsExtraRegErrRetryCntSharedForRegAndSubRequired();
    IMS_SINT32 nMaxCount = GET_N_CONFIG(m_nSlotId)->GetExtraRegErrMaxCount();

    if (!bSupported || nMaxCount <= 0)
    {
        return;
    }

    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nWparam);
    A_IMS_TRACE_I(APPID, "ProcessRegRetryCount :: reason (%d)", nReason, 0, 0);

    IMS_BOOL bIncreseRetryCnt =
            AosProvider::GetInstance()->GetRetryRepository(m_nSlotId)->IncreaseRetryCount(
                    (IsEmergency()) ? AosRetryRepository::TYPE_EMERGENCY
                                    : AosRetryRepository::TYPE_NORMAL);

    if (!bIncreseRetryCnt)
    {
        if (m_piContext->GetPcscf()->HasNextPcscf())
        {
            PostMessage(MSG_PCSCF_RECOVER, AosRegRecoveryType::PCSCF_CHANGE, 0);
        }
        else
        {
            ProcessPdnDisconnect();
        }
        return;
    }

    if (nReason == RETRY_COUNT_REG_RECOVER)
    {
        PostMessage(MSG_REG_RECOVER, 0, 0);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessOthers(IN IMSMSG& /* objMsg */) {}

PROTECTED VIRTUAL IMS_BOOL AosApplication::PreprocessStateMessage(IN IMSMSG& objMsg)
{
    if (objMsg.nMSG == MSG_CONNECTION)
    {
        PreprocessStateMessage_Connection(objMsg);
    }
    else if (objMsg.nMSG == MSG_CONDITION)
    {
        PreprocessStateMessage_Condition(objMsg);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::PreprocessStateMessage_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);

    if (nType == CONNECTION_DEACTIVATED &&
            !GET_N_CONFIG(m_nSlotId)->IsKeepRegRetryCntUponPdnReconnect())
    {
        m_piRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT);
    }

    if (!GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(APPID, "PreprocessStateMessage_Connection :: nType(%d), nReason(%d)", nType,
            nReason, 0);

    if (nType == CONNECTION_ACTIVATED ||
            (nType == CONNECTION_UPDATED && nReason == AosConnector::REASON_IPCAN_CAT_CHANGED))
    {
        IMS_BOOL bCurrEpdgEnabled = m_piContext->GetConnection()->IsEpdgEnabled();

        if (m_bEpdgEnabled != bCurrEpdgEnabled)
        {
            m_bEpdgEnabled = bCurrEpdgEnabled;
            UpdateMonitorNotify(IImsAosMonitor::TYPE_IPCAN,
                    (m_bEpdgEnabled) ? IIpcan::CATEGORY_WLAN : IIpcan::CATEGORY_MOBILE);
        }

        m_piRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_IPCAN);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::PreprocessStateMessage_Condition(IN IMSMSG& /* objMsg */)
{
    if (IsEmergency())
    {
        return IMS_TRUE;
    }

    if (GetState() == STATE_NOTREADY || GetState() == STATE_READY)
    {
        if (!m_pCondition->IsReady() && m_pCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
        {
            if (m_piContext->GetConnection()->IsActivationRequested() &&
                    m_piContext->GetConnection()->GetState() != IAosConnection::STATE_ACTIVE)
            {
                A_IMS_TRACE_D(APPID, "PreprocessStateMessage_Condition :: deactivate connection", 0,
                        0, 0);
                m_pConnector->Stop();
            }
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateNotReady_Condition(IN IMSMSG& /* objMsg */)
{
    if (IsConditionTimerSkippedDueToTimer())
    {
        return IMS_TRUE;
    }

    if (m_pCondition->IsReady())
    {
        SetAppState(STATE_READY);

        if (!IsEmergency())
        {
            m_pConnector->Start();
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateNotReady_Connection(IN IMSMSG& /* objMsg */)
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateReady_Condition(IN IMSMSG& /* objMsg */)
{
    if (!m_pCondition->IsReady())
    {
        SetAppState(STATE_NOTREADY);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateReady_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    switch (nType)
    {
        case CONNECTION_ACTIVATED:
            SetAppState(STATE_CONNECTING);
            m_piRegistration->Start();
            break;

        case CONNECTION_DEACTIVATED:
            if (!IsEmergency())
            {
                if (nReason == AosConnector::REASON_PERMANENTLY_FAILED)
                {
                    ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK);

                    m_pCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
                    m_pConnector->Stop(PLMN_BLOCK_PDN_STOP_WAITING_TIME_SECONDS);
                }
                else if (nReason == AosConnector::REASON_PCSCF_DISCOVERY_FAILED)
                {
                    m_pCondition->SetBlock(BLOCK_INVALID_CONNECTION);
                    m_pConnector->Stop();
                }
                else
                {
                    m_pConnector->Start();
                }
            }
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateConnecting_Condition(IN IMSMSG& /* objMsg */)
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateConnecting_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(APPID, "StateConnecting_Connection :: nType(%d), nReason(%d)", nType, nReason, 0);

    switch (nType)
    {
        case CONNECTION_DEACTIVATED:
            ProcessConnectionDeactivated(nReason);
            break;

        case CONNECTION_UPDATED:
            ProcessConnectionUpdated(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateConnecting_Registration(IN IMSMSG& objMsg)
{
    IMS_UINT32 nResult = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(
            APPID, "StateConnecting_Registration :: nResult(%d), nReason(%d)", nResult, nReason, 0);

    switch (nResult)
    {
        case IAosRegistration::RESULT_SUCCESS:
            ProcessRegSucceeded(nReason);
            break;

        case IAosRegistration::RESULT_TRYING:
            ProcessRegTrying_StateConnecting(nReason);
            break;

        case IAosRegistration::RESULT_FAILURE:
            ProcessRegFailed_StateConnecting(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateConnected_Condition(IN IMSMSG& /* objMsg */)
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateConnected_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(APPID, "StateConnected_Connection :: nType(%d), nReason(%d)", nType, nReason, 0);

    switch (nType)
    {
        case CONNECTION_DEACTIVATED:
            ProcessConnectionDeactivated(nReason);
            break;

        case CONNECTION_UPDATED:
            ProcessConnectionUpdated(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateConnected_Registration(IN IMSMSG& objMsg)
{
    IMS_UINT32 nResult = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(
            APPID, "StateConnected_Registration :: nResult(%d), nReason(%d)", nResult, nReason, 0);

    switch (nResult)
    {
        case IAosRegistration::RESULT_TRYING:
            ProcessRegTrying_StateConnected(nReason);
            break;

        case IAosRegistration::RESULT_FAILURE:
            ProcessRegFailed_StateConnected(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateUpdating_Condition(IN IMSMSG& /* objMsg */)
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateUpdating_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(APPID, "StateUpdating_Connection :: nType(%d), nReason(%d)", nType, nReason, 0);

    switch (nType)
    {
        case CONNECTION_DEACTIVATED:
            ProcessConnectionDeactivated(nReason);
            break;

        case CONNECTION_UPDATED:
            ProcessConnectionUpdated(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateUpdating_Registration(IN IMSMSG& objMsg)
{
    IMS_UINT32 nResult = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(
            APPID, "StateUpdating_Registration :: nResult(%d), nReason(%d)", nResult, nReason, 0);

    switch (nResult)
    {
        case IAosRegistration::RESULT_SUCCESS:
            ProcessRegSucceeded(nReason);
            break;

        case IAosRegistration::RESULT_TRYING:
            ProcessRegTrying_StateUpdating(nReason);
            break;

        case IAosRegistration::RESULT_FAILURE:
            ProcessRegFailed_StateUpdating(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateDisconnecting_Condition(IN IMSMSG& /* objMsg */)
{
    if (!m_pCondition->IsReady())
    {
        // DE-REG is keeping
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateDisconnecting_Connection(IN IMSMSG& objMsg)
{
    IMS_UINT32 nType = LONG_TO_INT(objMsg.nWparam);
    IMS_UINT32 nReason = LONG_TO_INT(objMsg.nLparam);

    A_IMS_TRACE_I(
            APPID, "StateDisconnecting_Connection :: nType(%d), nReason(%d)", nType, nReason, 0);

    switch (nType)
    {
        case CONNECTION_DEACTIVATED:
            ProcessConnectionDeactivated(nReason);
            break;

        case CONNECTION_UPDATED:
            ProcessConnectionUpdated_StateDisconnecting(nReason);
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::StateDisconnecting_Registration(IN IMSMSG& objMsg)
{
    IMS_UINT32 nResult = LONG_TO_INT(objMsg.nWparam);

    if (nResult == IAosRegistration::RESULT_TRYING)
    {
        return IMS_TRUE;
    }

    CleanAll(GetOffReason());
    Report_StateChanged();

    ProcessStateStart(APP_START_WAITING_TIME_MILLIS);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosApplication::ProcessRegTrying_StateConnecting(IN IMS_UINT32 /* nReason */)
{
    ProcessImsEstablishmentStart();
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_StateConnecting(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case IAosRegistration::REASON_FAILURE_FORBIDDEN:  // FALL-THROUGH
        case IAosRegistration::REASON_FAILURE_AUTHENTICATION:
            ProcessRegAuthenticationFailed();
            break;

        case IAosRegistration::REASON_FAILURE_USIM_AUTHENTICATION:
            ProcessRegUsimAuthenticationFailed();
            break;

        case IAosRegistration::REASON_FAILURE_TERMINATED:
            ProcessRegTerminated();
            break;

        case IAosRegistration::REASON_FAILURE_INTERNAL:
            ProcessRegInternalFailed();
            break;

        case IAosRegistration::REASON_FAILURE_PDN_RECONNECT:
            ProcessPdnDisconnect();
            break;

        case IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT:
            ProcessPdnBlockWithTime();
            break;

        case IAosRegistration::REASON_FAILURE_BANNDED:
            ProcessPdnBlock();
            break;

        case IAosRegistration::REASON_FAILURE_PCO_LIMITED_SERVICE:
            ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
            break;

        case IAosRegistration::REASON_FAILURE_PLMN_BLOCK_WITH_TIMEOUT:
            ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
            ProcessRegFailed_Start(nReason);
            break;

        default:
            ProcessRegFailed_Start(nReason);
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegTrying_StateConnected(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case IAosRegistration::REASON_TRYING_START:
            ProcessImsEstablishmentStart();
            SetAppState(STATE_CONNECTING);
            break;

        case IAosRegistration::REASON_TRYING_UPDATE:
            SetAppState(STATE_UPDATING);
            break;

        case IAosRegistration::REASON_TRYING_STOP:
            SetAppState(STATE_DISCONNECTING);
            break;

        default:
            break;
    }

    Report_StateChanged();
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_StateConnected(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case IAosRegistration::REASON_FAILURE_TERMINATED:
            ProcessRegFailed_Terminated();
            break;

        case IAosRegistration::REASON_FAILURE_INTERNAL:
            ProcessRegInternalFailed();
            break;

        case IAosRegistration::REASON_FAILURE_PDN_RECONNECT:
            ProcessPdnDisconnect();
            break;

        case IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT:
            ProcessPdnBlockWithTime();
            break;

        case IAosRegistration::REASON_FAILURE_FORBIDDEN_IN_WIFI:
            ProcessRegForbiddenInWifi();
            break;
        case IAosRegistration::REASON_FAILURE_NO_PCSCF_AVAILABLE:
            ProcessRegFailed_NoNextPcscfOnScscfRestoration();
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegTrying_StateUpdating(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case IAosRegistration::REASON_TRYING_START:
            ProcessImsEstablishmentStart();
            SetAppState(STATE_CONNECTING);
            break;

        case IAosRegistration::REASON_TRYING_STOP:
            SetAppState(STATE_DISCONNECTING);
            break;

        default:
            break;
    }

    Report_StateChanged();
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_StateUpdating(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case IAosRegistration::REASON_FAILURE_FORBIDDEN:  // FALL-THROUGH
        case IAosRegistration::REASON_FAILURE_AUTHENTICATION:
            ProcessRegAuthenticationFailed();
            break;

        case IAosRegistration::REASON_FAILURE_USIM_AUTHENTICATION:
            ProcessRegUsimAuthenticationFailed();
            break;

        case IAosRegistration::REASON_FAILURE_TERMINATED:
            ProcessRegFailed_Terminated();
            break;

        case IAosRegistration::REASON_FAILURE_INTERNAL:
            ProcessRegInternalFailed();
            break;

        case IAosRegistration::REASON_FAILURE_REG_TERMINATING:
            ProcessRegTerminating();
            break;

        case IAosRegistration::REASON_FAILURE_PDN_RECONNECT:
            ProcessPdnDisconnect();
            break;

        case IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT:
            ProcessPdnBlockWithTime();
            break;

        case IAosRegistration::REASON_FAILURE_BANNDED:
            ProcessPdnBlock();
            break;

        case IAosRegistration::REASON_FAILURE_FORBIDDEN_IN_WIFI:
            ProcessRegForbiddenInWifi();
            break;

        default:
            ProcessRegFailed_Update(nReason);
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessConnectionUpdated_StateDisconnecting(
        IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case AosConnector::REASON_IP_CHANGED:
            CleanAll();
            Report_StateChanged();
            ProcessStateStart(APP_START_WAITING_TIME_MILLIS);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessConnectionDeactivated(IN IMS_UINT32 nReason)
{
    if (nReason == AosConnector::REASON_PERMANENTLY_FAILED)
    {
        m_pCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED, IMS_FALSE);
        CleanAll(AosReason::DATA_PERMANENTLY_FAILED);
    }
    else if (nReason == AosConnector::REASON_IP_CHANGED)
    {
        CleanAll(AosReason::IP_CHANGED);
    }
    else
    {
        IMS_UINT32 nOffReason = GetOffReason();
        if (nOffReason == AosReason::WIFI_OFF || nOffReason == AosReason::AIRPLANE_MODE)
        {
            CleanAll(nOffReason);
        }
        else
        {
            CleanAll(AosReason::DATA_DISCONNECTED);
        }
    }

    Report_StateChanged(IMS_FALSE);
    ProcessStateStart();
}

PROTECTED VIRTUAL void AosApplication::ProcessConnectionUpdated(IN IMS_UINT32 nReason)
{
    switch (GetState())
    {
        case STATE_CONNECTING:  // FALL-THROUGH
        case STATE_CONNECTED:   // FALL-THROUGH
        case STATE_UPDATING:
            break;

        default:
            return;
    }

    switch (nReason)
    {
        case AosConnector::REASON_IP_CHANGED:
            CleanAll(AosReason::IP_CHANGED);
            Report_StateChanged(IMS_FALSE);
            ProcessStateStart();
            break;

        case AosConnector::REASON_PCSCF_CHANGED:
            ProcessConnectionUpdated_Pcscf();
            break;

        case AosConnector::REASON_IPCAN_CAT_CHANGED:
            PostMessage(MSG_IPCAN_CHANGED, 0, 0);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessConnectionUpdated_Pcscf()
{
    IMS_UINT32 nPcscfChangedType = m_piContext->GetPcscf()->GetChangedType();

    A_IMS_TRACE_I(
            APPID, "ProcessConnectionUpdated_Pcscf :: change type(%d)", nPcscfChangedType, 0, 0);

    switch (nPcscfChangedType)
    {
        case IAosPcscf::TYPE_CHANGED_SAME:
            break;
        case IAosPcscf::TYPE_CHANGED_REORDER:
            if (GET_N_CONFIG(m_nSlotId)->GetRegistrationPcscfUpdatePolicy() ==
                    CarrierConfig::Ims::REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME)
            {
                A_IMS_TRACE_I(
                        APPID, "ProcessConnectionUpdated_Pcscf :: reg will be updated", 0, 0, 0);
                PostMessage(MSG_REG_UPDATE, 0, 0);
            }
            break;

        case IAosPcscf::TYPE_CHANGED_DIFFERENT:
            PostMessage(MSG_REG_RECOVER, AosRegRecoveryType::KEEP_DATA_CONNECTION, 0);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegSucceeded(IN IMS_UINT32 /* nReason */)
{
    StopTimer(TIMER_IMS_ESTABLISHMENT);
    ClearOffReason();
    ClearDataFailureReason();
    SetAppState(STATE_CONNECTED);
    UpdateRegisteredRat(m_piContext->GetNetTracker()->GetNetworkType());
    PerformRatBlockActions(IMS_FALSE);
    if (!m_piContext->GetConnection()->IsEpdgEnabled())
    {
        ClearWifiRegBlock();
    }
    Report_StateChanged();

    UpdateConnectedServices(IMS_FALSE);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_Start(IN IMS_UINT32 /* nReason */)
{
    SetOffReason(AosReason::REG_FAILURE);
    Report_StateChanged(IMS_FALSE);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_Update(IN IMS_UINT32 /* nReason */)
{
    if (GET_N_CONFIG(m_nSlotId)->IsCdmalessFeatureTagRequired() && IsImsCall())
    {
        A_IMS_TRACE_I(
                APPID, "ProcessRegFailed_Update :: skip re-reg failuire during call", 0, 0, 0);
        return;
    }

    if (m_piRegistration->IsRefreshing())
    {
        SetOffReason(AosReason::REG_FAILURE);
        Report_StateChanged(IMS_FALSE);
    }
    else
    {
        SetAppState(STATE_CONNECTING);
        SetOffReason(AosReason::REG_FAILURE);
        Report_StateChanged();
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_Terminated()
{
    if (!IsRegRecoveryHeld())
    {
        ProcessRegTerminated();
    }
    else
    {
        SetOffReason(AosReason::REG_TERMINATED);
        SetAppState(STATE_CONNECTING);
        Report_StateChanged();
        m_pUtil->AddFeature(PENDING_REG_RECOVERY_HELD, m_nRegPending);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegFailed_NoNextPcscfOnScscfRestoration()
{
    if (GetState() == STATE_CONNECTED)
    {
        SetOffReason(AosReason::REG_ALL_PCSCF_FAILED);
        SetAppState(STATE_CONNECTING);
        Report_StateChanged();
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessDisconnectingState(IN IMS_UINT32 nReason /* = 0 */)
{
    if (m_piRegistration->IsRegistered())
    {
        SetOffReason(nReason);
        SetAppState(STATE_DISCONNECTING);
        Report_StateChanged();
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessNetworkEvent(
        IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx)
{
    A_IMS_TRACE_I(APPID, "ProcessNetworkEvent :: type(%d), state(%d)", nType, nState, 0);

    if (nType == IMS_EVENT_LTE_INFO)
    {
        if (m_nLteAttachState != nState || m_nLteExtraInfo != nStateEx)
        {
            m_nLteAttachState = nState;
            m_nLteExtraInfo = nStateEx;
            m_piRegistration->RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY,
                    (m_nLteAttachState == IMS_LTE_INFO_EPS_ONLY_ATTACHED ||
                            (m_nLteAttachState == IMS_LTE_INFO_COMBINED_ATTACHED &&
                                    m_nLteExtraInfo != IMS_LTE_INFO_EXTRA_NONE))
                            ? IAosRegistration::REASON_SET_ENABLE
                            : IAosRegistration::REASON_SET_DISABLE);
        }

        return;
    }

    if (nType == IMS_EVENT_VOICE_SERVICE_STATE)
    {
        m_nVoiceServiceState = nState;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessStateStart(IN IMS_UINT32 nTime /* = 0 */)
{
    if (nTime == 0)
    {
        PostMessage(MSG_CONDITION, 0, 0);
        return;
    }

    StartTimer(TIMER_MSG_CONDITION, nTime);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegControlEvent(
        IN IMS_UINT32 nType, IN IMS_UINT32 nReason)
{
    switch (nType)
    {
        case IMS_REG_CONTROL_RECOVER:
            if (nReason == IMS_REG_CONTROL_KEEP_DATA_CONNECTION)
            {
                PostMessage(MSG_REG_RECOVER, AosRegRecoveryType::KEEP_DATA_CONNECTION, 0);
            }
            else
            {
                PostMessage(MSG_REG_RECOVER, 0, 0);
            }
            break;

        case IMS_REG_CONTROL_UPDATE:
            PostMessage(MSG_REG_UPDATE, 1, 0);
            break;

        case IMS_REG_CONTROL_DESTROY:
            PostMessage(MSG_DESTROY, nReason, 0);
            break;

        case IMS_REG_CONTROL_STOP:
            ProcessDisconnectingState();
            PostMessage(MSG_REG_STOP, 0, 0);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegInternalFailed(IN IMS_UINT32 nReason /* = 0 */)
{
    if (nReason > 0)
    {
        CleanAll(nReason);
    }
    else
    {
        CleanAll(AosReason::REG_FAILURE);
    }

    Report_StateChanged(IMS_FALSE);

    ProcessStateStart(UNEXPECTED_ERROR_APP_START_WAITING_TIME_MILLIS);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegAuthenticationFailed()
{
    m_pCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);

    CleanAll(AosReason::IMS_DISABLED);
    Report_StateChanged(IMS_FALSE);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegUsimAuthenticationFailed()
{
    m_pCondition->SetBlock(BLOCK_USIM_AUTHENTICATION_FAILED);

    CleanAll(AosReason::IMS_DISABLED);
    Report_StateChanged(IMS_FALSE);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegForbiddenInWifi()
{
    if (!IsRegTypeNormal() ||
            GET_N_CONFIG(m_nSlotId)->GetSubConsecutiveRetryCntForRegForbiddenInWifi() <= 0)
    {
        return;
    }

    A_IMS_TRACE_I(APPID, "ProcessRegForbiddenInWifi", 0, 0, 0);
    m_pCondition->SetBlock(BLOCK_WIFI_REG_FORBIDDEN);

    CleanAll(AosReason::REG_TERMINATED);
    Report_StateChanged(IMS_FALSE);

    ProcessStateStart(UNEXPECTED_ERROR_APP_START_WAITING_TIME_MILLIS);
}

PROTECTED VIRTUAL void AosApplication::ProcessRegTerminated()
{
    CleanAll(AosReason::REG_TERMINATED);
    Report_StateChanged(IMS_FALSE);

    ProcessStateStart(UNEXPECTED_ERROR_APP_START_WAITING_TIME_MILLIS);
}

PROTECTED VIRTUAL void AosApplication::ProcessPingCommand()
{
    // Do nothing on AOS BASE
}

PROTECTED VIRTUAL void AosApplication::ProcessRegTerminating()
{
    A_IMS_TRACE_I(APPID, "ProcessRegTerminating ::", 0, 0, 0);
    if (IsImsCall())
    {
        SetOffReason(AosReason::REG_TERMINATING);
        SetAppState(STATE_CONNECTING);
        Report_StateChanged();

        PostMessage(MSG_REG_RECOVER, 0, 0);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessPdnDisconnect()
{
    if (m_pCondition->IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED) ||
            m_nOffReason == AosReason::POWER_OFF)
    {
        m_pConnector->Stop();
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsCallEndAndPdnReactivationByRegTerminated())
    {
        if (IsImsCall())
        {
            A_IMS_TRACE_I(APPID, "ProcessPdnDisconnect :: IMS call is not IDLE", 0, 0, 0);
            m_pConnector->Stop(DELAY_STOPPING_PDN_TO_KEEP_SESSION_TIME_SECONDS);
            SetOffReason(AosReason::REG_TERMINATING);
            SetAppState(STATE_NOTREADY);
            Report_StateChanged();
            return;
        }
    }

    if (GetOffReason() == AosReason::DATA_PERMANENTLY_FAILED)
    {
        A_IMS_TRACE_I(APPID, "ProcessPdnDisconnect :: Data permanently failed", 0, 0, 0);
        ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK);
        m_pConnector->Stop(PLMN_BLOCK_PDN_STOP_WAITING_TIME_SECONDS);
        return;
    }

    IMS_UINT32 nFinalErr = GET_N_CONFIG(m_nSlotId)->GetExtraRegErrFinalType();
    A_IMS_TRACE_I(APPID, "ProcessPdnDisconnect :: reg err final type - %d", nFinalErr, 0, 0);

    IMS_BOOL bNotifyPlmnBlock = IMS_FALSE;

    switch (nFinalErr)
    {
        case CarrierConfig::Ims::ERROR_TYPE_REPEATED:
            bNotifyPlmnBlock = IMS_TRUE;
            break;

        case CarrierConfig::Ims::ERROR_TYPE_CRITICAL:
            if (GET_N_CONFIG(m_nSlotId)->IsTestModeEnabled(CarrierConfig::Ims::
                                TEST_MODE_PERMANENT_FAILURE_WITHOUT_IMS_PDN_DEACTIVATION))
            {
                A_IMS_TRACE_I(
                        APPID, "ProcessPdnDisconnect :: IMS PDN is not disconnected", 0, 0, 0);
                return;
            }
            break;

        case CarrierConfig::Ims::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK:
            if (m_nRat == NW_REPORT_RADIO_NR)
            {
                bNotifyPlmnBlock = IMS_TRUE;
            }
            else if (m_nRat == NW_REPORT_RADIO_LTE)
            {
                if (m_nLteAttachState != IMS_LTE_INFO_COMBINED_ATTACHED ||
                        m_nLteExtraInfo != IMS_LTE_INFO_EXTRA_NONE)
                {
                    bNotifyPlmnBlock = IMS_TRUE;
                }
            }
            break;

        case CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK:
            PerformRatBlockActions(IMS_TRUE);
            NotifyDeregistered(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
            return;

        default:
            break;
    }

    IMS_BOOL bIsNrBlockCondition =
            GET_N_CONFIG(m_nSlotId)->GetRegTempPlmnBlockRatsOnAllPcscfsFail().Contains(
                    CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN) &&
            m_nRat == NW_REPORT_RADIO_NR;
    if (bIsNrBlockCondition)
    {
        bNotifyPlmnBlock = IMS_TRUE;
    }

    if (bNotifyPlmnBlock)
    {
        NotifyDeregistered(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
        m_pConnector->Stop(GET_N_CONFIG(m_nSlotId)->GetReleasePdnDelaySecAfterTempPlmnBlock());
    }
    else
    {
        m_pConnector->Stop();
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRoamingState(IN IMS_BOOL bRoaming)
{
    if (m_bDataRoaming != bRoaming)
    {
        A_IMS_TRACE_I(APPID, "ProcessRoamingState :: data roaming (%s)", _TRACE_B_(bRoaming), 0, 0);
        m_bDataRoaming = bRoaming;

        if (IsTimerRunning(TIMER_IMS_ESTABLISHMENT))
        {
            IMS_SINT32 nEstTime = GET_N_CONFIG(m_nSlotId)->GetImsEstablishmentTimeForLte();

            if (nEstTime <= 0)
            {
                return;
            }

            StartTimer(TIMER_IMS_ESTABLISHMENT, nEstTime * 1000);
        }
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessAppActivatedTimerExpired()
{
    StopTimer(TIMER_APP_ACTIVATED);

    CleanAll();
    ProcessStateStart(APP_START_WAITING_TIME_MILLIS);
}

PROTECTED VIRTUAL void AosApplication::ProcessAppConnectedTimerExpired()
{
    StopTimer(TIMER_APP_CONNECTED);
}

PROTECTED VIRTUAL void AosApplication::ProcessAppTerminatedTimerExpired()
{
    StopTimer(TIMER_APP_TERMINATED);
}

PROTECTED VIRTUAL void AosApplication::ProcessReconfigTimerExpired()
{
    StopTimer(TIMER_RECONFIG_GUARD);

    if (IsAllHandleDetached())
    {
        if (IsAllDetached())
        {
            ResetBlock(BLOCK_SERVICE_CONNECTING);
            return;
        }

        if (m_piRegistration->IsRegistered())
        {
            SetOffReason(AosReason::SERVICE_POLICY);
            SetAppState(STATE_DISCONNECTING);
            Report_StateChanged();
            PostMessage(MSG_REG_STOP, 0, 0);
            m_pCondition->SetBlock(BLOCK_ENABLER_DETACHED);
        }
        else
        {
            m_pCondition->SetBlock(BLOCK_ENABLER_DETACHED);

            CleanAll(AosReason::SERVICE_POLICY);
            Report_StateChanged();
        }

        ResetBlock(BLOCK_SERVICE_CONNECTING);
        return;
    }

    if (IsNotReady() || IsAllDetached())
    {
        ResetBlock(BLOCK_SERVICE_CONNECTING);
        ResetBlock(BLOCK_ENABLER_DETACHED);
        return;
    }

    if (m_pUtil->IsFeatureOn(PENDING_REG_RECOVERY_HELD, m_nRegPending))
    {
        A_IMS_TRACE_I(APPID,
                "ProcessReconfigTimerExpired :: reconfig is ignored because of reg recovery", 0, 0,
                0);
        return;
    }

    if (IsReconfigHandleChanged())
    {
        m_pUtil->RemoveFeature(PENDING_IPCAN_HELD, m_nRegPending);
        m_pUtil->RemoveFeature(PENDING_REG_UPDATE_HELD, m_nRegPending);
        PostMessage(MSG_REG_RECONFIG, 0, 0);
    }
    else
    {
        if (IsOn())
        {
            A_IMS_TRACE_I(APPID,
                    "ProcessReconfigTimerExpired :: reconfig is same but report state to handle", 0,
                    0, 0);
            Report_StateChanged(IMS_FALSE);
        }

        if (m_pUtil->IsFeatureOn(PENDING_IPCAN_HELD, m_nRegPending))
        {
            m_pUtil->RemoveFeature(PENDING_IPCAN_HELD, m_nRegPending);
            PostMessage(MSG_IPCAN_CHANGED, 0, 0);
        }

        if (m_pUtil->IsFeatureOn(PENDING_REG_UPDATE_HELD, m_nRegPending))
        {
            m_pUtil->RemoveFeature(PENDING_REG_UPDATE_HELD, m_nRegPending);
            PostMessage(MSG_REG_UPDATE, 0, 0);
        }
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegBlockedTimerExpired()
{
    StopTimer(TIMER_REG_BLOCKED);

    if ((GetState() == STATE_CONNECTING) &&
            (m_piRegistration->GetState() == IAosRegistration::STATE_OFFLINE))
    {
        m_piRegistration->Start();
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessRegStopTimerExpired()
{
    StopTimer(TIMER_REG_STOP);

    if (GetState() == STATE_DISCONNECTING)
    {
        m_piRegistration->Stop();
    }
    else
    {
        CleanAll();
        Report_StateChanged();
        ProcessStateStart(APP_START_WAITING_TIME_MILLIS);
    }
}

PROTECTED VIRTUAL void AosApplication::ProcessPdnBlockedTimerExpired()
{
    StopTimer(TIMER_PDN_BLOCKED);
    m_pCondition->ResetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
}

PROTECTED VIRTUAL void AosApplication::ProcessImsEstablishmentTimerExpired()
{
    StopTimer(TIMER_IMS_ESTABLISHMENT);

    if (IsImsCall() || !IsPlmnBlockRequired())
    {
        return;
    }

    if (m_pCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED) ||
            m_pCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED))
    {
        return;
    }

    AString strNa;
    IMS_UINT32 nTrafficPriorityBlock;
    m_piRegistration->GetProperty(
            IAosRegistration::PROPERTY_TRAFFIC_PRIORITY_BLOCK, nTrafficPriorityBlock, strNa);
    if (nTrafficPriorityBlock == AosProperty::AOS_TRUE)
    {
        A_IMS_TRACE_I(APPID, "ProcessImsEstablishmentTimerExpired :: traffic is blocked", 0, 0, 0);
        return;
    }

    A_IMS_TRACE_I(
            APPID, "ProcessImsEstablishmentTimerExpired :: PLMN is blocked with timeout", 0, 0, 0);
    NotifyDeregistered(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);

    IMS_UINT32 nFailureCount;
    m_piRegistration->GetProperty(
            IAosRegistration::PROPERTY_REG_FAILURE_COUNT, nFailureCount, strNa);

    if (m_piContext->GetConnection()->GetState() != IAosConnection::STATE_ACTIVE)
    {
        A_IMS_TRACE_I(APPID, "ims est timer :: not to stop ims pdn", 0, 0, 0);
        return;
    }

    if (m_piRegistration->IsRegistered() || nFailureCount > 0)
    {
        A_IMS_TRACE_I(APPID, "ims est timer :: not to stop ims pdn - (%d)", nFailureCount, 0, 0);
        if (GET_N_CONFIG(m_nSlotId)->IsUpdateOngoingRegRetryTimerOnImsEstTimerExpiry())
        {
            m_piRegistration->RequestCmd(
                    IAosRegistration::CMD_UPDATE_STOP_RETRY_TIMER_WITH_DEFAULT);
        }
        return;
    }

    m_pConnector->Stop(GET_N_CONFIG(m_nSlotId)->GetReleasePdnDelaySecAfterTempPlmnBlock());
}

PROTECTED VIRTUAL void AosApplication::ProcessRatBlockTimerExpired()
{
    /*
     * (b/379769225) Do not clear RAT blocks upon TIMER_RAT_BLOCK expiry
     * This logic is left in place for potential future use.
     *
     * Original code:
     *   NotifyDeregistered(AosReasonCode::CLEAR_RAT_BLOCKS);
     */

    m_pConnector->Stop();
    PerformRatBlockActions(IMS_FALSE);
}

PROTECTED VIRTUAL void AosApplication::ProcessPdnBlock() {}

PROTECTED VIRTUAL void AosApplication::ProcessPdnBlockWithTime()
{
    IMS_UINT32 nWaitTime = 0;
    AString strNa;
    m_piRegistration->GetProperty(
            IAosRegistration::PROPERTY_PDN_REACIVATE_WAIT_TIME, nWaitTime, strNa);

    if (nWaitTime > 0)
    {
        StartTimer(TIMER_PDN_BLOCKED, nWaitTime * 1000);
        m_pCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED, IMS_FALSE);
    }

    m_pConnector->Stop();
}

PROTECTED VIRTUAL void AosApplication::ProcessImsEstablishmentStart()
{
    if (!IsRegTypeNormal())
    {
        return;
    }

    IMS_SINT32 nEstTime = GetImsEstablishmentTime();

    if (nEstTime <= 0 || IsImsEstablishmentTimerStopRequired())
    {
        StopTimer(TIMER_IMS_ESTABLISHMENT);
        return;
    }

    if (IsTimerRunning(TIMER_IMS_ESTABLISHMENT))
    {
        if (m_nRat == m_piNetTracker->GetMobileNetworkType() ||
                !m_pUtil->IsSupportedNetworkTypeForCellular(m_nRat))
        {
            return;
        }
    }

    A_IMS_TRACE_D(APPID, "ProcessImsEstablishmentStart :: ims est time (%d sec)", nEstTime, 0, 0);

    StartTimer(TIMER_IMS_ESTABLISHMENT, nEstTime * 1000);
}

PROTECTED VIRTUAL void AosApplication::ProcessPlmnBlock(IN AosReasonCode eReason)
{
    if (!IsRegTypeNormal() || m_piContext->GetConnection()->IsEpdgEnabled())
    {
        return;
    }

    if (!IsPlmnBlockRequired())
    {
        return;
    }

    StopTimer(TIMER_IMS_ESTABLISHMENT);

    A_IMS_TRACE_D(APPID,
            "ProcessPlmnBlock :: Stop Ims Estb. Timer due to plmn block request with reason = %d",
            eReason, 0, 0);

    NotifyDeregistered(eReason);
}

PROTECTED VIRTUAL void AosApplication::Report_StateChanged(
        IN IMS_BOOL bIsStateChecked /* = IMS_TRUE */)
{
    IMS_UINT32 nReportingState = GetReportState();

    if (bIsStateChecked)
    {
        if (m_nReportState == nReportingState)
        {
            return;
        }
    }

    m_nReportState = nReportingState;

    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 nAt = 0; nAt < objHandles.GetSize(); nAt++)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);
        piHandle->App_StateChanged(m_nReportState, GetOffReason());
    }

    Report_Notify();

    if (m_nReportState != APP_DISCONNECTING)
    {
        UpdateRegState();
    }
}

PROTECTED VIRTUAL void AosApplication::Report_Notify()
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 nAt = 0; nAt < objHandles.GetSize(); nAt++)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);
        piHandle->App_Notify();
    }
}

PROTECTED VIRTUAL void AosApplication::Report_Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 nAt = 0; nAt < objHandles.GetSize(); nAt++)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);
        piHandle->Request(nType, nState);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::UpdateRegRecoveryHeld()
{
    IMS_BOOL bCurrHeld = IsImsCall();

    if (IsRegRecoveryHeld() != bCurrHeld)
    {
        SetRegRecoveryHeld(bCurrHeld);

        if (m_pUtil->IsFeatureOn(PENDING_REG_RECOVERY_HELD, m_nRegPending))
        {
            if (!bCurrHeld)
            {
                A_IMS_TRACE_I(APPID, "UpdateRegRecoveryHeld :: trigger reg recovery", 0, 0, 0);
                m_pUtil->RemoveFeature(PENDING_REG_RECOVERY_HELD, m_nRegPending);

                if (m_nRecoverReason == AosRegRecoveryType::PCSCF_CHANGE)
                {
                    PostMessage(MSG_PCSCF_RECOVER, m_nRecoverReason, 0);
                }
                else
                {
                    PostMessage(MSG_REG_RECOVER, m_nRecoverReason, 0);
                }
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosApplication::UpdateRegStopHeld()
{
    if (!IsImsCall())
    {
        if (m_pUtil->IsFeatureOn(PENDING_REG_STOP_HELD, m_nRegPending))
        {
            A_IMS_TRACE_I(APPID, "UpdateRegStopHeld :: trigger reg stop", 0, 0, 0);
            m_pUtil->RemoveFeature(PENDING_REG_STOP_HELD, m_nRegPending);

            ProcessDisconnectingState();
            PostMessage(MSG_REG_STOP, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosApplication::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    if (nDuration == 0)
    {
        return;
    }

    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_RECONFIG_GUARD:
            ppiTimer = &m_piReconfigTimer;
            break;

        case TIMER_MSG_CONDITION:
            ppiTimer = &m_piMsgConditionTimer;
            break;

        case TIMER_REG_STOP:
            ppiTimer = &m_piRegStopTimer;
            break;

        case TIMER_REG_BLOCKED:
            ppiTimer = &m_piRegBlockedTimer;
            break;

        case TIMER_APP_CONNECTED:
            ppiTimer = &m_piAppConnectedTimer;
            break;

        case TIMER_APP_ACTIVATED:
            ppiTimer = &m_piAppActivatedTimer;
            break;

        case TIMER_APP_TERMINATED:
            ppiTimer = &m_piAppTerminatedTimer;
            break;

        case TIMER_PDN_BLOCKED:
            ppiTimer = &m_piPdnBlockedTimer;
            break;

        case TIMER_IMS_ESTABLISHMENT:
            ppiTimer = &m_piImsEstablishmentTimer;
            break;

        case TIMER_RAT_BLOCK:
            ppiTimer = &m_piRatBlockTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer =
            m_pUtil->StartTimer(nDuration, this, AosProvider::GetLog()->AppTimerToString(nType));
}

PROTECTED VIRTUAL void AosApplication::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_RECONFIG_GUARD:
            ppiTimer = &m_piReconfigTimer;
            break;

        case TIMER_MSG_CONDITION:
            ppiTimer = &m_piMsgConditionTimer;
            break;

        case TIMER_REG_STOP:
            ppiTimer = &m_piRegStopTimer;
            break;

        case TIMER_REG_BLOCKED:
            ppiTimer = &m_piRegBlockedTimer;
            break;

        case TIMER_APP_CONNECTED:
            ppiTimer = &m_piAppConnectedTimer;
            break;

        case TIMER_APP_ACTIVATED:
            ppiTimer = &m_piAppActivatedTimer;
            break;

        case TIMER_APP_TERMINATED:
            ppiTimer = &m_piAppTerminatedTimer;
            break;

        case TIMER_PDN_BLOCKED:
            ppiTimer = &m_piPdnBlockedTimer;
            break;

        case TIMER_IMS_ESTABLISHMENT:
            ppiTimer = &m_piImsEstablishmentTimer;
            break;

        case TIMER_RAT_BLOCK:
            ppiTimer = &m_piRatBlockTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    m_pUtil->StopTimer(*ppiTimer, AosProvider::GetLog()->AppTimerToString(nType));
}

PROTECTED VIRTUAL void AosApplication::ClearTimers()
{
    /*
     * NOT STOP TIMER : TIMER_RECONFIG_GUARD, TIMER_PDN_BLOCKED, TIMER_IMS_ESTABLISHMENT,
     *                  TIMER_RAT_BLOCK
     */
    if (m_piMsgConditionTimer != IMS_NULL)
    {
        StopTimer(TIMER_MSG_CONDITION);
    }

    if (m_piRegStopTimer != IMS_NULL)
    {
        StopTimer(TIMER_REG_STOP);
    }

    if (m_piRegBlockedTimer != IMS_NULL)
    {
        StopTimer(TIMER_REG_BLOCKED);
    }

    if (m_piAppActivatedTimer != IMS_NULL)
    {
        StopTimer(TIMER_APP_ACTIVATED);
    }

    if (m_piAppConnectedTimer != IMS_NULL)
    {
        StopTimer(TIMER_APP_CONNECTED);
    }

    if (m_piAppTerminatedTimer != IMS_NULL)
    {
        StopTimer(TIMER_APP_TERMINATED);
    }
}

PROTECTED VIRTUAL void AosApplication::UpdateRegState()
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
        piRsm->SetImsRegState(nOn, (m_piRegistration->GetMode() == IAosRegistration::MODE_LIMITED));
    }
}

PROTECTED VIRTUAL IMS_UINT32 AosApplication::UpdateConnectedServices(
        IN IMS_BOOL /*bEnforceUpdateRegService*/)
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_UINT32 nReportServices = ImsAosService::NONE;
    IMS_BOOL bWlanIpcan = m_piContext->GetConnection()->IsEpdgEnabled();
    IMS_UINT32 nAt = 0;
    AString strLog;

    for (nAt = 0; nAt < objHandles.GetSize(); ++nAt)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);

        if (piHandle->GetRequestType() == IAosHandle::ATTACH && piHandle->IsRegBinded())
        {
            strLog.Append("[AppId = ");
            strLog.Append(piHandle->GetAppId().GetStr());
            strLog.Append(", SrvId = ");
            strLog.Append(piHandle->GetServiceId().GetStr());
            strLog.Append("]\n");

            nReportServices |= piHandle->GetServiceType();
        }
    }

    if (strLog.GetLength() > 0)
    {
        // Remove line-feed (LF)
        strLog.Chop(1);
    }

    A_IMS_TRACE_I(APPID, "connected :: %s", strLog.GetStr(), 0, 0);

    for (nAt = 0; nAt < objHandles.GetSize(); ++nAt)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);
        IImsAosMonitor* piMonitor = piHandle->GetMonitor();

        if (piMonitor != IMS_NULL)
        {
            piMonitor->ImsAosMonitor_Connected(nReportServices,
                    (bWlanIpcan) ? IIpcan::CATEGORY_WLAN : IIpcan::CATEGORY_MOBILE);
        }
    }

    return nReportServices;
}

PROTECTED VIRTUAL void AosApplication::UpdateRegisteredRat(IN IMS_UINT32 nRegisteredRat)
{
    m_nRegisteredRat = nRegisteredRat;
}

PROTECTED VIRTUAL void AosApplication::UpdateMonitorNotify(
        IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(APPID, "UpdateMonitorNotify :: nType(%d) , nState(%d)", nType, nState, 0);

    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 nAt = 0; nAt < objHandles.GetSize(); ++nAt)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);
        IImsAosMonitor* piMonitor = piHandle->GetMonitor();

        if (piMonitor != IMS_NULL)
        {
            piMonitor->ImsAosMonitor_Notify(nType, nState);
        }
    }
}

PROTECTED VIRTUAL void AosApplication::Init()
{
    IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig == IMS_NULL)
    {
        return;
    }
    piNConfig->SetListener(this);
    A_IMS_TRACE_D(APPID, "Init", 0, 0, 0);

    m_piRegistration = m_piContext->GetRegistration();
    m_piRegistration->SetListener(this);
    SetAppType(m_piRegistration->GetRegType());

    CreateAosCondition();
    CreateAosConnector();

    AddEventListener();

    m_piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (m_piCallTracker != IMS_NULL)
    {
        m_piCallTracker->SetListener(this);
    }

    m_pUtil = AosUtil::GetInstance();

    // Condition MUST be started in last position before setting app state
    m_pCondition->Start();

    SetAppState(STATE_NOTREADY);

    if (m_pCondition->IsReady())
    {
        Condition_Changed();
    }

    if (m_nAppType == TYPE_NORMAL)
    {
        if (piNConfig->IsWfcImsAvailable())
        {
            if (piNConfig->IsGeolocationPidfSupported(
                        CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            {
                CreateAosLocationStarter();
            }
        }

        IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
        if (piService != IMS_NULL)
        {
            piService->AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
            piService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
        }
    }

    if (IsRegStateUpdatedByNrLteRatChange())
    {
        SetNetTrackerListener();
    }
}

PROTECTED VIRTUAL void AosApplication::CleanUp()
{
    A_IMS_TRACE_D(APPID, "CleanUp", 0, 0, 0);

    CleanAll();

    StopTimer(TIMER_RECONFIG_GUARD);
    StopTimer(TIMER_PDN_BLOCKED);
    StopTimer(TIMER_IMS_ESTABLISHMENT);
    StopTimer(TIMER_RAT_BLOCK);

    if (m_piNetTracker != IMS_NULL)
    {
        m_piNetTracker->RemoveListener(this);
    }

    if (m_nAppType == TYPE_NORMAL)
    {
        IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
        if (piService != IMS_NULL)
        {
            piService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
            piService->RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        }

        IAosLocationStarter* piLs = AosProvider::GetInstance()->GetLocationStarter(m_nSlotId);

        if (piLs != IMS_NULL)
        {
            AosProvider::GetInstance()->SetLocationStarter(IMS_NULL, m_nSlotId);
            delete piLs;
        }
    }

    if (m_piCallTracker != IMS_NULL)
    {
        m_piCallTracker->RemoveListener(this);
    }

    RemoveEventListener();

    ClearConnector();

    if (m_pCondition != IMS_NULL)
    {
        m_pCondition->SetListener(IMS_NULL);
        m_pCondition->Stop();

        delete m_pCondition;
        m_pCondition = IMS_NULL;
    }

    if (m_piRegistration != IMS_NULL)
    {
        m_piRegistration->SetListener(IMS_NULL);
    }

    IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig != IMS_NULL)
    {
        piNConfig->RemoveListener(this);
    }
}

PROTECTED VIRTUAL void AosApplication::Condition_Changed(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsConditionTimerSkippedDueToTimer())
    {
        return;
    }

    A_IMS_TRACE_I(APPID, "Condition_Changed :: reason(%d)", nReason, 0, 0);

    PostMessage(MSG_CONDITION, nReason, 0);
}

PROTECTED VIRTUAL void AosApplication::Condition_RequestCommand(
        IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(APPID, "Condition_RequestCommand :: cmd(%d), reason(%d)", nCommand, nReason, 0);

    if (nCommand == AosCondition::REQUEST_PDN_DISCONNECT)
    {
        nCommand = AosCondition::REQUEST_STOP;
        SetOffReason(nReason);
    }

    if (IsRequestCmdHeldByCondition(nCommand, nReason))
    {
        return;
    }

    switch (nCommand)
    {
        case AosCondition::REQUEST_STOP:  // FALL-THROUGH
        case AosCondition::REQUEST_DESTROY:
            ProcessDisconnectingState(nReason);
            PostMessage(nCommand == AosCondition::REQUEST_STOP ? MSG_REG_STOP : MSG_DESTROY,
                    nReason, 0);
            break;

        case AosCondition::REQUEST_RESET_CONNECTION_RECOVERY:
            m_pConnector->ResetReadyRecovery();
            break;

        case AosCondition::REQUEST_RECOVER:
            PostMessage(MSG_REG_RECOVER, 0, 0);
            break;

        case AosCondition::REQUEST_REASON_UPDATE:
            if ((nReason == AosReason::WIFI_OFF && m_bEpdgEnabled && IsImsCall()) ||
                    nReason == AosReason::AIRPLANE_MODE)
            {
                A_IMS_TRACE_I(APPID, "AosReason is set by condition", 0, 0, 0);
                SetOffReason(nReason);
            }
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::Connector_Activated()
{
    A_IMS_TRACE_I(APPID, "Connection_Activated", 0, 0, 0);
    PostMessage(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
}

PROTECTED VIRTUAL void AosApplication::Connector_Deactivated(IN IMS_UINT32 nReason)
{
    A_IMS_TRACE_I(APPID, "Connection_Deactivated :: reason(%d)", nReason, 0, 0);

    // To allow IMS registration to resume upon IMS PDN reconnection, the USIM authentication
    // failure block is released when the IMS PDN disconnects. (SKT specific)
    m_pCondition->ResetBlock(BLOCK_USIM_AUTHENTICATION_FAILED);
    // If the data is disconnected, it will be newly connected. So, wifi connection will be also
    // newly connected
    ClearWifiRegBlock();

    if (IsNotReady())
    {
        return;
    }

    PostMessage(MSG_CONNECTION, CONNECTION_DEACTIVATED, nReason);
}

PROTECTED VIRTUAL void AosApplication::Connector_Updated(IN IMS_UINT32 nReason)
{
    A_IMS_TRACE_I(APPID, "Connector_Updated :: reason(%d)", nReason, 0, 0);
    PostMessage(MSG_CONNECTION, CONNECTION_UPDATED, nReason);
}

PROTECTED VIRTUAL void AosApplication::Registration_StateChanged(
        IN IMS_UINT32 nResult, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(
            APPID, "Registration_StateChanged :: result(%d) , reason(%d)", nResult, nReason, 0);

    PostMessage(MSG_REGISTRATION, nResult, nReason);
}

PROTECTED VIRTUAL void AosApplication::CallTracker_StateChanged(
        IN IMS_UINT32 nType, IN CallState eState)
{
    if (nType != IAosCallTracker::TYPE_NORMAL)
    {
        return;
    }

    IMS_BOOL bCurrState = (eState > CallState::IDLE) ? IMS_TRUE : IMS_FALSE;

    if (!bCurrState && m_pConnector->ProcessPendingPcscfChange())
    {
        m_pUtil->RemoveFeature(PENDING_REG_RECOVERY_HELD, m_nRegPending);
        m_pUtil->RemoveFeature(PENDING_REG_STOP_HELD, m_nRegPending);
        SetImsCall(IMS_FALSE);
        SetRegRecoveryHeld(IMS_FALSE);
        return;
    }

    if (IsImsCall() != bCurrState)
    {
        SetImsCall(bCurrState);

        if (UpdateRegStopHeld())
        {
            m_pUtil->RemoveFeature(PENDING_REG_RECOVERY_HELD, m_nRegPending);
        }
        else
        {
            UpdateRegRecoveryHeld();
        }
    }

    if (eState == CallState::RINGING)
    {
        m_piRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT, 0);
    }
}

PROTECTED VIRTUAL void AosApplication::NetTracker_StatusChanged()
{
    if (!IsRegStateUpdatedByNrLteRatChange())
    {
        return;
    }

    ProcessImsEstablishmentStart();

    IMS_UINT32 nNewRat = m_piContext->GetNetTracker()->GetMobileNetworkType();

    if (nNewRat == NW_REPORT_RADIO_NOSRV)
    {
        return;
    }

    if (m_nRat == nNewRat)
    {
        if (nNewRat == NW_REPORT_RADIO_NR && m_nLteAttachState == IMS_LTE_INFO_COMBINED_ATTACHED)
        {
            // Set the 5GS flag, which may have been changed by LTE info during RAT guard timer.
            m_piRegistration->RequestCmd(
                    IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE);
        }
        return;
    }

    IMS_UINT32 nEps5gsOnlyState;

    if (nNewRat == NW_REPORT_RADIO_LTE)
    {
        m_pCondition->ResetBlock(BLOCK_EPS_FALLBACK_STARTED);
        nEps5gsOnlyState = (m_nLteAttachState != IMS_LTE_INFO_COMBINED_ATTACHED)
                ? IAosRegistration::REASON_SET_ENABLE
                : IAosRegistration::REASON_SET_DISABLE;
    }
    else
    {
        nEps5gsOnlyState = (nNewRat == NW_REPORT_RADIO_NR) ? IAosRegistration::REASON_SET_ENABLE
                                                           : IAosRegistration::REASON_SET_DISABLE;
    }
    m_piRegistration->RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, nEps5gsOnlyState);

    if (!m_piContext->GetConnection()->IsEpdgEnabled() &&
            m_pUtil->IsSupportedNetworkTypeForCellular(m_nRat) &&
            m_pUtil->IsSupportedNetworkTypeForCellular(nNewRat))
    {
        if (IsOn() && IsRegUpdatedByNrLteRatChange() && !IsRegisteredNetwork(nNewRat))
        {
            if (IsTimerRunning(TIMER_RECONFIG_GUARD))
            {
                m_pUtil->RemoveFeature(PENDING_IPCAN_HELD, m_nRegPending);
                m_pUtil->AddFeature(PENDING_REG_UPDATE_HELD, m_nRegPending);
            }
            else
            {
                // nWparam : bIgnoreRetryTimer , nLparam : bExplicitUpdate
                PostMessage(MSG_REG_UPDATE, 0, 0);
            }
        }
    }

    if (!m_pConnector->IsReady())
    {
        m_pConnector->ResetReadyRecovery();
    }

    if (m_pCondition->IsReasonBlocked(BLOCK_INVALID_CONNECTION))
    {
        m_pCondition->ResetBlock(BLOCK_INVALID_CONNECTION);
    }

    if (IsBlockRat(nNewRat))
    {
        A_IMS_TRACE_D(APPID, "NetTracker_StatusChanged :: New RAT(0x%x), Blocked RATs(0x%x)",
                nNewRat, m_nBlockedRats, 0);
        PerformRatBlockActions(IMS_TRUE);
    }
    else if (m_pCondition->IsReasonBlocked(BLOCK_CELLULAR_RAT_BLOCK))
    {
        StopTimer(TIMER_RAT_BLOCK);
        m_pCondition->ResetBlock(BLOCK_CELLULAR_RAT_BLOCK);
    }

    m_nRat = nNewRat;
}

PROTECTED VIRTUAL void AosApplication::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_D(APPID, "NConfiguration_NotifyConfigChanged :: changed", 0, 0, 0);

    const IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig == IMS_NULL)
    {
        return;
    }

    if (piNConfig->IsWfcImsAvailable())
    {
        if (piNConfig->IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
        {
            CreateAosLocationStarter();
        }
    }
}

PROTECTED VIRTUAL void AosApplication::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    switch (nEvent)
    {
        case IMS_EVENT_REG_CONTROL:
            ProcessRegControlEvent(nWParam, nLParam);
            break;

        case IMS_EVENT_ROAMING_STATE:
            ProcessImsEstablishmentStart();
            ProcessRoamingState(nWParam == IMS_ROAMING_STATE_ON);
            break;

        case IMS_EVENT_VOICE_SERVICE_STATE:  // FALL-THROUGH
        case IMS_EVENT_LTE_INFO:
            ProcessNetworkEvent(nEvent, nWParam, nLParam);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosApplication::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piReconfigTimer)
    {
        ProcessReconfigTimerExpired();
        return;
    }

    if (piTimer == m_piMsgConditionTimer)
    {
        StopTimer(TIMER_MSG_CONDITION);
        ProcessStateStart();
        return;
    }

    if (piTimer == m_piRegStopTimer)
    {
        ProcessRegStopTimerExpired();
        return;
    }

    if (piTimer == m_piRegBlockedTimer)
    {
        ProcessRegBlockedTimerExpired();
        return;
    }

    if (piTimer == m_piAppActivatedTimer)
    {
        ProcessAppActivatedTimerExpired();
        return;
    }

    if (piTimer == m_piAppConnectedTimer)
    {
        ProcessAppConnectedTimerExpired();
        return;
    }

    if (piTimer == m_piAppTerminatedTimer)
    {
        ProcessAppTerminatedTimerExpired();
        return;
    }

    if (piTimer == m_piPdnBlockedTimer)
    {
        ProcessPdnBlockedTimerExpired();
        return;
    }

    if (piTimer == m_piImsEstablishmentTimer)
    {
        ProcessImsEstablishmentTimerExpired();
        return;
    }

    if (piTimer == m_piRatBlockTimer)
    {
        ProcessRatBlockTimerExpired();
        return;
    }
}

PROTECTED VIRTUAL void AosApplication::RegistrationControl_ControlRegistration(
        IN AosRegRequestType eType, IN AosPcscfOrder /* eOrder */, IN AosControlCause eCause)
{
    if (eCause == AosControlCause::IMS_SERVICE)
    {
        IMS_BOOL bImsServiceBlocked = m_pCondition->IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED);

        if (eType == AosRegRequestType::START)
        {
            if (bImsServiceBlocked)
            {
                m_pCondition->ResetBlock(BLOCK_IMS_SERVICE_DISABLED);
            }
        }
        else if (eType == AosRegRequestType::STOP)
        {
            if (!bImsServiceBlocked)
            {
                ProcessDisconnectingState();
                PostMessage(MSG_REG_STOP, 0, 0);
                m_pCondition->SetBlock(BLOCK_IMS_SERVICE_DISABLED, IMS_FALSE);
            }
        }

        return;
    }

    if (eType == AosRegRequestType::START)
    {
        // TODO: impl. except CURRENT
        PostMessage(MSG_REG_RECOVER, 0, 0);
        return;
    }

    if (eType == AosRegRequestType::REFRESH)
    {
        PostMessage(MSG_REG_UPDATE, 0, 1);
        return;
    }

    if (eType == AosRegRequestType::STOP)
    {
        if (eCause == AosControlCause::RADIO_SIM_REMOVED ||
                eCause == AosControlCause::RADIO_SIM_REFRESH)
        {
            m_piRegistration->SetReasonCode(AosReasonCode::NORMAL_DEREGISTRATION);
            if (!IsEmergency())
            {
                m_bPdnDeactivationRequired = IMS_TRUE;
            }
        }

        if (eCause == AosControlCause::DATA)
        {
            const IMS_UINT32 nOffReason = GetOffReason();
            IMS_UINT32 nFinalOffReason;

            switch (nOffReason)
            {
                case AosReason::AIRPLANE_MODE:
                    m_piRegistration->SetReasonCode(AosReasonCode::NORMAL_DEREGISTRATION);
                    nFinalOffReason = nOffReason;
                    break;

                case AosReason::WIFI_OFF:
                    nFinalOffReason = nOffReason;
                    break;

                default:
                    nFinalOffReason = AosReason::DATA_DISCONNECTED;
                    break;
            }

            ProcessDisconnectingState(nFinalOffReason);
        }
        else
        {
            ProcessDisconnectingState();
        }

        PostMessage(MSG_REG_STOP, 0, 0);
        return;
    }

    if (eType == AosRegRequestType::START_IMS_EST_TIMER)
    {
        if (GET_N_CONFIG(m_nSlotId)->GetImsEstablishmentTimeForLte() > 0)
        {
            PostMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
        }

        return;
    }
}

PROTECTED VIRTUAL void AosApplication::RegistrationControl_UpdateDataFailureReason(
        IN IMS_SINT32 nReason)
{
    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return;
    }

    SetDataFailureReason(nReason);
}

PROTECTED VIRTUAL void AosApplication::ServicePhone_LocationInfoChanged(IN LocationInfo eState)
{
    if (!GET_N_CONFIG(m_nSlotId)->IsReregRetryWithChangedCountryOnWifi())
    {
        return;
    }

    if (eState != LocationInfo::COUNTRY_CHANGED || !m_piContext->GetConnection()->IsEpdgEnabled())
    {
        return;
    }

    A_IMS_TRACE_I(APPID,
            "ServicePhone_LocationInfoChanged :: reg update due to country change over epdg", 0, 0,
            0);

    PostMessage(MSG_REG_UPDATE, 0, 0);
}
