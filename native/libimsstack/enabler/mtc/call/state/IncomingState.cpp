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

#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/IncomingState.h"
#include "call/termination/TerminationHandler.h"
#include "helper/MtcTimerWrapper.h"
#include "IMessage.h"
#include "ISession.h"
#include "SipStatusCode.h"
#include "utility/MessageUtil.h"
#include "helper/MtcSupplementaryService.h"
#include "precondition/QosDef.h"
#include "precondition/IMtcPreconditionManager.h"
#include "media/IMtcMediaManager.h"

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

PUBLIC VIRTUAL CallStateName IncomingState::HandleSrvccSuccess()
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IncomingState::HandleSrvccFailure(IN UpdateType /* eUpdateType */)
{
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IncomingState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        // TODO: introduce a new guard timer just in case? otherwise, delete this function.
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IncomingState::QosReserved(
        IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    if (!m_objContext.GetSession()->GetExtensionSet().IsAvailableOnBoth(
                MtcExtensionSet::OPTION_TAG_RPR))
    {
        return GetStateName();
    }

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_D("QosReserved : UE doesn't receive PRACK.", 0, 0, 0);
        return GetStateName();
    }

    IMS_TRACE_D("QosReserved : Media[%d] is reserved.", eMediaType, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession))
            ? QosCheckType::ALL_STATUS
            : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::QosReserveFailed(
        IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_D("QosReserveFailed", 0, 0, 0);
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        return RejectIncomingAndToTerminating(
                CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED));
    }

    if (eNextAction == QosLossPolicy::MODIFY)
    {
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
        UNUSED_PARAM(piSession);
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    m_objContext.GetMediaManager().Terminate();
    m_objContext.GetUiNotifier().SendStartFailed(TerminationHandler().Handle(*piSession));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdated", 0, 0, 0);
    IMessage* piMessage =
            MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_EARLY_UPDATE);
    m_objContext.GetSession()->HandleResponse(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (m_objContext.GetMediaManager().GetNegotiationState(piSession) ==
            NegotiationState::STATE_NEGOTIATED)
    {
        if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
        {
            m_objContext.GetPreconditionManager().StartQosTimer(piSession);
        }
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession))
            ? QosCheckType::ALL_STATUS
            : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdateFailed(
        IN ISession* /* piSession */)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateFailed", 0, 0, 0);
    m_objContext.GetMediaManager().Terminate();
    /*
    IMS_SINT32 nStatusCode = MessageUtil::GetResponseStatusCode(
            piSession, IMessage::SESSION_EARLY_UPDATE);
    TODO: failure handler
    */
    m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_SESSION_INTERNAL_ERROR));
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionEarlyMediaUpdateReceived", 0, 0, 0);
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_EARLY_UPDATE, *piMessage);

    if (!MessageUtil::HasSdp(piMessage))
    {
        if (m_objContext.GetSession()->GetMessageSender().RespondToEarlyUpdate(
                    SipStatusCode::SC_200) == IMS_FAILURE)
        {
            m_objContext.GetMediaManager().Terminate();
            m_objContext.GetUiNotifier().SendStartFailed(
                    CallReasonInfo(CODE_SESSION_INTERNAL_ERROR));
            return CallStateName::TERMINATING;
        }

        return GetStateName();
    }

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        if (SendResponseToEarlyUpdate(SipStatusCode::SC_488, m_objContext.GetSession()) ==
                IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
        }
        return GetStateName();
    }

    if (SendResponseToEarlyUpdate(SipStatusCode::SC_200, m_objContext.GetSession()) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_SESSION_INTERNAL_ERROR));
    }

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession))
            ? QosCheckType::ALL_STATUS
            : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionPRAckReceived", 0, 0, 0);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_PRACK);
    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_PRACK, *piMessage);

    if (OnSdpReceived(piSession, piMessage) != CODE_NONE)
    {
        SendResponseToPrack(SipStatusCode::SC_200);
        // According to RFC 6337, UE must send re-offer.
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE));
    }

    if (SendResponseToPrack(SipStatusCode::SC_200) == IMS_FAILURE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_SESSION_INTERNAL_ERROR));
    }

    SetLocalQosAvailableForWifiCalling(piSession);

    // TODO: CheckReadyToAlert()? common?
    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    if (!objPreconditionManager.IsResourceReserved(piSession, QosCheckType::LOCAL_STATUS))
    {
        objPreconditionManager.StartQosTimer(piSession);
    }

    QosCheckType eCheckType = (objPreconditionManager.HasPreconditionCapability(piSession))
            ? QosCheckType::ALL_STATUS
            : QosCheckType::LOCAL_STATUS;
    if (!objPreconditionManager.IsResourceReserved(piSession, eCheckType))
    {
        return GetStateName();
    }

    SendIncomingCallReceived();
    return CallStateName::ALERTING;
}

PUBLIC VIRTUAL CallStateName IncomingState::SessionRPRDeliveryFailed(IN ISession* /* piSession*/)
{
    IMS_TRACE_D("SessionRPRDeliveryFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT));
}

PUBLIC VIRTUAL CallStateName IncomingState::OnReceivingMediaDataFailed(IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_I("OnReceivingMediaDataFailed", 0, 0, 0);

    if (IsCallEndNeededByAudioInactivity(eMediaType))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NO_DATA));
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName IncomingState::OnMediaFailed(IN CallReasonInfo objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);
    return RejectIncomingAndToTerminating(CallReasonInfo(objReason));
}
