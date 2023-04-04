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
#include "ICarrierConfig.h"
#include "IMessage.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/UpdatingInfo.h"
#include "call/state/IMtcCallState.h"
#include "call/state/UpdatingState.h"
#include "call/termination/TerminationHandler.h"
#include "call/termination/UpdateErrorHandler.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/ISession.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "sipcore/SipStatusCode.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdatingState::UpdatingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::UPDATING, objContext)
{
    IMS_TRACE_D("+UpdatingState", 0, 0, 0);
}

PUBLIC VIRTUAL UpdatingState::~UpdatingState()
{
    IMS_TRACE_D("~UpdatingState", 0, 0, 0);
}

PUBLIC VIRTUAL void UpdatingState::OnExit()
{
    if (m_objContext.GetUpdatingInfo().HasPendingUpdate())
    {
        m_objContext.GetSession()->Update(UpdateType::REFRESH, IMS_FALSE, SipMethod::INVALID);
    }
    m_objContext.DeleteUpdatingInfo();
}

PUBLIC VIRTUAL CallStateName UpdatingState::Hold(IN MediaInfo& objMediaInfo)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [objMediaInfo](IMtcCallState* pState) mutable
            {
                return pState->Hold(objMediaInfo);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::Resume(IN MediaInfo& objMediaInfo)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [objMediaInfo](IMtcCallState* pState) mutable
            {
                return pState->Resume(objMediaInfo);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::Update(
        IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [=](IMtcCallState* pState) mutable
            {
                return pState->Update(eCallType, objMediaInfo);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::AcceptUpdate(
        IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_D("AcceptUpdate", 0, 0, 0);

    m_objContext.GetTimer().Stop(TIMER_CONVERT_USER_RESPONSE);

    IMtcSession* pSession = m_objContext.GetSession();
    ISession& objSession = pSession->GetISession();
    if (objSession.GetState() == ISession::STATE_ESTABLISHED)
    {
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
                m_objContext.GetMediaManager().GetMediaInfo(),
                m_objContext.GetSupplementaryService().GetServices());
        return CallStateName::ESTABLISHED;
    }
    m_objContext.GetUpdatingInfo().AdjustDirectionIfNeededForHoldOrResume(objMediaInfo);

    m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);
    pSession->SetCallType(eCallType);

    m_objContext.GetUpdatingInfo().GetModifiedInfo() =
            m_objContext.GetMediaManager().GetMediaInfo();

    if (pSession->AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
                m_objContext.GetUpdatingInfo().GetModifiedInfo(),
                m_objContext.GetSupplementaryService().GetServices());
        return CallStateName::ESTABLISHED;
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::RejectUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("RejectUpdate", 0, 0, 0);

    IMtcSession* pMtcSession = m_objContext.GetSession();
    // TODO: need to check ISession state?
    if (m_objContext.GetMediaManager().GetNegotiationState(&pMtcSession->GetISession()) !=
            NegotiationState::STATE_NEGOTIATED)
    {
        // TODO: Use this reject code in MessageFormatter#GetRejectStatusCode.
        IMS_SINT32 nRejectCode =
                ConfigService::GetConfigService()
                        ->GetCarrierConfig(m_objContext.GetSlotId())
                        ->GetInt(CarrierConfig::Assets::
                                        KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT);
        if (nRejectCode == SipStatusCode::SC_200)
        {
            return AcceptUpdate(pMtcSession->GetPreviousCallType(),
                    m_objContext.GetUpdatingInfo().GetNegotiatedInfo());
        }
    }

    m_objContext.GetTimer().Stop(TIMER_CONVERT_USER_RESPONSE);

    if (m_objContext.GetSession()->GetISession().GetState() == ISession::STATE_ESTABLISHED)
    {
        if (SendUpdate() == IMS_FAILURE)
        {
            // TODO
        }

        return CallStateName::UPDATING;
    }

    if (m_objContext.GetSession()->Reject(objReason) == IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::CancelUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("CancelUpdate", 0, 0, 0);

    m_objContext.GetTimer().Stop(TIMER_CONVERT_REMOTE_RESPONSE);

    if (m_objContext.GetSession()->CancelUpdate(objReason) == IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    StopTimer();

    const CallReasonInfo objTerminateReason = GetAudioInactivityReasonOnTermination(objReason);

    HandleTerminate(objTerminateReason);
    m_objContext.GetUiNotifier().SendTerminated(objTerminateReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    StopTimer();

    m_objContext.GetUiNotifier().SendTerminated(
            TerminationHandler(m_objContext).Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionUpdated", 0, 0, 0);

    StopTimer();

    if (HandleSdpAnswer() == IMS_FAILURE)
    {
        // TODO
    }

    if (SendAck() == IMS_FAILURE)
    {
        // TODO
    }

    if (m_objContext.GetUpdatingInfo().IsModifier() && m_objContext.GetUpdatingInfo().IsModified())
    {
        // TO make remote QoS sendrecv.
        m_objContext.GetPreconditionManager().OnMessageReceived(
                piSession, piSession->GetPreviousResponse(IMessage::SESSION_UPDATE));
    }

    return HandleModificationSucceeded();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionUpdateFailed", 0, 0, 0);

    StopTimer();

    IMessage* piResponse =
            m_objContext.GetMessageUtils().GetPreviousResponse(piSession, IMessage::SESSION_UPDATE);
    CallReasonInfo objReason = UpdateErrorHandler(m_objContext).Handle(piResponse);

    if (objReason.nCode == CODE_USER_TERMINATED_BY_REMOTE)
    {
        HandleTerminate(objReason);
        m_objContext.GetUiNotifier().SendTerminated(objReason);
        return CallStateName::TERMINATING;
    }
    else if (objReason.nCode == CODE_SIP_REQUEST_PENDING)
    {
        // Keep UpdatingState to block another outgoing request or pending operation
        // during this period. Also, UpdatingInfo is going to be deleted if transits to Established.
        // And, when a incoming request is received, transits to Established to handle the request.
        RecoverModificationFailure();
        m_objContext.GetTimer().Start(TIMER_GLARE_CONDITION, objReason.nExtraCode);
        return GetStateName();
    }

    RecoverModificationFailure();
    NotifyFailure();
    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionUpdateReceived(IN ISession* piSession)
{
    if (m_objContext.GetTimer().IsActive(TIMER_GLARE_CONDITION))
    {
        IMS_TRACE_I("SessionUpdateReceived during waiting glare condition timer", 0, 0, 0);

        NotifyFailure();
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [piSession](IMtcCallState* pState)
                {
                    return pState->SessionUpdateReceived(piSession);
                });
        return CallStateName::ESTABLISHED;
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionCancelDeliveryFailed(IN ISession*)
{
    // TODO: Add failure handle logic.
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);

    m_objContext.GetSession(piSession)->HandleResponse(
            ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        // TODO: Send CANCEL
        RecoverModificationFailure();
        NotifyFailure();
        return CallStateName::ESTABLISHED;
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionEarlyMediaUpdateFailed(IN ISession*)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);

    // TODO: Send CANCEL. Handle retry if 491.
    RecoverModificationFailure();
    NotifyFailure();
    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    IMtcSession* pMtcSession = m_objContext.GetSession();
    pMtcSession->HandleRequest(RequestType::EARLY_UPDATE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        if (pMtcSession->RespondToEarlyUpdate(SipStatusCode::SC_488) == IMS_FAILURE)
        {
            const CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
            RejectUpdate(objReason);
        }
        RecoverModificationFailure();
        return CallStateName::ESTABLISHED;
    }

    if (pMtcSession->RespondToEarlyUpdate(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        const CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
        RejectUpdate(objReason);
        RecoverModificationFailure();
        return GetStateName();
    }

    CheckPreconditionAndNotifyIncomingUpdate(piSession);
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionPRAckDelivered(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckDelivered", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
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

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (objMediaManager.GetNegotiationState(piSession) != NegotiationState::STATE_NEGOTIATED)
    {
        // TODO: need to check this?
        return GetStateName();
    }

    if (pSession->SendEarlyUpdate(UpdateType::NORMAL) == IMS_FAILURE)
    {
        // TODO: Send CANCEL
        RecoverModificationFailure();
        NotifyFailure();
        return CallStateName::ESTABLISHED;
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionPRAckDeliveryFailed(IN ISession*)
{
    // TODO: send CANCEL.
    RecoverModificationFailure();
    NotifyFailure();
    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    IMtcSession* pSession = m_objContext.GetSession(piSession);
    pSession->HandleRequest(RequestType::PRACK, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        pSession->RespondToPrack(SipStatusCode::SC_200);
        const CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        RejectUpdate(objReason);
        RecoverModificationFailure();
        return CallStateName::ESTABLISHED;
    }

    if (pSession->RespondToPrack(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        const CallReasonInfo objReason(CODE_LOCAL_INTERNAL_ERROR);
        RejectUpdate(objReason);
        RecoverModificationFailure();
        return CallStateName::ESTABLISHED;
    }

    CheckPreconditionAndNotifyIncomingUpdate(piSession);
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionRPRDeliveryFailed(IN ISession*)
{
    IMS_TRACE_D("SessionRPRDeliveryFailed", 0, 0, 0);

    // TODO: send CANCEL.
    RecoverModificationFailure();
    NotifyFailure();
    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionRPRReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_D("SessionRPRReceived", 0, 0, 0);
    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_UPDATE, nIndex);
    IMtcSession* pSession = m_objContext.GetSession(piSession);

    pSession->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        // TODO: Send CANCEL
        RecoverModificationFailure();
        NotifyFailure();
        return CallStateName::ESTABLISHED;
    }

    if (pSession->SendPrack() == IMS_FAILURE)
    {
        // TODO: Send CANCEL
        RecoverModificationFailure();
        NotifyFailure();
        return CallStateName::ESTABLISHED;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::Refresh_NotifyTimerExpired(
        OUT IMS_BOOL& bDoImplicitRefresh)
{
    bDoImplicitRefresh = IMS_FALSE;
    // TODO: if session_timer_update_required_in_session_update_by_reinvite_bool is true,
    // no need to refresh. session timer is updated by re-INVITE.
    m_objContext.GetUpdatingInfo().SetPendingUpdate(IMS_TRUE);
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    IMS_TRACE_D("OnTimerExpired : %d", nType, 0, 0);

    switch (nType)
    {
        case TIMER_CONVERT_USER_RESPONSE:
            return RejectUpdate(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE));
        case TIMER_CONVERT_REMOTE_RESPONSE:
            return CancelUpdate(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE));
        case TIMER_GLARE_CONDITION:
            return HandleRetry();
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::OnReceivingMediaDataFailed(
        IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType)
{
    IMS_TRACE_I(
            "OnReceivingMediaDataFailed : Media[%d] Protocol[%d]", eMediaType, eProtocolType, 0);

    if (IsCallEndNeededByAudioInactivity(eMediaType, eProtocolType))
    {
        CallReasonInfo objReason(CODE_MEDIA_NO_DATA);
        HandleTerminate(objReason);
        m_objContext.GetUiNotifier().SendTerminated(objReason);
        return CallStateName::TERMINATING;
    }

    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [=](IMtcCallState* pState)
            {
                return pState->OnReceivingMediaDataFailed(eMediaType, eProtocolType);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::OnVideoLowestBitRate()
{
    IMS_TRACE_I("OnVideoLowestBitRate", 0, 0, 0);
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [](IMtcCallState* pState)
            {
                return pState->OnVideoLowestBitRate();
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);

    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendTerminated(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::QosReserved(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("QosReserved : Media[%d] is reserved.", eMediaType, 0, 0);
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL || piMessage->GetStatusCode() != SipStatusCode::SC_200)
    {
        return GetStateName();
    }

    if (m_objContext.GetUpdatingInfo().IsModifier())
    {
        const IMtcPreconditionManager& objPreconditionManager =
                m_objContext.GetPreconditionManager();
        if (!objPreconditionManager.IsEarlyUpdateRequired(piSession))
        {
            return GetStateName();
        }

        if (!objPreconditionManager.IsAvailableToSendEarlyUpdate(piSession))
        {
            return GetStateName();
        }

        if (m_objContext.GetSession(piSession)->SendEarlyUpdate(UpdateType::NORMAL) == IMS_FAILURE)
        {
            IMS_TRACE_D("QosReserved : Fail to send UPDATE.", 0, 0, 0);
            RecoverModificationFailure();
        }
    }
    else
    {
        CheckPreconditionAndNotifyIncomingUpdate(piSession);
    }
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_I("QosReserveFailed", 0, 0, 0);
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        CallReasonInfo objReason(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
        HandleTerminate(objReason);
        m_objContext.GetUiNotifier().SendTerminated(objReason);

        return CallStateName::TERMINATING;
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [=](IMtcCallState* pState)
                {
                    return pState->QosReserveFailed(piSession, eNextAction);
                });
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::OnIpcanChanged(IN IMS_UINT32 eIpcan)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [eIpcan](IMtcCallState* pState)
            {
                return pState->OnIpcanChanged(eIpcan);
            });
    return GetStateName();
}

GLOBAL PUBLIC IMS_BOOL UpdatingState::IsPreconditionRequired(
        IN const MtcConfigurationProxy& objConfigProxy, IN const UpdatingInfo& objInfo)
{
    return objInfo.IsModified() && !objInfo.IsDowngraded() &&
            objConfigProxy.GetInt(Feature::POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING) ==
            CarrierConfig::ImsVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING;
}

PROTECTED VIRTUAL CallStateName UpdatingState::HandleSrvccStarted()
{
    IMS_TRACE_D("HandleSrvccStarted", 0, 0, 0);
    const CallReasonInfo objReason(CODE_LOCAL_CALL_VCC_ON_PROGRESSING);
    if (m_objContext.GetUpdatingInfo().IsModifier())  // TODO: proper condition?
    {
        return CancelUpdate(objReason);
    }
    else
    {
        return RejectUpdate(objReason);
    }
    return GetStateName();
}

PRIVATE
IMS_RESULT UpdatingState::HandleSdpAnswer()
{
    IMS_TRACE_D("HandleSdpAnswer", 0, 0, 0);

    ISession* piSession = &m_objContext.GetSession()->GetISession();

    IMessage* piMessage = IMS_NULL;
    if (m_objContext.GetUpdatingInfo().IsModifier())
    {
        piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
                piSession, IMessage::SESSION_UPDATE);
    }
    else
    {
        piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    }

    if (m_objContext.GetMessageUtils().HasSdp(piMessage) == IMS_FALSE)
    {
        return IMS_SUCCESS;
    }

    if (m_objContext.GetMediaManager().NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        // TODO
    }

    m_objContext.GetUpdatingInfo().GetModifiedInfo() =
            m_objContext.GetMediaManager().GetMediaInfo();
    m_objContext.GetPreconditionManager().OnSdpReceived(piSession, piMessage);

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

    return m_objContext.GetSession()->SendAck();
}

PRIVATE
IMS_RESULT UpdatingState::SendUpdate()
{
    IMS_TRACE_D("SendUpdate", 0, 0, 0);

    m_objContext.GetUpdatingInfo().GetModifyingInfo() =
            m_objContext.GetUpdatingInfo().GetNegotiatedInfo();
    m_objContext.GetUpdatingInfo().GetNegotiatedInfo() =
            m_objContext.GetUpdatingInfo().GetModifiedInfo();
    m_objContext.GetUpdatingInfo().GetAlertingInfo().eAudioDirection = DIRECTION_INVALID;
    m_objContext.GetUpdatingInfo().GetModifiedInfo().eAudioDirection = DIRECTION_INVALID;

    m_objContext.GetUpdatingInfo().SetModifier();
    m_objContext.GetMediaManager().SetMediaInfo(m_objContext.GetUpdatingInfo().GetModifyingInfo());

    IMtcSession* pSession = m_objContext.GetSession();

    if (pSession->Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetTimer().Start(TIMER_CONVERT_REMOTE_RESPONSE,
            m_objContext.GetConfigurationProxy().GetInt(Feature::CONVERT_REMOTE_RESPONSE_TIMER));

    return IMS_SUCCESS;
}

PRIVATE
CallStateName UpdatingState::HandleModificationSucceeded()
{
    IMS_TRACE_D("HandleModificationSucceeded", 0, 0, 0);

    NotifyHoldResumeState();

    IMS_BOOL bModified = m_objContext.GetUpdatingInfo().IsModified();
    CallStateName eCallStateName;

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
        ISession* piSession = &m_objContext.GetSession()->GetISession();
        m_objContext.GetMediaManager().Run(piSession, IMS_NULL, IMS_FALSE);

        if (bModified)
        {
            m_objContext.GetPreconditionManager().OnCallModified(piSession);
        }
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
        SendIncomingUpdate(m_objContext.GetMediaManager().GetNegotiatedCallType(
                &m_objContext.GetSession()->GetISession()));
        return CallStateName::UPDATING;
    }

    if (m_objContext.GetUpdatingInfo().IsHeld() || m_objContext.GetUpdatingInfo().IsResumed())
    {
        return CallStateName::ESTABLISHED;
    }

    UpdateCallType();

    m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());

    return CallStateName::ESTABLISHED;
}

PRIVATE
CallStateName UpdatingState::HandleReceivedModificationSucceeded()
{
    IMS_TRACE_D("HandleReceivedModificationSucceeded", 0, 0, 0);

    if (m_objContext.GetUpdatingInfo().IsAlerted())
    {
        UpdateCallType();

        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
                m_objContext.GetMediaManager().GetMediaInfo(),
                m_objContext.GetSupplementaryService().GetServices());

        return CallStateName::ESTABLISHED;
    }

    if (m_objContext.GetUpdatingInfo().IsModified())
    {
        SendIncomingUpdate(m_objContext.GetMediaManager().GetNegotiatedCallType(
                &m_objContext.GetSession()->GetISession()));
        return CallStateName::UPDATING;
    }

    if (m_objContext.GetUpdatingInfo().IsHeldBy() || m_objContext.GetUpdatingInfo().IsResumedBy())
    {
        return CallStateName::ESTABLISHED;
    }

    m_objContext.GetUiNotifier().SendUpdatedBy(&m_objContext.GetCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());

    return CallStateName::ESTABLISHED;
}

PRIVATE
CallStateName UpdatingState::HandleRetry()
{
    MediaInfo objMediaInfo = m_objContext.GetUpdatingInfo().GetModifyingInfo();
    UpdateType eType = m_objContext.GetUpdatingInfo().GetRequestingType();
    CallType eCallType = m_objContext.GetUpdatingInfo().GetTargetCallType();

    IMS_TRACE_I("HandleRetry UpdateType[%d]", eType, 0, 0);
    if (eType == UpdateType::HOLD)
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [objMediaInfo](IMtcCallState* pState) mutable
                {
                    return pState->Hold(objMediaInfo);
                });
    }
    else if (eType == UpdateType::RESUME)
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [objMediaInfo](IMtcCallState* pState) mutable
                {
                    return pState->Resume(objMediaInfo);
                });
    }
    else if (eType == UpdateType::SESSION)
    {
        // TODO: receiving 491 for RejectUpdate->SendUpdate.
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [eCallType, objMediaInfo](IMtcCallState* pState) mutable
                {
                    return pState->Update(eCallType, objMediaInfo);
                });
    }
    else if (eType == UpdateType::SRVCC_RECOVERED_CANCEL)
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [](IMtcCallState* pState)
                {
                    return pState->OnSrvccStateUpdated(SrvccState::CANCELED);
                });
    }
    else if (eType == UpdateType::SRVCC_RECOVERED_FAILURE)
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [](IMtcCallState* pState)
                {
                    return pState->OnSrvccStateUpdated(SrvccState::FAILED);
                });
    }

    // Other UpdateTypes are not used.
    return CallStateName::ESTABLISHED;
}

PRIVATE
void UpdatingState::RecoverModificationFailure()
{
    IMS_TRACE_D("RecoverModificationFailure", 0, 0, 0);

    IMtcSession* pSession = m_objContext.GetSession();
    pSession->SetCallType(pSession->GetPreviousCallType());
    m_objContext.GetMediaManager().RestoreSdp(&pSession->GetISession());
    m_objContext.GetPreconditionManager().OnCallModified(&pSession->GetISession());
}

PRIVATE
void UpdatingState::NotifyFailure()
{
    IMS_TRACE_D("NotifyFailure", 0, 0, 0);
    if (m_objContext.GetUpdatingInfo().IsHeld())
    {
        m_objContext.GetUiNotifier().SendHoldFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED));
    }
    else if (m_objContext.GetUpdatingInfo().IsResumed())
    {
        m_objContext.GetUiNotifier().SendResumeFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED));
    }
    else
    {
        m_objContext.GetUiNotifier().SendUpdateFailed(
                CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION));
    }
}

