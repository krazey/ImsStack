#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/IncomingState.h"
#include "call/termination/TerminationHandler.h"
#include "helper/MtcTimerWrapper.h"
#include "IMessage.h"
#include "ISession.h"
#include "SipStatusCode.h"
#include "utility/MessageUtil.h"
#include "helper/MtcSupplementaryService.h"
#include "precondition/QosDef.h"
#include "precondition/IMtcPreconditionManager.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
IncomingState::IncomingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::INCOMING, objContext)
{
    IMS_TRACE_D("+IncomingState", 0, 0, 0);
}

PUBLIC VIRTUAL
IncomingState::~IncomingState()
{
    IMS_TRACE_D("~IncomingState", 0, 0, 0);
}

PUBLIC VIRTUAL
CallStateName IncomingState::HandleSrvccSuccess()
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName IncomingState::HandleSrvccFailure(IN UpdateType /* eUpdateType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName IncomingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MT_PRACK_WAIT:
            // TODO: reject w/ REJECT_REASON_TO_MT_PRACK
            m_objContext.GetUiNotifier().SendStartFailed(FailReason(FAIL_REASON_UNKNOWN));
            return CallStateName::TERMINATING;
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName IncomingState::QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    if (!m_objContext.GetSession()->GetExtensionSet().IsAvailableOnBoth(
            MtcExtensionSet::OPTION_TAG_RPR))
    {
        return GetStateName();
    }

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_D("QosReserved : UE doesn't receive PRACK.", 0, 0, 0);
        return GetStateName();
    }

    IMS_TRACE_D("QosReserved : Media[%d] is reserved.", eMediaType, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_D("QosReserveFailed", 0, 0, 0);
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        FailReason objReason(REJECT_REASON_SESSION_FAIL_PRECONDITION);
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);
        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        return CallStateName::TERMINATING;
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
    }

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    m_objContext.GetUiNotifier().SendStartFailed(TerminationHandler().Handle(*piSession));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage = MessageUtil::GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    UpdateCallTypeFromMessage(piMessage, piSession);
    NegotiateExtension(m_objContext.GetSession(), piMessage, IMessage::SESSION_EARLY_UPDATE);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_MEDIA_NEGOFAIL));
    }

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (m_objContext.GetMediaManager().GetNegotiationState(piSession) ==
            NegotiationState::STATE_NEGOTIATED)
    {
        if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
        {
            m_objContext.GetPreconditionManager().StartQosTimer(piSession);
        }
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionEarlyMediaUpdateFailed(IN ISession* /* piSession */)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    /*
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    TODO: failure handler
    */

    m_objContext.GetUiNotifier().SendStartFailed(FailReason(REJECT_REASON_SESSION_FAIL));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

    UpdateCallTypeFromMessage(piMessage, piSession);
    NegotiateExtension(m_objContext.GetSession(), piMessage, IMessage::SESSION_EARLY_UPDATE);

    if (!MessageUtil::HasSdp(piMessage))
    {
        if (m_objContext.GetSession()->GetMessageSender()
                .RespondToEarlyUpdate(SipStatusCode::SC_200) == IMS_FAILURE)
        {
            m_objContext.GetUiNotifier().SendStartFailed(FailReason(REJECT_REASON_SESSION_FAIL));
            return CallStateName::TERMINATING;
        }

        return GetStateName();
    }

    // TODO: RFC 6337 offer/answer check.

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

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckReceived", 0, 0, 0);
    m_objContext.GetTimer().Stop(TIMER_MT_PRACK_WAIT);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);

    UpdateCallTypeFromMessage(piMessage, piSession);

    if (OnSdpReceived(piSession, piMessage) != FAIL_REASON_NONE)
    {
        if (SendResponseToEarlyUpdate(SipStatusCode::SC_488, m_objContext.GetSession()) ==
                IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_MEDIA_NEGOFAIL));
        }
        return GetStateName();
    }

    // TODO: RFC 6337 offer/answer check.

    if (SendResponseToPrack(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    SetLocalQosAvailableForWifiCalling(piSession);

    // TODO: CheckReadyToAlert()? common?
    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PRIVATE
void IncomingState::UpdateCallTypeFromMessage(IN IMessage* piMessage, IN ISession* piSession)
{
    CallType eNewCallType = MessageUtil::GetCallType(piMessage, piSession, IMS_FALSE);
    if (eNewCallType != CallType::UNKNOWN)
    {
        IMS_TRACE_D("UpdateCallTypeFromMessage : [%d] -> [%d]",
                m_objContext.GetCallInfo().eCallType, eNewCallType, 0);
        m_objContext.GetCallInfo().eCallType = eNewCallType;
    }
}
