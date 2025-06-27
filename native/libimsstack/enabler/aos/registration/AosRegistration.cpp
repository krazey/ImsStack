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
#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "IImsRadio.h"
#include "IIpcan.h"
#include "ISipConfig.h"
#include "ISubscriberConfig.h"

#include "CarrierConfig.h"

#include "IConfiguration.h"
#include "IRegContact.h"
#include "IRegistration.h"
#include "IRegistrationManager.h"
#include "IRegParameter.h"
#include "IRegSubscription.h"
#include "ISipHeader.h"
#include "ISipRtConfigHelper.h"
#include "ISipTransportHelper.h"
#include "Credential.h"
#include "Engine.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipFactory.h"
#include "SipProfile.h"
#include "SipRtConfig.h"
#include "SipStatusCode.h"
#include "SipHeaderName.h"

#include "GeolocationHelper.h"

#include "AosAppRequestType.h"
#include "IAosService.h"
#include "ImsAosParameter.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosHandle.h"
#include "interface/IAosLocationStarter.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosPcscf.h"
#include "interface/IAosRegistrationListener.h"
#include "interface/IAosRegStateManager.h"
#include "interface/IAosSubscriber.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosString.h"
#include "provider/AosUtil.h"
#include "registration/AosRegistration.h"
#include "registration/AosIpsecHelper.h"
#include "registration/AosSubscription.h"

__IMS_TRACE_TAG_AOS__;

#define REGID m_strTag.GetStr()

PUBLIC
AosRegistration::AosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
        ImsActivityEx(strRegId),
        m_piContext(piAppContext),
        m_piListener(IMS_NULL),
        m_nSlotId(piAppContext->GetSlotId()),
        m_piRegManager(IMS_NULL),
        m_piRegistration(IMS_NULL),
        m_piRegContact(IMS_NULL),
        m_piRegParameter(IMS_NULL),
        m_pSubscription(IMS_NULL),
        m_pIpsecHelper(IMS_NULL),
        m_pUtil(IMS_NULL),
        m_nFeature(FEATURE_NONE),
        m_nState(STATE_OFFLINE),
        m_nTxnPending(PENDING_NONE),
        m_bIsTransactionStarted(IMS_TRUE),
        m_bIsImsCall(IMS_FALSE),
        m_bIsBlocked(IMS_FALSE),
        m_bIsHeldByCall(IMS_FALSE),
        m_bIsAppReady(IMS_FALSE),
        m_bIsRadioWaiting(IMS_FALSE),
        m_bIsTrafficPriorityBlocked(IMS_FALSE),
        m_bIsReregFailureReportOnIpcanChangeRequired(IMS_FALSE),
        m_strRegId(strRegId),
        m_eRegType(AosRegistrationType::NORMAL),
        m_nFlowId(0),
        m_bFakeReg(IMS_FALSE),
        m_nRegMode(MODE_NORMAL),
        m_nPcscfPort(0),
        m_nRetryBaseTime(30),
        m_nRetryMaxTime(1800),
        m_nConsecutiveFailure(0),
        m_nConsecutiveFailureForPdnReactivated(0),
        m_nForbiddenCount(0),
        m_nSubConsecutiveFailureForRegForbiddenInWifi(0),
        m_piOfflineRecoverTimer(IMS_NULL),
        m_piStopRetryTimer(IMS_NULL),
        m_piRefreshTimer(IMS_NULL),
        m_piDeregTrafficTimer(IMS_NULL),
        m_piModeTimer(IMS_NULL),
        m_piTransactionTimer(IMS_NULL),
        m_piInternalErrorTimer(IMS_NULL),
        m_nAuthChallengeCount(0),
        m_nAuthFailureCount(0),
        m_nAuthIpsecCount(0),
        m_nErrorCountForServerSocket(0),
        m_bCallingNumberVerificationSupported(IMS_FALSE),
        m_nNetworkBindingFeatures(0),
        m_bEps5GsOnly(IMS_TRUE),
        m_nIpsecBlockReason(0),
        m_nImsRegState(IMS_REG_STATE_DEREGISTERED),
        m_nImsRegFeatures(ImsAosFeature::NONE),
        m_eImsRegNetwork(AosNetworkType::NONE),
        m_eImsReasonCode(AosReasonCode::UNSPECIFIED),
        m_pSipProfile(IMS_NULL),
        m_nPdnReactivateWaitTime(RETRY_DEFAULT_WAIT_TIME),
        m_nRegIpcanCategory(IIpcan::CATEGORY_MOBILE)
{
    // Init Object
    m_pUtil = AosUtil::GetInstance();
    m_piRegManager = Engine::GetRegistrationManager();

    // registration type
    m_eRegType = piAppContext->GetStaticProfile()->GetRegistrationType();

    // registration flow ID
    m_nFlowId = piAppContext->GetStaticProfile()->GetRegistrationFlowId();

    m_strTag.Sprintf("%d:%s", m_nSlotId, m_strRegId.GetStr());

    IMS_CHAR acLog[128 + 1] = {
            0,
    };
    IMS_Sprintf(acLog, 128, "REG(%s)//FLOW(%d)", REGID, m_nFlowId);

    IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig != IMS_NULL)
    {
        piNConfig->SetListener(this);
    }

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosRegistration = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosRegistration), this);
};

PUBLIC VIRTUAL AosRegistration::~AosRegistration()
{
    IMS_CHAR acLog[128 + 1] = {
            0,
    };
    IMS_Sprintf(acLog, 128, "REG(%s)/FLOW(%d)", REGID, m_nFlowId);

    IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig != IMS_NULL)
    {
        piNConfig->RemoveListener(this);
    }

    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosRegistration = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosRegistration), this);
}

PUBLIC VIRTUAL void AosRegistration::Start()
{
    A_IMS_TRACE_I(
            REGID, "Start :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    if (m_piStopRetryTimer != IMS_NULL &&
            GET_N_CONFIG(m_nSlotId)->IsKeepRegRetryTimerOnAllEnablersDetached())
    {
        A_IMS_TRACE_D(REGID, "Start :: Stop Retry timer is running. Wait for expiration", 0, 0, 0);
        return;
    }

    // ONLY start in OFFLINE
    if (m_nState != STATE_OFFLINE)
    {
        Destroy();
    }

    StopTimer(TIMER_OFFLINE_RECOVER);

    if (m_eRegType == AosRegistrationType::NORMAL && !IsTransactionStarted())
    {
        m_pUtil->AddFeature(PENDING_START, m_nTxnPending);
        A_IMS_TRACE_I(REGID, "Start :: reg can't be started due to block", 0, 0, 0);
        return;
    }

    if (!CheckRadioReadyAndSetTxnPending())
    {
        A_IMS_TRACE_I(REGID, "Start :: txn is pending due to radio", 0, 0, 0);
        return;
    }

    if (IsRegTypeEqual(AosRegistrationType::EMERGENCY) && !IsFakeRegistration())
    {
        SetTraffic(IMS_TRUE);
    }

    if (!CreateRegistration())
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REGISTERING);
    ReportTryingState();
}

