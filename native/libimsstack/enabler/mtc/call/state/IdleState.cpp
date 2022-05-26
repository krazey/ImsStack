#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "ICoreService.h"
#include "dialogevent/IDialogEvent.h"
#include "call/state/IdleState.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "JniMtcServiceThread.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "call/message/MessageSender.h"
#include "MtcDef.h"
#include "call/MtcSession.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/block/IMtcBlockChecker.h"
#include "helper/block/CallCountBlockRule.h"
#include "helper/block/CsCallBlockRule.h"
#include "helper/block/NetworkBlockRule.h"
#include "helper/block/ProcessingCallBlockRule.h"
#include "helper/block/VopsBlockRule.h"
#include "helper/block/TerminalBasedCallWaitingBlockRule.h"
#include "call/MtcUiNotifier.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "utility/MessageUtil.h"
#include "SipStatusCode.h"
#include "helper/MtcSupplementaryService.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
IdleState::IdleState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::IDLE, objContext),
        m_pBlockChecker(nullptr)
{
}

PUBLIC VIRTUAL IdleState::~IdleState() {}

PUBLIC VIRTUAL CallStateName IdleState::Start(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_D("Start", 0, 0, 0);
    m_eConferenceStartType = ConferenceType::NOT_CONFERENCE;

    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueStart(pMediaInfo);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(IN CallType eCallType,
        IN const AString& strTarget, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices, IN IMSList<ConfUser*> lstUsers)
{
    IMS_TRACE_D("StartConference", 0, 0, 0);
    m_eConferenceStartType = ConferenceType::START_CONFERENCE;  // TODO: deprecated.

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
    m_eConferenceStartType = ConferenceType::START_CONFERENCE;

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
    m_eConferenceStartType = ConferenceType::NOT_CONFERENCE;

    m_objContext.GetCallInfo().ePeerType = PeerType::MT;
    if (MessageUtil::IsFocusConf(piSession->GetPreviousRequest(IMessage::SESSION_START)))
    {
        m_objContext.GetCallInfo().bConference = IMS_TRUE;
    }

    m_objContext.GetUiNotifier().SetJniServiceThread(pServiceThread);

    m_objContext.SetSession(m_objContext.CreateSession(*piSession, CallType::UNKNOWN));

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
    if (m_objContext.GetConfigurationProxy().Is(Feature::REJECT_OFFERLESS_INVITE) &&
            !MessageUtil::HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_MEDIA_NEGOFAIL));
    }

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueHandleIncoming();
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetIncomingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", PS_FR(objReason), 0, 0);
    m_objContext.GetMediaManager().Terminate();
    m_objContext.GetUiNotifier().SendStartFailed(
            FailReason(ConvertTerminateReasonToFailReason(objReason.nReason)));

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

    InitMediaSession();

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(IMessage::SESSION_START, *piMessage);

    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_NOTSUPPORT));
    }

    m_objContext.GetCallInfo().eInitialCallType = m_objContext.GetSession()->GetCallType();

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    objPreconditionManager.CreateQos(piSession);
    UpdatePreconditionCapability(piSession, piMessage, IMS_FALSE);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_MEDIA_NEGOFAIL));
    }

    // TODO: OnPreconditionReceived()
    // need to check the nego state?
    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    if (IsRprSupported())
    {
        if (SendProvisionalResponse(IMS_FALSE) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
        }

        // m_objContext.GetTimer().Start(TIMER_MT_PRACK_WAIT,
        //         UCCONFIG_GET_INT(m_nSlotID, SESSION_TIME_MT_PRACKWAIT) * 1000);
    }
    else
    {
        IMS_TRACE_D("HandleIncoming - RPR is not supported.", 0, 0, 0);

        SendIncomingCallReceived();
        return CallStateName::ALERTING;
    }

    return CallStateName::INCOMING;
}

PRIVATE
CallStateName IdleState::ContinueStart(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("ContinueStart", 0, 0, 0);
    if (CreateISession(m_objContext.GetCallInfo().eInitialCallType) == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
        return CallStateName::TERMINATING;
    }

    InitMediaSession(pMediaInfo);

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->SendStart() == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
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
    if (CreateISession(m_objContext.GetCallInfo().eInitialCallType) == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
        return CallStateName::TERMINATING;
    }

    IMSList<AString> lstUris = GetEntryUrisFromConferenceUsers(lstUsers);
    SetResourceListForConference(*GetISession()->GetNextRequest(), lstUris);

    InitMediaSession(pMediaInfo);

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->SendStart() == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
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
