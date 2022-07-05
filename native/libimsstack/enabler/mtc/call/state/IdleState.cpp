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

#include "ICoreService.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IuMtcService.h"
#include "JniMtcServiceThread.h"
#include "MediaDef.h"
#include "MtcDef.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/block/CallCountBlockRule.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/CsCallBlockRule.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/NetworkBlockRule.h"
#include "call/block/ProcessingCallBlockRule.h"
#include "call/block/TerminalBasedCallWaitingBlockRule.h"
#include "call/block/VopsBlockRule.h"
#include "call/message/MessageSender.h"
#include "call/state/IdleState.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "dialogevent/IDialogEvent.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ussi/UssiController.h"
#include "ussi/UssiController.h"
#include "utility/MessageUtil.h"
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
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_D("Start", 0, 0, 0);

    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);

    if (m_objContext.IsUssi())
    {
        m_objOperationAfterBlockCheck = [&]()
        {
            return ContinueStartUssi(pMediaInfo);
        };
    }
    else
    {
        m_objOperationAfterBlockCheck = [&]()
        {
            return ContinueStart(pMediaInfo);
        };
    }

    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(IN CallType eCallType,
        IN const AString& strTarget, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices, IN IMSList<ConfUser*> lstUsers)
{
    IMS_TRACE_D("StartConference", 0, 0, 0);

    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetCallInfo().bConference = IMS_TRUE;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueConference(pMediaInfo, lstUsers);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(
        IN CallType eCallType, IN const AString& strTarget, IN IMSList<ConfUser*> lstUsers)
{
    IMS_TRACE_D("StartConference", 0, 0, 0);

    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetCallInfo().bConference = IMS_TRUE;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueConference(
                new MediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_INVALID, DIRECTION_INVALID,
                        AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID),
                lstUsers);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::HandleIncoming(
        IN ISession* piSession, IN JniMtcServiceThread* pServiceThread)
{
    IMS_TRACE_D("HandleIncoming", 0, 0, 0);
    m_objContext.GetUiNotifier().SetJniServiceThread(pServiceThread);
    m_objContext.GetCallInfo().eInitialCallType = CallType::UNKNOWN;
    m_objContext.GetCallInfo().ePeerType = PeerType::MT;

    MtcSession* pSession = m_objContext.CreateSession(piSession);
    if (pSession == IMS_NULL)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE));
    }

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
    pSession->HandleRequest(IMessage::SESSION_START, *piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(IMessage::SESSION_START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);

    if (!pSession->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS));
    }

    if (m_objContext.GetConfigurationProxy().Is(Feature::REJECT_OFFERLESS_INVITE) &&
            !MessageUtil::HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
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
    IMS_TRACE_I("Terminate : reason[%s]", PS_FR(objReason), 0, 0);
    m_objContext.GetMediaManager().Terminate();
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
            m_objContext.GetMediaManager().Terminate();
            if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
            {
                m_objContext.GetSession()->GetMessageSender().Reject(objResult.objReason);
            }
            m_objContext.GetUiNotifier().SendStartFailed(objResult.objReason);
            return CallStateName::TERMINATING;

        case IMtcBlockChecker::Result::Status::PENDING:
            return GetStateName();
    }
}

PUBLIC VIRTUAL CallStateName IdleState::OnAttached()
{
    ISession* piSession = GetISession();
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    InitMediaSession();

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    objPreconditionManager.CreateQos(piSession);
    UpdatePreconditionCapability(piSession, piMessage, IMS_FALSE);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    // TODO: OnPreconditionReceived()
    // need to check the nego state?
    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    if (IsRprSupported() &&
            !m_objContext.GetConfigurationProxy().Is(Feature::SEND_180_FOR_INITIAL_INVITE))
    {
        if (SendProvisionalResponse(IMS_FALSE) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_SESSION_INTERNAL_ERROR));
        }
    }
    else
    {
        SendIncomingCallReceived();
        return CallStateName::ALERTING;
    }

    RunMedia(piSession, piMessage);
    return CallStateName::INCOMING;
}

