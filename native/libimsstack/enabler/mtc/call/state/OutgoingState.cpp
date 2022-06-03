#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "ICoreService.h"
#include "dialogevent/IDialogEvent.h"
#include "call/IMtcCallContext.h"
#include "media/IMtcMediaManager.h"
#include "ImsAosParameter.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "call/message/MessageSender.h"
#include "utility/MessageUtil.h"
#include "MtcDef.h"
#include "call/MtcSession.h"
#include "helper/MtcTimerWrapper.h"
#include "call/MtcUiNotifier.h"
#include "call/state/OutgoingState.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "helper/MtcAosConnector.h"
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
        m_bRemoteAlerted(IMS_FALSE),
        m_nSilentRedialCount(0)
{
}

PUBLIC VIRTUAL OutgoingState::~OutgoingState()
{
    DeleteInactiveSessions();
}

PUBLIC VIRTUAL void OutgoingState::OnEnter()
{
    MtcSession* pSession = m_objContext.GetSession();
    if (pSession)
    {
        m_objSessions.Add(&pSession->GetISession(), pSession);
    }
}

PUBLIC VIRTUAL CallStateName OutgoingState::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", PS_FR(objReason), 0, 0);

    FailReason objConvertedReason(objReason);
    objConvertedReason.nReason = ConvertTerminateReasonToFailReason(objReason.nReason);

    MtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        HandleCancel(&pSession->GetISession(), objConvertedReason);
    }

    m_objContext.GetUiNotifier().SendStartFailed(objConvertedReason);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::HandleSrvccFailure(IN UpdateType eUpdateType)
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

