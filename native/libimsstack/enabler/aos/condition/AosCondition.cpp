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
#include "IImsPrivateProperty.h"
#include "ServiceEvent.h"
#include "ServiceUtil.h"
#include "ServiceTrace.h"
#include "ServiceNetworkPolicy.h"
#include "AosReason.h"
#include "IAosService.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "interface/IAosConditionListener.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriber.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"
#include "condition/AosBlock.h"
#include "condition/AosServiceAvailableCellular.h"
#include "condition/AosServiceAvailableWifi.h"
#include "condition/AosCondition.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosCondition::AosCondition(IN IAosAppContext* piAppContext) :
        m_piAppContext(piAppContext),
        m_nSlotId(m_piAppContext->GetSlotId()),
        m_piListener(IMS_NULL),
        m_pAvailableCellular(IMS_NULL),
        m_pAvailableWifi(IMS_NULL),
        m_piBlock(m_piAppContext->GetBlock()),
        m_eServiceType(GetServiceType()),
        m_bIsRefreshStarted(IMS_FALSE),
        m_bIsCombinedAttached(IMS_FALSE),
        m_bCellServiceAvailable(IMS_FALSE),
        m_bWifiServiceAvailable(IMS_FALSE),
        m_bIsTtyOn(IMS_FALSE),
        m_nHoldEvents(HOLD_EVENT_NONE),
        m_nListeners(LISTENER_ALL)
{
    m_strTag.Sprintf("%d:%s", m_nSlotId, m_piAppContext->GetProfileId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosCondition = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosCondition), this);

    Init();
}

PUBLIC VIRTUAL AosCondition::~AosCondition()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosCondition = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosCondition), this);
}

PUBLIC VIRTUAL void AosCondition::Start()
{
    A_IMS_TRACE_I(APPPROFILE, "Start", 0, 0, 0);

    AddServiceAvailable();
    AddAosServiceListener();
    AddEventListener();

    if (IsListenerEnabled(LISTENER_BLOCK))
    {
        SetInitialBlockReason();
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->SetListener(this);
        }
    }

    if (IsListenerEnabled(LISTENER_NETTRACKER))
    {
        IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
        if (piNetTracker != IMS_NULL)
        {
            piNetTracker->SetListener(this);
            piNetTracker->SetRatGuardTime(RAT_CHANGE_GUARD_TIME_MILLIS);
        }
    }

    if (IsListenerEnabled(LISTENER_SUBSCRIBER))
    {
        IAosSubscriber* piSubscriber = m_piAppContext->GetSubscriber();
        if (piSubscriber != IMS_NULL)
        {
            piSubscriber->SetListener(this);
        }
    }

    if (IsListenerEnabled(LISTENER_CALLTRACKER))
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCallTracker != IMS_NULL)
        {
            piCallTracker->SetListener(this);
        }
    }
}

PUBLIC VIRTUAL void AosCondition::Stop()
{
    A_IMS_TRACE_I(APPPROFILE, "Stop", 0, 0, 0);

    if (IsListenerEnabled(LISTENER_CALLTRACKER))
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);

        if (piCallTracker != IMS_NULL)
        {
            piCallTracker->RemoveListener(this);
        }
    }

    if (IsListenerEnabled(LISTENER_SUBSCRIBER))
    {
        IAosSubscriber* piSubscriber = m_piAppContext->GetSubscriber();
        if (piSubscriber != IMS_NULL)
        {
            piSubscriber->SetListener(IMS_NULL);
        }
    }

    if (IsListenerEnabled(LISTENER_NETTRACKER))
    {
        IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
        if (piNetTracker != IMS_NULL)
        {
            piNetTracker->RemoveListener(this);
        }
    }

    if (IsListenerEnabled(LISTENER_BLOCK))
    {
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->RemoveListener(this);
        }
    }

    RemoveEventListener();
    RemoveAosServiceListener();
    RemoveServiceAvailable();
}

