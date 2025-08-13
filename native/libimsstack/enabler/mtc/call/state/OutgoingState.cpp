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
#include "INetworkWatcher.h"
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
        m_pSilentRedialHelper(IMS_NULL),
        m_bWaitingServiceConnectedForRedial(IMS_FALSE),
        m_bMoResponseTimeoutForReasonTimerExpired(IMS_FALSE)
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

    m_objContext.GetPassiveTimerHolder().RemoveTimer(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X);
}

PUBLIC VIRTUAL CallStateName OutgoingState::Terminate(IN const CallReasonInfo& objReason)
{
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
        IN ISession* piSession, IN [[maybe_unused]] IMS_UINT32 eMediaType)
{
    const IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL || piMessage->GetStatusCode() != SipStatusCode::SC_200)
    {
        return GetStateName();
    }

    return MaySendPreconditionConfirmation(*piSession);
}

PUBLIC VIRTUAL CallStateName OutgoingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        CallReasonInfo objReason(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
        HandleCancel(piSession, objReason);

        // change the reason code for CSFB in this case. discuss if extra code is needed for csfb.
        if (m_objContext.IsCsfbAvailable())
        {
            objReason.nCode = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
        }
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionStarted(IN ISession* piSession)
{
    m_objContext.GetTimer().StopAll();

    IMessage* piMessage =
            m_objContext.GetMessageUtils().GetPreviousResponse(piSession, IMessage::SESSION_START);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleResponse(ResponseType::ACCEPT, *piMessage);
    m_objContext.GetSupplementaryService().UpdateTip(piMessage);
    m_objContext.GetSupplementaryService().UpdateSessionId(piMessage);

    if (m_objContext.GetCallInfo().bConference)
    {
        m_objContext.GetMediaManager().SetConferenceCall(IMS_TRUE);
    }

    if (HasNotRespondedQosConfirmation(*piSession))
    {
        // Once receiving a 200-INVITE, we assume that the remote QoS is already confirmed even if
        // the response for the UPDATE or the PRACK hasn't come. It won't be an issue when the
        // UPDATE was for the SRVCC, as well as for the QoS confirmation.
        IMS_TRACE_E(0, "No QoS confirmation from the remote, start anyway", 0, 0, 0);
        piSession->AbortEarlyUpdateTransaction();
        m_objContext.GetMediaManager().RestoreSdp(piSession);
    }
    else
    {
        IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
        if (eCallReason != CODE_NONE)
        {
            pSession->SendAck();
            CallReasonInfo objReason(eCallReason);
            HandleCancel(piSession, objReason);
            OnStartFailed(objReason);

            return CallStateName::TERMINATING;
        }
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
    if (IsNeedToIgnoreStartFailure() || m_bWaitingServiceConnectedForRedial)
    {
        return GetStateName();
    }

    const IMessage* piResponse =
            m_objContext.GetMessageUtils().GetPreviousResponse(piSession, IMessage::SESSION_START);
    CallReasonInfo objReason = StartErrorHandler(m_objContext, *piSession).Handle(piResponse);

    if (objReason.nCode == CODE_INTERNAL_REDIAL)
    {
        StopTimer(TIMER_MO_18X_WAIT);
        StopTimer(TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
        return HandleSilentRedialReason(objReason);
    }

    OnStartFailed(objReason, IMS_TRUE);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionTerminated(IN ISession* piSession)
{
    CallReasonInfo objReason = TerminationHandler(m_objContext).Handle(*piSession);
    OnStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);

    m_objContext.GetSession(piSession)->HandleResponse(
            ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.UpdatePemType(piSession, piMessage);

    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        CallReasonInfo objReason(eCallReason);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_TRUE);
    m_objContext.GetUiNotifier().SendProgressing();

    return MaySendPreconditionConfirmation(*piSession);
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    const IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    CallReasonInfo objReason = EarlyUpdateErrorHandler(m_objContext).Handle(piResponse);
    if (objReason.nCode == CODE_INTERNAL_REDIAL)
    {
        return HandleSilentRedialReason(objReason);
    }

    if (objReason.nCode == CODE_SIP_REQUEST_PENDING)
    {
        m_objContext.GetMediaManager().FinalizeSdp(piSession);
        m_objContext.GetTimer().Start(TIMER_RETRY_UPDATE, objReason.nExtraCode);
        return GetStateName();
    }

    if (objReason.nCode == CODE_INTERNAL_TERMINATE_EARLYDIALOG &&
            MultipleDialogHandler().OnDialogRequestFailed(m_objContext,
                    *m_objContext.GetSession(piSession)) == MultipleDialogHandler::Result::HANDLED)
    {
        return GetStateName();
    }

    HandleCancel(piSession, objReason.ConvertFromInternal());
    OnStartFailed(objReason.ConvertFromInternal());

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
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
    const IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL)
    {
        return GetStateName();
    }
    IMtcSession* pSession = m_objContext.GetSession(piSession);
    pSession->HandleResponse(ResponseType::PRACK_RESPONSE, *piMessage);

    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        CallReasonInfo objReason(eCallReason);
        HandleCancel(piSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START);

    if (nStatusCode == SipStatusCode::SC_183)
    {
        return MaySendPreconditionConfirmation(*piSession);
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::SessionPrackDeliveryFailed(IN ISession* piSession)
{
    IMS_SINT32 ePolicy = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT);
    IMS_TRACE_D("SessionPrackDeliveryFailed : Policy[%d]", ePolicy, 0, 0);

    if (ePolicy == ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_IGNORE)
    {
        return GetStateName();
    }

    if (ePolicy == ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_DIALOG &&
            MultipleDialogHandler().OnDialogRequestFailed(m_objContext,
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
    StopTimer(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);

    // to cover the case that "100 trying" is missed
    // starts UDP Keep-Alive when the first PR is received,
    if (nIndex == 0 && UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()))
    {
        m_pUdpKeepAliveSender.reset(m_objContext.CreateUdpKeepAliveSender());
        m_pUdpKeepAliveSender->Start();
    }

    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetResponseStatusCode(
            piSession, IMessage::SESSION_START, nIndex);
    if (nStatusCode == SipStatusCode::SC_100)
    {
        return On100TryingReceived();
    }

    StopTimer(TIMER_MO_18X_WAIT);
    StopTimer(TIMER_MO_CALL_INITIATION_TO_18X_WAIT);
    StartTimer(TIMER_MO_NOANSWER);
    m_objContext.GetPassiveTimerHolder().RemoveTimer(
            IPassiveTimerHolder::Type::REGISTRATION_TO_18X);

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
    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        CallReasonInfo objReason(eCallReason);
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
    StopTimer(TIMER_MO_18X_WAIT);
    StopTimer(TIMER_MO_CALL_INITIATION_TO_18X_WAIT);
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

    const IMS_BOOL bNeedToConfirm = m_objContext.GetConfigurationProxy().GetBoolean(
                                      ConfigVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL) &&
            IsNeedToSendLocalResourceConfirmation(piSession);
    if (!bNeedToConfirm)
    {
        // Assume that there is no case to send ANSWER in PRACK.
        // Therefore, when QoS Confirmation is unnecessary, MTC sends the PRACK first and then
        // proceed with internal processing.
        if (piSession->IsFinalResponseReceivedForInitialInviteRequest())
        {
            IMS_TRACE_E(0, "SessionRprReceived - Session already has final response.", 0, 0, 0);
        }
        else
        {
            if (pSession->SendPrack(IMS_FALSE) == IMS_FAILURE)
            {
                CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
                HandleCancel(piSession, objReason);
                OnStartFailed(objReason);

                return CallStateName::TERMINATING;
            }
        }
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

    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        if (MultipleDialogHandler().OnUnavailableDialogCreated(m_objContext,
                    *m_objContext.GetSession(piSession)) == MultipleDialogHandler::Result::HANDLED)
        {
            return GetStateName();
        }
        CallReasonInfo objReason(eCallReason);
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
        if (bNeedToConfirm)
        {
            objMediaManager.AdjustDirectionForLocalResourceConfirmation(pSession->GetCallType());

            if (pSession->SendPrack(IMS_TRUE) == IMS_FAILURE)
            {
                CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
                HandleCancel(piSession, objReason);
                OnStartFailed(objReason);

                return CallStateName::TERMINATING;
            }
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
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnReceivingNetworkToneFailed()
{
    m_objContext.GetUiNotifier().SendProgressing();
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
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

PUBLIC VIRTUAL CallStateName OutgoingState::OnRatChanged(
        IN IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType)
{
    if (eOldRatType == INetworkWatcher::RADIOTECH_TYPE_NR &&
            (eRatType == INetworkWatcher::RADIOTECH_TYPE_LTE ||
                    eRatType == INetworkWatcher::RADIOTECH_TYPE_LTE_CA))
    {
        IMS_TRACE_I("OnRatChanged : EPS-FB", 0, 0, 0);
        if (m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallback())
        {
            m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
            return PerformSilentRedial();
        }
    }

    return GetStateName();
}

PROTECTED VIRTUAL CallStateName OutgoingState::HandleAosConnected()
{
    StopTimer(TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL);

    if (m_bWaitingServiceConnectedForRedial)
    {
        m_bWaitingServiceConnectedForRedial = IMS_FALSE;
        return PerformSilentRedial();
    }

    if (m_objContext.GetEpsFallbackTrigger().IsWaitingRegistration())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
        return PerformSilentRedial();
    }

    return GetStateName();
}

PROTECTED VIRTUAL const CallReasonInfo OutgoingState::GetCallReasonInfoByAosDisconnection(
        IN IMS_UINT32 nAosReason, IN IMS_SINT32 nDataFailureReason) const
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }

    return MtcCallState::GetCallReasonInfoByAosDisconnection(nAosReason, nDataFailureReason);
}

PROTECTED VIRTUAL CallStateName OutgoingState::HandleAosDisconnectedByAllPcscfFailed()
{
    StopTimer(TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL);

    IMS_SINT32 eCode = (m_objContext.GetConfigurationProxy().GetBoolean(
                                ConfigVoice::KEY_CSFB_WHEN_ALL_PCSCF_UNAVAILABLE_BOOL) &&
                               m_objContext.IsCsfbAvailable())
            ? CODE_LOCAL_CALL_CS_RETRY_REQUIRED
            : CODE_LOCAL_NOT_REGISTERED;

    m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(eCode));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL:
            OnStartFailed(CallReasonInfo(CODE_SIP_SERVER_ERROR));
            return CallStateName::TERMINATING;
        case TIMER_MO_18X_WAIT:
        {
            CallReasonInfo objReason(CODE_TIMEOUT_1XX_WAITING);
            HandleCancel(GetISession(), objReason);
            OnStartFailed(objReason);
            return CallStateName::TERMINATING;
        }
        case TIMER_MO_CALL_INITIATION_TO_18X_WAIT:
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
        case TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON:
            m_bMoResponseTimeoutForReasonTimerExpired = IMS_TRUE;
            return GetStateName();
        default:
            return GetStateName();
    }
}

PUBLIC VIRTUAL CallStateName OutgoingState::OnConnectionFailed(
        IN IMS_UINT32 nFailureReason, IMS_UINT32 nWaitTimeMillis)
{
    if (m_objContext.GetMessageUtils().GetPreviousResponse(
                GetISession(), IMessage::SESSION_START) != IMS_NULL)
    {
        IMS_TRACE_I("OnConnectionFailed : INVITE was already received by the server.", 0, 0, 0);
        return GetStateName();
    }

    if (m_objContext.GetService().IsNr())
    {
        if (EpsFallbackTrigger::ShouldTriggerByReasonInfo(m_objContext,
                    ConvertConnectionFailureToCallReasonInfo(nFailureReason, nWaitTimeMillis)))
        {
            m_objContext.GetEpsFallbackTrigger().TriggerEpsFallback(
                    EpsFallbackReason::RADIO_CHECK_BLOCK);
            return GetStateName();
        }

        return GetStateName();
    }

    if (nFailureReason == IImsRadio::REASON_RRC_REJECT)
    {
        IMS_TRACE_D("OnConnectionFailed : Wait Transaction timeout for RRC reject in a LTE network",
                0, 0, 0);
        return GetStateName();
    }

    IMS_SINT32 eCode;
    IMS_SINT32 eExtraCode;
    if (m_objContext.IsCsfbAvailable())
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
CallStateName OutgoingState::On100TryingReceived()
{
    StartTimer(TIMER_MO_18X_WAIT);
    return GetStateName();
}

PRIVATE
void OutgoingState::HandleCancel(IN ISession* piSession, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleCancel", 0, 0, 0);
    StopTimer(TIMER_MO_18X_WAIT);
    StopTimer(TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);

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
CallStateName OutgoingState::MaySendPreconditionConfirmation(IN ISession& objSession)
{
    IMS_TRACE_D("MaySendPreconditionConfirmation", 0, 0, 0);
    if (m_objContext.GetMediaManager().GetNegotiationState(&objSession) !=
            NegotiationState::STATE_NEGOTIATED)
    {
        IMS_TRACE_I("NegotiationState is not STATE_NEGOTIATED.", 0, 0, 0);
        return GetStateName();
    }

    if (!IsNeedToSendLocalResourceConfirmation(&objSession))
    {
        return GetStateName();
    }

    m_objContext.GetMediaManager().AdjustDirectionForLocalResourceConfirmation(
            m_objContext.GetSession(&objSession)->GetCallType());

    if (SendEarlyUpdate(UpdateType::NORMAL, m_objContext.GetSession(&objSession)) == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "MaySendPreconditionConfirmation : Fail to send early UPDATE.", 0, 0, 0);

        CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
        HandleCancel(&objSession, objReason);
        OnStartFailed(objReason);

        return CallStateName::TERMINATING;
    }

    return GetStateName();
}

PRIVATE
CallReasonInfo OutgoingState::MayGetUpdatedReasonByResponseWaitTimeout(
        IN IMS_SINT32 nReasonCode) const
{
    if (nReasonCode != CODE_USER_TERMINATED)
    {
        return CallReasonInfo(CODE_NONE);
    }

    if (!m_bMoResponseTimeoutForReasonTimerExpired)
    {
        return CallReasonInfo(CODE_NONE);
    }

    IMS_TRACE_D("MayGetUpdatedReasonByResponseWaitTimeout", 0, 0, 0);

    IMtcSession* pSession = m_objContext.GetSession();
    if (pSession != IMS_NULL)
    {
        // only utilize the internal actions and ignore the return value
        // since this case is limited to cases where the user explicitly terminates the call,
        // as subsequent actions are limited
        StartErrorHandler(m_objContext, pSession->GetISession()).Handle(IMS_NULL);
    }

    return CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);
}

PRIVATE
CallStateName OutgoingState::HandleSilentRedialReason(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("HandleSilentRedialReason", 0, 0, 0);
    m_pSilentRedialHelper =
            &m_objContext.GetCallController().GetRedialHelper(m_objContext, objReason);

    switch (objReason.nExtraCode)
    {
        case EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF:
            m_bWaitingServiceConnectedForRedial = IMS_TRUE;
            m_objContext.GetTimer().Start(TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL,
                    m_objContext.GetConfigurationProxy().GetInt(
                            ConfigVoice::KEY_SILENT_REDIAL_REGISTRATION_WAIT_TIME_MILLIS_INT));
            break;
        case EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF:
        case EXTRA_CODE_REDIAL_EMERGENCY_WITH_ANONYMOUS:
            m_bWaitingServiceConnectedForRedial = IMS_TRUE;
            break;
        case EXTRA_CODE_REDIAL_BY_EPS_FALLBACK:
        case EXTRA_CODE_REDIAL_BY_EPS_FALLBACK_WITH_REG:
            break;
        case EXTRA_CODE_REDIAL_BY_RETRY_AFTER:
        {
            const IMS_SINT32 nRetryAfter = objReason.strExtraMessage.ToInt32();
            if (nRetryAfter < 0)
            {
                OnStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
                return CallStateName::TERMINATING;
            }
            return PerformSilentRedial(nRetryAfter == 0 ? DEFAULT_RETRY_AFTER : nRetryAfter);
        }
        default:
            return PerformSilentRedial();
    }

    return GetStateName();
}

PRIVATE
CallStateName OutgoingState::PerformSilentRedial(
        IN IMS_SINT32 nIntervalInMillis /* = INTERVAL_BY_TYPE*/)
{
    IMS_TRACE_D("PerformSilentRedial with interval[%d]", nIntervalInMillis, 0, 0);
    if (!m_pSilentRedialHelper)
    {
        return CallStateName::IDLE;
    }

    CallReasonInfo objResult = m_pSilentRedialHelper->Redial(nIntervalInMillis);
    if (objResult.nCode != CODE_NONE)
    {
        OnStartFailed(objResult);
        return CallStateName::TERMINATING;
    }

    return CallStateName::IDLE;
}

PRIVATE
IMS_BOOL OutgoingState::HasNotRespondedQosConfirmation(IN ISession& objISession) const
{
    // 3GPP TS 24.229 5.1.3.1 - NOTE 5
    if (m_objContext.GetMessageUtils().GetResponseStatusCode(
                &objISession, IMessage::SESSION_EARLY_UPDATE) == SipStatusCode::SC_INVALID)
    {
        const IMessage* piMessage = objISession.GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
        if (piMessage && m_objContext.GetMessageUtils().HasSdp(piMessage))
        {
            return IMS_TRUE;
        }
    }

    if (m_objContext.GetMessageUtils().GetResponseStatusCode(
                &objISession, IMessage::SESSION_PRACK) == SipStatusCode::SC_INVALID)
    {
        const IMessage* piMessage = objISession.GetPreviousRequest(IMessage::SESSION_PRACK);
        if (piMessage && m_objContext.GetMessageUtils().HasSdp(piMessage))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
void OutgoingState::OnStarted(IN const IMtcSession& objMtcSession)
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
