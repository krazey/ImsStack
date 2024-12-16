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
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/IncomingState.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/TerminationHandler.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
IncomingState::IncomingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::INCOMING, objContext)
{
    IMS_TRACE_D("+IncomingState", 0, 0, 0);
}

PUBLIC VIRTUAL IncomingState::~IncomingState()
{
    IMS_TRACE_D("~IncomingState", 0, 0, 0);
}

PUBLIC VIRTUAL void IncomingState::OnExit()
{
    m_objContext.GetTimer().Stop(TIMER_RETRY_UPDATE);
}

PUBLIC VIRTUAL CallStateName IncomingState::Reject(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Reject reason[%s]", _TRACE_CR_(objReason), 0, 0);
    return RejectIncomingAndToTerminating(objReason);
}

PUBLIC VIRTUAL CallStateName IncomingState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendTerminated(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    m_objContext.GetUiNotifier().SendIncomingCallRejected(
            TerminationHandler(m_objContext).Handle(*piSession));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    m_objContext.GetSession()->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    if (HandleReceivedSdp(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (objPreconditionManager.IsCheckingResourcesRequiredToAlertUser())
    {
        if (!objPreconditionManager.IsAvailableToAlertUser(piSession))
        {
            return GetStateName();
        }
    }

    m_objContext.GetUiNotifier().SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    IMessage* piResponse = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    CallReasonInfo objReason = EarlyUpdateErrorHandler(m_objContext).Handle(piResponse);
    if (objReason.nCode == CODE_SIP_REQUEST_PENDING)
    {
        m_objContext.GetMediaManager().FinalizeSdp(piSession);
        m_objContext.GetTimer().Start(TIMER_RETRY_UPDATE, objReason.nExtraCode);
        return GetStateName();
    }
    m_objContext.GetUiNotifier().SendIncomingCallRejected(
            CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
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

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (objPreconditionManager.IsCheckingResourcesRequiredToAlertUser())
    {
        if (!objPreconditionManager.IsAvailableToAlertUser(piSession))
        {
            return GetStateName();
        }
    }

    m_objContext.GetUiNotifier().SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionPrackReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPrackReceived", 0, 0, 0);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    IMtcSession* pSession = m_objContext.GetSession();

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

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (objPreconditionManager.IsCheckingResourcesRequiredToAlertUser())
    {
        if (!objPreconditionManager.IsAvailableToAlertUser(piSession))
        {
            return GetStateName();
        }
    }

    m_objContext.GetUiNotifier().SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionRprDeliveryFailed(IN ISession* /* piSession*/)
{
    IMS_TRACE_D("SessionRprDeliveryFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(
            CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK));
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionStartFailed(IN ISession* /* piSession */)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);
    if (IsNeedToIgnoreStartFailure())
    {
        return GetStateName();
    }

    m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_RETRY_UPDATE:
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

PUBLIC VIRTUAL CallStateName IncomingState::QosReserved(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_D("QosReserved : Media[%d] is reserved.", eMediaType, 0, 0);
    if (piSession->GetPreviousRequest(IMessage::SESSION_PRACK) == IMS_NULL)
    {
        return GetStateName();
    }

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsCheckingResourcesRequiredToAlertUser())
    {
        return GetStateName();
    }

    if (!objPreconditionManager.IsAvailableToAlertUser(piSession))
    {
        return GetStateName();
    }

    m_objContext.GetUiNotifier().SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::QosReserveFailed(
        IN ISession* /*piSession*/, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_D("QosReserveFailed", 0, 0, 0);
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_QOS_FAILURE));
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IncomingState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(objReason);
}

PUBLIC VIRTUAL CallStateName IncomingState::OnIpcanChanged(IN IMS_UINT32 eIpcan)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [eIpcan](IMtcCallState* pState)
            {
                return pState->OnIpcanChanged(eIpcan);
            });
    return GetStateName();
}

PROTECTED VIRTUAL CallStateName IncomingState::HandleAosConnected()
{
    IMS_TRACE_I("HandleAosConnected", 0, 0, 0);
    m_objContext.GetPreconditionManager().HandleQosOnIpcanChanged();

    if (m_objContext.GetEpsFallbackTrigger().IsWaitingEpsFallbackForNoTrigger() &&
            !m_objContext.GetService().IsNr())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
        m_objContext.GetUiNotifier().SendIncomingCallReceived();
        return CallStateName::ALERTING;
    }

    return GetStateName();
}
