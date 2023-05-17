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
#include "ServicePhoneInfo.h"
#include "IAosService.h"
#include "ImsMessage.h"
#include "INetworkWatcher.h"

#include "AosReason.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"

#include "IImsAosListener.h"
#include "IImsAosMonitor.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosApplication.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosRegStateManager.h"

#include "provider/AosLog.h"
#include "provider/AosProvider.h"
#include "provider/AosString.h"
#include "provider/AosUtil.h"
#include "handle/AosFeatureTag.h"
#include "handle/AosInfo.h"
#include "handle/AosHandle.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTagWithServiceType.GetStr()

BEGIN_STATE_MAP(AosHandle)
STATE_ENTRY(STATE_DISCONNECTED)
STATE_ENTRY(STATE_CONNECTING)
STATE_ENTRY(STATE_CONNECTED)
STATE_ENTRY(STATE_DISCONNECTING)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(AosHandle, STATE_DISCONNECTED)
STATE_MSG_ENTRY(HANDLE_MSG_BLOCK_STATUS, &AosHandle::StateDisconnected)
STATE_MSG_ENTRY(HANDLE_MSG_APP_STATUS, &AosHandle::StateDisconnected)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosHandle, STATE_CONNECTING)
STATE_MSG_ENTRY(HANDLE_MSG_BLOCK_STATUS, &AosHandle::StateConnecting)
STATE_MSG_ENTRY(HANDLE_MSG_APP_STATUS, &AosHandle::StateConnecting)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosHandle, STATE_CONNECTED)
STATE_MSG_ENTRY(HANDLE_MSG_BLOCK_STATUS, &AosHandle::StateConnected)
STATE_MSG_ENTRY(HANDLE_MSG_APP_STATUS, &AosHandle::StateConnected)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(AosHandle, STATE_DISCONNECTING)
STATE_MSG_ENTRY(HANDLE_MSG_BLOCK_STATUS, &AosHandle::StateDisconnecting)
STATE_MSG_ENTRY(HANDLE_MSG_APP_STATUS, &AosHandle::StateDisconnecting)
END_STATE_MSG_MAP()

PUBLIC
AosHandle::AosHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
        IN const AString& strServiceId, IN const IMS_UINT32 nServiceType) :
        m_piAppContext(piAppContext),
        m_nSlotId(piAppContext->GetSlotId()),
        m_strAppId(strAppId),
        m_strServiceId(strServiceId),
        m_nServiceType(nServiceType),
        m_nReqType(DETACH),
        m_bBind(IMS_FALSE),
        m_bNetworkBind(IMS_TRUE),
        m_bRegFeatureTagRequired(IMS_TRUE),
        m_bNotify(IMS_FALSE),
        m_piListener(IMS_NULL),
        m_piMonitor(IMS_NULL),
        m_piInfo(IMS_NULL),
        m_piWifiWatcher(IMS_NULL),
        m_nReason(AosReason::NONE),
        m_nSuspendedReason(AosReason::SUSPEND_NONE),
        m_bBlocked(IMS_FALSE),
        m_nBlocks(BLOCK_NONE),
        m_nHoldingBlocksForMobile(BLOCK_NONE),
        m_nHoldingBlocksForWifi(BLOCK_NONE),
        m_nHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED),
        m_nVopsState(IMS_VOICE_OVER_PS_SUPPORTED),
        m_nRoamingState(IMS_ROAMING_STATE_OFF),
        m_bVopsIgnoredForVolteEnabled(IMS_TRUE),
        m_bCsVoiceAvailable(IMS_FALSE),
        m_bEpdgEnabled(IMS_FALSE),
        m_bDataConnected(IMS_FALSE),
        m_bNetSrvIn(IMS_FALSE),
        m_nNetworkType(NW_REPORT_RADIO_INVALID),
        m_nAppState(APP_STATE_DISCONNECTED)
{
    IMS_CHAR acLog[256 + 1] = {
            0,
    };
    IMS_Sprintf(acLog, 256, "APP(%s)/SERVICE(%s)/TYPE(%d)", m_strAppId.GetStr(),
            m_strServiceId.GetStr(), m_nServiceType);

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandle = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosHandle), this);

    m_strTag.Sprintf("%d:%s", m_nSlotId, m_piAppContext->GetProfileId().GetStr());
    m_strTagWithServiceType.Sprintf(
            "%d:%s:%s", m_nSlotId, m_piAppContext->GetProfileId().GetStr(), ServiceTypeToString());

    SetHandleState(STATE_DISCONNECTED);

    IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig != IMS_NULL)
    {
        piNConfig->SetListener(this);
    }
}

PUBLIC VIRTUAL AosHandle::~AosHandle()
{
    IMS_CHAR acLog[256 + 1] = {
            0,
    };
    IMS_Sprintf(acLog, 256, "APP(%s)/SERVICE(%s)/TYPE(%d)", m_strAppId.GetStr(),
            m_strServiceId.GetStr(), m_nServiceType);

    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandle = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosHandle), this);

    IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig != IMS_NULL)
    {
        piNConfig->RemoveListener(this);
    }
}

PUBLIC VIRTUAL AString& AosHandle::GetAppId()
{
    return m_strAppId;
}

PUBLIC VIRTUAL AString& AosHandle::GetServiceId()
{
    return m_strServiceId;
}

PUBLIC VIRTUAL IMS_UINT32 AosHandle::GetServiceType()
{
    return m_nServiceType;
}

PUBLIC VIRTUAL IImsAosMonitor* AosHandle::GetMonitor()
{
    return m_piMonitor;
}

PUBLIC VIRTUAL IMS_SINT32 AosHandle::GetRequestType()
{
    return m_nReqType;
}

