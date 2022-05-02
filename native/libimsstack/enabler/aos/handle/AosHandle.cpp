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
#include "ImsMessage.h"
#include "INetworkWatcher.h"

#include "AoSReason.h"
#include "AoSAppRequestType.h"
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
#include "interface/IAosService.h"

#include "provider/AosProvider.h"
#include "provider/AosLog.h"
#include "provider/AosUtil.h"
#include "handle/AosFeatureTag.h"
#include "handle/AosInfo.h"
#include "handle/AosHandle.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

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

/*

Remarks

*/
PUBLIC
AosHandle::AosHandle
    (
        IN IAosAppContext* piAppContext,
        IN const AString& strAppId,
        IN const AString& strServiceId,
        IN const IMS_UINT32 nServiceType
    )
    : m_piAppContext(piAppContext)
    , m_nSlotId(piAppContext->GetSlotId())
    , m_strAppId(strAppId)
    , m_strServiceId(strServiceId)
    , m_nServiceType(nServiceType)
    , m_nReqType(DETACH)
    , m_bBind(IMS_FALSE)
    , m_bNetworkBind(IMS_TRUE)
    , m_bNotify(IMS_FALSE)
    , m_piListener(IMS_NULL)
    , m_piMonitor(IMS_NULL)
    , m_piInfo(IMS_NULL)
    , m_nReason(AoSReason::NONE)
    , m_nSuspendedReason(AoSReason::SUSPEND_NONE)
    , m_bBlocked(IMS_FALSE)
    , m_nBlocks(BLOCK_NONE)
    , m_nHoldingBlocksForMobile(BLOCK_NONE)
    , m_nHoldingBlocksForWifi(BLOCK_NONE)
    , m_nHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED)
    , m_bNetSrvIn(IMS_FALSE)
    , m_nNetworkType(NW_REPORT_RADIO_INVALID)
    , m_nAppState(APP_STATE_DISCONNECTED)
{
    IMS_CHAR acLog[256+1] = {0, };
    IMS_Sprintf(acLog, 256, "APP(%s)/SERVICE(%s)/TYPE(%d)", m_strAppId.GetStr(),
            m_strServiceId.GetStr(), m_nServiceType);

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandle = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosHandle), this);

    m_strTag.Sprintf("%d:%s", m_nSlotId, m_piAppContext->GetProfileId().GetStr());

    SetHandleState(STATE_DISCONNECTED);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosHandle::~AosHandle()
{
    IMS_CHAR acLog[256+1] = {0, };
    IMS_Sprintf(acLog, 256, "APP(%s)/SERVICE(%s)/TYPE(%d)", m_strAppId.GetStr(),
            m_strServiceId.GetStr(), m_nServiceType);

    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandle = %" PFLS_u "/%" PFLS_x, acLog,
            sizeof(AosHandle), this);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AString& AosHandle::GetAppId()
{
    return m_strAppId;
}