PUBLIC VIRTUAL void AosCondition::SetListener(IN IAosConditionListener* piListener)
{
    A_IMS_TRACE_D(APPPROFILE, "SetListener :: (%" PFLS_x ") is set", piListener, 0, 0);

    m_piListener = piListener;
}

PUBLIC VIRTUAL void AosCondition::SetBlock(
        IN BLOCK_REASON eReason, IN IMS_BOOL bNotify /* = IMS_TRUE */)
{
    m_piBlock->SetBlockReason(eReason, bNotify);
}

PUBLIC VIRTUAL void AosCondition::ResetBlock(
        IN BLOCK_REASON eReason, IN IMS_BOOL bNotify /* = IMS_TRUE */)
{
    m_piBlock->ResetBlockReason(eReason, bNotify);
}

PUBLIC VIRTUAL IMS_BOOL AosCondition::IsReasonBlocked(IN BLOCK_REASON eReason) const
{
    return m_piBlock->IsReasonBlocked(eReason);
}

PUBLIC VIRTUAL IMS_BOOL AosCondition::IsReady()
{
    IMS_BOOL bReturn;

    if (m_eServiceType == SERVICE_CELLULAR)
    {
        bReturn = m_bCellServiceAvailable;
    }
    else if (m_eServiceType == SERVICE_WIFI)
    {
        bReturn = m_bWifiServiceAvailable;
    }
    else  // if (m_eServiceType == SERVICE_WHOLE)
    {
        bReturn = (m_bCellServiceAvailable || m_bWifiServiceAvailable);
    }

    A_IMS_TRACE_I(APPPROFILE, "IsReady(%s) - Cellular(%s), WiFi(%s)", _TRACE_B_(bReturn),
            _TRACE_B_(m_bCellServiceAvailable), _TRACE_B_(m_bWifiServiceAvailable));
    return bReturn;
}

PUBLIC VIRTUAL IMS_UINT32 AosCondition::CheckServiceAvailable(IN SERVICE_TYPE eType)
{
    IMS_UINT32 nCheck = CHECK_NONE;

    if (m_pAvailableCellular != IMS_NULL && (eType == SERVICE_CELLULAR || eType == SERVICE_WHOLE))
    {
        m_pAvailableCellular->RefreshServiceAvailability();
        nCheck |= CHECK_CELLULAR;
    }

    if (m_pAvailableWifi != IMS_NULL && (eType == SERVICE_WIFI || eType == SERVICE_WHOLE))
    {
        m_pAvailableWifi->RefreshServiceAvailability();
        nCheck |= CHECK_WIFI;
    }

    return nCheck;
}

PUBLIC VIRTUAL IMS_BOOL AosCondition::CheckBadNetwork(IN SERVICE_TYPE eType)
{
    A_IMS_TRACE_D(APPPROFILE, "CheckBadNetwork :: Type[%d]", eType, 0, 0);
    IMS_BOOL bCheck = IMS_FALSE;

    if (m_pAvailableWifi == IMS_NULL || eType != SERVICE_WIFI)
    {
        return bCheck;
    }

    if (m_bCellServiceAvailable)
    {
        m_pAvailableWifi->StartToCheckNetworkConnection();
        bCheck = IMS_TRUE;
    }
    else
    {
        A_IMS_TRACE_D(APPPROFILE,
                "CheckBadNetwork :: Need to check network, but cellular is unavailable", 0, 0, 0);
    }

    return bCheck;
}

PUBLIC VIRTUAL void AosCondition::PrintBlockReasons() const
{
    m_piBlock->PrintBlockReasons();
}

PROTECTED VIRTUAL void AosCondition::AddServiceAvailable()
{
    A_IMS_TRACE_D(APPPROFILE, "AddServiceAvailable", 0, 0, 0);

    m_pAvailableCellular = new AosServiceAvailableCellular();
    m_pAvailableCellular->Init(m_piAppContext);
    m_pAvailableCellular->SetListener(this);

    m_pAvailableWifi = new AosServiceAvailableWifi();
    m_pAvailableWifi->Init(m_piAppContext);
    m_pAvailableWifi->SetListener(this);
}

