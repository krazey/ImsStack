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

#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IImsRadio.h"
#include "IMessage.h"
#include "IMtcCallController.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsAosParameter.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/SilentRedialHelper.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/OutgoingState.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/EmergencyStartErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/MultipleDialogHandler.h"
#include "helper/UdpKeepAliveSender.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "precondition/SdpPreconditionHelper.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
OutgoingState::OutgoingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::OUTGOING, objContext),
        m_pUdpKeepAliveSender(IMS_NULL),
        m_bWaitingRedial(IMS_FALSE)
{
}

PUBLIC VIRTUAL OutgoingState::~OutgoingState() {}

PUBLIC VIRTUAL void OutgoingState::OnExit()
{
    m_objContext.GetTimer().Stop(TIMER_RETRY_UPDATE);
    if (m_pUdpKeepAliveSender != IMS_NULL)
    {
        m_pUdpKeepAliveSender->Stop();
    }
}

PUBLIC VIRTUAL CallStateName OutgoingState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", _TRACE_CR_(objReason), 0, 0);

    CallReasonInfo objUpdatedReasonInfo = MayGetUpdatedReasonByResponseWaitTimeout(objReason.nCode);
    if (objUpdatedReasonInfo.nCode == CODE_NONE)
    {
        objUpdatedReasonInfo = objReason;
    }

    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        HandleCancel(&pSession->GetISession(), objUpdatedReasonInfo);
    }

    m_objContext.GetUiNotifier().SendStartFailed(objUpdatedReasonInfo);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::QosReserved(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("QosReserved : MediaType[%d]", eMediaType, 0, 0);

    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL || piMessage->GetStatusCode() != SipStatusCode::SC_200)
    {
        return GetStateName();
    }

    if (!IsNeedToSendLocalResourceConfirmation(piSession))
    {
        return GetStateName();
    }

    m_objContext.GetMediaManager().AdjustDirectionForLocalResourceConfirmation(
            m_objContext.GetSession(piSession)->GetCallType());

    if (SendEarlyUpdate(UpdateType::NORMAL, m_objContext.GetSession(piSession)) == IMS_FAILURE)
    {
        IMS_TRACE_D("QosReserved : Fail to send early UPDATE.", 0, 0, 0);

        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
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
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted", 0, 0, 0);
    IMessage* piMessage =
            m_objContext.GetMessageUtils().GetPreviousResponse(piSession, IMessage::SESSION_START);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    m_objContext.GetTimer().StopAll();
    pSession->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, *piMessage);
    m_objContext.GetSupplementaryService().UpdateTip(piMessage);
    m_objContext.GetSupplementaryService().UpdateSessionId(piMessage);

    if (m_objContext.GetCallInfo().bConference)
    {
        m_objContext.GetMediaManager().SetConferenceCall(IMS_TRUE);
    }

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        pSession->SendAck();
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);

        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (pSession->SendAck() == IMS_FAILURE)
    {
        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    StartEpsFallbackWatchdogIfNeeded(*piMessage);
    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_FALSE);
    OnStarted(*pSession);
    m_objContext.GetPreconditionManager().OnCallEstablished(piSession);

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);

    if (IsNeedToIgnoreStartFailure() || m_bWaitingRedial)
    {
        return GetStateName();
    }

    IMessage* piResponse =
            m_objContext.GetMessageUtils().GetPreviousResponse(piSession, IMessage::SESSION_START);
    CallReasonInfo objReason = StartErrorHandler(m_objContext, *piSession).Handle(piResponse);

    if (objReason.nCode == CODE_INTERNAL_REDIAL)
    {
        StopTimer(MtcCallState::TimerType::TIMER_MO_18X_WAIT);
        StopTimer(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);

        if (objReason.nExtraCode == EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF ||
                objReason.nExtraCode == EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF)
        {
            m_bWaitingRedial = IMS_TRUE;
            return GetStateName();
        }

        return HandleSilentRedial(piSession, objReason);
    }

    OnStartFailed(objReason, IMS_TRUE);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    CallReasonInfo objReason = TerminationHandler(m_objContext).Handle(*piSession);
    OnStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);

    m_objContext.GetSession(piSession)->HandleResponse(
            ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_TRUE);
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    CallReasonInfo objReason = EarlyUpdateErrorHandler(m_objContext).Handle(piResponse);
    if (objReason.nCode == CODE_INTERNAL_REDIAL)
    {
        if (objReason.nExtraCode == EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF)
        {
            m_bWaitingRedial = IMS_TRUE;
            return GetStateName();
        }

        return HandleSilentRedial(piSession, objReason);
    }

    if (objReason.nCode == CODE_SIP_REQUEST_PENDING)
    {
        m_objContext.GetMediaManager().FinalizeSdp(piSession);
        m_objContext.GetTimer().Start(TIMER_RETRY_UPDATE, objReason.nExtraCode);
        return GetStateName();
    }

    if (MultipleDialogHandler().OnDialogRequestFailed(m_objContext,
                *m_objContext.GetSession(piSession)) == MultipleDialogHandler::Result::HANDLED)
    {
        return GetStateName();
    }

    HandleCancel(piSession, objReason);
    OnStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleRequest(RequestType::EARLY_UPDATE, *piMessage);
    m_objContext.GetMediaManager().UpdatePemType(piSession, piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_488) == IMS_FAILURE)
        {
            CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
            HandleCancel(piSession, objReason);
            OnStartFailed(objReason);

            return CallStateName::TERMINATING;
        }
        return GetStateName();
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_TRUE);
    m_objContext.GetUiNotifier().SendProgressing();
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

    m_objContext.GetSipInterfaceFactory().GetISessionHolder().AddISession(
            m_objContext.GetCallKey(), piForkedSession);

    m_objContext.CreateSession(piForkedSession);
    m_objContext.GetMediaManager().CreateMediaProfile(piForkedSession, IMS_TRUE, IMS_TRUE);
    m_objContext.GetPreconditionManager().CreateQos(piForkedSession);

    MultipleDialogHandler().OnSessionForked(m_objContext, m_objContext.GetSession(piSession));

    // TODO: need any timer for the forked session?

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPrackDelivered(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPrackDelivered", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL)
    {
        return GetStateName();
    }
    IMtcSession* pSession = m_objContext.GetSession(piSession);
    pSession->HandleResponse(ResponseType::PRACK_RESPONSE, *piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    if (!IsNeedToSendLocalResourceConfirmation(piSession))
    {
        return GetStateName();
    }

    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START);

    if (nStatusCode == SipStatusCode::SC_183)
    {
        IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
        if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
        {
            return GetStateName();
        }

        objMediaManager.AdjustDirectionForLocalResourceConfirmation(pSession->GetCallType());

        if (SendEarlyUpdate(UpdateType::NORMAL, m_objContext.GetSession(piSession)) == IMS_FAILURE)
        {
            CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
            HandleCancel(piSession, objReason);
            OnStartFailed(objReason);

            return CallStateName::TERMINATING;
        }
    }
    else if (nStatusCode == SipStatusCode::SC_200)
    {
        // TODO: send update after sending ACK to 200 OK response.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPrackDeliveryFailed(IN ISession* piSession)
{
    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_IGNORE_PRACK_DELIVERY_FAILURE_BOOL))
    {
        IMS_TRACE_D("SessionPrackDeliveryFailed : Ignore", 0, 0, 0);
        return GetStateName();
    }

    if (MultipleDialogHandler().OnDialogRequestFailed(m_objContext,
                *m_objContext.GetSession(piSession)) == MultipleDialogHandler::Result::HANDLED)
    {
        return GetStateName();
    }

    // The case that a PRACK request is rejected with a 503 error rarely happens.
    // So, do not consider that case.
    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_PRACK);
    IMS_TRACE_D("SessionPrackDeliveryFailed statusCode[%d]", nStatusCode, 0, 0);

    CallReasonInfo objReason = CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK);
    if (nStatusCode != SipStatusCode::SC_INVALID)
    {
        objReason.nCode = CODE_SIP_METHOD_NOT_ALLOWED;  // TODO: convert response code?
    }
    HandleCancel(piSession, objReason);
    OnStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionProvisionalResponseReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionProvisionalResponseReceived", 0, 0, 0);
    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
    StopTimer(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    if (SipStatusCode::IsProvisional(nStatusCode))
    {
        StopTimer(TIMER_MO_18X_WAIT);
        m_objContext.GetPassiveTimerHolder().RemoveTimer(
                IPassiveTimerHolder::Type::REGISTRATION_TO_18X);
    }
    StartTimer(TIMER_MO_NOANSWER);

    // 100 Trying is not a reliable response so UdpKeepAliveSender is started
    // by receiving any first provisional response.
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()) && nIndex == 0)
    {
        m_pUdpKeepAliveSender.reset(m_objContext.CreateUdpKeepAliveSender());
        m_pUdpKeepAliveSender->Start();
    }

    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_START, nIndex);
    m_objContext.GetSession(piSession)->HandleResponse(
            ResponseType::PROVISIONAL_RESPONSE, *piMessage);

    AString strNotSupportedExtension;
    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(
                *piMessage, strNotSupportedExtension))
    {
        CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);
    m_objContext.GetSupplementaryService().UpdateSessionId(piMessage);

    // TODO: move to SessionAlerting
    if (nStatusCode == SipStatusCode::SC_199)
    {
        return GetStateName();
    }

    m_objContext.GetMediaManager().UpdatePemType(piSession, piMessage);

    // TODO: not to update precondition attributes?
    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_TRUE);
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionRprReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionRprReceived", 0, 0, 0);
    StopTimer(TIMER_MO_18X_WAIT);
    StopTimer(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    m_objContext.GetPassiveTimerHolder().RemoveTimer(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X);

    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_START, nIndex);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, *piMessage);

    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL) &&
            piMessage->GetStatusCode() == SipStatusCode::SC_183 &&
            m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        StopTimer(TIMER_MO_NOANSWER);
    }
    else
    {
        StartTimer(TIMER_MO_NOANSWER);
    }

    AString strNotSupportedExtension;
    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(
                *piMessage, strNotSupportedExtension))
    {
        CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);
    m_objContext.GetSupplementaryService().UpdateSessionId(piMessage);

    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
    // TODO: move to SessionAlerting
    if (nStatusCode == SipStatusCode::SC_199)
    {
        return GetStateName();
    }

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        if (MultipleDialogHandler().OnUnavailableDialogCreated(m_objContext,
                    *m_objContext.GetSession(piSession)) == MultipleDialogHandler::Result::HANDLED)
        {
            return GetStateName();
        }
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    if (objMediaManager.GetRemoteRtpPort(piSession, MEDIATYPE_AUDIO) == 0)
    {
        CallReasonInfo objReason(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (piSession->IsFinalResponseReceivedForInitialInviteRequest())
    {
        IMS_TRACE_E(0, "SessionRprReceived - Session already has final response.", 0, 0, 0);
    }
    else
    {
        IMS_BOOL bNeedToConfirm = IsNeedToSendLocalResourceConfirmation(piSession);
        if (bNeedToConfirm)
        {
            objMediaManager.AdjustDirectionForLocalResourceConfirmation(pSession->GetCallType());
        }

        if (pSession->SendPrack(bNeedToConfirm) == IMS_FAILURE)
        {
            CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
            HandleCancel(piSession, objReason);
            OnStartFailed(objReason);

            return CallStateName::TERMINATING;
        }
    }

    StartEpsFallbackWatchdogIfNeeded(*piMessage);
    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_TRUE);
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::UssiStarted(IN ISession* piSession)
{
    IMS_TRACE_D("UssiStarted", 0, 0, 0);
    return SessionStarted(piSession);
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingMediaDataStarted(
        IN IMS_UINT32 /*eMediaType*/, IN IMS_UINT32 /*eProtocolType*/)
{
    if (m_pUdpKeepAliveSender != IMS_NULL)
    {
        m_pUdpKeepAliveSender->Stop();
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneStarted()
{
    IMS_TRACE_I("OnReceivingNetworkToneStarted", 0, 0, 0);
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneFailed()
{
    IMS_TRACE_I("OnReceivingNetworkToneFailed", 0, 0, 0);
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);

    if (m_objContext.GetSession() == IMS_NULL)
    {
        return GetStateName();
    }
    ISession* piSession = &m_objContext.GetSession()->GetISession();
    HandleCancel(piSession, objReason);
    OnStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnIpcanChanged(IN IMS_UINT32 eIpcan)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [eIpcan](IMtcCallState* pState)
            {
                return pState->OnIpcanChanged(eIpcan);
            });
    return GetStateName();
}

PROTECTED VIRTUAL CallStateName OutgoingState::HandleAosConnected()
{
    IMS_TRACE_I("HandleAosConnected", 0, 0, 0);
    if (m_bWaitingRedial)
    {
        m_bWaitingRedial = IMS_FALSE;
        return HandleSilentRedial(&m_objContext.GetSession()->GetISession(),
                CallReasonInfo(CODE_INTERNAL_REDIAL,
                        m_objContext.GetCallInfo().IsEmergency()
                                ? EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF
                                : EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF));
    }

    if (m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoResponse())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
        return HandleSilentRedial(&m_objContext.GetSession()->GetISession(),
                CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_EPS_FALLBACK));
    }
    else if (m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoTrigger())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MO_18X_WAIT:
        {
            CallReasonInfo objReason(CODE_TIMEOUT_1XX_WAITING);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(objReason);
            return CallStateName::TERMINATING;
        }
        case TIMER_MO_NOANSWER:
        {
            CallReasonInfo objReason(CODE_TIMEOUT_NO_ANSWER);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(objReason);
            return CallStateName::TERMINATING;
        }
        case TIMER_RETRY_UPDATE:
            // TODO: Not considering that multiple early sessions are in glare condition.
            for (IMS_UINT32 i = 0; i < m_objContext.GetSessions().GetSize(); ++i)
            {
                IMtcSession* pSession = m_objContext.GetSessions().GetAt(i);
                if (pSession->GetOngoingUpdateType() != UpdateType::NONE)
                {
                    SendEarlyUpdate(pSession->GetOngoingUpdateType(), pSession);
                    break;
                }
            }
            return GetStateName();
        default:
            return GetStateName();
    }
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnConnectionFailed(IN
        [[maybe_unused]] IMS_UINT32 nFailureReason,
        IN [[maybe_unused]] IMS_UINT32 nWaitTimeMillis)
{
    if (m_objContext.GetMessageUtils().GetPreviousResponse(
                GetISession(), IMessage::SESSION_START) != IMS_NULL)
    {
        IMS_TRACE_I("OnConnectionFailed : INVITE was already received by the server.", 0, 0, 0);
        return GetStateName();
    }

    IMS_TRACE_E(0, "OnConnectionFailed", 0, 0, 0);

    if (EpsFallbackTrigger::ShouldTriggerByReasonInfo(m_objContext,
                ConvertConnectionFailureToCallReasonInfo(nFailureReason, nWaitTimeMillis)))
    {
        m_objContext.GetEpsFallbackTrigger().TriggerEpsFallback(
                EpsFallbackReason::NO_NETWORK_RESPONSE, IMS_TRUE);
        return GetStateName();
    }

    IMS_SINT32 eCode;
    IMS_SINT32 eExtraCode;
    if (m_objContext.GetService().IsCsfbAvailable())
    {
        eCode = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
        eExtraCode = EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
    }
    else
    {
        eCode = CODE_CALL_BARRED;
        eExtraCode = -1;
    }
    CallReasonInfo objReason(eCode, eExtraCode);
    HandleCancel(GetISession(), objReason);
    OnStartFailed(objReason);
    return CallStateName::TERMINATING;
}

PRIVATE
void OutgoingState::HandleCancel(IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleCancel", 0, 0, 0);
    StopTimer(MtcCallState::TimerType::TIMER_MO_18X_WAIT);
    StopTimer(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);

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
CallReasonInfo OutgoingState::MayGetUpdatedReasonByResponseWaitTimeout(IN IMS_SINT32 nReasonCode)
{
    if (nReasonCode != CODE_USER_TERMINATED)
    {
        return CallReasonInfo(CODE_NONE);
    }

    if (m_objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT) <= 0)
    {
        return CallReasonInfo(CODE_NONE);
    }

    if (m_objContext.GetTimer().IsActive(TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON))
    {
        return CallReasonInfo(CODE_NONE);
    }

    IMS_TRACE_D("MayGetUpdatedReasonByResponseWaitTimeout", 0, 0, 0);

    return CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);
}

