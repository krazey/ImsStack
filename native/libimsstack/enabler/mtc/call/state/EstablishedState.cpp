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
#include "ISipServerConnection.h"
#include "ServiceTrace.h"

#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/UpdatingInfo.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/MtcBlockChecker.h"
#include "call/state/EstablishedState.h"
#include "call/termination/TerminationHandler.h"
#include "conferencecall/IConferenceManager.h"
#include "conferencecall/IConferenceController.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "sipcore/SipStatusCode.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include "utility/MessageUtil.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EstablishedState::EstablishedState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ESTABLISHED, objContext)
{
}

PUBLIC VIRTUAL EstablishedState::~EstablishedState() {}

PUBLIC VIRTUAL CallStateName EstablishedState::Hold(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Hold", 0, 0, 0);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
    if (objMediaInfo.eADir == DIRECTION_INACTIVE)
    {
        m_objContext.SetHeldByMe(IMS_TRUE);
        m_objContext.GetUiNotifier().SendHeld(&(m_objContext.GetCallInfo()), &objMediaInfo,
                m_objContext.GetSupplementaryService().GetServices());
        delete pMediaInfo;
        return GetStateName();
    }

    if (HandleUpdate(UpdateType::HOLD, m_objContext.GetSession()->GetCallType(), pMediaInfo) ==
            IMS_FAILURE)
    {
        // TODO
    }

    delete pMediaInfo;
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Resume(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Resume", 0, 0, 0);
    if (HandleUpdate(UpdateType::RESUME, m_objContext.GetSession()->GetCallType(), pMediaInfo) ==
            IMS_FAILURE)
    {
        // TODO
    }

    delete pMediaInfo;
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Update(
        IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("Update", 0, 0, 0);

    m_objContext.GetUpdatingInfo().SetTargetCallType(eCallType);

    if (HandleUpdate(UpdateType::SESSION, eCallType, pMediaInfo) == IMS_FAILURE)
    {
        // TODO
    }

    delete pMediaInfo;
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    // SetTerminateCodeForInvitedSessionToConf

    const CallReasonInfo objTerminateReason = GetAudioInactivityReasonOnTermination(objReason);

    HandleTerminate(objTerminateReason);
    m_objContext.GetUiNotifier().SendTerminated(objTerminateReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    m_objContext.GetUiNotifier().SendTerminated(IsConferenceCallParticipant()
                    ? CallReasonInfo(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE)
                    : TerminationHandler().Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SessionUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_D("SessionUpdateReceived", 0, 0, 0);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_UPDATE);

    m_objContext.GetSession()->HandleRequest(IMessage::SESSION_UPDATE, *piMessage);
    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetNegotiatedInfo());
    m_objContext.GetUpdatingInfo().SetTargetCallType(
            MessageUtil::GetCallType(piMessage, piSession, IMS_TRUE));

    // TODO, conference

    IMS_RESULT eResult = IMS_SUCCESS;
    CallStateName eStateName = CallStateName::UPDATING;

    if (MessageUtil::HasSdp(piMessage))
    {
        auto pBlockChecker = std::make_unique<MtcBlockChecker>(GetCallUpdateBlockRules(), nullptr);
        IMtcBlockChecker::Result objResult = pBlockChecker->Check();

        if (objResult.eStatus == IMtcBlockChecker::Result::Status::UNBLOCKED)
        {
            eResult = HandleReceivedUpdate(eStateName);
        }
        else
        {
            m_objContext.GetSession()->Reject(objResult.objReason);
            eStateName = CallStateName::ESTABLISHED;
        }
    }
    else
    {
        eResult = HandleReceivedUpdateWithoutOffer(eStateName);
    }

    if (eResult != IMS_SUCCESS)
    {
        // TODO
    }

    if (eStateName == CallStateName::ESTABLISHED)
    {
        NotifyHoldResumeState();
        m_objContext.DeleteUpdatingInfo();
    }

    return eStateName;
}

PUBLIC VIRTUAL CallStateName EstablishedState::TerminateUssi(IN const CallReasonInfo& /*objReason*/)
{
    IMS_TRACE_D("TerminateUssi", 0, 0, 0);

    SendInfoForUssi(AString::ConstEmpty(), UssiError::CODE_1);
    m_objContext.GetUssiController()->SetNextActionByTerminateUssi();

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::UssiTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("UssiTerminated", 0, 0, 0);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_TERMINATE);
    UssiController* pUssiController = m_objContext.GetUssiController();

    if (!pUssiController->IsByeForUssi(piMessage))
    {
        return SessionTerminated(piSession);
    }

    pUssiController->ParseUssiBodyAndCheckResult(
            piMessage->GetMessage(), piMessage->GetMethod().ToInt());

    m_objContext.GetUiNotifier().SendTerminated(TerminationHandler().Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SendUssd(IN const AString& strUssd)
{
    IMS_TRACE_D("SendUssi", 0, 0, 0);

    SendInfoForUssi(strUssd);
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::UssiInfoReceived(
        IN ISession* piSession, IN ISipServerConnection* piSipServerConnection)
{
    IMS_TRACE_D("UssiInfoReceived", 0, 0, 0);
    UNUSED_PARAM(piSession);
    IMS_SINT32 nMethod = piSipServerConnection->GetMethod().ToInt();

    if (nMethod != SipMethod::INFO)
    {
        SendTransactionResponse(piSipServerConnection, SipStatusCode::SC_200);
        return GetStateName();
    }

    UssiController* pUssiController = m_objContext.GetUssiController();
    if (!pUssiController->IsUssiInfoReceived(piSipServerConnection) ||
            !pUssiController->HasXmlBodyInInfo(piSipServerConnection))
    {
        SendTransactionResponse(piSipServerConnection, SipStatusCode::SC_469, "Bad Info Package");
        return GetStateName();
    }

    UssiResult objResult = pUssiController->ParseUssiBodyAndCheckResult(
            piSipServerConnection->GetMessage(), nMethod);

    SendTransactionResponse(piSipServerConnection, SipStatusCode::SC_200);

    switch (objResult.eAction)
    {
        case UssiNextAction::SEND_INFO_WITH_ERROR_CODE:
            SendInfoForUssi(AString::ConstEmpty(), objResult.eErrorCode);
            break;
        case UssiNextAction::SEND_INFO_WITH_NOTIFY_ELEMENT:
            SendInfoForUssi(AString::ConstEmpty());
            break;
        default:
            break;
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::NotifyResponseToUssiInfo(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc)
{
    IMS_TRACE_D("NotifyResponseToUssiInfo", 0, 0, 0);
    CallStateName eState = ClientConnection_NotifyResponse(piScc, piForkedScc);

    if (m_objContext.GetUssiController()->GetLastResult().eAction ==
            UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE)
    {
        return TerminateUssiAfterInfoTransaction();
    }

    return eState;
}

PUBLIC VIRTUAL CallStateName EstablishedState::NotifyErrorToUssiInfo(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    IMS_TRACE_D("NotifyErrorToUssiInfo", 0, 0, 0);
    CallStateName eState = Error_NotifyError(piSc, nCode, strMessage);

    if (m_objContext.GetUssiController()->GetLastResult().eAction ==
            UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE)
    {
        return TerminateUssiAfterInfoTransaction();
    }

    return eState;
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnReceivingMediaDataFailed(
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

    CallType eCallType = m_objContext.GetSession()->GetCallType();
    if (eMediaType == MEDIATYPE_VIDEO && eCallType == CallType::VT)
    {
        // TODO: downgrade to voip
    }
    else if (eMediaType == MEDIATYPE_TEXT && eCallType == CallType::RTT)
    {
        // TODO: downgrade to voip
    }
    // TODO: check VIDEO_RTT case

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnVideoLowestBitRate()
{
    IMS_TRACE_I("OnVideoLowestBitRate", 0, 0, 0);

    CallType eCallType = m_objContext.GetSession()->GetCallType();
    if (eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT)
    {
        // TODO: downgrade to voip
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnMediaFailed(IN CallReasonInfo objReason)
{
    IMS_TRACE_I("OnMediaFailed", 0, 0, 0);

    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendTerminated(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::HandleIpcanChanged()
{
    IMS_TRACE_I("HandleIpcanChanged", 0, 0, 0);

    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);
    if (HandleUpdate(UpdateType::SESSION, m_objContext.GetSession()->GetCallType(),
                &objMediaInfo) == IMS_FAILURE)
    {
        // TODO
    }

    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::QosReserveFailed(
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

PRIVATE
IMS_RESULT EstablishedState::HandleUpdate(
        IN UpdateType eUpdateType, IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_D("HandleUpdate", 0, 0, 0);
    m_objContext.GetUpdatingInfo().SetModifier();

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetNegotiatedInfo());
    objMediaManager.SetMediaInfo(*pMediaInfo);

    IMtcSession* pSession = m_objContext.GetSession();

    if (objMediaManager.FormSdp(&(pSession->GetISession()), eCallType) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(
            &(pSession->GetISession()), IMS_FALSE);

    objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifyingInfo());

    if (pSession->Update(eUpdateType, IMS_FALSE, SipMethod::INVITE) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetTimer().Start(TIMER_CONVERT_REMOTE_RESPONSE,
            m_objContext.GetConfigurationProxy().GetInt(Feature::CONVERT_REMOTE_RESPONSE_TIMER));

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT EstablishedState::HandleReceivedUpdate(OUT CallStateName& eStateName)
{
    IMS_TRACE_D("HandleReceivedUpdate", 0, 0, 0);
    ISession& objSession = m_objContext.GetSession()->GetISession();
    if (m_objContext.GetMediaManager().NegotiateSdp(&objSession) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetMediaManager().GetMediaInfo(m_objContext.GetUpdatingInfo().GetAlertingInfo());

    m_objContext.GetPreconditionManager().UpdateQosAttributesFromSdp(&objSession);

    eStateName = CallStateName::UPDATING;

    if (m_objContext.GetUpdatingInfo().IsNeedToAlert())
    {
        SendIncomingUpdate(m_objContext.GetMediaManager().GetNegotiatedCallType(
                &m_objContext.GetSession()->GetISession()));

        return IMS_SUCCESS;
    }

    IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        eStateName = CallStateName::ESTABLISHED;
    }

    if (FormAutoAccept(IMS_FALSE) == IMS_FAILURE ||
            m_objContext.GetSession()->AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT EstablishedState::HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName)
{
    IMS_TRACE_D("HandleReceivedUpdateWithoutOffer", 0, 0, 0);
    eStateName = CallStateName::UPDATING;
    IMessage* piMessage =
            m_objContext.GetSession()->GetISession().GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        eStateName = CallStateName::ESTABLISHED;
    }
    else
    {
        if (FormAutoAccept(IMS_TRUE) == IMS_FAILURE)
        {
            // TODO
        }
    }

    if (m_objContext.GetSession()->AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT EstablishedState::FormAutoAccept(IN IMS_BOOL bWithoutOffer)
{
    IMS_TRACE_D("FormAutoAccept", 0, 0, 0);
    IMtcSession* pSession = m_objContext.GetSession();

    AdjustDirectionWithHeldByMe(bWithoutOffer);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (objMediaManager.FormSdp(&(pSession->GetISession()), pSession->GetCallType()) == IMS_FAILURE)
    {
        // TODO
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(
            &(pSession->GetISession()), IMS_FALSE);

    if (bWithoutOffer)
    {
        objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifyingInfo());
    }
    else
    {
        objMediaManager.GetMediaInfo(m_objContext.GetUpdatingInfo().GetModifiedInfo());
    }

    return IMS_SUCCESS;
}

PRIVATE
void EstablishedState::AdjustDirectionWithHeldByMe(IN IMS_BOOL bWithoutOffer)
{
    MediaInfo objMediaInfo;
    m_objContext.GetMediaManager().GetMediaInfo(objMediaInfo);

    IMS_SINT32 eNewDir = objMediaInfo.eADir;
    if (bWithoutOffer)
    {
        eNewDir = DIRECTION_SEND_RECEIVE;
    }

    if (m_objContext.IsHeldByMe())
    {
        if (eNewDir == DIRECTION_SEND_RECEIVE)
        {
            eNewDir = DIRECTION_SEND;
        }
        else if (eNewDir == DIRECTION_RECEIVE)
        {
            eNewDir = DIRECTION_INACTIVE;
        }
    }

    objMediaInfo.eADir = eNewDir;

    // TODO
    /*
    if (objMediaInfo.eVDir != DIRECTION_INVALID)
    {
        objMediaInfo.eVDir = ???;
    }
    if (objMediaInfo.eTDir != DIRECTION_INVALID)
    {
        objMediaInfo.eTDir = eNewDir;
    }
    */

    m_objContext.GetMediaManager().SetMediaInfo(objMediaInfo);
}

PRIVATE
IMS_BOOL EstablishedState::IsConferenceCallParticipant()
{
    IMSList<IMtcCall*> objConfCalls = m_objContext.GetCallManager().GetCallsInConference();
    if (objConfCalls.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    IConferenceController* piConfController =
            m_objContext.GetConferenceManager().GetController(objConfCalls.GetAt(0)->GetKey());
    if (piConfController == IMS_NULL)
    {
        return IMS_FALSE;
    }

    CallKey nKey = m_objContext.GetCallKey();
    if (piConfController->GetCallStatusInConference(nKey) == IndividualCallState::JOINED ||
            piConfController->GetCallStatusInConference(nKey) == IndividualCallState::JOINING)
    {
        IMS_TRACE_I("IsConferenceCallParticipant [%" PFLS_x "] call is joining to conference call",
                nKey, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMSList<IMtcBlockRule*> EstablishedState::GetCallUpdateBlockRules() const
{
    // No pending rules
    IMSList<IMtcBlockRule*> lstRules;
    lstRules.Append(new CallTypeBlockRule(
            m_objContext, m_objContext.GetUpdatingInfo().GetTargetCallType()));
    return lstRules;
}

PRIVATE
CallStateName EstablishedState::TerminateUssiAfterInfoTransaction()
{
    IMS_TRACE_D("TerminateUssiAfterInfoTransaction", 0, 0, 0);

    CallReasonInfo objReason(CODE_UNSPECIFIED);
    m_objContext.GetSession()->Terminate(IMS_TRUE, objReason);

    m_objContext.GetUiNotifier().SendStartFailed(objReason);

    return CallStateName::TERMINATING;
}
