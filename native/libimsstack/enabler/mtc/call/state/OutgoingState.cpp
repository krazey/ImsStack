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

#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "ICoreService.h"
#include "dialogevent/IDialogEvent.h"
#include "call/IMtcCallContext.h"
#include "media/IMtcMediaManager.h"
#include "ImsAosParameter.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "utility/MessageUtil.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcSession.h"
#include "helper/MtcTimerWrapper.h"
#include "call/MtcUiNotifier.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/OutgoingState.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "helper/MtcSupplementaryService.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
OutgoingState::OutgoingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::OUTGOING, objContext),
        m_bRemoteAlerted(IMS_FALSE),
        m_nSilentRedialCount(0)
{
}

PUBLIC VIRTUAL OutgoingState::~OutgoingState() {}

PUBLIC VIRTUAL CallStateName OutgoingState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", _TRACE_CR_(objReason), 0, 0);

    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        HandleCancel(&pSession->GetISession(), objReason);
    }

    m_objContext.GetUiNotifier().SendStartFailed(objReason);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::HandleSrvccFailure(IN UpdateType eUpdateType)
{
    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession == IMS_NULL)
    {
        return GetStateName();
    }

    if (m_objContext.GetMediaManager().GetNegotiationState(&pSession->GetISession()) !=
            NegotiationState::STATE_NEGOTIATED)
    {
        return GetStateName();
    }

    pSession->SendEarlyUpdate(eUpdateType);

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::QosReserved(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("QosReserved : MediaType[%d]", eMediaType, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.HasPreconditionCapability(piSession))
    {
        IMS_TRACE_D("QosReserved : There's no capability for precondition.", 0, 0, 0);
        return GetStateName();
    }

    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        IMS_TRACE_D("QosReserved : Resources of all media are not reserved.", 0, 0, 0);
        return GetStateName();
    }

    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL || piMessage->GetStatusCode() != SipStatusCode::SC_200)
    {
        IMS_TRACE_D("QosReserved : There's no PRACK or response to PRACK.", 0, 0, 0);
        return GetStateName();
    }

    if (piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE) != IMS_NULL &&
            SdpPreconditionHelper::IsLocalResourceReservedInSdp(
                    piSession, IMessage::SESSION_EARLY_UPDATE))
    {
        IMS_TRACE_D("QosReserved : UE already send early UPDATE with activated QoS.", 0, 0, 0);
        return GetStateName();
    }

    if (m_objContext.GetSession(piSession)->SendEarlyUpdate(UpdateType::NORMAL) == IMS_FAILURE)
    {
        IMS_TRACE_D("QosReserved : Fail to send early UPDATE.", 0, 0, 0);
        // TODO: erroer handling.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_I("QosReserveFailed", 0, 0, 0);
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        CallReasonInfo objReason(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
        HandleCancel(piSession, objReason);

        // change the reason code for CSFB in this case. discuss if extra code is needed for csfb.
        objReason.nCode = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
        m_objContext.GetUiNotifier().SendStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted", 0, 0, 0);
    IMessage* piMessage = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    m_objContext.GetTimer().StopAll();
    m_objContext.GetSession(piSession)->HandleResponse(IMessage::SESSION_START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    if (m_objContext.GetCallInfo().bConference)
    {
        m_objContext.GetMediaManager().SetConferenceCall(IMS_TRUE);
    }

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        pSession->SendAck();
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);

        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    UpdatePreconditionCapability(piSession, piMessage);
    m_objContext.GetPreconditionManager().SetRemoteResourceAvailable(piSession);

    if (pSession->SendAck() == IMS_FAILURE)
    {
        CallReasonInfo objReason(CODE_SESSION_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    RunMedia(piSession, piMessage);
    OnStarted(piSession);

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);

    IMessage* piResponse = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START);
    CallReasonInfo objReason = StartErrorHandler(m_objContext).Handle(piResponse);

    if (objReason.nCode == CODE_RETRY_AFTER_INTERNALONLY)
    {
        return HandleSilentRetry(objReason);
    }

    OnStartFailed(piSession, objReason);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    CallReasonInfo objReason = TerminationHandler().Handle(*piSession);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_EARLY_UPDATE);

    m_objContext.GetSession(piSession)->HandleResponse(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    // update remote qos status
    UpdatePreconditionCapability(piSession, piMessage);
    RunMedia(piSession, piMessage);

    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    IMessage* piResponse =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_EARLY_UPDATE);
    CallReasonInfo objReason = EarlyUpdateErrorHandler().Handle(piResponse);

    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleRequest(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    // TODO: which operator requires this?
    // m_objContext.GetTimer().Start(TIMER_MO_NOANSWER, 60000);

    m_objContext.GetMediaManager().UpdatePemType(piSession, piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_488) == IMS_FAILURE)
        {
            CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
        return GetStateName();
    }

    UpdatePreconditionCapability(piSession, piMessage);  // TODO: not in AlertingState?

    if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        CallReasonInfo objReason(CODE_SESSION_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    RunMedia(piSession, piMessage);
    SendProgressing();  // TODO: enforce remote alert to false?
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionForkedResponseReceived(
        IN ISession* piSession, IN ISession* piForkedSession)
{
    IMS_TRACE_D("SessionForkedResponseReceived", 0, 0, 0);
    if (piSession == IMS_NULL || piForkedSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "Session is null", 0, 0, 0);
        return GetStateName();
    }

    m_objContext.GetSipInterfaceFactory().GetISessionHolder()->AddISession(piForkedSession);

    m_objContext.CreateSession(piForkedSession);  // TODO: Need HandleResponse?
    m_objContext.GetMediaManager().CreateMediaProfile(piForkedSession, IMS_TRUE, IMS_TRUE);
    m_objContext.GetPreconditionManager().CreateQos(piForkedSession);

    OnSessionForked(piSession);

    // TODO: need any timer for the forked session?

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPRAckDelivered(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckDelivered", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL)
    {
        return GetStateName();
    }
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    UpdatePreconditionCapability(piSession, piMessage);

    pSession->HandleResponse(IMessage::SESSION_PRACK, *piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();

    SetLocalQosAvailableForWifiCalling(piSession);

    if (!objPreconditionManager.HasPreconditionCapability(piSession))
    {
        return GetStateName();
    }

    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        return GetStateName();
    }

    if (piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE) &&
            SdpPreconditionHelper::IsLocalResourceReservedInSdp(
                    piSession, IMessage::SESSION_EARLY_UPDATE))
    {
        return GetStateName();
    }

    if (SdpPreconditionHelper::IsLocalResourceReservedInSdp(piSession, IMessage::SESSION_START))
    {
        return GetStateName();
    }

    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START);

    if (nStatusCode == SipStatusCode::SC_183)
    {
        IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
        if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
        {
            return GetStateName();
        }

        if (pSession->SendEarlyUpdate(UpdateType::NORMAL) == IMS_FAILURE)
        {
            CallReasonInfo objReason(CODE_SESSION_INTERNAL_ERROR);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
    }
    else if (nStatusCode == SipStatusCode::SC_200)
    {
        // TODO: send update after sending ACK to 200 OK response.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPRAckDeliveryFailed(IN ISession* piSession)
{
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_PRACK);
    IMS_TRACE_D("SessionPRAckDeliveryFailed statusCode[%d]", nStatusCode, 0, 0);
    CallReasonInfo objReason = CallReasonInfo(
            nStatusCode == SipStatusCode::SC_INVALID ? CODE_NETWORK_RESP_TIMEOUT : CODE_UNSPECIFIED,
            nStatusCode);
    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionProvisionalResponseReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionProvisionalResponseReceived", 0, 0, 0);
    StopTimer(TIMER_MO_1XX_WAIT);
    StartTimer(TIMER_MO_NOANSWER);

    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START, nIndex);
    m_objContext.GetSession(piSession)->HandleResponse(IMessage::SESSION_START, *piMessage);

    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    IMS_SINT32 nStatusCode =
            MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SipStatusCode::SC_180)
    {
        m_bRemoteAlerted = IMS_TRUE;
    }
    if (nStatusCode == SipStatusCode::SC_199)
    {
        return GetStateName();
    }

    m_objContext.GetMediaManager().UpdatePemType(piSession, piMessage);

    // TODO: not to update precondition attributes?
    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    if (nStatusCode == SipStatusCode::SC_180 /* && local precondition support? */)
    {
        m_objContext.GetPreconditionManager().SetRemoteResourceAvailable(piSession);
    }

    RunMedia(piSession, piMessage);
    // TODO: StartE911RingBackTimer(m_pSessInfo->eCallType);
    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionRPRReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionRPRReceived", 0, 0, 0);
    StopTimer(TIMER_MO_1XX_WAIT);

    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_START, nIndex);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleResponse(IMessage::SESSION_START, *piMessage);

    if (m_objContext.GetConfigurationProxy().Is(
            Feature::STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY) &&
            piMessage->GetStatusCode() == SipStatusCode::SC_183 && MessageUtil::HasSdp(piMessage))
    {
        StopTimer(TIMER_MO_NOANSWER);
    }
    else
    {
        StartTimer(TIMER_MO_NOANSWER);
    }

    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(*piMessage))
    {
        CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    IMS_SINT32 nStatusCode =
            MessageUtil::GetResponseStatusCode(piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SipStatusCode::SC_180)
    {
        m_bRemoteAlerted = IMS_TRUE;
    }
    if (nStatusCode == SipStatusCode::SC_199)
    {
        return GetStateName();
    }

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    UpdatePreconditionCapability(piSession, piMessage);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (nStatusCode == SipStatusCode::SC_180)
    {
        objPreconditionManager.SetRemoteResourceAvailable(piSession);
    }

    if (pSession->SendPrack() == IMS_FAILURE)
    {
        // TODO: If there is no ISession in ISession::STATE_ESTABLISHED state and
        // not piSession->IsFinalResponseReceivedForInitialInviteRequest())
        {
            CallReasonInfo objReason(CODE_UNSPECIFIED);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
    }

    if (objMediaManager.GetNegotiationState(piSession) == NegotiationState::STATE_NEGOTIATED &&
            !objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    RunMedia(piSession, piMessage);
    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::UssiStarted(IN ISession* piSession)
{
    IMS_TRACE_D("UssiStarted", 0, 0, 0);
    return SessionStarted(piSession);
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneStarted()
{
    IMS_TRACE_I("OnReceivingNetworkToneStarted", 0, 0, 0);
    // TODO: send progressing with updated media info.
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneFailed()
{
    IMS_TRACE_I("OnReceivingNetworkToneFailed", 0, 0, 0);
    // TODO: send progressing with updated media info.
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnMediaFailed(IN CallReasonInfo objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);

    if (m_objContext.GetSession() == IMS_NULL)
    {
        return GetStateName();
    }
    ISession* piSession = &m_objContext.GetSession()->GetISession();
    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC
CallStateName OutgoingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MO_1XX_WAIT:
        {
            // TODO: fail reason name.
            CallReasonInfo objReason(CODE_TIMEOUT_1XX_WAITING);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(GetISession(), objReason);
            return CallStateName::TERMINATING;
        }
        case TIMER_MO_NOANSWER:
        {
            // TODO: fail reason name.
            CallReasonInfo objReason(CODE_TIMEOUT_NO_ANSWER);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(GetISession(), objReason);
            return CallStateName::TERMINATING;
        }
        case TimerType::TIMER_RETRY_AFTER:
            return ContinueSilentRetry();
        default:
            return GetStateName();
    }
}

PRIVATE
void OutgoingState::HandleCancel(IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleCancel", 0, 0, 0);
    StopTimer(MtcCallState::TimerType::TIMER_MO_1XX_WAIT);

    if (objReason.nCode == CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)
    {
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
    }

    IMtcSession* pSession = m_objContext.GetSession(piSession);
    if (pSession != IMS_NULL)
    {
        pSession->Terminate(IMS_FALSE, objReason);
    }
}

PRIVATE
CallStateName OutgoingState::HandleSilentRetry(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleSilentRetry", 0, 0, 0);

    if (m_nSilentRedialCount >=
            m_objContext.GetConfigurationProxy().GetInt(Feature::SILENT_REDIAL_MAX_RETRY_COUNT))
    {
        IMS_TRACE_D("HandleRetrySilent : Max retry count[%d] reached", m_nSilentRedialCount, 0, 0);
        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        // TODO: Trigger initial registeration if requird (by config?)
        return CallStateName::TERMINATING;
    }
    m_nSilentRedialCount += 1;

    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        m_objContext.RemoveSession(&pSession->GetISession());
    }

    m_objContext.GetMediaManager().Terminate();

    /* TODO: Policy: retry timer */
    IMS_SINT32 nRetryAfterSecond = objReason.nExtraCode;
    if (nRetryAfterSecond <= 0)
    {
        return ContinueSilentRetry();
    }
    else
    {
        m_objContext.GetTimer().Start(TimerType::TIMER_RETRY_AFTER, nRetryAfterSecond * 1000);
    }

    /* TODO: Policy: LTE
    m_objContext.GetAosConnector(m_objContext.GetService().GetServiceType())
            ->Control(ImsAosControl::FALLBACK_TO_LTE_AND_LET_ME_KNOW_IT);
    */

    return GetStateName();
}

PRIVATE
CallStateName OutgoingState::ContinueSilentRetry()
{
    IMS_TRACE_D("ContinueSilentRetry", 0, 0, 0);

    IMtcSession* pSession = m_objContext.CreateSession();
    if (pSession == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    InitMediaSession();
    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (pSession->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_UNSPECIFIED));
        return CallStateName::TERMINATING;
    }

    return GetStateName();
}

PRIVATE
void OutgoingState::HandleCountrySpecificServiceUrn(IN IMessage* piMessage)
{
    // If there is an alternative service URN in the Contact header of the 380 response,
    // it should be used to the subsequent emergency call.

    if ((piMessage->GetStatusCode() == SipStatusCode::SC_380) &&
            MessageUtil::IsHeaderPresent(piMessage, ISipHeader::CONTACT_NORMAL))
    {
        AString strServiceUrn;
        MessageUtil::GetHeader(piMessage, ISipHeader::CONTACT_NORMAL, strServiceUrn);

        if (strServiceUrn.StartsWith("urn:service:sos.country-specific"))
        {
            AString strNumber;
            MessageUtil::GetUserPart(piMessage, ISipHeader::TO, strNumber);
            m_objContext.GetDialingPlan().OnCountrySpecificServiceUrnReceived(
                    strNumber, strServiceUrn);
        }
    }
}

PRIVATE
void OutgoingState::SendProgressing()
{
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();

    MediaInfo objMediaInfo;
    objMediaManager.GetMediaInfo(objMediaInfo);

    m_objContext.GetUiNotifier().SendProgressing(&m_objContext.GetCallInfo(), &objMediaInfo,
            m_objContext.GetSupplementaryService().GetServices(), m_bRemoteAlerted);
}

PRIVATE
void OutgoingState::OnStarted(IN ISession* piSession)
{
    m_objContext.RemoveInactiveSessions(piSession);

    // TODO: stop call init timers

    SendStarted();
}

PRIVATE
void OutgoingState::OnStartFailed(IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    // TODO : need to modify this after emergency domain selection policy is decided.
    if (objReason.nCode == CODE_LOCAL_CALL_CS_RETRY_REQUIRED /*FAIL_REASON_SESSION_RETRY_R_RAT*/ &&
            objReason.nExtraCode == EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC)
    {
        HandleCountrySpecificServiceUrn(piSession->GetPreviousResponse(IMessage::SESSION_START));
    }

    m_objContext.GetUiNotifier().SendStartFailed(objReason);
}

PRIVATE
void OutgoingState::OnSessionForked(IN ISession* piOriginSession)
{
    if (m_objContext.GetConfigurationProxy().Is(
            Feature::MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING))
    {
        return;
    }

    IMtcSession* pOriginMtcSession = m_objContext.GetSession(piOriginSession);
    if (pOriginMtcSession == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("OnSessionForked : Terminate previous session", 0, 0, 0);

    pOriginMtcSession->Terminate(
            IMS_TRUE, CallReasonInfo(CODE_EARLYDIALOG_FORKED_TERMINATED_INTERNALONLY));

    m_objContext.GetMediaManager().DestroyMediaProfile(piOriginSession);
    m_objContext.RemoveSession(piOriginSession);
}
