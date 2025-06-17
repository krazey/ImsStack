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

PUBLIC
IncomingState::IncomingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::INCOMING, objContext)
{
}

PUBLIC VIRTUAL IncomingState::~IncomingState() {}

PUBLIC VIRTUAL void IncomingState::OnExit()
{
    m_objContext.GetTimer().Stop(TIMER_RETRY_UPDATE);
}

PUBLIC VIRTUAL CallStateName IncomingState::Reject(IN const CallReasonInfo& objReason)
{
    return RejectIncomingAndToTerminating(objReason);
}

PUBLIC VIRTUAL CallStateName IncomingState::Terminate(IN const CallReasonInfo& objReason)
{
    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendIncomingCallRejected(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionTerminated(IN ISession* piSession)
{
    m_objContext.GetUiNotifier().SendIncomingCallRejected(
            TerminationHandler(m_objContext).Handle(*piSession));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMessage* piMessage = m_objContext.GetMessageUtils().GetPreviousResponse(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    m_objContext.GetSession()->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, *piMessage);

    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(eCallReason));
    }

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (objPreconditionManager.IsCheckingResourcesRequiredToAlertUser())
    {
        if (!objPreconditionManager.IsAvailableToAlertUser(piSession))
        {
            return GetStateName();
        }
    }

    return OnReadyToAlert();
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
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

    return OnReadyToAlert();
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionPrackReceived(IN ISession* piSession)
{
    StopTimer(TIMER_MT_PRACK_WAIT);

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

    if (m_objContext.GetMessageUtils().IsResponseExist(
                &pSession->GetISession(), SipStatusCode::SC_180))
    {
        m_objContext.GetUiNotifier().SendIncomingCallReceived();
        return CallStateName::ALERTING;
    }

    const IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (objPreconditionManager.IsCheckingResourcesRequiredToAlertUser())
    {
        if (!objPreconditionManager.IsAvailableToAlertUser(piSession))
        {
            return GetStateName();
        }
    }

    return OnReadyToAlert();
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionRprDeliveryFailed(IN ISession* /* piSession*/)
{
    return RejectIncomingAndToTerminating(
            CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK));
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionStartFailed(IN ISession* /* piSession */)
{
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
        case TIMER_MT_ALERTING:
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER));
        case TIMER_MT_PRACK_WAIT:
            return RejectIncomingAndToTerminating(
                    CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK));
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
        IN ISession* piSession, IN [[maybe_unused]] IMS_UINT32 eMediaType)
{
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

    return OnReadyToAlert();
}

PUBLIC VIRTUAL CallStateName IncomingState::QosReserveFailed(
        IN ISession* /*piSession*/, IN QosLossPolicy eNextAction)
{
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
    if (m_objContext.GetEpsFallbackTrigger().IsWaitingRegistration() &&
            !m_objContext.GetService().IsNr())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();
        return OnReadyToAlert();
    }

    return GetStateName();
}
