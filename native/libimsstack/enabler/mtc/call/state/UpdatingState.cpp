#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/UpdatingState.h"
#include "call/termination/UpdateErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "call/UpdatingInfo.h"
#include "helper/MtcSupplementaryService.h"
#include "IMessage.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdatingState::UpdatingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::UPDATING, objContext)
{
    IMS_TRACE_D("+UpdatingState", 0, 0, 0);
}

PUBLIC VIRTUAL
UpdatingState::~UpdatingState()
{
    IMS_TRACE_D("~UpdatingState", 0, 0, 0);
}

PUBLIC VIRTUAL
void UpdatingState::OnExit()
{
    m_objContext.DeleteUpdatingInfo();
}

PUBLIC VIRTUAL
CallStateName UpdatingState::AcceptConvert(IN CallType /* eCallType */, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("AcceptConvert", 0, 0, 0);

    MtcSession* pSession = m_objContext.GetSession();
    ISession& objSession = pSession->GetISession();
    if (objSession.GetState() == ISession::STATE_ESTABLISHED)
    {
        MediaInfo objMediaInfo;
        m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetAll());
        return CallStateName::ESTABLISHED;
    }

    m_objContext.GetMediaManager().SetMediaInfo(*pMediaInfo);

    if (m_objContext.GetMediaManager().FormSdp(&objSession, CallType::VOIP) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(&objSession, IMS_FALSE);

    m_objContext.GetMediaManager().GetMediaInfo(
            m_objContext.GetUpdatingInfo().GetModifiedInfo());

    if (pSession->GetMessageSender().AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SIPMethod::UPDATE))
    {
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
                &m_objContext.GetUpdatingInfo().GetModifiedInfo(),
                m_objContext.GetSupplementaryService().GetAll());
        return CallStateName::ESTABLISHED;
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL
CallStateName UpdatingState::RejectConvert(IN const FailReason& objReason)
{
    IMS_TRACE_D("RejectConvert", 0, 0, 0);

    if (m_objContext.GetSession()->GetISession().GetState() == ISession::STATE_ESTABLISHED)
    {
        if (SendUpdate() == IMS_FAILURE)
        {
            // TODO
        }

        return CallStateName::UPDATING;
    }

    if (m_objContext.GetSession()->GetMessageSender().Reject(objReason) == IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL
CallStateName UpdatingState::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    FailReason objConvertedReason(objReason);
    objConvertedReason.nReason = ConvertTerminateReasonToFailReason(objReason.nReason);

    // SetTerminateCodeForInvitedSessionToConf

    HandleTerminate(objConvertedReason);

    return TransitToTerminating(objConvertedReason);
}

PUBLIC VIRTUAL
CallStateName UpdatingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    m_objContext.GetMediaManager().Terminate();

    return TransitToTerminating(TerminationHandler().Handle(*piSession));
}

PUBLIC VIRTUAL
CallStateName UpdatingState::SessionUpdated(IN ISession* /* piSession */)
{
    IMS_TRACE_D("SessionUpdated", 0, 0, 0);

    if (HandleSdpAnswer() == IMS_FAILURE)
    {
        // TODO
    }

    if (SendAck() == IMS_FAILURE)
    {
        // TODO
    }

    // TODO, update call type

    return HandleModificationSucceeded();
}

PUBLIC VIRTUAL
CallStateName UpdatingState::SessionUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionUpdateFailed", 0, 0, 0);

    IMessage* piResponse = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_UPDATE);
    FailReason objReason = UpdateErrorHandler(m_objContext).Handle(piResponse);

    if (objReason.nReason == FAIL_REASON_SESSION_DESTROYED)
    {
        HandleTerminate(objReason);
        return CallStateName::TERMINATING;
    }
    else    // TODO: retry
    {
        RecoverModificationFailure();
        return CallStateName::ESTABLISHED;
    }
}

PRIVATE
IMS_RESULT UpdatingState::HandleSdpAnswer()
{
    IMS_TRACE_D("HandleSdpAnswer", 0, 0, 0);

    ISession* piSession = &m_objContext.GetSession()->GetISession();

    IMessage* piMessage = IMS_NULL;
    if (m_objContext.GetUpdatingInfo().IsModifier())
    {
        piMessage = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_UPDATE);
    }
    else
    {
        piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    }

    if (MessageUtil::HasSdp(piMessage) == IMS_FALSE)
    {
        return IMS_SUCCESS;
    }

    if (m_objContext.GetMediaManager().NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        // TODO
    }

    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifiedInfo());
    m_objContext.GetPreconditionManager().UpdateQosAttributesFromSdp(piSession);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT UpdatingState::SendAck()
{
    IMS_TRACE_D("SendAck", 0, 0, 0);

    if (!m_objContext.GetUpdatingInfo().IsModifier())
    {
        return IMS_SUCCESS;
    }

    return m_objContext.GetSession()->GetMessageSender().SendAck();
}

