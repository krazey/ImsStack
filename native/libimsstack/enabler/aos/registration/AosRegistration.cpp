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
#include "ServiceNetwork.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "ServiceVoNr.h"

#include "IIpcan.h"
#include "INetworkIpSec.h"
#include "ISipConfig.h"
#include "ISubscriberConfig.h"
#include "IVoNr.h"

#include "CarrierConfig.h"

#include "IRegContact.h"
#include "IRegistration.h"
#include "IRegParameter.h"
#include "IRegSubscription.h"
#include "ISIPHeader.h"
#include "ISIPRTConfigHelper.h"
#include "ISIPTransportHelper.h"
#include "Configuration.h"
#include "Credential.h"
#include "RegistrationManager.h"
#include "SIP.h"
#include "SIPConfigProxy.h"
#include "SIPFactory.h"
#include "SIPProfile.h"
#include "SIPRTConfig.h"
#include "SIPStatusCode.h"
#include "SIPHeaderName.h"

#include "GeolocationHelper.h"

#include "AoSAppRequestType.h"
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
#include "interface/IAosService.h"
#include "interface/IAosSubscriber.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosString.h"
#include "provider/AosTrm.h"
#include "provider/AosUtil.h"
#include "provider/AosVonr.h"
#include "registration/AosRegistration.h"
#include "registration/AosIpsecHelper.h"
#include "registration/AosSubscription.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define REGID m_strTag.GetStr()

PUBLIC
AosRegistration::AosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId)
    : IMSActivityEx(strRegId)
    , m_piContext(piAppContext)
    , m_piListener(IMS_NULL)
    , m_nSlotId(piAppContext->GetSlotId())
    , m_pRegManager(IMS_NULL)
    , m_piRegistration(IMS_NULL)
    , m_piRegContact(IMS_NULL)
    , m_piRegParameter(IMS_NULL)
    , m_pSubscription(IMS_NULL)
    , m_pIpsecHelper(IMS_NULL)
    , m_bIsIpsecSupported(IMS_FALSE)
    , m_bIsIpsecInit(IMS_FALSE)
    , m_pKeepAlive(IMS_NULL)
    , m_pUtil(IMS_NULL)
    , m_piTrm(IMS_NULL)
    , m_piVonr(IMS_NULL)
    , m_nFeature(FEATURE_NONE)
    , m_nState(STATE_OFFLINE)
    , m_nTxnPending(PENDING_NONE)
    , m_bIsTransactionStarted(IMS_TRUE)
    , m_bIsImsCall(IMS_FALSE)
    , m_bIsBlocked(IMS_FALSE)
    , m_bIsTrmBlocked(IMS_FALSE)
    , m_bIsHeldByCall(IMS_FALSE)
    , m_bIsAppReady(IMS_FALSE)
    , m_strRegId(strRegId)
    , m_eRegType(AosRegistrationType::NORMAL)
    , m_nFlowId(0)
    , m_bFakeReg(IMS_FALSE)
    , m_nRegMode(MODE_NORMAL)
    , m_nPcscfPort(0)
    , m_nPcscfIndex(0)
    , m_nRetryBaseTime(30)
    , m_nRetryMaxTime(1800)
    , m_nUpperBoundWaitTime(0)
    , m_nConsecutiveFailure(0)
    , m_nConsecutiveFailureForPdnReactivated(0)
    , m_nForbiddenCount(0)
    , m_piOfflineRecoverTimer(IMS_NULL)
    , m_piStopRetryTimer(IMS_NULL)
    , m_piRefreshTimer(IMS_NULL)
    , m_piExpiredTimer(IMS_NULL)
    , m_piModeTimer(IMS_NULL)
    , m_piTransactionTimer(IMS_NULL)
    , m_piInternalErrorTimer(IMS_NULL)
    , m_nAuthChallengeCount(0)
    , m_nErrorCountForServerSocket(0)
    , m_bCallingNumberVerificationSupported(IMS_FALSE)
    , m_nNetworkBindingFeatures(0)
    , m_nImsRegState(IMS_REG_STATE_DEREGISTERED)
    , m_nImsRegFeatures(ImsAosFeature::NONE)
    , m_pSipProfile(IMS_NULL)
    , m_nRegIpcanCategory(IIpcan::CATEGORY_MOBILE)
    , m_nPdnReactivateWaitTime(30)
{
    // Init Object
    m_pUtil = AosUtil::GetInstance();
    m_pRegManager = RegistrationManager::GetInstance();

    // registration type
    m_eRegType = piAppContext->GetStaticProfile()->GetRegistrationType();

    // registration flow ID
    m_nFlowId = piAppContext->GetStaticProfile()->GetRegistrationFlowId();

    m_strTag.Sprintf("%d:%s", m_nSlotId, m_strRegId.GetStr());

    IMS_CHAR acLog[128+1] = {0, };
    IMS_Sprintf(acLog, 128, "REG(%s)//FLOW(%d)", REGID, m_nFlowId);

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosRegistration = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosRegistration), this);
};

PUBLIC VIRTUAL
AosRegistration::~AosRegistration()
{
    IMS_CHAR acLog[128+1] = {0, };
    IMS_Sprintf(acLog, 128, "REG(%s)/FLOW(%d)", REGID, m_nFlowId);

    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosRegistration = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosRegistration), this);
}