PRIVATE
CallStateName OutgoingState::HandleSilentRedial(
        IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleSilentRedial", 0, 0, 0);

    if (objReason.nExtraCode == EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK)
    {
        m_objContext.GetEpsFallbackTrigger().TriggerEpsFallback(
                EpsFallbackReason::NO_NETWORK_RESPONSE, IMS_TRUE);
        return GetStateName();
    }

    IMS_RESULT nResult =
            m_objContext.GetCallController().GetRedialHelper(m_objContext, objReason).Redial();

    if (nResult == IMS_FAILURE)
    {
        CallReasonInfo objReasonToUi(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
        IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
                piSession, IMessage::SESSION_START);
        if (piResponse != IMS_NULL)
        {
            IMS_SINT32 nLastResponseCode = piResponse->GetStatusCode();
            if (nLastResponseCode == SipStatusCode::SC_488)
            {
                objReasonToUi.nCode = CODE_SIP_NOT_ACCEPTABLE;
                objReasonToUi.nExtraCode = nLastResponseCode;
            }
            else if (nLastResponseCode >= SipStatusCode::SC_300 &&
                    nLastResponseCode < SipStatusCode::SC_400)
            {
                objReasonToUi.nCode = CODE_SIP_REDIRECTED;
                objReasonToUi.nExtraCode = nLastResponseCode;
            }
        }
        OnStartFailed(objReasonToUi);
        return CallStateName::TERMINATING;
    }

    return CallStateName::IDLE;
}

