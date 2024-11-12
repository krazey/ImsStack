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
#include "ISipHeader.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/state/AlertingState.h"
#include "call/termination/CancelHandler.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/UdpKeepAliveSender.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
AlertingState::AlertingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ALERTING, objContext),
        m_pUdpKeepAliveSender(IMS_NULL)
{
}

PUBLIC VIRTUAL AlertingState::~AlertingState() {}

PUBLIC VIRTUAL void AlertingState::OnEnter()
{
    if (UdpKeepAliveSender::IsRequired(m_objContext.GetConfigurationProxy()))
    {
        m_pUdpKeepAliveSender.reset(m_objContext.CreateUdpKeepAliveSender());
        m_pUdpKeepAliveSender->Start();
    }
}

PUBLIC VIRTUAL void AlertingState::OnExit()
{
    m_objContext.GetTimer().Stop(TIMER_GLARE_CONDITION);
    if (m_pUdpKeepAliveSender != IMS_NULL)
    {
        m_pUdpKeepAliveSender->Stop();
    }
}

PUBLIC VIRTUAL CallStateName AlertingState::HandleUserAlert()
{
    IMS_TRACE_D("HandleUserAlert", 0, 0, 0);
    if (m_objContext.GetSession()->SendProvisionalResponse(IMS_TRUE, IsRprRequired()) ==
            IMS_FAILURE)
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
    if (m_pUdpKeepAliveSender != IMS_NULL)
    {
        m_pUdpKeepAliveSender->Stop();
    }

    if (bCallTypeChanged &&
            m_objContext.GetMediaManager().GetNegotiationState(&pSession->GetISession()) ==
                    NegotiationState::STATE_NEGOTIATED)
    {
        if (SendEarlyUpdate(UpdateType::NORMAL, pSession) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        }
        return GetStateName();
    }

    if (pSession->Accept() == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    m_objContext.GetMediaManager().Run(GetISession(), IMS_NULL, IMS_FALSE);

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

    m_objContext.GetUiNotifier().SendStarted();

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
    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        // TODO TerminateAndToTerminating() ?
        CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
        pSession->Terminate(IMS_TRUE, objReason);

        m_objContext.GetUiNotifier().SendStartFailed(objReason);
        return CallStateName::TERMINATING;
    }

    m_objContext.GetMediaManager().Run(piSession, piMessage, IMS_FALSE);
    m_objContext.GetUiNotifier().SendStarted();
    m_objContext.GetPreconditionManager().OnCallEstablished(piSession);

    IMS_SINT32 nDelayTime = m_objContext.GetConfigurationProxy().GetInt(
            Feature::DELAY_UPDATE_AFTER_CONNECTED_TIMER);
    if (nDelayTime > 0)
    {
        m_objContext.GetTimer().Start(TIMER_DELAY_UPDATE_AFTER_CONNECTED, nDelayTime);
    }

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

    UpdateType eUpdateType = pSession->GetOngoingUpdateType();
    pSession->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    if (eUpdateType == UpdateType::NORMAL)
    {
        // if there is another case sending early UPDATE other than SRVCC, need to be checked.
        if (pSession->Accept() == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        }
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    CallReasonInfo objReason = EarlyUpdateErrorHandler(m_objContext).Handle(piResponse);
    if (objReason.nCode == CODE_SIP_REQUEST_PENDING)
    {
        m_objContext.GetMediaManager().FinalizeSdp(piSession);
        m_objContext.GetTimer().Start(TIMER_GLARE_CONDITION, objReason.nExtraCode);
        return GetStateName();
    }
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

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
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

PUBLIC VIRTUAL CallStateName AlertingState::SessionPrackReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPrackReceived", 0, 0, 0);
    // FIXME: It's same as IncomingState except QoS check and UI notifying

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    IMtcSession* pSession = m_objContext.GetSession(piSession);
    pSession->HandleRequest(RequestType::PRACK, *piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
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

PUBLIC VIRTUAL CallStateName AlertingState::SessionRprDeliveryFailed(IN ISession* /* piSession*/)
{
    IMS_TRACE_D("SessionRprDeliveryFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(
            CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK));
}

PUBLIC VIRTUAL CallStateName AlertingState::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);
    if (IsNeedToIgnoreStartFailure())
    {
        return GetStateName();
    }

    if (piSession->GetState() == ISession::STATE_ESTABLISHED)
    {
        // This condition occurs when no ACK is received within the 200 OK retransmission timer
        // period. In such cases, the UE should send a BYE to notify the remote party that the
        // call is terminated.
        const CallReasonInfo objReasonInfo(CODE_SIP_SERVER_ERROR);
        return Terminate(objReasonInfo);
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
        case TIMER_GLARE_CONDITION:
        {
            IMtcSession* pSession = m_objContext.GetSession();
            SendEarlyUpdate(pSession->GetOngoingUpdateType(), pSession);
            return GetStateName();
        }
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