PUBLIC VIRTUAL void AosHandle::SetRequestType(IN IMS_SINT32 nReqType)
{
    A_IMS_TRACE_I(
            APPPROFILE, "SetRequestType :: [%s]", (nReqType == ATTACH) ? "ATTACH" : "DETACH", 0, 0);

    m_nReqType = nReqType;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::IsRegBinded()
{
    return m_bBind;
}

PUBLIC VIRTUAL void AosHandle::SetRegBinded(IN IMS_BOOL bBind)
{
    A_IMS_TRACE_I(APPPROFILE, "SetRegBinded :: [%s]", (bBind) ? "ATTACHED" : "DETACHED", 0, 0);

    m_bBind = bBind;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::IsNetworkRegBinded()
{
    return m_bNetworkBind;
}

PUBLIC VIRTUAL void AosHandle::SetNetworkRegBinded(IN IMS_BOOL bNetworkBind)
{
    if (bNetworkBind == m_bNetworkBind)
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "SetNetworkRegBinded :: [%s]",
            (bNetworkBind) ? "ATTACHED" : "DETACHED", 0, 0);

    m_bNetworkBind = bNetworkBind;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::IsRegFeatureTagRequired()
{
    return m_bRegFeatureTagRequired;
}

PUBLIC VIRTUAL AosFeatureTagList& AosHandle::GetFeatureTagList()
{
    return m_objFeatureTagList;
}

PUBLIC VIRTUAL AosFeatureTagList& AosHandle::GetBindedFeatureTagList()
{
    return m_objBindedFeatureTagList;
}

PUBLIC VIRTUAL void AosHandle::ProcessFeatureTagChange()
{
    if (m_objFeatureTagList.Equals(m_objBindedFeatureTagList) == IMS_FALSE)
    {
        IMS_UINT32 nState = GetState();
        A_IMS_TRACE_I(APPPROFILE, "ProcessFeatureTagChange :: Feature tag is changed, nState [%s]",
                StateToString(nState), 0, 0);

        switch (nState)
        {
            case STATE_CONNECTING:  // FALL-THROUGH
            case STATE_CONNECTED:
                m_piAppContext->GetApp()->Reconfig();
                break;

            default:
                break;
        }
    }
}

PUBLIC VIRTUAL void AosHandle::Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState /* = 0 */)
{
    (void)nType;
    (void)nState;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::App_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nParam)
{
    A_IMS_TRACE_I(APPPROFILE, "App_StateChanged :: [state(%d)]", nState, 0, 0);

    switch (nState)
    {
        case IAosApplication::APP_DISCONNECTED:  // FALL-THROUGH
        case IAosApplication::APP_CONNECTED:     // FALL-THROUGH
        case IAosApplication::APP_UPDATING:      // FALL-THROUGH
        case IAosApplication::APP_DISCONNECTING:
        {
            IMSMSG objMSG(HANDLE_MSG_APP_STATUS, nState, nParam);
            OnStateMessage(objMSG);
            return IMS_TRUE;
        }
        default:
            return IMS_FALSE;
    }
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::App_Notify()
{
    if (m_piListener == IMS_NULL)
    {
        A_IMS_TRACE_D(APPPROFILE, "App_Notify :: no listener", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_bNotify == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "App_Notify :: no notification", 0, 0, 0);
        return IMS_FALSE;
    }

    // notify the state to Enabler
    switch (GetState())
    {
        case STATE_DISCONNECTED:  // FALL-THROUGH
        case STATE_CONNECTING:
            m_piListener->ImsAos_Disconnected(GetImsAosReason(m_nReason));
            break;
        case STATE_CONNECTED:
            m_piListener->ImsAos_Connected(
                    GetFeatures(), m_piAppContext->GetConnection()->GetIpcanCategory());
            break;
        case STATE_DISCONNECTING:
            m_piListener->ImsAos_Disconnecting(GetImsAosReason(m_nReason));
            break;
        default:
            return IMS_FALSE;
    }

    ReportRegState();

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosHandle::Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked)
{
    (void)nType;
    (void)bBlocked;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::Control(IN IMS_UINT32 nType)
{
    A_IMS_TRACE_I(APPPROFILE, "Control :: Type[%d]", nType, 0, 0);

    return m_piAppContext->GetApp()->RequestCmd(nType);
}

PUBLIC VIRTUAL IImsAosInfo* AosHandle::GetAosInfo()
{
    return m_piInfo;
}

PUBLIC VIRTUAL IMS_UINT32 AosHandle::GetFeatures()
{
    if (!IsImsConnected())
    {
        return ImsAosFeature::NONE;
    }

    return (m_objBindedFeatureTagList.GetFeatures() &
            ~(m_objBindedFeatureTagList.GetUnavailableFeatures()));
}

PUBLIC VIRTUAL IMS_UINT32 AosHandle::GetSuspendedReason()
{
    return m_nSuspendedReason;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::IsFeatureConnected(IN IMS_UINT32 nFeature)
{
    return (GetFeatures() & nFeature) > 0;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::IsImsConnected()
{
    IMS_UINT32 nState = GetState();

    A_IMS_TRACE_D(APPPROFILE, "IsImsConnected :: connected(%s)",
            _TRACE_B_(nState == STATE_CONNECTED), 0, 0);

    return (nState == STATE_CONNECTED);
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::IsImsSuspended()
{
    A_IMS_TRACE_D(APPPROFILE, "IsImsSuspended :: reason(%x)", m_nSuspendedReason, 0, 0);

    return (m_nSuspendedReason != AosReason::SUSPEND_NONE);
}

PUBLIC VIRTUAL void AosHandle::SetListener(IN IImsAosListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC VIRTUAL void AosHandle::SetMonitor(IN IImsAosMonitor* piMonitor)
{
    m_piMonitor = piMonitor;
}

PUBLIC VIRTUAL IMS_BOOL AosHandle::SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService)
{
    if (nService != ImsAosService::MTC || !bReady)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_D(
            APPPROFILE, "SetReady :: bReady[%s], nService(%d)", _TRACE_B_(bReady), nService, 0);

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL)
    {
        piCallTracker->SetMtcReady();
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL void AosHandle::UpdateFeature(IN IMS_UINT32 /*nFeatures*/)
{
    // It is for SipController so it will be overridden in AosSipController
}

PUBLIC VIRTUAL void AosHandle::UpdateFeature(IN ImsList<ImsAosFeatureTag*>& /*objFeatureTag*/)
{
    // It is for SipController so it will be overridden in AosSipControllers
}

PUBLIC VIRTUAL void AosHandle::CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState)
{
    A_IMS_TRACE_D(APPPROFILE, "CallTracker_StateChanged :: nType=%d, nState=%d", nType,
            static_cast<IMS_UINT32>(eState), 0);
    // Implemented in child
}

PUBLIC VIRTUAL void AosHandle::NetTracker_StatusChanged()
{
    if (IsEmergencyService())
    {
        return;
    }

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();

    if (bCurrSrvIn != m_bNetSrvIn)
    {
        A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: service state changed >> (%s)",
                (bCurrSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE", 0, 0);

        // IN Service
        if (bCurrSrvIn)
        {
            ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE);
        }
        // OUT service
        else
        {
            ProcessImsSuspended(AosReason::SUSPEND_NO_SERVICE);
        }

        m_bNetSrvIn = bCurrSrvIn;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsDeregOn3gNetwork())
    {
        if (bCurrSrvIn)
        {
            if (IsSupportedNetworkType(GetNetworkType()))
            {
                if (IsHandleBlocked(BLOCK_3G))
                {
                    ProcessBlock(BLOCK_3G, IMS_FALSE);
                }
            }
            else if (Is3G(GetNetworkType()))
            {
                ProcessBlock(BLOCK_3G, IMS_TRUE);
            }
        }
    }

    IMS_BOOL bCurrDataConnected = IsDataConnected();
    IMS_BOOL bIpcanChanged = UpdateIpcan();

    if (m_bDataConnected != bCurrDataConnected)
    {
        if (bCurrDataConnected)
        {
            ReevaluateBlocks();
        }
        else
        {
            BackupAllBlocks();
        }

        m_bDataConnected = bCurrDataConnected;

        ProcessDataConnectionChanged();
    }
    else
    {
        if (bIpcanChanged)
        {
            ReevaluateBlocks();
        }
    }
}

PUBLIC VIRTUAL void AosHandle::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_D(APPPROFILE, "NConfiguration_NotifyConfigChanged", 0, 0, 0);

    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL)
    {
        InitializeHoldingBlocksPolicy();
        InitializeServiceBlock();
        InitializeServiceFeature();
        InitializeFeatureTags();

        IMSMSG objMSG(HANDLE_MSG_BLOCK_STATUS, 0, 0);
        OnStateMessage(objMSG);
    }
}

PROTECTED VIRTUAL void AosHandle::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    InitializeHoldingBlocksPolicy();
    InitializeServiceBlock();
    InitializeServiceFeature();
    InitializeFeatureTags();

    AddListeners();

    m_piInfo = new AosInfo(m_piAppContext);
    m_piWifiWatcher = PhoneInfoService::GetPhoneInfoService()->GetWifiWatcher();

    IMSMSG objMSG(HANDLE_MSG_BLOCK_STATUS, 0, 0);
    OnStateMessage(objMSG);
}

PROTECTED VIRTUAL void AosHandle::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    if (m_piInfo != IMS_NULL)
    {
        delete m_piInfo;
    }

    RemoveListeners();
}

PROTECTED
void AosHandle::SetHandleState(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(APPPROFILE, "SetHandleState :: %s >> %s", StateToString(GetState()),
            StateToString(nState), 0);

    SetState(nState);

    if (nState == STATE_DISCONNECTED || nState == STATE_DISCONNECTING)
    {
        ClearSuspendedReason();
    }
}

PROTECTED
void AosHandle::SetReason(IN IMS_UINT32 nReason)
{
    m_nReason = nReason;
}

PROTECTED
void AosHandle::ClearSuspendedReason()
{
    A_IMS_TRACE_D(APPPROFILE, "ClearSuspendedReason", 0, 0, 0);
    m_nSuspendedReason = AosReason::SUSPEND_NONE;
}

PROTECTED
IMS_UINT32 AosHandle::GetAppState()
{
    IMS_UINT32 nAppState = APP_STATE_DISCONNECTED;

    switch (GetState())
    {
        case STATE_CONNECTED:
            nAppState = APP_STATE_CONNECTED;
            break;

        case STATE_DISCONNECTING:
            nAppState = APP_STATE_DISCONNECTING;
            break;

        default:
            break;
    }

    return nAppState;
}

PROTECTED
IMS_UINT32 AosHandle::GetImsAosReason(IN IMS_UINT32 nAosReason)
{
    IMS_UINT32 nImsAosReason = ImsAosReason::NOT_SPECIFIED;

    switch (nAosReason)
    {
        case AosReason::BAD_BATTERY:  // FALL-THROUGH
        case AosReason::POWER_OFF:
            nImsAosReason = ImsAosReason::POWER_OFF;
            break;
        case AosReason::AIRPLANE_MODE:  // FALL-THROUGH
        case AosReason::DATA_DISCONNECTED:
            nImsAosReason = ImsAosReason::DATA_DISCONNECTED;
            break;
        case AosReason::NO_LTE_COVERAGE:
            nImsAosReason = ImsAosReason::NO_RAT_COVERAGE;
            break;
        case AosReason::SERVICE_POLICY:
            nImsAosReason = ImsAosReason::SERVICE_POLICY;
            break;
        case AosReason::SERVICE_BLOCKED:
            nImsAosReason = ImsAosReason::SERVICE_BLOCKED;
            break;
        case AosReason::SRV_OUT:
            nImsAosReason = ImsAosReason::OUT_OF_SERVICE;
            break;
        case AosReason::REG_TERMINATED:
            nImsAosReason = ImsAosReason::REG_TERMINATED;
            break;
        case AosReason::INITIAL_REG_REQUESTED:
            nImsAosReason = ImsAosReason::REG_NEW_REQUIRED;
            break;
        case AosReason::REG_TERMINATING:
            nImsAosReason = ImsAosReason::REG_TERMINATING;
            break;
        default:
            break;
    }

    return nImsAosReason;
}

PROTECTED
IMS_UINT32 AosHandle::GetImsAosReasonForSuspend(IN IMS_UINT32 nAosReason)
{
    IMS_UINT32 nImsAosReason = ImsAosReason::SUSPEND_NONE;

    switch (nAosReason)
    {
        case AosReason::SUSPEND_NONE:
            break;
        case AosReason::SUSPEND_NO_SERVICE:
            nImsAosReason = ImsAosReason::SUSPEND_OUT_OF_SERVICE;
            break;
        case AosReason::SUSPEND_NO_LTE_COVERAGE:
            nImsAosReason = ImsAosReason::SUSPEND_NO_RAT_COVERAGE;
            break;
        default:
            break;
    }

    return nImsAosReason;
}

PROTECTED
IMS_BOOL AosHandle::IsEpdgEnabled() const
{
    return m_bEpdgEnabled;
}

PROTECTED
IMS_BOOL AosHandle::IsEqualNetworkType(IN IMS_UINT32 nType, IN AosNetworkType eType) const
{
    return (nType == static_cast<IMS_UINT32>(eType));
}

PROTECTED
IMS_BOOL AosHandle::IsCapabilityExisted(
        IN IMS_UINT32 nCapabilities, IN AosCapability eCapability) const
{
    return (nCapabilities & static_cast<IMS_UINT32>(eCapability));
}

PROTECTED
IMS_BOOL AosHandle::IsCapabilityExistedForNetworkType(
        IN IMS_UINT32 nNetworkType, IN AosCapability eCapability) const
{
    IMS_UINT32 nCapabilities = static_cast<IMS_UINT32>(AosCapability::NONE);

    switch (nNetworkType)
    {
        case NW_REPORT_RADIO_LTE:
            nCapabilities =
                    m_objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE));
            break;

        case NW_REPORT_RADIO_NR:
            nCapabilities = m_objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR));
            break;

        case NW_REPORT_RADIO_WLAN:
            nCapabilities =
                    m_objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN));
            break;

        default:
            break;
    }

    return IsCapabilityExisted(nCapabilities, eCapability);
}