PUBLIC VIRTUAL CallStateName IdleState::HandleIncomingUssi(
        IN ISession* piSession, IN JniMtcServiceThread* pServiceThread)
{
    IMS_TRACE_D("HandleIncomingUssi", 0, 0, 0);

    m_objContext.GetCallInfo().ePeerType = PeerType::MT;
    m_objContext.GetCallInfo().bUssi = IMS_TRUE;

    m_objContext.GetUiNotifier().SetJniServiceThread(pServiceThread);

    m_objContext.CreateSession(piSession);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    if (m_objContext.GetConfigurationProxy().Is(Feature::REJECT_OFFERLESS_INVITE) &&
            !MessageUtil::HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    if (!m_objContext.GetUssiController()->HasValidXmlBodyForNetworkInitiatedUssi(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE));
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
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(IMessage::SESSION_START, *piMessage);

    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS));
    }

    m_objContext.GetCallInfo().eInitialCallType = m_objContext.GetSession()->GetCallType();
    InitMediaSession();

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PRIVATE
CallStateName IdleState::ContinueStart(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("ContinueStart", 0, 0, 0);
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    InitMediaSession(pMediaInfo);

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    StartTimer(MtcCallState::TimerType::TIMER_MO_1XX_WAIT);

    return CallStateName::OUTGOING;
}

PRIVATE
CallStateName IdleState::ContinueConference(
        IN MediaInfo* pMediaInfo, IN IMSList<ConfUser*> lstUsers)
{
    IMS_TRACE_D("ContinueConference", 0, 0, 0);
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    IMSList<AString> lstUris = GetEntryUrisFromConferenceUsers(lstUsers);
    SetResourceListForConference(*GetISession()->GetNextRequest(), lstUris);

    InitMediaSession(pMediaInfo);

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    StartTimer(MtcCallState::TimerType::TIMER_MO_1XX_WAIT);

    return CallStateName::OUTGOING;
}

PRIVATE
CallStateName IdleState::ContinueHandleIncoming()
{
    IMS_TRACE_D("ContinueHandleIncoming", 0, 0, 0);

    SendPreIncomingCallReceived();

    return GetStateName();
}

PRIVATE
CallStateName IdleState::ContinueStartUssi(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("ContinueStartUssi", 0, 0, 0);
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        objMediaManager.Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    InitMediaSession(pMediaInfo);

    if (m_objContext.GetUssiController()->FormStartUssiRequest(
            m_objContext.GetParticipantInfo().GetRemoteNumber()) == IMS_FAILURE)
    {
        objMediaManager.Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    IMS_UINT32 eMediaTypes =
            MtcMediaUtil::GetMediaTypesFromCallType(m_objContext.GetCallInfo().eInitialCallType);
    objMediaManager.SetRtpPort(GetISession(), eMediaTypes, 0);

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        objMediaManager.Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    StartTimer(MtcCallState::TimerType::TIMER_MO_1XX_WAIT);
    return CallStateName::OUTGOING;
}

PRIVATE
IMSList<AString> IdleState::GetEntryUrisFromConferenceUsers(IN const IMSList<ConfUser*>& lstUsers)
{
    // TODO: Pass param as entry URIs for MtcCall I/F.
    // So this method will be moved to outside of MtcCall
    IMSList<AString> lstEntryUris;
    for (IMS_SIZE_T index = 0; index < lstUsers.GetSize(); index++)
    {
        // TODO: Implement GetEntryUri (operatior specific?)
        // lstEntryUris.Append(GetEntryUri(lstUsers.GetAt(index)));
    }
    return lstEntryUris;
}

PRIVATE
void IdleState::SetResourceListForConference(
        IN_OUT IMessage& objMessage, IN IMSList<AString>& lstEntryUris)
{
    if (lstEntryUris.GetSize() == 0)
    {
        return;
    }
    objMessage.AddHeader(SipHeaderName::CONTENT_TYPE, "multipart/mixed");
    // messageSender->SetResourceListsBody(pIMessage, AString::ConstNull(), lstEntryUris, IMS_TRUE);
}

PRIVATE
IMSList<IMtcBlockRule*> IdleState::GetIncomingCallBlockRules()
{
    IMSList<IMtcBlockRule*> lstRules;

    lstRules.Append(new VopsBlockRule(m_objContext));
    lstRules.Append(new NetworkBlockRule(m_objContext));
    lstRules.Append(new ProcessingCallBlockRule(m_objContext));
    lstRules.Append(new CsCallBlockRule(m_objContext));
    lstRules.Append(new CallCountBlockRule(m_objContext));
    lstRules.Append(new CallTypeBlockRule(m_objContext, m_objContext.GetSession()->GetCallType()));
    lstRules.Append(new TerminalBasedCallWaitingBlockRule(m_objContext));

    return lstRules;
}

PRIVATE
IMSList<IMtcBlockRule*> IdleState::GetOutgoingCallBlockRules()
{
    IMSList<IMtcBlockRule*> lstRules;

    // IMS call won't be initiated if
    // - VoPS is 0 or can be ignored
    // - The current network is N/A.

    // TODO: SSAC
    lstRules.Append(new ProcessingCallBlockRule(m_objContext));
    lstRules.Append(new CsCallBlockRule(m_objContext));
    lstRules.Append(new CallCountBlockRule(m_objContext));

    return lstRules;
}
