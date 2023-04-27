/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IMessage.h"
#include "ISipClientConnection.h"
#include "ISipConnection.h"
#include "ISipServerConnection.h"
#include "ImsAosReason.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/UpdatingInfo.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/MtcCallState.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/CurrentLocationDiscoveryController.h"
#include "helper/IMtcAosConnector.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcCallState::MtcCallState(IN CallStateName eStateName, IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_eStateName(eStateName)
{
}

PUBLIC VIRTUAL MtcCallState::~MtcCallState() {}

PUBLIC VIRTUAL void MtcCallState::OnEnter() {}

PUBLIC VIRTUAL void MtcCallState::OnExit() {}

PUBLIC VIRTUAL CallStateName MtcCallState::HandleIncoming(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Start(IN CallType /* eCallType */,
        IN const AString& /* strTarget */, IN MediaInfo& /* pMediaInfo */,
        IN const ImsMap<SuppType, SuppService*>& /* lstSuppServices */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::HandleUserAlert()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Accept(
        IN CallType /* eCallType */, IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Reject(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Hold(IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Resume(IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::AcceptResume(
        IN CallType /* eCallType */, IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::RejectResume(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Update(
        IN CallType /* eCallType */, IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::AcceptUpdate(
        IN CallType /* eCallType */, IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::RejectUpdate(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::CancelUpdate(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Terminate(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::StartConference(IN CallType /* eCallType */,
        IN const AString&, IN MediaInfo& /* pMediaInfo */,
        IN const ImsMap<SuppType, SuppService*>& /* lstSuppServices */,
        IN const ImsList<ConfUser*>& /* lstUsers */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::StartConference(IN CallType /* eCallType */,
        IN const AString& /* strTarget */, IN const ImsList<ConfUser*>& /* lstUsers */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::HandleIncomingUssi(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnUssiAttached()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::UssiStarted(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::AcceptUssi(
        IN CallType /* eCallType */, IN MediaInfo& /* pMediaInfo */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::TerminateUssi(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::UssiTerminated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SendUssd(IN const AString& /* strUssd */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::UssiInfoReceived(
        IN ISession* /* piSession */, IN ISipServerConnection* /* piSipServerConnection */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::NotifyResponseToUssiInfo(
        IN ISipClientConnection* /* piScc */, IN ISipClientConnection* /* piForkedScc */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::NotifyErrorToUssiInfo(IN ISipConnection* /* piSc */,
        IN IMS_SINT32 /* nCode */, IN const AString& /* strMessage */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionAlerting(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionReferenceReceived(
        IN ISession* /* piSession */, IN IReference* /* piReference */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionStarted(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionStartFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionTerminated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionUpdated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionUpdateFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionUpdateReceived(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionCancelDelivered(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionCancelDeliveryFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionEarlyMediaUpdated(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionEarlyMediaUpdateFailed(
        IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionEarlyMediaUpdateReceived(
        IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionForkedResponseReceived(
        IN ISession* /* piSession */, IN ISession* /* piForkedSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionPRAckDelivered(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionPRAckDeliveryFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionPRAckReceived(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionProvisionalResponseReceived(
        IN ISession* /* piSession */, IN IMS_UINT32 /* nIndex */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionRPRDeliveryFailed(IN ISession* /* piSession */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionRPRReceived(
        IN ISession* /* piSession */, IN IMS_UINT32 /* nIndex */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::SessionTransactionReceived(
        IN ISession* /* piSession */, IN ISipServerConnection* piSipServerConnection)
{
    IMS_TRACE_I("SessionTransactionReceived", 0, 0, 0);

    if (CurrentLocationDiscoveryController::IsCurrentLocationDiscoveryInfoReceived(
            *piSipServerConnection))
    {
        m_objContext.GetCurrentLocationDiscoveryController().OnCurrentLocationDiscoveryInfoReceived(
                *piSipServerConnection);
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Refresh_NotifyCompleted(
        IN ISipClientConnection* /* piScc */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Refresh_NotifyTerminated()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Refresh_NotifyTimerExpired(
        OUT IMS_BOOL& bDoImplicitRefresh)
{
    bDoImplicitRefresh = IMS_TRUE;
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnTimerExpired(IN IMS_SINT32 /* nType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnBlockChecked(
        IN IMtcBlockChecker::Result /* objResult */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::QosReserved(
        IN ISession* /* piSession */, IN IMS_UINT32 /* eMediaType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::QosReserveFailed(
        IN ISession* /* piSession */, IN QosLossPolicy /* eNextAction */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnInternalFailure()
{
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnAttached()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* /*piForkedScc*/)
{
    if (piScc->Receive() != IMS_SUCCESS)
    {
        return GetStateName();
    }

    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    IMS_TRACE_D("ClientConnection_NotifyResponse : StatusCode[%d]", nStatusCode, 0, 0);

    if (SipStatusCode::IsFinal(nStatusCode))
    {
        piScc->Close();
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::Error_NotifyError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    IMS_TRACE_D("Error_NotifyError : Code[%d] Message[%s]", nCode, strMessage.GetStr(), 0);
    piSc->Close();

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnReceivingMediaDataStarted(
        IN IMS_UINT32 /*eMediaType*/, IN IMS_UINT32 /*eProtocolType*/)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnReceivingMediaDataFailed(
        IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType)
{
    IMS_TRACE_I(
            "OnReceivingMediaDataFailed : Media[%d] Protocol[%d]", eMediaType, eProtocolType, 0);

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnVideoLowestBitRate()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnReceivingNetworkToneStarted()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnReceivingNetworkToneFailed()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnMediaFailed(IN const CallReasonInfo& /* objReason */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnSrvccStateUpdated(IN SrvccState eState)
{
    m_objContext.GetMediaManager().SetSrvccState(eState);
    switch (eState)
    {
        case SrvccState::IDLE:
            return GetStateName();
        case SrvccState::STARTED:
            return HandleSrvccStarted();
        case SrvccState::SUCCEEDED:
        {
            const CallReasonInfo objReason(CODE_LOCAL_CALL_VCC_ON_PROGRESSING);
            return Terminate(objReason);
        }
        case SrvccState::FAILED:
            return SendUpdateBySrvcc(UpdateType::SRVCC_RECOVERED_FAILURE);
        default:  // SrvccState::CANCELED:
            return SendUpdateBySrvcc(UpdateType::SRVCC_RECOVERED_CANCEL);
    }
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnAosStateChanged(
        IN MtcAosState eState, IN IMS_UINT32 eAosReason)
{
    IMS_TRACE_I("OnAosStateChanged state[%d]", eState, 0, 0);
    switch (eState)
    {
        case MtcAosState::CONNECTED:
            return HandleAosConnected();
        case MtcAosState::DISCONNECTED:
        case MtcAosState::DISCONNECTING:
            return HandleAosDisconnected(eAosReason);
        default:  // case MtcAosState::SUSPENDED:
            return GetStateName();
    }
}

PUBLIC VIRTUAL CallStateName MtcCallState::OnIpcanChanged(IN IMS_UINT32 /*eIpcan*/)
{
    return GetStateName();
}

PROTECTED VIRTUAL CallStateName MtcCallState::SendUpdateBySrvcc(IN UpdateType eType)
{
    // Not checking the state because EstablishedState overrides this and UpdatingState will put
    // the operation into MtcPendingOperationHolder.
    for (IMS_UINT32 i = 0; i < m_objContext.GetSessions().GetSize(); ++i)
    {
        SendEarlyUpdate(eType, m_objContext.GetSessions().GetAt(i));
    }
    return GetStateName();
}

PROTECTED
CallStateName MtcCallState::HandleAosConnected()
{
    IMS_TRACE_I("HandleAosConnected", 0, 0, 0);
    m_objContext.GetPreconditionManager().HandleQosOnIpcanChanged();
    return GetStateName();
}

PROTECTED
CallStateName MtcCallState::HandleAosDisconnected(IN IMS_UINT32 eAosReason)
{
    if (m_objContext.GetService().GetSrvccState() == SrvccState::STARTED)
    {
        IMS_TRACE_I("HandleAosDisconnected ignore during srvcc", 0, 0, 0);
        return GetStateName();
    }

    if (m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoResponse() ||
            m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoTrigger())
    {
        IMS_TRACE_I("HandleAosDisconnected ignore during EPS Fallback", 0, 0, 0);
        return GetStateName();
    }

    if (m_objContext.GetConfigurationProxy().Is(
                Feature::REGISTRATION_DISCONNECT_REASON_TO_TERMINATE_ONGOING_CALL, eAosReason))
    {
        const CallReasonInfo objReason(GetCallReasonByAosReason(eAosReason));
        return Terminate(objReason);
    }
    return GetStateName();
}

PROTECTED
void MtcCallState::HandleTerminate(IN const CallReasonInfo& objReason) const
{
    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession == IMS_NULL)
    {
        return;
    }

    pSession->Terminate(IMS_TRUE, objReason);
}

PROTECTED
void MtcCallState::NotifyHoldResumeState()
{
    const MediaInfo& objMediaInfo = m_objContext.GetMediaManager().GetMediaInfo();

    if (m_objContext.GetUpdatingInfo().IsHeld())
    {
        m_objContext.SetHeldByMe(IMS_TRUE);
        m_objContext.GetUiNotifier().SendHeld(&(m_objContext.GetCallInfo()), objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }
    else if (m_objContext.GetUpdatingInfo().IsResumed())
    {
        m_objContext.SetHeldByMe(IMS_FALSE);
        m_objContext.GetUiNotifier().SendResumed(&(m_objContext.GetCallInfo()), objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }

    if (m_objContext.GetUpdatingInfo().IsHeldBy())
    {
        m_objContext.GetUiNotifier().SendHeldBy(&(m_objContext.GetCallInfo()), objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }
    else if (m_objContext.GetUpdatingInfo().IsResumedBy())
    {
        m_objContext.GetUiNotifier().SendResumedBy(&(m_objContext.GetCallInfo()), objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
    }
}

PROTECTED
ISession* MtcCallState::GetISession()
{
    return &m_objContext.GetSession()->GetISession();
}

PROTECTED
void MtcCallState::InitMediaSession()
{
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();

    objMediaManager.CreateMediaSession();

    objMediaManager.CreateMediaProfile(
            &m_objContext.GetSession()->GetISession(), IMS_FALSE, IMS_TRUE);
    objMediaManager.SetConferenceCall(m_objContext.GetCallInfo().bConference);

    if (objMediaManager.GetMediaInfo().eVideoQuality == VIDEO_QUALITY_NOTUSED)
    {
        // TODO: This will be verified and can be changed when Media Interface is ready.
        // Assumes VIDEO_QUALITY_NOTUSED used only in case of Call Pull
        objMediaManager.SetRtpPort(&m_objContext.GetSession()->GetISession(), MEDIATYPE_VIDEO, 0);
    }
}

PROTECTED
IMS_SINT32 MtcCallState::OnSdpReceived(IN ISession* piSession, IN IMessage* piMessage)
{
    if (m_objContext.GetMessageUtils().HasSdp(piMessage) == IMS_FALSE)
    {
        IMS_TRACE_D("OnSdpReceived - No SDP received.", 0, 0, 0);
        if (IsAnswerMandatory(piSession, piMessage))
        {
            IMS_TRACE_E(0, "Answer must be included.", 0, 0, 0);
            return CODE_MEDIA_NOT_ACCEPTABLE;
        }
        return CODE_NONE;
    }

    if (IsNeedToIgnore(piSession, piMessage) == IMS_TRUE)
    {
        return CODE_NONE;
    }

    if (IsInvalidOfferAnswer(piSession, piMessage) == IMS_TRUE)
    {
        return CODE_MEDIA_NOT_ACCEPTABLE;
    }

    if (m_objContext.GetMediaManager().NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        IMS_TRACE_D("OnSdpReceived - Nego SDP Failed", 0, 0, 0);
        // TODO: return fail reason? IMS_RESULT? it's always NEGOFAIL?
        return CODE_MEDIA_NOT_ACCEPTABLE;
    }

    m_objContext.GetPreconditionManager().OnSdpReceived(piSession, piMessage);

    IMS_TRACE_D("OnSdpReceived - Nego Done", 0, 0, 0);
    return CODE_NONE;
}

PROTECTED
void MtcCallState::RunMedia(IN ISession* piSession, IN IMessage* piMessage)
{
    IMS_BOOL bEarly =
            !m_objContext.GetMessageUtils().IsResponseExist(piSession, SipStatusCode::SC_200);
    m_objContext.GetMediaManager().Run(piSession, piMessage, bEarly);
}

PROTECTED
IMS_RESULT MtcCallState::SendEarlyUpdate(IN UpdateType eType, IN IMtcSession* piMtcSession)
{
    if (!piMtcSession)
    {
        return IMS_FAILURE;
    }

    IMS_TRACE_D("SendEarlyUpdate UpdateType[%d]", eType, 0, 0);
    ISession& objSession = piMtcSession->GetISession();
    if (m_objContext.GetMediaManager().GetNegotiationState(&objSession) ==
            NegotiationState::STATE_NEGOTIATED)
    {
        return piMtcSession->SendEarlyUpdate(eType);
    }
    return IMS_FAILURE;
}

PROTECTED
CallStateName MtcCallState::RejectIncomingAndToTerminating(IN const CallReasonInfo& objReason)
{
    if (objReason.nCode == CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED ||  // TODO: remove?
            objReason.nCode == CODE_REJECT_QOS_FAILURE)
    {
        m_objContext.GetPreconditionManager().FormPreconditionSdp(
                &m_objContext.GetSession()->GetISession(), IMS_TRUE);
    }

    m_objContext.GetSession()->Reject(objReason);
    m_objContext.GetUiNotifier().SendStartFailed(objReason);
    return CallStateName::TERMINATING;
}

PROTECTED
void MtcCallState::SendPreIncomingCallReceived()
{
    IMS_TRACE_D("SendPreIncomingCallReceived", 0, 0, 0);

    m_objContext.GetUiNotifier().SendPreIncomingCallReceived(m_objContext.GetCallKey());
}

PROTECTED
void MtcCallState::SendIncomingCallReceived()
{
    m_objContext.GetUiNotifier().SendIncomingCallReceived(m_objContext.GetCallKey(),
            m_objContext.GetCallInfo(), m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices(),
            m_objContext.GetParticipantInfo());
}

PROTECTED
void MtcCallState::SendStarted()
{
    m_objContext.GetUiNotifier().SendStarted(&m_objContext.GetCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
}

PROTECTED
void MtcCallState::SendIncomingUpdate(IN CallType eCallType)
{
    IMS_TRACE_D("SendIncomingUpdate", 0, 0, 0);

    m_objContext.GetUpdatingInfo().SetAlerted();

    m_objContext.GetUiNotifier().SendIncomingUpdate(eCallType, &m_objContext.GetCallInfo(),
            m_objContext.GetUpdatingInfo().GetAlertingInfo(),
            m_objContext.GetSupplementaryService().GetServices());

    m_objContext.GetTimer().Start(TIMER_CONVERT_USER_RESPONSE,
            m_objContext.GetConfigurationProxy().GetInt(Feature::CONVERT_USER_RESPONSE_TIMER));
}

PROTECTED
IMS_BOOL MtcCallState::IsRprSupported() const
{
    return m_objContext.GetSession()->GetExtensionSet().IsAvailableOnBoth(
            MtcExtensionSet::OPTION_TAG_RPR);
}

PROTECTED
IMS_BOOL MtcCallState::IsNeedToIgnore(IN ISession* piSession, IN const IMessage* piMessage) const
{
    if (IsPreviewOfAnswer(piSession, piMessage))
    {
        return IMS_TRUE;
    }

    NegotiationState eState = m_objContext.GetMediaManager().GetNegotiationState(piSession);
    if (eState != NegotiationState::STATE_NEGOTIATED)
    {
        return IMS_FALSE;
    }

    if (piMessage->GetMethod().Equals(SipMethod::INVITE))
    {
        if (piMessage->GetMessage()->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            IMS_SINT32 nConfig = piSession->GetConfiguration();
            if ((nConfig & ISession::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE) !=
                    ISession::CONFIG_NONE)
            {
                IMS_TRACE_I("IsNeedToIgnore - Ignore a subsequent OFFER in a response", 0, 0, 0);
                return IMS_TRUE;
            }

            IMS_TRACE_I("IsNeedToIgnore - Handle a subsequent OFFER in a response", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if (piMessage->GetMethod().Equals(SipMethod::ACK))
    {
        // TODO: isn't this invalid so need to terminate the call drop?
        IMS_TRACE_I("IsNeedToIgnore - Offer is included in ACK", 0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MtcCallState::IsInvalidOfferAnswer(
        IN ISession* piSession, IN const IMessage* piMessage) const
{
    NegotiationState eState = m_objContext.GetMediaManager().GetNegotiationState(piSession);
    if (eState == NegotiationState::STATE_OFFER_RECEIVED)
    {
        IMS_TRACE_E(0, "offer is received in STATE_OFFER_RECEIVED state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (eState == NegotiationState::STATE_NEGOTIATED)
    {
        if (piMessage->GetMethod().Equals(SipMethod::INVITE) == IMS_FALSE)
        {
            // Regarding INVITE case, IsNeedToIgnore() should control.
            if (piMessage->GetMessage()->GetType() == ISipMessage::TYPE_RESPONSE)
            {
                IMS_TRACE_E(0, "invalid Offer is received.", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL MtcCallState::IsPreviewOfAnswer(IN ISession* piSession, IN const IMessage* piMessage)
{
    if (piSession->IsSdpNegotiationAllowedForNonRpr())
    {
        return IMS_FALSE;
    }

    if (piMessage->GetMethod().Equals(SipMethod::INVITE) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (SipStatusCode::IsProvisional(piMessage->GetStatusCode()) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (piMessage->GetMessage()->IsMessageRpr())
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("IsPreviewOfAnswer it's a preview. wait the real Answer", 0, 0, 0);
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MtcCallState::IsAnswerMandatory(IN ISession* piSession, IN const IMessage* piMessage) const
{
    if (m_objContext.GetMediaManager().GetNegotiationState(piSession) !=
            NegotiationState::STATE_OFFER_SENT)
    {
        return IMS_FALSE;
    }

    const SipMethod& eMethod = piMessage->GetMethod();
    if (eMethod.Equals(SipMethod::INVITE))
    {
        if (piMessage->GetMessage()->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            if (piMessage->GetStatusCode() == SipStatusCode::SC_200)
            {
                // RFC 6337. Table 1. Case 1
                return IMS_TRUE;
            }
        }
        return IMS_FALSE;
    }

    if (eMethod.Equals(SipMethod::ACK))
    {
        // RFC 6337. Table 1. Case 2
        return IMS_TRUE;
    }

    if (eMethod.Equals(SipMethod::PRACK))
    {
        // RFC 6337. Table 1. Case 4, 5
        return IMS_TRUE;
    }

    if (eMethod.Equals(SipMethod::UPDATE))
    {
        if (piMessage->GetMessage()->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            // RFC 6337. Table 1. Case 6
            return IMS_TRUE;
        }
        return IMS_FALSE;
    }

    return IMS_FALSE;
}

PROTECTED
void MtcCallState::StartTimer(IN IMS_UINT32 nType) const
{
    m_objContext.GetTimer().Start(nType, GetTimeInMilliseconds(nType));
}

PROTECTED
void MtcCallState::StopTimer(IN IMS_UINT32 nType) const
{
    if (m_objContext.GetTimer().IsActive(nType))
    {
        m_objContext.GetTimer().Stop(nType);
    }
}

PROTECTED
IMS_SINT32 MtcCallState::GetTimeInMilliseconds(IN IMS_UINT32 nType) const
{
    IMS_BOOL bNormal = !m_objContext.GetCallInfo().bEmergency;
    Feature eFeature = Feature::UNKNOWN;
    switch (nType)
    {
        case TIMER_MO_100_WAIT:
            eFeature = bNormal ? Feature::MO_CALL_REQUEST_TIMEOUT : Feature::EMERGENCY_T_CALL_TIMER;
            break;
        case TIMER_MO_18X_WAIT:
            eFeature = bNormal ? Feature::TIMER_18X : Feature::EMERGENCY_18X_TIMER;
            break;
        case TIMER_MO_NOANSWER:
            eFeature = bNormal ? Feature::RINGBACK_TIMER : Feature::EMERGENCY_RINGBACK_TIMER;
            break;
        case TIMER_MT_ALERTING:
            eFeature = Feature::RINGING_TIMER;
            break;
        case TIMER_CONVERT_USER_RESPONSE:
            eFeature = Feature::CONVERT_USER_RESPONSE_TIMER;
            break;
        case TIMER_CONVERT_REMOTE_RESPONSE:
            eFeature = Feature::CONVERT_REMOTE_RESPONSE_TIMER;
            break;
        default:
            return -1;
    }

    return m_objContext.GetConfigurationProxy().GetInt(eFeature);
}

PROTECTED
void MtcCallState::SendInfoForUssi(
        IN const AString& strUssdString, IN UssiError eErrorCode /*= UssiError::CODE_NONE*/)
{
    IMS_TRACE_D("SendInfoForUssi", 0, 0, 0);
    ISipClientConnection* piConnection = m_objContext.CreateClientConnection(SipMethod::INFO);
    if (!piConnection)
    {
        return;
    }

    if (m_objContext.GetUssiController()->FormInfoRequest(
                piConnection, strUssdString, eErrorCode) == IMS_SUCCESS)
    {
        piConnection->Send();
    }
    else
    {
        piConnection->Close();
    }
}

PROTECTED
void MtcCallState::SendTransactionResponse(IN ISipServerConnection* piSipServerConnection,
        IN IMS_UINT32 nResponseCode, IN const AString& strPhrase /* = AString::ConstEmpty() */)
{
    IMS_TRACE_D("SendTransactionResponse", 0, 0, 0);
    if (!piSipServerConnection)
    {
        return;
    }

    if (strPhrase.GetLength() > 0)
    {
        piSipServerConnection->SetReasonPhrase(strPhrase);
    }

    piSipServerConnection->InitResponse(nResponseCode);
    piSipServerConnection->Send();
    piSipServerConnection->Close();
}

PROTECTED
IMS_BOOL MtcCallState::IsCallEndNeededByAudioInactivity(
        IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) const
{
    IMS_BOOL bNeedToEnd = IMS_FALSE;
    if (eMediaType != MEDIATYPE_AUDIO)
    {
        return bNeedToEnd;
    }

    IMS_SINT32 nAdditionalInfo = -1;
    if (m_objContext.GetMediaManager().IsOnHold())
    {
        if (eProtocolType == MEDIA_PROTOCOL_RTCP)
        {
            nAdditionalInfo = CarrierConfig::Ims::RTCP_INACTIVITY_ON_HOLD;
        }
    }
    else
    {
        if (m_objContext.GetService().IsEmergency())
        {
            if (eProtocolType == MEDIA_PROTOCOL_RTP)
            {
                nAdditionalInfo = CarrierConfig::Ims::E911_RTP_INACTIVITY_ON_CONNECTED;
            }
            else if (eProtocolType == MEDIA_PROTOCOL_RTCP)
            {
                nAdditionalInfo = CarrierConfig::Ims::E911_RTCP_INACTIVITY_ON_CONNECTED;
            }
        }
        else
        {
            if (eProtocolType == MEDIA_PROTOCOL_RTP)
            {
                nAdditionalInfo = CarrierConfig::Ims::RTP_INACTIVITY_ON_CONNECTED;
            }
            else if (eProtocolType == MEDIA_PROTOCOL_RTCP)
            {
                nAdditionalInfo = CarrierConfig::Ims::RTCP_INACTIVITY_ON_CONNECTED;
            }
        }
    }

    bNeedToEnd = (nAdditionalInfo < 0)
            ? IMS_FALSE
            : m_objContext.GetConfigurationProxy().Is(
                      Feature::AUDIO_INACTIVITY_CALL_END_REASON, nAdditionalInfo);
    IMS_TRACE_D("IsCallEndNeededByAudioInactivity : %s", _TRACE_B_(bNeedToEnd), 0, 0);

    return bNeedToEnd;
}

PROTECTED
CallReasonInfo MtcCallState::GetAudioInactivityReasonOnTermination(
        IN const CallReasonInfo& objReason)
{
    if (objReason.nCode != CODE_USER_TERMINATED)
    {
        return objReason;
    }

    if (m_objContext.GetMediaManager().IsAudioInactive() == IMS_FALSE)
    {
        return objReason;
    }

    IMS_TRACE_D("GetAudioInactivityReasonOnTermination", 0, 0, 0);

    return CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_RTP_TIMEOUT);
}

PROTECTED
IMS_BOOL MtcCallState::IsNeedToIgnoreStartFailure() const
{
    if (m_objContext.GetService().GetSrvccState() != SrvccState::STARTED)
    {
        return IMS_FALSE;
    }

    IMtcAosConnector* pConnector = m_objContext.GetService().GetAosConnector();
    if (pConnector == IMS_NULL || pConnector->IsImsConnected() == IMS_TRUE)
    {
        IMS_TRACE_D("IsNeedToIgnoreStartFailure invoked by remote so terminate", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("IsNeedToIgnoreStartFailure invoked by AoS disconnected so wait by SRVCC completed",
            0, 0, 0);
    return IMS_TRUE;
}

PROTECTED
void MtcCallState::StartEpsFallbackWatchdogIfNeeded(IN IMessage& objMessage) const
{
    if (objMessage.GetStatusCode() != SipStatusCode::SC_183 &&
            objMessage.GetStatusCode() != SipStatusCode::SC_200)
    {
        return;
    }

    if (m_objContext.GetMessageUtils().HasSdp(&objMessage) == IMS_FALSE)
    {
        return;
    }

    if (EpsFallbackTrigger::IsRequired(m_objContext.GetConfigurationProxy()) == IMS_FALSE)
    {
        return;
    }

    if (m_objContext.GetService().IsNr() == IMS_FALSE)
    {
        return;
    }

    m_objContext.GetEpsFallbackTrigger().StartWatchdog();
}

PROTECTED
IMS_SINT32 MtcCallState::GetCallReasonByAosReason(IN IMS_UINT32 nAosReason)
{
    switch (nAosReason)
    {
        case ImsAosReason::OUT_OF_SERVICE:
            return CODE_LOCAL_NETWORK_NO_SERVICE;
        case ImsAosReason::POWER_OFF:
            return CODE_LOCAL_POWER_OFF;
        case ImsAosReason::NO_RAT_COVERAGE:
            return CODE_LOCAL_NETWORK_NO_LTE_COVERAGE;
        case ImsAosReason::SERVICE_POLICY:
        case ImsAosReason::SERVICE_BLOCKED:
            return CODE_LOCAL_SERVICE_UNAVAILABLE;
        case ImsAosReason::DATA_DISCONNECTED:
            return CODE_LOCAL_NETWORK_NO_SERVICE;
        case ImsAosReason::REG_TERMINATED:
        case ImsAosReason::REG_NEW_REQUIRED:
            return CODE_LOCAL_NOT_REGISTERED;
        case ImsAosReason::SUSPEND_OUT_OF_SERVICE:
            return CODE_LOCAL_NETWORK_NO_SERVICE;
        case ImsAosReason::SUSPEND_NO_RAT_COVERAGE:
            return CODE_LOCAL_NETWORK_NO_LTE_COVERAGE;
        default:  // NOT_SPECIFIED
            return CODE_LOCAL_NOT_REGISTERED;
    }
}