PROTECTED
IMS_BOOL AosHandle::IsNetworkTypeMatchedToRat(IMS_UINT32 nNetworkType, IMS_UINT32 nRat) const
{
    switch (nRat)
    {
        case NW_REPORT_RADIO_LTE:
            return IsEqualNetworkType(nNetworkType, AosNetworkType::LTE);
        case NW_REPORT_RADIO_NR:
            return IsEqualNetworkType(nNetworkType, AosNetworkType::NR);
        case NW_REPORT_RADIO_WLAN:
            return IsEqualNetworkType(nNetworkType, AosNetworkType::IWLAN);
        default:
            return IMS_FALSE;
    }
}

PROTECTED
IMS_BOOL AosHandle::IsWifiConnected()
{
    return (m_piWifiWatcher->GetState() == IWifiWatcher::STATE_CONNECTED);
}

PROTECTED
IMS_BOOL AosHandle::IsDataConnected()
{
    IAosConnection* piConnection = m_piAppContext->GetConnection();

    return (piConnection->GetState() == IAosConnection::STATE_ACTIVE);
}

PROTECTED
IMS_BOOL AosHandle::IsEmergencyService()
{
    return (m_nServiceType == ImsAosService::EMERGENCY_MTC ||
            m_nServiceType == ImsAosService::EMERGENCY_MTS);
}

