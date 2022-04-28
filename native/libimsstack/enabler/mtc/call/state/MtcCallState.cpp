#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/state/MtcCallState.h"
#include "call/UpdatingInfo.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "IMessage.h"
#include "ISIPHeader.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "media/IMtcMediaManager.h"
#include "MtcDef.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include "SIPStatusCode.h"
#include "utility/MessageUtil.h"

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
        IN const IMSMap<SuppType, SuppService*>& /* lstSuppServices */)
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
        IN const IMSMap<SuppType, SuppService*>& /* lstSuppServices */,
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
    m_objContext.GetUiNotifier().SendTerminated(objReason);
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
                m_objContext.GetSupplementaryService().GetServices());
    }
    else if (m_objContext.GetUpdatingInfo().IsResumed())
    {
        m_objContext.SetHeldByMe(IMS_FALSE);
        m_objContext.GetUiNotifier().SendResumed(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }

    if (m_objContext.GetUpdatingInfo().IsHeldBy())
    {
        m_objContext.GetUiNotifier().SendHeldBy(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }
    else if (m_objContext.GetUpdatingInfo().IsResumedBy())
    {
        m_objContext.GetUiNotifier().SendResumedBy(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }
}

PROTECTED
IMS_RESULT MtcCallState::CreateISession()
{
    ISession* piSession = m_objContext.GetSipInterfaceFactory().GetISessionHolder()->GetISession(
            m_objContext.GetService().GetICoreService(),
            m_objContext.GetParticipantInfo().GetLocalUri(),
            m_objContext.GetParticipantInfo().GetRemoteUri());
    if (piSession == IMS_NULL)
    {
        return IMS_FAILURE;
    }
    piSession->SetImplicitRoutingRequired(IMS_TRUE);
    m_objContext.SetSession(m_objContext.CreateSession(*piSession));

    return IMS_SUCCESS;
}

PROTECTED
ISession* MtcCallState::GetISession()
{
    return &m_objContext.GetSession()->GetISession();
}

PROTECTED
void MtcCallState::InitMediaSession(IN MediaInfo* pMediaInfo/* = IMS_NULL*/)
{
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (pMediaInfo)
    {
        objMediaManager.SetMediaInfo(*pMediaInfo);
    }
    if (m_objContext.GetCallInfo().ePeerType == PeerType::MO)
    {
        objMediaManager.CreateMediaSession(
                m_objContext.GetUiNotifier().GetJniMediaThread());
    }
    else
    {
        objMediaManager.CreateMediaSession(IMS_NULL);
    }

    objMediaManager.CreateMediaProfile(
            &m_objContext.GetSession()->GetISession(), IMS_FALSE, IMS_TRUE);
    objMediaManager.SetConferenceCall(m_objContext.GetCallInfo().bConference);
}

PROTECTED
IMS_SINT32 MtcCallState::OnSdpReceived(IN ISession* piSession, IN IMessage* piMessage)
{
    if (IMS_FALSE)
    {
        // TODO: IsInvalidOfferAnswer(); by RFC 6337
        return FAIL_REASON_MEDIA_NEGOFAIL;
    }

    if (MessageUtil::HasSdp(piMessage) == IMS_FALSE)
    {
        IMS_TRACE_D("OnSdpReceived - No SDP", 0, 0, 0);
        return FAIL_REASON_NONE;
    }

    if (m_objContext.GetMediaManager().NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        IMS_TRACE_D("OnSdpReceived - Nego SDP Failed", 0, 0, 0);
        // TODO: return fail reasone? IMS_RESULT? it's always NEGOFAIL?
        return FAIL_REASON_MEDIA_NEGOFAIL;
    }

    RunMedia(piSession, piMessage);

    m_objContext.GetPreconditionManager().UpdateQosAttributesFromSdp(piSession);

    IMS_TRACE_D("OnSdpReceived - Nego Done", 0, 0, 0);
    return FAIL_REASON_NONE;
}

PROTECTED
ResultSetSdp MtcCallState::SetSdpToSend(IN IMS_BOOL bAllowReOffer,
        IN ISession* piSession/* = IMS_NULL*/)
{
    // TODO: RFC 6337 instead of bAllowReOffer?

    if (piSession == IMS_NULL)
    {
        piSession = GetISession();
    }

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    // TODO: need to check sending offer not in STATE_NEGOTIATED?
    if (!bAllowReOffer &&
            objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED)
    {
        IMS_TRACE_D("SetSdpToSend - nothing to update", 0, 0, 0);
        return ResultSetSdp::NO_SDP;
    }

    if (objMediaManager.FormSdp(piSession, m_objContext.GetCallInfo().eCallType) == IMS_FAILURE)
    {
        IMS_TRACE_D("SetSdpToSend - Form SDP Failed", 0, 0, 0);
        return ResultSetSdp::FAILURE;
    }

    IMS_TRACE_D("SetSdpToSend - Set Done", 0, 0, 0);

    // TODO: bFailure to true for failure cases is not in this api?
    m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_FALSE);

    return ResultSetSdp::SUCCESS;
}

PROTECTED
void MtcCallState::RunMedia(IN ISession* piSession, IN IMessage* piMessage)
{
    IMS_BOOL bEarly = !MessageUtil::IsResponseExist(piSession, SIPStatusCode::SC_200);
    IMS_BOOL b180Received = m_objContext.GetCallInfo().ePeerType == PeerType::MO &&
            MessageUtil::IsResponseExist(piSession, SIPStatusCode::SC_180);

    m_objContext.GetMediaManager().Run(piSession, piMessage, bEarly, b180Received);
}

PROTECTED
IMS_RESULT MtcCallState::SendProvisionalResponse(IN IMS_BOOL bUserAlert)
{
    ResultSetSdp eSetSdpResult = SetSdpToSend(IMS_FALSE);
    if (eSetSdpResult == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    // TODO: determine the response code based on the configuration for KR carriers?
    IMS_SINT32 nStatusCode = bUserAlert ? SIPStatusCode::SC_180 : SIPStatusCode::SC_183;

    return m_objContext.GetSession()->GetMessageSender().SendProvisionalResponse(
            nStatusCode, IsRprSupported(), eSetSdpResult == ResultSetSdp::SUCCESS, IsCallWaiting());
}

PROTECTED
IMS_RESULT MtcCallState::SendEarlyUpdate(IN MtcSession* pMtcSession)
{
    IMS_TRACE_D("SendEarlyUpdate", 0, 0, 0);

    if (SetSdpToSend(IMS_TRUE, &pMtcSession->GetISession()) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    return pMtcSession->GetMessageSender()
            .SendEarlyUpdate(UpdateType::NORMAL);
}

PROTECTED
IMS_RESULT MtcCallState::SendResponseToEarlyUpdate(IN IMS_SINT32 eStatusCode,
        IN MtcSession* pMtcSession)
{
    IMS_TRACE_D("SendResponseToEarlyUpdate", 0, 0, 0);

    // TODO: check status code in SetSdpToSend()?
    if (SIPStatusCode::IsFinalSuccess(eStatusCode) &&
            SetSdpToSend(IMS_FALSE, &pMtcSession->GetISession()) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    return pMtcSession->GetMessageSender()
            .RespondToEarlyUpdate(eStatusCode);
}

PROTECTED
IMS_RESULT MtcCallState::SendResponseToPrack(IN IMS_SINT32 eStatusCode)
{
    IMS_TRACE_D("SendResponseToPrack", 0, 0, 0);
    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    return m_objContext.GetSession()->GetMessageSender()
            .RespondToPrack(eStatusCode);
}

PROTECTED
CallStateName MtcCallState::RejectAndToTerminating(IN IMS_SINT32 nFailReason)
{
    FailReason objReason(nFailReason);
    return RejectAndToTerminating(objReason);
}

PROTECTED
CallStateName MtcCallState::RejectAndToTerminating(IN const FailReason& objFailReason)
{
    m_objContext.GetSession()->GetMessageSender().Reject(objFailReason);
    return TransitToTerminating(objFailReason);
}

PROTECTED
void MtcCallState::SendIncomingCallReceived()
{
    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendIncomingCallReceived(
            m_objContext.GetCallKey(),
            m_objContext.GetCallInfo(),
            objMediaInfo,
            m_objContext.GetSupplementaryService().GetServices(),
            m_objContext.GetParticipantInfo());
}

PROTECTED
void MtcCallState::SendStarted()
{
    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendStarted(
            &m_objContext.GetCallInfo(),
            &objMediaInfo,
            m_objContext.GetSupplementaryService().GetServices());
}

PROTECTED
void MtcCallState::SendIncomingUpdate(IN CallType eCallType)
{
    IMS_TRACE_D("SendIncomingUpdate", 0, 0, 0);

    m_objContext.GetUpdatingInfo().SetAlerted();

    CallInfo& objInfo = m_objContext.GetCallInfo();
    objInfo.eCallType = eCallType;

    m_objContext.GetUiNotifier().SendIncomingUpdate(&objInfo,
            &m_objContext.GetUpdatingInfo().GetAlertingInfo(),
            m_objContext.GetSupplementaryService().GetServices());

    m_objContext.GetTimer().Start(TIMER_CONVERT_USER_RESPONSE,
            m_objContext.GetConfigurationProxy().GetInt(Feature::CONVERT_USER_RESPONSE_TIMER));
}

PROTECTED
void MtcCallState::UpdatePreconditionCapability(IN ISession* piSession, IN IMessage* piMessage,
        IN IMS_BOOL bCheckeSdp/* = IMS_TRUE*/)
{
    if (bCheckeSdp && !MessageUtil::HasSdp(piMessage))
    {
        return;
    }

    IMS_BOOL bRemoteCapability = IMS_FALSE;
    IMS_BOOL bHasSupportedHeader =
            MessageUtil::HasValue(piMessage, MessageUtil::STR_PRECONDITION, ISIPHeader::SUPPORTED);
    IMS_BOOL bHasRequireHeader =
            MessageUtil::HasValue(piMessage, MessageUtil::STR_PRECONDITION, ISIPHeader::REQUIRE);

    if ((!bCheckeSdp || SdpPreconditionHelper::IsPreconditionIncludedInSdp(piSession)) &&
            (bHasSupportedHeader || bHasRequireHeader))
    {
        bRemoteCapability = IMS_TRUE;
    }

    IMS_TRACE_D("UpdatePreconditionCapability : Precondition Capability on remote UE[%s]",
            _TRACE_B_(bRemoteCapability), 0, 0);

    m_objContext.GetPreconditionManager().UpdatePreconditionCapability(
            piSession, bRemoteCapability);
}

PROTECTED
void MtcCallState::SetLocalQosAvailableForWifiCalling(IN ISession* piSession)
{
    IMS_TRACE_D("SetLocalQosAvailableForWifiCalling", 0, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();

    if (!m_objContext.GetService().IsWlanIpCanType())
    {
        return;
    }

    objPreconditionManager.SetLocalResourceAvailable(piSession);
}

PROTECTED
IMS_RESULT MtcCallState::NegotiateExtension(IN MtcSession* pMtcSession, IN IMessage* piMessage,
        IN IMS_UINT32 eMethod)
{
    if (piMessage == IMS_NULL || piMessage->GetMessage() == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (piMessage->GetMessage()->GetType() == ISIPMessage::TYPE_RESPONSE)
    {
        pMtcSession->GetExtensionSet().HandleResponse(eMethod, *piMessage);
    }
    else
    {
        pMtcSession->GetExtensionSet().HandleRequest(eMethod, *piMessage);
    }

    if (!pMtcSession->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        return IMS_FAILURE;
    }
    return IMS_SUCCESS;
}

PROTECTED
IMS_BOOL MtcCallState::IsRprSupported() const
{
    return m_objContext.GetSession()->GetExtensionSet().IsAvailableOnBoth(
            MtcExtensionSet::OPTION_TAG_RPR);
}

PROTECTED
IMS_BOOL MtcCallState::IsCallWaiting() const
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
