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
    IMS_BOOL bMediaNegotiated = IMS_TRUE;   // TODO:
    IMS_BOOL bRprSupport = m_objContext.GetSession()->GetExtensionSet()
            .IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_RPR);
    IMS_BOOL bIsCallWaiting = IsCallWaiting();
    if (m_objContext.GetSession()->GetMessageSender().SendProvisionalResponse(
            SIPStatusCode::SC_180,
            bRprSupport, // TODO: From config: imsvoice.prack_supported_for_18x_bool
            !bMediaNegotiated,
            bIsCallWaiting) == IMS_FAILURE)
    {
        FailReason objReason(REJECT_REASON_SESSION_FAIL);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);
        return TransitToTerminating(objReason);
    }

    // m_objContext.GetTimer().Start(TIMER_MT_ALERTING,
    //         UCCONFIG_GET_INT(m_nSlotID, SESSION_TIME_MT_ALERTING) * 1000);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
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

    ISession& objSession = m_objContext.GetSession()->GetISession();

    if (objMediaManager.GetNegotiationState(&objSession) == NegotiationState::STATE_OFFER_RECEIVED)
    {
        if (objMediaManager.FormSdp(&objSession, CallType::VOIP) == IMS_FAILURE)
        {
            FailReason objReason(REJECT_REASON_MEDIA_FORMFAIL);
            m_objContext.GetSession()->GetMessageSender().Reject(objReason);
            return TransitToTerminating(objReason);
        }

        m_objContext.GetPreconditionManager().FormPreconditionSdp(&objSession, IMS_FALSE);
    }

    if (SendAccept() == IMS_FAILURE)
    {
        return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    objMediaManager.Run(&objSession, IMS_NULL, IMS_FALSE, IMS_FALSE);

    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName AlertingState::Reject(IN const FailReason& objReason)
{
    m_objContext.GetSession()->GetMessageSender().Reject(objReason);
    // TODO: Convert reason (UCSession::ConvertRejectReasonToFailReason)

    return TransitToTerminating(objReason);
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
    UNUSED_PARAM(piSession);

    if (eNextAction == QosLossPolicy::RELEASE)
    {
        FailReason objReason(REJECT_REASON_SESSION_FAIL_PRECONDITION);
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);
        return TransitToTerminating(objReason);
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
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    UpdateCallTypeFromMessage(piMessage, piSession);
    m_objContext.GetSession()->GetExtensionSet().HandleRequest(IMessage::SESSION_ACK, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (MessageUtil::HasSdp(piMessage) &&
            (objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_OFFER_SENT))
    {
        if (objMediaManager.NegotiateSdp(piSession) == IMS_FAILURE)
        {
            FailReason objReason(FAIL_REASON_MEDIA_NEGOFAIL);
            m_objContext.GetSession()->GetMessageSender().Terminate(IMS_TRUE, objReason);
            return TransitToTerminating(objReason);
        }

        objMediaManager.Run(piSession, piMessage, IMS_FALSE, IMS_FALSE);
    }

    MediaInfo objMediaInfo;
    objMediaManager.GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendStarted(
            &m_objContext.GetCallInfo(),
            &objMediaInfo,
            m_objContext.GetSupplementaryService().GetAll());

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionTerminated(IN ISession* piSession)
{
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

    if (SendAccept() == IMS_FAILURE)
    {
        return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
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
    // FIXME: It's same as IncomingState except QoS check and UI notifying

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

    m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_FALSE);

    if (m_objContext.GetSession()->GetMessageSender()
            .RespondToEarlyUpdate(SIPStatusCode::SC_200) == IMS_FAILURE)
    {
        return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL
CallStateName AlertingState::SessionPRAckReceived(IN ISession* piSession)
{
    // FIXME: It's same as IncomingState except QoS check and UI notifying

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
    }

    if (m_objContext.GetSession()->GetMessageSender()
            .RespondToPrack(SIPStatusCode::SC_200) == IMS_FAILURE)
    {
        return TransitToTerminating(FailReason(REJECT_REASON_SESSION_FAIL));
    }

    return CallStateName::ALERTING;
}

PRIVATE
IMS_RESULT AlertingState::SendAccept()
{
    if (m_objContext.GetSession()->GetMessageSender().Accept() == IMS_FAILURE)
    {
        FailReason objReason(REJECT_REASON_SESSION_FAIL);
        m_objContext.GetSession()->GetMessageSender().Reject(objReason);

        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
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

PRIVATE
IMS_BOOL AlertingState::IsCallWaiting()
{
    IMSList<IMtcCall*> lstCalls = m_objContext.GetCallManager().GetCalls();

    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::ESTABLISHED || eState == IMtcCall::State::UPDATING)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