PROTECTED VIRTUAL void AosCondition::RemoveServiceAvailable()
{
    A_IMS_TRACE_D(APPPROFILE, "RemoveServiceAvailable", 0, 0, 0);

    if (m_pAvailableWifi != IMS_NULL)
    {
        m_pAvailableWifi->RemoveListener(this);
        m_pAvailableWifi->CleanUp();
        delete m_pAvailableWifi;
        m_pAvailableWifi = IMS_NULL;
    }

    if (m_pAvailableCellular != IMS_NULL)
    {
        m_pAvailableCellular->RemoveListener(this);
        m_pAvailableCellular->CleanUp();
        delete m_pAvailableCellular;
        m_pAvailableCellular = IMS_NULL;
    }
}

PROTECTED VIRTUAL void AosCondition::AddAosServiceListener()
{
    IAosService* pService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (pService != IMS_NULL)
    {
        pService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
        pService->AddListener(DYNAMIC_CAST(IAosServiceSettingListener*, this));
    }
}

PROTECTED VIRTUAL void AosCondition::RemoveAosServiceListener()
{
    IAosService* pService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (pService != IMS_NULL)
    {
        pService->RemoveListener(DYNAMIC_CAST(IAosServiceSettingListener*, this));
        pService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }
}

PROTECTED VIRTUAL void AosCondition::AddEventListener()
{
    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_nSlotId);

    if (GET_N_CONFIG(m_nSlotId) == IMS_NULL)
    {
        return;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsVoLteRoamingAvailable() == IMS_FALSE)
    {
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
    }
}

PROTECTED VIRTUAL void AosCondition::RemoveEventListener()
{
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_nSlotId);
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
}

