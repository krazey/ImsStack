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
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "utility/MessageUtil.h"

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

PUBLIC VIRTUAL CallStateName UpdatingState::Hold(IN MediaInfo* pMediaInfo)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [pMediaInfo](IMtcCallState* pState)
            {
                return pState->Hold(pMediaInfo);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::Resume(IN MediaInfo* pMediaInfo)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [pMediaInfo](IMtcCallState* pState)
            {
                return pState->Resume(pMediaInfo);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::Update(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    m_objContext.GetPendingOperationHolder().PushPendingOperation(
            [=](IMtcCallState* pState)
            {
                return pState->Update(eCallType, pMediaInfo);
            });
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName UpdatingState::AcceptUpdate(
        IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("AcceptUpdate", 0, 0, 0);

    m_objContext.GetTimer().Stop(TIMER_CONVERT_USER_RESPONSE);

    IMtcSession* pSession = m_objContext.GetSession();
    ISession& objSession = pSession->GetISession();
    if (objSession.GetState() == ISession::STATE_ESTABLISHED)
    {
        MediaInfo objMediaInfo;
        m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
        return CallStateName::ESTABLISHED;
    }
    m_objContext.GetUpdatingInfo().AdjustDirectionIfNeededForHoldOrResume(*pMediaInfo);

    m_objContext.GetMediaManager().SetMediaInfo(*pMediaInfo);

    if (m_objContext.GetMediaManager().FormSdp(&objSession, eCallType) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(&objSession, IMS_FALSE);

    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifiedInfo());

    if (pSession->AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()),
                &m_objContext.GetUpdatingInfo().GetModifiedInfo(),
                m_objContext.GetSupplementaryService().GetServices());
        return CallStateName::ESTABLISHED;
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::RejectUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("RejectUpdate", 0, 0, 0);

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

PUBLIC VIRTUAL CallStateName UpdatingState::SessionUpdated(IN ISession* /* piSession */)
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

    return HandleModificationSucceeded();
}

PUBLIC VIRTUAL CallStateName UpdatingState::SessionUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionUpdateFailed", 0, 0, 0);

    StopTimer();

    IMessage* piResponse = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_UPDATE);
    CallReasonInfo objReason = UpdateErrorHandler(m_objContext).Handle(piResponse);

    if (objReason.nCode == CODE_USER_TERMINATED_BY_REMOTE)
    {
        HandleTerminate(objReason);
        m_objContext.GetUiNotifier().SendTerminated(objReason);
        return CallStateName::TERMINATING;
    }
    else  // TODO: retry
    {
        RecoverModificationFailure();
        return CallStateName::ESTABLISHED;
    }
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

PUBLIC VIRTUAL CallStateName UpdatingState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);

    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendTerminated(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName UpdatingState::QosReserveFailed(
        IN ISession* /* piSession */, IN QosLossPolicy eNextAction)
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
        // TODO: downgrade to voip. send early update or send re-INVITE after call establishment.
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
        piMessage = MessageUtil::GetPreviousResponse(piSession, IMessage::SESSION_UPDATE);
    }
    else
    {
        piMessage = piSession->GetPreviousRequest(IMessage::SESSION_ACK);
    }

    if (MessageUtil::HasSdp(piMessage) == IMS_FALSE)
    {
        return IMS_SUCCESS;
    }

    if (m_objContext.GetMediaManager().NegotiateSdp(piSession) != NegotiationResult::NO_ERROR)
    {
        // TODO
    }

    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifiedInfo());
    m_objContext.GetPreconditionManager().UpdateQosAttributesFromSdp(piSession);

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
    m_objContext.GetUpdatingInfo().GetAlertingInfo().eADir = DIRECTION_INVALID;
    m_objContext.GetUpdatingInfo().GetModifiedInfo().eADir = DIRECTION_INVALID;

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

    CallStateName eCallStateName = CallStateName::ESTABLISHED;

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
        m_objContext.GetMediaManager().Run(
                &m_objContext.GetSession()->GetISession(), IMS_NULL, IMS_FALSE);
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

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
    m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()), &objMediaInfo,
            m_objContext.GetSupplementaryService().GetServices());

    return CallStateName::ESTABLISHED;
}

PRIVATE
CallStateName UpdatingState::HandleReceivedModificationSucceeded()
{
    IMS_TRACE_D("HandleReceivedModificationSucceeded", 0, 0, 0);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    if (m_objContext.GetUpdatingInfo().IsAlerted())
    {
        UpdateCallType();

        m_objContext.GetUiNotifier().SendUpdated(&(m_objContext.GetCallInfo()), &objMediaInfo,
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

    m_objContext.GetUiNotifier().SendUpdatedBy(&m_objContext.GetCallInfo(), &objMediaInfo,
            m_objContext.GetSupplementaryService().GetServices());

    return CallStateName::ESTABLISHED;
}

PRIVATE
void UpdatingState::RecoverModificationFailure()
{
    IMS_TRACE_D("RecoverModificationFailure", 0, 0, 0);

    m_objContext.GetMediaManager().RestoreSdp(&m_objContext.GetSession()->GetISession());

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
