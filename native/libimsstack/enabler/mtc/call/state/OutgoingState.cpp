#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "configuration/ConfigDef.h"
#include "ICoreService.h"
#include "dialogevent/IDialogEvent.h"
#include "call/IMtcCallContext.h"
#include "media/IMtcMediaManager.h"
#include "ISession.h"
#include "ISIPHeader.h"
#include "ISIPMessage.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "call/message/MessageSender.h"
#include "utility/MessageUtil.h"
#include "MtcDef.h"
#include "call/MtcSession.h"
#include "helper/MtcTimerWrapper.h"
#include "call/MtcUiNotifier.h"
#include "call/state/OutgoingState.h"
#include "call/termination/StartErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "SIPAddress.h"
#include "SIPHeaderName.h"
#include "SIPStatusCode.h"
#include "helper/MtcSupplementaryService.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include "dialingplan/MtcDialingPlan.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
OutgoingState::OutgoingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::OUTGOING, objContext),
        m_objSessions(IMSMap<ISession*, MtcSession*>()),
        m_bRemoteAlerted(IMS_FALSE)
{
}

PUBLIC VIRTUAL
OutgoingState::~OutgoingState()
{
    DeleteInactiveSessions();
}

PUBLIC VIRTUAL
void OutgoingState::OnEnter()
{
    MtcSession* pSession = m_objContext.GetSession();
    if (pSession)
    {
        m_objSessions.Add(&pSession->GetISession(), pSession);
    }
}

PUBLIC VIRTUAL
CallStateName OutgoingState::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", PS_FR(objReason), 0, 0);

    FailReason objConvertedReason(objReason);
    objConvertedReason.nReason = ConvertTerminateReasonToFailReason(objReason.nReason);

    HandleCancel(&m_objContext.GetSession()->GetISession(), objConvertedReason);

    return TransitToTerminating(objConvertedReason);
}