PROTECTED VIRTUAL void AosCondition::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(APPPROFILE, "Event_NotifyEvent :: [E(%s)/W(%s)/L(%d)]",
            AosLog::EventToString(nEvent), (nWParam > 0) ? "ON" : "OFF", nLParam);

    switch (nEvent)
    {
        case IMS_EVENT_ROAMING_STATE:
            ProcessRoamingEvent(nWParam, nLParam);
            break;

        case IMS_EVENT_IMS_VOICE_OVER_PS_STATE:
            ProcessImsVopsEvent(nWParam);
            break;

        case IMS_EVENT_LTE_INFO:
            ProcessLteInfoEvent(nWParam, nLParam);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosCondition::CallTracker_StateChanged(
        IN IMS_UINT32 nType, IN CallState eState)
{
    A_IMS_TRACE_I(APPPROFILE, "(%s) Call state [%d]",
            nType == IAosCallTracker::TYPE_CS ? "CS" : "Normal", static_cast<IMS_UINT32>(eState),
            0);

    if (nType == IAosCallTracker::TYPE_CS)
    {
        if (eState == CallState::OFFHOOK)
        {
            SetBlock(BLOCK_CSCALL_STARTED);
        }
        else
        {
            ResetBlock(BLOCK_CSCALL_STARTED);
        }
    }

    SendConditionEvent(
            AosServiceAvailable::EVENT_CALL, nType, static_cast<IMS_UINT32>(eState), SERVICE_WIFI);
}

PROTECTED VIRTUAL void AosCondition::NetTracker_StatusChanged()
{
    A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged()", 0, 0, 0);
    SendConditionEvent(AosServiceAvailable::EVENT_NETWORK, 0, SERVICE_CELLULAR);
}

PROTECTED VIRTUAL void AosCondition::Subscriber_StateChanged(
        IN IMS_UINT32 nState, IN IMS_UINT32 nParam /* = 0 */)
{
    (void)nParam;

    if (GET_N_CONFIG(m_nSlotId) != IMS_NULL &&
            GET_N_CONFIG(m_nSlotId)->IsSupportLimitedAdminSmsMode())
    {
        UpdateRegistrationMode();
    }

    switch (nState)
    {
        case IAosSubscriber::REFRESH_STARTED:
            RequestCommand(REQUEST_STOP);
            ProcessBlockReason(IMS_TRUE, BLOCK_SUBSCRIBER_INCOMPLETED);
            m_bIsRefreshStarted = IMS_TRUE;
            break;

        case IAosSubscriber::REFRESH_COMPLETED:  // FALL-THROUGH
        case IAosSubscriber::READY:
            ProcessBlockReason(IMS_FALSE, BLOCK_SUBSCRIBER_INCOMPLETED);
            if (IsRefreshStarted())
            {
                m_bIsRefreshStarted = IMS_FALSE;
                ClearRegistrationAndDataFailureBlocks();
            }
            break;
        case IAosSubscriber::REFRESH_FAILED:  // FALL-THROUGH
        case IAosSubscriber::NOT_READY:       // FALL-THROUGH
        default:
            ProcessBlockReason(IMS_TRUE, BLOCK_SUBSCRIBER_INCOMPLETED);
            break;
    }
}

PROTECTED VIRTUAL void AosCondition::Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
{
    A_IMS_TRACE_I(APPPROFILE, "Block_Changed :: Reason(%s)(%d) - %s",
            AosBlock::BlockReasonToString(nType), nType, (nParam > 0) ? "BLOCK" : "NOT_BLOCK");

    SendConditionEvent(AosServiceAvailable::EVENT_BLOCK, nType, nParam);
}

PROTECTED VIRTUAL void AosCondition::ServiceAvailable_Changed()
{
    m_bCellServiceAvailable = m_pAvailableCellular->IsAvailable();
    m_bWifiServiceAvailable = m_pAvailableWifi->IsAvailable();

    A_IMS_TRACE_I(APPPROFILE, "ServiceAvailable_Changed :: cellular(%s) , wifi(%s)",
            _TRACE_B_(m_bCellServiceAvailable), _TRACE_B_(m_bWifiServiceAvailable), 0);

    if (m_bCellServiceAvailable == IMS_FALSE)
    {
        if (m_pAvailableWifi->StopToCheckNetworkConnection())
        {
            // MSG_AVAILABLE_CHECK is posted again
            return;
        }
    }

    if (m_piListener != IMS_NULL)
    {
        m_piListener->Condition_Changed();
    }
}

PROTECTED VIRTUAL void AosCondition::ServiceAvailable_RequestCommand(
        IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason)
{
    RequestCommand(nCommand, nReason);
}

PROTECTED VIRTUAL void AosCondition::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_D(APPPROFILE, "NConfiguration_NotifyConfigChanged :: changed", 0, 0, 0);

    const IAosNConfiguration* piNConfig = GET_N_CONFIG(m_nSlotId);

    if (piNConfig == IMS_NULL)
    {
        return;
    }

    m_eServiceType = GetServiceType();

    if (!piNConfig->IsVoLteRoamingAvailable())
    {
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
    }
}

// AosServicePhoneListener
PROTECTED VIRTUAL void AosCondition::ServicePhone_AosStart()
{
    A_IMS_TRACE_D(APPPROFILE, "ServicePhone_AosStart()", 0, 0, 0);
    ProcessAosStartEvent();
}

PROTECTED VIRTUAL void AosCondition::ServicePhone_LocationInfoChanged(IN LocationInfo eState)
{
    if (!GET_N_CONFIG(m_nSlotId)->UseWfcCountryCodeAvailabilityCheck())
    {
        return;
    }

    A_IMS_TRACE_D(APPPROFILE, "ServicePhone_LocationInfoChanged() :: State(%d)", eState, 0, 0);
    ProcessLocationInfo(eState);
}