PRIVATE
IMS_RESULT UpdatingState::SendUpdate()
{
    IMS_TRACE_D("SendUpdate", 0, 0, 0);

    m_objContext.GetUpdatingInfo().GetModifyingInfo() =
            m_objContext.GetUpdatingInfo().GetNegotiatedInfo();
    m_objContext.GetUpdatingInfo().GetNegotiatedInfo() =
            m_objContext.GetUpdatingInfo().GetModifiedInfo();
    m_objContext.GetUpdatingInfo().GetAlertingInfo().eADir = DIRECTION_INVALID;
    m_objContext.GetUpdatingInfo().GetModifiedInfo().eADir = DIRECTION_INVALID;

    m_objContext.GetUpdatingInfo().SetModifier();
    m_objContext.GetMediaManager().SetMediaInfo(
            m_objContext.GetUpdatingInfo().GetModifyingInfo());

    MtcSession* pSession = m_objContext.GetSession();
    if (m_objContext.GetMediaManager().FormSdp(&(pSession->GetISession()), CallType::VOIP) ==
            IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(
            &(pSession->GetISession()), IMS_FALSE);

    if (pSession->GetMessageSender().Update(UpdateType::SESSION, IMS_FALSE) == IMS_FAILURE)
    {
        // TODO
    }

    return IMS_SUCCESS;
}

PRIVATE
CallStateName UpdatingState::HandleModificationSucceeded()
{
    IMS_TRACE_D("HandleModificationSucceeded", 0, 0, 0);

    NotifyHoldResumeState();

    CallStateName eCallStateName = CallStateName::ESTABLISHED;

    if (m_objContext.GetUpdatingInfo().IsModifier())
    {
        eCallStateName = HandleRequestedModificationSucceeded();
    }
    else
    {
        eCallStateName = HandleReceivedModificationSucceeded();
    }

    if (eCallStateName == CallStateName::ESTABLISHED)
    {
        m_objContext.GetMediaManager().Run(&m_objContext.GetSession()->GetISession(),
                IMS_NULL, IMS_FALSE, IMS_FALSE);
    }

    return eCallStateName;

}

PRIVATE
CallStateName UpdatingState::HandleRequestedModificationSucceeded()
{
    IMS_TRACE_D("HandleRequestedModificationSucceeded", 0, 0, 0);

    if (m_objContext.GetUpdatingInfo().IsRequestedModifying() == IMS_FALSE &&
            m_objContext.GetUpdatingInfo().IsModified())
    {
        NotifyIncomingUpdate();
        return CallStateName::UPDATING;
    }

    if (m_objContext.GetUpdatingInfo().IsHeld() || m_objContext.GetUpdatingInfo().IsResumed())
    {
        return CallStateName::ESTABLISHED;
    }

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
    m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()), &objMediaInfo,
            m_objContext.GetSupplementaryService().GetAll());

    return CallStateName::ESTABLISHED;
}

PRIVATE
CallStateName UpdatingState::HandleReceivedModificationSucceeded()
{
    IMS_TRACE_D("HandleReceivedModificationSucceeded", 0, 0, 0);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    if (m_objContext.GetUpdatingInfo().IsAlerted())
    {
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
                &objMediaInfo, m_objContext.GetSupplementaryService().GetAll());

        return CallStateName::ESTABLISHED;
    }

    if (m_objContext.GetUpdatingInfo().IsModified())
    {
        NotifyIncomingUpdate();
        return CallStateName::UPDATING;
    }

    if (m_objContext.GetUpdatingInfo().IsHeldBy() || m_objContext.GetUpdatingInfo().IsResumedBy())
    {
        return CallStateName::ESTABLISHED;
    }

    m_objContext.GetUiNotifier().SendUpdatedBy(&m_objContext.GetCallInfo(),
            &objMediaInfo, m_objContext.GetSupplementaryService().GetAll());

    return CallStateName::ESTABLISHED;
}

PRIVATE
void UpdatingState::RecoverModificationFailure()
{
    IMS_TRACE_D("RecoverModificationFailure", 0, 0, 0);

    m_objContext.GetMediaManager().RestoreSdp(&m_objContext.GetSession()->GetISession());

    FailReason objReason(FAIL_REASON_NONE);
    if (m_objContext.GetUpdatingInfo().IsHeld())
    {
        m_objContext.GetUiNotifier().SendHoldFailed(objReason);
    }
    else if (m_objContext.GetUpdatingInfo().IsResumed())
    {
        m_objContext.GetUiNotifier().SendResumeFailed(objReason);
    }
    else
    {
        m_objContext.GetUiNotifier().SendUpdateFailed(objReason);
    }
}

PRIVATE
void UpdatingState::NotifyIncomingUpdate()
{
    IMS_TRACE_D("NotifyIncomingUpdate", 0, 0, 0);

    CallType eCallType = CallType::VOIP;
    // TODO, update CallType
    CallInfo& objInfo = m_objContext.GetCallInfo();
    objInfo.eCallType = eCallType;

    m_objContext.GetUiNotifier().SendIncomingUpdate(&objInfo,
            &m_objContext.GetUpdatingInfo().GetAlertingInfo(),
            m_objContext.GetSupplementaryService().GetAll());
}