PUBLIC VIRTUAL
void AosRegistration::Start()
{
    A_IMS_TRACE_I(REGID, "Start :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState),
            0, 0);

    // ONLY start in OFFLINE
    if (m_nState != STATE_OFFLINE)
    {
        Destroy();
    }

    StopTimer(TIMER_OFFLINE_RECOVER);

    if (!CheckTrmReadyAndSetTxnPending())
    {
        A_IMS_TRACE_I(REGID, "Start :: txn is pending due to TRM", 0, 0, 0);
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

PUBLIC VIRTUAL
void AosRegistration::Stop()
{
    A_IMS_TRACE_I(REGID, "Stop :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState),
            0, 0);

    switch (m_nState)
    {
        case STATE_OFFLINE: // FALL-THROUGH
        case STATE_REGISTERING: // FALL-THROUGH
        case STATE_REGSTOP: // FALL-THROUGH
        case STATE_REFRESHING:
            Destroy();
            ReportStateChanged(RESULT_SUCCESS);
            break;

        case STATE_REGISTERED: // FALL-THROUGH
        case STATE_REFRESHSTOP:
            StopSubscription();

            if (!SendDeregister())
            {
                Destroy();
                ReportStateChanged(RESULT_SUCCESS);
                return;
            }
            SetState(STATE_DEREGISTERING);
            ReportTryingState();
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL
void AosRegistration::Update(IN IMS_BOOL bIgnoreRetryTimer /* = IMS_FALSE */,
        IN IMS_BOOL bExplicitUpdate /* = IMS_TRUE */)
{
    A_IMS_TRACE_I(REGID, "Update :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState),
            0, 0);

    switch (m_nState)
    {
        case STATE_REGSTOP: // FALL-THROUGH
        case STATE_REFRESHSTOP:
            ProcessRetryInRegStopped(bIgnoreRetryTimer);
            break;

        case STATE_REGISTERED:
            ProcessReregister();
            break;

        case STATE_REGISTERING: // FALL-THROUGH
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

PUBLIC VIRTUAL
void AosRegistration::Reconfig()
{
    A_IMS_TRACE_I(REGID, "Reconfig :: state(%s)", AosProvider::GetLog()->RegStateToString(m_nState),
        0, 0);

    switch (m_nState)
    {
        case STATE_REGSTOP: // FALL-THROUGH
        case STATE_REFRESHSTOP:
            UpdateRegBinding();
            break;

        case STATE_REGISTERED:
            if (UpdateRegBinding())
            {
                ProcessReregister();
            }
            break;

        case STATE_REGISTERING: // FALL-THROUGH
        case STATE_REFRESHING:
            m_pUtil->AddFeature(PENDING_RECONFIG, m_nTxnPending);
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL
void AosRegistration::Destroy()
{
    ClearRegParameters();

    DestroyRegistration();

    DestroySocket();

    StopKeepAlive();

    SetState(STATE_OFFLINE);
}

PUBLIC VIRTUAL
void AosRegistration::SetListener(IN IAosRegistrationListener* piRegListener)
{
    m_piListener = piRegListener;
}

PUBLIC VIRTUAL
void AosRegistration::RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason /* = 0 */)
{
    A_IMS_TRACE_I(REGID, "RequestCmd :: nCmdType (%d), nReason (%d)", nCmdType, nReason, 0);

    switch (nCmdType)
    {
        case CMD_INIT_PCSCF:
            if (nReason == REASON_INIT_PCSCF_CLEAR)
            {
                m_nPcscfIndex = 0;
            }
            break;

        case CMD_INIT_AWT:
            SetRetryTime();
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

        case CMD_SCSCF_RESTORATION:
            ProcessScscfRestoration();
            break;

        case CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT:
            ClearErrorCount();
            break;

        case CMD_REPORT_REG_STATE:
            ReportRegState();
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL
IMS_UINT32 AosRegistration::GetMode()
{
    A_IMS_TRACE_I(REGID, "GetMode :: (%d)", m_nRegMode, 0, 0);
    return m_nRegMode;
}

PUBLIC VIRTUAL
IMS_UINT32 AosRegistration::GetProperty(IN IMS_UINT32 nType, OUT IMS_UINT32& nValue,
        OUT AString& strValue)
{
    IMSList<AString> strList;

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
                const AStringArray& objUris = m_piRegistration->GetAssociatedURIs();

                if (!objUris.IsEmpty())
                {
                    strValue = objUris.GetFirstElement();
                }
            }
            break;

        case PROPERTY_PATH:
            if (m_piRegistration != IMS_NULL)
            {
                ISIPMessage* piMessage = m_piRegistration->GetPreviousResponse();

                if (piMessage != IMS_NULL)
                {
                    strValue = piMessage->GetHeader(ISIPHeader::PATH);
                }
            }
            break;

        case PROPERTY_LAST_PATH:
            if (m_piRegistration != IMS_NULL)
            {
                ISIPMessage* piMessage = m_piRegistration->GetPreviousResponse();
                if (piMessage != IMS_NULL)
                {
                    strList = piMessage->GetHeaders(ISIPHeader::PATH);
                    if(strList.IsEmpty() == IMS_FALSE)
                    {
                        strValue = strList.GetAt(strList.GetSize()-1);
                    }
                }
            }
            break;

        case PROPERTY_SUPPORTED:
            if (m_piRegistration != IMS_NULL)
            {
                ISIPMessage* piMessage = m_piRegistration->GetPreviousRequest();

                if (piMessage != IMS_NULL)
                {
                    strValue = piMessage->GetHeader(ISIPHeader::SUPPORTED);
                }
            }
            break;

        case PROPERTY_SERVICE_ROUTE:
            if (m_piRegistration != IMS_NULL)
            {
                ISIPMessage* piMessage = m_piRegistration->GetPreviousResponse();

                if (piMessage != IMS_NULL)
                {
                    strValue = piMessage->GetHeader(ISIPHeader::SERVICE_ROUTE);
                }
            }
            break;

        case PROPERTY_PROTECTED:
            nValue = AoSRegProtectedType::REG_UNPROTECTED;

            if (GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN)
            {
                nValue = AoSRegProtectedType::REG_PROTECTED;
            }
            else if (GetRegIpcanCategory() == IIpcan::CATEGORY_MOBILE)
            {
                if ((m_pIpsecHelper != IMS_NULL) && m_pIpsecHelper->IsEstablished())
                {
                    nValue = AoSRegProtectedType::REG_PROTECTED;
                }
            }

            break;

        case PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION:
            if (m_bCallingNumberVerificationSupported)
            {
                nValue = AoSSupportability::SUPPORTED;
            }
            else
            {
                nValue = AoSSupportability::NOT_SUPPORTED;
            }
            break;

        case PROPERTY_NETWORK_BINDING_FEATURES:
            nValue = m_nNetworkBindingFeatures;
            break;

        case PROPERTY_PDN_REACIVATE_WAIT_TIME:
            nValue = m_nPdnReactivateWaitTime;
            break;

        default:
            break;
    }

    return 0;
}

PUBLIC VIRTUAL
IMS_UINT32 AosRegistration::GetState()
{
    A_IMS_TRACE_I(REGID, "GetState :: (%s)", AosProvider::GetLog()->RegStateToString(m_nState),
            0, 0);
    return m_nState;
}

PUBLIC VIRTUAL
AosRegistrationType AosRegistration::GetRegType()
{
    return m_eRegType;
}

PUBLIC VIRTUAL
IMS_BOOL AosRegistration::IsRegistered()
{
    return (IsRefreshing() || m_nState == STATE_REGISTERED);
}

PUBLIC VIRTUAL
IMS_BOOL AosRegistration::IsRefreshing()
{
    return (m_nState == STATE_REFRESHING || m_nState == STATE_REFRESHSTOP);
}

PUBLIC VIRTUAL
IMS_BOOL AosRegistration::IsRetryTimer()
{
    return (m_piStopRetryTimer != IMS_NULL || m_piOfflineRecoverTimer != IMS_NULL);
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::IsRetryHeld()
{
    return (m_nState == STATE_REGSTOP || m_nState == STATE_REFRESHSTOP);
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::IsTerminated()
{
    return m_pUtil->IsFeatureOn(PENDING_TERMINATED, m_nTxnPending);
}

PROTECTED VIRTUAL
void AosRegistration::SetAppReady(IN IMS_BOOL bReady)
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
            AosProvider::GetLog()->RegStateToString(this->m_nState),
            AosProvider::GetLog()->RegStateToString(m_nState), 0);

    m_nState = nState;

    if (m_eRegType == AosRegistrationType::NORMAL)
    {
        UpdateReason();
        UpdateDetailState(m_nState);
    }

    if (m_pUtil->IsFeatureOn(FEATURE_TRM, m_nFeature))
    {
        if (IsRegTypeEqual(AosRegistrationType::NORMAL))
        {
            m_piTrm->Set(IAosTrm::TYPE_REG, IsRegTrying());
        }
        else if (IsRegTypeEqual(AosRegistrationType::EMERGENCY))
        {
            IAosTrm* piAosTrm = AosProvider::GetInstance()->GetTrm(m_nSlotId);
            if (piAosTrm != IMS_NULL)
            {
                piAosTrm->SetEmegency(IAosTrm::TYPE_REG, IsRegTrying());
            }
        }
    }

    if (m_pUtil->IsFeatureOn(FEATURE_VONR, m_nFeature) && m_piVonr != IMS_NULL)
    {
        m_piVonr->Set(IAosVonr::MODULE_REG, IsRegTrying());
    }
}

PROTECTED
void AosRegistration::SetMode(IN IMS_UINT32 nMode)
{
    A_IMS_TRACE_I(REGID, "SetMode :: (%s)", AosProvider::GetLog()->RegModeToString(nMode), 0, 0);
    m_nRegMode = nMode;
}

PROTECTED
void AosRegistration::SetFakeReg(IN IMS_BOOL bFake)
{
    A_IMS_TRACE_I(REGID, "SetFakeReg :: (%s)", (bFake) ? "ON" : "OFF", 0, 0);
    m_bFakeReg = bFake;
}

PROTECTED
void AosRegistration::SetIsIpsecSupported(IN IMS_BOOL bSupported)
{
    A_IMS_TRACE_I(REGID, "SetIsIpsecSupported :: (%s)", (bSupported) ? "ON" : "OFF", 0, 0);
    m_bIsIpsecSupported = bSupported;
}

PROTECTED
void AosRegistration::SetIsIpsecInit(IN IMS_BOOL bInit)
{
    A_IMS_TRACE_I(REGID, "SetIsIpsecInit :: (%s)", (bInit) ? "TRUE" : "FALSE", 0, 0);
    m_bIsIpsecInit = bInit;
}

PROTECTED
void AosRegistration::SetBlocked(IN IMS_BOOL bBlocked)
{
    A_IMS_TRACE_I(REGID, "SetBlocked :: (%s)", (bBlocked) ? "BLOCK" : "NOT BLOCK", 0, 0);
    m_bIsBlocked = bBlocked;
}

PROTECTED
void AosRegistration::SetTrmBlocked(IN IMS_BOOL bTrmBlocked)
{
    A_IMS_TRACE_I(REGID, "SetTrmBlocked :: (%s)", (bTrmBlocked) ? "BLOCK" : "NOT BLOCK", 0, 0);
    m_bIsTrmBlocked = bTrmBlocked;
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
void AosRegistration::SetRetryTime()
{
    m_nRetryBaseTime = GET_N_CONFIG(m_nSlotId)->GetRegistrationRetryBaseTime() / 1000;
    m_nRetryMaxTime = GET_N_CONFIG(m_nSlotId)->GetRegistrationRetryMaxTime() / 1000;

    A_IMS_TRACE_I(REGID, "m_nRetryBaseTime (%d) , m_nRetryMaxTime (%d)",
            m_nRetryBaseTime, m_nRetryMaxTime, 0);
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

    return m_bIsIpsecSupported;
}

PROTECTED
IMS_BOOL AosRegistration::IsIpsecInit() const
{
    return m_bIsIpsecInit;
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
IMS_BOOL AosRegistration::IsTrmBlocked() const
{
    return m_bIsTrmBlocked;
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
IMS_BOOL AosRegistration::IsNetworkFeatureBindingSupported(IN IAosHandle* /* piHandle */)
{
    /* TODO_CONFIG : implement with different solution
    if (m_pUtil->IsFeatureOn(piHandle->GetServiceType(),
            m_piContext->GetConfig()->GetNetworkRegFeatureBindingSupportedServices()))
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
IMS_SINT32 AosRegistration::GetRegExpires()
{
    return (m_piRegContact != IMS_NULL) ? m_piRegContact->GetExpires() : -1;
}

PROTECTED
void AosRegistration::IncreaseConsecutiveFailCount()
{
    m_nConsecutiveFailure++;
}

PROTECTED
IMS_BOOL AosRegistration::UpdatePreloadedRoute(IN const AString& strPcscf
        /* = AString::ConstNull() */, IN const IMS_UINT32 nPcscfPort /* = 0 */)
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

    if (m_nFeature & FEATURE_TRM)
    {
        strFeature += "FEATURE_TRM | ";
    }

    if (m_nFeature & FEATURE_TRM_BLOCK)
    {
        strFeature += "FEATURE_TRM_BLOCK | ";
    }

    if (m_nFeature & FEATURE_VONR)
    {
        strFeature += "FEATURE_VONR | ";
    }

    return strFeature;
}

PROTECTED
AosNetworkType AosRegistration::GetNetworkTypeForImsRegState()
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
        case NW_REPORT_RADIO_WCDMA: // FALL-THROUGH
        case NW_REPORT_RADIO_HSPA:
            return AosNetworkType::UTRAN;

        default:
            return AosNetworkType::NONE;
    }
}

PROTECTED
IMS_SINT32 AosRegistration::GetRegIpcanCategory()
{
    return m_nRegIpcanCategory;
}

PROTECTED
IMS_UINT32 AosRegistration::GetRegFeatures()
{
    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
    IMS_UINT32 nFeatures = ImsAosFeature::NONE;

    for (IMS_UINT32 i = 0; i < objHandles.GetSize(); ++i)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(i);

        if (piHandle->GetRequestType() == IAosHandle::ATTACH && piHandle->IsRegBinded())
        {
            nFeatures |= piHandle->GetBindedFeatureTagList().GetFeatures();
        }
    }

    A_IMS_TRACE_I(REGID, "GetRegFeatures :: (%d)", nFeatures, 0, 0);

    return nFeatures;
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
        case STATE_OFFLINE:
            nImsRegState = IMS_REG_STATE_DEREGISTERED;
            break;

        default:
            return;
    }

    IMS_UINT32 nImsRegFeatures = GetRegFeatures();
    if (m_nImsRegState == nImsRegState)
    {
        if (m_nImsRegState == IMS_REG_STATE_DEREGISTERED)
        {
            return;
        }
        if (m_nImsRegFeatures == nImsRegFeatures)
        {
            return;
        }
    }

    m_nImsRegState = nImsRegState;
    m_nImsRegFeatures = nImsRegFeatures;

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        if (m_nImsRegState == IMS_REG_STATE_DEREGISTERED)
        {
            piService->NotifyDeregistered(AosReasonCode::UNSPECIFIED);
        }
        else if (m_nImsRegState == IMS_REG_STATE_REGISTERING)
        {
            IMSList<AString> objFeatureTags = IMSList<AString>();
            piService->NotifyRegistering(GetNetworkTypeForImsRegState(),
                    m_nImsRegFeatures, objFeatureTags);
        }
        else if (m_nImsRegState == IMS_REG_STATE_REGISTERED)
        {
            IMSList<AString> objFeatureTags = IMSList<AString>();
            piService->NotifyRegistered(GetNetworkTypeForImsRegState(),
                    m_nImsRegFeatures, objFeatureTags);
        }
    }
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::OnMessage(IN IMSMSG& objMsg)
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

        case MSG_REG_TERMINATED_BY_NOTIFY:
            ProcessRegTerminatedByNotify();
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

PROTECTED VIRTUAL
void AosRegistration::Init()
{
    A_IMS_TRACE_D(REGID, "Init", 0, 0, 0);

    InitFeatures();

    if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        m_piContext->GetNetTracker()->SetListener(this);
    }
    else
    {
        m_piContext->GetBlock()->SetListener(this);
    }

    if (IsCallStateRequired())
    {
        IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCt != IMS_NULL)
        {
            piCt->SetListener(this);
        }
    }

    if (m_pUtil->IsFeatureOn(FEATURE_TRM_BLOCK, m_nFeature))
    {
        m_piTrm = new AosTrm(m_nSlotId);
        AosProvider::GetInstance()->SetTrm(m_piTrm, m_nSlotId);
        m_piTrm->SetListener(this);
    }

    if (m_pUtil->IsFeatureOn(FEATURE_IPSEC, m_nFeature))
    {
        INetworkIpSec* piNetIpsec = NetworkService::GetNetworkService()->GetIpSec();
        if (piNetIpsec != IMS_NULL)
        {
            piNetIpsec->SetSdbFlushCapability(IMS_TRUE);
            piNetIpsec->FlushPolicies();
        }
    }

    if (m_pUtil->IsFeatureOn(FEATURE_VONR, m_nFeature))
    {
        m_piVonr = new AosVonr(m_piContext);
        AosProvider::GetInstance()->SetVonr(m_piVonr, m_nSlotId);
    }
}

PROTECTED VIRTUAL
void AosRegistration::InitFeatures()
{
    if (GET_N_CONFIG(m_nSlotId)->IsSubscription())
    {
        m_pUtil->AddFeature(FEATURE_SUBSCRIPTION, m_nFeature);
    }

    if (GET_N_CONFIG(m_nSlotId)->IsIpsecEnabled())
    {
        SetIsIpsecSupported(IMS_TRUE);
        m_pUtil->AddFeature(FEATURE_IPSEC, m_nFeature);
   }

    ITrm* piPhoneTrm = PhoneInfoService::GetPhoneInfoService()->GetTrm();
    if (piPhoneTrm != IMS_NULL && piPhoneTrm->IsTrmSupported())
    {
        m_pUtil->AddFeature(FEATURE_TRM, m_nFeature);

        if (IsRegTypeEqual(AosRegistrationType::NORMAL))
        {
            m_pUtil->AddFeature(FEATURE_TRM_BLOCK, m_nFeature);
        }
    }

    if (IsRegTypeEqual(AosRegistrationType::NORMAL))
    {
        IVoNr* piServiceVonr  = VoNrService::GetVoNrService()->GetVoNr(m_nSlotId);
        if (piServiceVonr != IMS_NULL && piServiceVonr->IsVoNrSupported())
        {
            m_pUtil->AddFeature(FEATURE_VONR, m_nFeature);
        }
    }

    A_IMS_TRACE_I(REGID, "InitFeature :: features (%s)", FeatureToString().GetStr(), 0, 0);
}

PROTECTED VIRTUAL
void AosRegistration::CleanUp()
{
    A_IMS_TRACE_D(REGID, "CleanUp", 0, 0, 0);

    Destroy();

    StopTimer(TIMER_OFFLINE_RECOVER);

    if (m_piVonr != IMS_NULL)
    {
        AosProvider::GetInstance()->SetVonr(IMS_NULL, m_nSlotId);
        delete DYNAMIC_CAST(AosVonr*, m_piVonr);
    }

    if (m_piTrm != IMS_NULL)
    {
        m_piTrm->RemoveListener(this);
        AosProvider::GetInstance()->SetTrm(IMS_NULL, m_nSlotId);
        delete DYNAMIC_CAST(AosTrm*, m_piTrm);
    }

    IAosCallTracker* piCt = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCt != IMS_NULL)
    {
        piCt->RemoveListener(this);
    }

    if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        IAosNetTracker* piNt = m_piContext->GetNetTracker();
        if (piNt != IMS_NULL)
        {
            piNt->RemoveListener(this);
        }
    }
    else
    {
        IAosBlock* piBlock = m_piContext->GetBlock();
        if (piBlock != IMS_NULL)
        {
            piBlock->RemoveListener(this);
        }
    }
}

PROTECTED VIRTUAL
void AosRegistration::DestroyEx()
{
    ClearRegParameters(IMS_FALSE);

    DestroyRegistration();

    DestroySocket();

    StopKeepAlive();

    SetState(STATE_OFFLINE);
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::IsGeolocationInfoRequired()
{
    IMS_BOOL bRequired = IMS_FALSE;

    if (GetRegType() == AosRegistrationType::NORMAL)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfSupported(
                CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
        {
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() &&
                    GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN)
            {
                bRequired = IMS_TRUE;
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
    else if (GetRegType() == AosRegistrationType::EMERGENCY)
    {
        if (GET_N_CONFIG(m_nSlotId)->IsGeolocationPidfSupported(
                CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
        {
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() &&
                    GetRegIpcanCategory() == IIpcan::CATEGORY_WLAN)
            {
                bRequired = IMS_TRUE;
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::IsHandlingServerSocketErrorRequired(IN IMS_SINT32 nReason)
{
    if ((nReason == IRegistration::REASON_SERVER_SOCKET_ERROR) &&
            IsRegistered() && (GetRegType() == AosRegistrationType::NORMAL))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL
void AosRegistration::ReportStateChanged(IN IMS_UINT32 nResult, IN IMS_UINT32 nReason /* = 0 */)
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->Registration_StateChanged(nResult, nReason);
    }
}

PROTECTED VIRTUAL
void AosRegistration::PreNotify(IN IMS_UINT32 nReason)
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->Registration_PreNotify(nReason);
    }
}

PROTECTED VIRTUAL
void AosRegistration::ReportTryingState()
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

    if (m_pRegManager->GetRegistration(m_nSlotId, m_nFlowId) != IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "PrepareRegistration :: destroy registration for creating", 0, 0, 0);

        IMSList<IRegContact*> objContactList = m_piRegistration->GetAllContacts();

        for (IMS_UINT32 nAt = 0; nAt < objContactList.GetSize(); ++nAt)
        {
            m_piRegistration->DestroyContact(objContactList.GetAt(nAt));
        }

        m_pRegManager->DestroyRegistration(m_piRegistration);
        m_piRegistration = IMS_NULL;

        DestroySubscription();
    }

    if (IsIpsecInit()) // TODO : check init condition
    {
        if (GET_N_CONFIG(m_nSlotId)->IsIpsecEnabled())
        {
            SetIsIpsecSupported(IMS_TRUE);
        }
        else
        {
            SetIsIpsecSupported(IMS_FALSE);
        }
    }

    DestroySocket();
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::CreateRegistration()
{
    PrepareRegistration();

    SetRetryTime();

    if (!SetAor())
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_D(REGID, "CreateRegistration :: m_nFlowId (%d), strAoR (%s)",
            m_nFlowId, m_strPuid.GetStr(), 0);

    if (m_pRegManager->CreateRegistration(m_nFlowId, m_strPuid, IsFakeRegistration()) == IMS_FALSE)
    {
        A_IMS_TRACE_I(REGID, "create reg is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piRegistration = m_pRegManager->GetRegistration(m_nSlotId, m_nFlowId);

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
    IPAddress objPcscf(m_strPcscf);
    m_objIpa = piConnection->GetLocalAddress((objPcscf.IsIPv6Address())
            ? IPAddress::IPV6: IPAddress::IPV4);

    CreateContact();

    if (m_piRegContact == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "contact is null", 0, 0, 0);
        return IMS_FALSE;
    }

    StartRegBinding();

    AddSpecificOperation();

    m_piRegParameter = m_piRegistration->GetParameter();

    if (UpdatePreloadedRoute() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    SetDefaultTransport();

    SetStaticIpQos();

    SetActiveBindingsRestorationUsage();

    if (IsIpsecSupported())
    {
        CreateIpsecHelper();
    }

    SetTcpCriterionLength();

    return SendRegister(IMS_FALSE, IMS_TRUE);
}

PROTECTED VIRTUAL
void AosRegistration::DestroyRegistration()
{
    A_IMS_TRACE_I(REGID, "DestroyRegistration", 0, 0, 0);

    ClearAuthChallengedCount();
    ClearPending();
    ClearNetworkBindingFeatures();

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
    m_pRegManager->DestroyRegistration(m_piRegistration);

    m_piRegistration = IMS_NULL;
    m_piRegContact = IMS_NULL;
    m_piRegParameter = IMS_NULL;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::StartRegBinding()
{
    // attach services (add and create binding)
    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::UpdateRegBinding()
{
    if (m_piRegContact == IMS_NULL || m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "UpdateRegBinding :: reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
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
            else // IAosHandle::ATTACH
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
                    A_IMS_TRACE_I(REGID, "UpdateRegBinding :: service(%d) is not binded "
                            "by network", piHandle->GetServiceType(), 0, 0);
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
        UpdateFinalAddFeatureTag();
        m_piRegContact->RecalculateCallerCapabilities();
    }

    return bChanged;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::UpdateNetworkRegBinding()
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

    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::UpdateNetworkRegFeatureBinding()
{
    return IMS_FALSE;

#if 0 // TODO : update impl.
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

    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SendRegister(IN IMS_BOOL bRestore /* = IMS_FALSE */,
        IN IMS_BOOL bInitial /* = IMS_FALSE */)
{
    A_IMS_TRACE_I(REGID, "SendRegister", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        A_IMS_TRACE_D(REGID, "reg is null", 0, 0, 0);
        return IMS_FALSE;
    }

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

    if (IsIpsecSupported())
    {
        m_pIpsecHelper->Create(bInitial);
    }

    if (m_piRegistration->Register() == IMS_FAILURE)
    {
        A_IMS_TRACE_I(REGID, "register is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SendRegisterEx(IN IMS_SINT32 nMinExpireValue,
        IN IMS_BOOL bAddHalfExpireValue /* = IMS_FALSE */)
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

    if (m_piRegistration->Register(nExpireValue) == IMS_FAILURE)
    {
        A_IMS_TRACE_I(REGID, "register is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SendDeregister()
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

    if (IsIpsecSupported())
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::AddOperation_OnSendRegister()
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::AddOperation_OnSendDeregister()
{
    ControlPrivateHeader();

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosRegistration::AddOperation_OnNotifyAkaResponse()
{
    ControlPrivateHeader();

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void AosRegistration::CreateContact()
{
    m_piRegContact = m_piRegistration->CreateContact(m_objIpa, m_pUtil->GetLocalPort(m_nSlotId));

    // the child class will set contact parameter
}

PROTECTED VIRTUAL
void AosRegistration::AddSpecificOperation()
{
    if (m_eRegType == AosRegistrationType::EMERGENCY)
    {
         m_piRegContact->AddUriParameter("sos");
    }

    AddAccesstypeFeatureTag();
}

PROTECTED VIRTUAL
void AosRegistration::AddAccesstypeFeatureTag()
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
        strValue = (m_piContext->GetConnection()->IsEpdgEnabled()) ?
                AosString::STR_ACCESS_TYPE_WLAN1 : AosString::STR_ACCESS_TYPE_CELLULAR2;
    }
    else // CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE
    {
        strValue = (m_piContext->GetConnection()->IsEpdgEnabled()) ?
                AosString::STR_ACCESS_TYPE_WLAN : AosString::STR_ACCESS_TYPE_CELLULAR;
    }

    m_piRegContact->AddHeaderParameter(AosString::STR_ACCESS_TYPE_FEATURE, strValue);
}

PROTECTED VIRTUAL
void AosRegistration::AddFeatureTag(IN IAosHandle* piHandle)
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
        objBindedList.CopyFeatures(objList.GetFeatures());
    }

    if (piHandle->GetServiceType() == ImsAosService::MTC ||
            piHandle->GetServiceType() == ImsAosService::EMERGENCY_MTC)
    {
        AddFeatureTagForMtc(piHandle->GetBindedFeatureTagList().GetFeatures(), IMS_FALSE);
    }
}

PROTECTED VIRTUAL
void AosRegistration::AddFeatureTagForMtc(IN IMS_UINT32 nRegFeatures, IN IMS_BOOL bFinalFeatureTag)
{
    // If bFinalFeatureTag is true, the extra header shouldn't be added
    // because it manages as reference count
    if ((nRegFeatures & ImsAosFeature::VIDEO) > 0)
    {
        m_pUtil->UpdateFeatureTagOptions(ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO, IMS_TRUE,
                m_nSlotId);
    }
    if (((nRegFeatures & ImsAosFeature::TEXT) > 0) && bFinalFeatureTag == IMS_FALSE)
    {
        m_piRegContact->AddExtraCapability(AosString::STR_RTT_FEATURE, AString::ConstNull());
    }
    if (((nRegFeatures & ImsAosFeature::USSI) > 0) && bFinalFeatureTag == IMS_FALSE)
    {
        m_piRegContact->AddExtraCapability(AosString::STR_USSI_FEATURE, AString::ConstNull());
    }
    if ((nRegFeatures & ImsAosFeature::VERSTAT) > 0)
    {
        m_piRegContact->AddHeaderParameter(AosString::STR_VERSTAT_FEATURE);
    }
}

PROTECTED VIRTUAL
void AosRegistration::RemoveFeatureTag(IN IAosHandle* piHandle)
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

PROTECTED VIRTUAL
void AosRegistration::RemoveFeatureTagForMtc(IN IMS_UINT32 nRegFeatures)
{
    if ((nRegFeatures & ImsAosFeature::VIDEO) > 0)
    {
        m_pUtil->UpdateFeatureTagOptions(ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO, IMS_FALSE,
                m_nSlotId);
    }
    if ((nRegFeatures & ImsAosFeature::TEXT) > 0)
    {
        m_piRegContact->RemoveExtraCapability(AosString::STR_RTT_FEATURE, AString::ConstNull());
    }
    if ((nRegFeatures & ImsAosFeature::USSI) > 0)
    {
        m_piRegContact->RemoveExtraCapability(AosString::STR_USSI_FEATURE, AString::ConstNull());
    }
    if ((nRegFeatures & ImsAosFeature::VERSTAT) > 0)
    {
        m_piRegContact->RemoveHeaderParameter(AosString::STR_VERSTAT_FEATURE);
    }
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::UpdateFeatureTag(IN IAosHandle* piHandle)
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

PROTECTED VIRTUAL
void AosRegistration::UpdateFinalAddFeatureTag()
{
    A_IMS_TRACE_I(REGID, "UpdateFinalAddFeatureTag", 0, 0, 0);

    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();
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
                        m_piRegContact->AddHeaderParameter(pFeature->GetName(),
                                pFeature->GetValue());
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SetAor()
{
    IAosSubscriber* pSubscriber = m_piContext->GetSubscriber();

    AStringArray objImpu;

    if (IsFakeRegistration() == IMS_FALSE)
    {
        objImpu = pSubscriber->GetConfiguredImpus();
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
            A_IMS_TRACE_D(REGID, "SetAor :: GetAssociatedURIs from normal registration", 0, 0, 0);
            IRegistration* piRegistration = RegistrationManager::GetInstance()->GetRegistration(
                    m_nSlotId, static_cast<IMS_UINT32>(AosRegistrationFlowId::NORMAL));
            objImpu = (piRegistration == IMS_NULL) ? pSubscriber->GetFakeImpus()
                    : piRegistration->GetAssociatedURIs();
        }
    }

    if (objImpu.IsEmpty())
    {
        A_IMS_TRACE_I(REGID, "SetAor :: No Puids", 0, 0, 0);
        return IMS_FALSE;
    }
    else
    {
        m_strPuid = objImpu.GetElementAt(0);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SetPcscf()
{
    IAosPcscf* piPcscf = m_piContext->GetPcscf();

    // TODO : remove m_nPcscfIndex and use from AosPcscf class
    if (!piPcscf->HasPcscf(m_nPcscfIndex))
    {
        ClearPcscf();
        if (!piPcscf->HasPcscf(m_nPcscfIndex))
        {
            A_IMS_TRACE_I(REGID, "SetPcscf :: invalid index", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    const AStringArray& objPcscfs = piPcscf->GetPcscfs();
    const IMSList<IMS_SINT32>& objPcscfPorts = piPcscf->GetPcscfsPorts();

    m_strPcscf = objPcscfs.GetElementAt(m_nPcscfIndex);
    m_nPcscfPort = objPcscfPorts.GetAt(m_nPcscfIndex);

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void AosRegistration::SetRefreshPolicy()
{
    m_piRegistration->SetRefreshPolicy(IRegistration::REFRESH_POLICY_SPEC, 1200, 50, 600);
}

PROTECTED VIRTUAL
void AosRegistration::SetFailureState()
{
    SetState((GetState() == STATE_REFRESHING) ? STATE_REFRESHSTOP : STATE_REGSTOP);
}

PROTECTED VIRTUAL
void AosRegistration::SetRetryState()
{
    SetState((GetState() == STATE_REFRESHSTOP) ? STATE_REFRESHING: STATE_REGISTERING);
}

PROTECTED VIRTUAL
void AosRegistration::SetTcpCriterionLength()
{
    if (GET_N_CONFIG(m_nSlotId)->GetSipPreferredTransport() !=
            CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP)
    {
        return;
    }

    IMS_SINT32 nMtu = m_piContext->GetConnection()->GetMtu();
    IMS_SINT32 nSipThresholdSize =
        GET_N_CONFIG(m_nSlotId)->GetSipMessageThresholdForTransportChange();

    IMS_UINT32 nLength = 0;

    if (nMtu > 0)
    {
        if (nMtu > nSipThresholdSize)
        {
            nLength = nMtu - nSipThresholdSize;
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() && m_objIpa.IsIPv6Address())
            {
                if (nLength > SIP_MTU_MAX_SIZE_VIA_WIFI)
                {
                    nLength = SIP_MTU_MAX_SIZE_VIA_WIFI;
                }
            }
        }

    }
    else
    {
        nLength = (m_objIpa.IsIPv6Address()) ? GET_N_CONFIG(m_nSlotId)->GetIpv6MtuSize() :
                GET_N_CONFIG(m_nSlotId)->GetIpv4MtuSize();
    }

    if (nLength == 0)
    {
        nLength = (nSipThresholdSize > SIP_MTU_MAX_SIZE_VIA_WIFI) ? SIP_MTU_MAX_SIZE_VIA_WIFI :
                nSipThresholdSize;
    }

    A_IMS_TRACE_I(REGID, "SetTcpCriterionLength :: TCP length (%d), MTU (%d), Threshold (%d)",
            nLength, nMtu, nSipThresholdSize);

    if (m_pSipProfile.IsNull())
    {
        m_pSipProfile = new SIPProfile();
    }

    m_pSipProfile->SetTcpCriterionLength(nLength);
    m_piRegistration->SetSIPProfile(m_pSipProfile.Get());
}

PROTECTED VIRTUAL
void AosRegistration::SetDefaultTransport()
{
}

PROTECTED VIRTUAL
void AosRegistration::SetStaticIpQos()
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

    ISIPRTConfigHelper* piRtConfigHelper = SIPFactory::GetRTConfigHelper(m_nSlotId);
    if (piRtConfigHelper != IMS_NULL)
    {
        piRtConfigHelper->RemoveConfig(SIPRTConfig::CONFIG_I_IP_QOS, IMS_NULL);

        SIPRTConfig::IPQoS objIpQos;
        A_IMS_TRACE_I(REGID, "SetStaticIpQos : Set DSCP to %d", nDscp, 0, 0);

        objIpQos.nValue = nDscp << 2;
        objIpQos.objIP = m_objIpa;
        objIpQos.nPort = 0;

        piRtConfigHelper->SetConfig(SIPRTConfig::CONFIG_I_IP_QOS, &objIpQos);
    }
}

PROTECTED VIRTUAL
void AosRegistration::SetDynamicIpQos()
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

    ISIPTransportHelper* piHelper = SIPFactory::GetTransportHelper(m_nSlotId);
    if (piHelper != IMS_NULL)
    {
        SIPRTConfig::IPQoS objIpQos;
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

        objIpQos.objIP = m_objIpa;
        objIpQos.nPort = 0;

        piHelper->SetIPQoS(&objIpQos);
    }
}

PROTECTED VIRTUAL
void AosRegistration::SetActiveBindingsRestorationUsage()
{
    if (GetRegType() != AosRegistrationType::NORMAL)
    {
        return;
    }

    if (m_piRegistration != IMS_NULL)
    {
        m_piRegistration->SetActiveBindingsRestorationUsage(IMS_TRUE);
    }
}

PROTECTED VIRTUAL
void AosRegistration::UpdateTransactionStarted()
{
    if (m_eRegType == AosRegistrationType::EMERGENCY)
    {
        m_bIsTransactionStarted = IsImsCall();
    }
    else
    {
        m_bIsTransactionStarted = !(IsBlocked() || IsHeldByCall() || IsTrmBlocked());
    }

    A_IMS_TRACE_I(REGID, "UpdateTransactionStarted :: (%s)",
            (m_bIsTransactionStarted) ? "READY" : "NOT READY", 0, 0);
}

PROTECTED VIRTUAL
IMS_UINT32 AosRegistration::GetActualWaitTime()
{
    if (GET_N_CONFIG(m_nSlotId)->GetRegistrationActualWaitTimePolicy() ==
            CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        const IMSVector<IMS_SINT32>& objInterval =
                GET_N_CONFIG(m_nSlotId)->GetRegistrationRetryIntervals();
        IMS_UINT32 nSize = objInterval.GetSize();

        if (nSize > 0)
        {
            IMS_SINT32 nAt = (m_nConsecutiveFailure > nSize) ? nSize: m_nConsecutiveFailure;

            const IMSVector<IMS_SINT32>& objUpperRandom =
                    GET_N_CONFIG(m_nSlotId)->GetRegistrationRandomRetryIntervals();

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

    return m_pUtil->WaitTimeForFlowRecovery(m_nRetryBaseTime,
            m_nRetryMaxTime, m_nConsecutiveFailure);
}

PROTECTED VIRTUAL
IMS_UINT32 AosRegistration::GetUpperBoundTime()
{
    A_IMS_TRACE_I(REGID,
            "GetUpperBoundTime :: max-time(%d), base-time(%d), consecutive-failures (%d)",
            m_nRetryMaxTime, m_nRetryBaseTime, m_nConsecutiveFailure);

    return m_pUtil->CalculateUpperBoundTime(m_nRetryBaseTime, m_nRetryMaxTime,
            m_nConsecutiveFailure);
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SetFirstPcscf(IN IMS_BOOL bUpdateParameter /* = IMS_TRUE */)
{
    AString strPcscf;
    IMS_UINT32 nPort;

    if (m_piContext->GetPcscf()->GetFirstPcscf(strPcscf, nPort) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    m_strPcscf = strPcscf;
    m_nPcscfPort = nPort;

    m_nPcscfIndex = 0;

    if (!bUpdateParameter)
    {
        return IMS_TRUE;
    }

    return UpdatePreloadedRoute();
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::SetNextPcscf(IN IMS_BOOL bUpdateParameter /* = IMS_TRUE */)
{
    AString strPcscf;
    IMS_UINT32 nPort;

    if (m_piContext->GetPcscf()->GetNextPcscf(strPcscf, nPort) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    m_nPcscfIndex = m_piContext->GetPcscf()->GetCurrentIndex();

    m_strPcscf = strPcscf;
    m_nPcscfPort = nPort;

    if (!bUpdateParameter)
    {
        return IMS_TRUE;
    }

    return UpdatePreloadedRoute();
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::TryNextPcscf()
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::TryNextPcscf(IN IMS_BOOL bFlowRecoveryOnAllFail,
        IN IMS_BOOL bHonorRetryAfter /* = IMS_FALSE */)
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::IsNextPcscf()
{
    return m_piContext->GetPcscf()->HasNextPcscf();
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::IsRetryStopped()
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

PROTECTED VIRTUAL
void AosRegistration::ClearRegParameters(IN IMS_BOOL bClearPcscf /* = IMS_TRUE */)
{
    ClearTimers();
    ClearRetryValues();
    ClearAuthChallengedCount();
    ClearPending();
    ClearCallingNumberVerification();

    if (bClearPcscf)
    {
        ClearPcscf();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ClearPcscf()
{
    m_nPcscfIndex = 0;

    IAosPcscf* piPcscf = m_piContext->GetPcscf();
    if (piPcscf != IMS_NULL)
    {
        piPcscf->SetFirstPcscfIndex();
        piPcscf->ResetAllPcscfTried();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ClearRetryValues(IN IMS_BOOL bRegSuccess /* = IMS_FALSE */)
{
    if (bRegSuccess)
    {
        m_nConsecutiveFailureForPdnReactivated = 0;
        m_nForbiddenCount = 0;
    }

    m_nConsecutiveFailure = 0;
    m_nUpperBoundWaitTime = 0;
}

PROTECTED VIRTUAL
void AosRegistration::ClearAuthChallengedCount()
{
    m_nAuthChallengeCount = 0;
}

PROTECTED VIRTUAL
void AosRegistration::ClearErrorCount()
{
    m_nErrorCountForServerSocket = 0;
}

PROTECTED VIRTUAL
void AosRegistration::ClearNetworkBindingFeatures()
{
    m_nNetworkBindingFeatures = 0;
}

PROTECTED VIRTUAL
void AosRegistration::DestroySocket()
{
}

PROTECTED VIRTUAL
void AosRegistration::ReportRegState()
{
}

PROTECTED VIRTUAL
void AosRegistration::CheckPending()
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::CheckTrmReadyAndSetTxnPending()
{
    if (m_pUtil->IsFeatureOn(FEATURE_TRM_BLOCK, m_nFeature))
    {
        if(!m_piTrm->IsReady())
        {
            SetTrmBlocked(IMS_TRUE);
            UpdateTransactionStarted();
            m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void AosRegistration::ProcessSetIpsec(IN IMS_UINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "ProcessSetIpsec :: (%d)", nReason, 0, 0);

    if (nReason == IAosRegistration::REASON_SET_IPSEC_INIT)
    {
        SetIsIpsecInit(IMS_TRUE);
    }
    else if (nReason == IAosRegistration::REASON_SET_IPSEC_ENABLE)
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

PROTECTED VIRTUAL
void AosRegistration::ProcessRefreshRegInfo()
{
    A_IMS_TRACE_I(REGID, "ProcessRefreshRegInfo", 0, 0, 0);

    if (m_piRegContact != IMS_NULL)
    {
        m_piRegContact->RecalculateCallerCapabilities();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessIpcanChanged()
{
    A_IMS_TRACE_I(REGID, "ProcessIpcanChanged()", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId)->IsRegistrationWhenIpcanChangedWithImsActiveCallHeld()
             && IsImsCall())
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
    }
    else
    {
        Update();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessScscfRestoration()
{
    A_IMS_TRACE_I(REGID, "ProcessScscfRestoration()", 0, 0, 0);

    IAosPcscf* piPcscf = m_piContext->GetPcscf();

    if (piPcscf == IMS_NULL)
    {
        return;
    }

    piPcscf->RemoveCurrentPcscf();

    Destroy();

    if (piPcscf->GetPcscfCount() > 0)
    {
        Start();
    }
    else
    {
        // make p-cscf discovery
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessPendingTransaction()
{
    A_IMS_TRACE_I(REGID, "ProcessPendingTransaction", 0, 0, 0);

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
                if (!SendRegister((GetState() == STATE_REFRESHSTOP) ? IMS_FALSE : IMS_TRUE))
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

PROTECTED VIRTUAL
void AosRegistration::ProcessRetryInRegStopped(IN IMS_BOOL bIgnoreTimer /* = IMS_FALSE */)
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

    if (!CheckTrmReadyAndSetTxnPending())
    {
        A_IMS_TRACE_I(REGID, "ProcessRetryInRegStopped :: txn is pending due to TRM", 0, 0, 0);
        return;
    }

    if (!SendRegister((m_nState == STATE_REGSTOP) ? IMS_TRUE : IMS_FALSE))
    {
        ProcessUnpredictableFailure();
        return;
    }

    SetState((m_nState == STATE_REGSTOP) ? STATE_REGISTERING : STATE_REFRESHING);
    ReportTryingState();
}

PROTECTED VIRTUAL
void AosRegistration::ProcessReregister()
{
    if (m_nState != STATE_REGISTERED)
    {
        return;
    }

    if (!CheckTrmReadyAndSetTxnPending())
    {
        A_IMS_TRACE_I(REGID, "ProcessReregister :: txn is pending due to TRM", 0, 0, 0);
        SetState(STATE_REFRESHSTOP);
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

PROTECTED VIRTUAL
void AosRegistration::ProcessReinitiate(IN IMS_BOOL bClearPcscf /* = IMS_TRUE */)
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

PROTECTED VIRTUAL
void AosRegistration::ProcessUpdatePending()
{
    if (m_pUtil->IsFeatureOn(PENDING_UPDATE, m_nTxnPending))
    {
        m_pUtil->RemoveFeature(PENDING_UPDATE, m_nTxnPending);
        Update();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessReconfigPending()
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

PROTECTED VIRTUAL
void AosRegistration::ProcessUnpredictableFailure()
{
    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_INTERNAL);
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::ProcessUnpredictableFailureHeldByCall()
{
    if (IsRegTypeEqual(AosRegistrationType::NORMAL) &&
            IsRegistered() && IsImsCall())
    {
        SetHeldByCall(IMS_TRUE);
        UpdateTransactionStarted();
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        SetState(STATE_REFRESHSTOP);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL
void AosRegistration::ProcessRegTerminated()
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
        Destroy();
        A_IMS_TRACE_D(REGID, "ProcessRegTerminated :: ignore and restore after expiring timer",
            0, 0, 0);
        return;
    }

    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_TERMINATED);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessRegTerminatedByNotify()
{
    // TODO : need the block permanently
    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_TERMINATED);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessAuthenticationFailed()
{

    if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() ==
            CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED)
    {
        if (GetState() == STATE_REGISTERING)
        {
            ProcessDefaultFlowRecovery_Start();
            return;
        }

        if (GetState() == STATE_REFRESHING)
        {
            ProcessDefaultFlowRecovery_Update();
            return;
        }
    }

    Destroy();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_AUTHENTICATION);
}


PROTECTED VIRTUAL
void AosRegistration::ProcessRegRequiredWithWaitTime(IN IMS_SINT32 nWaitTime)
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

PROTECTED VIRTUAL
void AosRegistration::ProcessRegRequiredWithNextPcscf()
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
    }

    ClearPcscf();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_TERMINATED);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessSubReinitiate()
{
    DestroySubscription();
    StartSubscription();
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::ProcessForbiddenFailed(IN IMS_SINT32 nStatusCode)
{
    if (!IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrCode(), nStatusCode))
    {
        return IMS_FALSE;
    }

    m_nForbiddenCount++;

    IMS_SINT32 nMaxCount = (GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrMaxCount().GetSize() > 0) ?
            GET_N_CONFIG(m_nSlotId)->GetRegPermanentErrMaxCount().GetAt(0) : 1;


    if (m_nForbiddenCount >= nMaxCount)
    {
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::ProcessSubscriberFailed(IN IMS_SINT32 nStatusCode)
{
    if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() !=
            CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED)
    {
        return IMS_FALSE;
    }

    if (IsRegistered())
    {
        if (!IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetSpecificReregistrationErrorCode(),
                nStatusCode))
        {
            return IMS_FALSE;
        }
        else
        {
            if (m_nConsecutiveFailure == 1 && GetState() == STATE_REFRESHING)
            {
                A_IMS_TRACE_I(REGID,
                        "ProcessSubscriberFailed :: ignore subscriber failure", 0, 0, 0);
                ClearRetryValues();
                PostMessage(MSG_REG_REINITIATE, 0, 0);
                return IMS_TRUE;
            }
        }
    }
    else
    {
        if (!IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorCode(),
                nStatusCode))
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
                const SIPAddress objAor(m_strPuid);

                m_piRegistration->SetAOR(objAor);
                m_piContext->GetPcscf()->SetAllPcscfValid();

                ProcessImsiBasedSubscriber();

                SetMode(MODE_LIMITED);
                UpdateModeToHandles();
            }
            return IMS_TRUE;
        }
    }

    ProcessImsiBasedSubscriber();
    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::ProcessAkaResponseFailed()
{
    return m_pIpsecHelper->Create(IMS_FALSE);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessAwtRecovery()
{
    A_IMS_TRACE_I(REGID, "ProcessAwtRecovery", 0, 0, 0);

    IncreaseConsecutiveFailCount();

    StartTimer(TIMER_STOP_RETRY, GetActualWaitTime() * 1000);

    SetFailureState();
    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessIpsecFallback(IN IMS_BOOL bIsIpsecRetry)
{
    A_IMS_TRACE_I(REGID, "ProcessIpsecFallback", 0, 0, 0);

    if ((bIsIpsecRetry && !IsIpsecSupported()) || (!bIsIpsecRetry && IsIpsecSupported()))
    {
        Destroy();
        SetIsIpsecSupported(bIsIpsecRetry);

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

PROTECTED VIRTUAL
void AosRegistration::ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode /* 0 */)
{
    A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Start", 0, 0, 0);

    IMS_SINT32 nAwtPolicy = GET_N_CONFIG(m_nSlotId)->GetRegistrationActualWaitTimePolicy();
    IMS_UINT32 nRetryAfter = 0;
    IMS_UINT32 nAwt = 0;

    if (GET_N_CONFIG(m_nSlotId)->IsRegErrCodeWithRetryAfterTimeOnlyDefined())
    {
        if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetRegErrCodeWithRetryAfterTime(),
                nStatusCode))
        {
            nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);
        }
    }
    else
    {
        nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);
    }

    if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EVERY_PCSCF)
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
        return;
    }
    else
    {
        IncreaseConsecutiveFailCount();
    }

    if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        if (nRetryAfter > 0)
        {
            nAwt = nRetryAfter;
        }
        else
        {
            nAwt = GetActualWaitTime();
        }

        m_piContext->GetPcscf()->SetCurrentPcscfTried();
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
        return;
    }
    else
    {
        nAwt = GetActualWaitTime();
    }

    if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EACH_PCSCF)
    {
        if (SetNextPcscf())
        {
            IMS_UINT32 nRetryTime = (nRetryAfter > 0) ? nRetryAfter : nAwt;
            StartTimer(TIMER_STOP_RETRY, nRetryTime * 1000);

            SetState(STATE_REGSTOP);
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
            return;
        }
    }
    else
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE,
                (nRetryAfter > 0) ? nRetryAfter : nAwt);
        if (TryNextPcscf())
        {
            SetState(STATE_REGISTERING);
            ReportTryingState();
            return;
        }
    }


    SetState(STATE_REGSTOP);

    if (nAwtPolicy == CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EACH_PCSCF)
    {
        m_nPdnReactivateWaitTime = 0;
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
    }
    else
    {
        m_nPdnReactivateWaitTime = (nRetryAfter == 0 || nRetryAfter > nAwt) ? nAwt : nRetryAfter;
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT_WITH_AWT);
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode /* = 0 */)
{
    A_IMS_TRACE_I(REGID, "ProcessDefaultFlowRecovery_Update", 0, 0, 0);

    IMS_SINT32 nAwtPolicy = GET_N_CONFIG(m_nSlotId)->GetRegistrationActualWaitTimePolicy();
    IMS_UINT32 nRetryAfter = 0;

    if (GET_N_CONFIG(m_nSlotId)->IsRegErrCodeWithRetryAfterTimeOnlyDefined())
    {
        if (IsErrorCodeExisted(GET_N_CONFIG(m_nSlotId)->GetReregErrCodeWithRetryAfterTime(),
                nStatusCode))
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
        if (nStatusCode == SIPStatusCode::SC_481)
        {
            ProcessReinitiate(IMS_FALSE);
            return;
        }

        IncreaseConsecutiveFailCount();

        if (nRetryAfter == 0)
        {
            nRetryAfter = GetActualWaitTime();
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

            /* TODO: check to need about outage case
            if (IsRegFailedWithOutage(nStatusCode))
            {
                ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_OUTAGE);
                return;
            }
            */
        }

        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_GENERAL);
    }
    else
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
        }
        else
        {
            ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
        }
    }
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::ProcessStartFailed_305()
{
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_nSlotId)->GetRegistrationRetrySip305CodePolicy();

    if (nPolicy == CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP)
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
        ProcessStandardPcscfSelection();
        return IMS_TRUE;
    }
    else if (nPolicy == CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF)
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

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_403()
{
}

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_420()
{
    IMS_BOOL bIsExtensionUnsupported =
            m_pUtil->IsParameterIncluded(m_piRegistration->GetPreviousResponse(),
            ISIPHeader::UNSUPPORTED, AosString::STR_SEC_AGREE);

    if (bIsExtensionUnsupported)
    {
        ProcessIpsecFallback(IMS_FALSE);
    }
    else
    {
        ProcessDefaultFlowRecovery_Start();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_421()
{
    IMS_BOOL bIsExtensionRequired = m_pUtil->IsParameterIncluded(
            m_piRegistration->GetPreviousResponse(), ISIPHeader::REQUIRE, AosString::STR_SEC_AGREE);

    if (m_pUtil->IsFeatureOn(FEATURE_IPSEC, m_nFeature) && bIsExtensionRequired)
    {
        ProcessIpsecFallback(IMS_TRUE);
    }
    else
    {
        ProcessDefaultFlowRecovery_Start();
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_423()
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

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_503()
{
    /* legacy impl
    if (!m_piContext->GetConfig()->IsStandardSpecificationSupported())
    {
        ProcessDefaultFlowRecovery_Start(SIPStatusCode::SC_503);
        return;
    }
    */

    IncreaseConsecutiveFailCount();
    IMS_UINT32 nRetryAfter = m_pUtil->GetRetryAfterValue(m_piRegistration);

    if (nRetryAfter == 0)
    {
        m_piContext->GetPcscf()->SetCurrentPcscfInvalid();
        ProcessStandardPcscfSelection();
    }
    else
    {
        IMS_SINT32 nTimerF = SIPConfigProxy::GetTimerValueTF(m_nSlotId, IMS_NULL,
                Configuration::GetInstance()->GetSipConfig(m_nSlotId)->GetSipConfigV(), IMS_TRUE);

        A_IMS_TRACE_I(REGID, "ProcessStartFailed_503 :: TF (%d), RA (%d)",
                nTimerF, nRetryAfter, 0);

        if (nTimerF > 0)
        {
            if (nRetryAfter > static_cast<IMS_UINT32>(nTimerF))
            {
                m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE, nRetryAfter);
                ProcessStandardPcscfSelection();
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
            ProcessDefaultFlowRecovery_Start(SIPStatusCode::SC_503);
        }
    }
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::ProcessUpdateFailed_305()
{
    IMS_SINT32 nPolicy = GET_N_CONFIG(m_nSlotId)->GetReregistrationRetrySip305CodePolicy();

    if (nPolicy == CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP)
    {
        ProcessDefaultFlowRecovery_Update(SIPStatusCode::SC_305);
        return IMS_TRUE;
    }
    else if (nPolicy == CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF)
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

PROTECTED VIRTUAL
void AosRegistration::ProcessUpdateFailed_403()
{
}

PROTECTED VIRTUAL
void AosRegistration::ProcessUpdateFailed_423()
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

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessStartFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    if (ProcessForbiddenFailed(nStatusCode) || ProcessSubscriberFailed(nStatusCode))
    {
        return;
    }

    if (nStatusCode == SIPStatusCode::SC_305)
    {
        if (ProcessStartFailed_305())
        {
            return;
        }
    }

    if (GET_N_CONFIG(m_nSlotId)->GetRegistrationRetrySip503CodePolicy() ==
            CarrierConfig::Assets::SIP_503_CODE_POLICY_3GPP &&
            nStatusCode == SIPStatusCode::SC_503)
    {
        ProcessStartFailed_503();
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() ==
            CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED)
    {
        if (SIPStatusCode::IsFinalFailure(nStatusCode))
        {
            if (IsErrorCodeExistedForSpecificRegistration(nStatusCode) ||
                    IsErrorCodeExistedForSpecificRegistration(nStatusCode / 100))
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
        case SIPStatusCode::SC_420:
            ProcessStartFailed_420();
            break;
        // 421
        case SIPStatusCode::SC_421:
            ProcessStartFailed_421();
            break;
            break;
        // 423
        case SIPStatusCode::SC_423:
            ProcessStartFailed_423();
            break;

        default:
            // Other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Start(nStatusCode);
            break;
    }

}

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_TxnTimeout()
{
    if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() ==
            CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED)
    {
        if (IsErrorCodeExistedForSpecificRegistration(
                CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F))
        {
            if (IsPdnReactivationRequired())
            {
                return;
            }
        }

        ProcessDefaultFlowRecovery_Start();
        return;
    }

    if (IsErrorCodeExistedForSpecificRegistration(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F))
    {
        ProcessDefaultFlowRecovery_Start();
        return;
    }

    IncreaseConsecutiveFailCount();

    IMS_UINT32 nAwt = GetActualWaitTime();
    m_piContext->GetPcscf()->SetCurrentPcscfInvalid(IMS_TRUE, nAwt);

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

PROTECTED VIRTUAL
void AosRegistration::ProcessStartFailed_Others(IN IMS_SINT32 nReason)
{
    // TODO: add the recovery for REASON_INTERNAL_ERROR
    if (IsErrorCodeExistedForSpecificRegistration(CarrierConfig::Assets::REG_ERROR_CODE_OTHER))
    {
        ProcessDefaultFlowRecovery_Start();
        return;
    }

    if (nReason == IRegistration::REASON_CLIENT_SOCKET_ERROR)
    {
        if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() ==
                CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED)
        {
            if (IsErrorCodeExistedForSpecificRegistration(
                    CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT))
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

PROTECTED VIRTUAL
void AosRegistration::ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode)
{
    A_IMS_TRACE_I(REGID, "ProcessUpdateFailed_StatusCode :: Code(%d) ", nStatusCode, 0, 0);

    if (ProcessUnpredictableFailureHeldByCall())
    {
        return;
    }

    if (ProcessForbiddenFailed(nStatusCode) || ProcessSubscriberFailed(nStatusCode))
    {
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(
            m_nSlotId)->GetReregRetryErrCodeWithInitialRegWithSamePcscf(),
            CarrierConfig::Assets::REG_ERROR_CODE_ALL_RESP))
    {
        ProcessDefaultFlowRecovery_Update(nStatusCode);
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() ==
            CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED)
    {
        ProcessDefaultFlowRecovery_Update(nStatusCode);
        return;
    }

    if (nStatusCode == SIPStatusCode::SC_305)
    {
        if (ProcessUpdateFailed_305())
        {
            return;
        }
    }

    switch (nStatusCode)
    {
        case SIPStatusCode::SC_403: // FALL-THROUGH
        case SIPStatusCode::SC_408: // FALL-THROUGH
        case SIPStatusCode::SC_500: // FALL-THROUGH
        case SIPStatusCode::SC_504:
            ProcessReinitiate(IMS_FALSE);
            return;

        default:
            break;
    }

    switch (nStatusCode)
    {
        // 423
        case SIPStatusCode::SC_423:
            ProcessUpdateFailed_423();
            break;

        default:
            // other 4xx, 5xx, 6xx response
            ProcessDefaultFlowRecovery_Update(nStatusCode);
            break;
    }

}

PROTECTED VIRTUAL
void AosRegistration::ProcessUpdateFailed_TxnTimeout()
{
    if (ProcessUnpredictableFailureHeldByCall())
    {
        return;
    }

    ProcessDefaultFlowRecovery_Update();
}

PROTECTED VIRTUAL
void AosRegistration::ProcessUpdateFailed_Others(IN IMS_SINT32 nReason)
{
    if (GET_N_CONFIG(m_nSlotId)->GetRegistrationActualWaitTimePolicy() ==
            CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL)
    {
        ProcessDefaultFlowRecovery_Update();
        return;
    }

    if (IsErrorCodeExisted(GET_N_CONFIG(
            m_nSlotId)->GetReregRetryErrCodeWithInitialRegWithSamePcscf(),
            CarrierConfig::Assets::REG_ERROR_CODE_OTHER))
    {
        ProcessDefaultFlowRecovery_Update();
        return;
    }

    if (nReason == IRegistration::REASON_CLIENT_SOCKET_ERROR)
    {
        if (GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorPolicy() ==
                CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED)
        {
            if (IsErrorCodeExistedForSpecificRegistration(
                    CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT))
            {
                m_nConsecutiveFailureForPdnReactivated++;
                ProcessDefaultFlowRecovery_Update();
                return;
            }
        }
    }

    ProcessAwtRecovery();
}

PROTECTED VIRTUAL
void AosRegistration::ProcessStandardPcscfSelection()
{
    if (TryNextPcscf())
    {
        SetState(STATE_REGISTERING);
        ReportTryingState();
        return;
    }

    ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_PDN_RECONNECT);
}

PROTECTED VIRTUAL
void AosRegistration::RecordImpu()
{
    if (m_eRegType != AosRegistrationType::NORMAL)
    {
        return;
    }

    ISubscriberInfo* piSubsInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_nSlotId);
    const AStringArray& objAssociatedUris = m_piRegistration->GetAssociatedURIs();

    IMS_CHAR strSize[4];
    IMS_Itoa(strSize, objAssociatedUris.GetCount(), 10);

    piSubsInfo->SetPreference("impu_list", "size", AString(strSize), PREFERENCE_VALUE_STRING);
    A_IMS_TRACE_I(REGID, "RecordImpu :: size (%d) m_nSlotId(%d)", objAssociatedUris.GetCount(),
            m_nSlotId, 0);

    IMSList<AString> objUris = IMSList<AString>();
    for (IMS_SINT32 i = 0; i < objAssociatedUris.GetCount() && (i < 9); i++)
    {
        IMS_CHAR strKey[4];
        IMS_Itoa(strKey, i, 10);

        SIPAddress objAssociatedUri(objAssociatedUris.GetElementAt(i));

        AString strAor = objAssociatedUri.GetURI();

        if (strAor == AString::ConstNull() || strAor.IsEmpty())
        {
            continue;
        }

        piSubsInfo->SetPreference("impu_list", AString(strKey), strAor, PREFERENCE_VALUE_STRING);
        objUris.Append(strAor);

        A_IMS_TRACE_D(REGID, "RecordImpu :: key(%d) value(%s) m_nSlotId(%d)", i, strAor.GetStr(),
                m_nSlotId);
    }

    if (objUris.GetSize() > 0)
    {
        IAosService* piService = AosProvider::GetInstance()->GetService();
        if (piService != IMS_NULL)
        {
            piService->NotifyAssociatedUriChanged(objUris);
        }
    }
}

PROTECTED VIRTUAL
void AosRegistration::Registration_AuthenticationChallenged(IN IMS_SINT32 nAlgorithm,
        OUT IMS_BOOL& bResponseToChallenge)
{
    A_IMS_TRACE_I(REGID, "Registration_AuthenticationChallenged", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    m_nAuthChallengeCount++;

    if (!IsAuthChallengeMoreAllowed())
    {
        bResponseToChallenge = IMS_FALSE;
        return;
    }

    bResponseToChallenge = IMS_TRUE;

    if (IsIpsecSupported())
    {
        if (!m_pIpsecHelper->ProcessAuthChallenged(nAlgorithm))
        {
            bResponseToChallenge = IMS_FALSE;
            SetIsIpsecSupported(IMS_FALSE);

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

PROTECTED VIRTUAL
void AosRegistration::Registration_NotifyAKAResponse(IN IMS_SINT32 nResult,
        IN const ByteArray& objIK, IN const ByteArray& objCK, OUT IMS_BOOL& bResultOfSA)
{
    A_IMS_TRACE_I(REGID, "Registration_NotifyAKAResponse :: result(%d)", nResult, 0, 0);

    bResultOfSA = IMS_FALSE;

    if (!IsIpsecSupported())
    {
        return;
    }

    if(nResult != IMS_AKA::RESULT_OK)
    {
        A_IMS_TRACE_I(REGID, "Aka response is failed , wait next 401 message", 0, 0, 0);
        bResultOfSA = IMS_TRUE;

        if (!ProcessAkaResponseFailed())
        {
            ProcessRegTerminated();
            return;
        }
    }
    else
    {
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

        if (!AddOperation_OnNotifyAkaResponse())
        {
            bResultOfSA = IMS_FALSE;
            return;
        }
    }
}

PROTECTED VIRTUAL
void AosRegistration::Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh)
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

    CheckTrmReadyAndSetTxnPending();

    if (!IsTransactionStarted())
    {
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        SetState(STATE_REFRESHSTOP);
        return;
    }

    if (IsIpsecSupported())
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

PROTECTED VIRTUAL
void AosRegistration::Registration_Started()
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

    if (IsIpsecSupported())
    {
        if (m_pIpsecHelper->IsEstablished())
        {
            m_pIpsecHelper->ProcessRegStarted();
        }
        else
        {
            A_IMS_TRACE_I(REGID, "Registration_Started :: ipsec is not established", 0, 0, 0);
            SetIsIpsecSupported(IMS_FALSE);
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

    StartKeepAlive();
}

PROTECTED VIRTUAL
void AosRegistration::Registration_StartFailed(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "Registration_StartFailed :: (%s)",
            AosProvider::GetLog()->RegReasonToString(nReason), 0, 0);

    IMS_SINT32 nStatusCode = -1;

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

    if (IsIpsecSupported())
    {
        m_pIpsecHelper->ProcessRegStartFailed();
    }

    nStatusCode = m_pUtil->GetResponseCode(m_piRegistration->GetPreviousResponse());

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

PROTECTED VIRTUAL
void AosRegistration::Registration_Updated()
{
    A_IMS_TRACE_I(REGID, "Registration_Updated", 0, 0, 0);

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    ClearRetryTimers();
    ClearRetryValues(IMS_TRUE);
    ClearAuthChallengedCount();

    if (IsIpsecSupported())
    {
        if (!m_pIpsecHelper->ProcessRegUpdated())
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
            StartSubscription();
        }
    }

    UpdateCallingNumberVerification();

    UpdateNetworkRegBinding();

    UpdateNetworkRegFeatureBinding();

    ReportStateChanged(RESULT_SUCCESS);

    CheckPending();

    RecordImpu();

    StopKeepAlive();
    StartKeepAlive();
}

PROTECTED VIRTUAL
void AosRegistration::Registration_UpdateFailed(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "Registration_UpdateFailed :: (%s)",
            AosProvider::GetLog()->RegReasonToString(nReason), 0, 0);

    IMS_SINT32 nStatusCode = -1;

    if (!IsAuthChallengeMoreAllowed())
    {
        ProcessAuthenticationFailed();
        return;
    }

    ClearAuthChallengedCount();

    nStatusCode = m_pUtil->GetResponseCode(m_piRegistration->GetPreviousResponse());

    switch (nReason)
    {
        case IRegistration::REASON_STATUS_CODE:
            ProcessUpdateFailed_StatusCode(nStatusCode);
            break;

        case IRegistration::REASON_TRANSACTION_TIMEOUT:
            ProcessUpdateFailed_TxnTimeout();
            break;

        default:
            ProcessUpdateFailed_Others(nReason);
            break;
    }

    CheckPending();

    StopKeepAlive();
}

PROTECTED VIRTUAL
void AosRegistration::Registration_Removed()
{
    A_IMS_TRACE_I(REGID, "Registration_Removed", 0, 0, 0);

    Destroy();
    ReportStateChanged(RESULT_SUCCESS);
}

PROTECTED VIRTUAL
void AosRegistration::Registration_Terminated(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(REGID, "Registration_Terminated :: (%s)",
            AosProvider::GetLog()->RegReasonToString(nReason), 0, 0);

    if (IsHandlingServerSocketErrorRequired(nReason))
    {
        if (IsReconnectingServerSocketErrorAllowed())
        {
            ++m_nErrorCountForServerSocket;
            A_IMS_TRACE_I(REGID, "server socket error count (%d)",
                    m_nErrorCountForServerSocket, 0, 0);

            StartTimer(TIMER_INTERNAL_ERROR, INTERNAL_ERROR_INTERVAL * 1000);
        }
        else
        {
            A_IMS_TRACE_I(REGID, "re-initiate registration due to max server socket error",
                    0, 0, 0);

            ClearErrorCount();
            PostMessage(MSG_REG_REINITIATE, 0, 0);
        }

        return;
    }

    ProcessRegTerminated();
}

PROTECTED VIRTUAL
void AosRegistration::ProcessOfflineRecoverTimerExpired()
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

    CheckTrmReadyAndSetTxnPending();

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
    else
    {
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        return;
    }

}

PROTECTED VIRTUAL
void AosRegistration::ProcessStopRetryTimerExpired()
{
    StopTimer(TIMER_STOP_RETRY);

    if (!IsRetryHeld())
    {
        return;
    }

    if (IsImsCall())
    {
        SetHeldByCall(IMS_TRUE);
        UpdateTransactionStarted();
    }

    CheckTrmReadyAndSetTxnPending();

    if (IsTransactionStarted())
    {
        if (SendRegister((GetState() == STATE_REFRESHSTOP) ? IMS_FALSE : IMS_TRUE))
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
    else
    {
        m_pUtil->AddFeature(PENDING_TRANSACTION, m_nTxnPending);
        return;
    }
}

PROTECTED VIRTUAL
void AosRegistration::ProcessRefreshTimerExpired()
{
    StopTimer(TIMER_REFRESH);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessExpiredTimerExpired()
{
    StopTimer(TIMER_EXPIRED);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessModeTimerExpired()
{
    StopTimer(TIMER_MODE);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessTransactionTimerExpired()
{
    StopTimer(TIMER_TRANSACTION);
}

PROTECTED VIRTUAL
void AosRegistration::ProcessInternalErrorTimerExpired()
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

PROTECTED VIRTUAL
void AosRegistration::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
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

        case TIMER_EXPIRED:
            ppiTimer = &m_piExpiredTimer;
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

    *ppiTimer = m_pUtil->StartTimer(nDuration, this,
            AosProvider::GetLog()->RegTimerToString(nType));
}

PROTECTED VIRTUAL
void AosRegistration::StopTimer(IN IMS_UINT32 nType)
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

        case TIMER_EXPIRED:
            ppiTimer = &m_piExpiredTimer;
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

PROTECTED VIRTUAL
void AosRegistration::ClearRetryTimers()
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

PROTECTED VIRTUAL
void AosRegistration::ClearTimers()
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

    if (m_piExpiredTimer != IMS_NULL)
    {
        StopTimer(TIMER_EXPIRED);
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

PROTECTED VIRTUAL
void AosRegistration::Timer_TimerExpired(IN ITimer *piTimer)
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

    if (piTimer == m_piExpiredTimer)
    {
        ProcessExpiredTimerExpired();
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::CreateSubscription()
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

    StartSubscription();

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::DestroySubscription()
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::StartSubscription()
{
    if (!m_pUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, m_nFeature))
    {
        return IMS_FALSE;
    }

    if (m_pSubscription == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_pUtil->IsFeatureOn(FEATURE_TRM_BLOCK, m_nFeature))
    {
        if(!m_piTrm->IsReady())
        {
            A_IMS_TRACE_I(REGID, "StartSubscription :: txn is pending due to TRM", 0, 0, 0);
            SetTrmBlocked(IMS_TRUE);
            m_pUtil->AddFeature(PENDING_SUBSCRIPTION, m_nTxnPending);
            return IMS_FALSE;
        }
    }

    A_IMS_TRACE_I(REGID, "StartSubscription", 0, 0, 0);

    m_pSubscription->Start();

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::StopSubscription()
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

PROTECTED VIRTUAL
AosSubscription* AosRegistration::GetSubscription(IN IRegSubscription* piRegSubscription)
{
    return new AosSubscription(m_piContext, piRegSubscription,
            m_piRegistration->GetAuthorizedAOR().ToString(), m_piRegContact->GetContactAddress());
}

PROTECTED VIRTUAL
void AosRegistration::ProcessSubscription_Success()
{
}

PROTECTED VIRTUAL
void AosRegistration::ProcessSubscription_Failed()
{
}

PROTECTED VIRTUAL
void AosRegistration::ProcessSubscription_Terminated(IN IMS_SINT32 nTerminateType /* = 0 */)
{
    A_IMS_TRACE_I(REGID, "ProcessSubscription_Terminated, TerminateType : (%d)",
            nTerminateType, 0, 0);

    DestroySubscription();
}

PROTECTED VIRTUAL
void AosRegistration::ProcessRegEventRegistered()
{
}

PROTECTED VIRTUAL
void AosRegistration::UpdateReason()
{

}

PROTECTED VIRTUAL
void AosRegistration::Subscription_StateChanged(IN IMS_SINT32 nState,
        IN IMS_SINT32 nReason /*= 0 */)
{
    A_IMS_TRACE_I(REGID, "Subscription_StateChanged :: state(%d) , reason (%d)",
            nState, nReason, 0);

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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::Subscription_CanBeTransmitted()
{
    if (!IsTransactionStarted())
    {
        m_pUtil->AddFeature(PENDING_SUBSCRIPTION, m_nTxnPending);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void AosRegistration::Subscription_NotifyReceived(IN IMS_SINT32 nEvent)
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

PROTECTED VIRTUAL
void AosRegistration::Subscription_Request(IN IMS_SINT32 nCommand,
        IN IMS_SINT32 nRetryAfter /*= 0 */)
{
    A_IMS_TRACE_I(REGID, "Subscription_Request :: nCommand (%d), nRetryAfter (%d) ",
            nCommand, nRetryAfter, 0);

    switch (nCommand)
    {
        case AosSubscription::COMMAND_REG_REQUIRED:
            PostMessage(MSG_REG_REQUIRED_WITH_WAIT_TIME, nRetryAfter, 0);
            break;
        case AosSubscription::COMMAND_REG_REQUIRED_WITH_NEXT_PCSCF:
            PostMessage(MSG_REG_REQUIRED_WITH_NEXT_PCSCF, 0, 0);
            break;
        case AosSubscription::COMMAND_REG_TERMINATED:
            PostMessage(MSG_REG_TERMINATED_BY_NOTIFY, 0, 0);
            break;
        case AosSubscription::COMMAND_SUB_REQUIRED:
            PostMessage(MSG_SUB_REINITIATE, 0, 0);
            break;
        case AosSubscription::COMMAND_SUB_TERMINATED:
            PostMessage(MSG_SUB_TERMINATED, 0, 0);
            break;
        default :
            break;
    }
}

PROTECTED VIRTUAL
void AosRegistration::CreateIpsecHelper()
{
    DestroyIpsecHelper();

    m_pIpsecHelper = new AosIpsecHelper(m_piRegContact, m_piRegParameter, m_piContext, m_strRegId);
    m_pIpsecHelper->InitIpsec();
}

PROTECTED VIRTUAL
void AosRegistration::DestroyIpsecHelper()
{
    if (m_pIpsecHelper != IMS_NULL)
    {
        delete m_pIpsecHelper;
        m_pIpsecHelper = IMS_NULL;
    }
}

PROTECTED VIRTUAL
void AosRegistration::KeepAlive_DetectedFlowFailed()
{
}

PROTECTED VIRTUAL
void AosRegistration::StartKeepAlive()
{
    // will be removed
    /*
    if (!m_piContext->GetConfig()->IsKeepAlive())
    {
        return;
    }

    IMS_SINT32 nKeepAliveTime = m_pUtil->GetKeepAliveValue(m_piRegistration->GetPreviousResponse());

    if (nKeepAliveTime > 0)
    {
        if(m_pKeepAlive == IMS_NULL)
        {
            m_pKeepAlive = AosProvider::GetInstance()->CreateKeepAlive();
        }

        IPAddress objPcscf(m_strPcscf);
        m_pKeepAlive->SetTransport(m_objIpa, m_pUtil->GetLocalPort(m_nSlotId),
                objPcscf, m_nPcscfPort, SIP::TRANSPORT_TCP);
        m_pKeepAlive->Start(nKeepAliveTime);
        m_pKeepAlive->SetListener(this);
    }
    else
    {
        A_IMS_TRACE_I(REGID, "No Keep Alive Parameter ", 0, 0, 0);
    }
    */
}

PROTECTED VIRTUAL
void AosRegistration::StopKeepAlive()
{
    // will be removed
    /*
    if (!m_piContext->GetConfig()->IsKeepAlive())
    {
        return;
    }

    A_IMS_TRACE_I(REGID, "StopKeepAlive", 0, 0, 0);

    if (m_pKeepAlive != IMS_NULL)
    {
        m_pKeepAlive->Stop();
        delete m_pKeepAlive;
        m_pKeepAlive = IMS_NULL;
    }
    */
}

PROTECTED VIRTUAL
void AosRegistration::Block_Changed(IN IMS_UINT32 /* nType  = 0 */,
        IN IMS_UINT32 /* nParam  = 0 */)
{
    IMS_BOOL bCurrBlocked = !m_piContext->GetBlock()->IsCleared();

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

PROTECTED VIRTUAL
void AosRegistration::CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    if (nType != IAosCallTracker::TYPE_NORMAL)
    {
        return;
    }

    SetImsCall((nState > IAosCallTracker::STATE_IDLE) ? IMS_TRUE : IMS_FALSE);

    if (!IsImsCall() && IsHeldByCall())
    {
        SetHeldByCall(IMS_FALSE);
        UpdateTransactionStarted();

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

PROTECTED VIRTUAL
void AosRegistration::NetTracker_StatusChanged()
{
    IMS_BOOL bCurrBlocked = m_piContext->GetNetTracker()->IsSuspended();

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

PROTECTED VIRTUAL
void AosRegistration::Trm_PriorityChanged()
{
    if (IsTrmBlocked())
    {
        if (m_piTrm->IsReady())
        {
            SetTrmBlocked(IMS_FALSE);
            UpdateTransactionStarted();

            if (IsTransactionStarted())
            {
                ProcessPendingTransaction();
            }
        }
    }
}

PROTECTED VIRTUAL
IMS_RESULT AosRegistration::MessageMediator_AdjustMessage(IN_OUT ISIPMessage* piSipMsg,
        IN IMS_SINT32 nMessage /* = MESSAGE_NORMAL */)
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

PROTECTED VIRTUAL
IMS_BOOL AosRegistration::AddLocationHeaderBody(IN_OUT ISIPMessage* piSipMsg,
        IN IMS_SINT32 nMessage /* = MESSAGE_NORMAL*/)
{
    if (!IsGeolocationInfoRequired())
    {
        return IMS_FALSE;
    }

    GeolocationPidfCreator* pPidfCreator
            = GeolocationHelper::GetInstance()->GetPidfCreator(m_nSlotId);

    if (pPidfCreator == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ByteArray objContent;

    if (!pPidfCreator->CreateWithPosition(AString::ConstNull(), objContent))
    {
        return IMS_FALSE;
    }

    if (nMessage == IMessageMediator::MESSAGE_RESUBMIT)
    {
        piSipMsg->RemoveHeader(ISIPHeader::UNKNOWN, SIPHeaderName::CONTENT_ID);
        piSipMsg->RemoveHeader(ISIPHeader::CONTENT_TYPE);
        piSipMsg->RemoveHeader(ISIPHeader::CONTENT_LENGTH);

        // Removes the previous message body
        piSipMsg->RemoveBodyParts();
    }

    ISIPMessageBodyPart* piBodyPart = piSipMsg->CreateBodyPart();

    // Set a Location
    piBodyPart->SetContent(objContent);

    // Set a Location Content-Type
    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE, "application/pidf+xml");

    AString strNewContentId = GeolocationHelper::CreateContentId(m_nSlotId);

    // Set a Location Content-ID
    AString strContentId;
    strContentId.Sprintf("<%s>", strNewContentId.GetStr());
    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_ID, strContentId);

    // Set the Content-Length header
    AString strClen;
    strClen.SetNumber(objContent.GetLength());
    piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_UNKNOWN,
            strClen, SIPHeaderName::CONTENT_LENGTH);

    // Set Geolocation header
    AString strGeolocation;
    strGeolocation.Sprintf("<cid:%s>", strNewContentId.GetStr());
    piSipMsg->SetHeader(ISIPHeader::UNKNOWN, strGeolocation, "Geolocation");

    return IMS_TRUE;
}

PRIVATE
void AosRegistration::ControlPrivateHeader()
{
    if (!IsRegTypeEqual(AosRegistrationType::NORMAL))
    {
        return;
    }

    IMS_SINT32 nPrivateHeader = GET_N_CONFIG(m_nSlotId)->GetRegistrationPrivateHeader();
    if (nPrivateHeader == CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED)
    {
        return;
    }

    if (nPrivateHeader == CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO)
    {
        SetPlaniHeader();
    }
}

PRIVATE
IMS_UINT32 AosRegistration::GetSpecificErrWaitTime()
{
    IMSVector<IMS_SINT32>& objErrTime = GET_N_CONFIG(m_nSlotId)->GetSpecificRegErrWaitTime();
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
        Destroy();
        ReportStateChanged(RESULT_FAILURE, REASON_FAILURE_FORBIDDEN);
    }
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

    ISIPRTConfigHelper* piConfHelper = SIPFactory::GetRTConfigHelper(m_nSlotId);

    if (bSet)
    {
        if (piConfHelper->GetHeader(AString(AosString::STR_P_LAST_ACCESS_NETWORK_INFO)) != IMS_NULL)
        {
            return;
        }
    }

    SIPRTConfig::Header* pPlaniHeader = new SIPRTConfig::Header();
    pPlaniHeader->strName = AosString::STR_P_LAST_ACCESS_NETWORK_INFO;

    if (bSet)
    {
        pPlaniHeader->strParameter =
                SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetUtcFormat(IMS_TRUE);
        pPlaniHeader->strParameter.Replace(':', "%3A");
        pPlaniHeader->strParameter.Prepend('\"');
        pPlaniHeader->strParameter.Append('\"');

        piConfHelper->SetConfig(SIPRTConfig::CONFIG_I_SIP_HEADER, pPlaniHeader);
    }
    else
    {
        piConfHelper->RemoveConfig(SIPRTConfig::CONFIG_I_SIP_HEADER, pPlaniHeader);
    }

    delete pPlaniHeader;
}

PRIVATE
void AosRegistration::UpdateUserInfoInContact()
{
    // TODO_CONFIG : check ATT
#if 0
    if (m_piContext->GetConfig()->IsRequiredToUpdateContact() == IMS_FALSE)
    {
        return;
    }

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    const AStringArray& objAssociatedUris = m_piRegistration->GetAssociatedURIs();

    if (objAssociatedUris.IsEmpty())
    {
        IMS_TRACE_I("No P-Associated-URI found", 0, 0, 0);
        return;
    }

    AString objMsisdn = AString::ConstNull();
    AosUtil::GetInstance()->GetMsisdn(objMsisdn, m_nSlotId);

    AString strDefaultUserInfo;

    for (IMS_SINT32 nIndex = 0; nIndex < objAssociatedUris.GetCount(); nIndex++)
    {
        const AString& objAssociatedUri = objAssociatedUris.GetElementAt(nIndex);

        IMS_TRACE_D("P-Associated-URI: %s", objAssociatedUri.GetStr(), 0, 0);

        AString strUserInfo;
        AosUtil::GetInstance()->GetUserInfoFromSipAddress(objAssociatedUri, strUserInfo);

        if (strUserInfo.IsNULL())
        {
            return;
        }

        if (nIndex == 0)
        {
            strDefaultUserInfo = strUserInfo;
        }

        if (objMsisdn.IsNULL())
        {
            break;
        }

        if (objAssociatedUri.Contains(objMsisdn))
        {
            IMS_TRACE_I("MSISDN based URI found, Update Contact header", 0, 0, 0);
            m_piRegistration->SetUserInfoForContactHeader(strUserInfo);
            return;
        }
    }

    if (!strDefaultUserInfo.IsNULL()) {
        IMS_TRACE_I("Update with the default one", 0, 0, 0);
        m_piRegistration->SetUserInfoForContactHeader(strDefaultUserInfo);
    }
#endif
}

PRIVATE
void AosRegistration::UpdateCallingNumberVerification()
{
/*  TODO : change impl.
    if (m_piContext->GetConfig()->IsCallingNumberVerificationSupported() == IMS_FALSE)
    {
        return;
    }

    m_bCallingNumberVerificationSupported = IMS_FALSE;

    if (m_piRegistration == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bIsFeatureIncluded = m_pUtil->IsParameterIncluded(
            m_piRegistration->GetPreviousResponse(), ISIPHeader::UNKNOWN,
            AosString::STR_FEATURE_CAPS, AosString::STR_VERSTAT_FEATURE);

    if (bIsFeatureIncluded)
    {
        IMS_TRACE_I("Calling number verification is supported", 0, 0, 0);
        m_bCallingNumberVerificationSupported = IMS_TRUE;
    }
    else
    {
        IMS_TRACE_I("Calling number verification is not supported", 0, 0, 0);
    }
*/
}

PRIVATE
void AosRegistration::UpdateRegIpcanCategory()
{
    m_nRegIpcanCategory = m_piContext->GetConnection()->GetIpcanCategory();
}

PRIVATE
void AosRegistration::UpdateModeToHandles()
{
    IMSMap<AString, IAosHandle*>& objHandles = m_piContext->GetHandles();

    for (IMS_UINT32 nAt = 0; nAt < objHandles.GetSize(); ++nAt)
    {
        IAosHandle* piHandle = objHandles.GetValueAt(nAt);
        piHandle->Request(IAosHandle::TYPE_LIMITED_MODE, (GetMode() == MODE_LIMITED) ?
                IAosHandle::STATE_ADD : IAosHandle::STATE_REMOVE);
    }
}

PRIVATE
IMS_BOOL AosRegistration::IsErrorCodeExisted(IN const IMSVector<IMS_SINT32>& objErrorCode,
        IN IMS_SINT32 nCode) const
{
    for (int i = 0; i < objErrorCode.GetSize(); i++)
    {
        if (nCode == objErrorCode.GetAt(i))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AosRegistration::IsErrorCodeExistedForSpecificRegistration(IN IMS_SINT32 nCode) const
{
    IMSVector<IMS_SINT32>& objErrorCode =
            GET_N_CONFIG(m_nSlotId)->GetSpecificRegistrationErrorCode();

    for (int i = 0; i < objErrorCode.GetSize(); i++)
    {
        if (nCode == objErrorCode.GetAt(i))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AosRegistration::IsPdnReactivationRequired()
{
    m_nConsecutiveFailureForPdnReactivated++;

    // TODO: check eps_only or nr
    if ((m_piContext->GetPcscf()->GetPcscfCount() * 2) <= m_nConsecutiveFailureForPdnReactivated)
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
    IMS_SINT32 nRemainTime = 0;

    if (nExpireTime > 0 && nAwt > 0)
    {
        if (nExpireTime > 1200)
        {
            nRemainTime = 600 - nAwt;
        }
        else
        {
            nRemainTime = (nExpireTime / 2) - nAwt;
        }

        A_IMS_TRACE_I(REGID, "IsRegExpiredDuringAwt :: Expire(%d) , AWT(%d) , Remain(%d)",
                nExpireTime, nAwt, nRemainTime);

        if (nRemainTime <= 0)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}