PROTECTED VIRTUAL void AosCondition::ServicePhone_PhoneNumberStateChanged(
        IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState)
{
    A_IMS_TRACE_D(APPPROFILE, "ServicePhone_PhoneNumberStateChanged() :: IsRefresh(%s), State(%d)",
            _TRACE_B_(bIsRefresh), eState, 0);
    ProcessPhoneNumberAvailableEvent(bIsRefresh, eState);
}

PROTECTED VIRTUAL void AosCondition::ServicePhone_PlmnChanged()
{
    A_IMS_TRACE_D(APPPROFILE, "ServicePhone_PlmnChanged()", 0, 0, 0);
    ProcessPlmnEvent();
}

PROTECTED VIRTUAL void AosCondition::ServicePhone_PowerOff()
{
    A_IMS_TRACE_D(APPPROFILE, "ServicePhone_PowerOff()", 0, 0, 0);
    ProcessPowerEvent();
}

// AosServiceSettingListener
PROTECTED VIRTUAL void AosCondition::ServiceSetting_AirplaneChanged(IN IMS_BOOL bIsOn)
{
    A_IMS_TRACE_D(APPPROFILE, "ServiceSetting_AirplaneChanged() :: %s", _TRACE_B_(bIsOn), 0, 0);
    ProcessAirPlaneEvent(bIsOn);
}

PROTECTED VIRTUAL void AosCondition::ServiceSetting_ServiceChanged(
        IN ServiceSetting eState, IN IMS_UINT32 nServiceBits)
{
    A_IMS_TRACE_D(APPPROFILE, "ServiceSetting_ServiceChanged() :: eState(%d), nServiceBits(%d)",
            eState, nServiceBits, 0);
    ProcessImsServiceEvent(eState, nServiceBits);
}

PROTECTED VIRTUAL void AosCondition::ServiceSetting_TtyChanged(IN IMS_BOOL bIsOn)
{
    if (!GET_N_CONFIG(m_nSlotId)->IsVolteTtySupported())
    {
        return;
    }

    ProcessTtyEvent(bIsOn);
}

PROTECTED
void AosCondition::Init()
{
    IAosConnection* piAosConnection = m_piAppContext->GetConnection();
    IMS_SINT32 nCnxType = (piAosConnection != IMS_NULL) ? piAosConnection->GetConnectionType()
                                                        : NetworkPolicy::APN_NONE;

    if ((nCnxType == NetworkPolicy::APN_WIFI) || (nCnxType == NetworkPolicy::APN_EMERGENCY))
    {
        RemoveListener(LISTENER_NETTRACKER);
    }
}

PROTECTED
void AosCondition::AddListener(IN IMS_UINT32 nType)
{
    m_nListeners |= nType;
}

PROTECTED
void AosCondition::RemoveListener(IN IMS_UINT32 nType)
{
    m_nListeners &= ~(nType);
}

PROTECTED
IMS_BOOL AosCondition::IsListenerEnabled(IN IMS_UINT32 nType) const
{
    return (m_nListeners & nType);
}

/*

Remarks
    AddHold means that the condition or event will be ignored from now on.
    So firstly, reset a block with the event held.
    And secondly, a related event will be ignored when it is received in Event_NotifyEvent().
    Refer to RemoveHold() too.
*/
PROTECTED
void AosCondition::AddHold(IN IMS_UINT32 nEvent, IN IMS_BOOL bIsEventReset /* = IMS_FALSE */)
{
    m_nHoldEvents |= nEvent;

    if (!bIsEventReset)
        return;

    switch (nEvent)
    {
        case HOLD_EVENT_ROAMING:
            ProcessBlockReason(IMS_FALSE, BLOCK_CELLULAR_ROAMING);
            break;

        case HOLD_EVENT_IMS_SERVICE:
            ProcessBlockReason(IMS_FALSE, BLOCK_IMS_DISABLED);
            break;

        default:
            break;
    }
}