PRIVATE
void UpdatingState::StopTimer()
{
    IMS_TRACE_D("StopTimer", 0, 0, 0);

    if (m_objContext.GetUpdatingInfo().IsModifier())
    {
        m_objContext.GetTimer().Stop(TIMER_CONVERT_REMOTE_RESPONSE);
        m_objContext.GetTimer().Stop(TIMER_GLARE_CONDITION);
    }

    if (m_objContext.GetUpdatingInfo().IsAlerted())
    {
        m_objContext.GetTimer().Stop(TIMER_CONVERT_USER_RESPONSE);
    }
}

PRIVATE
void UpdatingState::UpdateCallType()
{
    IMtcSession* pSession = m_objContext.GetSession();
    CallType eOldCallType = pSession->GetCallType();
    CallType eNewCallType =
            m_objContext.GetMediaManager().GetNegotiatedCallType(&pSession->GetISession());

    if (eOldCallType == eNewCallType)
    {
        return;
    }

    pSession->SetCallType(eNewCallType);
    IMS_TRACE_D("UpdateCallType : [%d] -> [%d]", eOldCallType, eNewCallType, 0);
}

PRIVATE
void UpdatingState::CheckPreconditionAndNotifyIncomingUpdate(IN ISession* piSession)
{
    if (!m_objContext.GetUpdatingInfo().IsAlerted() &&
            m_objContext.GetPreconditionManager().IsAvailableToAlertUser(piSession))
    {
        SendIncomingUpdate(m_objContext.GetMediaManager().GetNegotiatedCallType(piSession));
    }
}