PUBLIC VIRTUAL void AosRegistration::Stop()
{
    A_IMS_TRACE_I(
            REGID, "Stop :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    switch (m_nState)
    {
        case STATE_OFFLINE:      // FALL-THROUGH
        case STATE_REGISTERING:  // FALL-THROUGH
        case STATE_REGSTOP:      // FALL-THROUGH
        case STATE_REFRESHING:
            Destroy();
            ReportStateChanged(RESULT_SUCCESS);
            break;

        case STATE_REGISTERED:  // FALL-THROUGH
        case STATE_REFRESHSTOP:
            ProcessStop();
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void AosRegistration::Update(IN IMS_BOOL bIgnoreRetryTimer /* = IMS_FALSE */,
        IN IMS_BOOL bExplicitUpdate /* = IMS_TRUE */)
{
    A_IMS_TRACE_I(
            REGID, "Update :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);

    switch (m_nState)
    {
        case STATE_REGSTOP:  // FALL-THROUGH
        case STATE_REFRESHSTOP:
            ProcessRetryInRegStopped(bIgnoreRetryTimer);
            break;

        case STATE_REGISTERED:
            ProcessReregister();
            break;

        case STATE_REGISTERING:  // FALL-THROUGH
        case STATE_REFRESHING:
            if (bExplicitUpdate)
            {
                m_pUtil->AddFeature(PENDING_UPDATE, m_nTxnPending);
            }
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void AosRegistration::Reconfig()
{
    A_IMS_TRACE_I(REGID, "Reconfig :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState),
            0, 0);

    switch (m_nState)
    {
        case STATE_REGSTOP:  // FALL-THROUGH
        case STATE_REFRESHSTOP:
            UpdateRegBinding();
            break;

        case STATE_REGISTERED:
            if (UpdateRegBinding())
            {
                ProcessReregister();
            }
            break;

        case STATE_REGISTERING:  // FALL-THROUGH
        case STATE_REFRESHING:
            m_pUtil->AddFeature(PENDING_RECONFIG, m_nTxnPending);
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void AosRegistration::Destroy()
{
    SetContactAddressConfiguration(IMS_FALSE);
    ClearRegParameters();
    DestroyRegistration();
    SetState(STATE_OFFLINE);
}

PUBLIC VIRTUAL void AosRegistration::SetListener(IN IAosRegistrationListener* piRegListener)
{
    m_piListener = piRegListener;
}

PUBLIC VIRTUAL void AosRegistration::RequestCmd(
        IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(REGID, "RequestCmd :: nCmdType (%d), nReason (%d)", nCmdType, nReason, 0);

    switch (nCmdType)
    {
        case CMD_INIT_PCSCF:
            if (nReason == REASON_INIT_PCSCF_CLEAR)
            {
                m_piContext->GetPcscf()->SetFirstPcscfIndex();
            }
            break;

        case CMD_INIT_AWT:
            SetRetryTime();
            break;

        case CMD_CLEAR_RETRY_COUNT:
            ClearRetryCount(IMS_TRUE);
            break;

        case CMD_SET_IPSEC:
            ProcessSetIpsec(nReason);
            break;

        case CMD_REFRESH_REGINFO:
            ProcessRefreshRegInfo();
            break;

        case CMD_UPDATE_REG_BINDING:
            UpdateRegBinding();
            break;

        case CMD_IPCAN_CHANGED:
            ProcessIpcanChanged();
            break;

        case CMD_UPDATE_IPCAN:
            ProcessUpdateIpcan();
            break;

        case CMD_SCSCF_RESTORATION:
            ProcessScscfRestoration(nReason);
            break;

        case CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT:
            ClearErrorCount();
            break;

        case CMD_UPDATE_FEATURE_WITHOUT_REG:
            UpdateDetailState(GetState());
            break;

        case CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED:
            if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPolicy() ==
                            CarrierConfig::Ims::ERROR_POLICY_PDN_REACTIVATED &&
                    m_bEps5GsOnly)
            {
                m_nConsecutiveFailureForPdnReactivated++;
            }
            break;

        case CMD_SET_EPS_5GS_ONLY:
            m_bEps5GsOnly = (nReason == REASON_SET_ENABLE);
            break;

        case CMD_CLOSE_UNSECURE_TCP_SOCKET:
            CloseUnsecureTcpSocket();
            break;

        case CMD_UPDATE_STOP_RETRY_TIMER_WITH_DEFAULT:
            UpdateStopRetryTimer(RETRY_DEFAULT_WAIT_TIME);
            break;

        case CMD_REINITIATE_REG_WITH_RETRY_AFTER:
            ReinitiateRegistration(nReason);
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL IMS_UINT32 AosRegistration::GetMode()
{
    A_IMS_TRACE_I(REGID, "GetMode :: (%d)", m_nRegMode, 0, 0);
    return m_nRegMode;
}

PUBLIC VIRTUAL IMS_UINT32 AosRegistration::GetProperty(
        IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue)
{
    ImsList<AString> strList;

    switch (nType)
    {
        case PROPERTY_LOCAL_PORT:
            nValue = m_pUtil->GetLocalPort(m_nSlotId);
            break;

        case PROPERTY_PCSCF_PORT:
            nValue = m_nPcscfPort;
            break;

        case PROPERTY_LOCAL_ADDRESS:
            strValue = m_objIpa.ToString();
            break;

        case PROPERTY_PCSCF_ADDRESS:
            strValue = m_strPcscf;
            break;

        case PROPERTY_ASSOCIATED_URI:
            if (m_piRegistration != IMS_NULL)
            {
                const AStringArray& objUris = m_piRegistration->GetAssociatedUris();

                if (!objUris.IsEmpty())
                {
                    strValue = objUris.GetFirstElement();
                }
            }
            break;

        case PROPERTY_PATH:
            if (m_piRegistration != IMS_NULL)
            {
                const ISipMessage* piMessage = m_piRegistration->GetPreviousResponse();

                if (piMessage != IMS_NULL)
                {
                    strValue = piMessage->GetHeader(ISipHeader::PATH);
                }
            }
            break;

        case PROPERTY_LAST_PATH:
            if (m_piRegistration != IMS_NULL)
            {
                const ISipMessage* piMessage = m_piRegistration->GetPreviousResponse();
                if (piMessage != IMS_NULL)
                {
                    strList = piMessage->GetHeaders(ISipHeader::PATH);
                    if (strList.IsEmpty() == IMS_FALSE)
                    {
                        strValue = strList.GetAt(strList.GetSize() - 1);
                    }
                }
            }
            break;

        case PROPERTY_SUPPORTED:
            if (m_piRegistration != IMS_NULL)
            {
                const ISipMessage* piMessage = m_piRegistration->GetPreviousRequest();

                if (piMessage != IMS_NULL)
                {
                    strValue = piMessage->GetHeader(ISipHeader::SUPPORTED);
                }
            }
            break;

        case PROPERTY_SERVICE_ROUTE:
            if (m_piRegistration != IMS_NULL)
            {
                const ISipMessage* piMessage = m_piRegistration->GetPreviousResponse();

                if (piMessage != IMS_NULL)
                {
                    strValue = piMessage->GetHeader(ISipHeader::SERVICE_ROUTE);
                }
            }
            break;

        case PROPERTY_PROTECTED:
            nValue = AosRegProtectedType::REG_UNPROTECTED;

            if (GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN)
            {
                nValue = AosRegProtectedType::REG_PROTECTED;
            }
            else if (GetRegIpcanCategory() == IIpcan::CATEGORY_MOBILE)
            {
                if (m_pIpsecHelper != IMS_NULL && m_pIpsecHelper->IsEstablished())
                {
                    nValue = AosRegProtectedType::REG_PROTECTED;
                }
            }

            break;

        case PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION:
            if (m_bCallingNumberVerificationSupported)
            {
                nValue = AosSupportability::SUPPORTED;
            }
            else
            {
                nValue = AosSupportability::NOT_SUPPORTED;
            }
            break;

        case PROPERTY_NETWORK_BINDING_FEATURES:
            nValue = m_nNetworkBindingFeatures;
            break;

        case PROPERTY_PDN_REACIVATE_WAIT_TIME:
            nValue = m_nPdnReactivateWaitTime;
            break;

        case PROPERTY_REG_FAILURE_COUNT:
            nValue = m_nConsecutiveFailure;
            break;

        case PROPERTY_TRAFFIC_PRIORITY_BLOCK:
            nValue = (m_bIsTrafficPriorityBlocked) ? AosProperty::AOS_TRUE : AosProperty::AOS_FALSE;
            break;

        default:
            break;
    }

    return 0;
}

PUBLIC VIRTUAL IMS_UINT32 AosRegistration::GetState()
{
    A_IMS_TRACE_I(
            REGID, "GetState :: (%s)", AosProvider::GetLog()->RegStateToString(m_nState), 0, 0);
    return m_nState;
}

PUBLIC VIRTUAL AosRegistrationType AosRegistration::GetRegType()
{
    return m_eRegType;
}

PUBLIC VIRTUAL IMS_SINT32 AosRegistration::GetImsRegType()
{
    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        return IMS_REG_TYPE_NORMAL;
    }

    if (m_eRegType == AosRegistrationType::EMERGENCY)
    {
        return (IsFakeRegistration() ? IMS_REG_TYPE_FAKE : IMS_REG_TYPE_EMERGENCY);
    }

    if (m_eRegType == AosRegistrationType::FAKE)
    {
        return IMS_REG_TYPE_FAKE;
    }

    return IMS_REG_TYPE_INVALID;
}

PUBLIC VIRTUAL IMS_BOOL AosRegistration::IsInCallbackMode()
{
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosRegistration::IsRegistered()
{
    return (IsRefreshing() || m_nState == STATE_REGISTERED);
}

PUBLIC VIRTUAL IMS_BOOL AosRegistration::IsRefreshing()
{
    return (m_nState == STATE_REFRESHING || m_nState == STATE_REFRESHSTOP);
}

PUBLIC VIRTUAL IMS_BOOL AosRegistration::IsRetryTimer()
{
    return (m_piStopRetryTimer != IMS_NULL || m_piOfflineRecoverTimer != IMS_NULL);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::IsRetryHeld()
{
    return (m_nState == STATE_REGSTOP || m_nState == STATE_REFRESHSTOP);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::IsTerminated()
{
    return m_pUtil->IsFeatureOn(PENDING_TERMINATED, m_nTxnPending);
}

PROTECTED VIRTUAL void AosRegistration::SetAppReady(IN IMS_BOOL bReady)
{
    m_bIsAppReady = bReady;
}

PROTECTED
void AosRegistration::SetState(IN IMS_UINT32 nState)
{
    if (m_nState == nState)
    {
        return;
    }

    A_IMS_TRACE_I(REGID, "SetState :: old state(%s) to state(%s)",
            AosProvider::GetLog()->RegStateToString(m_nState),
            AosProvider::GetLog()->RegStateToString(nState), 0);

    IMS_UINT32 nOldState = m_nState;
    m_nState = nState;

    if (m_eRegType != AosRegistrationType::RCS)
    {
        UpdateDetailState(m_nState);
    }

    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        if (!IsRegTrying())
        {
            SetTraffic(IMS_FALSE);

            if (nOldState == STATE_DEREGISTERING && m_piDeregTrafficTimer != IMS_NULL)
            {
                StopTimer(TIMER_DEREG_TRAFFIC);
                SetTrafficForDeregister(IMS_FALSE);
            }
        }
    }
}

PROTECTED
void AosRegistration::SetMode(IN IMS_UINT32 nMode)
{
    A_IMS_TRACE_I(REGID, "SetMode :: (%s)", AosProvider::GetLog()->RegModeToString(nMode), 0, 0);
    m_nRegMode = nMode;

    UpdateModeToHandles();
}

PROTECTED
void AosRegistration::SetFakeReg(IN IMS_BOOL bFake)
{
    A_IMS_TRACE_I(REGID, "SetFakeReg :: (%s)", (bFake) ? "ON" : "OFF", 0, 0);
    m_bFakeReg = bFake;
}

PROTECTED
void AosRegistration::SetBlocked(IN IMS_BOOL bBlocked)
{
    A_IMS_TRACE_I(REGID, "SetBlocked :: (%s)", (bBlocked) ? "BLOCK" : "NOT BLOCK", 0, 0);
    m_bIsBlocked = bBlocked;
}

PROTECTED
void AosRegistration::SetHeldByCall(IN IMS_BOOL bHeld)
{
    A_IMS_TRACE_I(REGID, "SetHeldByCall :: (%s)", (bHeld) ? "HOLD" : "NOT HOLD", 0, 0);
    m_bIsHeldByCall = bHeld;
}

PROTECTED
void AosRegistration::SetImsCall(IN IMS_BOOL bStarted)
{
    A_IMS_TRACE_I(REGID, "SetImsCall :: (%s)", (bStarted) ? "STARTED" : "TERMINATED", 0, 0);
    m_bIsImsCall = bStarted;
}

PROTECTED
void AosRegistration::SetRadioWaiting(IN IMS_BOOL bWaiting)
{
    m_bIsRadioWaiting = bWaiting;
}

PROTECTED
void AosRegistration::SetTrafficPriorityBlocked(IN IMS_BOOL bBlocked)
{
    A_IMS_TRACE_I(REGID, "SetTrafficPriorityBlocked :: (%s)",
            (bBlocked) ? "BLOCKED" : "NOT BLOCKED", 0, 0);
    m_bIsTrafficPriorityBlocked = bBlocked;
}

PROTECTED
void AosRegistration::SetRetryTime()
{
    m_nRetryBaseTime = GET_N_CONFIG(m_nSlotId)->GetRegistrationRetryBaseTime() / 1000;
    m_nRetryMaxTime = GET_N_CONFIG(m_nSlotId)->GetRegistrationRetryMaxTime() / 1000;

    A_IMS_TRACE_I(REGID, "m_nRetryBaseTime (%d) , m_nRetryMaxTime (%d)", m_nRetryBaseTime,
            m_nRetryMaxTime, 0);
}

PROTECTED
IMS_BOOL AosRegistration::SetTraffic(IN IMS_BOOL bStarted)
{
    if (m_eRegType != AosRegistrationType::NORMAL && m_eRegType != AosRegistrationType::EMERGENCY)
    {
        return IMS_FALSE;
    }

    IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);

    if (piTransaction != IMS_NULL)
    {
        if (bStarted)
        {
            if (m_eRegType == AosRegistrationType::NORMAL)
            {
                return piTransaction->StartTraffic(
                        IAosTransaction::TYPE_REG, m_piContext->GetNetTracker()->GetNetworkType());
            }
            else
            {
                piTransaction->StartEmergencyTraffic(
                        m_piContext->GetNetTracker()->GetNetworkType());
            }
        }
        else
        {
            if (m_eRegType == AosRegistrationType::NORMAL)
            {
                piTransaction->StopTraffic(IAosTransaction::TYPE_REG);
            }
            else
            {
                piTransaction->StopEmergencyTraffic();
            }
        }
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosRegistration::SetTrafficForDeregister(IN IMS_BOOL bStarted)
{
    IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);

    if (piTransaction != IMS_NULL)
    {
        if (bStarted)
        {
            return piTransaction->StartTraffic(
                    IAosTransaction::TYPE_DEREG, m_piContext->GetNetTracker()->GetNetworkType());
        }
        else
        {
            piTransaction->StopTraffic(IAosTransaction::TYPE_DEREG);
        }
    }

    return IMS_TRUE;
}

PROTECTED
void AosRegistration::SetTrafficListener(IN IMS_BOOL bSet)
{
    if (m_eRegType != AosRegistrationType::NORMAL && m_eRegType != AosRegistrationType::EMERGENCY)
    {
        return;
    }

    IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);

    if (piTransaction == IMS_NULL)
    {
        return;
    }

    IMS_UINT32 nType = (m_eRegType == AosRegistrationType::NORMAL)
            ? IAosTransaction::TYPE_REG
            : IAosTransaction::TYPE_EMERGENCY;

    if (bSet)
    {
        piTransaction->SetListener(nType, this);
    }
    else
    {
        piTransaction->RemoveListener(nType, this);
    }
}

PROTECTED
void AosRegistration::UpdateRegIpcanCategory()
{
    m_nRegIpcanCategory = m_piContext->GetConnection()->GetIpcanCategory();
}

PROTECTED
void AosRegistration::ClearPending()
{
    m_nTxnPending = PENDING_NONE;
}

PROTECTED
void AosRegistration::ClearCallingNumberVerification()
{
    m_bCallingNumberVerificationSupported = IMS_FALSE;
}

PROTECTED
IMS_BOOL AosRegistration::IsFakeRegistration() const
{
    return m_bFakeReg;
}

PROTECTED
IMS_BOOL AosRegistration::IsIpsecSupported() const
{
    if (IsFakeRegistration())
    {
        return IMS_FALSE;
    }

    return (m_pUtil->IsFeatureOn(FEATURE_IPSEC, m_nFeature)) &&
            (m_nIpsecBlockReason == IPSEC_BLOCK_NONE);
}

PROTECTED
IMS_BOOL AosRegistration::IsAuthChallengedAgain() const
{
    return (m_nAuthChallengeCount > 1);
}

PROTECTED
IMS_BOOL AosRegistration::IsAuthChallengeMoreAllowed()
{
    return (m_nAuthChallengeCount < AUTHENTICATION_RETRY_MAX_COUNT);
}

PROTECTED
IMS_BOOL AosRegistration::IsAuthFailureMaxCountReached() const
{
    IMS_SINT32 nAuthFailureMaxCount = GET_N_CONFIG(m_nSlotId)->GetAuthFailureRetryMaxCnt();

    if (nAuthFailureMaxCount <= 0)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(REGID, "IsAuthFailureMaxCountReached: %d", m_nAuthFailureCount, 0, 0);

    return (m_nAuthFailureCount >= (nAuthFailureMaxCount - 1));
}

PROTECTED
IMS_BOOL AosRegistration::IsTransactionStarted() const
{
    return m_bIsTransactionStarted;
}

PROTECTED
IMS_BOOL AosRegistration::IsBlocked() const
{
    return m_bIsBlocked;
}

PROTECTED
IMS_BOOL AosRegistration::IsHeldByCall() const
{
    return m_bIsHeldByCall;
}

PROTECTED
IMS_BOOL AosRegistration::IsImsCall() const
{
    return m_bIsImsCall;
}

PROTECTED
IMS_BOOL AosRegistration::IsAppReady() const
{
    return m_bIsAppReady;
}

PROTECTED
IMS_BOOL AosRegistration::IsExtraCapabilityRequiredForFeatureTag(IN IMS_UINT32 nOption) const
{
    return (m_eRegType == AosRegistrationType::RCS ||
            nOption == AosFeatureTag::OPTION_EXTRA_CAPABILITY);
}

PROTECTED
IMS_BOOL AosRegistration::IsReconnectingServerSocketErrorAllowed() const
{
    return (m_nErrorCountForServerSocket < RECONNECT_SERVER_SOCKET_ERROR_MAX_COUNT);
}

PROTECTED
IMS_BOOL AosRegistration::IsRegTypeEqual(IN AosRegistrationType eType) const
{
    return (m_eRegType == eType);
}

PROTECTED
IMS_BOOL AosRegistration::IsRegTrying() const
{
    return (m_nState == STATE_REGISTERING || m_nState == STATE_REFRESHING);
}

PROTECTED
IMS_BOOL AosRegistration::IsNetworkBindingSupported(IN IAosHandle* /* piHandle */)
{
    /* TODO_CONFIG : implement with different solution
    if (m_pUtil->IsFeatureOn(piHandle->GetServiceType(),
            m_piContext->GetConfig()->GetNetworkRegBindingSupportedServices()))
    {
        return IMS_TRUE;
    }
    */

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosRegistration::IsCallStateRequired() const
{
    return (m_eRegType == AosRegistrationType::EMERGENCY ||
            m_eRegType == AosRegistrationType::NORMAL);
}

PROTECTED
IMS_BOOL AosRegistration::IsValidCallType(IN IMS_UINT32 nType) const
{
    return (nType == IAosCallTracker::TYPE_NORMAL) ||
            ((nType == IAosCallTracker::TYPE_EMERGENCY &&
                    GET_N_CONFIG(m_nSlotId)->IsRegRequiredAfterImsECallEndOnRegHeld()));
}

PROTECTED
IMS_BOOL AosRegistration::IsRadioWaiting() const
{
    return m_bIsRadioWaiting;
}

PROTECTED
IMS_BOOL AosRegistration::IsTrafficPriorityBlocked() const
{
    return m_bIsTrafficPriorityBlocked;
}

PROTECTED
IMS_BOOL AosRegistration::IsReregFailureReportOnIpcanChangeRequired() const
{
    return m_bIsReregFailureReportOnIpcanChangeRequired;
}

PROTECTED
IMS_BOOL AosRegistration::IsErrorCodeExisted(
        IN const ImsVector<IMS_SINT32>& objErrorCode, IN IMS_SINT32 nCode) const
{
    for (int i = 0; i < objErrorCode.GetSize(); i++)
    {
        IMS_SINT32 nErrorCode = objErrorCode.GetAt(i);
        if (nCode == nErrorCode)
        {
            return IMS_TRUE;
        }

        if (SipStatusCode::IsFinalFailure(nCode))
        {
            IMS_SINT32 nGroupCode = (nCode / 100);
            if (nErrorCode == CarrierConfig::Ims::REG_ERROR_CODE_ALL_RESP ||
                    nErrorCode == nGroupCode)
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosRegistration::IsRegForbiddenInWifi()
{
    IMS_SINT32 nSubRetryCntConfig = GET_N_CONFIG(m_piContext->GetSlotId())
                                            ->GetSubConsecutiveRetryCntForRegForbiddenInWifi();
    if (nSubRetryCntConfig <= 0)
    {
        return IMS_FALSE;
    }

    m_nSubConsecutiveFailureForRegForbiddenInWifi++;
    if (m_nSubConsecutiveFailureForRegForbiddenInWifi < nSubRetryCntConfig)
    {
        A_IMS_TRACE_I(REGID, "IsRegForbiddenInWifi :: retry count(%d)",
                m_nSubConsecutiveFailureForRegForbiddenInWifi, 0, 0);
        return IMS_FALSE;
    }

    m_nSubConsecutiveFailureForRegForbiddenInWifi = 0;
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosRegistration::IsConnectionFailureForOfflineRecovery(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode) const
{
    if (nFailureReason == IImsRadio::REASON_NAS_FAILURE ||
            nFailureReason == IImsRadio::REASON_RRC_REJECT)
    {
        return IMS_TRUE;
    }

    if (nFailureReason == IImsRadio::REASON_RACH_FAILURE &&
            nCauseCode == IImsRadioConnectionListener::CAUSE_CODE_SR_LLF_TIMER_START)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_SINT32 AosRegistration::GetRegExpires()
{
    return (m_piRegContact != IMS_NULL) ? m_piRegContact->GetExpires() : -1;
}

PROTECTED
void AosRegistration::IncreaseConsecutiveFailCount()
{
    if (!GET_N_CONFIG(m_nSlotId)->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
    {
        m_nConsecutiveFailure++;
    }
}

PROTECTED
IMS_BOOL AosRegistration::UpdatePreloadedRoute(IN const AString& strPcscf
        /* = AString::ConstNull() */,
        IN const IMS_UINT32 nPcscfPort /* = 0 */)
{
    if (m_piRegParameter == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "parameter is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (strPcscf.GetLength() > 0)
    {
        m_strPcscf = strPcscf;
    }

    if (nPcscfPort != 0)
    {
        m_nPcscfPort = nPcscfPort;
    }

    m_piRegParameter->RemoveAllPreloadedRoutes();
    m_piRegParameter->AddPreloadedRoute(m_strPcscf, static_cast<IMS_SINT32>(m_nPcscfPort));

    return IMS_TRUE;
}

PROTECTED
AString AosRegistration::FeatureToString()
{
    AString strFeature = "| ";

    if (m_nFeature & FEATURE_SUBSCRIPTION)
    {
        strFeature += "FEATURE_SUBSCRIPTION | ";
    }

    if (m_nFeature & FEATURE_IPSEC)
    {
        strFeature += "FEATURE_IPSEC | ";
    }

    return strFeature;
}

PROTECTED
AosNetworkType AosRegistration::GetNetworkTypeForImsRegState() const
{
    if (m_pUtil->IsWifiTest())
    {
        return AosNetworkType::LTE;
    }

    switch (m_piContext->GetNetTracker()->GetNetworkType())
    {
        case NW_REPORT_RADIO_WLAN:
            return AosNetworkType::IWLAN;
        case NW_REPORT_RADIO_LTE:
            return AosNetworkType::LTE;
        case NW_REPORT_RADIO_NR:
            return AosNetworkType::NR;
        case NW_REPORT_RADIO_WCDMA:  // FALL-THROUGH
        case NW_REPORT_RADIO_HSPA:
            return AosNetworkType::UTRAN;

        default:
            return AosNetworkType::NONE;
    }
}

PROTECTED
AosReasonCode AosRegistration::GetReasonCode() const
{
    return m_eImsReasonCode;
}

PROTECTED
IMS_SINT32 AosRegistration::GetRegIpcanCategory() const
{
    return m_nRegIpcanCategory;
}

PROTECTED
IMS_UINT32 AosRegistration::GetRegFeatures()
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_UINT32 nFeatures = ImsAosFeature::NONE;

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (piHandle->GetRequestType() == IAosHandle::ATTACH && piHandle->IsRegBinded())
        {
            nFeatures |= (piHandle->GetBindedFeatureTagList().GetFeatures()) &
                    (~piHandle->GetBindedFeatureTagList().GetUnavailableFeatures());
        }
    }

    A_IMS_TRACE_I(REGID, "GetRegFeatures :: (%x)", nFeatures, 0, 0);

    return nFeatures;
}

PROTECTED
void AosRegistration::NotifyFailureWithImsReason(IN IMS_SINT32 nReason, IN IMS_SINT32 nStatusCode)
{
    if (GetImsRegType() != IMS_REG_TYPE_NORMAL)
    {
        return;
    }

    switch (nReason)
    {
        case IRegistration::REASON_TRANSACTION_TIMEOUT:  // FALL-THROUGH
        case IRegistration::REASON_CLIENT_SOCKET_ERROR:  // FALL-THROUGH
        case IRegistration::REASON_SERVER_SOCKET_ERROR:
            m_eImsReasonCode = AosReasonCode::REG_RESP_NETWORK_TIMEOUT;
            break;
        case IRegistration::REASON_STATUS_CODE:
            if (nStatusCode == SipStatusCode::SC_403)
            {
                m_eImsReasonCode = AosReasonCode::REG_RESP_403;
            }
            break;
        default:
            break;
    }
}

PROTECTED
void AosRegistration::NotifyDeregistered()
{
    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    IMS_SINT32 nImsRegType = GetImsRegType();
    if (piService != IMS_NULL && nImsRegType != IAosRegistration::IMS_REG_TYPE_INVALID)
    {
        A_IMS_TRACE_D(REGID, "NotifyDeregistered :: RegType(%d), ImsReasonCode(%d)", nImsRegType,
                m_eImsReasonCode, 0);
        piService->NotifyDeregistered(
                nImsRegType, GetNetworkTypeForImsRegState(), m_eImsReasonCode);
    }
}

PROTECTED
IMS_BOOL AosRegistration::UpdateCallingNumberVerification()
{
    if (!GET_N_CONFIG(m_nSlotId)->IsVerstatSupportedBasedOnNetworkForReg())
    {
        return IMS_FALSE;
    }

    if (m_piRegistration == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_bCallingNumberVerificationSupported =
            m_pUtil->IsParameterIncluded(m_piRegistration->GetPreviousResponse(),
                    ISipHeader::FEATURE_CAPS, AosString::STR_VERSTAT_FEATURE);

    A_IMS_TRACE_D(REGID, "Network supports verstat (%s)",
            _TRACE_B_(m_bCallingNumberVerificationSupported), 0, 0);

    return IMS_TRUE;
}

PROTECTED IMS_BOOL AosRegistration::IsPdnReconnectWithDelayRequiredOnWfcSetupFailure()
{
    IMS_SINT32 nPdnReconnectionDelay =
            GET_N_CONFIG(m_nSlotId)->GetPdnReconnectDelayOnWfcSetupFailAllPcscfsWithCsRoam();

    if (nPdnReconnectionDelay <= 0)
    {
        return IMS_FALSE;
    }

    if (!m_piContext->GetConnection()->IsEpdgEnabled())
    {
        return IMS_FALSE;
    }

    if (!m_piContext->GetNetTracker()->IsRoaming())
    {
        return IMS_FALSE;
    }

    if (m_pUtil->IsSupportedNetworkTypeForCellular(
                m_piContext->GetNetTracker()->GetMobileVoiceNetworkType()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
void AosRegistration::ProcessStopForPdnReconnectWithAwt()
{
    if (m_nState == STATE_REGISTERED || m_nState == STATE_REFRESHSTOP)
    {
        A_IMS_TRACE_D(REGID, "ProcessStopForPdnReconnectWithAwt :: Deregister", 0, 0, 0);

        StopSubscription();

        if (SendDeregister())
        {
            SetState(STATE_DEREGISTERING);
            PrepareTrafficForDeregister();
            m_pUtil->AddFeature(PENDING_PDN_RECONNECT_WITH_AWT, m_nTxnPending);

            /* NOTE: ReportTryingState() is shouldn't be called here.
             * AosApplication state is CONNECTING at this moment, changed by
             * AosApplication::ProcessRegFailed_NoNextPcscfOnScscfRestoration().
             * ReportTryingState() starts ImsEstablishmentTimer if App state is CONNECTING.
             * This is not the expected behavior in this scenario. */

            return;
        }
    }

    Destroy();
    ReconnectPdnWithDelayOnWfcSetupFail();
}

PROTECTED
void AosRegistration::PrepareTrafficForDeregister()
{
    IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);
    if (piTransaction != IMS_NULL)
    {
        if (piTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG))
        {
            SetTrafficForDeregister(IMS_TRUE);
            StartTimer(TIMER_DEREG_TRAFFIC, DEREGISTRATION_TRAFFIC_MAX_TIME * 1000);
        }
        else
        {
            SetTrafficPriorityBlocked(IMS_TRUE);
            m_pUtil->AddFeature(PENDING_STOP, m_nTxnPending);
        }
    }
}

PROTECTED
void AosRegistration::ReconnectPdnWithDelayOnWfcSetupFail()
{
    m_nPdnReactivateWaitTime =
            GET_N_CONFIG(m_nSlotId)->GetPdnReconnectDelayOnWfcSetupFailAllPcscfsWithCsRoam();
    A_IMS_TRACE_D(REGID, "ReconnectPdnWithDelayOnWfcSetupFail :: PDN reconnect with ATW(%d)",
            m_nPdnReactivateWaitTime, 0, 0);
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
}

PROTECTED
void AosRegistration::UpdateDetailState(IN IMS_UINT32 nState)
{
    IMS_SINT32 nImsRegState = IMS_REG_STATE_DEREGISTERED;

    switch (nState)
    {
        case STATE_REGISTERING:
            nImsRegState = IMS_REG_STATE_REGISTERING;
            break;
        case STATE_REGISTERED:
            nImsRegState = IMS_REG_STATE_REGISTERED;
            break;
        case STATE_OFFLINE:  // FALL-THROUGH
        case STATE_REGSTOP:
            nImsRegState = IMS_REG_STATE_DEREGISTERED;
            break;

        default:
            return;
    }

    IMS_UINT32 nImsRegFeatures = GetRegFeatures();
    AosNetworkType eImsRegNetwork = GetNetworkTypeForImsRegState();
    if (m_nImsRegState == nImsRegState)
    {
        if (m_nImsRegState == IMS_REG_STATE_DEREGISTERED)
        {
            return;
        }
        if (m_nImsRegFeatures == nImsRegFeatures && m_eImsRegNetwork == eImsRegNetwork)
        {
            return;
        }
    }

    AosReasonCode eReason = GetReasonCode();
    if (eReason == AosReasonCode::PLMN_BLOCK || eReason == AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT)
    {
        if (eImsRegNetwork == AosNetworkType::NONE)
        {
            eImsRegNetwork = m_eImsRegNetwork;
        }
    }

    m_nImsRegState = nImsRegState;
    m_nImsRegFeatures = nImsRegFeatures;
    m_eImsRegNetwork = eImsRegNetwork;

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    IMS_SINT32 nImsRegType = GetImsRegType();
    if (piService != IMS_NULL && nImsRegType != IAosRegistration::IMS_REG_TYPE_INVALID)
    {
        if (m_nImsRegState == IMS_REG_STATE_DEREGISTERED)
        {
            piService->NotifyDeregistered(nImsRegType, m_eImsRegNetwork, eReason);
            m_eImsReasonCode = AosReasonCode::UNSPECIFIED;
        }
        else if (m_nImsRegState == IMS_REG_STATE_REGISTERING)
        {
            ImsList<AString> objFeatureTags = ImsList<AString>();
            piService->NotifyRegistering(
                    nImsRegType, m_eImsRegNetwork, m_nImsRegFeatures, objFeatureTags);
        }
        else if (m_nImsRegState == IMS_REG_STATE_REGISTERED)
        {
            ImsList<AString> objFeatureTags = ImsList<AString>();
            piService->NotifyRegistered(
                    nImsRegType, m_eImsRegNetwork, m_nImsRegFeatures, objFeatureTags);
        }
    }
}

PROTECTED
void AosRegistration::UpdateStopRetryTimer(IN IMS_UINT32 nRetryTime)
{
    if (m_nState != STATE_REGSTOP || m_piStopRetryTimer == IMS_NULL)
    {
        return;
    }

    A_IMS_TRACE_D(REGID, "UpdateStopRetryTimer", 0, 0, 0);

    ClearRetryCount(IMS_TRUE);

    StartTimer(TIMER_STOP_RETRY, nRetryTime * 1000);
}

PROTECTED
void AosRegistration::ReinitiateRegistration(IN IMS_UINT32 nRetryAfterSec)
{
    A_IMS_TRACE_D(REGID, "ReinitiateRegistration :: RA = (%d)s", nRetryAfterSec, 0, 0);

    if (GetImsRegType() != IMS_REG_TYPE_NORMAL)
    {
        return;
    }

    DestroyEx();
    ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
    StartTimer(TIMER_OFFLINE_RECOVER, nRetryAfterSec * 1000);
}

PROTECTED
void AosRegistration::SetRetryTimeToProperty(IN IMS_UINT32 nSeconds)
{
    if (!GET_N_CONFIG(m_nSlotId)->IsCdmalessFeatureTagRequired())
    {
        return;
    }

    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return;
    }

    AString strName;
    AString strSecs;
    strName = (m_nSlotId == 0) ? AosString::STR_REG_RETRY_TIME0 : AosString::STR_REG_RETRY_TIME1;
    strSecs.SetNumber(nSeconds);

    UtilService::GetUtilService()->GetSystemProperty()->Set(strName, strSecs);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::OnMessage(IN IMSMSG& objMsg)
{
    A_IMS_TRACE_I(REGID, "OnMessage :: (%s)",
            AosProvider::GetLog()->RegMessageToString(objMsg.nMSG), 0, 0);

    switch (objMsg.nMSG)
    {
        case MSG_REG_REINITIATE:
            ProcessReinitiate();
            break;

        case MSG_REG_UPDATE:
            ProcessUpdatePending();
            break;

        case MSG_REG_RECONFIG:
            ProcessReconfigPending();
            break;

        case MSG_REG_REQUIRED_WITH_WAIT_TIME:
            ProcessRegRequiredWithWaitTime(static_cast<IMS_SINT32>(objMsg.nWparam));
            break;

        case MSG_REG_REQUIRED_WITH_NEXT_PCSCF:
            ProcessRegRequiredWithNextPcscf();
            break;

        case MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION:
            ProcessScscfRestoration(objMsg.nWparam);
            break;

        case MSG_REG_TERMINATED_BY_NOTIFY:
            ProcessRegTerminatedByNotify();
            break;

        case MSG_REG_FORBIDDEN_IN_WIFI:
            ProcessRegForbbidenInWifi();
            break;

        case MSG_SUB_REINITIATE:
            ProcessSubReinitiate();
            break;

        case MSG_SUB_TERMINATED:
            ProcessSubscription_Terminated(static_cast<IMS_SINT32>(objMsg.nWparam));
            break;

        case MSG_REG_EVENT_REGISTERED:
            ProcessRegEventRegistered();
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::Init()
{
    A_IMS_TRACE_D(REGID, "Init", 0, 0, 0);

    InitFeatures();

    m_piContext->GetBlock()->SetListener(this);

    if (IsCallStateRequired())
    {
        IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCt != IMS_NULL)
        {
            piCt->SetListener(this);
        }
    }

    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
        if (piService != IMS_NULL)
        {
            piService->AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        }
    }

    SetTrafficListener(IMS_TRUE);
}

PROTECTED VIRTUAL void AosRegistration::InitFeatures()
{
    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsSubscription())
        {
            m_pUtil->AddFeature(FEATURE_SUBSCRIPTION, m_nFeature);
        }
    }

    if (GET_N_CONFIG(m_nSlotId)->IsIpsecEnabled())
    {
        m_pUtil->AddFeature(FEATURE_IPSEC, m_nFeature);
    }

    A_IMS_TRACE_I(REGID, "InitFeature :: features (%s)", FeatureToString().GetStr(), 0, 0);
}

PROTECTED VIRTUAL void AosRegistration::CleanUp()
{
    A_IMS_TRACE_D(REGID, "CleanUp", 0, 0, 0);

    DestroyEx();

    StopTimer(TIMER_OFFLINE_RECOVER);

    SetTrafficListener(IMS_FALSE);

    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
        if (piService != IMS_NULL)
        {
            piService->RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        }
    }

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->RemoveListener(this);
    }

    IAosBlock* piBlock = m_piContext->GetBlock();
    if (piBlock != IMS_NULL)
    {
        piBlock->RemoveListener(this);
    }
}

PROTECTED VIRTUAL void AosRegistration::DestroyEx()
{
    SetContactAddressConfiguration(IMS_FALSE);
    ClearRegParameters(IMS_FALSE);
    DestroyRegistration();
    SetState(STATE_OFFLINE);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::IsGeolocationInfoRequired()
{
    IMS_BOOL bRequired = IMS_FALSE;

    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
        {
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() &&
                    GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN)
            {
                if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfInWfcInitReg())
                {
                    return IMS_TRUE;
                }
                else
                {
                    bRequired = IMS_TRUE;
                }
            }
        }

        if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
        {
            if (GetRegIpcanCategory() == IIpcan::CATEGORY_MOBILE)
            {
                bRequired = IMS_TRUE;
            }
        }
    }
    else if (m_eRegType == AosRegistrationType::EMERGENCY)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
        {
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() &&
                    GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN)
            {
                if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfInWfcInitReg())
                {
                    return IMS_TRUE;
                }
                else
                {
                    bRequired = IMS_TRUE;
                }
            }
        }

        if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
        {
            if (GetRegIpcanCategory() == IIpcan::CATEGORY_MOBILE)
            {
                bRequired = IMS_TRUE;
            }
        }
    }
    else
    {
        return IMS_FALSE;
    }

    /*
    IAosLocationStarter* piLs = AosProvider::GetInstance()->GetLocationStarter(m_nSlotId);
    if (piLs != IMS_NULL &&
            !piLs->IsPolicyEnabled(IAosLocationStarter::POLICY_START_ON_VOLTE_AVAILABLE))
    {
        if (GetRegIpcanCategory() == IIpcan::CATEGORY_MOBILE)
        {
            return IMS_FALSE;
        }
    }
    */

    if (!bRequired)
    {
        return IMS_FALSE;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsIpsecEnabled())
    {
        if (m_pIpsecHelper == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (m_pIpsecHelper->IsEstablished() == IMS_FALSE)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::IsHandlingServerSocketErrorRequired(
        IN IMS_SINT32 nReason)
{
    if ((nReason == IRegistration::REASON_SERVER_SOCKET_ERROR) && IsRegistered() &&
            (m_eRegType == AosRegistrationType::NORMAL))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ReportStateChanged(
        IN IMS_UINT32 nResult, IN IMS_UINT32 nReason /* = 0 */)
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->Registration_StateChanged(nResult, nReason);
    }
}

PROTECTED VIRTUAL void AosRegistration::ReportTryingState()
{
    IMS_UINT32 nReasonEx = REASON_TRYING_START;

    switch (m_nState)
    {
        case STATE_REFRESHING:
            nReasonEx = REASON_TRYING_UPDATE;
            break;

        case STATE_DEREGISTERING:
            nReasonEx = REASON_TRYING_STOP;
            break;

        default:
            break;
    }

    ReportStateChanged(RESULT_TRYING, nReasonEx);
}

PRIVATE
void AosRegistration::PrepareRegistration()
{
    A_IMS_TRACE_I(REGID, "PrepareRegistration", 0, 0, 0);

    if (m_piRegistration != IMS_NULL &&
            m_piRegManager->GetRegistration(m_nSlotId, m_nFlowId) != IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "PrepareRegistration :: destroy registration for creating", 0, 0, 0);

        ImsList<IRegContact*> objContactList = m_piRegistration->GetAllContacts();

        for (IMS_UINT32 nAt = 0; nAt < objContactList.GetSize(); ++nAt)
        {
            m_piRegistration->DestroyContact(objContactList.GetAt(nAt));
        }

        m_piRegManager->DestroyRegistration(m_piRegistration);
        m_piRegistration = IMS_NULL;

        DestroySubscription();
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::CreateRegistration()
{
    PrepareRegistration();

    SetRetryTime();

    if (!SetAor())
    {
        return IMS_FALSE;
    }

    m_piRegistration = GetRegistration();

    if (m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piRegistration->SetListener(this);

    m_piRegistration->SetSipMessageMediator(this);

    SetRefreshPolicy();

    if (!SetPcscf())
    {
        A_IMS_TRACE_I(REGID, "pcscf is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    IAosConnection* piConnection = m_piContext->GetConnection();

    // select local IP version based on PCSCF IP version
    IpAddress objPcscf(m_strPcscf);
    m_objIpa = piConnection->GetLocalAddress(
            (objPcscf.IsIPv6Address()) ? IpAddress::IPV6 : IpAddress::IPV4);

    CreateContact();

    if (m_piRegContact == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "contact is null", 0, 0, 0);
        return IMS_FALSE;
    }

    StartRegBinding();

    m_piRegParameter = m_piRegistration->GetParameter();

    const IAosSubscriber* pSubscriber = m_piContext->GetSubscriber();
    if (GET_N_CONFIG(m_nSlotId)->IsGibaSupportedForERegInRoaming() && pSubscriber != IMS_NULL &&
            pSubscriber->HasValidTemporaryPublicUserIdForGiba())
    {
        A_IMS_TRACE_D(REGID, "Temporary PUID for GIBA has been set", 0, 0, 0);
        m_piRegistration->SetUserIdentityNotifier(this);
        m_piRegParameter->SetAuthenticationCredentials(IMS_FALSE);
    }

    AddSpecificOperation();

    if (UpdatePreloadedRoute() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    SetStaticIpQos();

    SetActiveBindingsRestorationUsage();

    if (IsIpsecSupported())
    {
        CreateIpsecHelper();
    }

    SetTcpCriterionLength();

    return SendRegister(IMS_FALSE, IMS_TRUE);
}

PROTECTED VIRTUAL void AosRegistration::DestroyRegistration()
{
    A_IMS_TRACE_I(REGID, "DestroyRegistration", 0, 0, 0);

    if (GetState() == STATE_DEREGISTERING &&
            m_pUtil->IsFeatureOn(PENDING_PDN_RECONNECT_WITH_AWT, m_nTxnPending))
    {
        ReconnectPdnWithDelayOnWfcSetupFail();
    }

    ClearAuthChallengedCount();
    ClearPending();
    ClearNetworkBindingFeatures();

    SetRadioWaiting(IMS_FALSE);
    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        UpdateTransactionStarted();
    }
    SetTraffic(IMS_FALSE);
    SetReregFailureReportOnIpcanChangeRequired(IMS_FALSE);

    DestroyIpsecHelper();

    if (m_pSubscription != IMS_NULL)
    {
        DestroySubscription();
    }

    if (m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "reg is already destroyed", 0, 0, 0);
        return;
    }

    m_piRegistration->DestroyContact(m_piRegContact);
    m_piRegistration->SetListener(IMS_NULL);
    m_piRegManager->DestroyRegistration(m_piRegistration);

    m_piRegistration = IMS_NULL;
    m_piRegContact = IMS_NULL;
    m_piRegParameter = IMS_NULL;
}

PROTECTED VIRTUAL IRegistration* AosRegistration::GetRegistration()
{
    A_IMS_TRACE_D(REGID, "GetRegistration :: m_nFlowId (%d), strAoR (%s)", m_nFlowId,
            m_strPuid.GetStr(), 0);

    IMS_BOOL bEmergency = (m_eRegType == AosRegistrationType::EMERGENCY ||
            m_eRegType == AosRegistrationType::FAKE);

    if (!m_piRegManager->CreateRegistration(
                m_nSlotId, m_nFlowId, m_strPuid, bEmergency, IsFakeRegistration()))
    {
        A_IMS_TRACE_I(REGID, "create reg is failed", 0, 0, 0);
        return IMS_NULL;
    }

    return m_piRegManager->GetRegistration(m_nSlotId, m_nFlowId);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::StartRegBinding()
{
    // attach services (add and create binding)
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    AString strLog;

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (piHandle->GetRequestType() == IAosHandle::ATTACH)
        {
            m_piRegistration->CreateBinding(piHandle->GetAppId(), piHandle->GetServiceId());
            m_piRegContact->AddService(piHandle->GetAppId(), piHandle->GetServiceId());

            piHandle->SetRegBinded(IMS_TRUE);
            AddFeatureTag(piHandle);
        }
        else
        {
            piHandle->SetRegBinded(IMS_FALSE);
        }

        piHandle->SetNetworkRegBinded(IMS_TRUE);

        strLog.Append("[");
        strLog.Append(piHandle->GetAppId().GetStr());
        strLog.Append("]");
        strLog.Append("[");
        strLog.Append(piHandle->GetServiceId().GetStr());
        strLog.Append("]");
        strLog.Append("[request/");
        strLog.Append((piHandle->GetRequestType()) ? "ATTACH" : "DETACH");
        strLog.Append("]");
        strLog.Append("[regbinded/");
        strLog.Append((piHandle->IsRegBinded()) ? "ATTACHED" : "DETACHED");
        strLog.Append("]\n");
    }

    A_IMS_TRACE_I(REGID, "StartRegBinding :: %s", strLog.GetStr(), 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::UpdateRegBinding()
{
    if (m_piRegContact == IMS_NULL || m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "UpdateRegBinding :: reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_BOOL bChanged = IMS_FALSE;
    AString strLog;

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        strLog.Append("[");
        strLog.Append(piHandle->GetAppId().GetStr());
        strLog.Append("]");
        strLog.Append("[");
        strLog.Append(piHandle->GetServiceId().GetStr());
        strLog.Append("]");
        strLog.Append("[request/");
        strLog.Append((piHandle->GetRequestType()) ? "ATTACH" : "DETACH");
        strLog.Append("]\n");

        if (piHandle->IsRegBinded())
        {
            if (piHandle->GetRequestType() == IAosHandle::DETACH)
            {
                m_piRegContact->RemoveService(piHandle->GetAppId(), piHandle->GetServiceId());
                m_piRegistration->DestroyBinding(piHandle->GetAppId(), piHandle->GetServiceId());

                piHandle->SetRegBinded(IMS_FALSE);
                RemoveFeatureTag(piHandle);
                bChanged = IMS_TRUE;
            }
            else  // IAosHandle::ATTACH
            {
                if (UpdateFeatureTag(piHandle))
                {
                    bChanged = IMS_TRUE;
                }
            }
        }
        else
        {
            if (piHandle->GetRequestType() == IAosHandle::ATTACH)
            {
                if (IsNetworkBindingSupported(piHandle) && !piHandle->IsNetworkRegBinded())
                {
                    A_IMS_TRACE_I(REGID,
                            "UpdateRegBinding :: service(%d) is not binded "
                            "by network",
                            piHandle->GetServiceType(), 0, 0);
                    continue;
                }

                m_piRegistration->CreateBinding(piHandle->GetAppId(), piHandle->GetServiceId());
                m_piRegContact->AddService(piHandle->GetAppId(), piHandle->GetServiceId());

                piHandle->SetRegBinded(IMS_TRUE);
                AddFeatureTag(piHandle);
                bChanged = IMS_TRUE;
            }
        }
    }

    A_IMS_TRACE_I(REGID, "UpdateRegBinding :: %s", strLog.GetStr(), 0, 0);

    if (bChanged)
    {
        if (IsRegistered())
        {
            IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
            IMS_SINT32 nImsRegType = GetImsRegType();
            if (piService != IMS_NULL && nImsRegType == IAosRegistration::IMS_REG_TYPE_NORMAL)
            {
                piService->NotifyImsFeatureChanged(
                        nImsRegType, GetNetworkTypeForImsRegState(), GetRegFeatures());
            }
        }

        UpdateFinalAddFeatureTag();
        m_piRegContact->RecalculateCallerCapabilities();
    }

    return bChanged;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::UpdateNetworkRegBinding()
{
    return IMS_FALSE;
    // TODO : update the capabilities based on network ones
#if 0
    if (m_piContext->GetConfig()->GetNetworkRegBindingSupportedServices()
            == ImsAosService::NONE)
    {
        return IMS_FALSE;
    }

    if (m_piRegContact == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "UpdateNetworkRegBinding :: reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_BOOL bChanged = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (!IsNetworkBindingSupported(piHandle))
        {
            continue;
        }

        if (piHandle->IsRegBinded())
        {
            if (piHandle->GetRequestType() == IAosHandle::ATTACH)
            {
                if (m_piRegContact->IsServiceRegistered(
                        piHandle->GetAppId(), piHandle->GetServiceId()))
                {
                    continue;
                }

                m_piRegContact->RemoveService(piHandle->GetAppId(), piHandle->GetServiceId());
                m_piRegistration->DestroyBinding(piHandle->GetAppId(), piHandle->GetServiceId());

                piHandle->SetRegBinded(IMS_FALSE);
                piHandle->SetNetworkRegBinded(IMS_FALSE);
                RemoveFeatureTag(piHandle);
                bChanged = IMS_TRUE;
                A_IMS_TRACE_I(REGID,
                        "UpdateNetworkRegBinding :: service (%d) is not binded by network",
                        piHandle->GetServiceType(), 0, 0);
            }
        }
    }

    if (bChanged)
    {
        UpdateFinalAddFeatureTag();
        m_piRegContact->RecalculateCallerCapabilities();
    }

    return bChanged;
#endif
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::UpdateNetworkRegFeatureBinding()
{
    return IMS_FALSE;

#if 0  // TODO : update impl.
    if (m_piContext->GetConfig()->GetNetworkRegFeatureBindingSupportedServices()
            == ImsAosService::NONE)
    {
        return IMS_FALSE;
    }

    if (m_piRegContact == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "UpdateNetworkRegFeatureBinding :: reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (!IsNetworkFeatureBindingSupported(piHandle))
        {
            continue;
        }

        if (piHandle->IsRegBinded())
        {
            if (piHandle->GetRequestType() == IAosHandle::ATTACH)
            {
                AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
                for (IMS_UINT32 nAt = 0; nAt < objBindedList.GetSize(); ++nAt)
                {
                    AosFeatureTag* pFeature = objBindedList.GetAt(nAt);

                    if ((pFeature->GetType() != 0) && (pFeature->GetName().GetLength() > 0))
                    {
                        if (m_piRegContact->IsFeatureRegistered(
                                pFeature->GetName(), pFeature->GetValue()))
                        {
                            A_IMS_TRACE_D(REGID, "UpdateNetworkRegFeatureBinding :: (%d) %s = %s",
                                    pFeature->GetType(), pFeature->GetName().GetStr(),
                                    pFeature->GetValue().GetStr());

                            m_nNetworkBindingFeatures |= pFeature->GetType();
                        }
                    }
                }
            }
        }
    }

    return IMS_TRUE;
#endif
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::Register(IN IMS_SINT32 nMinExpireValue)
{
    if (nMinExpireValue > 0)
    {
        return (m_piRegistration->Register(nMinExpireValue) == IMS_FAILURE) ? IMS_FALSE : IMS_TRUE;
    }

    return (m_piRegistration->Register() == IMS_FAILURE) ? IMS_FALSE : IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SendRegister(
        IN IMS_BOOL bRestore /* = IMS_FALSE */, IN IMS_BOOL bInitial /* = IMS_FALSE */)
{
    A_IMS_TRACE_I(REGID, "SendRegister", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    SetContactAddressConfiguration(IMS_FALSE);

    if (bRestore)
    {
        m_piRegistration->Restore();
        if (IsIpsecSupported())
        {
            UpdatePreloadedRoute();
        }
    }

    UpdateRegIpcanCategory();

    SetDynamicIpQos();

    AddOperation_OnSendRegister();

    if (m_pIpsecHelper != IMS_NULL && IsIpsecSupported())
    {
        m_pIpsecHelper->Create(bInitial || bRestore);
    }

    if (!Register(-1))
    {
        A_IMS_TRACE_I(REGID, "register is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    SetContactAddressConfiguration(IMS_TRUE);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SendRegisterEx(
        IN IMS_SINT32 nMinExpireValue, IN IMS_BOOL bAddHalfExpireValue /* = IMS_FALSE */)
{
    A_IMS_TRACE_I(REGID, "SendRegister :: min expire (%d)", nMinExpireValue, 0, 0);

    IMS_SINT32 nExpireValue = nMinExpireValue;

    if (bAddHalfExpireValue)
    {
        nExpireValue += nMinExpireValue / 2;
    }

    if (m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    UpdateRegIpcanCategory();

    SetDynamicIpQos();

    AddOperation_OnSendRegister();

    if (!Register(nExpireValue))
    {
        A_IMS_TRACE_I(REGID, "register is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessStop()
{
    A_IMS_TRACE_I(REGID, "ProcessStop", 0, 0, 0);

    StopSubscription();

    if (!SendDeregister())
    {
        Destroy();
        ReportStateChanged(RESULT_SUCCESS);
        return IMS_FALSE;
    }
    SetState(STATE_DEREGISTERING);

    PrepareTrafficForDeregister();

    ReportTryingState();
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SendDeregister()
{
    A_IMS_TRACE_I(REGID, "SendDeregister", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    UpdateRegIpcanCategory();

    if (AddOperation_OnSendDeregister() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (m_pIpsecHelper != IMS_NULL && IsIpsecSupported())
    {
        m_pIpsecHelper->Create(IMS_FALSE);
    }

    if (m_piRegistration->Deregister() == IMS_FAILURE)
    {
        A_IMS_TRACE_I(REGID, "deregister is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::AddOperation_OnSendRegister()
{
    if (m_piRegContact == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_piRegContact->RemoveHeaderParameter(AosString::STR_ACCESS_TYPE_FEATURE);
    AddAccesstypeFeatureTag();

    ControlPrivateHeader();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::AddOperation_OnSendDeregister()
{
    ControlPrivateHeader();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::AddOperation_OnNotifyAkaResponse()
{
    ControlPrivateHeader();

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::CreateContact()
{
    m_piRegContact = m_piRegistration->CreateContact(m_objIpa, m_pUtil->GetLocalPort(m_nSlotId));
}

PROTECTED VIRTUAL void AosRegistration::AddSpecificOperation()
{
    if (m_eRegType == AosRegistrationType::EMERGENCY)
    {
        m_piRegContact->AddUriParameter("sos");

        if (GET_N_CONFIG(m_nSlotId)->IsERegWithOnlyTcpInRoaming() && m_piRegParameter != IMS_NULL &&
                m_piContext->GetNetTracker()->IsRoaming())
        {
            m_piRegParameter->SetTransportExt(Sip::TRANSPORT_EXT_TCP);
        }
    }

    if (GET_N_CONFIG(m_nSlotId)->IsTcpRequiredForReg() && m_piRegParameter != IMS_NULL)
    {
        m_piRegParameter->SetTransportExtForRegOnly(Sip::TRANSPORT_EXT_TCP);
    }

    if (!GET_N_CONFIG(m_nSlotId)->IsSipOverIpsecInRoamingEnabled())
    {
        if (m_piContext->GetNetTracker()->IsRoaming())
        {
            UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_ROAMING);
        }
        else
        {
            UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_ROAMING);
        }
    }

    AddAccesstypeFeatureTag();
}

PROTECTED VIRTUAL void AosRegistration::AddAccesstypeFeatureTag()
{
    IMS_SINT32 nAccessType =
            GET_N_CONFIG(m_nSlotId)->GetRegistrationPreferredAccessTypeFeatureTag();

    if (nAccessType == CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED)
    {
        return;
    }

    AString strValue;

    if (nAccessType == CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED)
    {
        strValue = (m_piContext->GetConnection()->IsEpdgEnabled())
                ? AosString::STR_ACCESS_TYPE_WLAN1
                : AosString::STR_ACCESS_TYPE_CELLULAR2;
    }
    else  // CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE
    {
        strValue = (m_piContext->GetConnection()->IsEpdgEnabled())
                ? AosString::STR_ACCESS_TYPE_WLAN
                : AosString::STR_ACCESS_TYPE_CELLULAR;
    }

    m_piRegContact->AddHeaderParameter(AosString::STR_ACCESS_TYPE_FEATURE, strValue);
}

PROTECTED VIRTUAL void AosRegistration::AddFeatureTag(IN IAosHandle* piHandle)
{
    AosFeatureTagList& objList = piHandle->GetFeatureTagList();
    if (objList.GetSize() > 0)
    {
        for (IMS_UINT32 nAt = 0; nAt < objList.GetSize(); ++nAt)
        {
            AosFeatureTag* pFeature = objList.GetAt(nAt);

            if (IsExtraCapabilityRequiredForFeatureTag(pFeature->GetOption()))
            {
                m_piRegContact->AddExtraCapability(pFeature->GetName(), pFeature->GetValue());
            }
            else
            {
                m_piRegContact->AddHeaderParameter(pFeature->GetName(), pFeature->GetValue());
            }
        }

        AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
        objBindedList.Copy(objList);
        objBindedList.PrintFeatureTagList();
    }
    else
    {
        AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
        objBindedList.Clear();
        objBindedList.CopyFeatures(objList);
    }

    if (piHandle->GetServiceType() == ImsAosService::MTC ||
            piHandle->GetServiceType() == ImsAosService::EMERGENCY_MTC)
    {
        AddFeatureTagForMtc(piHandle->GetBindedFeatureTagList().GetFeatures(), IMS_FALSE);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::AddFeatureTagForMtc(
        IN IMS_UINT32 nRegFeatures, IN IMS_BOOL bFinalFeatureTag)
{
    // If bFinalFeatureTag is true, the extra header shouldn't be added
    // because it manages as reference count

    IMS_BOOL bFeatureTagUpdated = IMS_FALSE;

    if (m_pUtil->UpdateFeatureTagOptions(ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO,
                (nRegFeatures & ImsAosFeature::VIDEO) > 0, m_nSlotId))
    {
        bFeatureTagUpdated = IMS_TRUE;
    }

    if (m_pUtil->UpdateFeatureTagOptions(ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT,
                (nRegFeatures & ImsAosFeature::TEXT) > 0, m_nSlotId))
    {
        bFeatureTagUpdated = IMS_TRUE;
    }

    if (((nRegFeatures & ImsAosFeature::NW_INIT_USSI) > 0) && bFinalFeatureTag == IMS_FALSE)
    {
        m_piRegContact->AddExtraCapability(
                AosString::STR_NW_INIT_USSI_FEATURE, AString::ConstNull());
    }

    if (((nRegFeatures & ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY) > 0) && !bFinalFeatureTag)
    {
        m_piRegContact->AddExtraCapability(
                FeatureTags::CALL_COMPOSER_VIA_TELEPHONY, AString::ConstNull());
    }

    if ((nRegFeatures & ImsAosFeature::VERSTAT) > 0)
    {
        m_piRegContact->AddHeaderParameter(AosString::STR_VERSTAT_FEATURE);
    }

    if (bFeatureTagUpdated)
    {
        m_piRegContact->RecalculateCallerCapabilities();
    }

    return bFeatureTagUpdated;
}

PROTECTED VIRTUAL void AosRegistration::RemoveFeatureTag(IN IAosHandle* piHandle)
{
    AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
    if (objBindedList.GetSize() > 0)
    {
        for (IMS_UINT32 nAt = 0; nAt < objBindedList.GetSize(); ++nAt)
        {
            AosFeatureTag* pFeature = objBindedList.GetAt(nAt);

            if (IsExtraCapabilityRequiredForFeatureTag(pFeature->GetOption()))
            {
                m_piRegContact->RemoveExtraCapability(pFeature->GetName(), pFeature->GetValue());
            }
            else
            {
                m_piRegContact->RemoveHeaderParameter(pFeature->GetName(), pFeature->GetValue());
            }
        }
    }

    if (piHandle->GetServiceType() == ImsAosService::MTC ||
            piHandle->GetServiceType() == ImsAosService::EMERGENCY_MTC)
    {
        RemoveFeatureTagForMtc(objBindedList.GetFeatures());
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::RemoveFeatureTagForMtc(IN IMS_UINT32 nRegFeatures)
{
    IMS_BOOL bFeatureTagUpdated = IMS_FALSE;

    if ((nRegFeatures & ImsAosFeature::VIDEO) > 0)
    {
        if (m_pUtil->UpdateFeatureTagOptions(
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO, IMS_FALSE, m_nSlotId))
        {
            bFeatureTagUpdated = IMS_TRUE;
        }
    }
    if ((nRegFeatures & ImsAosFeature::TEXT) > 0)
    {
        if (m_pUtil->UpdateFeatureTagOptions(
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT, IMS_FALSE, m_nSlotId))
        {
            bFeatureTagUpdated = IMS_TRUE;
        }
    }
    if ((nRegFeatures & ImsAosFeature::NW_INIT_USSI) > 0)
    {
        m_piRegContact->RemoveExtraCapability(
                AosString::STR_NW_INIT_USSI_FEATURE, AString::ConstNull());
    }
    if ((nRegFeatures & ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY) > 0)
    {
        m_piRegContact->RemoveExtraCapability(
                FeatureTags::CALL_COMPOSER_VIA_TELEPHONY, AString::ConstNull());
    }
    if ((nRegFeatures & ImsAosFeature::VERSTAT) > 0)
    {
        m_piRegContact->RemoveHeaderParameter(AosString::STR_VERSTAT_FEATURE);
    }

    return bFeatureTagUpdated;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::UpdateFeatureTag(IN IAosHandle* piHandle)
{
    // check if same or not
    AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
    AosFeatureTagList& objList = piHandle->GetFeatureTagList();

    if (objBindedList.Equals(objList))
    {
        return IMS_FALSE;
    }

    RemoveFeatureTag(piHandle);
    AddFeatureTag(piHandle);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::UpdateFinalAddFeatureTag()
{
    A_IMS_TRACE_I(REGID, "UpdateFinalAddFeatureTag", 0, 0, 0);

    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);
        if (piHandle->GetRequestType() == IAosHandle::ATTACH)
        {
            AosFeatureTagList& objBindedList = piHandle->GetBindedFeatureTagList();
            if (objBindedList.GetSize() > 0)
            {
                if (IsNetworkBindingSupported(piHandle) && !piHandle->IsNetworkRegBinded())
                {
                    A_IMS_TRACE_I(REGID,
                            "UpdateFinalAddFeatureTag :: service (%d) is not binded by network",
                            piHandle->GetServiceType(), 0, 0);
                    continue;
                }

                for (IMS_UINT32 nAt = 0; nAt < objBindedList.GetSize(); ++nAt)
                {
                    AosFeatureTag* pFeature = objBindedList.GetAt(nAt);

                    if (IsExtraCapabilityRequiredForFeatureTag(pFeature->GetOption()))
                    {
                        // should not add extra header because it manages as reference count
                    }
                    else
                    {
                        m_piRegContact->AddHeaderParameter(
                                pFeature->GetName(), pFeature->GetValue());
                    }
                }
            }
            if (piHandle->GetServiceType() == ImsAosService::MTC ||
                    piHandle->GetServiceType() == ImsAosService::EMERGENCY_MTC)
            {
                AddFeatureTagForMtc(objBindedList.GetFeatures(), IMS_TRUE);
            }
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SetAor()
{
    const IAosSubscriber* pSubscriber = m_piContext->GetSubscriber();

    AStringArray objImpu;

    if (IsFakeRegistration() == IMS_FALSE)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsGibaSupportedForERegInRoaming() && pSubscriber != IMS_NULL &&
                pSubscriber->HasValidTemporaryPublicUserIdForGiba())
        {
            A_IMS_TRACE_D(REGID, "SetAor :: Temporary PUID for GIBA has been set", 0, 0, 0);
            objImpu.AddElement(pSubscriber->GetTemporaryPublicUserIdForGiba());
        }
        else
        {
            objImpu = pSubscriber->GetConfiguredImpus();
        }
    }
    else if (!GET_N_CONFIG(m_nSlotId)->IsEmergencyCallBasedOnPauOfNormalRegistrationSupported())
    {
        objImpu = pSubscriber->GetFakeImpus();
    }
    else
    {
        IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
        if (piRsm == IMS_NULL || piRsm->GetImsRegState() != IMS_REG_ON)
        {
            objImpu = pSubscriber->GetFakeImpus();
        }
        else
        {
            A_IMS_TRACE_D(REGID, "SetAor :: GetAssociatedUris from normal registration", 0, 0, 0);
            const IRegistration* piRegistration = m_piRegManager->GetRegistration(
                    m_nSlotId, static_cast<IMS_UINT32>(AosRegistrationFlowId::NORMAL));
            objImpu = (piRegistration == IMS_NULL) ? pSubscriber->GetFakeImpus()
                                                   : piRegistration->GetAssociatedUris();
        }
    }

    if (objImpu.IsEmpty())
    {
        A_IMS_TRACE_I(REGID, "SetAor :: No Puids", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objImpu.GetCount() > 1)
    {
        if (IsNeedToSetLimitedMode() && m_piContext->GetConnection()->IsLimitedServicePcoValue())
        {
            m_strPuid = objImpu.GetElementAt(1);
            SetMode(MODE_LIMITED);
        }
        else
        {
            m_strPuid = objImpu.GetElementAt(0);
            if (m_eRegType == AosRegistrationType::NORMAL)
            {
                SetMode(MODE_NORMAL);
            }
        }
    }
    else
    {
        m_strPuid = objImpu.GetElementAt(0);
        if (IsNeedToSetLimitedMode())
        {
            SetMode(MODE_LIMITED);
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SetPcscf()
{
    IAosPcscf* piPcscf = m_piContext->GetPcscf();
    IMS_SINT32 nPcscfIndex = piPcscf->GetCurrentIndex();

    if (!piPcscf->HasPcscf(nPcscfIndex))
    {
        ClearPcscf();
        nPcscfIndex = piPcscf->GetCurrentIndex();
        if (!piPcscf->HasPcscf(nPcscfIndex))
        {
            A_IMS_TRACE_I(REGID, "SetPcscf :: invalid index", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    const AStringArray& objPcscfs = piPcscf->GetPcscfs();
    const ImsList<IMS_SINT32>& objPcscfPorts = piPcscf->GetPcscfsPorts();

    m_strPcscf = objPcscfs.GetElementAt(nPcscfIndex);
    m_nPcscfPort = objPcscfPorts.GetAt(nPcscfIndex);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::SetRefreshPolicy()
{
    m_piRegistration->SetRefreshPolicy(IRegistration::REFRESH_POLICY_SPEC, 1200, 50, 600);
}

PROTECTED VIRTUAL void AosRegistration::SetFailureState()
{
    SetState((GetState() == STATE_REFRESHING) ? STATE_REFRESHSTOP : STATE_REGSTOP);
}

PROTECTED VIRTUAL void AosRegistration::SetRetryState()
{
    SetState((GetState() == STATE_REFRESHSTOP) ? STATE_REFRESHING : STATE_REGISTERING);
}

PROTECTED VIRTUAL void AosRegistration::SetTcpCriterionLength()
{
    if (GET_N_CONFIG(m_nSlotId)->GetSipPreferredTransport() !=
            CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP)
    {
        return;
    }

    const IMS_SINT32 nMtu = m_piContext->GetConnection()->GetMtu();
    const IMS_SINT32 nSipThresholdSize =
            GET_N_CONFIG(m_nSlotId)->GetSipMessageThresholdForTransportChange();

    IMS_SINT32 nLength = 0;

    if (nMtu > 0)
    {
        if (nMtu > nSipThresholdSize)
        {
            nLength = nMtu - nSipThresholdSize;
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() && m_objIpa.IsIPv6Address())
            {
                if (nMtu > MTU_MAX_SIZE_VIA_WIFI)
                {
                    nLength = MTU_MAX_SIZE_VIA_WIFI - nSipThresholdSize;
                }
            }
        }
    }
    else
    {
        nLength = (m_objIpa.IsIPv6Address()) ? GET_N_CONFIG(m_nSlotId)->GetIpv6MtuSize()
                                             : GET_N_CONFIG(m_nSlotId)->GetIpv4MtuSize();
    }

    if (nLength <= 0)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() && m_objIpa.IsIPv6Address())
        {
            nLength = MTU_MAX_SIZE_VIA_WIFI - DEFAULT_SIP_THRESHOLD_SIZE;
        }
        else
        {
            nLength = MTU_MAX_SIZE_VIA_MOBILE - DEFAULT_SIP_THRESHOLD_SIZE;
        }
    }

    A_IMS_TRACE_I(REGID, "SetTcpCriterionLength :: TCP length (%d), MTU (%d), Threshold (%d)",
            nLength, nMtu, nSipThresholdSize);

    if (m_pSipProfile.IsNull())
    {
        m_pSipProfile = new SipProfile();
    }

    m_pSipProfile->SetTcpCriterionLength(nLength);
    m_piRegistration->SetSipProfile(m_pSipProfile.Get());
}

PROTECTED VIRTUAL void AosRegistration::SetStaticIpQos()
{
    IMS_SINT32 nPreferredImsDscp = GET_N_CONFIG(m_nSlotId)->GetPreferredImsDscp();
    if (nPreferredImsDscp == CarrierConfig::Ims::PREFERRED_DSCP_NONE)
    {
        return;
    }

    IMS_UINT32 nDscp = GET_N_CONFIG(m_nSlotId)->GetImsSignallingDscp();
    if (nDscp == 0)
    {
        return;
    }

    IMS_BOOL bWlan = m_piContext->GetConnection()->IsEpdgEnabled();
    if (bWlan)
    {
        if (nPreferredImsDscp == CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR)
        {
            return;
        }
    }
    else
    {
        if (nPreferredImsDscp == CarrierConfig::Ims::PREFERRED_DSCP_WIFI)
        {
            return;
        }
    }

    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(m_nSlotId);
    if (piRtConfigHelper != IMS_NULL)
    {
        piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_IP_QOS, IMS_NULL);

        SipRtConfig::IpQos objIpQos;
        A_IMS_TRACE_I(REGID, "SetStaticIpQos : Set DSCP to %d", nDscp, 0, 0);

        objIpQos.nValue = nDscp << 2;
        objIpQos.objIpAddr = m_objIpa;
        objIpQos.nPort = 0;

        piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_IP_QOS, &objIpQos);
    }
}

PROTECTED VIRTUAL void AosRegistration::SetDynamicIpQos()
{
    IMS_SINT32 nPreferredImsDscp = GET_N_CONFIG(m_nSlotId)->GetPreferredImsDscp();

    if (nPreferredImsDscp == CarrierConfig::Ims::PREFERRED_DSCP_NONE)
    {
        return;
    }

    IMS_UINT32 nDscp = GET_N_CONFIG(m_nSlotId)->GetImsSignallingDscp();
    if (nDscp == 0)
    {
        return;
    }

    IMS_BOOL bWlan = m_piContext->GetConnection()->IsEpdgEnabled();
    IMS_BOOL bSetDscp = IMS_TRUE;
    if (bWlan)
    {
        if (nPreferredImsDscp == CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR)
        {
            bSetDscp = IMS_FALSE;
        }
    }
    else
    {
        if (nPreferredImsDscp == CarrierConfig::Ims::PREFERRED_DSCP_WIFI)
        {
            bSetDscp = IMS_FALSE;
        }
    }

    ISipTransportHelper* piHelper = SipFactory::GetTransportHelper(m_nSlotId);
    if (piHelper != IMS_NULL)
    {
        SipRtConfig::IpQos objIpQos;
        if (bSetDscp)
        {
            A_IMS_TRACE_I(REGID, "SetDynamicIpQos : Set DSCP to %d", nDscp, 0, 0);
            objIpQos.nValue = nDscp << 2;
        }
        else
        {
            A_IMS_TRACE_I(REGID, "SetDynamicIpQos : Set DSCP to 0", 0, 0, 0);
            objIpQos.nValue = 0;
        }

        objIpQos.objIpAddr = m_objIpa;
        objIpQos.nPort = 0;

        piHelper->SetIpQos(&objIpQos);
    }
}

PROTECTED VIRTUAL void AosRegistration::SetActiveBindingsRestorationUsage()
{
    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return;
    }

    if (m_piRegistration != IMS_NULL)
    {
        m_piRegistration->SetActiveBindingsRestorationUsage(IMS_TRUE);
    }
}

PROTECTED VIRTUAL void AosRegistration::SetReregFailureReportOnIpcanChangeRequired(
        IN IMS_BOOL bRequired)
{
    m_bIsReregFailureReportOnIpcanChangeRequired = bRequired;
}

PROTECTED VIRTUAL void AosRegistration::UpdateTransactionStarted()
{
    if (GET_N_CONFIG(m_nSlotId)->IsCdmalessFeatureTagRequired() && GetState() == STATE_REFRESHSTOP)
    {
        SetHeldByCall(IMS_FALSE);
    }

    m_bIsTransactionStarted =
            !(IsBlocked() || IsHeldByCall() || IsRadioWaiting() || IsTrafficPriorityBlocked());

    A_IMS_TRACE_I(REGID, "UpdateTransactionStarted :: (%s)",
            (m_bIsTransactionStarted) ? "READY" : "NOT READY", 0, 0);
}

PROTECTED VIRTUAL void AosRegistration::UpdateIpsecSupported(
        IN IMS_BOOL bSupported, IN IMS_UINT32 nReason /* = 0 */)
{
    if (!m_pUtil->IsFeatureOn(FEATURE_IPSEC, m_nFeature))
    {
        return;
    }

    if (!bSupported)
    {
        m_nIpsecBlockReason |= nReason;
    }
    else
    {
        m_nIpsecBlockReason &= ~(nReason);
    }

    A_IMS_TRACE_I(REGID, "UpdateIpsecSupported :: (%x)", m_nIpsecBlockReason, 0, 0);
}

PROTECTED VIRTUAL IMS_UINT32 AosRegistration::GetActualWaitTime()
{
    IMS_SINT32 nDefaultWaitTime = GET_N_CONFIG(m_nSlotId)->GetRegDefaultWaitTime();

    if (nDefaultWaitTime > 0)
    {
        return nDefaultWaitTime;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy() ==
            CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        const ImsVector<IMS_SINT32>& objInterval = GET_N_CONFIG(m_nSlotId)->GetRegRetryIntervals();
        IMS_UINT32 nSize = objInterval.GetSize();

        if (nSize > 0)
        {
            IMS_SINT32 nAt = (m_nConsecutiveFailure > nSize) ? nSize : m_nConsecutiveFailure;

            const ImsVector<IMS_SINT32>& objUpperRandom =
                    GET_N_CONFIG(m_nSlotId)->GetRegRandomRetryIntervals();

            if (objUpperRandom.GetSize() == nSize)
            {
                IMS_UINT32 nAwt = objInterval.GetAt(nAt - 1);
                if (objUpperRandom.GetAt(nAt - 1) > 0)
                {
                    nAwt += IMS_SYS_GetRandom(objUpperRandom.GetAt(nAt - 1) + 1);
                }

                A_IMS_TRACE_I(REGID, "GetActualWaitTime :: failure count (%d) , awt (%d)",
                        m_nConsecutiveFailure, nAwt, 0);
                return nAwt;
            }
        }
    }

    A_IMS_TRACE_I(REGID,
            "GetActualWaitTime :: max-time(%d), base-time(%d), consecutive-failures (%d)",
            m_nRetryMaxTime, m_nRetryBaseTime, m_nConsecutiveFailure);

    return m_pUtil->WaitTimeForFlowRecovery(
            m_nRetryBaseTime, m_nRetryMaxTime, m_nConsecutiveFailure);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SetFirstPcscf(
        IN IMS_BOOL bUpdateParameter /* = IMS_TRUE */)
{
    AString strPcscf;
    IMS_UINT32 nPort;

    if (m_piContext->GetPcscf()->GetFirstPcscf(strPcscf, nPort) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    m_strPcscf = strPcscf;
    m_nPcscfPort = nPort;

    if (!bUpdateParameter)
    {
        return IMS_TRUE;
    }

    return UpdatePreloadedRoute();
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::SetNextPcscf(
        IN IMS_BOOL bUpdateParameter /* = IMS_TRUE */)
{
    if (IsRetryOnSamePcscfRequired())
    {
        A_IMS_TRACE_I(REGID, "SetNextPcscf :: Use current pcscf", 0, 0, 0);
        return IMS_TRUE;
    }

    AString strPcscf;
    IMS_UINT32 nPort;

    if (m_piContext->GetPcscf()->GetNextPcscf(strPcscf, nPort) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    m_strPcscf = strPcscf;
    m_nPcscfPort = nPort;

    if (!bUpdateParameter)
    {
        return IMS_TRUE;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsIpsecInitializedWithNewPcscf())
    {
        ClearIpsecBlock();
        if (IsIpsecSupported())
        {
            CreateIpsecHelper();
        }
    }

    if (UpdatePreloadedRoute())
    {
        if (GET_N_CONFIG(m_piContext->GetSlotId())
                        ->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
        {
            IMS_UINT32 nType = (m_eRegType == AosRegistrationType::NORMAL)
                    ? AosRetryRepository::TYPE_NORMAL
                    : AosRetryRepository::TYPE_EMERGENCY;

            AosProvider::GetInstance()
                    ->GetRetryRepository(m_piContext->GetSlotId())
                    ->ResetRetryCount(nType);
        }
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::TryNextPcscf()
{
    if (SetNextPcscf())
    {
        SetState(STATE_REGSTOP);
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);

        if (!SendRegister(IMS_TRUE))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        A_IMS_TRACE_I(REGID, "TryNextPcscf :: failure ", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::TryNextPcscf(
        IN IMS_BOOL bFlowRecoveryOnAllFail, IN IMS_BOOL bHonorRetryAfter /* = IMS_FALSE */)
{
    A_IMS_TRACE_I(REGID, "TryNextPcscf", 0, 0, 0);

    if (SetNextPcscf())
    {
        if ((bHonorRetryAfter) && (m_pUtil->GetRetryAfterValue(m_piRegistration) > 0))
        {
            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            StartTimer(TIMER_STOP_RETRY, m_pUtil->GetRetryAfterValue(m_piRegistration) * 1000);
            return IMS_TRUE;
        }

        if (SendRegister(IMS_TRUE) == IMS_FALSE)
        {
            SetState(STATE_REGSTOP);
            return IMS_FALSE;
        }

        SetState(STATE_REGISTERING);
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        return IMS_TRUE;
    }

    if (bFlowRecoveryOnAllFail)
    {
        SetFirstPcscf();
        ProcessDefaultFlowRecovery_Start();
        return IMS_TRUE;
    }

    A_IMS_TRACE_I(REGID, "no more pcscf available and no way to recover", 0, 0, 0);

    SetState(STATE_REGSTOP);
    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::IsRetryStopped()
{
    if (IsRetryTimer())
    {
        return IMS_FALSE;
    }

    if (IsRetryHeld())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::IsRetryOnSamePcscfRequired() const
{
    IMS_SINT32 nRegRetryCountPerPcscf = GET_N_CONFIG(m_nSlotId)->GetRegRetryCountPerPcscf();

    if (nRegRetryCountPerPcscf > 0)
    {
        return (nRegRetryCountPerPcscf >= m_piContext->GetPcscf()->GetCurrentPcscfTriedCount());
    }

    IMS_SINT32 nRegRetryCountOnSinglePcscf =
            GET_N_CONFIG(m_nSlotId)->GetRegRetryCountOnSinglePcscf();

    if (nRegRetryCountOnSinglePcscf > 0 && m_piContext->GetPcscf()->GetPcscfCount() == 1)
    {
        return (nRegRetryCountOnSinglePcscf >=
                m_piContext->GetPcscf()->GetCurrentPcscfTriedCount());
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ClearRegParameters(IN IMS_BOOL bClearPcscf /* = IMS_TRUE */)
{
    ClearTimers();
    ClearRetryCount();
    ClearRetryValues();
    ClearAuthChallengedCount();
    ClearPending();
    ClearCallingNumberVerification();

    if (bClearPcscf)
    {
        ClearPcscf();
    }

    if (GET_N_CONFIG(m_nSlotId)->IsGibaSupportedForERegInRoaming())
    {
        IAosSubscriber* pSubscriber = m_piContext->GetSubscriber();
        if (pSubscriber != IMS_NULL)
        {
            pSubscriber->ClearTemporaryPublicUserIdForGiba();
        }

        UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_GIBA);
    }
}

PROTECTED VIRTUAL void AosRegistration::ClearPcscf()
{
    IAosPcscf* piPcscf = m_piContext->GetPcscf();
    if (piPcscf != IMS_NULL)
    {
        piPcscf->SetFirstPcscfIndex();
        piPcscf->SetAllPcscfValid();
        piPcscf->ResetAllPcscfTried();
        piPcscf->ResetAllPcscfTriedCount();
    }
}

PROTECTED VIRTUAL void AosRegistration::ClearRetryCount(IN IMS_BOOL bForced /* = IMS_FALSE */)
{
    if (bForced == IMS_FALSE)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsKeepRegRetryCntUponPdnReconnect() ||
                (GET_N_CONFIG(m_nSlotId)->GetRegRetryCountResetPolicy() !=
                        CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION))
        {
            return;
        }
    }

    A_IMS_TRACE_D(REGID, "ClearRetryCount :: (%d) -> (%d)", m_nConsecutiveFailure, 0, 0);
    m_nConsecutiveFailure = 0;
    m_nSubConsecutiveFailureForRegForbiddenInWifi = 0;

    if (GET_N_CONFIG(m_piContext->GetSlotId())->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
    {
        IMS_UINT32 nType = (m_eRegType == AosRegistrationType::NORMAL)
                ? AosRetryRepository::TYPE_NORMAL
                : AosRetryRepository::TYPE_EMERGENCY;
        AosProvider::GetInstance()
                ->GetRetryRepository(m_piContext->GetSlotId())
                ->ResetRetryCount(nType);
    }

    SetRetryTimeToProperty(0);
}

PROTECTED VIRTUAL void AosRegistration::ClearRetryValues(IN IMS_BOOL bRegSuccess /* = IMS_FALSE */)
{
    if (bRegSuccess)
    {
        m_nConsecutiveFailureForPdnReactivated = 0;
        m_nForbiddenCount = 0;
    }
}

PROTECTED VIRTUAL void AosRegistration::ClearAuthChallengedCount()
{
    m_nAuthChallengeCount = 0;
    m_nAuthFailureCount = 0;
}

PROTECTED VIRTUAL void AosRegistration::ClearAuthIpsecCount()
{
    m_nAuthIpsecCount = 0;
}

PROTECTED VIRTUAL void AosRegistration::ClearErrorCount()
{
    m_nErrorCountForServerSocket = 0;
}

PROTECTED VIRTUAL void AosRegistration::ClearNetworkBindingFeatures()
{
    m_nNetworkBindingFeatures = 0;
}

PROTECTED VIRTUAL void AosRegistration::ClearIpsecBlock()
{
    ClearAuthIpsecCount();
    m_nIpsecBlockReason = 0;
}

PROTECTED VIRTUAL void AosRegistration::CloseUnsecureTcpSocket()
{
    IpAddress objIpaPcscf(m_strPcscf);
    SipFactory::GetTransportHelper(m_piContext->GetSlotId())
            ->DestroyTcpSocket(m_objIpa, 0, objIpaPcscf, m_nPcscfPort);
}

PROTECTED VIRTUAL void AosRegistration::CheckPending()
{
    if (m_pUtil->IsFeatureOn(PENDING_RECONFIG, m_nTxnPending))
    {
        A_IMS_TRACE_I(REGID, "CheckPending :: Post Msg (%s)",
                AosProvider::GetLog()->RegMessageToString(MSG_REG_RECONFIG), 0, 0);

        PostMessage(MSG_REG_RECONFIG, 0, 0);
    }

    if (m_pUtil->IsFeatureOn(PENDING_UPDATE, m_nTxnPending))
    {
        A_IMS_TRACE_I(REGID, "CheckPending :: Post Msg (%s)",
                AosProvider::GetLog()->RegMessageToString(MSG_REG_UPDATE), 0, 0);

        PostMessage(MSG_REG_UPDATE, 0, 0);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::CheckRadioReadyAndSetTxnPending()
{
    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return IMS_TRUE;
    }

    IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);

    if (piTransaction == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (!piTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG))
    {
        A_IMS_TRACE_I(REGID, "CheckRadioReadyAndSetTxnPending :: trx is not allowed", 0, 0, 0);

        SetTrafficPriorityBlocked(IMS_TRUE);
        UpdateTransactionStarted();
        m_pUtil->AddFeature(PENDING_TRAFFIC, m_nTxnPending);
        return IMS_FALSE;
    }
    else
    {
        if (SetTraffic(IMS_TRUE))
        {
            return IMS_TRUE;
        }

        SetRadioWaiting(IMS_TRUE);
        UpdateTransactionStarted();
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        return IMS_FALSE;
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessPendingPlmnBlockOnUpdateFailure()
{
    if (GET_N_CONFIG(m_nSlotId)->IsExtraReregErrInRoamingAsFailureHandled() && m_bEps5GsOnly &&
            m_piContext->GetNetTracker()->IsRoaming())
    {
        m_pUtil->AddFeature(PENDING_PLMN_BLOCK_HELD_BY_CALL, m_nTxnPending);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessPlmnBlockWithPcoLimitedModeOnStartFailure()
{
    if (m_piContext->GetConnection()->IsLimitedServicePcoValue() && m_nRegMode == MODE_LIMITED)
    {
        Destroy();
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PCO_LIMITED_SERVICE);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessPlmnBlockOnUpdateFailure()
{
    if (GET_N_CONFIG(m_nSlotId)->IsExtraReregErrInRoamingAsFailureHandled() && m_bEps5GsOnly &&
            m_piContext->GetNetTracker()->IsRoaming())
    {
        m_eImsReasonCode = AosReasonCode::PLMN_BLOCK;
        Destroy();
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessSetIpsec(IN IMS_UINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "ProcessSetIpsec :: (%d)", nReason, 0, 0);

    if (nReason == IAosRegistration::REASON_SET_IPSEC_ENABLE)
    {
        ProcessIpsecFallback(IMS_TRUE);
    }
    else if (nReason == IAosRegistration::REASON_SET_IPSEC_DISABLE)
    {
        ProcessIpsecFallback(IMS_FALSE);
    }
    else
    {
        A_IMS_TRACE_D(REGID, "command is invalid", 0, 0, 0);
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessRefreshRegInfo()
{
    A_IMS_TRACE_I(REGID, "ProcessRefreshRegInfo", 0, 0, 0);

    if (m_piRegContact != IMS_NULL)
    {
        m_piRegContact->RecalculateCallerCapabilities();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessIpcanChanged()
{
    A_IMS_TRACE_I(REGID, "ProcessIpcanChanged()", 0, 0, 0);

    if (!IsRegTypeEqual(AosRegistrationType::EMERGENCY))
    {
        if (GET_N_CONFIG(m_nSlotId)->IsRegWithIpcanChangedDuringImsCallHeld() && IsImsCall())
        {
            // if required not to update while calling,
            // then add feature PENDING_UPDATE.
            // when calls end, it'll do an actual update.
            m_pUtil->AddFeature(PENDING_UPDATE_HELD_BY_CALL, m_nTxnPending);

            UpdateRegIpcanCategory();

            if (GetState() == STATE_REGISTERED)
            {
                SetState(STATE_REFRESHING);
                ReportTryingState();

                SetState(STATE_REGISTERED);
                ReportStateChanged(RESULT_SUCCESS);
            }

            return;
        }
    }

    if (GetState() == STATE_REGISTERED)
    {
        SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);
    }

    Update();
}

PROTECTED VIRTUAL void AosRegistration::ProcessUpdateIpcan()
{
    A_IMS_TRACE_I(REGID, "ProcessUpdateIpcan()", 0, 0, 0);

    IMS_BOOL bCurrBlocked = !((m_piContext->GetConnection()->IsEpdgEnabled())
                    ? m_piContext->GetBlock()->IsCleared(SERVICE_WIFI)
                    : m_piContext->GetBlock()->IsCleared(SERVICE_CELLULAR));

    if (IsBlocked() != bCurrBlocked)
    {
        SetBlocked(bCurrBlocked);
        UpdateTransactionStarted();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessScscfRestoration(
        IN IMS_UINT32 nUnavailableTimeForCurrentPcscf)
{
    A_IMS_TRACE_I(REGID, "ProcessScscfRestoration :: nUnavailableTimeForCurrentPcscf (%d)",
            nUnavailableTimeForCurrentPcscf, 0, 0);

    IAosPcscf* piPcscf = m_piContext->GetPcscf();

    if (piPcscf == IMS_NULL)
    {
        return;
    }

    if (nUnavailableTimeForCurrentPcscf > 0)
    {
        piPcscf->SetCurrentPcscfInvalid(IMS_TRUE, nUnavailableTimeForCurrentPcscf);
    }
    else
    {
        piPcscf->SetCurrentPcscfInvalid();
    }

    if (piPcscf->HasNextPcscf())
    {
        DestroyEx();
        SetNextPcscf(IMS_FALSE);
        Start();
    }
    else
    {
        IAosHandle* piHandleMtc = m_piContext->GetHandle(ImsAosService::MTC);
        if (piHandleMtc != IMS_NULL && piHandleMtc->IsRegToNextPcscfRequested())
        {
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_NO_PCSCF_AVAILABLE);

            if (IsPdnReconnectWithDelayRequiredOnWfcSetupFailure())
            {
                ProcessStopForPdnReconnectWithAwt();
                return;
            }
        }

        Destroy();

        // make p-cscf discovery
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessPendingTransaction()
{
    A_IMS_TRACE_I(REGID, "ProcessPendingTransaction :: pending=%x", m_nTxnPending, 0, 0);

    if (m_pUtil->IsFeatureOn(PENDING_STOP, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_STOP, m_nTxnPending);
        if (m_nState == STATE_DEREGISTERING)
        {
            StartTimer(TIMER_DEREG_TRAFFIC, DEREGISTRATION_TRAFFIC_MAX_TIME * 1000);
            SetTrafficForDeregister(IMS_TRUE);
            return;
        }
    }

    if (m_pUtil->IsFeatureOn(PENDING_START, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_START, m_nTxnPending);
        Start();
        return;
    }

    if (m_pUtil->IsFeatureOn(PENDING_TRAFFIC, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_TRAFFIC | PENDING_TRANSACTION, m_nTxnPending);

        if (m_nState == STATE_REGSTOP || m_nState == STATE_REFRESHSTOP)
        {
            Update(IMS_TRUE, IMS_TRUE);
        }
        else
        {
            Start();
        }

        return;
    }

    if (m_pUtil->IsFeatureOn(PENDING_TRANSACTION, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_TRANSACTION, m_nTxnPending);

        if (IsRetryTimer())
        {
            // nothing to do
        }
        else
        {
            if (IsRetryHeld())
            {
                if (!SendRegister(GetState() != STATE_REFRESHSTOP))
                {
                    ProcessUnpredictableFailure();
                    return;
                }

                SetRetryState();
                ReportTryingState();
            }
            else
            {
                if (GetState() == STATE_OFFLINE)
                {
                    if (!CreateRegistration())
                    {
                        ProcessUnpredictableFailure();
                        return;
                    }
                    else
                    {
                        SetRetryState();
                        ReportTryingState();
                    }
                }
                else
                {
                    // nothing to do
                }
            }
        }
    }

    if (m_pUtil->IsFeatureOn(PENDING_SUBSCRIPTION, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_SUBSCRIPTION, m_nTxnPending);
        StartSubscription();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessRetryInRegStopped(
        IN IMS_BOOL bIgnoreTimer /* = IMS_FALSE */)
{
    if (bIgnoreTimer)
    {
        if (!IsRetryHeld())
        {
            return;
        }
    }
    else
    {
        if (!IsRetryStopped())
        {
            return;
        }
    }

    ClearRetryTimers();

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(REGID, "ProcessRetryInRegStopped :: txn couldn't be started", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_TRAFFIC, m_nTxnPending);
        return;
    }

    if (!CheckRadioReadyAndSetTxnPending())
    {
        A_IMS_TRACE_I(REGID, "ProcessRetryInRegStopped :: txn is pending due to radio", 0, 0, 0);
        return;
    }

    if (IsRegTypeEqual(AosRegistrationType::EMERGENCY) && !IsFakeRegistration() && !IsImsCall())
    {
        SetTraffic(IMS_TRUE);
    }

    if (!SendRegister(m_nState == STATE_REGSTOP))
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState((m_nState == STATE_REGSTOP) ? STATE_REGISTERING : STATE_REFRESHING);
    ReportTryingState();
}

PROTECTED VIRTUAL void AosRegistration::ProcessReregister()
{
    if (m_nState != STATE_REGISTERED)
    {
        return;
    }

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(REGID, "ProcessReregister :: txn couldn't be started", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_TRAFFIC, m_nTxnPending);
        SetState(STATE_REFRESHSTOP);
        return;
    }

    if (!CheckRadioReadyAndSetTxnPending())
    {
        A_IMS_TRACE_I(REGID, "ProcessReregister :: txn is pending due to radio", 0, 0, 0);
        if (IsRadioWaiting())
        {
            m_nState = STATE_REFRESHSTOP;
        }
        else
        {
            SetState(STATE_REFRESHSTOP);
        }
        return;
    }

    if (!SendRegister())
    {
        if (ProcessUnpredictableFailureHeldByCall())
        {
            A_IMS_TRACE_I(REGID, "ProcessReregister :: failure is pending due to call", 0, 0, 0);
            return;
        }

        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REFRESHING);
    ReportTryingState();
}

PROTECTED VIRTUAL void AosRegistration::ProcessReinitiate(IN IMS_BOOL bClearPcscf /* = IMS_TRUE */)
{
    A_IMS_TRACE_I(REGID, "ProcessReinitiate", 0, 0, 0);

    if (bClearPcscf)
    {
        Destroy();
    }
    else
    {
        DestroyEx();
    }

    if (!CreateRegistration())
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState(STATE_REGISTERING);
    ReportTryingState();
}

PROTECTED VIRTUAL void AosRegistration::ProcessUpdatePending()
{
    if (m_pUtil->IsFeatureOn(PENDING_UPDATE, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_UPDATE, m_nTxnPending);
        Update();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessReconfigPending()
{
    if (m_pUtil->IsFeatureOn(PENDING_RECONFIG, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_RECONFIG, m_nTxnPending);
        Reconfig();

        if (GetState() == STATE_REFRESHING)
        {
            m_pUtil->RemoveFeature(PENDING_UPDATE, m_nTxnPending);
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessUnpredictableFailure()
{
    m_eImsReasonCode = AosReasonCode::INTERNAL_ERROR;
    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_INTERNAL);
}

PROTECTED VIRTUAL void AosRegistration::ProcessNextPcscfUnsuccessful(
        IN IMS_UINT32 nPdnReactivateWaitTime, IN IMS_UINT32 nReason)
{
    SetState(STATE_REGSTOP);
    m_nPdnReactivateWaitTime = nPdnReactivateWaitTime;
    ReportStateChanged(RESULT_FAILURE, nReason);
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessUnpredictableFailureHeldByCall()
{
    if (GET_N_CONFIG(m_nSlotId)->IsCdmalessFeatureTagRequired())
    {
        return IMS_FALSE;
    }

    if (IsRegTypeEqual(AosRegistrationType::NORMAL) && IsRegistered() && IsImsCall())
    {
        SetHeldByCall(IMS_TRUE);
        UpdateTransactionStarted();
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        SetState(STATE_REFRESHSTOP);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegTerminated()
{
    if (IsImsCall())
    {
        A_IMS_TRACE_D(REGID, "ProcessRegTerminated :: after call is release, retry reg", 0, 0, 0);

        ClearTimers();
        SetState(STATE_OFFLINE);

        SetHeldByCall(IMS_TRUE);
        UpdateTransactionStarted();
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        return;
    }

    if (IsRetryTimer())
    {
        DestroyEx();
        A_IMS_TRACE_D(
                REGID, "ProcessRegTerminated :: ignore and restore after expiring timer", 0, 0, 0);
        return;
    }

    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_TERMINATED);
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegTerminatedByNotify()
{
    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_FORBIDDEN);
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegForbbidenInWifi()
{
    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_FORBIDDEN_IN_WIFI);
}

PROTECTED VIRTUAL void AosRegistration::ProcessAuthenticationFailed()
{
    IMS_BOOL bIsUsimAuthFailureNeeded = IsUsimAuthFailureHandlingNeeded();

    if (bIsUsimAuthFailureNeeded)
    {
        m_eImsReasonCode = AosReasonCode::USIM_AUTHENTICATION_FAILURES;
    }
    else if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPolicy() ==
            CarrierConfig::Ims::ERROR_POLICY_PDN_REACTIVATED)
    {
        IMS_UINT32 nState = GetState();

        if (nState == STATE_REGISTERING)
        {
            ProcessDefaultFlowRecovery_Start();
            return;
        }

        if (nState == STATE_REFRESHING)
        {
            ProcessDefaultFlowRecovery_Update();
            return;
        }
    }

    Destroy();

    ReportStateChanged(RESULT_FAILURE,
            bIsUsimAuthFailureNeeded ? REASON_FAILURE_USIM_AUTHENTICATION
                                     : REASON_FAILURE_AUTHENTICATION);
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegRequiredWithWaitTime(IN IMS_SINT32 nWaitTime)
{
    if (nWaitTime <= 0)
    {
        ProcessReinitiate();
        return;
    }

    if (!IsAppReady())
    {
        Destroy();
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_TERMINATED);
        return;
    }

    if (!IsImsCall())
    {
        Destroy();
        ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
    }
    else
    {
        ClearTimers();
        ClearPending();
        SetState(STATE_OFFLINE);
    }

    StartTimer(TIMER_OFFLINE_RECOVER, nWaitTime * 1000);
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegRequiredWithSamePcscf()
{
    IMS_UINT32 nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);

    DestroyEx();

    if (nRetryAfter > 0)
    {
        ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
        StartTimer(TIMER_OFFLINE_RECOVER, nRetryAfter * 1000);
    }
    else
    {
        if (!CreateRegistration())
        {
            ProcessUnpredictableFailure();
            return;
        }

        SetState(STATE_REGISTERING);
        ReportTryingState();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegRequiredWithNextPcscf()
{
    DestroyEx();

    if (SetNextPcscf(IMS_FALSE))
    {
        if (!CreateRegistration())
        {
            ProcessUnpredictableFailure();
            return;
        }

        SetState(STATE_REGISTERING);
        ReportTryingState();
        return;
    }
    else
    {
        if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrFinalType() ==
                CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK)
        {
            A_IMS_TRACE_I(REGID,
                    "ProcessRegRequiredWithNextPcscf :: All P-CSCFs attempted, RAT block required",
                    0, 0, 0);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
            return;
        }
    }

    ClearPcscf();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_TERMINATED);
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegRequiredWithAvailableNextPcscf(
        IN IMS_BOOL bSetCurrentPcscfInvalid, IN IMS_UINT32 nReconnectTime /* = 0 */)
{
    IMS_UINT32 nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);

    DestroyEx();

    if (bSetCurrentPcscfInvalid)
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
    }

    if (SetNextPcscf(IMS_FALSE))
    {
        if (nRetryAfter > 0 && m_piContext->GetPcscf()->GetPcscfCount() == 1)
        {
            A_IMS_TRACE_D(
                    REGID, "ProcessRegRequiredWithAvailableNextPcscf :: RA(%d)", nRetryAfter, 0, 0);
            ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
            StartTimer(TIMER_OFFLINE_RECOVER, nRetryAfter * 1000);
            return;
        }

        if ((GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy() ==
                    CarrierConfig::Ims::AWT_POLICY_RFC_RULE))
        {
            ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
            IncreaseConsecutiveFailCount();
            StartTimer(TIMER_OFFLINE_RECOVER, GetActualWaitTime() * 1000);
            return;
        }

        if (!CreateRegistration())
        {
            ProcessUnpredictableFailure();
            return;
        }

        SetState(STATE_REGISTERING);
        ReportTryingState();
    }
    else
    {
        if (nReconnectTime > 0)
        {
            m_nPdnReactivateWaitTime = nReconnectTime;
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
        }
        else
        {
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessRegRequiredWithIpVersionChange()
{
    if (ProcessIpVersionChange())
    {
        IMS_UINT32 nRetryTime = m_pUtil->GetRetryAfterValue(m_piRegistration);
        if (nRetryTime == 0)
        {
            nRetryTime = GetActualWaitTime();
        }

        DestroyEx();
        StartTimer(TIMER_OFFLINE_RECOVER, nRetryTime * 1000);
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessSubReinitiate()
{
    DestroySubscription();

    if (!IsRegistered())
    {
        return;
    }

    CreateSubscription();
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessForbiddenFailed(IN IMS_SINT32 nStatusCode)
{
    if (!IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrCode(), nStatusCode))
    {
        return IMS_FALSE;
    }

    m_nForbiddenCount++;

    IMS_SINT32 nMaxCount = (GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrMaxCount().GetSize() > 0)
            ? GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrMaxCount().GetAt(0)
            : 1;

    if (m_nForbiddenCount >= nMaxCount)
    {
        if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrFinalType() ==
                CarrierConfig::Ims::ERROR_TYPE_CRITICAL)
        {
            m_eImsReasonCode = AosReasonCode::PLMN_BLOCK;
        }

        Destroy();
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_FORBIDDEN);
    }
    else
    {
        if (IsRegistered())
        {
            ProcessDefaultFlowRecovery_Update();
        }
        else
        {
            ProcessDefaultFlowRecovery_Start();
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessSubscriberFailed(IN IMS_SINT32 nStatusCode)
{
    if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPolicy() !=
            CarrierConfig::Ims::ERROR_POLICY_SUBSCRIBER_FAILED)
    {
        return IMS_FALSE;
    }

    if (IsRegistered())
    {
        if (!IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraReregErrCode(), nStatusCode))
        {
            return IMS_FALSE;
        }
        else
        {
            if (m_nConsecutiveFailure == 1 && GetState() == STATE_REFRESHING)
            {
                A_IMS_TRACE_I(
                        REGID, "ProcessSubscriberFailed :: ignore subscriber failure", 0, 0, 0);
                ClearRetryValues();
                ClearRetryCount();
                PostMessage(MSG_REG_REINITIATE, 0, 0);
                return IMS_TRUE;
            }
        }
    }
    else
    {
        if (!IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraRegErrCode(), nStatusCode))
        {
            return IMS_FALSE;
        }
    }

    DestroySubscription();

    m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
    const AStringArray& objPuids = m_piContext->GetSubscriber()->GetConfiguredImpus();

    if (objPuids.GetCount() > 1)
    {
        if (m_strPuid.EqualsIgnoreCase(objPuids.GetElementAt(0)))
        {
            if (SetNextPcscf())
            {
                StartTimer(TIMER_STOP_RETRY, GetSpecificErrWaitTime() * 1000);
                SetState(STATE_REGSTOP);
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            }
            else
            {
                A_IMS_TRACE_D(REGID, "ProcessSubscriberFailed", 0, 0, 0);

                m_strPuid = objPuids.GetElementAt(1);
                const SipAddress objAor(m_strPuid);

                m_piRegistration->SetAor(objAor);
                m_piContext->GetPcscf()->SetAllPcscfValid();

                ProcessImsiBasedSubscriber();

                SetMode(MODE_LIMITED);
            }
            return IMS_TRUE;
        }
    }

    ProcessImsiBasedSubscriber();
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessAkaResponseFailed()
{
    return m_pIpsecHelper->Create(IMS_FALSE);
}

PROTECTED VIRTUAL void AosRegistration::ProcessAwtRecovery()
{
    A_IMS_TRACE_I(REGID, "ProcessAwtRecovery", 0, 0, 0);

    IncreaseConsecutiveFailCount();

    StartTimer(TIMER_STOP_RETRY, GetActualWaitTime() * 1000);

    SetFailureState();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
}

PROTECTED VIRTUAL void AosRegistration::ProcessIpsecFallback(IN IMS_BOOL bIsSupported)
{
    A_IMS_TRACE_I(REGID, "ProcessIpsecFallback", 0, 0, 0);

    if ((bIsSupported && !IsIpsecSupported()) || (!bIsSupported && IsIpsecSupported()))
    {
        Destroy();
        UpdateIpsecSupported(bIsSupported, IPSEC_BLOCK_ERROR);

        if (!CreateRegistration())
        {
            ProcessUnpredictableFailure();
            return;
        }

        SetState(STATE_REGISTERING);
    }
    else
    {
        ProcessDefaultFlowRecovery_Start();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessRequiredWfcErrMessage(IN IMS_SINT32 nStatusCode)
{
    if (!IsRegTypeEqual(AosRegistrationType::NORMAL))
    {
        return;
    }

    if (m_piContext->GetConnection()->IsEpdgEnabled() == IMS_FALSE)
    {
        return;
    }

    switch (nStatusCode)
    {
        case SipStatusCode::SC_403:
            ProcessRequiredWfcErrMessage_403();
            break;
        case SipStatusCode::SC_500:
            ProcessRequiredWfcErrMessage_500();
            break;
        default:
            ProcessRequiredWfcErrMessage_Others();
            break;
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessDefaultFlowRecovery_Start(
        IN IMS_SINT32 nStatusCode /* 0 */)
{
    A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start", 0, 0, 0);

    m_piContext->GetPcscf()->IncreaseCurrentPcscfTriedCount();

    IMS_UINT32 nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);
    if (GET_N_CONFIG(m_nSlotId)->IsRegErrCodeWithRetryAfterTimeOnlyDefined())
    {
        if (!IsErrorCodeExisted(
                    GET_N_CONFIG(m_nSlotId)->GetRegErrCodeWithRetryAfterTime(), nStatusCode))
        {
            nRetryAfter = 0;
        }
    }

    switch (GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy())
    {
        case CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EVERY_PCSCF:
            ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(nRetryAfter);
            break;

        case CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL:
            IncreaseConsecutiveFailCount();
            ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(nRetryAfter);
            break;

        case CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EACH_PCSCF:
            IncreaseConsecutiveFailCount();
            ProcessDefaultFlowRecovery_StartWithFailureToEachPcscf(nRetryAfter);
            break;

        case CarrierConfig::Ims::AWT_POLICY_RFC_RULE:  // FALL-THROUGH
        default:
            IncreaseConsecutiveFailCount();
            ProcessDefaultFlowRecovery_StartWithRfcRule(nRetryAfter);
            break;
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(
        IN IMS_UINT32 nRetryAfter)
{
    m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
    if (SetNextPcscf())
    {
        if (nRetryAfter > 0)
        {
            StartTimer(TIMER_STOP_RETRY, nRetryAfter * 1000);

            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        }
        else
        {
            if (!SendRegister(IMS_TRUE))
            {
                ProcessUnpredictableFailure();
                return;
            }
            SetState(STATE_REGISTERING);
            ReportTryingState();
        }
    }
    else
    {
        m_piContext->GetPcscf()->SetAllPcscfValid();
        if (SetFirstPcscf())
        {
            if (nRetryAfter == 0)
            {
                IncreaseConsecutiveFailCount();
                nRetryAfter = GetActualWaitTime();
            }

            StartTimer(TIMER_STOP_RETRY, nRetryAfter * 1000);

            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        }
        else
        {
            ProcessUnpredictableFailure();
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(
        IN IMS_UINT32 nRetryAfter)
{
    IMS_UINT32 nAwt = 0;

    if (GET_N_CONFIG(m_nSlotId)->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
    {
        if (AosProvider::GetInstance()
                        ->GetRetryRepository(m_piContext->GetSlotId())
                        ->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL))
        {
            if (nRetryAfter > 0)
            {
                nAwt = nRetryAfter;
            }
            else
            {
                const ImsVector<IMS_SINT32>& objInterval =
                        GET_N_CONFIG(m_nSlotId)->GetRegRetryIntervals();

                nAwt = (objInterval.GetSize() > 0) ? objInterval.GetAt(0) : RETRY_DEFAULT_WAIT_TIME;
            }

            StartTimer(TIMER_STOP_RETRY, nAwt * 1000);
            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        }
        else
        {
            m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
            if (SetNextPcscf())
            {
                StartTimer(TIMER_STOP_RETRY, nAwt * 1000);
                SetState(STATE_REGSTOP);
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            }
            else
            {
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
            }
        }
    }
    else
    {
        if (nRetryAfter > 0)
        {
            nAwt = nRetryAfter;
        }
        else
        {
            nAwt = GetActualWaitTime();
        }

        IAosPcscf* piPcscf = m_piContext->GetPcscf();

        piPcscf->SetCurrentPcscfTried();
        if (SetNextPcscf())
        {
            StartTimer(TIMER_STOP_RETRY, nAwt * 1000);
            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        }
        else
        {
            // TODO: T3402 block
            m_nPdnReactivateWaitTime = nAwt;
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessDefaultFlowRecovery_StartWithFailureToEachPcscf(
        IN IMS_UINT32 nRetryAfter)
{
    if (SetNextPcscf())
    {
        IMS_UINT32 nRetryTime = (nRetryAfter > 0) ? nRetryAfter : GetActualWaitTime();
        StartTimer(TIMER_STOP_RETRY, nRetryTime * 1000);

        SetState(STATE_REGSTOP);
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
    }
    else
    {
        ProcessNextPcscfUnsuccessful(0, REASON_FAILURE_PDN_RECONNECT);
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessDefaultFlowRecovery_StartWithRfcRule(
        IN IMS_UINT32 nRetryAfter)
{
    IMS_BOOL bBlockPcscf = GET_N_CONFIG(m_nSlotId)->IsBlockPcscfOnRegFailure();

    if (nRetryAfter > 0)  // IR.92
    {
        if (bBlockPcscf)
        {
            m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE, nRetryAfter);
        }

        if (TryNextPcscf())
        {
            SetState(STATE_REGISTERING);
            ReportTryingState();
        }
        else
        {
            ProcessNextPcscfUnsuccessful(nRetryAfter, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
        }
    }
    else  // 3GPP TS 24.229
    {
        IMS_UINT32 nAwt = GetActualWaitTime();

        if (bBlockPcscf)
        {
            m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE, nAwt + 300);
        }

        if (SetNextPcscf())
        {
            StartTimer(TIMER_STOP_RETRY, nAwt * 1000);
            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        }
        else
        {
            IMS_BOOL bIsNrBlockCondition =
                    GET_N_CONFIG(m_nSlotId)->GetRegTempPlmnBlockRatsOnAllPcscfsFail().Contains(
                            CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN) &&
                    GetNetworkTypeForImsRegState() == AosNetworkType::NR;
            if (bIsNrBlockCondition)
            {
                ProcessNextPcscfUnsuccessful(0, REASON_FAILURE_PDN_RECONNECT);
            }
            else
            {
                ProcessNextPcscfUnsuccessful(nAwt, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
            }
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessDefaultFlowRecovery_Update(
        IN IMS_SINT32 nStatusCode /* = 0 */)
{
    A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Update", 0, 0, 0);

    IMS_SINT32 nAwtPolicy = GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy();
    IMS_UINT32 nRetryAfter = 0;

    if (GET_N_CONFIG(m_nSlotId)->IsRegErrCodeWithRetryAfterTimeOnlyDefined())
    {
        if (IsErrorCodeExisted(
                    GET_N_CONFIG(m_nSlotId)->GetReregErrCodeWithRetryAfterTime(), nStatusCode))
        {
            nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);
        }
    }

    if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EVERY_PCSCF)
    {
        nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);

        DestroyEx();

        if (nRetryAfter > 0)
        {
            ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
            StartTimer(TIMER_OFFLINE_RECOVER, nRetryAfter * 1000);
        }
        else
        {
            if (!CreateRegistration())
            {
                ProcessUnpredictableFailure();
                return;
            }

            SetState(STATE_REGISTERING);
            ReportTryingState();
        }
    }
    else if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(nStatusCode, nRetryAfter);
    }
    else
    {
        ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE);
    }
}

PROTECTED VIRTUAL void
AosRegistration::ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
        IN IMS_SINT32 nStatusCode, IN IMS_UINT32 nRetryAfter)
{
    if (GET_N_CONFIG(m_nSlotId)->IsExtraRegErrRetryCntSharedForRegAndSubRequired())
    {
        if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraReregErrCode(), nStatusCode))
        {
            ProcessRegRequiredWithAvailableNextPcscf(IMS_TRUE);
        }
        else
        {
            if (AosProvider::GetInstance()
                            ->GetRetryRepository(m_piContext->GetSlotId())
                            ->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL))
            {
                IMS_UINT32 nAwt = m_pUtil->GetRetryAfterValue(m_piRegistration);
                if (nAwt == 0)
                {
                    const ImsVector<IMS_SINT32>& objInterval =
                            GET_N_CONFIG(m_nSlotId)->GetRegRetryIntervals();

                    nAwt = (objInterval.GetSize() > 0) ? objInterval.GetAt(0)
                                                       : RETRY_DEFAULT_WAIT_TIME;
                }

                StartTimer(TIMER_STOP_RETRY, nAwt * 1000);
                SetState(STATE_REFRESHSTOP);
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            }
            else
            {
                ProcessRegRequiredWithAvailableNextPcscf(IMS_TRUE);
            }
        }
    }
    else
    {
        if (nStatusCode == SipStatusCode::SC_481)
        {
            ProcessReinitiate(IMS_FALSE);
            return;
        }

        IncreaseConsecutiveFailCount();

        if (nRetryAfter == 0)
        {
            nRetryAfter = GetActualWaitTime();
            if (nRetryAfter == 0)
            {
                nRetryAfter = GetSpecificErrWaitTime();
            }
        }

        if (m_nConsecutiveFailure == 1 && !IsRegExpiredDuringAwt(nRetryAfter))
        {
            StartTimer(TIMER_STOP_RETRY, nRetryAfter * 1000);
            SetState(STATE_REFRESHSTOP);
        }
        else
        {
            if (m_nConsecutiveFailure == 1)
            {
                StartTimer(TIMER_OFFLINE_RECOVER, nRetryAfter * 1000);
                SetState(STATE_OFFLINE);
            }
            else
            {
                StartTimer(TIMER_STOP_RETRY, nRetryAfter * 1000);
                SetState(STATE_REGSTOP);

                m_piContext->GetPcscf()->SetCurrentPcscfTried();
            }

            DestroySubscription();

            if (!SetNextPcscf())
            {
                // TODO: T3402 block
                m_nPdnReactivateWaitTime = nRetryAfter;
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
                return;
            }
        }

        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessStartFailed_305()
{
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_nSlotId)->GetRegRetrySip305CodePolicy();

    if (nPolicy == CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP)
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
        ProcessStandardPcscfSelection();
        return IMS_TRUE;
    }
    else if (nPolicy == CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF)
    {
        if (m_piContext->GetPcscf()->IsFirstPcscf())
        {
            ProcessStandardPcscfSelection();
        }
        else
        {
            if (SetFirstPcscf())
            {
                if (!SendRegister(IMS_TRUE))
                {
                    ProcessUnpredictableFailure();
                    return IMS_TRUE;
                }
                SetState(STATE_REGISTERING);
                ReportTryingState();
            }
            else
            {
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
            }
        }
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_420()
{
    IMS_BOOL bIsExtensionUnsupported =
            m_pUtil->IsParameterIncluded(m_piRegistration->GetPreviousResponse(),
                    ISipHeader::UNSUPPORTED, AosString::STR_SEC_AGREE);

    if (bIsExtensionUnsupported)
    {
        ProcessIpsecFallback(IMS_FALSE);
    }
    else
    {
        ProcessDefaultFlowRecovery_Start();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_421()
{
    IMS_BOOL bIsExtensionRequired = m_pUtil->IsParameterIncluded(
            m_piRegistration->GetPreviousResponse(), ISipHeader::REQUIRE, AosString::STR_SEC_AGREE);

    if (m_pUtil->IsFeatureOn(FEATURE_IPSEC, m_nFeature) && bIsExtensionRequired)
    {
        ProcessIpsecFallback(IMS_TRUE);
    }
    else
    {
        ProcessDefaultFlowRecovery_Start();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_423()
{
    IMS_SINT32 nMinTime = m_pUtil->GetMinExpiresValue(m_piRegistration->GetPreviousResponse());

    if (nMinTime > 0)
    {
        if (!SendRegisterEx(nMinTime))
        {
            ProcessUnpredictableFailure();
            return;
        }

        SetState(STATE_REGISTERING);
    }
    else
    {
        ProcessDefaultFlowRecovery_Start();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_503()
{
    IMS_UINT32 nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);

    if (nRetryAfter == 0)
    {
        ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_503);
    }
    else
    {
        IMS_SINT32 nTimerF = SipConfigProxy::GetTimerValueF(m_nSlotId, IMS_NULL,
                Engine::GetConfiguration()->GetSipConfig(m_nSlotId)->GetSipConfigV(), IMS_TRUE);

        A_IMS_TRACE_I(REGID, "ProcessStartFailed_503 :: TF (%d), RA (%d)", nTimerF, nRetryAfter, 0);

        if (nTimerF > 0)
        {
            IncreaseConsecutiveFailCount();

            if (nRetryAfter > static_cast<IMS_UINT32>(nTimerF))
            {
                m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE, nRetryAfter);
                ProcessStandardPcscfSelection(nRetryAfter);
            }
            else
            {
                StartTimer(TIMER_STOP_RETRY, nRetryAfter * 1000);

                SetState(STATE_REGSTOP);
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            }
        }
        else
        {
            ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_503);
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessRequiredWfcErrMessage_403()
{
    if (!GET_N_CONFIG(m_piContext->GetSlotId())
                    ->IsWfcErrorMessageSupported(CarrierConfig::ImsWfc::WFC_ERROR_REG_403) &&
            !GET_N_CONFIG(m_piContext->GetSlotId())
                     ->IsWfcErrorMessageSupported(
                             CarrierConfig::ImsWfc::WFC_ERROR_NOT_SUPPORTED_COUNTRY))
    {
        return;
    }

    AosReasonCode eReason = AosReasonCode::WFC_REG_RESP_403;
    if (m_pUtil->IsDifferentCountry(
                UtilService::GetUtilService()->GetPrivateProperty()->GetPersistent(
                        ImsPrivateProperties::Persistent::KEY_SIM_COUNTRY, m_nSlotId),
                m_nSlotId))
    {
        eReason = AosReasonCode::WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY;
    }

    if (m_eImsReasonCode == eReason)
    {
        return;
    }

    m_eImsReasonCode = eReason;
}

PROTECTED VIRTUAL void AosRegistration::ProcessRequiredWfcErrMessage_500()
{
    if (m_eImsReasonCode == AosReasonCode::WFC_REG_RESP_500)
    {
        return;
    }

    if (!GET_N_CONFIG(m_piContext->GetSlotId())
                    ->IsWfcErrorMessageSupported(CarrierConfig::ImsWfc::WFC_ERROR_REG_500))
    {
        return;
    }

    m_eImsReasonCode = AosReasonCode::WFC_REG_RESP_500;
}

PROTECTED VIRTUAL void AosRegistration::ProcessRequiredWfcErrMessage_Others()
{
    if (m_eImsReasonCode == AosReasonCode::WFC_REG_RESP_OTHER_FAILURES)
    {
        return;
    }

    if (!GET_N_CONFIG(m_piContext->GetSlotId())
                    ->IsWfcErrorMessageSupported(CarrierConfig::ImsWfc::WFC_ERROR_OTHER_FAILURES))
    {
        return;
    }

    m_eImsReasonCode = AosReasonCode::WFC_REG_RESP_OTHER_FAILURES;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessUpdateFailed_305()
{
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_nSlotId)->GetReregRetrySip305CodePolicy();

    if (nPolicy == CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP)
    {
        ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_305);
        return IMS_TRUE;
    }
    else if (nPolicy == CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF)
    {
        if (m_piContext->GetPcscf()->IsFirstPcscf())
        {
            if (!SetNextPcscf(IMS_FALSE))
            {
                ProcessUnpredictableFailure();
                return IMS_TRUE;
            }
        }
        else
        {
            if (!SetFirstPcscf(IMS_FALSE))
            {
                ProcessUnpredictableFailure();
                return IMS_TRUE;
            }
        }

        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
        DestroyEx();

        if (!CreateRegistration())
        {
            ProcessUnpredictableFailure();
            return IMS_TRUE;
        }

        SetState(STATE_REGISTERING);
        ReportTryingState();

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessUpdateFailed_423()
{
    IMS_SINT32 nMinTime = m_pUtil->GetMinExpiresValue(m_piRegistration->GetPreviousResponse());

    if (nMinTime > 0)
    {
        if (!SendRegisterEx(nMinTime))
        {
            ProcessUnpredictableFailure();
            return;
        }

        SetState(STATE_REFRESHING);
    }
    else
    {
        ProcessDefaultFlowRecovery_Update();
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessStartFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeForPcscfDiscovery(), nStatusCode))
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
        ProcessStandardPcscfSelection();
        return;
    }

    if (ProcessForbiddenFailed(nStatusCode) || ProcessSubscriberFailed(nStatusCode))
    {
        return;
    }

    if (ProcessPlmnBlockWithPcoLimitedModeOnStartFailure())
    {
        return;
    }

    if (nStatusCode == SipStatusCode::SC_305)
    {
        if (ProcessStartFailed_305())
        {
            return;
        }
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeWithoutIpsec(), nStatusCode))
    {
        ProcessIpsecFallback(IMS_FALSE);
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsRegRetryWithIpVerFallback() &&
            nStatusCode >= SipStatusCode::SC_500 && nStatusCode < SipStatusCode::SC_600)
    {
        if (m_piContext->GetPcscf()->HasNextPcscf() || !ProcessRegRequiredWithIpVersionChange())
        {
            ProcessDefaultFlowRecovery_Start(nStatusCode);
        }

        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetRegRetrySip503CodePolicy() ==
                    CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP &&
            nStatusCode == SipStatusCode::SC_503)
    {
        ProcessStartFailed_503();
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPolicy() ==
            CarrierConfig::Ims::ERROR_POLICY_PDN_REACTIVATED)
    {
        if (SipStatusCode::IsFinalFailure(nStatusCode))
        {
            if (m_bEps5GsOnly ||
                    IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraRegErrCode(), nStatusCode))
            {
                if (IsPdnReactivationRequired())
                {
                    return;
                }
            }
        }
    }

    switch (nStatusCode)
    {
        // 420
        case SipStatusCode::SC_420:
            ProcessStartFailed_420();
            break;
        // 421
        case SipStatusCode::SC_421:
            ProcessStartFailed_421();
            break;
        // 423
        case SipStatusCode::SC_423:
            ProcessStartFailed_423();
            break;

        default:
            // Other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Start(nStatusCode);
            break;
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_TxnTimeout()
{
    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeForPcscfDiscovery(),
                CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();

        if (GET_N_CONFIG(m_nSlotId)->GetRegRetryTimerFPolicy() ==
                CarrierConfig::Ims::TIMER_F_POLICY_SPEC_WITH_AWT)
        {
            IncreaseConsecutiveFailCount();
            ProcessStandardPcscfSelection(GetActualWaitTime());
        }
        else
        {
            ProcessStandardPcscfSelection();
        }
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPolicy() ==
            CarrierConfig::Ims::ERROR_POLICY_PDN_REACTIVATED)
    {
        if (m_bEps5GsOnly ||
                IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraRegErrCode(),
                        CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
        {
            if (IsPdnReactivationRequired())
            {
                return;
            }
        }

        ProcessDefaultFlowRecovery_Start();
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeWithoutIpsec(),
                CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
    {
        ProcessIpsecFallback(IMS_FALSE);
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraRegErrCode(),
                CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
    {
        ProcessDefaultFlowRecovery_Start();
        return;
    }

    m_piContext->GetPcscf()->IncreaseCurrentPcscfTriedCount();
    IncreaseConsecutiveFailCount();

    IMS_UINT32 nAwt = GetActualWaitTime();
    if (!IsRetryOnSamePcscfRequired())
    {
        if (GET_N_CONFIG(m_nSlotId)->IsBlockPcscfOnRegFailure())
        {
            m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE, nAwt + 300);
        }
    }

    if (TryNextPcscf())
    {
        SetState(STATE_REGISTERING);
        ReportTryingState();
        return;
    }

    m_nPdnReactivateWaitTime = nAwt;

    SetState(STATE_REGSTOP);
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
}

PROTECTED VIRTUAL void AosRegistration::ProcessStartFailed_Others(IN IMS_SINT32 nReason)
{
    // TODO: add the recovery for REASON_INTERNAL_ERROR
    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraRegErrCode(),
                CarrierConfig::Ims::REG_ERROR_CODE_OTHER))
    {
        ProcessDefaultFlowRecovery_Start();
        return;
    }

    if (nReason == IRegistration::REASON_CLIENT_SOCKET_ERROR)
    {
        if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPolicy() ==
                CarrierConfig::Ims::ERROR_POLICY_PDN_REACTIVATED)
        {
            if (m_bEps5GsOnly ||
                    IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetExtraRegErrCode(),
                            CarrierConfig::Ims::REG_ERROR_CODE_TRANSPORT))
            {
                if (IsPdnReactivationRequired())
                {
                    return;
                }
            }

            ProcessDefaultFlowRecovery_Start();
            return;
        }
    }

    ProcessAwtRecovery();
}

PROTECTED VIRTUAL void AosRegistration::ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessUpdateFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    if (IsImsCall())
    {
        if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregErrCodeForCallEnd(), nStatusCode))
        {
            Destroy();
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_REG_TERMINATING);
            return;
        }
    }

    if (ProcessUnpredictableFailureHeldByCall())
    {
        ProcessPendingPlmnBlockOnUpdateFailure();
        return;
    }

    if (ProcessPlmnBlockOnUpdateFailure())
    {
        return;
    }

    if (nStatusCode == SipStatusCode::SC_305)
    {
        if (ProcessUpdateFailed_305())
        {
            return;
        }
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregErrCodeForInitRegWithAvailablePcscf(),
                nStatusCode))
    {
        ProcessRegRequiredWithAvailableNextPcscf(IMS_TRUE);
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeWithoutIpsec(), nStatusCode))
    {
        ProcessIpsecFallback(IMS_FALSE);
        return;
    }

    if (IsErrorCodeExisted(
                GET_N_CONFIG(m_nSlotId)->GetReregErrCodeForImsPdnReactivation(), nStatusCode))
    {
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
        return;
    }

    if (ProcessForbiddenFailed(nStatusCode) || ProcessSubscriberFailed(nStatusCode))
    {
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregRetryErrCodeForInitRegWithSamePcscf(),
                CarrierConfig::Ims::REG_ERROR_CODE_ALL_RESP))
    {
        ProcessRegRequiredWithSamePcscf();
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy() !=
            CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        switch (nStatusCode)
        {
            case SipStatusCode::SC_403:  // FALL-THROUGH
            case SipStatusCode::SC_408:  // FALL-THROUGH
            case SipStatusCode::SC_504:
                ProcessReinitiate(IMS_FALSE);
                return;

            default:
                break;
        }
    }

    switch (nStatusCode)
    {
        // 423
        case SipStatusCode::SC_423:
            ProcessUpdateFailed_423();
            break;

        default:
            // other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Update(nStatusCode);
            break;
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessUpdateFailed_TxnTimeout()
{
    if (ProcessUnpredictableFailureHeldByCall())
    {
        ProcessPendingPlmnBlockOnUpdateFailure();
        return;
    }

    if (ProcessPlmnBlockOnUpdateFailure())
    {
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregErrCodeForInitRegWithAvailablePcscf(),
                CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
    {
        if (GET_N_CONFIG(m_nSlotId)->GetRegRetryTimerFPolicy() ==
                CarrierConfig::Ims::TIMER_F_POLICY_SPEC_WITH_AWT)
        {
            IncreaseConsecutiveFailCount();
            ProcessRegRequiredWithAvailableNextPcscf(IMS_TRUE, GetActualWaitTime());
        }
        else
        {
            ProcessRegRequiredWithAvailableNextPcscf(IMS_TRUE);
        }

        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeWithoutIpsec(),
                CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
    {
        ProcessIpsecFallback(IMS_FALSE);
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregRetryErrCodeForInitRegWithSamePcscf(),
                CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F))
    {
        ProcessRegRequiredWithSamePcscf();
        return;
    }

    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL void AosRegistration::ProcessUpdateFailed_Others(IN IMS_SINT32 /* nReason */)
{
    if (GET_N_CONFIG(m_nSlotId)->GetRegActualWaitTimePolicy() ==
            CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        ProcessDefaultFlowRecovery_Update();
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregRetryErrCodeForInitRegWithSamePcscf(),
                CarrierConfig::Ims::REG_ERROR_CODE_OTHER))
    {
        ProcessRegRequiredWithSamePcscf();
        return;
    }

    ProcessAwtRecovery();
}

PROTECTED VIRTUAL void AosRegistration::ProcessStandardPcscfSelection(
        IN IMS_UINT32 nRetryAfter /* = 0 */)
{
    A_IMS_TRACE_D(REGID, "ProcessStandardPcscfSelection :: nRetryAfter(%d)", nRetryAfter, 0, 0);

    m_piContext->GetPcscf()->IncreaseCurrentPcscfTriedCount();

    if (TryNextPcscf())
    {
        SetState(STATE_REGISTERING);
        ReportTryingState();
        return;
    }

    SetState(STATE_REGSTOP);

    if (nRetryAfter > 0)
    {
        m_nPdnReactivateWaitTime = nRetryAfter;
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
    }
    else
    {
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::ProcessIpVersionChange()
{
    IpAddress objPcscf(m_strPcscf);
    IMS_SINT32 nTargetIpVersion = objPcscf.IsIpv4Address() ? IpAddress::IPV6 : IpAddress::IPV4;

    IAosConnection* piConnection = m_piContext->GetConnection();
    IpAddress objIpa = piConnection->GetLocalAddress(nTargetIpVersion);
    if (objIpa.GetVersion() != nTargetIpVersion)
    {
        return IMS_FALSE;
    }

    const AStringArray& objNewPcscfs = piConnection->GetPcscfAddress(nTargetIpVersion);
    if (objNewPcscfs.GetCount() <= 0)
    {
        return IMS_FALSE;
    }

    m_piContext->GetPcscf()->UpdatePcscfs(objNewPcscfs);
    m_piContext->GetPcscf()->SetFirstPcscfIndex();

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegEventChange(IN IMS_UINT32 nStatusCode)
{
    if (!IsRegTypeEqual(AosRegistrationType::NORMAL))
    {
        return;
    }

    IMS_SINT32 nPolicy = GET_N_CONFIG(m_piContext->GetSlotId())->GetUsatRegEventDownloadPolicy();
    if (nPolicy == CarrierConfig::Ims::USAT_REG_EVENT_NOT_DOWNLOAD)
    {
        return;
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_piContext->GetSlotId());
    if (piService == IMS_NULL)
    {
        return;
    }

    if (nPolicy == CarrierConfig::Ims::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD)
    {
        piService->NotifyRegEventState((nStatusCode <= SipStatusCode::SC_INVALID)
                        ? SipStatusCode::SC_INVALID
                        : nStatusCode);
    }

    // Notice : Handling for USAT_REG_EVENT_CONDITIONAL_DOWNLOAD has not yet been considered.
}

PROTECTED VIRTUAL void AosRegistration::RecordImpu()
{
    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return;
    }

    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_nSlotId);
    const AStringArray& objAssociatedUris = m_piRegistration->GetAssociatedUris();

    IMS_CHAR strSize[4];
    IMS_Itoa(strSize, objAssociatedUris.GetCount(), 10);

    piSubsInfo->SetPreference("impu_list", "size", AString(strSize));
    A_IMS_TRACE_I(REGID, "RecordImpu :: size (%d) m_nSlotId(%d)", objAssociatedUris.GetCount(),
            m_nSlotId, 0);

    ImsList<AString> objUris = ImsList<AString>();
    for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount() && (i < 9); i++)
    {
        IMS_CHAR strKey[4];
        IMS_Itoa(strKey, i, 10);

        SipAddress objAssociatedUri(objAssociatedUris.GetElementAt(i));

        AString strAor = objAssociatedUri.GetUri();

        if (strAor == AString::ConstNull() || strAor.IsEmpty())
        {
            continue;
        }

        piSubsInfo->SetPreference("impu_list", AString(strKey), strAor);
        objUris.Append(strAor);

        A_IMS_TRACE_D(REGID, "RecordImpu :: key(%d) value(%s) m_nSlotId(%d)", i, strAor.GetStr(),
                m_nSlotId);
    }

    if (objUris.GetSize() > 0)
    {
        IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
        if (piService != IMS_NULL)
        {
            piService->NotifyAssociatedUriChanged(objUris);
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::Registration_AuthenticationChallenged(
        IN IMS_SINT32 nAlgorithm, OUT IMS_BOOL& bResponseToChallenge)
{
    A_IMS_TRACE_I(REGID, "Registration_AuthenticationChallenged", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    m_nAuthChallengeCount++;

    if (!IsAuthChallengeMoreAllowed() || IsAuthFailureMaxCountReached())
    {
        bResponseToChallenge = IMS_FALSE;

        if (IsUsimAuthFailureHandlingNeeded())
        {
            ProcessAuthenticationFailed();
        }

        return;
    }

    bResponseToChallenge = IMS_TRUE;

    if (m_pIpsecHelper != IMS_NULL && IsIpsecSupported())
    {
        if (!m_pIpsecHelper->ProcessAuthChallenged(nAlgorithm))
        {
            bResponseToChallenge = IMS_FALSE;

            m_nAuthIpsecCount++;
            if (m_nAuthIpsecCount >
                    GET_N_CONFIG(m_nSlotId)->GetRegRetryCountWithIpsecOnAuthFailure())
            {
                UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_AUTHENTICATION);
            }

            if (GetState() == STATE_REGISTERING)
            {
                DestroyIpsecHelper();
                PostMessage(MSG_REG_REINITIATE, 0, 0);
            }
            else
            {
                // For other states, UpdateFailed() or Registration_Removed() will be invoked.
                // And then we will have an initial registration with IPSec disabled.
            }
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::Registration_NotifyAkaResponse(IN IMS_SINT32 nResult,
        IN const ByteArray& objIK, IN const ByteArray& objCK, OUT IMS_BOOL& bResultOfSA)
{
    A_IMS_TRACE_I(REGID, "Registration_NotifyAkaResponse :: result(%d)", nResult, 0, 0);

    bResultOfSA = IMS_FALSE;

    if (m_pIpsecHelper == IMS_NULL || !IsIpsecSupported())
    {
        return;
    }

    if (nResult != ImsAkaParam::RESULT_OK)
    {
        A_IMS_TRACE_I(REGID, "Aka response is failed , wait next 401 message", 0, 0, 0);

        if (!ProcessAkaResponseFailed())
        {
            ProcessRegTerminated();
            return;
        }

        if (nResult == ImsAkaParam::RESULT_NOK_MAC_INVALID && IsUsimAuthFailureHandlingNeeded())
        {
            ProcessAuthenticationFailed();
            return;
        }

        bResultOfSA = IMS_TRUE;
    }
    else
    {
        m_nAuthFailureCount++;

        if (IsAuthChallengedAgain())
        {
            m_pIpsecHelper->CreateOnChallenging();
        }

        if (!m_pIpsecHelper->SetPcscfPortnSpi())
        {
            ProcessRegTerminated();
            return;
        }

        if ((GetState() == STATE_REGISTERING) || (m_pIpsecHelper->IsPcscfServerPortDifferent()))
        {
            if (!m_pIpsecHelper->UpdatePreloadedRoute(m_strPcscf))
            {
                ProcessRegTerminated();
                return;
            }
        }

        if (!m_pIpsecHelper->MakeSas(m_strPcscf, m_objIpa, objIK, objCK))
        {
            ProcessRegTerminated();
            return;
        }
        else
        {
            bResultOfSA = IMS_TRUE;
        }

        SetContactAddressConfiguration(IMS_FALSE);

        if (!AddOperation_OnNotifyAkaResponse())
        {
            bResultOfSA = IMS_FALSE;
            return;
        }

        SetContactAddressConfiguration(IMS_TRUE);
    }
}

PROTECTED VIRTUAL void AosRegistration::Registration_RefreshTimerExpired(
        OUT IMS_BOOL& bDoImplicitRefresh)
{
    A_IMS_TRACE_I(REGID, "Registration_RefreshTimerExpired", 0, 0, 0);

    bDoImplicitRefresh = IMS_FALSE;

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    if (GetState() != STATE_REGISTERED)
    {
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetRegOutOfServicePolicy() ==
            CarrierConfig::Ims::REG_OOS_POLICY_DESTROY)
    {
        if (m_piContext->GetNetTracker()->IsSuspended())
        {
            ProcessUnpredictableFailure();
            return;
        }
    }

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(REGID, "Registration_RefreshTimerExpired :: txn can't be started", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_TRAFFIC, m_nTxnPending);
        SetState(STATE_REFRESHSTOP);
        return;
    }

    CheckRadioReadyAndSetTxnPending();

    if (!IsTransactionStarted())
    {
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        if (IsRadioWaiting())
        {
            m_nState = STATE_REFRESHSTOP;
        }
        else
        {
            SetState(STATE_REFRESHSTOP);
        }
        return;
    }

    if (m_pIpsecHelper != IMS_NULL && IsIpsecSupported())
    {
        if (!m_pIpsecHelper->Create(IMS_FALSE))
        {
            ProcessRegTerminated();
            return;
        }
    }

    bDoImplicitRefresh = IMS_TRUE;

    SetState(STATE_REFRESHING);
    ReportTryingState();
}

PROTECTED VIRTUAL void AosRegistration::Registration_Started()
{
    A_IMS_TRACE_I(REGID, "Registration_Started", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    ClearErrorCount();
    ClearRetryTimers();
    ClearRetryValues(IMS_TRUE);
    ClearAuthChallengedCount();
    ClearAuthIpsecCount();

    if (m_eRegType != AosRegistrationType::NORMAL ||
            GET_N_CONFIG(m_nSlotId)->GetRegRetryCountResetPolicy() ==
                    CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION)
    {
        ClearRetryCount(IMS_TRUE);
    }

    m_piContext->GetPcscf()->ResetCurrentPcscfTriedCount();

    if (IsIpsecSupported())
    {
        if (m_pIpsecHelper != IMS_NULL && m_pIpsecHelper->IsEstablished())
        {
            m_pIpsecHelper->ProcessRegStarted();
        }
        else
        {
            A_IMS_TRACE_I(REGID, "Registration_Started :: ipsec is not established", 0, 0, 0);
            UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_NOT_ESTABLISHED);
            DestroyIpsecHelper();
            PostMessage(MSG_REG_REINITIATE, 0, 0);
            return;
        }
    }

    SetState(STATE_REGISTERED);

    UpdateUserInfoInContact();

    UpdateCallingNumberVerification();

    CreateSubscription();

    UpdateNetworkRegBinding();

    UpdateNetworkRegFeatureBinding();

    ReportStateChanged(RESULT_SUCCESS);

    CheckPending();

    RecordImpu();
}

PROTECTED VIRTUAL void AosRegistration::Registration_StartFailed(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "Registration_StartFailed :: (%s)",
            AosProvider::GetLog()->RegReasonToString(nReason), 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    if (!IsAuthChallengeMoreAllowed())
    {
        ProcessAuthenticationFailed();
        return;
    }

    ClearAuthChallengedCount();
    ClearAuthIpsecCount();

    IMS_SINT32 nStatusCode = m_pUtil->GetResponseCode(m_piRegistration->GetPreviousResponse());
    NotifyFailureWithImsReason(nReason, nStatusCode);
    ProcessRequiredWfcErrMessage(nStatusCode);
    ProcessRegEventChange(nStatusCode);

    switch (nReason)
    {
        case IRegistration::REASON_STATUS_CODE:
            ProcessStartFailed_StatusCode(nStatusCode);
            break;

        case IRegistration::REASON_TRANSACTION_TIMEOUT:
            ProcessStartFailed_TxnTimeout();
            break;

        default:
            ProcessStartFailed_Others(nReason);
            break;
    }

    CheckPending();
}

PROTECTED VIRTUAL void AosRegistration::Registration_Updated()
{
    A_IMS_TRACE_I(REGID, "Registration_Updated", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    ClearRetryTimers();
    ClearRetryValues(IMS_TRUE);
    ClearAuthChallengedCount();
    ClearAuthIpsecCount();
    SetReregFailureReportOnIpcanChangeRequired(IMS_FALSE);

    if (m_eRegType != AosRegistrationType::NORMAL ||
            GET_N_CONFIG(m_nSlotId)->GetRegRetryCountResetPolicy() ==
                    CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION)
    {
        ClearRetryCount(IMS_TRUE);
    }

    if (IsIpsecSupported())
    {
        if (m_pIpsecHelper == IMS_NULL || !m_pIpsecHelper->ProcessRegUpdated())
        {
            ProcessUnpredictableFailure();
            return;
        }
    }

    SetState(STATE_REGISTERED);

    if (m_pSubscription == IMS_NULL)
    {
        CreateSubscription();
    }
    else
    {
        if (m_pUtil->IsFeatureOn(PENDING_SUBSCRIPTION, m_nTxnPending))
        {
            m_pUtil->RemoveFeature(PENDING_SUBSCRIPTION, m_nTxnPending);
            StartSubscription(IMS_FALSE);
        }
    }

    UpdateCallingNumberVerification();

    UpdateNetworkRegBinding();

    UpdateNetworkRegFeatureBinding();

    ReportStateChanged(RESULT_SUCCESS);

    CheckPending();

    RecordImpu();
}

PROTECTED VIRTUAL void AosRegistration::Registration_UpdateFailed(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "Registration_UpdateFailed :: (%s)",
            AosProvider::GetLog()->RegReasonToString(nReason), 0, 0);

    if (!IsAuthChallengeMoreAllowed())
    {
        ProcessAuthenticationFailed();
        return;
    }

    ClearAuthChallengedCount();
    ClearAuthIpsecCount();

    switch (nReason)
    {
        case IRegistration::REASON_STATUS_CODE:
            ProcessUpdateFailed_StatusCode(
                    m_pUtil->GetResponseCode(m_piRegistration->GetPreviousResponse()));
            break;

        case IRegistration::REASON_TRANSACTION_TIMEOUT:
            ProcessUpdateFailed_TxnTimeout();
            break;

        default:
            ProcessUpdateFailed_Others(nReason);
            break;
    }

    CheckPending();

    if (IsReregFailureReportOnIpcanChangeRequired())
    {
        NotifyTechnologyChangeFailed();
    }
}

PROTECTED VIRTUAL void AosRegistration::Registration_Removed()
{
    A_IMS_TRACE_I(REGID, "Registration_Removed", 0, 0, 0);

    IMS_BOOL bDeregisteringState = (GetState() == STATE_DEREGISTERING) ? IMS_TRUE : IMS_FALSE;
    IMS_UINT32 nTxnPending = m_nTxnPending;

    Destroy();

    if (bDeregisteringState)
    {
        SipFactory::GetTransportHelper(m_piContext->GetSlotId())->DestroyAllSockets(0, m_objIpa);

        if (m_pUtil->IsFeatureOn(PENDING_PDN_RECONNECT_WITH_AWT, nTxnPending))
        {
            ReconnectPdnWithDelayOnWfcSetupFail();
        }
        else
        {
            ReportStateChanged(RESULT_SUCCESS);
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::Registration_Terminated(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "Registration_Terminated :: (%s)",
            AosProvider::GetLog()->RegReasonToString(nReason), 0, 0);

    if (IsHandlingServerSocketErrorRequired(nReason))
    {
        if (IsReconnectingServerSocketErrorAllowed())
        {
            if (m_piInternalErrorTimer != IMS_NULL)
            {
                return;
            }

            ++m_nErrorCountForServerSocket;
            A_IMS_TRACE_I(
                    REGID, "server socket error count (%d)", m_nErrorCountForServerSocket, 0, 0);

            StartTimer(TIMER_INTERNAL_ERROR, INTERNAL_ERROR_INTERVAL * 1000);
        }
        else
        {
            A_IMS_TRACE_I(
                    REGID, "re-initiate registration due to max server socket error", 0, 0, 0);

            ClearErrorCount();
            PostMessage(MSG_REG_REINITIATE, 0, 0);
        }

        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsCallEndAndPdnReactivationByRegTerminated())
    {
        if (IsImsCall())
        {
            SetHeldByCall(IMS_FALSE);
            UpdateTransactionStarted();

            Destroy();
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
            return;
        }
    }

    ProcessRegTerminated();
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::RegUserIdentity_ReorderUserIdentities(
        IN const AStringArray& objUserIds, OUT AStringArray& objReorderedUserIds)
{
    if (!GET_N_CONFIG()->IsGibaSupportedForERegInRoaming())
    {
        return IMS_FALSE;
    }

    const IAosSubscriber* pSubscriber = m_piContext->GetSubscriber();
    if (pSubscriber == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pSubscriber->HasValidTemporaryPublicUserIdForGiba())
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(REGID, "RegUserIdentity_ReorderUserIdentities", 0, 0, 0);

    /**
     * CASE: The 200 OK for REGISTER message does not contain a P-Associated-URI header.
     *       If the engine provides only a single temporary public user identity, replace it with
     *       the list of configured IMPUs.
     */
    if (objUserIds.GetCount() == 1 &&
            objUserIds.GetElementAt(0).Equals(pSubscriber->GetTemporaryPublicUserIdForGiba()))
    {
        A_IMS_TRACE_I(REGID, "RegUserIdentity_ReorderUserIdentities :: Change UserIds", 0, 0, 0);
        objReorderedUserIds = pSubscriber->GetConfiguredImpus();
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosRegistration::ProcessOfflineRecoverTimerExpired()
{
    StopTimer(TIMER_OFFLINE_RECOVER);

    if (GetState() != STATE_OFFLINE)
    {
        return;
    }

    if (!IsAppReady())
    {
        return;
    }

    if (IsImsCall())
    {
        SetHeldByCall(IMS_TRUE);
        UpdateTransactionStarted();
    }

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(REGID, "ProcessOfflineRecoverTimerExpired :: txn can't be started", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_TRAFFIC, m_nTxnPending);
        return;
    }

    CheckRadioReadyAndSetTxnPending();

    if (IsTransactionStarted())
    {
        if (CreateRegistration())
        {
            SetState(STATE_REGISTERING);
            ReportTryingState();
        }
        else
        {
            ProcessUnpredictableFailure();
            return;
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessStopRetryTimerExpired()
{
    StopTimer(TIMER_STOP_RETRY);

    if (!IsRetryHeld())
    {
        return;
    }

    if (IsImsCall())
    {
        IAosHandle* piHandleMtc = m_piContext->GetHandle(ImsAosService::MTC);
        if (piHandleMtc == IMS_NULL || !piHandleMtc->IsRegToNextPcscfRequested())
        {
            SetHeldByCall(IMS_TRUE);
            UpdateTransactionStarted();
        }
    }

    if (!IsTransactionStarted())
    {
        A_IMS_TRACE_I(REGID, "ProcessStopRetryTimerExpired :: txn can't be started", 0, 0, 0);
        m_pUtil->AddFeature(PENDING_TRAFFIC, m_nTxnPending);
        return;
    }

    CheckRadioReadyAndSetTxnPending();

    if (IsTransactionStarted())
    {
        if (SendRegister(GetState() != STATE_REFRESHSTOP))
        {
            SetRetryState();
            ReportTryingState();
        }
        else
        {
            ProcessUnpredictableFailure();
            return;
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessRefreshTimerExpired()
{
    StopTimer(TIMER_REFRESH);
}

PROTECTED VIRTUAL void AosRegistration::ProcessDeregTrafficTimerExpired()
{
    StopTimer(TIMER_DEREG_TRAFFIC);
    SetTrafficForDeregister(IMS_FALSE);
}

PROTECTED VIRTUAL void AosRegistration::ProcessModeTimerExpired()
{
    StopTimer(TIMER_MODE);
}

PROTECTED VIRTUAL void AosRegistration::ProcessTransactionTimerExpired()
{
    StopTimer(TIMER_TRANSACTION);
}

PROTECTED VIRTUAL void AosRegistration::ProcessInternalErrorTimerExpired()
{
    StopTimer(TIMER_INTERNAL_ERROR);

    if (IsRegistered())
    {
        if (m_piRegistration != IMS_NULL)
        {
            m_piRegistration->RestoreActiveBindings();
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_OFFLINE_RECOVER:
            ppiTimer = &m_piOfflineRecoverTimer;
            break;

        case TIMER_STOP_RETRY:
            ppiTimer = &m_piStopRetryTimer;
            break;

        case TIMER_REFRESH:
            ppiTimer = &m_piRefreshTimer;
            break;

        case TIMER_DEREG_TRAFFIC:
            ppiTimer = &m_piDeregTrafficTimer;
            break;

        case TIMER_MODE:
            ppiTimer = &m_piModeTimer;
            break;

        case TIMER_TRANSACTION:
            ppiTimer = &m_piTransactionTimer;
            break;

        case TIMER_INTERNAL_ERROR:
            ppiTimer = &m_piInternalErrorTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer =
            m_pUtil->StartTimer(nDuration, this, AosProvider::GetLog()->RegTimerToString(nType));

    if (nType == TIMER_STOP_RETRY)
    {
        SetRetryTimeToProperty(nDuration / 1000);
    }
}

PROTECTED VIRTUAL void AosRegistration::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_OFFLINE_RECOVER:
            ppiTimer = &m_piOfflineRecoverTimer;
            break;

        case TIMER_STOP_RETRY:
            ppiTimer = &m_piStopRetryTimer;
            break;

        case TIMER_REFRESH:
            ppiTimer = &m_piRefreshTimer;
            break;

        case TIMER_DEREG_TRAFFIC:
            ppiTimer = &m_piDeregTrafficTimer;
            break;

        case TIMER_MODE:
            ppiTimer = &m_piModeTimer;
            break;

        case TIMER_TRANSACTION:
            ppiTimer = &m_piTransactionTimer;
            break;

        case TIMER_INTERNAL_ERROR:
            ppiTimer = &m_piInternalErrorTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    m_pUtil->StopTimer(*ppiTimer, AosProvider::GetLog()->RegTimerToString(nType));
}

PROTECTED VIRTUAL void AosRegistration::ClearRetryTimers()
{
    if (m_piOfflineRecoverTimer != IMS_NULL)
    {
        StopTimer(TIMER_OFFLINE_RECOVER);
    }

    if (m_piStopRetryTimer != IMS_NULL)
    {
        StopTimer(TIMER_STOP_RETRY);
    }
}

PROTECTED VIRTUAL void AosRegistration::ClearTimers()
{
    // TIMER_OFFLINE_RECOVER should not be stopped

    if (m_piStopRetryTimer != IMS_NULL)
    {
        StopTimer(TIMER_STOP_RETRY);
    }

    if (m_piRefreshTimer != IMS_NULL)
    {
        StopTimer(TIMER_REFRESH);
    }

    if (m_piDeregTrafficTimer != IMS_NULL)
    {
        StopTimer(TIMER_DEREG_TRAFFIC);
        SetTrafficForDeregister(IMS_FALSE);
    }

    if (m_piModeTimer != IMS_NULL)
    {
        StopTimer(TIMER_MODE);
    }

    if (m_piTransactionTimer != IMS_NULL)
    {
        StopTimer(TIMER_TRANSACTION);
    }

    if (m_piInternalErrorTimer != IMS_NULL)
    {
        StopTimer(TIMER_INTERNAL_ERROR);
    }
}

PROTECTED VIRTUAL void AosRegistration::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piOfflineRecoverTimer)
    {
        ProcessOfflineRecoverTimerExpired();
        return;
    }

    if (piTimer == m_piStopRetryTimer)
    {
        ProcessStopRetryTimerExpired();
        return;
    }

    if (piTimer == m_piRefreshTimer)
    {
        ProcessRefreshTimerExpired();
        return;
    }

    if (piTimer == m_piDeregTrafficTimer)
    {
        ProcessDeregTrafficTimerExpired();
        return;
    }

    if (piTimer == m_piModeTimer)
    {
        ProcessModeTimerExpired();
        return;
    }

    if (piTimer == m_piTransactionTimer)
    {
        ProcessTransactionTimerExpired();
        return;
    }

    if (piTimer == m_piInternalErrorTimer)
    {
        ProcessInternalErrorTimerExpired();
        return;
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::CreateSubscription()
{
    if (!m_pUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, m_nFeature))
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(REGID, "CreateSubscription", 0, 0, 0);

    DestroySubscription();

    if (m_piRegistration == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IRegSubscription* piRegSubscription = m_piRegistration->CreateSubscription();

    if (piRegSubscription == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "sub is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pSubscription = GetSubscription(piRegSubscription);
    m_pSubscription->Initialize();
    m_pSubscription->SetListener(this);

    StartSubscription(IMS_FALSE);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::DestroySubscription()
{
    if (!m_pUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, m_nFeature))
    {
        return IMS_FALSE;
    }

    m_pUtil->RemoveFeature(PENDING_SUBSCRIPTION, m_nTxnPending);

    if (m_pSubscription == IMS_NULL)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(REGID, "DestroySubscription", 0, 0, 0);

    m_pSubscription->Destroy();
    m_pSubscription = IMS_NULL;

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::StartSubscription(
        IN IMS_BOOL bIsRadioCheckRequired /* = IMS_TRUE */)
{
    if (!m_pUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, m_nFeature))
    {
        return IMS_FALSE;
    }

    if (m_pSubscription == IMS_NULL)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(REGID, "StartSubscription", 0, 0, 0);

    m_pSubscription->Start(bIsRadioCheckRequired);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::StopSubscription()
{
    if (!m_pUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, m_nFeature))
    {
        return IMS_FALSE;
    }

    if (m_pSubscription == IMS_NULL)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(REGID, "StopSubscription", 0, 0, 0);

    m_pSubscription->Stop();

    return IMS_TRUE;
}

PROTECTED VIRTUAL AosSubscription* AosRegistration::GetSubscription(
        IN IRegSubscription* piRegSubscription)
{
    return new AosSubscription(m_piContext, piRegSubscription,
            m_piRegistration->GetAuthorizedAor().ToString(), m_piRegContact->GetContactAddress());
}

PROTECTED VIRTUAL void AosRegistration::ProcessSubscription_Success()
{
    if (GET_N_CONFIG(m_nSlotId)->GetRegRetryCountResetPolicy() ==
            CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION)
    {
        ClearRetryCount(IMS_TRUE);
    }
}

PROTECTED VIRTUAL void AosRegistration::ProcessSubscription_Failed() {}

PROTECTED VIRTUAL void AosRegistration::ProcessSubscription_Terminated(
        IN IMS_SINT32 nTerminateType /* = 0 */)
{
    A_IMS_TRACE_I(
            REGID, "ProcessSubscription_Terminated, TerminateType : (%d)", nTerminateType, 0, 0);

    DestroySubscription();
}

PROTECTED VIRTUAL void AosRegistration::ProcessRegEventRegistered()
{
    if (GET_N_CONFIG(m_nSlotId)->GetRegRetryCountResetPolicy() ==
            CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_NOTIFY)
    {
        ClearRetryCount(IMS_TRUE);
    }
}

PROTECTED VIRTUAL void AosRegistration::Subscription_StateChanged(
        IN IMS_SINT32 nState, IN IMS_SINT32 nReason /*= 0 */)
{
    A_IMS_TRACE_I(
            REGID, "Subscription_StateChanged :: state(%d) , reason (%d)", nState, nReason, 0);

    if (m_pSubscription == IMS_NULL || m_piRegistration == IMS_NULL)
    {
        return;
    }

    switch (nReason)
    {
        case AosSubscription::REASON_SUB_ESTABLISHED:
            ProcessSubscription_Success();
            break;

        case AosSubscription::REASON_SUB_FAILED:
            ProcessSubscription_Failed();
            break;

        case AosSubscription::REASON_SUB_TERMINATED:
            PostMessage(MSG_SUB_TERMINATED, 0, 0);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::Subscription_CanBeTransmitted()
{
    if (!IsTransactionStarted())
    {
        m_pUtil->AddFeature(PENDING_SUBSCRIPTION, m_nTxnPending);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::Subscription_NotifyReceived(IN IMS_SINT32 nEvent)
{
    A_IMS_TRACE_I(REGID, "Subscription_NotifyReceived :: event (%d) ", nEvent, 0, 0);

    if (m_pSubscription == IMS_NULL || m_piRegistration == IMS_NULL)
    {
        return;
    }

    if (nEvent == AosSubscription::EVENT_REGISTERED)
    {
        PostMessage(MSG_REG_EVENT_REGISTERED, 0, 0);
    }
}

PROTECTED VIRTUAL void AosRegistration::Subscription_Request(IN IMS_SINT32 nCommand,
        IN IMS_SINT32 nRetryAfter /*= 0 */, IN IMS_BOOL bAwt /*= IMS_FALSE*/)
{
    A_IMS_TRACE_I(REGID, "Subscription_Request :: nCommand(%d), nRetryAfter(%d) bAwt(%s)", nCommand,
            nRetryAfter, _TRACE_B_(bAwt));

    switch (nCommand)
    {
        case AosSubscription::CMD_REG_REQUIRED:
        {
            m_eImsReasonCode = AosReasonCode::NETWORK_TRIGGERED_DEREGISTER;
            IMS_SINT32 nTime = nRetryAfter;
            if (bAwt)
            {
                IncreaseConsecutiveFailCount();
                nTime = GetActualWaitTime();
            }
            PostMessage(MSG_REG_REQUIRED_WITH_WAIT_TIME, nTime, 0);
            break;
        }
        case AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF:
            m_eImsReasonCode = AosReasonCode::NETWORK_TRIGGERED_DEREGISTER;
            PostMessage(MSG_REG_REQUIRED_WITH_NEXT_PCSCF, 0, 0);
            break;
        case AosSubscription::CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION:
            m_eImsReasonCode = AosReasonCode::NETWORK_TRIGGERED_DEREGISTER;
            PostMessage(MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION,
                    static_cast<IMS_UINT32>(nRetryAfter), 0);
            break;
        case AosSubscription::CMD_REG_REQUIRED_WITH_SUB_403_MSG_IN_WIFI:
        {
            m_eImsReasonCode = AosReasonCode::WFC_SUB_RESP_403;
            if (IsRegForbiddenInWifi())
            {
                PostMessage(MSG_REG_FORBIDDEN_IN_WIFI, 0, 0);
                break;
            }

            IMS_SINT32 nTime = nRetryAfter;
            if (bAwt)
            {
                IncreaseConsecutiveFailCount();
                nTime = GetActualWaitTime();
            }
            PostMessage(MSG_REG_REQUIRED_WITH_WAIT_TIME, nTime, 0);
            break;
        }
        case AosSubscription::CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG_IN_WIFI:
        {
            m_eImsReasonCode = AosReasonCode::WFC_SUB_NOTIFY_TERMINATED;
            if (IsRegForbiddenInWifi())
            {
                PostMessage(MSG_REG_FORBIDDEN_IN_WIFI, 0, 0);
                break;
            }

            IMS_SINT32 nTime = nRetryAfter;
            if (bAwt)
            {
                IncreaseConsecutiveFailCount();
                nTime = GetActualWaitTime();
            }
            PostMessage(MSG_REG_REQUIRED_WITH_WAIT_TIME, nTime, 0);
            break;
        }
        case AosSubscription::CMD_REG_TERMINATED:
            m_eImsReasonCode = AosReasonCode::NETWORK_TRIGGERED_DEREGISTER;
            PostMessage(MSG_REG_TERMINATED_BY_NOTIFY, 0, 0);
            break;
        case AosSubscription::CMD_SUB_REQUIRED:
            PostMessage(MSG_SUB_REINITIATE, 0, 0);
            break;
        case AosSubscription::CMD_SUB_TERMINATED:
            PostMessage(MSG_SUB_TERMINATED, 0, 0);
            break;
        case AosSubscription::CMD_RESET_SUB_RETRY_CNT_FOR_WIFI:
            m_nSubConsecutiveFailureForRegForbiddenInWifi = 0;
            break;
        default:
            break;
    }
}

PROTECTED VIRTUAL void AosRegistration::CreateIpsecHelper()
{
    DestroyIpsecHelper();

    m_pIpsecHelper = new AosIpsecHelper(m_piRegContact, m_piRegParameter, m_piContext, m_strRegId);
    m_pIpsecHelper->InitIpsec();
}

PROTECTED VIRTUAL void AosRegistration::DestroyIpsecHelper()
{
    if (m_pIpsecHelper != IMS_NULL)
    {
        delete m_pIpsecHelper;
        m_pIpsecHelper = IMS_NULL;
    }
}

PROTECTED VIRTUAL void AosRegistration::Block_Changed(
        IN IMS_UINT32 /* nType  = 0 */, IN IMS_UINT32 /* nParam  = 0 */)
{
    IMS_BOOL bCurrBlocked = !((m_piContext->GetConnection()->IsEpdgEnabled())
                    ? m_piContext->GetBlock()->IsCleared(SERVICE_WIFI)
                    : m_piContext->GetBlock()->IsCleared(SERVICE_CELLULAR));

    if (IsBlocked() != bCurrBlocked)
    {
        SetBlocked(bCurrBlocked);
        UpdateTransactionStarted();

        if (IsTransactionStarted())
        {
            ProcessPendingTransaction();
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::CallTracker_StateChanged(
        IN IMS_UINT32 nType, IN CallState eState)
{
    if (!IsValidCallType(nType))
    {
        return;
    }

    IMS_BOOL bImsCallStarted = IMS_FALSE;
    if (GET_N_CONFIG(m_nSlotId)->IsRegRequiredAfterImsECallEndOnRegHeld())
    {
        IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCt != IMS_NULL)
        {
            bImsCallStarted = piCt->IsNormalCallActive() || piCt->IsEmergencyCallActive();
        }
    }
    else
    {
        // IAosCallTracker::TYPE_NORMAL state only.
        bImsCallStarted = (eState > CallState::IDLE) ? IMS_TRUE : IMS_FALSE;
    }

    SetImsCall(bImsCallStarted);

    if (!IsImsCall() && IsHeldByCall())
    {
        SetHeldByCall(IMS_FALSE);
        UpdateTransactionStarted();

        if (m_pUtil->IsFeatureOn(PENDING_PLMN_BLOCK_HELD_BY_CALL, m_nTxnPending))
        {
            m_pUtil->RemoveFeature(PENDING_PLMN_BLOCK_HELD_BY_CALL, m_nTxnPending);

            if (ProcessPlmnBlockOnUpdateFailure())
            {
                return;
            }
        }

        if (GET_N_CONFIG(m_nSlotId)->IsRegRequiredAfterImsCallEndOnRegHeld() ||
                GET_N_CONFIG(m_nSlotId)->IsRegRequiredAfterImsECallEndOnRegHeld())
        {
            ProcessReinitiate();
            return;
        }

        if (IsTransactionStarted())
        {
            ProcessPendingTransaction();
        }
    }

    if (!IsImsCall() && (m_pUtil->IsFeatureOn(PENDING_UPDATE_HELD_BY_CALL, m_nTxnPending)))
    {
        m_pUtil->RemoveFeature(PENDING_UPDATE_HELD_BY_CALL, m_nTxnPending);
        Update();
    }
}

PROTECTED VIRTUAL void AosRegistration::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_D(REGID, "NConfiguration_NotifyConfigChanged", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL)
    {
        if (m_eRegType == AosRegistrationType::NORMAL)
        {
            if (GET_N_CONFIG(m_nSlotId)->IsSubscription())
            {
                m_pUtil->AddFeature(FEATURE_SUBSCRIPTION, m_nFeature);
            }
            else
            {
                m_pUtil->RemoveFeature(FEATURE_SUBSCRIPTION, m_nFeature);
            }
        }

        if (GET_N_CONFIG(m_nSlotId)->IsIpsecEnabled())
        {
            m_pUtil->AddFeature(FEATURE_IPSEC, m_nFeature);
        }
        else
        {
            m_pUtil->RemoveFeature(FEATURE_IPSEC, m_nFeature);
        }
        ClearIpsecBlock();

        IAosSubscriber* pSubscriber = m_piContext->GetSubscriber();
        if (pSubscriber != IMS_NULL)
        {
            pSubscriber->ClearTemporaryPublicUserIdForGiba();
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::Transaction_OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    if (nFailureReason == IImsRadio::REASON_ACCESS_DENIED)
    {
        Destroy();

        ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
        StartTimer(TIMER_OFFLINE_RECOVER, CONNECTION_FAILURE_RETRY_DEFAULT_WAIT_TIME * 1000);
    }
    else
    {
        if (IsConnectionFailureForOfflineRecovery(nFailureReason, nCauseCode))
        {
            A_IMS_TRACE_I(REGID,
                    "Transaction_OnConnectionFailed :: reason (%d), cause code (%d), wait time ms "
                    "(%d)",
                    nFailureReason, nCauseCode, nWaitTimeMillis);

            if (nWaitTimeMillis > (CONNECTION_FAILURE_RETRY_DEFAULT_WAIT_TIME * 1000))
            {
                DestroyEx();

                ReportStateChanged(RESULT_TRYING, REASON_TRYING_START);
                StartTimer(TIMER_OFFLINE_RECOVER, nWaitTimeMillis);
                return;
            }
        }

        Transaction_OnConnectionSetupPrepared();
    }
}

PROTECTED VIRTUAL void AosRegistration::Transaction_OnConnectionSetupPrepared()
{
    if (IsRadioWaiting())
    {
        SetRadioWaiting(IMS_FALSE);
        UpdateTransactionStarted();

        if (IsTransactionStarted())
        {
            ProcessPendingTransaction();
        }
    }
}

PROTECTED VIRTUAL void AosRegistration::Transaction_OnTrafficPriorityChanged()
{
    if (IsTrafficPriorityBlocked())
    {
        IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);

        if (piTransaction == IMS_NULL ||
                piTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG))
        {
            SetTrafficPriorityBlocked(IMS_FALSE);
            UpdateTransactionStarted();

            if (IsTransactionStarted() || m_pUtil->IsFeatureOn(PENDING_STOP, m_nTxnPending))
            {
                ProcessPendingTransaction();
            }
        }
    }
}

PROTECTED VIRTUAL IMS_RESULT AosRegistration::MessageMediator_AdjustMessage(
        IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage /* = MESSAGE_NORMAL */)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (AddLocationHeaderBody(piSipMsg, nMessage) == IMS_FALSE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL IMS_BOOL AosRegistration::AddLocationHeaderBody(
        IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage /* = MESSAGE_NORMAL*/)
{
    if (!IsGeolocationInfoRequired())
    {
        return IMS_FALSE;
    }

    if (nMessage == IMessageMediator::MESSAGE_RESUBMIT)
    {
        piSipMsg->RemoveHeader(ISipHeader::UNKNOWN, SipHeaderName::CONTENT_ID);
        piSipMsg->RemoveHeader(ISipHeader::CONTENT_TYPE);
        piSipMsg->RemoveHeader(ISipHeader::CONTENT_LENGTH);

        // Removes the previous message body
        piSipMsg->RemoveBodyParts();
    }

    const GeolocationPidfCreator* pPidfCreator =
            GeolocationHelper::GetInstance()->GetPidfCreator(m_nSlotId);

    if (pPidfCreator == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ByteArray objContent;
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_nSlotId)->GetGeolocationPidfFormingPolicy();

    if (nPolicy == CarrierConfig::Ims::GEOLOCATION_POLICY_WITHOUT_POSITION)
    {
        if (!pPidfCreator->CreateWithoutPosition(
                    AString::ConstNull(), IMS_FALSE, IMS_FALSE, objContent))
        {
            return IMS_FALSE;
        }
    }
    else if (nPolicy == CarrierConfig::Ims::GEOLOCATION_POLICY_WITH_POSITION)
    {
        if (!pPidfCreator->CreateWithPosition(AString::ConstNull(), objContent))
        {
            return IMS_FALSE;
        }
    }
    else if (nPolicy == CarrierConfig::Ims::GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY)
    {
        if (!pPidfCreator->CreateWithPositionAndCountry(AString::ConstNull(), objContent))
        {
            return IMS_FALSE;
        }
    }
    else if (nPolicy == CarrierConfig::Ims::GEOLOCATION_POLICY_WITHOUT_CIVIC)
    {
        if (!pPidfCreator->CreateWithoutCivic(AString::ConstNull(), objContent))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        return IMS_FALSE;
    }

    ISipMessageBodyPart* piBodyPart = piSipMsg->CreateBodyPart();

    // Set a Location
    piBodyPart->SetContent(objContent);

    // Set a Location Content-Type
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, "application/pidf+xml");

    AString strNewContentId = GeolocationHelper::CreateContentId(m_nSlotId);

    // Set a Location Content-ID
    AString strContentId;
    strContentId.Sprintf("<%s>", strNewContentId.GetStr());
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_ID, strContentId);

    // Set the Content-Length header
    AString strClen;
    strClen.SetNumber(objContent.GetLength());
    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_UNKNOWN, strClen, SipHeaderName::CONTENT_LENGTH);

    // Set Geolocation header
    AString strGeolocation;
    strGeolocation.Sprintf("<cid:%s>", strNewContentId.GetStr());
    piSipMsg->SetHeader(ISipHeader::GEOLOCATION, strGeolocation);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosRegistration::RegistrationControl_UpdateDataFailureReason(
        IN IMS_SINT32 /* nReason */)
{
    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return;
    }

    // TODO(b/383031808): Report dataFailCause as mExtraCode directly when deregistered due to PDN
    // disconnection
}

PRIVATE
void AosRegistration::ControlPrivateHeader()
{
    switch (GET_N_CONFIG(m_nSlotId)->GetRegistrationPrivateHeader())
    {
        case CarrierConfig::ImsWfc::REGISTRATION_P_CELLULAR_NETWORK_INFO:
            SetPcniHeader();
            break;
        case CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO:
            if (IsRegTypeEqual(AosRegistrationType::NORMAL))
            {
                SetPlaniHeader();
            }
            break;
        default:
            return;
    }
}

PRIVATE
IMS_UINT32 AosRegistration::GetSpecificErrWaitTime()
{
    ImsVector<IMS_SINT32>& objErrTime = GET_N_CONFIG(m_nSlotId)->GetExtraRegErrWaitTime();
    IMS_SINT32 nWaitTime = (objErrTime.GetSize() > 1) ? objErrTime.GetAt(0) : -1;

    return ((nWaitTime > 0) ? static_cast<IMS_UINT32>(nWaitTime) : RETRY_DEFAULT_WAIT_TIME);
}

PRIVATE
void AosRegistration::ProcessImsiBasedSubscriber()
{
    A_IMS_TRACE_D(REGID, "ProcessImsiBasedSubscriber", 0, 0, 0);

    if (SetNextPcscf())
    {
        StartTimer(TIMER_STOP_RETRY, GetSpecificErrWaitTime() * 1000);
        SetState(STATE_REGSTOP);
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
    }
    else
    {
        if (GET_N_CONFIG(m_nSlotId)->GetExtraRegErrFinalType() ==
                CarrierConfig::Ims::ERROR_TYPE_CRITICAL)
        {
            m_eImsReasonCode = AosReasonCode::PLMN_BLOCK;
        }

        Destroy();
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_FORBIDDEN);
    }
}

PRIVATE
void AosRegistration::SetContactAddressConfiguration(IN IMS_BOOL bAdd)
{
    if (!GET_N_CONFIG(m_nSlotId)->IsContactUriValidationChecked())
    {
        return;
    }

    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(m_nSlotId);

    if (piRtConfigHelper == IMS_NULL)
    {
        return;
    }

    if (bAdd)
    {
        if (m_piRegistration == IMS_NULL)
        {
            return;
        }

        const ISipMessage* piMessage = m_piRegistration->GetPreviousRequest();

        if (piMessage == IMS_NULL)
        {
            return;
        }

        SipRtConfig::RegContactAddress objContactAddress;

        objContactAddress.objUri = m_piRegistration->GetPreferredContact()->GetContactAddress();
        objContactAddress.strCallId = piMessage->GetHeader(ISipHeader::CALL_ID);
        piRtConfigHelper->SetConfig(SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS, &objContactAddress);
    }
    else
    {
        piRtConfigHelper->RemoveConfig(SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS, IMS_NULL);
    }
}

PRIVATE
void AosRegistration::SetPcniHeader()
{
    IMS_BOOL bSet = m_pIpsecHelper != IMS_NULL && m_pIpsecHelper->IsEstablished() &&
            GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN;

    ISipRtConfigHelper* piConfHelper = SipFactory::GetRtConfigHelper(m_nSlotId);
    if (bSet &&
            piConfHelper->GetHeader(AString(AosString::STR_P_CELLULAR_NETWORK_INFO)) != IMS_NULL)
    {
        return;
    }

    SipRtConfig::Header* pPcniHeader = new SipRtConfig::Header();
    pPcniHeader->strName = AosString::STR_P_CELLULAR_NETWORK_INFO;

    if (bSet)
    {
        piConfHelper->SetConfig(SipRtConfig::CONFIG_I_SIP_HEADER, pPcniHeader);
    }
    else
    {
        piConfHelper->RemoveConfig(SipRtConfig::CONFIG_I_SIP_HEADER, pPcniHeader);
    }

    delete pPcniHeader;
}

PRIVATE
void AosRegistration::SetPlaniHeader()
{
    IMS_BOOL bSet = IMS_FALSE;
    if (m_pIpsecHelper != IMS_NULL)
    {
        if ((m_pIpsecHelper->IsEstablished()) && (GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN))
        {
            bSet = IMS_TRUE;
        }
    }

    ISipRtConfigHelper* piConfHelper = SipFactory::GetRtConfigHelper(m_nSlotId);

    if (bSet)
    {
        if (piConfHelper->GetHeader(AString(AosString::STR_P_LAST_ACCESS_NETWORK_INFO)) != IMS_NULL)
        {
            return;
        }
    }

    SipRtConfig::Header* pPlaniHeader = new SipRtConfig::Header();
    pPlaniHeader->strName = AosString::STR_P_LAST_ACCESS_NETWORK_INFO;

    if (bSet)
    {
        pPlaniHeader->strParameter =
                SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetUtcFormat(IMS_TRUE);
        pPlaniHeader->strParameter.Replace(':', "%3A");
        pPlaniHeader->strParameter.Prepend('\"');
        pPlaniHeader->strParameter.Append('\"');

        piConfHelper->SetConfig(SipRtConfig::CONFIG_I_SIP_HEADER, pPlaniHeader);
    }
    else
    {
        piConfHelper->RemoveConfig(SipRtConfig::CONFIG_I_SIP_HEADER, pPlaniHeader);
    }

    delete pPlaniHeader;
}

PRIVATE
void AosRegistration::UpdateUserInfoInContact()
{
    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetUserInfoPolicyForNonRegisterMessage() ==
            CarrierConfig::Ims::CONTACT_USER_INFO_POLICY_NONE)
    {
        A_IMS_TRACE_D(REGID, "UpdateUserInfoInContact :: apply none policy", 0, 0, 0);
        m_piRegistration->SetUserInfoForContactHeader(AString::ConstEmpty());
    }
}

PRIVATE
void AosRegistration::UpdateModeToHandles()
{
    ImsMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);
        piHandle->Request(IAosHandle::TYPE_LIMITED_MODE,
                (GetMode() == MODE_LIMITED) ? IAosHandle::STATE_ADD : IAosHandle::STATE_REMOVE);
    }
}

PRIVATE
void AosRegistration::NotifyTechnologyChangeFailed()
{
    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    IMS_SINT32 nImsRegType = GetImsRegType();
    if (piService != IMS_NULL && nImsRegType != IAosRegistration::IMS_REG_TYPE_INVALID)
    {
        A_IMS_TRACE_D(REGID, "NotifyTechnologyChangeFailed :: RegType(%d), ImsReasonCode(%d)",
                nImsRegType, m_eImsReasonCode, 0);
        piService->NotifyTechnologyChangeFailed(
                nImsRegType, GetNetworkTypeForImsRegState(), m_eImsReasonCode);
    }
}

PRIVATE
IMS_BOOL AosRegistration::IsPdnReactivationRequired()
{
    IMS_SINT32 nCntForOnlyAttached =
            GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached();

    IMS_SINT32 nCntForCombinedAttached =
            GET_N_CONFIG(m_nSlotId)->GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached();

    if (nCntForOnlyAttached <= 0 || nCntForCombinedAttached <= 0)
    {
        return IMS_FALSE;
    }

    m_nConsecutiveFailureForPdnReactivated++;

    IMS_UINT32 nPdnReactivatedCnt = m_piContext->GetPcscf()->GetPcscfCount();

    if (m_bEps5GsOnly)
    {
        nPdnReactivatedCnt *= nCntForOnlyAttached;
    }
    else
    {
        nPdnReactivatedCnt *= nCntForCombinedAttached;
    }

    if (nPdnReactivatedCnt <= m_nConsecutiveFailureForPdnReactivated)
    {
        m_nPdnReactivateWaitTime = 0;
        m_nConsecutiveFailureForPdnReactivated = 0;
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AosRegistration::IsRegExpiredDuringAwt(IN IMS_UINT32 nAwt)
{
    IMS_SINT32 nExpireTime = GetRegExpires();
    IMS_SINT32 nActualWaitTime = static_cast<IMS_SINT32>(nAwt);

    if (nExpireTime > 0 && nActualWaitTime > 0)
    {
        IMS_SINT32 nRemainTime;

        if (nExpireTime > 1200)
        {
            nRemainTime = 600 - nActualWaitTime;
        }
        else
        {
            nRemainTime = (nExpireTime / 2) - nActualWaitTime;
        }

        A_IMS_TRACE_I(REGID, "IsRegExpiredDuringAwt :: Expire(%d) , AWT(%d) , Remain(%d)",
                nExpireTime, nActualWaitTime, nRemainTime);

        if (nRemainTime <= 0)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AosRegistration::IsNeedToSetLimitedMode()
{
    IMS_BOOL bResult = IMS_FALSE;

    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL &&
            GET_N_CONFIG(m_nSlotId)->IsSupportLimitedAdminSmsMode() &&
            m_eRegType == AosRegistrationType::NORMAL && !IsFakeRegistration())
    {
        bResult = IMS_TRUE;
    }

    A_IMS_TRACE_D(REGID, "IsNeedToSetLimitedMode : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PRIVATE
IMS_BOOL AosRegistration::IsUsimAuthFailureHandlingNeeded()
{
    return m_piContext->GetSubscriber()->IsUsim() &&
            IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrCode(),
                    CarrierConfig::Ims::REG_ERROR_CODE_USIM_AUTHENTICATION);
}