/*

Remarks
    If RemoveHold() is invoked with an event (or condition).
    AosCondition will take care of the event or condition from now on.

*/
PROTECTED
void AosCondition::RemoveHold(IN IMS_UINT32 nEvent, IN IMS_BOOL bIsEventReset /* = IMS_FALSE */)
{
    m_nHoldEvents &= ~(nEvent);

    if (!bIsEventReset)
        return;

    switch (nEvent)
    {
        case HOLD_EVENT_ROAMING:
            ProcessBlockReason(IMS_FALSE, BLOCK_CELLULAR_ROAMING);
            break;

        case HOLD_EVENT_IMS_SERVICE:
            ProcessBlockReason(IMS_FALSE, BLOCK_IMS_DISABLED);
            break;

        default:
            break;
    }
}

PROTECTED
IMS_BOOL AosCondition::IsHeld(IN IMS_UINT32 nEvent) const
{
    return (m_nHoldEvents & nEvent);
}

PROTECTED
IMS_BOOL AosCondition::IsRefreshStarted() const
{
    return m_bIsRefreshStarted;
}

PROTECTED
void AosCondition::SetInitialBlockReason()
{
    A_IMS_TRACE_D(APPPROFILE, "SetInitialBlockReason()", 0, 0, 0);

    if (IsListenerEnabled(LISTENER_NETTRACKER))
    {
        SendConditionEvent(AosServiceAvailable::EVENT_NETWORK, 0, SERVICE_CELLULAR);
    }

    SendConditionEvent(AosServiceAvailable::EVENT_WIFI_STATE, 0, SERVICE_WIFI);

    if (IsListenerEnabled(LISTENER_SUBSCRIBER))
    {
        ProcessBlockReason(IMS_TRUE, BLOCK_SUBSCRIBER_INCOMPLETED, IMS_FALSE);
    }

    if (IsServiceBlockedByMenu())
    {
        ProcessBlockReason(IMS_TRUE, BLOCK_IMS_DISABLED);
    }

    SetStartBlockReason();
}

PROTECTED
void AosCondition::SetStartBlockReason()
{
    m_piBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
}

PROTECTED
void AosCondition::ProcessBlockReason(
        IN IMS_BOOL bIsBlockSet, IN BLOCK_REASON eReason, IN IMS_BOOL bNotify /* = IMS_TRUE */)
{
    if (bIsBlockSet)
    {
        m_piBlock->SetBlockReason(eReason, bNotify);
    }
    else
    {
        m_piBlock->ResetBlockReason(eReason, bNotify);
    }
}

PROTECTED
void AosCondition::ProcessAosStartEvent()
{
    ProcessBlockReason(IMS_FALSE, BLOCK_AOS_INCOMPLETED);
}

PROTECTED
void AosCondition::ProcessAirPlaneEvent(IN IMS_BOOL bIsOn)
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessAirPlaneEvent(), bIsOn(%s)", _TRACE_B_(bIsOn), 0, 0);
    SendConditionEvent(AosServiceAvailable::EVENT_AIRPLANE, static_cast<IMS_UINT32>(bIsOn));

    if (bIsOn)
    {
        RequestCommand(REQUEST_STOP, AosReason::AIRPLANE_MODE);

        ClearRegistrationAndDataFailureBlocks();
    }
}

PROTECTED
void AosCondition::ProcessPowerEvent()
{
    RequestCommand(REQUEST_STOP, AosReason::POWER_OFF);
    ProcessBlockReason(IMS_TRUE, BLOCK_POWER_OFF);
}

PROTECTED
void AosCondition::ProcessRoamingEvent(IN IMS_UINT32 nPsState, IN IMS_UINT32 nCsState)
{
    IMS_UINT32 nState = (nPsState == IMS_ROAMING_STATE_OFF) ? nCsState : IMS_ROAMING_STATE_ON;
    if (IsHeld(HOLD_EVENT_ROAMING))
    {
        return;
    }

    SendConditionEvent(AosServiceAvailable::EVENT_ROAMING, nState, SERVICE_CELLULAR);

    ClearRegistrationAndDataFailureBlocks();
    ProcessBlockReason(IMS_FALSE, BLOCK_IMS_DISABLED);
}