/*

Remarks

*/
PUBLIC VIRTUAL
AString& AosHandle::GetServiceId()
{
    return m_strServiceId;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_UINT32 AosHandle::GetServiceType()
{
    return m_nServiceType;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IImsAosMonitor* AosHandle::GetMonitor()
{
    return m_piMonitor;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 AosHandle::GetRequestType()
{
    return m_nReqType;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::SetRequestType(IN IMS_SINT32 nReqType)
{
    A_IMS_TRACE_I(APPPROFILE, "SetRequestType :: [%s/%s] [%s]",
            m_strAppId.GetStr(), m_strServiceId.GetStr(),
            (nReqType == ATTACH)? "ATTACH" : "DETACH");

    m_nReqType = nReqType;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosHandle::IsRegBinded()
{
    return m_bBind;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::SetRegBinded(IN IMS_BOOL bBind)
{
    A_IMS_TRACE_I(APPPROFILE, "SetRegBinded :: [%s/%s] [%s]",
            m_strAppId.GetStr(), m_strServiceId.GetStr(),
            (bBind) ? "ATTACHED" : "DETACHED");

    m_bBind = bBind;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosHandle::IsNetworkRegBinded()
{
    return m_bNetworkBind;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::SetNetworkRegBinded(IN IMS_BOOL bNetworkBind)
{
    if (bNetworkBind == m_bNetworkBind)
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "SetNetworkRegBinded :: [%s/%s] [%s]",
            m_strAppId.GetStr(), m_strServiceId.GetStr(),
            (bNetworkBind) ? "ATTACHED" : "DETACHED");

    m_bNetworkBind = bNetworkBind;
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosFeatureTagList& AosHandle::GetFeatureTagList()
{
    return m_objFeatureTagList;
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosFeatureTagList& AosHandle::GetBindedFeatureTagList()
{
    return m_objBindedFeatureTagList;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::ProcessFeatureTagChange()
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessFeatureTagChange :: [%s/%s]",
            m_strAppId.GetStr(), m_strServiceId.GetStr(), 0);

    if (m_objFeatureTagList.Equals(m_objBindedFeatureTagList) == IMS_FALSE)
    {
        IMS_UINT32 nState = GetState();
        A_IMS_TRACE_I(APPPROFILE,
                "ProcessFeatureTagChange :: Feature tag is changed, nState [%s]",
                StateToString(nState), 0, 0);

        switch (nState)
        {
            case STATE_CONNECTING: // FALL-THROUGH
            case STATE_CONNECTED:
                m_piAppContext->GetApp()->Reconfig();
                break;

            case STATE_DISCONNECTING: // FALL-THROUGH
            case STATE_DISCONNECTED:
                break;
            default:
                break;
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState /* = 0 */)
{
    (void) nType;
    (void) nState;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::App_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nParam)
{
    A_IMS_TRACE_I(APPPROFILE, "App_StateChanged :: [%s/%s] [state(%d)]",
            m_strAppId.GetStr(), m_strServiceId.GetStr(), nState);

    switch (nState)
    {
        case IAosApplication::APP_DISCONNECTED: // FALL-THROUGH
        case IAosApplication::APP_CONNECTED: // FALL-THROUGH
        case IAosApplication::APP_UPDATING: // FALL-THROUGH
        case IAosApplication::APP_DISCONNECTING:
        {
            IMSMSG objMSG(HANDLE_MSG_APP_STATUS, nState, nParam);
            OnStateMessage(objMSG);
            break;
        }
        default:
            break;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::App_Notify()
{
    if (m_piListener == IMS_NULL)
    {
        A_IMS_TRACE_D(APPPROFILE, "App_Notify :: [%s/%s] no listener",
                m_strAppId.GetStr(), m_strServiceId.GetStr(), 0);
        return;
    }

    if (m_bNotify == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "App_Notify :: [%s/%s] no notification",
                m_strAppId.GetStr(), m_strServiceId.GetStr(), 0);
        return;
    }


    if (!CheckAppNotificationAndSetAppState())
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "App_Notify :: [%s/%s]",
            m_strAppId.GetStr(), m_strServiceId.GetStr(), 0);

    // notify the state to Enabler
    switch (GetState())
    {
        case STATE_DISCONNECTED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->ImsAos_Disconnected(GetImsAosReason(m_nReason));
            }
            break;
        case STATE_CONNECTING:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->ImsAos_Disconnected(GetImsAosReason(m_nReason));
            }
            break;
        case STATE_CONNECTED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->ImsAos_Connected(GetFeatures(), 0);
            }
            break;
        case STATE_DISCONNECTING:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->ImsAos_Disconnecting(GetImsAosReason(m_nReason));
            }
            break;
        default:
            break;
    }

    ReportRegState();
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked)
{
    (void) nType;
    (void) bBlocked;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosHandle::Control(IN IMS_UINT32 nType)
{
    A_IMS_TRACE_I(APPPROFILE, "Control :: Type[%d]", nType, 0, 0);

    return m_piAppContext->GetApp()->RequestCmd(nType);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IImsAosInfo* AosHandle::GetAosInfo()
{
    return m_piInfo;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_UINT32 AosHandle::GetFeatures()
{
    return IsImsConnected() ? m_objBindedFeatureTagList.GetFeatures() : ImsAosFeature::NONE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_UINT32 AosHandle::GetSuspendedReason()
{
    return m_nSuspendedReason;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosHandle::IsFeatureConnected(IN IMS_UINT32 nFeature)
{
    return (GetFeatures() & nFeature) > 0;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosHandle::IsImsConnected()
{
    IMS_UINT32 nState = GetState();

    A_IMS_TRACE_D(APPPROFILE, "IsImsConnected :: [%s] connected(%s)",
            m_strServiceId.GetStr(), _TRACE_B_(nState == STATE_CONNECTED), 0);

    return (nState == STATE_CONNECTED);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosHandle::IsImsSuspended()
{
    A_IMS_TRACE_D(APPPROFILE, "IsImsSuspended :: [%s] reason(%x)",
            m_strServiceId.GetStr(), m_nSuspendedReason, 0);

    return (m_nSuspendedReason != AoSReason::SUSPEND_NONE);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::SetListener(IN IImsAosListener* piListener)
{
    m_piListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::SetMonitor(IN IImsAosMonitor* piMonitor)
{
    m_piMonitor = piMonitor;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::UpdateFeature(IN IMS_UINT32 /*nFeatures*/)
{
    // It is for SipController so it will be overridden in AosSipController
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::UpdateFeature(IN IMSList<ImsAosFeatureTag*>& /*objFeatureTag*/)
{
    // It is for SipController so it will be overridden in AosSipControllers
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    A_IMS_TRACE_D(APPPROFILE,
            "CallTracker_StateChanged :: nType=%d, nState=%d", nType, nState, 0);
    // Implemented in child
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::NetTracker_StatusChanged()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();

    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();

    if (bCurrSrvIn != m_bNetSrvIn)
    {
        A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: service state changed >> (%s)",
                (bCurrSrvIn)? "IN_SERVICE" : "OUT_OF_SERVICE", 0, 0);

        // IN Service
        if (bCurrSrvIn)
        {
            ProcessImsResumed(AoSReason::SUSPEND_NO_SERVICE);
        }
        // OUT service
        else
        {
            ProcessImsSuspended(AoSReason::SUSPEND_NO_SERVICE);
        }

        m_bNetSrvIn = bCurrSrvIn;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    if (piNetTracker != IMS_NULL)
    {
        piNetTracker->SetListener(this);
    }

    IAosService* piAosService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piAosService != IMS_NULL)
    {
        piAosService->AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        piAosService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
        piAosService->AddListener(DYNAMIC_CAST(IAosServiceSettingListener*, this));
    }

    m_piInfo = new AosInfo(m_piAppContext);

    InitializeServiceBlock();
    InitializeServiceFeature();
    InitializeFeatureTags();

    IMSMSG objMSG(HANDLE_MSG_BLOCK_STATUS, 0, 0);
    OnStateMessage(objMSG);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    if (piNetTracker != IMS_NULL)
    {
        piNetTracker->RemoveListener(this);
    }

    IAosService* piAosService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piAosService != IMS_NULL)
    {
        piAosService->RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, this));
        piAosService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
        piAosService->RemoveListener(DYNAMIC_CAST(IAosServiceSettingListener*, this));
    }

    if (m_piInfo != IMS_NULL)
    {
        delete DYNAMIC_CAST(AosInfo*, m_piInfo);
        m_piInfo = IMS_NULL;
    }
}

/*

Remarks

*/
PROTECTED
void AosHandle::SetHandleState(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(APPPROFILE, "SetHandleState :: %s >> %s",
            StateToString(GetState()), StateToString(nState), 0);

    SetState(nState);

    if (nState == STATE_DISCONNECTED || nState == STATE_DISCONNECTING)
    {
        ClearSuspendedReason();
    }
}

/*

Remarks

*/
PROTECTED
void AosHandle::SetReason(IN IMS_UINT32 nReason)
{
    this->m_nReason = nReason;
}

/*

Remarks

*/
PROTECTED
void AosHandle::ClearSuspendedReason()
{
    A_IMS_TRACE_D(APPPROFILE, "ClearSuspendedReason", 0, 0, 0);
    m_nSuspendedReason = AoSReason::SUSPEND_NONE;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::CheckAppNotificationAndSetAppState()
{
    return IMS_TRUE;
    /* TODO_CONFIG
    if (!AosUtil::GetInstance()->IsFeatureOn(GetServiceType(),
            m_piAppContext->GetConfig()->GetCheckingAppNotificationSupportedServices()))
    {
        return IMS_TRUE;
    }

    IMS_UINT32 nReportState = GetAppState();
    if (m_nAppState == nReportState)
    {
        return (nReportState == APP_STATE_CONNECTED) ? IMS_TRUE : IMS_FALSE;
    }

    m_nAppState = nReportState;
    return IMS_TRUE;
    */
}

/*

Remarks

*/
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

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetImsAosReason(IN IMS_UINT32 nAosReason)
{
    IMS_UINT32 nImsAosReason = ImsAosReason::NONE;

    switch (nAosReason)
    {
        case AoSReason::NONE:
            break;
        case AoSReason::BAD_BATTERY: // FALL-THROUGH
        case AoSReason::POWER_OFF:
            nImsAosReason = ImsAosReason::POWER_OFF;
            break;
        case AoSReason::AIRPLANE_MODE: //FALL-THROUGH
        case AoSReason::DATA_DISCONNECTED:
            nImsAosReason = ImsAosReason::DATA_DISCONNECTED;
            break;
        case AoSReason::NO_LTE_COVERAGE:
            nImsAosReason = ImsAosReason::NO_RAT_COVERAGE;
            break;
        case AoSReason::SERVICE_POLICY:
            nImsAosReason = ImsAosReason::SERVICE_POLICY;
            break;
        case AoSReason::SERVICE_BLOCKED:
            nImsAosReason = ImsAosReason::SERVICE_BLOCKED;
            break;
        case AoSReason::SRV_OUT:
            nImsAosReason = ImsAosReason::OUT_OF_SERVICE;
            break;
        case AoSReason::REG_REFRESH_FORBIDDEN: // FALL-THROUGH
        case AoSReason::REG_TERMINATED_EXPIRE: // FALL-THROUGH
        case AoSReason::REG_TERMINATED:
            nImsAosReason = ImsAosReason::REG_TERMINATED;
            break;
        case AoSReason::INITIAL_REG_REQUESTED:
            nImsAosReason = ImsAosReason::REG_NEW_REQUIRED;
            break;
        default:
            nImsAosReason = ImsAosReason::NOT_SPECIFIED;
            break;
    }

    return nImsAosReason;
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetImsAosReasonForSuspend(IN IMS_UINT32 nAosReason)
{
    IMS_UINT32 nImsAosReason = ImsAosReason::SUSPEND_NONE;

    switch (nAosReason)
    {
        case AoSReason::SUSPEND_NONE:
            break;
        case AoSReason::SUSPEND_NO_SERVICE:
            nImsAosReason = ImsAosReason::SUSPEND_OUT_OF_SERVICE;
            break;
        case AoSReason::SUSPEND_NO_LTE_COVERAGE:
            nImsAosReason = ImsAosReason::SUSPEND_NO_RAT_COVERAGE;
            break;
        default:
            break;
    }

    return nImsAosReason;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::IsEpdgEnabled() const
{
    return m_piAppContext->GetConnection()->IsEpdgEnabled();
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::IsEqualNetworkType(IN IMS_UINT32 nType, IN AosNetworkType eType) const
{
    return (nType == static_cast<IMS_UINT32>(eType));
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::IsCapabilityExisted(IN IMS_UINT32 nCapabilities,
        IN AosCapability eCapability) const
{
    return (nCapabilities & static_cast<IMS_UINT32>(eCapability));
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::IsNetworkTypeMatchedToRat(IMS_UINT32 nNetworkType, IMS_UINT32 nRat)
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

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetNetworkType() const
{
    return m_piAppContext->GetNetTracker()->GetNetworkType();
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetMobileNetworkType() const
{
    return m_piAppContext->GetNetTracker()->GetMobileNetworkType();
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetBlock(IN IMS_UINT32 nEvent)
{
    switch (nEvent)
    {
        case IMS_EVENT_IMS_VOICE_OVER_PS_STATE:
            return BLOCK_VOPS;

        default:
            break;
    }

    return BLOCK_NONE;
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetAosReason(IN IMS_UINT32 nBlock)
{
    switch (nBlock)
    {
        case BLOCK_NETWORK:
            return AoSReason::NO_LTE_COVERAGE;

        default:
            break;
    }

    return AoSReason::NONE;
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosHandle::GetAosFeature(IN IMS_UINT32 nBlock)
{
    IMS_UINT32 nFeature = ImsAosFeature::NONE;

    switch (nBlock)
    {
        case BLOCK_VOLTE_CAPABILITY: // FALL-THROUGH
        case BLOCK_VOWIFI_CAPABILITY: // FALL-THROUGH
        case BLOCK_NETWORK: // FALL-THROUGH
        case BLOCK_VOPS:
            nFeature = ImsAosFeature::MMTEL;
            break;

        case BLOCK_VILTE_CAPABILITY: // FALL-THROUGH
        case BLOCK_VIWIFI_CAPABILITY:
            nFeature = ImsAosFeature::VIDEO;
            break;

        case BLOCK_SMS_OVER_IP_NETWORK_INDICATION:
            nFeature = ImsAosFeature::SMSIP;
            break;

        default:
            break;
    }

    return nFeature;
}

/*

Remarks

*/
PROTECTED
void AosHandle::AddBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
{
    nBlocks |= nBlock;
}

/*

Remarks

*/
PROTECTED
void AosHandle::RemoveBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
{
    nBlocks &= ~(nBlock);
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::PreProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded)
{
    if (IsEpdgEnabled())
    {
        if (IsBlockForWifi(nBlock))
        {
            return IMS_FALSE;
        }

        if (bAdded)
        {
            AddBlock(nBlock, m_nHoldingBlocksForMobile);
        }
        else
        {
            RemoveBlock(nBlock, m_nHoldingBlocksForMobile);
        }

        A_IMS_TRACE_D(ServiceTypeToString(), "PreProcessBlock :: nHoldingBlocksForMobile (%x)",
                m_nHoldingBlocksForMobile, 0, 0);

        return IMS_TRUE;
    }

    if (IsBlockForMobile(nBlock))
    {
        return IMS_FALSE;
    }

    if (bAdded)
    {
        AddBlock(nBlock, m_nHoldingBlocksForWifi);
    }
    else
    {
        RemoveBlock(nBlock, m_nHoldingBlocksForWifi);
    }

    A_IMS_TRACE_D(ServiceTypeToString(), "PreProcessBlock :: nHoldingBlocksForWifi (%x)",
            m_nHoldingBlocksForWifi, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
void AosHandle::ProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded,
        IN IMS_BOOL bPreProcess /* = IMS_TRUE */)
{
    if (bPreProcess && GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        if (PreProcessBlock(nBlock, bAdded))
        {
            A_IMS_TRACE_D(APPPROFILE, "ProcessBlock :: WIFI(%s) , not handled(%x)",
                _TRACE_B_(IsEpdgEnabled()), m_nBlocks, 0);
            return;
        }
    }

    if (bAdded)
    {
        AddBlock(nBlock, m_nBlocks);
    }
    else
    {
        RemoveBlock(nBlock, m_nBlocks);
    }

    ProcessFeatureBlock(GetAosFeature(nBlock), bAdded);

    ProcessCheckBlock(nBlock);
}

/*

Remarks

*/
PROTECTED
void AosHandle::ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked)
{
    if (bBlocked)
    {
        m_objFeatureTagList.RemoveFeature(nFeature);
    }
    else
    {
        m_objFeatureTagList.AddFeature(nFeature);
    }

    A_IMS_TRACE_D(APPPROFILE, "ProcessFeatureBlock :: [%s/%s] Updated feature = [%d]",
            m_strAppId.GetStr(), m_strServiceId.GetStr() , m_objFeatureTagList.GetFeatures());

    UpdateFeatureTags();
}

/*

Remarks

*/
PROTECTED
void AosHandle::ProcessCheckBlock(IN IMS_UINT32 nBlock/* = 0 */,
        IN IMS_BOOL bRunStateMachine/* = IMS_TRUE */)
{
    IMS_BOOL bCurrBlocked = IMS_FALSE;

    bCurrBlocked = IsHandleBlocked();

    A_IMS_TRACE_D(APPPROFILE, "ProcessCheckBlock :: old(%s) -> current(%s) , blocks(%x)",
            _TRACE_B_(m_bBlocked), _TRACE_B_(bCurrBlocked), m_nBlocks);

    if (m_bBlocked != bCurrBlocked)
    {
        m_bBlocked = bCurrBlocked;

        if (bRunStateMachine)
        {
            IMS_UINT32 nReason = (m_bBlocked) ? GetAosReason(nBlock) : 0;
            IMSMSG objMSG(HANDLE_MSG_BLOCK_STATUS, nReason, 0);
            OnStateMessage(objMSG);
        }

        ProcessBlockChanged(); // jryou: this was for update "cs,volte"
    }
    else
    {
        if (!m_bBlocked)
        {
            ProcessFeatureTagChange();
        }
    }
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosHandle::IsHandleBlocked(IN IMS_UINT32 nType) const
{
    return (m_nBlocks & nType);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::IsHandleBlocked() const
{
    return (m_nBlocks != BLOCK_NONE);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ProcessBlockChanged()
{
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::IsBlockForMobile(IN IMS_UINT32 /* nBlock */) const
{
    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::IsBlockForWifi(IN IMS_UINT32 /* nBlock */) const
{
    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ProcessCapabilitiesChanged(
        IN const IMSMap<IMS_UINT32, IMS_UINT32>& /* objNewCapabilities */)
{
    // Implemented in child
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ProcessNetworkChanged()
{
    // Implemented in child
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ProcessVopsStateChanged(IN IMS_UINT32 /*nState*/)
{
    // Implemented in child
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::StateDisconnected(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "[%s] StateDisconnected :: (%s)",
            m_strServiceId.GetStr(), MsgToString(objMSG.nMSG), 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
        {
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
        }
        case HANDLE_MSG_APP_STATUS:
        {
            // Don't care
            break;
        }
        default:
        {
            break;
        }

    }
    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::StateConnecting(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "[%s] StateConnecting :: (%s)" ,
            m_strServiceId.GetStr(), MsgToString(objMSG.nMSG), 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
        {
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
                // Don't care - already connecting
            }
            break;
        }

        case HANDLE_MSG_APP_STATUS:
        {
            IMS_UINT32 nState = LONG_TO_INT(objMSG.nWparam);
            SetReason(LONG_TO_INT(objMSG.nLparam));

            IMS_TRACE_I("HANDLE_MSG_APP_STATUS :: State(%d) , m_nReason(%d)", nState, m_nReason, 0);

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
        {
            break;
        }

    }
    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::StateConnected(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "[%s] StateConnected :: (%s)" ,
            m_strServiceId.GetStr(), MsgToString(objMSG.nMSG), 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
        {
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
                // Don't care
            }
            break;
        }

        case HANDLE_MSG_APP_STATUS:
        {
            IMS_UINT32 nState = LONG_TO_INT(objMSG.nWparam);
            SetReason(LONG_TO_INT(objMSG.nLparam));

            IMS_TRACE_I("HANDLE_MSG_APP_STATUS :: State(%d) , m_nReason(%d)", nState, m_nReason, 0);

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

                case IAosApplication::APP_UPDATING:
                    m_bNotify = IMS_FALSE;
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
        {
            break;
        }

    }
    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::StateDisconnecting(IN IMSMSG& objMSG)
{
    A_IMS_TRACE_I(APPPROFILE, "[%s] StateDisconnecting :: (%s)",
            m_strServiceId.GetStr(), MsgToString(objMSG.nMSG), 0);

    m_bNotify = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case HANDLE_MSG_BLOCK_STATUS:
        {
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
        }
        case HANDLE_MSG_APP_STATUS:
        {
            IMS_UINT32 nState = LONG_TO_INT(objMSG.nWparam);
            SetReason(LONG_TO_INT(objMSG.nLparam));

            IMS_TRACE_I("HANDLE_MSG_APP_STATUS :: State(%d) , m_nReason(%d)", nState, m_nReason, 0);

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
        {
            break;
        }

    }
    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::IsBlocked() const
{
    A_IMS_TRACE_I(APPPROFILE, "IsBlocked :: (%s)", _TRACE_B_(m_bBlocked), 0, 0);

    return m_bBlocked;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::IsSupportedNetworkType(IN IMS_UINT32 nType) const
{
    return AosUtil::GetInstance()->IsSupportedNetworkType(nType);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandle::IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const
{
    return AosUtil::GetInstance()->IsSupportedNetworkTypeForCellular(nType);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::InitializeServiceBlock()
{
    /*
        The child class will implement this method because the block condition may be changed
        according to Enabler or System like VoPS, Call Setting, etc

        The base class only processes the condition from Enabler
    */
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::InitializeServiceFeature()
{
    /*
        Will be implemented on child classes.
    */
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::InitializeFeatureTags()
{
    m_objFeatureTagList.ClearFeatureTags();

    // VZW Req. - VZ_REQ_IMS_22939
    if (GET_N_CONFIG(m_nSlotId)->IsCdmalessFeatureTagRequired())
    {
        m_objFeatureTagList.AddFeatureTag("+cdmaless");
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::UpdateFeatureTags()
{
    if (m_objFeatureTagList.GetFeatures() == ImsAosFeature::NONE)
    {
        m_objFeatureTagList.ClearFeatureTags();
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ProcessImsSuspended(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsImsConnected() == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsSuspended :: ims isn't connected",
                0, 0, 0);
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsSuspended :: nReason(%d)", nReason, 0, 0);

    SetSuspendedReason(nReason);

    if (IsImsSuspended() == IMS_TRUE)
    {
        m_nReason = nReason;
        if (m_piListener != IMS_NULL)
        {
            m_piListener->ImsAos_Suspended(GetImsAosReasonForSuspend(m_nReason));
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ProcessImsResumed(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsImsConnected() == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsResumed :: ims isn't connected", 0, 0, 0);
        return;
    }

    if (IsImsSuspended() == IMS_FALSE)
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsResumed :: ims isn't suspended", 0, 0, 0);
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: nReason(%d)", nReason, 0, 0);

    ResetSuspendedReason(nReason);

    if (IsImsSuspended() == IMS_FALSE)
    {
        m_nReason = nReason;
        if (m_piListener != IMS_NULL)
        {
            m_piListener->ImsAos_Resumed();
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::CheckSuspended()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();

    A_IMS_TRACE_I(APPPROFILE, "CheckSuspended :: service (%s) >> (%s)",
            (m_bNetSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE",
            (bCurrSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE", 0);

    if (bCurrSrvIn == IMS_FALSE)
    {
        m_nSuspendedReason |= AoSReason::SUSPEND_NO_SERVICE;
    }

    m_bNetSrvIn = bCurrSrvIn;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::SetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AoSReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason |= AoSReason::SUSPEND_NO_SERVICE;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ResetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AoSReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason &= ~(AoSReason::SUSPEND_NO_SERVICE);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandle::ReportRegState()
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
        case STATE_DISCONNECTED: // FALL-THROUGH
        case STATE_CONNECTING: // FALL-THROUGH
        case STATE_DISCONNECTING: // FALL-THROUGH
        default:
            piRSM->SetRegState(m_nServiceType, IMS_REG_OFF);
            break;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(APPPROFILE, "Event_NotifyEvent :: [E(%s)/W(%d)/L(%d)]",
            AosLog::EventToString(nEvent), nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_EVENT_IMS_VOICE_OVER_PS_STATE:
            ProcessVopsStateChanged(nWParam);
            break;

        case IMS_EVENT_OMADM_UPDATED:
            // jryou: This will be added later

            // ProcessDMUpdated(nWParam, nLParam);
            break;

        case IMS_EVENT_ROAMING_STATE:
            // jryou: This will be added later

            // ProcessCsRoamingStateChanged(nLParam);
            // ProcessLteRoamingStateChanged(nWParam);
            break;

        case IMS_EVENT_ROAMING_PREFERRED_VOICE_CALL_NETWORK:
            // jryou: This will be added later

            // ProcessRoamingPreferenceNetwork(nWParam);
            break;

        case IMS_EVENT_CONFIG_UPDATE:
            // jryou: This will be added later

            // if sms over ip
            break;

        default:
            break;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::RegistrationControl_NotifyCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objCapabilities)
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    ProcessCapabilitiesChanged(objCapabilities);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandle::ServiceSetting_RoamingPreferredVoiceNetworkChanged(
        IN RoamingPreferredVoiceNetwork /*eState*/)
{

}

/*

Remarks

*/
PROTECTED GLOBAL
const IMS_CHAR* AosHandle::StateToString(IN IMS_UINT32 nState)
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

/*

Remarks

*/
PROTECTED GLOBAL
const IMS_CHAR* AosHandle::MsgToString(IN IMS_UINT32 nMsg)
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

/*

Remarks

*/
PROTECTED GLOBAL
const IMS_CHAR* AosHandle::RadioTypeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case NW_REPORT_RADIO_WLAN:
            return "WLAN";

        case NW_REPORT_RADIO_LTE:
            return "LTE";

        case NW_REPORT_RADIO_NR:
            return "NR";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PROTECTED
const IMS_CHAR* AosHandle::ServiceTypeToString()
{
    switch (m_nServiceType)
    {
        case ImsAosService::MTC:
            return "SERVICE_MTC";

        case ImsAosService::MTS:
            return "SERVICE_MTS";

        case ImsAosService::EMERGENCY_MTC:
            return "SERVICE_EMERGENCY_MTC";

        case ImsAosService::EMERGENCY_MTS:
            return "SERVICE_EMERGENCY_MTS";

        case ImsAosService::UCE:
            return "SERVICE_UCE";

        case ImsAosService::SIP_CONTROLLER:
            return "SERVICE_SIP_CONTROLLER";

        default:
            return "__INVALID__";
    }
}