PROTECTED
IMS_BOOL AosHandle::IsRoaming() const
{
    return (m_nRoamingState == IMS_ROAMING_STATE_ON);
}

PROTECTED
IMS_UINT32 AosHandle::GetNetworkType() const
{
    return m_piAppContext->GetNetTracker()->GetNetworkType();
}

PROTECTED
IMS_UINT32 AosHandle::GetMobileNetworkType() const
{
    return m_piAppContext->GetNetTracker()->GetMobileNetworkType();
}

PROTECTED
IMS_UINT32 AosHandle::GetMobileChangingNetworkType() const
{
    return m_piAppContext->GetNetTracker()->GetMobileChangingNetworkType();
}

PROTECTED
IMS_UINT32 AosHandle::GetAosFeature(IN IMS_UINT32 nBlock)
{
    IMS_UINT32 nFeature = ImsAosFeature::NONE;

    switch (nBlock)
    {
        case BLOCK_VOLTE_CAPABILITY:   // FALL-THROUGH
        case BLOCK_VOWIFI_CAPABILITY:  // FALL-THROUGH
        case BLOCK_VOPS:
            nFeature = ImsAosFeature::MMTEL;
            break;

        case BLOCK_VILTE_CAPABILITY:  // FALL-THROUGH
        case BLOCK_VIWIFI_CAPABILITY:
            nFeature = ImsAosFeature::VIDEO;
            break;

        case BLOCK_SMS_CAPABILITY:  // FALL-THROUGH
        case BLOCK_SMS_OVER_IP_NETWORK_INDICATION:
            nFeature = ImsAosFeature::SMSIP;
            break;

        case BLOCK_CALL_COMPOSER_CAPABILITY:
            nFeature = ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
            break;

        default:
            break;
    }

    return nFeature;
}

PROTECTED
void AosHandle::AddBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
{
    nBlocks |= nBlock;
}

PROTECTED
void AosHandle::RemoveBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
{
    nBlocks &= ~(nBlock);
}

PROTECTED
void AosHandle::SetBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks, IN IMS_BOOL bAdd)
{
    if (bAdd)
    {
        AddBlock(nBlock, nBlocks);
    }
    else
    {
        RemoveBlock(nBlock, nBlocks);
    }
}

PROTECTED
IMS_BOOL AosHandle::PreProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded)
{
    if (IsEpdgEnabled())
    {
        if (IsBlockForWifi(nBlock))
        {
            return IMS_FALSE;
        }

        SetBlock(nBlock, m_nHoldingBlocksForMobile, bAdded);

        A_IMS_TRACE_D(APPPROFILE, "PreProcessBlock :: HoldingBlocksForMobile (%x)",
                m_nHoldingBlocksForMobile, 0, 0);

        if (!bAdded && IsHandleBlocked(nBlock))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    if (IsBlockForMobile(nBlock))
    {
        return IMS_FALSE;
    }

    SetBlock(nBlock, m_nHoldingBlocksForWifi, bAdded);

    A_IMS_TRACE_D(APPPROFILE, "PreProcessBlock :: HoldingBlocksForWifi (%x)",
            m_nHoldingBlocksForWifi, 0, 0);

    if (!bAdded && IsHandleBlocked(nBlock))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
void AosHandle::ProcessBlock(
        IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded, IN IMS_BOOL bPreProcess /* = IMS_TRUE */)
{
    A_IMS_TRACE_D(APPPROFILE, "ProcessBlock :: nBlock[%x], bAdded[%s], bPreProcess[%s]", nBlock,
            _TRACE_B_(bAdded), _TRACE_B_(bPreProcess));

    if (bPreProcess)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessBlock :: Data connection [%s]",
                _TRACE_B_(m_bDataConnected), 0, 0);

        if (!m_bDataConnected)
        {
            if (HoldBlockForInvalidNetwork(nBlock, bAdded))
            {
                A_IMS_TRACE_D(APPPROFILE, "ProcessBlock :: No data connection. Not handled (%x)",
                        m_nBlocks, 0, 0);
                return;
            }
        }

        if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
        {
            if (PreProcessBlock(nBlock, bAdded))
            {
                A_IMS_TRACE_D(APPPROFILE, "ProcessBlock :: Not handled. (%x)", m_nBlocks, 0, 0);
                return;
            }
        }

        if (!bAdded)
        {
            if (IsHandleBlocked(m_nHoldingBlocksForMobile, nBlock))
            {
                A_IMS_TRACE_D(APPPROFILE,
                        "ProcessBlock :: Remove block from m_nHoldingBlocksForMobile", 0, 0, 0);
                RemoveBlock(nBlock, m_nHoldingBlocksForMobile);
            }

            if (IsHandleBlocked(m_nHoldingBlocksForWifi, nBlock))
            {
                A_IMS_TRACE_D(APPPROFILE,
                        "ProcessBlock :: Remove block from m_nHoldingBlocksForWifi", 0, 0, 0);
                RemoveBlock(nBlock, m_nHoldingBlocksForWifi);
            }
        }
    }

    SetBlock(nBlock, m_nBlocks, bAdded);

    ProcessFeatureBlock(GetAosFeature(nBlock), bAdded);

    ProcessCheckBlock();
}

