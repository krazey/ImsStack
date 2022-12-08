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
#include "ISession.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/state/AlertingState.h"
#include "call/termination/CancelHandler.h"
#include "call/termination/TerminationHandler.h"
#include "configuration/ConfigDef.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/UdpKeepAliveSender.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/SipHeaderName.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
AlertingState::AlertingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ALERTING, objContext)
{
}

PUBLIC VIRTUAL AlertingState::~AlertingState() {}

PUBLIC VIRTUAL void AlertingState::OnEnter()
{
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()))
    {
        m_objContext.GetUdpKeepAliveSender().Start();
    }
}

PUBLIC VIRTUAL void AlertingState::OnExit()
{
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()))
    {
        m_objContext.GetUdpKeepAliveSender().Stop();
    }
}

PUBLIC VIRTUAL CallStateName AlertingState::HandleUserAlert()
{
    IMS_TRACE_D("HandleUserAlert", 0, 0, 0);
    if (m_objContext.GetSession()->SendProvisionalResponse(IMS_TRUE) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }
    StartTimer(TIMER_MT_ALERTING);

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::Accept(
        IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_D("Accept", 0, 0, 0);
    IMtcSession* pSession = m_objContext.GetSession();

    IMS_BOOL bCallTypeChanged = pSession->GetCallType() != eCallType;
    pSession->SetCallType(eCallType);

    m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);

    m_objContext.GetTimer().StopAll();
    if (bCallTypeChanged)
    {
        if (pSession->SendEarlyUpdate(UpdateType::NORMAL) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        }
        return GetStateName();
    }

    if (pSession->Accept() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    RunMedia(GetISession(), IMS_NULL);

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::Reject(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Reject", 0, 0, 0);
    return RejectIncomingAndToTerminating(objReason);
}

PUBLIC VIRTUAL CallStateName AlertingState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendTerminated(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName AlertingState::AcceptUssi(
        IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_D("AcceptUssi", 0, 0, 0);
    IMtcSession* pSession = m_objContext.GetSession();

    pSession->SetCallType(eCallType);
    m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);

    m_objContext.GetTimer().StopAll();

    if (m_objContext.GetUssiController()->FormAcceptUssi() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    if (pSession->Accept() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::UssiStarted(IN ISession* piSession)
{
    IMS_TRACE_D("UssiStarted - ACK received", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    m_objContext.GetSession()->HandleRequest(RequestType::ACK, *piMessage);

    UssiResult objResult = m_objContext.GetUssiController()->ParseUssiBodyAndCheckResult(
            piMessage->GetMessage(), piMessage->GetMethod().ToInt());

    SendStarted();

    if (objResult.eAction != UssiNextAction::NOTHING)
    {
        SendInfoForUssi(AString::ConstEmpty(), objResult.eErrorCode);
    }

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted - ACK received", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    IMtcSession* pSession = m_objContext.GetSession();

    pSession->HandleRequest(RequestType::ACK, *piMessage);

    // TODO: need to check NegotiationState::STATE_OFFER_SENT?
    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        // TODO TerminateAndToTerminating() ?
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        pSession->Terminate(IMS_TRUE, objReason);

        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        return CallStateName::TERMINATING;
    }

    RunMedia(piSession, piMessage);
    SendStarted();

    m_objContext.GetPreconditionManager().CheckLocalResourceAvailableOnCallEstablished(piSession);

    return CallStateName::ESTABLISHED;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_TERMINATE);
    if (piMessage == IMS_NULL)
    {
        return CallStateName::TERMINATING;
    }

    m_objContext.GetUiNotifier().SendStartFailed(piMessage->GetMethod().Equals(SipMethod::CANCEL)
                    ? CancelHandler(m_objContext).Handle(*piMessage)
                    : TerminationHandler(m_objContext).Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousResponse(IMessage::SESSION_EARLY_UPDATE);
    IMtcSession* pSession = m_objContext.GetSession();

    pSession->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    if (IsUpdateBySrvcc(piSession) == IMS_FALSE)
    {
        // if there is another case sending early UPDATE other than SRVCC, need to be checked.
        if (pSession->Accept() == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        }
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdateFailed(
        IN ISession* /* piSession */)
{
    /*
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    TODO: failure handler
    */

    m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    IMtcSession* pSession = m_objContext.GetSession();
    pSession->HandleRequest(RequestType::EARLY_UPDATE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_488) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
        }
        return GetStateName();
    }

    if (pSession->RespondToEarlyUpdate(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    IMtcSession* pSession = m_objContext.GetSession(piSession);
    pSession->HandleRequest(RequestType::PRACK, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        pSession->RespondToPrack(SipStatusCode::SC_200);
        // According to RFC 6337, UE must send re-offer.
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE));
    }

    if (pSession->RespondToPrack(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionRPRDeliveryFailed(IN ISession* /* piSession*/)
{
    IMS_TRACE_D("SessionRPRDeliveryFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(
            CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK));
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionStartFailed(IN ISession* /* piSession */)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);
    if (IsNeedToIgnoreStartFailure())
    {
        return GetStateName();
    }

    m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName AlertingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MT_ALERTING:
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER));
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_D("QosReserveFailed", 0, 0, 0);

    if (eNextAction == QosLossPolicy::RELEASE)
    {
        m_objContext.GetPreconditionManager().FormPreconditionSdp(piSession, IMS_TRUE);
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_QOS_FAILURE));
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call established.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);

    return RejectIncomingAndToTerminating(objReason);
}

PUBLIC VIRTUAL CallStateName AlertingState::OnIpcanChanged(IN IMS_UINT32 eIpcan)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [eIpcan](IMtcCallState* pState)
            {
                return pState->OnIpcanChanged(eIpcan);
            });
    return GetStateName();
}

PROTECTED VIRTUAL CallStateName AlertingState::SendUpdateBySrvcc(IN UpdateType eType)
{
    IMtcSession* piMtcSession = m_objContext.GetSession();
    if (piMtcSession == IMS_NULL)
    {
        return GetStateName();
    }

    ISession& objSession = piMtcSession->GetISession();

    if (m_objContext.GetMediaManager().GetNegotiationState(&objSession) ==
            NegotiationState::STATE_NEGOTIATED)
    {
        piMtcSession->SendEarlyUpdate(eType);
    }
    return GetStateName();
}

PRIVATE
IMS_BOOL AlertingState::IsUpdateBySrvcc(IN ISession* piSession) const
{
    IMessage* piUpdateMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    if (piUpdateMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (piUpdateMessage->GetState() != IMessage::STATE_SENT)
    {
        // TODO: glare condition
        return IMS_FALSE;
    }

    AString strReason;
    MessageUtil::GetHeader(piUpdateMessage, ISipHeader::UNKNOWN, strReason, SipHeaderName::REASON);
    if (strReason.Equals(MessageUtil::STR_REASON_HANDOVER_CANCELLED) ||
            strReason.Equals(MessageUtil::STR_REASON_FAILURE_TO_TRANSITION))
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}