PROTECTED
void AosCondition::ProcessPlmnEvent()
{
    ProcessBlockReason(IMS_FALSE, BLOCK_PERMANENT_DATA_FAILED);
}

PROTECTED
void AosCondition::ProcessPhoneNumberAvailableEvent(
        IN IMS_BOOL /*bIsRefresh*/, IN PhoneNumberState eState)
{
    if (eState == PhoneNumberState::RETRY_FAILURE)
    {
        return;
    }

    ProcessBlockReason(IMS_FALSE, BLOCK_PERMANENT_DATA_FAILED);
    ProcessBlockReason(IMS_FALSE, BLOCK_INVALID_CONNECTION);

    RequestCommand(REQUEST_RESET_CONNECTION_RECOVERY);
}

PROTECTED
void AosCondition::ProcessImsServiceEvent(IN ServiceSetting eState, IN IMS_UINT32 /*nServiceBits*/)
{
    if (IsHeld(HOLD_EVENT_IMS_SERVICE))
    {
        return;
    }

    if (eState == ServiceSetting::ON)
    {
        ProcessBlockReason(IMS_FALSE, BLOCK_IMS_DISABLED);
        ClearRegistrationAndDataFailureBlocks();
    }
    else if (eState == ServiceSetting::OFF)
    {
        RequestCommand(REQUEST_PDN_DISCONNECT, AosReason::IMS_DISABLED);
        ProcessBlockReason(IMS_TRUE, BLOCK_IMS_DISABLED);
    }
}

PROTECTED
void AosCondition::ProcessTtyEvent(IN IMS_BOOL bIsOn)
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessTtyEvent(), bIsOn(%s)", _TRACE_B_(bIsOn), 0, 0);
    m_bIsTtyOn = bIsOn;

    if (!GET_N_CONFIG(m_nSlotId)->IsRttSupported() || m_bIsCombinedAttached)
    {
        if (m_bIsTtyOn)
        {
            RequestCommand(REQUEST_STOP, AosReason::TTYMODEON);
            ProcessBlockReason(IMS_TRUE, BLOCK_TTY_MODE_ON);
        }
    }

    if (!m_bIsTtyOn)
    {
        ProcessBlockReason(IMS_FALSE, BLOCK_TTY_MODE_ON);
    }
}

PROTECTED
void AosCondition::ProcessImsVopsEvent(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessImsVopsEvent(), nState(%d)", nState, 0, 0);

    SendConditionEvent(AosServiceAvailable::EVENT_VOPS, nState, SERVICE_CELLULAR);
}

PROTECTED
void AosCondition::ProcessLocationInfo(IN LocationInfo eState)
{
    if (eState != LocationInfo::CHANGED)
    {
        return;
    }

    SendConditionEvent(
            AosServiceAvailable::EVENT_LOCATION, static_cast<IMS_UINT32>(eState), SERVICE_WIFI);
}

PROTECTED
void AosCondition::ProcessLteInfoEvent(IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx)
{
    A_IMS_TRACE_I(
            APPPROFILE, "ProcessLteInfoEvent(), nState(%d), nStateEx(%d)", nState, nStateEx, 0);

    m_bIsCombinedAttached =
            (nState == IMS_LTE_INFO_COMBINED_ATTACHED && nStateEx == IMS_LTE_INFO_EXTRA_NONE);
}

