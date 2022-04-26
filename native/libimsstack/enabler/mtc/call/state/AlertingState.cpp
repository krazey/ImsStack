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
#include "SIPStatusCode.h"
#include "utility/MessageUtil.h"
#include "helper/MtcSupplementaryService.h"
#include "precondition/QosDef.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
AlertingState::AlertingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ALERTING, objContext)
{
}

PUBLIC VIRTUAL
AlertingState::~AlertingState()
{
}

PUBLIC VIRTUAL
CallStateName AlertingState::HandleUserAlert()
{
    IMS_TRACE_D("HandleUserAlert", 0, 0, 0);
    if (SendProvisionalResponse(IMS_TRUE) == IMS_FAILURE)
    {
        return RejectAndToTerminating(REJECT_REASON_SESSION_FAIL);
    }
    // m_objContext.GetTimer().Start(TIMER_MT_ALERTING,
    //         UCCONFIG_GET_INT(m_nSlotID, SESSION_TIME_MT_ALERTING) * 1000);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Accept", 0, 0, 0);
    IMS_BOOL bCallTypeChanged = m_objContext.GetCallInfo().eCallType != eCallType;

    m_objContext.GetCallInfo().eCallType = eCallType;

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
        return RejectAndToTerminating(REJECT_REASON_SESSION_FAIL);
    }

    RunMedia(GetISession(), IMS_NULL);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::Reject(IN const FailReason& objReason)
{
    IMS_TRACE_D("Reject", 0, 0, 0);
    // TODO: Convert reason (UCSession::ConvertRejectReasonToFailReason)
    return RejectAndToTerminating(objReason);
}

PUBLIC VIRTUAL
CallStateName AlertingState::HandleSrvccSuccess()
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::HandleSrvccFailure(IN UpdateType /* eUpdateType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MT_ALERTING:
            // TODO: reject w/ REJECT_REASON_TO_MT_NOANSWER
            // return TransitToTerminating(objReason);
            return GetStateName();
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_D("QosReserveFailed", 0, 0, 0);

    if (eNextAction == QosLossPolicy::RELEASE)
    {
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
        return RejectAndToTerminating(REJECT_REASON_SESSION_FAIL_PRECONDITION);
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call established.
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted - ACK received", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    UpdateCallTypeFromMessage(piMessage, piSession);
    m_objContext.GetSession()->GetExtensionSet().HandleRequest(IMessage::SESSION_ACK, *piMessage);

    // TODO: need to check NegotiationState::STATE_OFFER_SENT?
    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        // TODO TerminateAndToTerminating() ?
        FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
        m_objContext.GetSession()->GetMessageSender().Terminate(IMS_TRUE, objReason);
        return TransitToTerminating(objReason);
    }

    SendStarted();
    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_TERMINATE);

    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SIPMethod::CANCEL))
    {
        return TransitToTerminating(CancelHandler().Handle(*piMessage));
    }
    return TransitToTerminating(TerminationHandler().Handle(*piSession));
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage = MessageUtil::GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    UpdateCallTypeFromMessage(piMessage, piSession);
    NegotiateExtension(m_objContext.GetSession(), piMessage, IMessage::SESSION_EARLY_UPDATE);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        return RejectAndToTerminating(REJECT_REASON_MEDIA_NEGOFAIL);
    }

    if (SendAccept() == IMS_FAILURE)
    {
        return RejectAndToTerminating(REJECT_REASON_SESSION_FAIL);
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionEarlyMediaUpdateFailed(IN ISession* /* piSession */)
{
    /*
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    TODO: failure handler
    */

    return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

    UpdateCallTypeFromMessage(piMessage, piSession); // TODO: why only in AlertingState?
    NegotiateExtension(m_objContext.GetSession(), piMessage, IMessage::SESSION_EARLY_UPDATE);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        if (SendResponseToEarlyUpdate(SIPStatusCode::SC_488, m_objContext.GetSession()) ==
                IMS_FAILURE)
        {
            return RejectAndToTerminating(REJECT_REASON_MEDIA_NEGOFAIL);
        }
        return GetStateName();
    }

    // TODO: RFC 6337 offer/answer check.

    if (SendResponseToEarlyUpdate(SIPStatusCode::SC_200, m_objContext.GetSession()) == IMS_FAILURE)
    {
        return RejectAndToTerminating(REJECT_REASON_SESSION_FAIL);
    }

    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    m_objContext.GetTimer().Stop(TIMER_MT_PRACK_WAIT);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);

    UpdateCallTypeFromMessage(piMessage, piSession);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        if (SendResponseToPrack(SIPStatusCode::SC_488) == IMS_FAILURE)
        {
            return RejectAndToTerminating(REJECT_REASON_MEDIA_NEGOFAIL);
        }
        return GetStateName();
    }

    // TODO: RFC 6337 offer/answer check.

    if (SendResponseToPrack(SIPStatusCode::SC_200) == IMS_FAILURE)
    {
        return RejectAndToTerminating(REJECT_REASON_SESSION_FAIL);
    }
    return CallStateName::ALERTING;
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

PRIVATE
void AlertingState::UpdateCallTypeFromMessage(IN IMessage* piMessage, IN ISession* piSession)
{
    CallType eNewCallType = MessageUtil::GetCallType(piMessage, piSession, IMS_FALSE);
    if (eNewCallType != CallType::UNKNOWN)
    {
        IMS_TRACE_D("UpdateCallTypeFromMessage : [%d] -> [%d]",
                m_objContext.GetCallInfo().eCallType, eNewCallType, 0);
        m_objContext.GetCallInfo().eCallType = eNewCallType;
    }
}
