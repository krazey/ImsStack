#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/AlertingState.h"
#include "call/termination/CancelHandler.h"
#include "call/termination/TerminationHandler.h"
#include "configuration/ConfigDef.h"
#include "helper/MtcTimerWrapper.h"
#include "IMessage.h"
#include "ISession.h"
#include "SipStatusCode.h"
#include "utility/MessageUtil.h"
#include "helper/MtcSupplementaryService.h"
#include "precondition/QosDef.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ServiceTrace.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
AlertingState::AlertingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ALERTING, objContext)
{
}

PUBLIC VIRTUAL AlertingState::~AlertingState() {}

PUBLIC VIRTUAL CallStateName AlertingState::HandleUserAlert()
{
    IMS_TRACE_D("HandleUserAlert", 0, 0, 0);
    if (SendProvisionalResponse(IMS_TRUE) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }
    StartTimer(TIMER_MT_ALERTING);

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Accept", 0, 0, 0);

    IMS_BOOL bCallTypeChanged = m_objContext.GetSession()->GetCallType() != eCallType;
    m_objContext.GetSession()->SetCallType(eCallType);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.SetMediaInfo(*pMediaInfo);

    if (bCallTypeChanged)
    {
        // TODO: Send early update
        return GetStateName();
    }

    m_objContext.GetTimer().StopAll();

    if (SendAccept() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    RunMedia(GetISession(), IMS_NULL);

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::Reject(IN const FailReason& objReason)
{
    IMS_TRACE_D("Reject", 0, 0, 0);
    return RejectIncomingAndToTerminating(objReason);
}

PUBLIC VIRTUAL CallStateName AlertingState::HandleSrvccSuccess()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::HandleSrvccFailure(IN UpdateType /* eUpdateType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MT_ALERTING:
            return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_TO_MT_NOANSWER));
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_D("QosReserveFailed", 0, 0, 0);

    if (eNextAction == QosLossPolicy::RELEASE)
    {
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL_PRECONDITION));
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call established.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted - ACK received", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_ACK, *piMessage);

    // TODO: need to check NegotiationState::STATE_OFFER_SENT?
    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        // TODO TerminateAndToTerminating() ?
        m_objContext.GetMediaManager().Terminate();
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        m_objContext.GetSession()->GetMessageSender().Terminate(IMS_TRUE, objReason);
        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        return CallStateName::TERMINATING;
    }

    SendStarted();
    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);
    m_objContext.GetMediaManager().Terminate();
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_TERMINATE);
    if (piMessage == IMS_NULL)
    {
        return CallStateName::TERMINATING;
    }

    m_objContext.GetUiNotifier().SendStartFailed(piMessage->GetMethod().Equals(SipMethod::CANCEL)
                    ? CancelHandler().Handle(*piMessage)
                    : TerminationHandler().Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_EARLY_UPDATE);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_MEDIA_NEGOFAIL));
    }

    if (SendAccept() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdateFailed(
        IN ISession* /* piSession */)
{
    m_objContext.GetMediaManager().Terminate();
    /*
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    TODO: failure handler
    */

    m_objContext.GetUiNotifier().SendStartFailed(FailReason(REJECT_REASON_SESSION_FAIL));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        if (SendResponseToEarlyUpdate(SipStatusCode::SC_488, m_objContext.GetSession()) ==
                IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_MEDIA_NEGOFAIL));
        }
        return GetStateName();
    }

    if (SendResponseToEarlyUpdate(SipStatusCode::SC_200, m_objContext.GetSession()) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_PRACK, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        SendResponseToPrack(SipStatusCode::SC_200);
        // According to RFC 6337, UE must send re-offer.
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_NOTACCEPTABLEHERE));
    }

    if (SendResponseToPrack(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionRPRDeliveryFailed(IN ISession* /* piSession*/)
{
    IMS_TRACE_D("SessionRPRDeliveryFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_TO_MT_PRACK));
}

PUBLIC VIRTUAL CallStateName AlertingState::AcceptUssi(
        IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("AcceptUssi", 0, 0, 0);

    m_objContext.GetSession()->SetCallType(eCallType);
    m_objContext.GetMediaManager().SetMediaInfo(*pMediaInfo);

    m_objContext.GetTimer().StopAll();

    if (m_objContext.GetUssiController()->FormAcceptUssi() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    if (SendAccept() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::UssiStarted(IN ISession* piSession)
{
    IMS_TRACE_D("UssiStarted - ACK received", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_ACK, *piMessage);

    UssiResult objResult = m_objContext.GetUssiController()->ParseUssiBodyAndCheckResult(
            piMessage->GetMessage(), piMessage->GetMethod().ToInt());

    SendStarted();

    if (objResult.eAction != UssiNextAction::NOTHING)
    {
        SendInfoForUssi(AString::ConstEmpty(), objResult.eErrorCode);
    }

    return CallStateName::ESTABLISHED;
}

PRIVATE
IMS_RESULT AlertingState::SendAccept()
{
    IMS_TRACE_D("SendAccept", 0, 0, 0);

    // TODO: "REJECT_REASON_MEDIA_FORMFAIL" is required?
    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    return m_objContext.GetSession()->GetMessageSender().Accept();
}
