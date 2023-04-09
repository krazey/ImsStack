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

#include "ICoreService.h"
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
#include "SipHeaderName.h"
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
#include "call/termination/StartErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/IMessage.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
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
        m_bRemoteAlerted(IMS_FALSE),
        m_bTimer100WaitExpired(IMS_FALSE),
        m_bWaitingRedialEmergency(IMS_FALSE)
{
}

PUBLIC VIRTUAL OutgoingState::~OutgoingState() {}

PUBLIC VIRTUAL void OutgoingState::OnExit()
{
    m_objContext.GetTimer().Stop(TIMER_GLARE_CONDITION);
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()))
    {
        m_objContext.GetUdpKeepAliveSender().Stop();
    }
}

PUBLIC VIRTUAL CallStateName OutgoingState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("Terminate : reason[%s]", _TRACE_CR_(objReason), 0, 0);

    IMtcSession* pSession = m_objContext.GetSession();
    if (HandleB1TimerAfterTerminate(pSession, objReason) == IMS_TRUE)
    {
        return CallStateName::TERMINATING;
    }

    if (pSession != IMS_NULL)
    {
        HandleCancel(&pSession->GetISession(), objReason);
    }

    m_objContext.GetUiNotifier().SendStartFailed(objReason);
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

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsEarlyUpdateRequired(piSession))
    {
        return GetStateName();
    }

    if (!objPreconditionManager.IsAvailableToSendEarlyUpdate(piSession))
    {
        return GetStateName();
    }

    if (SendEarlyUpdate(UpdateType::NORMAL, m_objContext.GetSession(piSession)) == IMS_FAILURE)
    {
        IMS_TRACE_D("QosReserved : Fail to send early UPDATE.", 0, 0, 0);

        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

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
        m_objContext.GetUiNotifier().SendStartFailed(objReason);

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
    m_objContext.GetSession(piSession)->HandleResponse(
            ResponseType::PROVISIONAL_RESPONSE, *piMessage);
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

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (pSession->SendAck() == IMS_FAILURE)
    {
        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    StartEpsFallbackWatchdogIfNeeded(*piMessage);
    RunMedia(piSession, piMessage);
    OnStarted(piSession);
    m_objContext.GetPreconditionManager().OnCallEstablished(piSession);

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);

    if (IsNeedToIgnoreStartFailure())
    {
        return GetStateName();
    }

    IMessage* piResponse =
            m_objContext.GetMessageUtils().GetPreviousResponse(piSession, IMessage::SESSION_START);
    CallReasonInfo objReason = StartErrorHandler(m_objContext).Handle(piResponse);

    if (objReason.nCode == CODE_INTERNAL_REDIAL)
    {
        if (objReason.nExtraCode == EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF)
        {
            m_bWaitingRedialEmergency = IMS_TRUE;
            return GetStateName();
        }

        return HandleSilentRedial(piSession, objReason);
    }

    OnStartFailed(piSession, objReason);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    CallReasonInfo objReason = TerminationHandler(m_objContext).Handle(*piSession);
    OnStartFailed(piSession, objReason);

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

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    RunMedia(piSession, piMessage);

    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    CallReasonInfo objReason = EarlyUpdateErrorHandler(m_objContext).Handle(piResponse);
    if (objReason.nCode == CODE_SIP_REQUEST_PENDING)
    {
        m_objContext.GetMediaManager().FinalizeSdp(piSession);
        m_objContext.GetTimer().Start(TIMER_GLARE_CONDITION, objReason.nExtraCode);
        return GetStateName();
    }
    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleRequest(RequestType::EARLY_UPDATE, *piMessage);

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

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
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
    pSession->HandleResponse(ResponseType::PRACK_RESPONSE, *piMessage);

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsEarlyUpdateRequired(piSession))
    {
        return GetStateName();
    }

    if (!objPreconditionManager.IsAvailableToSendEarlyUpdate(piSession))
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

        if (SendEarlyUpdate(UpdateType::NORMAL, m_objContext.GetSession(piSession)) == IMS_FAILURE)
        {
            CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
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
    if (m_objContext.GetConfigurationProxy().Is(Feature::IGNORE_PRACK_DELIVERY_FAILURE))
    {
        IMS_TRACE_D("SessionPRAckDeliveryFailed : Ignore", 0, 0, 0);
        return GetStateName();
    }

    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_PRACK);
    IMS_TRACE_D("SessionPRAckDeliveryFailed statusCode[%d]", nStatusCode, 0, 0);

    CallReasonInfo objReason = CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK);
    if (nStatusCode != SipStatusCode::SC_INVALID)
    {
        objReason.nCode = CODE_SIP_METHOD_NOT_ALLOWED;  // TODO: convert response code?
    }
    HandleCancel(piSession, objReason);
    OnStartFailed(piSession, objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionProvisionalResponseReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionProvisionalResponseReceived", 0, 0, 0);
    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
    StopTimer(TIMER_MO_100_WAIT);
    if (SipStatusCode::IsProvisional(nStatusCode))
    {
        StopTimer(TIMER_MO_18X_WAIT);
    }
    StartTimer(TIMER_MO_NOANSWER);

    // 100 Trying is not a reliable response so UdpKeepAliveSender is started
    // by receiving any first provisional response.
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()) && nIndex == 0)
    {
        m_objContext.GetUdpKeepAliveSender().Start();
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
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

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

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    RunMedia(piSession, piMessage);
    // TODO: StartE911RingBackTimer(m_pSessInfo->eCallType);
    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionRPRReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionRPRReceived", 0, 0, 0);
    StopTimer(TIMER_MO_100_WAIT);
    StopTimer(TIMER_MO_18X_WAIT);

    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_START, nIndex);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, *piMessage);

    if (m_objContext.GetConfigurationProxy().Is(
                Feature::STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY) &&
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
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetSupplementaryService().UpdateTip(piMessage);

    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
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

    if (objMediaManager.GetRemoteRtpPort(piSession, MEDIATYPE_AUDIO) == 0)
    {
        CallReasonInfo objReason(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
        HandleCancel(piSession, objReason);
        OnStartFailed(piSession, objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (pSession->SendPrack() == IMS_FAILURE)
    {
        // TODO: If there is no ISession in ISession::STATE_ESTABLISHED state and
        // not piSession->IsFinalResponseReceivedForInitialInviteRequest())
        {
            CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
            HandleCancel(piSession, objReason);
            OnStartFailed(piSession, objReason);

            return CallStateName::TERMINATING;
        }
    }

    StartEpsFallbackWatchdogIfNeeded(*piMessage);
    RunMedia(piSession, piMessage);
    SendProgressing();
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
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()))
    {
        m_objContext.GetUdpKeepAliveSender().Stop();
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneStarted()
{
    IMS_TRACE_I("OnReceivingNetworkToneStarted", 0, 0, 0);
    SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneFailed()
{
    IMS_TRACE_I("OnReceivingNetworkToneFailed", 0, 0, 0);
    SendProgressing();
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
    OnStartFailed(piSession, objReason);

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
    if (m_bWaitingRedialEmergency)
    {
        m_bWaitingRedialEmergency = IMS_FALSE;
        return HandleSilentRedial(&m_objContext.GetSession()->GetISession(),
                CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF));
    }

    if (EpsFallbackTrigger::IsRequired(m_objContext.GetConfigurationProxy()) &&
            m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoResponse())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
        return HandleSilentRedial(&m_objContext.GetSession()->GetISession(),
                CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT));
    }

    m_objContext.GetPreconditionManager().HandleQosOnIpcanChanged();
    return GetStateName();
}

PUBLIC
CallStateName OutgoingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MO_100_WAIT:
            // it's handled in TransactionTimerUpdateHelper
            m_bTimer100WaitExpired = IMS_TRUE;
            return GetStateName();
        case TIMER_MO_18X_WAIT:
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
        case TIMER_GLARE_CONDITION:
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

PRIVATE
void OutgoingState::HandleCancel(IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleCancel", 0, 0, 0);
    StopTimer(MtcCallState::TimerType::TIMER_MO_100_WAIT);
    StopTimer(MtcCallState::TimerType::TIMER_MO_18X_WAIT);

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
IMS_BOOL OutgoingState::HandleB1TimerAfterTerminate(
        IN IMtcSession* piMtcSession, IN const CallReasonInfo& objReason)
{
    if (objReason.nCode != CODE_USER_TERMINATED)
    {
        return IMS_FALSE;
    }

    if (m_bTimer100WaitExpired == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    Feature eFeature = m_objContext.GetService().IsWlanIpCanType()
            ? Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL
            : Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL;
    if (m_objContext.GetConfigurationProxy().GetInt(eFeature) !=
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("HandleB1TimerAfterTerminate", 0, 0, 0);

    // To invoke HandleTransactionTimeout() / ControlAos()
    StartErrorHandler(m_objContext).Handle(IMS_NULL);

    // To set Reason Header.
    const CallReasonInfo objNewReason(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);
    HandleCancel(&piMtcSession->GetISession(), objNewReason);
    m_objContext.GetUiNotifier().SendStartFailed(objNewReason);

    return IMS_TRUE;
}

PRIVATE
CallStateName OutgoingState::HandleSilentRedial(
        IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleSilentRedial", 0, 0, 0);

    if (objReason.nExtraCode == EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK)
    {
        m_objContext.GetEpsFallbackTrigger().TriggerEpsFallback(
                EpsFallbackReason::NO_NETWORK_RESPONSE);
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
        OnStartFailed(piSession, objReasonToUi);
        return CallStateName::TERMINATING;
    }

    return CallStateName::IDLE;
}

PRIVATE
void OutgoingState::HandleCountrySpecificServiceUrn(IN IMessage* piMessage)
{
    // If there is an alternative service URN in the Contact header of the 380 response,
    // it should be used to the subsequent emergency call.
    IMS_TRACE_D("HandleCountrySpecificServiceUrn", 0, 0, 0);

    if ((piMessage->GetStatusCode() == SipStatusCode::SC_380) &&
            m_objContext.GetMessageUtils().IsHeaderPresent(piMessage, ISipHeader::CONTACT_NORMAL))
    {
        AString strServiceUrn =
                m_objContext.GetMessageUtils().GetHeader(piMessage, ISipHeader::CONTACT_NORMAL);

        if (strServiceUrn.Contains("urn:service:sos.country-specific"))
        {
            AString strNumber =
                    m_objContext.GetMessageUtils().GetUserPart(piMessage, ISipHeader::TO);
            m_objContext.GetDialingPlan().OnCountrySpecificServiceUrnReceived(
                    strNumber, strServiceUrn);
        }
    }
}

PRIVATE
void OutgoingState::SendProgressing()
{
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();

    MediaInfo objMediaInfo = objMediaManager.GetMediaInfo();
    if (objMediaManager.IsLocalTone())
    {
        objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    }

    m_objContext.GetUiNotifier().SendProgressing(&m_objContext.GetCallInfo(), objMediaInfo,
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
    if (objReason.nCode == CODE_SIP_ALTERNATE_EMERGENCY_CALL &&
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
            IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED));

    m_objContext.GetMediaManager().DestroyMediaProfile(piOriginSession);
    m_objContext.RemoveSession(piOriginSession);
}
