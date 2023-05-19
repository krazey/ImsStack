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

#include "AString.h"
#include "ICoreService.h"
#include "IImsAosInfo.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "MtcDef.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/block/CallCountBlockRule.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/CallWaitingBlockRule.h"
#include "call/block/CsCallBlockRule.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/NetworkBlockRule.h"
#include "call/block/ProcessingCallBlockRule.h"
#include "call/block/RadioBlockRule.h"
#include "call/block/SsacBlockRule.h"
#include "call/block/TimerBlockRule.h"
#include "call/block/VopsBlockRule.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/IdleState.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/IMessage.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "dialogevent/IMultiEndpointManager.h"
#include "helper/IMtcAosConnector.h"
#include "helper/LastComeFirstServedHelper.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ussi/UssiController.h"
#include "utility/IMessageUtils.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
IdleState::IdleState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::IDLE, objContext),
        m_pBlockChecker(nullptr),
        m_objOperationAfterBlockCheck(nullptr)
{
}

PUBLIC VIRTUAL IdleState::~IdleState() {}

PUBLIC VIRTUAL CallStateName IdleState::Start(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_D("Start [%s]", strTarget.GetStr(), 0, 0);
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);

    if (IsCallPull())
    {
        if (HandleCallPull() == IMS_FAILURE)
        {
            return CallStateName::TERMINATING;
        }
    }
    else
    {
        m_objContext.GetCallInfo().eInitialCallType = eCallType;
        m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);
    }
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;

    if (m_objContext.IsUssi())
    {
        m_objOperationAfterBlockCheck = [&]()
        {
            return ContinueStartUssi();
        };
    }
    else
    {
        m_objOperationAfterBlockCheck = [&]()
        {
            return ContinueStart();
        };
    }

    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(IN CallType eCallType,
        IN const AString& strTarget, IN MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices,
        IN const ImsList<ConfUser*>& lstUsers)
{
    IMS_TRACE_D("StartConference", 0, 0, 0);
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);
    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetCallInfo().bConference = IMS_TRUE;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueConference(lstUsers);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(
        IN CallType eCallType, IN const AString& strTarget, IN const ImsList<ConfUser*>& lstUsers)
{
    IMS_TRACE_D("StartConference", 0, 0, 0);

    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetCallInfo().bConference = IMS_TRUE;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    IMS_SINT32 nVideoDirection = DIRECTION_INVALID;
    if (eCallType == CallType::VT)
    {
        nVideoDirection = DIRECTION_SEND_RECEIVE;
    }
    m_objContext.GetMediaManager().SetMediaInfo(MediaInfo(DIRECTION_SEND_RECEIVE, nVideoDirection,
            DIRECTION_INVALID, AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID));

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueConference(lstUsers);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::HandleIncoming(IN ISession* piSession)
{
    IMS_TRACE_D("HandleIncoming", 0, 0, 0);
    m_objContext.GetCallInfo().eInitialCallType = CallType::UNKNOWN;
    m_objContext.GetCallInfo().ePeerType = PeerType::MT;

    IMtcSession* pSession = m_objContext.CreateSession(piSession);
    if (pSession == IMS_NULL)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
    pSession->HandleRequest(RequestType::START, *piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(RequestType::START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);

    AString strNotSupportedExtension;
    if (!pSession->GetExtensionSet().IsSupportRequiredExtensions(
                *piMessage, strNotSupportedExtension))
    {
        return RejectIncomingAndToTerminating(
                CallReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strNotSupportedExtension));
    }

    if (m_objContext.GetConfigurationProxy().Is(Feature::REJECT_OFFERLESS_INVITE) &&
            !m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    if (LastComeFirstServedHelper::IsSupported(m_objContext.GetConfigurationProxy()))
    {
        m_objContext.GetLastComeFirstServedHelper().OnCallReceived(m_objContext.GetCallKey());
    }

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueHandleIncoming();
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetIncomingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", _TRACE_CR_(objReason), 0, 0);
    m_objContext.GetUiNotifier().SendStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IdleState::OnBlockChecked(IN IMtcBlockChecker::Result objResult)
{
    switch (objResult.eStatus)
    {
        case IMtcBlockChecker::Result::Status::UNBLOCKED:
            m_pBlockChecker.reset();
            return m_objOperationAfterBlockCheck();

        case IMtcBlockChecker::Result::Status::BLOCKED:
            m_pBlockChecker.reset();

            if (IsEpsFallbackRequired(objResult.objReason))
            {
                m_objContext.GetEpsFallbackTrigger().TriggerEpsFallback(
                        EpsFallbackReason::NO_NETWORK_TRIGGER, IMS_TRUE);
                return GetStateName();
            }

            if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
            {
                m_objContext.GetSession()->Reject(objResult.objReason);
            }
            m_objContext.GetUiNotifier().SendStartFailed(objResult.objReason.ConvertFromInternal());
            return CallStateName::TERMINATING;

        default:  // IMtcBlockChecker::Result::Status::PENDING:
            return GetStateName();
    }
}

PUBLIC VIRTUAL CallStateName IdleState::OnAttached()
{
    ISession* piSession = GetISession();
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    InitMediaSession();
    m_objContext.GetPreconditionManager().CreateQos(piSession);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (IsRprSupported())
    {
        if (m_objContext.GetSession()->SendProvisionalResponse(IMS_FALSE) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        }
    }
    else
    {
        m_objContext.GetUiNotifier().SendIncomingCallReceived();
        return CallStateName::ALERTING;
    }

    StartEpsFallbackWatchdogIfNeeded(*piSession->GetPreviousResponse(IMessage::SESSION_START));
    return CallStateName::INCOMING;
}

PROTECTED VIRTUAL CallStateName IdleState::HandleAosConnected()
{
    IMS_TRACE_I("HandleAosConnected", 0, 0, 0);

    if (m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoTrigger())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();

        m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
                m_objContext.CreateBlockChecker(GetBlockRulesAfterEpsFallback()));
        return OnBlockChecked(m_pBlockChecker->Check());
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IdleState::HandleIncomingUssi(IN ISession* piSession)
{
    IMS_TRACE_D("HandleIncomingUssi", 0, 0, 0);

    m_objContext.GetCallInfo().ePeerType = PeerType::MT;
    m_objContext.GetCallInfo().bUssi = IMS_TRUE;
    m_objContext.CreateSession(piSession);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    if (m_objContext.GetConfigurationProxy().Is(Feature::REJECT_OFFERLESS_INVITE) &&
            !m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    if (!m_objContext.GetUssiController()->HasValidXmlBodyForNetworkInitiatedUssi(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_UNKNOWN));
    }

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueHandleIncoming();
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetIncomingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::OnUssiAttached()
{
    IMS_TRACE_D("OnUssiAttached", 0, 0, 0);

    ISession* piSession = GetISession();

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    // TODO: need to check
    m_objContext.GetSession()->HandleRequest(RequestType::START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(RequestType::START, *piMessage);

    AString strNotSupportedExtension;
    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(
                *piMessage, strNotSupportedExtension))
    {
        return RejectIncomingAndToTerminating(
                CallReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strNotSupportedExtension));
    }

    m_objContext.GetCallInfo().eInitialCallType = m_objContext.GetSession()->GetCallType();
    InitMediaSession();

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    m_objContext.GetUiNotifier().SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PRIVATE
CallStateName IdleState::ContinueStart()
{
    IMS_TRACE_D("ContinueStart", 0, 0, 0);
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    InitMediaSession();

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    StartTimer(MtcCallState::TimerType::TIMER_MO_100_WAIT);
    if (!m_objContext.GetTimer().IsActive(TIMER_MO_18X_WAIT))
    {
        // The 18x wait timer may already be activated in some redial cases.
        // In this case, don't restart the 18x wait timer.
        StartTimer(MtcCallState::TimerType::TIMER_MO_18X_WAIT);
    }

    return CallStateName::OUTGOING;
}

PRIVATE
CallStateName IdleState::ContinueConference(IN const ImsList<ConfUser*>& lstUsers)
{
    IMS_TRACE_D("ContinueConference UserSize[%d]", lstUsers.GetSize(), 0, 0);
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    SetResourceListForConference(*GetISession()->GetNextRequest(), lstUsers);

    InitMediaSession();

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    StartTimer(MtcCallState::TimerType::TIMER_MO_100_WAIT);
    StartTimer(MtcCallState::TimerType::TIMER_MO_18X_WAIT);

    return CallStateName::OUTGOING;
}

PRIVATE
CallStateName IdleState::ContinueHandleIncoming()
{
    IMS_TRACE_D("ContinueHandleIncoming", 0, 0, 0);

    m_objContext.GetUiNotifier().SendPreIncomingCallReceived();

    return GetStateName();
}

PRIVATE
CallStateName IdleState::ContinueStartUssi()
{
    IMS_TRACE_D("ContinueStartUssi", 0, 0, 0);
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    InitMediaSession();

    if (m_objContext.GetUssiController()->FormStartUssiRequest(
                m_objContext.GetParticipantInfo().GetRemoteNumber()) == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    IMS_UINT32 eMediaTypes =
            MtcMediaUtil::GetMediaTypesFromCallType(m_objContext.GetCallInfo().eInitialCallType);
    objMediaManager.SetRtpPort(GetISession(), eMediaTypes, 0);

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    StartTimer(MtcCallState::TimerType::TIMER_MO_100_WAIT);
    StartTimer(MtcCallState::TimerType::TIMER_MO_18X_WAIT);
    return CallStateName::OUTGOING;
}

IMS_BOOL IdleState::IsEpsFallbackRequired(IN const CallReasonInfo& objReason) const
{
    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT ||
            !EpsFallbackTrigger::IsRequired(m_objContext.GetConfigurationProxy()) ||
            !m_objContext.GetService().IsNr())

    {
        return IMS_FALSE;
    }

    if (objReason.nCode == CODE_ACCESS_CLASS_BLOCKED)
    {
        return IMS_TRUE;
    }

    const IMS_UINT32 nWaitTimeMillis = objReason.nExtraCode;
    const IMS_UINT32 nTimerVzw =
            m_objContext.GetConfigurationProxy().GetInt(Feature::MO_CALL_REQUEST_TIMEOUT);
    if (objReason.nCode == CODE_INTERNAL_RRC_REJECT && nWaitTimeMillis >= nTimerVzw)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void IdleState::SetResourceListForConference(
        IN_OUT IMessage& objMessage, IN const ImsList<ConfUser*>& lstUsers)
{
    if (lstUsers.GetSize() == 0)
    {
        return;
    }
    objMessage.AddHeader(SipHeaderName::CONTENT_TYPE, "multipart/mixed");

    // TODO: LGU needs to set false the 5th param.
    m_objContext.GetMessageUtils().SetResourceList(
            &objMessage, m_objContext, AString::ConstNull(), lstUsers, IMS_TRUE, IMS_TRUE);
}

PRIVATE
ImsList<IMtcBlockRule*> IdleState::GetIncomingCallBlockRules()
{
    ImsList<IMtcBlockRule*> lstRules;

    lstRules.Append(new VopsBlockRule(m_objContext));
    lstRules.Append(new NetworkBlockRule(m_objContext,
            *PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_objContext.GetSlotId())));
    lstRules.Append(new ProcessingCallBlockRule(m_objContext));
    lstRules.Append(new CsCallBlockRule(m_objContext));
    lstRules.Append(new CallCountBlockRule(m_objContext));
    lstRules.Append(new CallTypeBlockRule(m_objContext));
    lstRules.Append(new CallWaitingBlockRule(m_objContext));
    lstRules.Append(new SsacBlockRule(m_objContext, m_objContext.GetSession()->GetCallType()));
    lstRules.Append(new RadioBlockRule(m_objContext, m_objContext.GetSession()->GetCallType()));
    return lstRules;
}

PRIVATE
ImsList<IMtcBlockRule*> IdleState::GetOutgoingCallBlockRules()
{
    ImsList<IMtcBlockRule*> lstRules;

    lstRules.Append(new ProcessingCallBlockRule(m_objContext));
    lstRules.Append(new CsCallBlockRule(m_objContext));
    lstRules.Append(new CallCountBlockRule(m_objContext));
    lstRules.Append(new SsacBlockRule(m_objContext, m_objContext.GetCallInfo().eInitialCallType));
    lstRules.Append(new RadioBlockRule(m_objContext, m_objContext.GetCallInfo().eInitialCallType));
    lstRules.Append(new TimerBlockRule(
            m_objContext.GetPassiveTimerHolder(), m_objContext.GetCallInfo().bEmergency));

    return lstRules;
}

PRIVATE
ImsList<IMtcBlockRule*> IdleState::GetBlockRulesAfterEpsFallback()
{
    ImsList<IMtcBlockRule*> lstRules;

    lstRules.Append(new SsacBlockRule(m_objContext, m_objContext.GetCallInfo().eInitialCallType));

    return lstRules;
}

PRIVATE
IMS_BOOL IdleState::IsCallPull() const
{
    const SuppService* pSs = m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL);
    return pSs && pSs->nValue > 0;
}

PRIVATE
IMS_RESULT IdleState::HandleCallPull()
{
    IMultiEndpointManager* piMultiEndpointManager = m_objContext.GetMultiEndpointManager();
    if (!piMultiEndpointManager)
    {
        Terminate(CallReasonInfo(CODE_MULTIENDPOINT_NOT_SUPPORTED));
        return IMS_FAILURE;
    }

    IMultiEndpointManager::PullingDialogInfo objDialogInfo = piMultiEndpointManager->GetDialogInfo(
            m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL)->nValue);

    if (objDialogInfo.strCallId.GetLength() <= 0 || objDialogInfo.strLocalTag.GetLength() <= 0 ||
            objDialogInfo.strRemoteTag.GetLength() <= 0)
    {
        Terminate(CallReasonInfo(CODE_CALL_PULL_OUT_OF_SYNC));
        return IMS_FAILURE;
    }

    m_objContext.GetCallInfo().eInitialCallType = objDialogInfo.eCallType;
    m_objContext.GetMediaManager().SetMediaInfo(*objDialogInfo.pMediaInfo);

    return IMS_SUCCESS;
}