PROTECTED
IMS_BOOL AosHandle::ProcessCheckBlock(IN IMS_BOOL bRunStateMachine /* = IMS_TRUE */)
{
    IMS_BOOL bCurrBlocked = IsHandleBlocked();

    A_IMS_TRACE_D(APPPROFILE, "ProcessCheckBlock :: old(%s) -> current(%s) , blocks(%x)",
            _TRACE_B_(m_bBlocked), _TRACE_B_(bCurrBlocked), m_nBlocks);

    if (m_bBlocked != bCurrBlocked)
    {
        m_bBlocked = bCurrBlocked;

        if (bRunStateMachine)
        {
            IMSMSG objMSG(HANDLE_MSG_BLOCK_STATUS, 0, 0);
            OnStateMessage(objMSG);
        }

        ProcessBlockChanged();

        return IMS_TRUE;
    }
    else
    {
        if (!m_bBlocked)
        {
            ProcessFeatureTagChange();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED
void AosHandle::ProcessUnavailableFeature(IN IMS_UINT32 nFeature, IN IMS_BOOL bAdd)
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessUnavailableFeature :: nFeature[%x], bAdd[%s]", nFeature,
            _TRACE_B_(bAdd), 0);

    if (bAdd)
    {
        m_objFeatureTagList.AddUnavailableFeature(nFeature);
        m_objBindedFeatureTagList.AddUnavailableFeature(nFeature);
    }
    else
    {
        m_objFeatureTagList.RemoveUnavailableFeature(nFeature);
        m_objBindedFeatureTagList.RemoveUnavailableFeature(nFeature);
    }
}

PROTECTED
void AosHandle::ProcessUnavailableFeatureChanged()
{
    IMS_UINT32 nState = GetState();
    A_IMS_TRACE_I(APPPROFILE,
            "ProcessUnavailableFeatureChanged :: Unavailable feature has changed, nState [%s]",
            StateToString(nState), 0, 0);

    IAosRegistration* piRegistration = m_piAppContext->GetRegistration();
    piRegistration->RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG);

    if (nState == STATE_CONNECTED && m_piListener != IMS_NULL)
    {
        m_piListener->ImsAos_Connected(
                GetFeatures(), m_piAppContext->GetConnection()->GetIpcanCategory());
    }
}

PROTECTED
void AosHandle::BackupAllBlocks()
{
    if (!IsSupportedNetworkTypeForCellular(GetMobileNetworkType()))
    {
        BackupBlocks(m_objHoldingBlocksPolicyForMobile, m_nHoldingBlocksForMobile);
    }

    if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable() && !IsWifiConnected())
    {
        BackupBlocks(m_objHoldingBlocksPolicyForWifi, m_nHoldingBlocksForWifi);
    }
}

PROTECTED
void AosHandle::BackupBlocks(
        IN ImsList<IMS_UINT32>& objHoldingBlocksPolicy, IN_OUT IMS_UINT32& nHoldingBlocks)
{
    for (IMS_UINT32 i = 0; i < objHoldingBlocksPolicy.GetSize(); i++)
    {
        IMS_UINT32 nBlock = objHoldingBlocksPolicy.GetAt(i);
        if (IsHandleBlocked(nBlock))
        {
            A_IMS_TRACE_D(APPPROFILE, "BackupBlocks :: Reset block[%x] and set to HoldingBlocks",
                    nBlock, 0, 0);
            AddBlock(nBlock, nHoldingBlocks);
            ProcessBlock(nBlock, IMS_FALSE, IMS_FALSE);
        }
    }
}

PROTECTED
void AosHandle::RestoreBlocks(
        IN ImsList<IMS_UINT32>& objHoldingBlocksPolicy, IN_OUT IMS_UINT32& nHoldingBlocks)
{
    for (IMS_UINT32 i = 0; i < objHoldingBlocksPolicy.GetSize(); i++)
    {
        IMS_UINT32 nBlock = objHoldingBlocksPolicy.GetAt(i);
        if (IsHandleBlocked(nHoldingBlocks, nBlock))
        {
            A_IMS_TRACE_D(APPPROFILE, "RestoreBlocks :: Set block[%x] and reset from HoldingBlocks",
                    nBlock, 0, 0);
            RemoveBlock(nBlock, nHoldingBlocks);
            ProcessBlock(nBlock, IMS_TRUE, IMS_FALSE);
        }
    }
}