PUBLIC VIRTUAL CallStateName OutgoingState::QosReserved(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("QosReserved : MediaType[%d]", eMediaType, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.HasPreconditionCapability(piSession))
    {
        IMS_TRACE_D("QosReserved : There's no capability for precondition.", 0, 0, 0);
        return GetStateName();
    }

    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        IMS_TRACE_D("QosReserved : Resources of all media are not reserved.", 0, 0, 0);
        return GetStateName();
    }

    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL || piMessage->GetStatusCode() != SipStatusCode::SC_200)
    {
        IMS_TRACE_D("QosReserved : There's no PRACK or response to PRACK.", 0, 0, 0);
        return GetStateName();
    }

    if (piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE) != IMS_NULL &&
            SdpPreconditionHelper::IsLocalResourceReservedInSdp(
                    piSession, IMessage::SESSION_EARLY_UPDATE))
    {
        IMS_TRACE_D("QosReserved : UE already send early UPDATE with activated QoS.", 0, 0, 0);
        return GetStateName();
    }

    // send early UPDATE
    if (SendEarlyUpdate(m_objSessions.GetValue(piSession)) == IMS_FAILURE)
    {
        IMS_TRACE_D("QosReserved : Fail to send early UPDATE.", 0, 0, 0);
        // TODO: erroer handling.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        FailReason objReason(FAIL_REASON_SESSION_PRECONDITION);
        HandleCancel(piSession, objReason);
        m_objContext.GetUiNotifier().SendStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted", 0, 0, 0);
    IMessage* piMessage = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START);

    m_objContext.GetTimer().StopAll();
    m_objSessions.GetValue(piSession)->HandleResponse(IMessage::SESSION_START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    if (MessageUtil::IsFocusConf(piMessage))
    {
        m_objContext.GetCallInfo().bConference = IMS_TRUE;
        m_objContext.GetMediaManager().SetConferenceCall(IMS_TRUE);
    }

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        m_objSessions.GetValue(piSession)->GetMessageSender().SendAck();
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    UpdatePreconditionCapability(piSession, piMessage);
    m_objContext.GetPreconditionManager().SetRemoteResourceAvailable(piSession);

    if (SendAck(piSession) == IMS_FAILURE)
    {
        FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    OnStarted(piSession);

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);
    m_objContext.GetMediaManager().Terminate();

    IMessage* piResponse = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START);
    FailReason objReason = StartErrorHandler(m_objContext).Handle(piResponse);

    if (objReason.nReason == FAIL_REASON_SESSION_RETRY_SILENT)
    {
        return HandleSilentRetry(objReason);
    }

    OnStartFailed(piSession, objReason);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);
    m_objContext.GetMediaManager().Terminate();

    FailReason objReason = TerminationHandler().Handle(*piSession);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_EARLY_UPDATE);

    m_objSessions.GetValue(piSession)->HandleResponse(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    // update remote qos status
    UpdatePreconditionCapability(piSession, piMessage);

    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    IMessage* piResponse =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_EARLY_UPDATE);
    FailReason objReason = EarlyUpdateErrorHandler().Handle(piResponse);

    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

    m_objSessions.GetValue(piSession)->HandleRequest(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    // TODO: which operator requires this?
    // m_objContext.GetTimer().Start(TIMER_MO_NOANSWER, 60000);

    m_objContext.GetMediaManager().UpdatePemType(piSession, piMessage);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        if (SendResponseToEarlyUpdate(SipStatusCode::SC_488, m_objSessions.GetValue(piSession)) ==
                IMS_FAILURE)
        {
            FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
        return GetStateName();
    }

    UpdatePreconditionCapability(piSession, piMessage);  // TODO: not in AlertingState?

    if (SendResponseToEarlyUpdate(SipStatusCode::SC_200, m_objSessions.GetValue(piSession)) ==
            IMS_FAILURE)
    {
        FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    SendProgressing();  // TODO: enforce remote alert to false?
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionForkedResponseReceived(
        IN ISession* piSession, IN ISession* piForkedSession)
{
    IMS_TRACE_D("SessionForkedResponseReceived", 0, 0, 0);
    if (piSession == IMS_NULL || piForkedSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "Session is null", 0, 0, 0);
        return GetStateName();
    }

    m_objContext.GetSipInterfaceFactory().GetISessionHolder()->AddISession(piForkedSession);

    CallType eCallType = m_objSessions.GetValue(piSession)->GetCallType();
    m_objSessions.Add(piForkedSession, m_objContext.CreateSession(*piForkedSession, eCallType));
    m_objContext.GetMediaManager().CreateMediaProfile(piForkedSession, IMS_TRUE, IMS_TRUE);
    m_objContext.GetPreconditionManager().CreateQos(piForkedSession);

    OnSessionForked(piSession);

    // TODO: need any timer for the forked session?

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPRAckDelivered(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckDelivered", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    UpdatePreconditionCapability(piSession, piMessage);

    m_objSessions.GetValue(piSession)->HandleResponse(IMessage::SESSION_PRACK, *piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();

    SetLocalQosAvailableForWifiCalling(piSession);

    if (!objPreconditionManager.HasPreconditionCapability(piSession))
    {
        return GetStateName();
    }

    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        return GetStateName();
    }

    if (piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE) &&
            SdpPreconditionHelper::IsLocalResourceReservedInSdp(
                    piSession, IMessage::SESSION_EARLY_UPDATE))
    {
        return GetStateName();
    }

    if (SdpPreconditionHelper::IsLocalResourceReservedInSdp(piSession, IMessage::SESSION_START))
    {
        return GetStateName();
    }

    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START);

    if (nStatusCode == SipStatusCode::SC_183)
    {
        IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
        if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
        {
            return GetStateName();
        }

        if (SendEarlyUpdate(m_objSessions.GetValue(piSession)) == IMS_FAILURE)
        {
            FailReason objReason(FAIL_REASON_SESSION_SETUPFAILED);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
    }
    else if (nStatusCode == SipStatusCode::SC_200)
    {
        // TODO: send update after sending ACK to 200 OK response.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPRAckDeliveryFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckDeliveryFailed", 0, 0, 0);
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START);
    FailReason objReason =
            FailReason(nStatusCode == SipStatusCode::SC_INVALID ? FAIL_REASON_SESSION_RES_TIMEOUT
                                                                : FAIL_REASON_SESSION_SETUPFAILED,
                    nStatusCode);
    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionProvisionalResponseReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionProvisionalResponseReceived", 0, 0, 0);
    StopTimer(TIMER_MO_1XX_WAIT);
    StartTimer(TIMER_MO_NOANSWER);

    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START, nIndex);
    m_objSessions.GetValue(piSession)->HandleResponse(IMessage::SESSION_START, *piMessage);

    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        FailReason objReason(FAIL_REASON_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    IMS_SINT32 nStatusCode =
            MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SipStatusCode::SC_180)
    {
        m_bRemoteAlerted = IMS_TRUE;
    }
    if (nStatusCode == SipStatusCode::SC_199)
    {
        // TODO: An early dialog is terminated
        return GetStateName();
    }

    m_objContext.GetMediaManager().UpdatePemType(piSession, piMessage);

    // TODO: not to update precondition attributes?
    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    if (nStatusCode == SipStatusCode::SC_180 /* && local precondition support? */)
    {
        m_objContext.GetPreconditionManager().SetRemoteResourceAvailable(piSession);
    }

    // TODO: StartE911RingBackTimer(m_pSessInfo->eCallType);
    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionRPRReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionRPRReceived", 0, 0, 0);
    StopTimer(TIMER_MO_1XX_WAIT);

    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START, nIndex);
    m_objSessions.GetValue(piSession)->HandleResponse(IMessage::SESSION_START, *piMessage);

    if (m_objContext.GetConfigurationProxy().Is(
            Feature::STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY) &&
            piMessage->GetStatusCode() == SipStatusCode::SC_183 && MessageUtil::HasSdp(piMessage))
    {
        StopTimer(TIMER_MO_NOANSWER);
    }
    else
    {
        StartTimer(TIMER_MO_NOANSWER);
    }

    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        FailReason objReason(FAIL_REASON_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    IMS_SINT32 nStatusCode =
            MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SipStatusCode::SC_180)
    {
        m_bRemoteAlerted = IMS_TRUE;
    }
    if (nStatusCode == SipStatusCode::SC_199)
    {
        // TODO: An early dialog is terminated
        return GetStateName();
    }

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    UpdatePreconditionCapability(piSession, piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (nStatusCode == SipStatusCode::SC_180)
    {
        objPreconditionManager.SetRemoteResourceAvailable(piSession);
    }

    if (SendPrack(piSession) == IMS_FAILURE)
    {
        // TODO: If there is no ISession in ISession::STATE_ESTABLISHED state and
        // not piSession->IsFinalResponseReceivedForInitialInviteRequest())
        {
            FailReason objReason(FAIL_REASON_UNKNOWN);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
    }

    if (objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED &&
            !objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::UssiStarted(IN ISession* piSession)
{
    IMS_TRACE_D("UssiStarted", 0, 0, 0);
    return SessionStarted(piSession);
}

PUBLIC
CallStateName OutgoingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MO_1XX_WAIT:
        {
            // TODO: fail reason name.
            FailReason objReason(FAIL_REASON_TO_MO_PROGRESSING);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(GetISession(), objReason);
            return CallStateName::TERMINATING;
        }
        case TIMER_MO_NOANSWER:
        {
            // TODO: fail reason name.
            FailReason objReason(FAIL_REASON_TO_MO_STARTED);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(GetISession(), objReason);
            return CallStateName::TERMINATING;
        }
        case TimerType::TIMER_RETRY_AFTER:
            return ContinueSilentRetry();
        default:
            return GetStateName();
    }
}

PRIVATE
IMS_RESULT OutgoingState::SendPrack(IN ISession* piSession)
{
    IMS_TRACE_D("SendPrack", 0, 0, 0);

    // Not supporting to send Offer in PRACK.
    if (SetSdpToSend(IMS_FALSE, piSession) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    return m_objSessions.GetValue(piSession)->GetMessageSender().SendPrack();
}

PRIVATE
IMS_RESULT OutgoingState::SendAck(IN ISession* piSession)
{
    IMS_TRACE_D("SendAck", 0, 0, 0);

    if (SetSdpToSend(IMS_FALSE, piSession) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    return m_objSessions.GetValue(piSession)->GetMessageSender().SendAck();
}

PRIVATE
void OutgoingState::HandleCancel(IN ISession* piSession, IN const FailReason& objReason)
{
    IMS_TRACE_D("HandleCancel", 0, 0, 0);
    StopTimer(MtcCallState::TimerType::TIMER_MO_1XX_WAIT);

    if (objReason.nReason == FAIL_REASON_SESSION_EARLYDIALOG)
    {
        return;
    }

    m_objContext.GetMediaManager().Terminate();
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
    if (m_objSessions.GetIndexOfKey(piSession) >= 0)
    {
        m_objSessions.GetValue(piSession)->GetMessageSender().Terminate(IMS_FALSE, objReason);
    }
}

PRIVATE
CallStateName OutgoingState::HandleSilentRetry(IN const FailReason& objReason)
{
    IMS_TRACE_D("HandleSilentRetry", 0, 0, 0);

    if (m_nSilentRedialCount >=
            m_objContext.GetConfigurationProxy().GetInt(Feature::SILENT_REDIAL_MAX_RETRY_COUNT))
    {
        IMS_TRACE_D("HandleRetrySilent : Max retry count[%d] reached", m_nSilentRedialCount, 0, 0);
        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        // TODO: Trigger initial registeration if requird (by config?)
        return CallStateName::TERMINATING;
    }
    m_nSilentRedialCount += 1;

    MtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        m_objContext.SetSession(IMS_NULL);
        m_objSessions.Remove(&pSession->GetISession());
        delete pSession;
    }

    /* TODO: Policy: retry timer */
    IMS_SINT32 nRetryAfterSecond = objReason.nExtra;
    if (nRetryAfterSecond <= 0)
    {
        return ContinueSilentRetry();
    }
    else
    {
        m_objContext.GetTimer().Start(TimerType::TIMER_RETRY_AFTER, nRetryAfterSecond * 1000);
    }

    /* TODO: Policy: LTE
    m_objContext.GetAosConnector(m_objContext.GetService().GetServiceType())
            ->Control(ImsAosControl::FALLBACK_TO_LTE_AND_LET_ME_KNOW_IT);
    */

    return GetStateName();
}

PRIVATE
CallStateName OutgoingState::ContinueSilentRetry()
{
    IMS_TRACE_D("ContinueRetrySilent", 0, 0, 0);

    if (CreateISession(m_objContext.GetCallInfo().eInitialCallType) == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
        return CallStateName::TERMINATING;
    }
    MtcSession* pSession = m_objContext.GetSession();
    if (pSession)
    {
        m_objSessions.Add(&pSession->GetISession(), pSession);
    }

    InitMediaSession();
    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (pSession->SendStart() == IMS_FAILURE)
    {
        m_objContext.GetMediaManager().Terminate();
        m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
        return CallStateName::TERMINATING;
    }

    return GetStateName();
}

PRIVATE
void OutgoingState::HandleCountrySpecificServiceUrn(IN IMessage* piMessage)
{
    // If there is an alternative service URN in the Contact header of the 380 response,
    // it should be used to the subsequent emergency call.

    if ((piMessage->GetStatusCode() == SipStatusCode::SC_380) &&
            MessageUtil::IsHeaderPresent(piMessage, ISipHeader::CONTACT_NORMAL))
    {
        AString strServiceUrn;
        MessageUtil::GetHeader(piMessage, ISipHeader::CONTACT_NORMAL, strServiceUrn);

        if (strServiceUrn.StartsWith("urn:service:sos.country-specific"))
        {
            AString strNumber;
            MessageUtil::GetUserPart(piMessage, ISipHeader::TO, strNumber);
            m_objContext.GetDialingPlan().OnCountrySpecificServiceUrnReceived(
                    strNumber, strServiceUrn);
        }
    }
}

PRIVATE
void OutgoingState::SendProgressing()
{
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();

    MediaInfo objMediaInfo;
    objMediaManager.GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendProgressing(&m_objContext.GetCallInfo(), &objMediaInfo,
            m_objContext.GetSupplementaryService().GetServices(), m_bRemoteAlerted);
}

PRIVATE
void OutgoingState::OnStarted(IN ISession* piSession)
{
    m_objContext.SetSession(m_objSessions.GetValue(piSession));

    // TODO: stop call init timers

    SendStarted();
}

PRIVATE
void OutgoingState::OnStartFailed(IN ISession* piSession, IN const FailReason& objReason)
{
    if (objReason.nReason == FAIL_REASON_SESSION_RETRY_R_RAT &&
            objReason.nExtra == CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC)
    {
        HandleCountrySpecificServiceUrn(piSession->GetPreviousResponse(IMessage::SESSION_START));
    }

    m_objContext.GetUiNotifier().SendStartFailed(objReason);
}

PRIVATE
void OutgoingState::OnSessionForked(IN ISession* piOriginSession)
{
    if (m_objContext.GetConfigurationProxy().Is(
                Feature::MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING))
    {
        return;
    }

    IMS_SINT32 nIndex = m_objSessions.GetIndexOfKey(piOriginSession);
    if (nIndex < 0)
    {
        return;
    }

    IMS_TRACE_I("OnSessionForked : Terminate previous session", 0, 0, 0);

    MtcSession* pOriginMtcSession = m_objSessions.GetValueAt(nIndex);
    pOriginMtcSession->GetMessageSender().Terminate(
            IMS_TRUE, FailReason(FAIL_REASON_SESSION_EARLYDIALOG));

    m_objContext.GetMediaManager().DestroyMediaProfile(piOriginSession);
    m_objContext.GetPreconditionManager().DestroyQos(piOriginSession);
    delete pOriginMtcSession;
    m_objSessions.RemoveAt(nIndex);
    m_objContext.SetSession(m_objSessions.GetValueAt(0));
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