PROTECTED
void AosCondition::ClearRegistrationAndDataFailureBlocks()
{
    A_IMS_TRACE_D(APPPROFILE, "ClearRegistrationAndDataFailureBlocks", 0, 0, 0);
    ProcessBlockReason(IMS_FALSE, BLOCK_AUTHENTICATION_FAILED);
    ProcessBlockReason(IMS_FALSE, BLOCK_PERMANENT_REG_FAILED);
    ProcessBlockReason(IMS_FALSE, BLOCK_PERMANENT_DATA_FAILED);
    ProcessBlockReason(IMS_FALSE, BLOCK_INVALID_CONNECTION);

    RequestCommand(REQUEST_RESET_CONNECTION_RECOVERY);
}

PROTECTED
SERVICE_TYPE AosCondition::GetServiceType()
{
    SERVICE_TYPE eType = SERVICE_CELLULAR;

    if (GET_N_CONFIG(m_nSlotId) == IMS_NULL)
    {
        return eType;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsVoLteAvailable() && GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        eType = SERVICE_WHOLE;
    }
    else if (!GET_N_CONFIG(m_nSlotId)->IsVoLteAvailable() &&
            GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        eType = SERVICE_WIFI;
    }

    return eType;
}

PROTECTED
void AosCondition::SendConditionEvent(IN IMS_UINT32 eEvent, IN IMS_UINT32 nState,
        IN IMS_SINT32 nStateEx /* = -1*/, IN SERVICE_TYPE eServiceType /*= SERVICE_WHOLE*/)
{
    A_IMS_TRACE_D(APPPROFILE, "SendConditionEvent :: eEvent(%d), nState(%d), nStateEx(%d)", eEvent,
            nState, nStateEx);

    if (eServiceType == SERVICE_CELLULAR || eServiceType == SERVICE_WHOLE)
    {
        if (m_pAvailableCellular != IMS_NULL)
        {
            m_pAvailableCellular->HandleEvent(eEvent, nState, nStateEx);
        }
    }

    if (eServiceType == SERVICE_WIFI || eServiceType == SERVICE_WHOLE)
    {
        if (m_pAvailableWifi != IMS_NULL)
        {
            m_pAvailableWifi->HandleEvent(eEvent, nState, nStateEx);
        }
    }
}

PROTECTED
IMS_BOOL AosCondition::RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason /* = 0*/) const
{
    if (m_piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_D(APPPROFILE, "RequestCommand :: Command(%d), Reason(%d)", nCommand, nReason, 0);

    m_piListener->Condition_RequestCommand(nCommand, nReason);
    return IMS_TRUE;
}

PROTECTED
void AosCondition::UpdateRegistrationMode() const
{
    const IAosSubscriber* piSubscriber = m_piAppContext->GetSubscriber();

    if (m_piBlock->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED) && piSubscriber->IsReady())
    {
        IMS_SINT32 nImpuCount = piSubscriber->GetConfiguredImpus().GetCount();
        if (nImpuCount > 1)
        {
            A_IMS_TRACE_D(APPPROFILE, "UpdateRegistrationMode :: NORMAL", 0, 0, 0);
            m_piAppContext->GetRegistration()->RequestCmd(
                    IAosRegistration::CMD_SET_MODE, IAosRegistration::MODE_NORMAL);
        }
        else if (nImpuCount == 1)
        {
            A_IMS_TRACE_D(APPPROFILE, "UpdateRegistrationMode :: LIMITED", 0, 0, 0);
            m_piAppContext->GetRegistration()->RequestCmd(
                    IAosRegistration::CMD_SET_MODE, IAosRegistration::MODE_LIMITED);
        }
    }
}

PROTECTED
IMS_BOOL AosCondition::IsServiceBlockedByMenu() const
{
    IImsPrivateProperty* piProperty = UtilService::GetUtilService()->GetPrivateProperty();
    AString strTestImsDeregister = piProperty->GetPersistent(
            ImsPrivateProperties::Persistent::KEY_TEST_IMS_DEREGISTER, m_nSlotId);

    A_IMS_TRACE_D(APPPROFILE, "IsServiceBlockedByMenu : %s", strTestImsDeregister.GetStr(), 0, 0);
    return strTestImsDeregister.Equals("YES");
}