PUBLIC VIRTUAL
CallStateName OutgoingState::HandleSrvccFailure(IN UpdateType eUpdateType)
{
    const IMS_UINT32 nLast = m_objSessions.GetSize() - 1;

    if (m_objContext.GetMediaManager().GetNegotiationState(m_objSessions.GetKeyAt(nLast)) !=
            NegotiationState::STATE_NEGOTIATED)
    {
        return GetStateName();
    }

    m_objSessions.GetValueAt(nLast)->GetMessageSender().SendEarlyUpdate(eUpdateType);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::OnTimerExpired(IN IMS_SINT32 /* nType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("QosReserved : MediaType[%d]", eMediaType, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.HasPreconditionCapability(piSession))
    {
        IMS_TRACE_D("QosReserved : There's no capability for precondition.", 0, 0, 0);
        return GetStateName();
    }

    if (!objPreconditionManager.IsQosEnabled(piSession, QosCheckType::LOCAL_STATUS))
    {
        IMS_TRACE_D("QosReserved : Resources of all media are not reserved.", 0, 0, 0);
        return GetStateName();
    }

    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL || piMessage->GetStatusCode() != SIPStatusCode::SC_200)
    {
        IMS_TRACE_D("QosReserved : There's no PRACK or response to PRACK.", 0, 0, 0);
        return GetStateName();
    }

    if (piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE) != IMS_NULL &&
            SdpPreconditionHelper::IsQosActivatedInSdp(piSession, IMessage::SESSION_EARLY_UPDATE))
    {
        IMS_TRACE_D("QosReserved : UE already send early UPDATE with activated QoS.", 0, 0, 0);
        return GetStateName();
    }

    // send early UPDATE
    if (m_objContext.GetMediaManager().GetNegotiationState(piSession) !=
            NegotiationState::STATE_NEGOTIATED)
    {
        return GetStateName();
    }

    if (m_objContext.GetMediaManager().FormSdp(piSession, CallType::VOIP) == IMS_FAILURE)
    {
        return GetStateName();
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_FALSE);
    if (m_objSessions.GetValue(piSession)->GetMessageSender().SendEarlyUpdate(
            UpdateType::NORMAL) == IMS_FAILURE)
    {
        IMS_TRACE_D("QosReserved : Fail to send early UPDATE.", 0, 0, 0);
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        FailReason objReason(FAIL_REASON_SESSION_PRECONDITION);
        HandleCancel(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionStarted(IN ISession* piSession)
{
    IMessage* piMessage = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START);

    m_objContext.GetTimer().StopAll();

    m_objContext.GetCallInfo().bRttCapable = IsRttCapable(piMessage);
    UpdateCallType(piSession, piMessage, IMS_FALSE);
    HandleTip(piMessage);
    m_objSessions.GetValue(piSession)->GetExtensionSet().HandleResponse(
            IMessage::SESSION_START, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();

    if (MessageUtil::IsFocusConf(piMessage))
    {
        m_objContext.GetCallInfo().bConference = IMS_TRUE;
        m_objContext.GetCallInfo().bConferenceSubscriptionRequired =
                ConferenceConfigurationWrapper::IsConferenceSubscriptionRequired();

        objMediaManager.SetConferenceCall(IMS_TRUE);
    }

    objMediaManager.CreateMediaProfile(piSession, IMS_FALSE, IMS_TRUE);

    if (MessageUtil::HasSdp(piMessage) &&
            objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        m_objSessions.GetValue(piSession)->GetMessageSender().SendAck();
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    UpdatePreconditionCapability(piSession, piMessage);
    m_objContext.GetPreconditionManager().UpdatePreconditionAttributes(piSession);
    m_objContext.GetPreconditionManager().EnableRemoteCurrentStatus(piSession);

    if (objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_OFFER_RECEIVED)
    {
        if (objMediaManager.FormSdp(piSession, CallType::VOIP) == IMS_FAILURE)
        {
            m_objSessions.GetValue(piSession)->GetMessageSender().SendAck();
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
        }

        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_FALSE);
    }

    if (m_objSessions.GetValue(piSession)->GetMessageSender().SendAck() == IMS_FAILURE)
    {
        FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    objMediaManager.Run(piSession, piMessage, IMS_FALSE, IMS_FALSE);
    OnStarted(piSession);

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionStartFailed(IN ISession* piSession)
{
    m_objContext.GetMediaManager().Terminate();

    IMessage* piResponse = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START);
    FailReason objReason = StartErrorHandler(m_objContext).Handle(piResponse);

    OnStartFailed(piSession, objReason);

    return TransitToTerminating(objReason);
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionTerminated(IN ISession* piSession)
{
    m_objContext.GetMediaManager().Terminate();

    FailReason objReason = TerminationHandler().Handle(*piSession);
    OnStartFailed(piSession, objReason);

    return TransitToTerminating(objReason);
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMessage* piMessage = MessageUtil::GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);

    m_objContext.GetCallInfo().bRttCapable = IsRttCapable(piMessage);
    UpdateCallType(piSession, piMessage, IMS_TRUE);
    m_objSessions.GetValue(piSession)->GetExtensionSet().HandleResponse(
            IMessage::SESSION_EARLY_UPDATE, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.HandleRingBackTone(piSession, piMessage);

    if (m_objContext.GetMediaManager().NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        HandleCancel(piSession, FailReason(FAIL_REASON_MEDIA_NEGOFAIL));
        // TODO: SendStartFailedToListn(failReason, bTerminated);
    }

    // update remote qos status
    UpdatePreconditionCapability(piSession, piMessage);
    m_objContext.GetPreconditionManager().UpdatePreconditionAttributes(piSession);

    objMediaManager.Run(piSession, piMessage, IMS_TRUE, IMS_FALSE);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendProgressing(
            &m_objContext.GetCallInfo(),
            &objMediaInfo,
            m_objContext.GetSupplementaryService().GetAll(),
            m_bRemoteAlerted);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    FailReason objReason(FAIL_REASON_NONE);
    /*
    if (UC_FAILURE(m_objContext.GetSlotId())->EarlyUpdateFailure(piSession, nStatusCode, objReason))
    {
        return GetCurrentState();
    }
    */

    if (nStatusCode == SIPStatusCode::SC_INVALID)
    {
        objReason.nReason = FAIL_REASON_SESSION_RES_TIMEOUT;
    }
    else
    {
        objReason.nReason = FAIL_REASON_SESSION_SETUPFAILED;
    }
    objReason.nExtra = nStatusCode;

    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return TransitToTerminating(objReason);
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

    m_objContext.GetCallInfo().bRttCapable = IsRttCapable(piMessage);
    UpdateCallType(piSession, piMessage, IMS_TRUE);
    m_objSessions.GetValue(piSession)->GetExtensionSet().HandleRequest(
            IMessage::SESSION_EARLY_UPDATE, *piMessage);

    m_objContext.GetTimer().Start(TIMER_MO_NOANSWER, 60000);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.HandleRingBackTone(piSession, piMessage);

    if (!MessageUtil::HasSdp(piMessage))
    {
        if (m_objSessions.GetValue(piSession)->GetMessageSender()
                .RespondToEarlyUpdate(SIPStatusCode::SC_200) == IMS_FAILURE)
        {
            FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);
            return TransitToTerminating(objReason);
        }
        return GetStateName();
    }

    if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
    {
        if (m_objSessions.GetValue(piSession)->GetMessageSender()
                .RespondToEarlyUpdate(SIPStatusCode::SC_400) == IMS_FAILURE)
        {
            FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);
            return TransitToTerminating(objReason);
        }
        return GetStateName();
    }

    if (objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR ||
            objMediaManager.FormSdp(piSession, CallType::VOIP) == IMS_FAILURE)
    {
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    UpdatePreconditionCapability(piSession, piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    objPreconditionManager.UpdatePreconditionAttributes(piSession);
    objPreconditionManager.FormPreconditionSdp(piSession, IMS_FALSE);

    if (m_objSessions.GetValue(piSession)->GetMessageSender()
            .RespondToEarlyUpdate(SIPStatusCode::SC_200) == IMS_FAILURE)
    {
        FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    objMediaManager.Run(piSession, piMessage, IMS_TRUE, IMS_FALSE /* 180Received - should fix*/);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendProgressing(
            &m_objContext.GetCallInfo(),
            &objMediaInfo,
            m_objContext.GetSupplementaryService().GetAll());

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionForkedResponseReceived(
        IN ISession* piSession, IN ISession* piForkedSession)
{
    if (piSession == IMS_NULL || piForkedSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "Session is null", 0, 0, 0);
        return GetStateName();
    }

    m_objContext.GetSipInterfaceFactory().GetISessionHolder()->AddISession(piForkedSession);

    m_objSessions.Add(piForkedSession, m_objContext.CreateSession(*piForkedSession));
    m_objContext.GetMediaManager().CreateMediaProfile(piForkedSession, IMS_TRUE, IMS_TRUE);
    m_objContext.GetPreconditionManager().CreateQos(piForkedSession);

    // TODO: need any timer for the forked session?

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionPRAckDelivered(IN ISession* piSession)
{
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    UpdatePreconditionCapability(piSession, piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    objPreconditionManager.UpdatePreconditionAttributes(piSession);

    if (!objPreconditionManager.HasPreconditionCapability(piSession))
    {
        return GetStateName();
    }

    IMS_BOOL bLocalQosEnabled =
            objPreconditionManager.IsQosEnabled(piSession, QosCheckType::LOCAL_STATUS);

    if (!bLocalQosEnabled)
    {
        return GetStateName();
    }

    if (piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE) != IMS_NULL &&
            SdpPreconditionHelper::IsQosActivatedInSdp(piSession, IMessage::SESSION_EARLY_UPDATE))
    {
        return GetStateName();
    }

    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START);

    if (nStatusCode == SIPStatusCode::SC_183)
    {
        IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
        if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
        {
            return GetStateName();
        }

        if (objMediaManager.FormSdp(piSession, CallType::VOIP) == IMS_FAILURE)
        {
            return GetStateName();
        }

        objPreconditionManager.FormPreconditionSdp(piSession, IMS_FALSE);
        if (m_objSessions.GetValue(piSession)->GetMessageSender().SendEarlyUpdate(
                UpdateType::NORMAL) == IMS_FAILURE)
        {
            IMS_TRACE_D("SessionPRAckDelivered : Fail to send early UPDATE.", 0, 0, 0);
        }
    }
    else if (nStatusCode == SIPStatusCode::SC_200)
    {
        // TODO: send update after sending ACK to 200 OK response.
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionPRAckDeliveryFailed(IN ISession* piSession)
{
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START);
    FailReason objReason = FailReason(
            nStatusCode == SIPStatusCode::SC_INVALID ?
            FAIL_REASON_SESSION_RES_TIMEOUT :
            FAIL_REASON_SESSION_SETUPFAILED,
            nStatusCode);
    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return TransitToTerminating(objReason);
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionProvisionalResponseReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    m_objContext.GetTimer().Stop(TIMER_MO_1XX_WAIT);
    m_objContext.GetTimer().Start(TIMER_MO_NOANSWER, 60000);

    IMessage* piMessage = MessageUtil::GetPreviousResponse(
            piSession, IMessage::SESSION_START, nIndex);
    m_objSessions.GetValue(piSession)->GetExtensionSet().HandleResponse(
            IMessage::SESSION_START, *piMessage);
    if (!m_objSessions.GetValue(piSession)->GetExtensionSet()
            .IsSupportRequiredExtensions(*piMessage))
    {
        FailReason objReason(FAIL_REASON_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    HandleTip(piMessage);

    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SIPStatusCode::SC_180)
    {
        m_bRemoteAlerted = IMS_TRUE;
    }
    if (nStatusCode == SIPStatusCode::SC_199)
    {
        // TODO: An early dialog is terminated
        return GetStateName();
    }

    UpdateCallType(piSession, piMessage, IMS_FALSE);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.HandleRingBackTone(piSession, piMessage);

    if (piSession->IsSDPNegotiationAllowedForNonRPR() && MessageUtil::HasSdp(piMessage))
    {
        if (objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
        {
            FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);
            return TransitToTerminating(objReason);
        }
    }

    if (nStatusCode == SIPStatusCode::SC_180/* && local precondition support? */)
    {
        m_objContext.GetPreconditionManager().EnableRemoteCurrentStatus(piSession);
    }

    objMediaManager.Run(piSession, piMessage, IMS_TRUE, IMS_FALSE);

    // TODO: StartE911RingBackTimer(m_pSessInfo->eCallType);
    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendProgressing(
            &m_objContext.GetCallInfo(),
            &objMediaInfo,
            m_objContext.GetSupplementaryService().GetAll(),
            m_bRemoteAlerted);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName OutgoingState::SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    m_objContext.GetTimer().Stop(TIMER_MO_1XX_WAIT);
    m_objContext.GetTimer().Start(TIMER_MO_NOANSWER, 60000);

    IMessage* piMessage = MessageUtil::GetPreviousResponse(
            piSession, IMessage::SESSION_START, nIndex);
    m_objSessions.GetValue(piSession)->GetExtensionSet().HandleResponse(
            IMessage::SESSION_START, *piMessage);
    if (!m_objSessions.GetValue(piSession)->GetExtensionSet()
            .IsSupportRequiredExtensions(*piMessage))
    {
        FailReason objReason(FAIL_REASON_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    HandleTip(piMessage);

    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SIPStatusCode::SC_180)
    {
        m_bRemoteAlerted = IMS_TRUE;
    }
    if (nStatusCode == SIPStatusCode::SC_199)
    {
        // TODO: An early dialog is terminated
        return GetStateName();
    }

    UpdateCallType(piSession, piMessage, IMS_FALSE);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.HandleRingBackTone(piSession, piMessage);

    if (MessageUtil::HasSdp(piMessage) &&
            objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);
        return TransitToTerminating(objReason);
    }

    UpdatePreconditionCapability(piSession, piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    objPreconditionManager.UpdatePreconditionAttributes(piSession);

    if (nStatusCode == SIPStatusCode::SC_180)
    {
        objPreconditionManager.EnableRemoteCurrentStatus(piSession);
    }

    if (m_objSessions.GetValue(piSession)->GetMessageSender().SendPrack() == IMS_FAILURE)
    {
        // TODO: If there is no ISession in ISession::STATE_ESTABLISHED state and
        // not piSession->IsFinalResponseReceivedForInitialInviteRequest())
        {
            FailReason objReason(FAIL_REASON_UNKNOWN);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);
            return TransitToTerminating(objReason);
        }
    }

    objMediaManager.Run(piSession, piMessage, IMS_TRUE, IMS_FALSE);

    if (objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED &&
            !objPreconditionManager.IsQosEnabled(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendProgressing(
            &m_objContext.GetCallInfo(),
            &objMediaInfo,
            m_objContext.GetSupplementaryService().GetAll(),
            m_bRemoteAlerted);

    return GetStateName();
}

PRIVATE
void OutgoingState::HandleCancel(IN ISession* piSession, IN const FailReason& objReason)
{
    m_objContext.GetTimer().Stop(MtcCallState::TimerType::TIMER_MO_1XX_WAIT);

    if (objReason.nReason != FAIL_REASON_SESSION_EARLYDIALOG)
    {
        m_objContext.GetMediaManager().Terminate();
    }

    if (objReason.nReason == FAIL_REASON_SESSION_EARLYDIALOG)
    {
        return;
    }

    if (objReason.nReason == FAIL_REASON_SESSION_PRECONDITION)
    {
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
    }

    /* TODO: move to MessageSender?
    if (piSession->GetState() <= ISession::STATE_INITIATED)
    {
        IMS_TRACE_I("HandleCancel : %d %s", piSession->GetState(), PS_FR(objReason), 0);
        return;
    }
    */
    m_objSessions.GetValue(piSession)->GetMessageSender().Terminate(IMS_FALSE, objReason);
}

PRIVATE
void OutgoingState::HandleRetryAfter(IN const FailReason& objReason)
{
    IMS_SINT32 nRetryAfter = objReason.nExtra * 1000;
    m_objContext.GetTimer().Start(MtcCallState::TimerType::TIMER_RETRY_AFTER, nRetryAfter);
}

PRIVATE
IMS_BOOL OutgoingState::IsRttCapable(IN IMessage* piMessage)
{
    AString strContact;
    MessageUtil::GetHeader(piMessage, ISIPHeader::CONTACT_NORMAL, strContact);
    return MessageUtil::ContainsTag(strContact, "text");
}

PRIVATE
void OutgoingState::UpdateCallType(
        IN ISession* /* piSession */, IN IMessage* /* piMessage */, IN IMS_BOOL /* bPeerView */)
{
    CallType eCallType = /* MessageUtil::CheckSessionType(piMessage, piSession, bPeerView,
            m_objContext.GetCallInfo().eServiceType) */ CallType::VOIP;
    if (eCallType == CallType::UNKNOWN)
    {
        IMS_TRACE_I("UpdateCallType : NOT UPDATE", 0, 0, 0);
        return;
    }

    m_objContext.GetCallInfo().eCallType = eCallType;
}

PRIVATE
void OutgoingState::HandleTip(IN IMessage* piMessage)
{
    AString strPrivacy;
    MessageUtil::GetHeader(piMessage, ISIPHeader::PRIVACY, strPrivacy);
    IMS_BOOL bHasPAssertedIdentity =
            MessageUtil::IsHeaderPresent(piMessage, ISIPHeader::P_ASSERTED_IDENTITY);

    SuppService* pSuppService = new SuppService();
    pSuppService->nType = SUPP_TYPE_TIP;
    if (!bHasPAssertedIdentity && strPrivacy.EqualsIgnoreCase("id"))
    {
        pSuppService->nValue = TIP_TYPE_RESTRICTED;
    }
    else if (!bHasPAssertedIdentity && !(strPrivacy.EqualsIgnoreCase("id")))
    {
        pSuppService->nValue = TIP_TYPE_NONE;
    }
    else
    {
        pSuppService->nValue = TIP_TYPE_IDENTITY;
        AString strNumber;
        AString strName;
        MessageUtil::GetUserPart(piMessage, ISIPHeader::P_ASSERTED_IDENTITY, strNumber);
        MessageUtil::GetDisplayName(piMessage, ISIPHeader::P_ASSERTED_IDENTITY, strName);
        pSuppService->aStrValue.Append(strNumber);
        pSuppService->aStrValue.Append(',');
        pSuppService->aStrValue.Append(strName);
    }
    m_objContext.GetSupplementaryService().Add(pSuppService);
}

PRIVATE
void OutgoingState::HandleCountrySpecificServiceUrn(IN IMessage* piMessage)
{
    // If there is an alternative service URN in the Contact header of the 380 response,
    // it should be used to the subsequent emergency call.

    if ((piMessage->GetStatusCode() == SIPStatusCode::SC_380) &&
            MessageUtil::IsHeaderPresent(piMessage, ISIPHeader::CONTACT_NORMAL))
    {
        AString strServiceUrn;
        MessageUtil::GetHeader(piMessage, ISIPHeader::CONTACT_NORMAL, strServiceUrn);

        if (strServiceUrn.StartsWith("urn:service:sos.country-specific"))
        {
            AString strNumber;
            MessageUtil::GetUserPart(piMessage, ISIPHeader::TO, strNumber);
            m_objContext.GetDialingPlan().OnCountrySpecificServiceUrnReceived(
                    strNumber, strServiceUrn);
        }
    }
}

PRIVATE
void OutgoingState::OnStarted(IN ISession* piSession)
{
    m_objContext.SetSession(m_objSessions.GetValue(piSession));

    // TODO: stop call init timers

    if (!m_objContext.IsEct())
    {
        MediaInfo objMediaInfo;
        m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

        m_objContext.GetUiNotifier().SendStarted(
                &m_objContext.GetCallInfo(),
                &objMediaInfo,
                m_objContext.GetSupplementaryService().GetAll());
    }
}

PRIVATE
void OutgoingState::OnStartFailed(IN ISession* piSession, IN const FailReason& objReason)
{
    if (objReason.nReason == FAIL_REASON_SESSION_RETRY_SILENT)
    {
        HandleRetryAfter(objReason);
        return;
    }

    if (objReason.nReason == FAIL_REASON_SESSION_RETRY_R_RAT
            && objReason.nExtra == CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC)
    {
        HandleCountrySpecificServiceUrn(piSession->GetPreviousResponse(IMessage::SESSION_START));
    }
}

PRIVATE
void OutgoingState::DeleteInactiveSessions()
{
    MtcSession* pActiveSession = m_objContext.GetSession();
    for (IMS_UINT32 i = 0; i < m_objSessions.GetSize(); i++)
    {
        MtcSession* pDeleteSession = m_objSessions.GetValueAt(i);
        if (pDeleteSession != pActiveSession)
        {
            m_objContext.GetPreconditionManager().DestroyQos(&pDeleteSession->GetISession());
            delete pDeleteSession;
        }
    }
}

PRIVATE
void OutgoingState::UpdatePreconditionCapability(IN ISession* piSession, IN IMessage* piMessage)
{
    if (!MessageUtil::HasSdp(piMessage))
    {
        return;
    }

    IMS_BOOL bRemoteCapability = IMS_FALSE;
    IMS_BOOL bHasSupportedHeader =
            MessageUtil::HasValue(piMessage, MessageUtil::STR_PRECONDITION, ISIPHeader::SUPPORTED);
    IMS_BOOL bHasRequireHeader =
            MessageUtil::HasValue(piMessage, MessageUtil::STR_PRECONDITION, ISIPHeader::REQUIRE);

    if (SdpPreconditionHelper::IsPreconditionIncludedInSdp(piSession) &&
            (bHasSupportedHeader || bHasRequireHeader))
    {
        bRemoteCapability = IMS_TRUE;
    }

    IMS_TRACE_D("UpdatePreconditionCapability : Precondition Capability on remote UE[%d]",
            bRemoteCapability, 0, 0);

    m_objContext.GetPreconditionManager().UpdatePreconditionCapability(
            piSession, bRemoteCapability);
}
