#include "configuration/ConfigDef.h"
#include "call/IMtcCallContext.h"
#include "media/IMtcMediaManager.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "call/state/MtcCallState.h"
#include "MtcDef.h"
#include "helper/MtcTimerWrapper.h"
#include "call/MtcUiNotifier.h"
#include "call/MtcSession.h"
#include "precondition/QosDef.h"
#include "call/UpdatingInfo.h"
#include "helper/MtcSupplementaryService.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcCallState::MtcCallState(IN CallStateName eStateName, IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_eStateName(eStateName)
{
}

PUBLIC VIRTUAL
MtcCallState::~MtcCallState()
{
}

PUBLIC VIRTUAL
void MtcCallState::OnEnter()
{
}

PUBLIC VIRTUAL
void MtcCallState::OnExit()
{
}

PUBLIC VIRTUAL
CallStateName MtcCallState::HandleIncoming(
        IN ISession* /* piSession */,
        IN JniMtcServiceThread* /*pServiceThread*/)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Start(IN CallType /* eCallType */,
        IN CONST AString& /* strTarget */,
        IN MediaInfo* /* pMediaInfo */,
        IN const IMSMap<IMS_UINT32, SuppService*>& /* lstSuppServices */,
        IN JniMediaSessionThread* /* pJniMediaThread */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::HandleUserAlert()
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Accept(IN CallType /* eCallType */, IN MediaInfo* /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Reject(IN const FailReason& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Hold(IN MediaInfo* /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Resume(IN MediaInfo* /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::AcceptResume(
        IN CallType /* eCallType */, IN MediaInfo* /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::RejectResume(IN const FailReason& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Convert(
        IN CallType /* eCallType */, IN MediaInfo* /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::AcceptConvert(
        IN CallType /* eCallType */, IN MediaInfo* /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::RejectConvert(IN const FailReason& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::CancelConvert(IN const FailReason& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::Terminate(IN const FailReason& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SendDtmf(
        IN CONST AString& /* strSignal */, IN IMS_SINT32 /* nDuration */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SendUssi(IN CONST AString& /* strUssi */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::StartConference(
        IN CallType /* eCallType */,
        IN const AString&,
        IN MediaInfo* /* pMediaInfo */,
        IN const IMSMap<IMS_UINT32, SuppService*>& /* lstSuppServices */,
        IN IMSList<ConfUser*> /* lstUsers */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::StartConference(
        IN CallType /* eCallType */,
        IN const AString& /* strTarget */,
        IN IMSList<ConfUser*> /* lstUsers */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::ExpandToConference(IN CallInfo* /* pCallInfo */,
        IN IMSList<ConfUser*> /* lstUsers */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::MergeToConference(
        IN CallType /* eCallType */, IN CallInfo* /* pCallInfo */,
        IN IMSList<ConfUser*> /* lstUsers */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::HandleSrvccSuccess()
{
    return TransitToTerminating(FailReason(FAIL_REASON_SESSION_SRVCC));
}

PUBLIC VIRTUAL
CallStateName MtcCallState::HandleSrvccFailure(IN UpdateType /* eUpdateType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionAlerting(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionReferenceReceived(
        IN ISession* /* piSession */, IN IReference* /* piReference */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionStarted(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionStartFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionTerminated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionUpdated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionUpdateFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionUpdateReceived(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionCancelDelivered(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionCancelDeliveryFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionEarlyMediaUpdated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionEarlyMediaUpdateFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionEarlyMediaUpdateReceived(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionForkedResponseReceived(
        IN ISession* /* piSession */, IN ISession* /* piForkedSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionPRAckDelivered(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionPRAckDeliveryFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionPRAckReceived(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionProvisionalResponseReceived(
        IN ISession* /* piSession */, IN IMS_UINT32 /* nIndex */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionRPRDeliveryFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionRPRReceived(
        IN ISession* /* piSession */, IN IMS_UINT32 /* nIndex */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::SessionTransactionReceived(
        IN ISession* /* piSession */, IN ISIPServerConnection* /* piSipServerConnection */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::MessageMediator_AdjustMessage(
            IN_OUT ISIPMessage* /*piSipMessage*/,
            IN IMS_SINT32 /*nMessage = IMessageMediator::MESSAGE_NORMAL*/)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::OnTimerExpired(IN IMS_SINT32 /* nType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::OnBlockChecked(IN IMtcBlockChecker::Result /* objResult */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::QosReserved(
        IN ISession* /* piSession */, IN IMS_UINT32 /* eMediaType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::QosReserveFailed(
        IN ISession* /* piSession */, IN QosLossPolicy /* eNextAction */)
{
    return GetStateName();
}

PUBLIC VIRTUAL
CallStateName MtcCallState::OnInternalFailure()
{
    return CallStateName::TERMINATING;
}

PROTECTED
void MtcCallState::HandleTerminate(IN const FailReason& objReason)
{
    m_objContext.GetMediaManager().Terminate();

    MtcSession* pSession = m_objContext.GetSession();
    if (pSession == IMS_NULL)
    {
        return;
    }

    pSession->GetMessageSender().Terminate(IMS_TRUE, objReason);
}

PROTECTED
IMS_SINT32 MtcCallState::ConvertTerminateReasonToFailReason(IN IMS_SINT32 eReason)
{
    IMS_SINT32 eFailReason = FAIL_REASON_NONE;

    switch (eReason)
    {
        case IuMtcCall::TERMINATE_REASON_NORMAL:
            eFailReason = FAIL_REASON_SESSION_TERMINATED;
            break;
        case IuMtcCall::TERMINATE_REASON_USER:
            eFailReason = FAIL_REASON_SESSION_USERTERMINATE;
            break;
        case IuMtcCall::TERMINATE_REASON_LOW_BATTERY:
            eFailReason = FAIL_REASON_SERVICE_LOWBATTERY;
            break;
        case IuMtcCall::TERMINATE_REASON_POWER_OFF:
            eFailReason = FAIL_REASON_SERVICE_POWEROFF;
            break;
        case IuMtcCall::TERMINATE_REASON_VCC:
            eFailReason = FAIL_REASON_SESSION_SRVCC;
            break;

        default:
            eFailReason = eReason;
            break;
    }

    IMS_TRACE_I("ConvertTerminateReasonToFailReason : [%d]->[%s]", eReason,
            PS_FailReason(eFailReason), 0);
    return eFailReason;
}

PROTECTED
CallStateName MtcCallState::TransitToTerminating(IN const FailReason& objReason)
{
    switch (GetStateName())
    {
    case CallStateName::IDLE:
    case CallStateName::OUTGOING:
    case CallStateName::INCOMING:
    case CallStateName::ALERTING:
        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        break;
    case CallStateName::ESTABLISHED:
    case CallStateName::UPDATING:
        m_objContext.GetUiNotifier().SendTerminated(objReason);
        break;
    case CallStateName::TERMINATING:
        break;
    }

    return CallStateName::TERMINATING;
}

PROTECTED
void MtcCallState::NotifyHoldResumeState()
{
    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    if (m_objContext.GetUpdatingInfo().IsHeld())
    {
        m_objContext.SetHeldByMe(IMS_TRUE);
        m_objContext.GetUiNotifier().SendHeld(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetAll());
    }
    else if (m_objContext.GetUpdatingInfo().IsResumed())
    {
        m_objContext.SetHeldByMe(IMS_FALSE);
        m_objContext.GetUiNotifier().SendResumed(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetAll());
    }

    if (m_objContext.GetUpdatingInfo().IsHeldBy())
    {
        m_objContext.GetUiNotifier().SendHeldBy(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetAll());
    }
    else if (m_objContext.GetUpdatingInfo().IsResumedBy())
    {
        m_objContext.GetUiNotifier().SendResumedBy(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetAll());
    }
}