PROTECTED
IMS_BOOL AosHandle::HoldBlockForInvalidNetwork(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded)
{
    if (IsBlockForMobile(nBlock))
    {
        if (IsSupportedNetworkTypeForCellular(GetMobileNetworkType()))
        {
            return IMS_FALSE;
        }

        SetBlock(nBlock, m_nHoldingBlocksForMobile, bAdded);

        A_IMS_TRACE_D(APPPROFILE, "HoldBlockForInvalidNetwork :: HoldingBlocksForMobile (%x)",
                m_nHoldingBlocksForMobile, 0, 0);

        if (!bAdded && IsHandleBlocked(nBlock))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    if (IsBlockForWifi(nBlock))
    {
        if (!GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
        {
            return IMS_FALSE;
        }

        if (IsWifiConnected())
        {
            return IMS_FALSE;
        }

        SetBlock(nBlock, m_nHoldingBlocksForWifi, bAdded);

        A_IMS_TRACE_D(APPPROFILE, "HoldBlockForInvalidNetwork :: HoldingBlocksForWifi (%x)",
                m_nHoldingBlocksForWifi, 0, 0);

        if (!bAdded && IsHandleBlocked(nBlock))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void AosHandle::ReevaluateBlocks()
{
    if (m_bEpdgEnabled)
    {
        BackupBlocks(m_objHoldingBlocksPolicyForMobile, m_nHoldingBlocksForMobile);
        RestoreBlocks(m_objHoldingBlocksPolicyForWifi, m_nHoldingBlocksForWifi);
    }
    else
    {
        BackupBlocks(m_objHoldingBlocksPolicyForWifi, m_nHoldingBlocksForWifi);
        RestoreBlocks(m_objHoldingBlocksPolicyForMobile, m_nHoldingBlocksForMobile);
    }
}

PROTECTED
IMS_BOOL AosHandle::UpdateIpcan()
{
    IMS_BOOL bEpdgEnabled = m_piAppContext->GetConnection()->IsEpdgEnabled();
    if (m_bEpdgEnabled != bEpdgEnabled)
    {
        m_bEpdgEnabled = bEpdgEnabled;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosHandle::IsHandleBlocked(IN const IMS_UINT32& nBlocks, IN IMS_UINT32 nType) const
{
    return (nBlocks & nType);
}

PROTECTED
IMS_BOOL AosHandle::IsHandleBlocked(IN IMS_UINT32 nType) const
{
    return (m_nBlocks & nType);
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::IsHandleBlocked() const
{
    return (m_nBlocks != BLOCK_NONE);
}

PROTECTED VIRTUAL void AosHandle::ProcessBlockChanged() {}

PROTECTED VIRTUAL IMS_BOOL AosHandle::IsBlockForMobile(IN IMS_UINT32 nBlock) const
{
    for (IMS_UINT32 i = 0; i < m_objHoldingBlocksPolicyForWifi.GetSize(); i++)
    {
        if (nBlock == m_objHoldingBlocksPolicyForWifi.GetAt(i))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::IsBlockForWifi(IN IMS_UINT32 nBlock) const
{
    for (IMS_UINT32 i = 0; i < m_objHoldingBlocksPolicyForMobile.GetSize(); i++)
    {
        if (nBlock == m_objHoldingBlocksPolicyForMobile.GetAt(i))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AosHandle::ReevaluateUnavailableFeature()
{
    // Implemented in child classes
}

PROTECTED VIRTUAL void AosHandle::ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked)
{
    if (bBlocked)
    {
        m_objFeatureTagList.RemoveFeature(nFeature);
    }
    else
    {
        m_objFeatureTagList.AddFeature(nFeature);
    }

    A_IMS_TRACE_D(APPPROFILE, "ProcessFeatureBlock :: Updated feature = [%x]",
            m_objFeatureTagList.GetFeatures(), 0, 0);
}

PROTECTED VIRTUAL void AosHandle::ProcessCapabilitiesChanged(
        IN const ImsMap<IMS_UINT32, IMS_UINT32>& /* objNewCapabilities */)
{
    // Implemented in child
}

PROTECTED VIRTUAL void AosHandle::ProcessDataConnectionChanged()
{
    // Implemented in child
}

PROTECTED VIRTUAL void AosHandle::ProcessNetworkChanged()
{
    // Implemented in child
}

PROTECTED VIRTUAL void AosHandle::ProcessVopsStateChanged(
        IN IMS_UINT32 /*nState*/, IN IMS_BOOL /*bUpdateState = IMS_TRUE*/)
{
    // Implemented in child
}

PROTECTED VIRTUAL void AosHandle::ProcessPsRoamingStateChanged(IN IMS_UINT32 nState)
{
    m_nRoamingState = nState;
}

PROTECTED VIRTUAL void AosHandle::ProcessNetworkEvent(
        IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nExtraInfo)
{
    if (nType == IMS_EVENT_LTE_INFO)
    {
        A_IMS_TRACE_I(APPPROFILE, "ProcessNetworkEvent :: type(%d), state(%d)", nType, nState, 0);
        m_bCsVoiceAvailable = (nState == IMS_LTE_INFO_COMBINED_ATTACHED &&
                (nExtraInfo == IMS_LTE_INFO_EXTRA_NONE));
    }
}

PROTECTED VIRTUAL void AosHandle::AddListeners()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    if (piNetTracker != IMS_NULL)
    {
        piNetTracker->SetListener(this);
    }

    IAosService* piAosService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piAosService != IMS_NULL)
    {
        piAosService->AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        piAosService->AddListener(DYNAMIC_CAST(IAosServiceSettingListener*, this));
    }
}

PROTECTED VIRTUAL void AosHandle::RemoveListeners()
{
    IAosService* piAosService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piAosService != IMS_NULL)
    {
        piAosService->RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        piAosService->RemoveListener(DYNAMIC_CAST(IAosServiceSettingListener*, this));
    }

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    if (piNetTracker != IMS_NULL)
    {
        piNetTracker->RemoveListener(this);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::StateDisconnected(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "StateDisconnected :: (%s)", MsgToString(objMSG.nMSG), 0, 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
            if (IsBlocked())
            {
                // Don't care - already blocked
            }
            else
            {
                SetHandleState(STATE_CONNECTING);

                SetRequestType(ATTACH);
                m_piAppContext->GetApp()->Reconfig();

                m_bNotify = IMS_TRUE;
            }
            break;

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::StateConnecting(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "StateConnecting :: (%s)", MsgToString(objMSG.nMSG), 0, 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
            if (IsBlocked())
            {
                SetHandleState(STATE_DISCONNECTED);

                SetRequestType(DETACH);
                m_piAppContext->GetApp()->Reconfig();

                m_bNotify = IMS_TRUE;
                App_Notify();
            }
            else
            {
                ProcessFeatureTagChange();
            }
            break;

        case HANDLE_MSG_APP_STATUS:
        {
            IMS_UINT32 nState = LONG_TO_INT(objMSG.nWparam);
            SetReason(LONG_TO_INT(objMSG.nLparam));

            A_IMS_TRACE_I(APPPROFILE, "HANDLE_MSG_APP_STATUS :: State(%d), m_nReason(%d)", nState,
                    m_nReason, 0);

            switch (nState)
            {
                case IAosApplication::APP_CONNECTED:
                    if (IsRegBinded())
                    {
                        ClearSuspendedReason();
                        CheckSuspended();
                        SetHandleState(STATE_CONNECTED);
                    }

                    m_bNotify = IMS_TRUE;
                    break;

                case IAosApplication::APP_DISCONNECTED:
                    // report the failure to Enabler for silent/offline dialing
                    m_bNotify = IMS_TRUE;
                    break;

                default:
                    // Don't care
                    break;
            }
            break;
        }

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::StateConnected(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "StateConnected :: (%s)", MsgToString(objMSG.nMSG), 0, 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
            if (IsBlocked())
            {
                SetHandleState(STATE_DISCONNECTING);
                m_bNotify = IMS_TRUE;

                IMS_UINT32 nReason = LONG_TO_INT(objMSG.nWparam);

                if (nReason > 0)
                {
                    SetReason(nReason);
                    App_Notify();
                }

                SetRequestType(DETACH);
                m_piAppContext->GetApp()->Reconfig();
            }
            else
            {
                ProcessFeatureTagChange();
            }
            break;

        case HANDLE_MSG_APP_STATUS:
        {
            IMS_UINT32 nState = LONG_TO_INT(objMSG.nWparam);
            SetReason(LONG_TO_INT(objMSG.nLparam));

            A_IMS_TRACE_I(APPPROFILE, "HANDLE_MSG_APP_STATUS :: State(%d), m_nReason(%d)", nState,
                    m_nReason, 0);

            switch (nState)
            {
                case IAosApplication::APP_DISCONNECTED:
                    SetHandleState(STATE_CONNECTING);
                    m_bNotify = IMS_TRUE;
                    break;

                case IAosApplication::APP_CONNECTED:
                    if (!IsRegBinded())
                    {
                        if (IsBlocked())
                        {
                            SetHandleState(STATE_DISCONNECTED);
                        }
                        else
                        {
                            SetHandleState(STATE_CONNECTING);
                        }
                    }
                    m_bNotify = IMS_TRUE;
                    break;

                case IAosApplication::APP_DISCONNECTING:
                    // Enabler will release the resource as sending BYE
                    SetHandleState(STATE_DISCONNECTING);
                    m_bNotify = IMS_TRUE;
                    break;

                default:
                    break;
            }
            break;
        }

        default:
            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::StateDisconnecting(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "StateDisconnecting :: (%s)", MsgToString(objMSG.nMSG), 0, 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:

            if (IsBlocked())
            {
                SetRequestType(DETACH);
                m_piAppContext->GetApp()->Reconfig();
            }
            else
            {
                SetRequestType(ATTACH);
                SetHandleState(STATE_CONNECTING);
                m_piAppContext->GetApp()->Reconfig();
                m_bNotify = IMS_TRUE;
                App_Notify();
            }
            break;

        case HANDLE_MSG_APP_STATUS:
        {
            IMS_UINT32 nState = LONG_TO_INT(objMSG.nWparam);
            SetReason(LONG_TO_INT(objMSG.nLparam));

            A_IMS_TRACE_I(APPPROFILE, "HANDLE_MSG_APP_STATUS :: State(%d), m_nReason(%d)", nState,
                    m_nReason, 0);

            switch (nState)
            {
                case IAosApplication::APP_DISCONNECTED:
                    if (IsBlocked())
                    {
                        SetHandleState(STATE_DISCONNECTED);
                    }
                    else
                    {
                        SetHandleState(STATE_CONNECTING);
                    }

                    m_bNotify = IMS_TRUE;
                    break;

                case IAosApplication::APP_CONNECTED:
                    if (IsRegBinded())
                    {
                        if (!IsBlocked())
                        {
                            SetHandleState(STATE_CONNECTED);
                            m_bNotify = IMS_TRUE;
                        }
                    }
                    else
                    {
                        if (IsBlocked())
                        {
                            SetHandleState(STATE_DISCONNECTED);
                        }
                        else
                        {
                            SetHandleState(STATE_CONNECTING);
                        }

                        m_bNotify = IMS_TRUE;
                    }
                    break;

                case IAosApplication::APP_UPDATING:
                    if (IsRegBinded())
                    {
                        if (!IsBlocked())
                        {
                            SetHandleState(STATE_CONNECTING);
                            m_bNotify = IMS_TRUE;
                        }
                    }
                    else
                    {
                        if (IsBlocked())
                        {
                            SetHandleState(STATE_DISCONNECTED);
                        }
                        else
                        {
                            SetHandleState(STATE_CONNECTING);
                        }

                        m_bNotify = IMS_TRUE;
                    }
                    break;

                case IAosApplication::APP_DISCONNECTING:
                    m_bNotify = IMS_TRUE;
                    break;

                default:
                    break;
            }
            break;
        }
        default:

            break;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::IsBlocked() const
{
    A_IMS_TRACE_I(APPPROFILE, "IsBlocked :: (%s)", _TRACE_B_(m_bBlocked), 0, 0);

    return m_bBlocked;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::IsSupportedNetworkType(IN IMS_UINT32 nType) const
{
    return AosUtil::GetInstance()->IsSupportedNetworkType(nType);
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const
{
    return AosUtil::GetInstance()->IsSupportedNetworkTypeForCellular(nType);
}

PROTECTED VIRTUAL void AosHandle::InitializeHoldingBlocksPolicy()
{
    m_objHoldingBlocksPolicyForMobile.Clear();
    m_objHoldingBlocksPolicyForWifi.Clear();

    if (GET_N_CONFIG(m_nSlotId)->IsDeregOn3gNetwork())
    {
        m_objHoldingBlocksPolicyForMobile.Append(BLOCK_3G);
    }
}

PROTECTED VIRTUAL void AosHandle::InitializeServiceBlock()
{
    /*
        The child class will implement this method because the block condition may be changed
        according to Enabler or System like VoPS, Call Setting, etc

        The base class only processes the condition from Enabler
    */
}

PROTECTED VIRTUAL void AosHandle::InitializeServiceFeature()
{
    /*
        Will be implemented on child classes.
    */
}

PROTECTED VIRTUAL void AosHandle::InitializeFeatureTags()
{
    m_objFeatureTagList.ClearFeatureTags();

    // VZW Req. - VZ_REQ_IMS_22939
    if (GET_N_CONFIG(m_nSlotId)->IsCdmalessFeatureTagRequired())
    {
        m_objFeatureTagList.AddFeatureTag(FeatureTags::CDMALESS);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::ProcessImsSuspended(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsEmergencyService())
    {
        return IMS_FALSE;
    }

    if (IsImsConnected() == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsSuspended :: ims isn't connected", 0, 0, 0);
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsSuspended :: nReason(%d)", nReason, 0, 0);

    SetSuspendedReason(nReason);

    if (IsImsSuspended() == IMS_TRUE)
    {
        m_nReason = nReason;
        if (m_piListener != IMS_NULL)
        {
            m_piListener->ImsAos_Suspended(GetImsAosReasonForSuspend(m_nReason));

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::ProcessImsResumed(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsEmergencyService())
    {
        return IMS_FALSE;
    }

    if (IsImsConnected() == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsResumed :: ims isn't connected", 0, 0, 0);
        return IMS_FALSE;
    }

    if (IsImsSuspended() == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsResumed :: ims isn't suspended", 0, 0, 0);
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: nReason(%d)", nReason, 0, 0);

    ResetSuspendedReason(nReason);

    if (IsImsSuspended() == IMS_FALSE)
    {
        m_nReason = nReason;
        if (m_piListener != IMS_NULL)
        {
            m_piListener->ImsAos_Resumed();

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosHandle::CheckSuspended()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();

    A_IMS_TRACE_I(APPPROFILE, "CheckSuspended :: service (%s) >> (%s)",
            (m_bNetSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE",
            (bCurrSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE", 0);

    if (bCurrSrvIn == IMS_FALSE)
    {
        m_nSuspendedReason |= AosReason::SUSPEND_NO_SERVICE;
    }

    m_bNetSrvIn = bCurrSrvIn;
}

PROTECTED VIRTUAL void AosHandle::SetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AosReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason |= AosReason::SUSPEND_NO_SERVICE;
    }
}

PROTECTED VIRTUAL void AosHandle::ResetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AosReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason &= ~(AosReason::SUSPEND_NO_SERVICE);
    }
}

PROTECTED VIRTUAL void AosHandle::ReportRegState()
{
    IAosRegStateManager* piRSM = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (piRSM == IMS_NULL)
    {
        return;
    }

    switch (GetState())
    {
        case STATE_CONNECTED:
            piRSM->SetRegState(m_nServiceType, IMS_REG_ON);
            break;
        case STATE_DISCONNECTED:   // FALL-THROUGH
        case STATE_CONNECTING:     // FALL-THROUGH
        case STATE_DISCONNECTING:  // FALL-THROUGH
        default:
            piRSM->SetRegState(m_nServiceType, IMS_REG_OFF);
            break;
    }
}

PROTECTED VIRTUAL IMS_BOOL AosHandle::Is3G(IN IMS_UINT32 nNetworkType) const
{
    switch (nNetworkType)
    {
        case NW_REPORT_RADIO_WCDMA:  // FALL-THROUGH
        case NW_REPORT_RADIO_HSPA:
            return IMS_TRUE;

        default:
            return IMS_FALSE;
    }
}

PUBLIC VIRTUAL void AosHandle::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(APPPROFILE, "Event_NotifyEvent :: [E(%s)/W(%d)/L(%d)]",
            AosLog::EventToString(nEvent), nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_EVENT_IMS_VOICE_OVER_PS_STATE:
            if (!AosUtil::GetInstance()->IsWifiTest())
            {
                ProcessVopsStateChanged(nWParam);
            }
            break;

        case IMS_EVENT_ROAMING_STATE:
            // jryou: This will be added later
            // ProcessCsRoamingStateChanged(nLParam);

            ProcessPsRoamingStateChanged(nWParam);
            break;

        case IMS_EVENT_LTE_INFO:
            ProcessNetworkEvent(nEvent, nWParam, nLParam);
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void AosHandle::RegistrationControl_NotifyCapabilitiesChanged(
        IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities)
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    ProcessCapabilitiesChanged(objCapabilities);
}

PUBLIC VIRTUAL void AosHandle::ServiceSetting_RoamingPreferredVoiceNetworkChanged(
        IN RoamingPreferredVoiceNetwork /*eState*/)
{
}

PROTECTED GLOBAL const IMS_CHAR* AosHandle::StateToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case STATE_DISCONNECTED:
            return "STATE_DISCONNECTED";

        case STATE_CONNECTING:
            return "STATE_CONNECTING";

        case STATE_CONNECTED:
            return "STATE_CONNECTED";

        case STATE_DISCONNECTING:
            return "STATE_DISCONNECTING";

        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosHandle::MsgToString(IN IMS_UINT32 nMsg)
{
    switch (nMsg)
    {
        case HANDLE_MSG_BLOCK_STATUS:
            return "HANDLE_MSG_BLOCK_STATUS";

        case HANDLE_MSG_APP_STATUS:
            return "HANDLE_MSG_APP_STATUS";

        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosHandle::RadioTypeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case NW_REPORT_RADIO_WLAN:
            return "WLAN";

        case NW_REPORT_RADIO_LTE:
            return "LTE";

        case NW_REPORT_RADIO_NR:
            return "NR";

        case NW_REPORT_RADIO_WCDMA:  // FALL-THROUGH
        case NW_REPORT_RADIO_HSPA:
            return "3G";

        default:
            return "__INVALID__";
    }
}

PROTECTED
const IMS_CHAR* AosHandle::ServiceTypeToString()
{
    switch (m_nServiceType)
    {
        case ImsAosService::MTC:  // FALL-THROUGH
        case ImsAosService::EMERGENCY_MTC:
            return "mtc";

        case ImsAosService::MTS:  // FALL-THROUGH
        case ImsAosService::EMERGENCY_MTS:
            return "mts";

        case ImsAosService::UCE:
            return "uce";

        case ImsAosService::SIP_CONTROLLER:
            return "sip_controller";

        default:
            return "invalid";
    }
}