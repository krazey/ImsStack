#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/EstablishedState.h"
#include "call/termination/TerminationHandler.h"
#include "call/UpdatingInfo.h"
#include "conferencecall/ConferenceManager.h"
#include "conferencecall/IConferenceController.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "IMessage.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ServiceTrace.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EstablishedState::EstablishedState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ESTABLISHED, objContext)
{
}

PUBLIC VIRTUAL EstablishedState::~EstablishedState() {}

PUBLIC VIRTUAL CallStateName EstablishedState::Hold(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Hold", 0, 0, 0);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
    if (objMediaInfo.eADir == DIRECTION_INACTIVE)
    {
        m_objContext.SetHeldByMe(IMS_TRUE);
        m_objContext.GetUiNotifier().SendHeld(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
        return GetStateName();
    }

    if (HandleUpdate(UpdateType::HOLD, m_objContext.GetSession()->GetCallType(), pMediaInfo) ==
            IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Resume(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Resume", 0, 0, 0);
    if (HandleUpdate(UpdateType::RESUME, m_objContext.GetSession()->GetCallType(), pMediaInfo) ==
            IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Convert(
        IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Convert", 0, 0, 0);
    if (HandleUpdate(UpdateType::SESSION, eCallType, pMediaInfo) == IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);
    FailReason objConvertedReason(objReason);
    objConvertedReason.nReason = ConvertTerminateReasonToFailReason(objReason.nReason);

    // SetTerminateCodeForInvitedSessionToConf

    HandleTerminate(objConvertedReason);
    m_objContext.GetUiNotifier().SendTerminated(objConvertedReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);
    m_objContext.GetMediaManager().Terminate();

    m_objContext.GetUiNotifier().SendTerminated(IsConferenceCallParticipant()
                    ? FailReason(FAIL_REASON_CONF_JOINED)
                    : TerminationHandler().Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SessionUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionUpdateReceived", 0, 0, 0);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_UPDATE);

    m_objContext.GetSession()->HandleResponse(IMessage::SESSION_UPDATE, *piMessage);
    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetNegotiatedInfo());

    // TODO, conference

    IMS_RESULT eResult = IMS_SUCCESS;
    CallStateName eStateName = CallStateName::UPDATING;

    if (MessageUtil::HasSdp(piMessage))
    {
        eResult = HandleReceivedUpdate(eStateName);
    }
    else
    {
        eResult = HandleReceivedUpdateWithoutOffer(eStateName);
    }

    if (eResult != IMS_SUCCESS)
    {
        // TODO
    }

    if (eStateName == CallStateName::ESTABLISHED)
    {
        NotifyHoldResumeState();
        m_objContext.DeleteUpdatingInfo();
    }

    return eStateName;
}

PRIVATE
IMS_RESULT EstablishedState::HandleUpdate(
        IN UpdateType eUpdateType, IN CallType /* eCallType */, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("HandleUpdate", 0, 0, 0);
    m_objContext.GetUpdatingInfo().SetModifier();
    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetNegotiatedInfo());
    m_objContext.GetMediaManager().SetMediaInfo(*pMediaInfo);

    MtcSession* pSession = m_objContext.GetSession();
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();

    if (objMediaManager.FormSdp(&(pSession->GetISession()), CallType::VOIP) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(
            &(pSession->GetISession()), IMS_FALSE);

    objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifyingInfo());

    if (pSession->GetMessageSender().Update(eUpdateType, IMS_FALSE) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetTimer().Start(TIMER_CONVERT_REMOTE_RESPONSE,
            m_objContext.GetConfigurationProxy().GetInt(Feature::CONVERT_REMOTE_RESPONSE_TIMER));

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT EstablishedState::HandleReceivedUpdate(OUT CallStateName& eStateName)
{
    IMS_TRACE_D("HandleReceivedUpdate", 0, 0, 0);
    ISession& objSession = m_objContext.GetSession()->GetISession();
    if (m_objContext.GetMediaManager().NegotiateSdp(&objSession) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetAlertingInfo());

    m_objContext.GetPreconditionManager().UpdateQosAttributesFromSdp(&objSession);

    eStateName = CallStateName::UPDATING;

    if (m_objContext.GetUpdatingInfo().IsNeedToAlert())
    {
        CallType eCallType = CallType::VOIP;
        // TODO, update CallType
        SendIncomingUpdate(eCallType);

        return IMS_SUCCESS;
    }

    IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        eStateName = CallStateName::ESTABLISHED;
    }

    if (FormAutoAccept(IMS_FALSE) == IMS_FAILURE ||
            m_objContext.GetSession()->GetMessageSender().AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT EstablishedState::HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName)
{
    IMS_TRACE_D("HandleReceivedUpdateWithoutOffer", 0, 0, 0);
    eStateName = CallStateName::UPDATING;
    IMessage* piMessage =
            m_objContext.GetSession()->GetISession().GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        eStateName = CallStateName::ESTABLISHED;
    }
    else
    {
        if (FormAutoAccept(IMS_TRUE) == IMS_FAILURE)
        {
            // TODO
        }
    }

    if (m_objContext.GetSession()->GetMessageSender().AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT EstablishedState::FormAutoAccept(IN IMS_BOOL bWithoutOffer)
{
    IMS_TRACE_D("FormAutoAccept", 0, 0, 0);
    MtcSession* pSession = m_objContext.GetSession();

    AdjustDirectionWithHeldByMe(bWithoutOffer);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    // m_objContext.GetCallInfo().eCallType
    if (objMediaManager.FormSdp(&(pSession->GetISession()), CallType::VOIP) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(
            &(pSession->GetISession()), IMS_FALSE);

    if (bWithoutOffer)
    {
        objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifyingInfo());
    }
    else
    {
        objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifiedInfo());
    }

    return IMS_SUCCESS;
}

PRIVATE
void EstablishedState::AdjustDirectionWithHeldByMe(IN IMS_BOOL bWithoutOffer)
{
    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    IMS_SINT32 eNewDir = objMediaInfo.eADir;
    if (bWithoutOffer)
    {
        eNewDir = DIRECTION_SEND_RECEIVE;
    }

    if (m_objContext.IsHeldByMe())
    {
        if (eNewDir == DIRECTION_SEND_RECEIVE)
        {
            eNewDir = DIRECTION_SEND;
        }
        else if (eNewDir == DIRECTION_RECEIVE)
        {
            eNewDir = DIRECTION_INACTIVE;
        }
    }

    objMediaInfo.eADir = eNewDir;

    // TODO
    /*
    if (objMediaInfo.eVDir != DIRECTION_INVALID)
    {
        objMediaInfo.eVDir = ???;
    }
    if (objMediaInfo.eTDir != DIRECTION_INVALID)
    {
        objMediaInfo.eTDir = eNewDir;
    }
    */

    m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);
}

PRIVATE
IMS_BOOL EstablishedState::IsConferenceCallParticipant()
{
    IMSList<IMtcCall*> objConfCalls = m_objContext.GetCallManager().GetCallsInConference();
    if (objConfCalls.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    IConferenceController* piConfController =
            m_objContext.GetConferenceManager().GetController(objConfCalls.GetAt(0)->GetKey());
    if (piConfController == IMS_NULL)
    {
        return IMS_FALSE;
    }

    CallKey nKey = m_objContext.GetCallKey();
    if (piConfController->GetCallStatusInConference(nKey) == IndividualCallState::JOINED ||
            piConfController->GetCallStatusInConference(nKey) == IndividualCallState::JOINING)
    {
        IMS_TRACE_I("IsConferenceCallParticipant [%" PFLS_x "] call is joining to conference call",
                nKey, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