PRIVATE
void OutgoingState::OnStarted(IN IMtcSession& objMtcSession)
{
    MultipleDialogHandler().OnStarted(m_objContext, objMtcSession);

    // TODO: stop call init timers

    m_objContext.GetUiNotifier().SendStarted();
}

PRIVATE
void OutgoingState::OnStartFailed(
        IN const CallReasonInfo& objReason, IN IMS_BOOL bReasonFromErrorHandler /* = IMS_FALSE*/)
{
    if (m_objContext.GetCallInfo().IsEmergency() && !bReasonFromErrorHandler)
    {
        const auto objMaybeOverriddenReason =
                EmergencyStartErrorHandler::MaybeGetOverriddenCallReasonInfo(
                        m_objContext.GetConfigurationProxy(), objReason);
        if (objMaybeOverriddenReason)
        {
            IMS_TRACE_I("OnStartFailed : Override CallReasonInfo for emergency call", 0, 0, 0);
            return m_objContext.GetUiNotifier().SendStartFailed(objMaybeOverriddenReason.value());
        }
    }

    m_objContext.GetUiNotifier().SendStartFailed(objReason);
}

PRIVATE
CallReasonInfo OutgoingState::ConvertConnectionFailureToCallReasonInfo(
        IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) const
{
    switch (nFailureReason)
    {
        case IImsRadio::REASON_RRC_REJECT:
            return CallReasonInfo(CODE_INTERNAL_RRC_REJECT, nWaitTimeMillis);
        case IImsRadio::REASON_ACCESS_DENIED:
            return CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED);
        case IImsRadio::REASON_INTERNAL_ERROR:
            return CallReasonInfo(CODE_RADIO_INTERNAL_ERROR);
        default:
            return CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE);
    }
}
