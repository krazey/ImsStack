#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/IncomingState.h"
#include "call/termination/TerminationHandler.h"
#include "helper/MtcTimerWrapper.h"
#include "IMessage.h"
#include "ISession.h"
#include "SIPStatusCode.h"
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
}

PUBLIC VIRTUAL
IncomingState::~IncomingState()
{
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
            return TransitToTerminating(FailReason(FAIL_REASON_UNKNOWN));
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
    if (!objPreconditionManager.IsQosEnabled(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        FailReason objReason(REJECT_REASON_SESSION_FAIL_PRECONDITION);
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);
        return TransitToTerminating(objReason);
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
    return TransitToTerminating(TerminationHandler().Handle(*piSession));
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMessage* piMessage = MessageUtil::GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);

    UpdateCallTypeFromMessage(piMessage, piSession);
    m_objContext.GetSession()->GetExtensionSet().HandleResponse(
            IMessage::SESSION_EARLY_UPDATE, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();

    if (MessageUtil::HasSdp(piMessage))
    {
        if (objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
        {
            FailReason objReason(REJECT_REASON_MEDIA_NEGOFAIL);
            m_objContext.GetSession()->GetMessageSender().Reject(objReason);
            return TransitToTerminating(objReason);
        }

        objPreconditionManager.UpdatePreconditionAttributes(piSession);
    }

    if (objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED)
    {
        if (!objPreconditionManager.IsQosEnabled(piSession, QosCheckType::LOCAL_STATUS))
        {
            m_objContext.GetPreconditionManager().StartQosTimer(piSession);
        }
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsQosEnabled(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionEarlyMediaUpdateFailed(IN ISession* /* piSession */)
{
    /*
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    TODO: failure handler
    */

    return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

    UpdateCallTypeFromMessage(piMessage, piSession);

    m_objContext.GetSession()->GetExtensionSet().HandleRequest(
            IMessage::SESSION_EARLY_UPDATE, *piMessage);

    if (!MessageUtil::HasSdp(piMessage))
    {
        if (m_objContext.GetSession()->GetMessageSender()
                .RespondToEarlyUpdate(SIPStatusCode::SC_200) == IMS_FAILURE)
        {
            return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
        }

        return GetStateName();
    }

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
    {
        if (m_objContext.GetSession()->GetMessageSender().
                RespondToEarlyUpdate(SIPStatusCode::SC_400) == IMS_FAILURE)
        {
            FailReason objReason(REJECT_REASON_MEDIA_NEGOFAIL);
            m_objContext.GetSession()->GetMessageSender().Reject(objReason);
            return TransitToTerminating(objReason);
        }

        return GetStateName();
    }

    if (objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        FailReason objReason(REJECT_REASON_MEDIA_NEGOFAIL);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);
        return TransitToTerminating(objReason);
    }

    if (objMediaManager.FormSdp(piSession, CallType::VOIP) == IMS_FAILURE)
    {
        FailReason objReason(REJECT_REASON_MEDIA_FORMFAIL);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);
        return TransitToTerminating(objReason);
    }

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    objPreconditionManager.UpdatePreconditionAttributes(piSession);

    if (!objPreconditionManager.IsQosEnabled(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    objPreconditionManager.FormPreconditionSdp(piSession, IMS_FALSE);

    if (m_objContext.GetSession()->GetMessageSender()
            .RespondToEarlyUpdate(SIPStatusCode::SC_200) == IMS_FAILURE)
    {
        return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsQosEnabled(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName IncomingState::SessionPRAckReceived(IN ISession* piSession)
{
    m_objContext.GetTimer().Stop(TIMER_MT_PRACK_WAIT);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);

    UpdateCallTypeFromMessage(piMessage, piSession);

    if (MessageUtil::HasSdp(piMessage))
    {
        IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
        if (objMediaManager.NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
        {
            FailReason objReason(REJECT_REASON_MEDIA_NEGOFAIL);
            m_objContext.GetSession()->GetMessageSender().Reject(objReason);
            return TransitToTerminating(objReason);
        }

        if (objMediaManager.FormSdp(piSession, CallType::VOIP) == IMS_FAILURE)
        {
            FailReason objReason(REJECT_REASON_MEDIA_FORMFAIL);
            m_objContext.GetSession()->GetMessageSender().Reject(objReason);
            return TransitToTerminating(objReason);
        }

        IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
        objPreconditionManager.UpdatePreconditionAttributes(piSession);
        objPreconditionManager.FormPreconditionSdp(piSession, IMS_FALSE);

        if (!objPreconditionManager.IsQosEnabled(piSession, QosCheckType::LOCAL_STATUS))
        {
            objPreconditionManager.StartQosTimer(piSession);
        }
    }

    if (m_objContext.GetSession()->GetMessageSender()
            .RespondToPrack(SIPStatusCode::SC_200) == IMS_FAILURE)
    {
        return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    QosCheckType eCheckType =
            (m_objContext.GetPreconditionManager().HasPreconditionCapability(piSession)) ?
            QosCheckType::ALL_STATUS : QosCheckType::LOCAL_STATUS;
    if (!m_objContext.GetPreconditionManager().IsQosEnabled(piSession, eCheckType))
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
