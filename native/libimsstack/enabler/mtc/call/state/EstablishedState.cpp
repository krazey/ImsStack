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
#include "IMessage.h"
#include "ISipClientConnection.h"
#include "ISipServerConnection.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/MtcCallStringUtils.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/RttAutoUpgrader.h"
#include "call/UpdatingInfo.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/MtcBlockChecker.h"
#include "call/block/SrvccBlockRule.h"
#include "call/state/EstablishedState.h"
#include "call/state/UpdatingState.h"
#include "call/termination/TerminationHandler.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/IConferenceManager.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/CurrentLocationDiscoveryController.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/OperationAsyncRunner.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include "utility/IMessageUtils.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EstablishedState::EstablishedState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::ESTABLISHED, objContext)
{
}

PUBLIC VIRTUAL EstablishedState::~EstablishedState()
{
    m_objContext.ReleaseAsyncOperation(this);
}

PUBLIC VIRTUAL void EstablishedState::OnEnter()
{
    if (CurrentLocationDiscoveryController::IsPeriodicLocationDiscoveryRequired(
                m_objContext.GetCallInfo().IsEmergency(),
                m_objContext.GetConfigurationProxy().GetInt(
                        ConfigEmergency::KEY_CALL_PERIODIC_LOCATION_DISCOVERY_METHOD_INT)))
    {
        m_objContext.GetCurrentLocationDiscoveryController().StartPeriodicLocationDiscovery();
    }

    if (RttAutoUpgrader::IsRequired(m_objContext.GetConfigurationProxy(),
                m_objContext.GetCallInfo(), m_objContext.GetSession()))
    {
        m_objContext.CreateRttAutoUpgrader();
    }

    if (ShouldPendOperation())
    {
        m_objContext.RunAsyncOperation(this,
                [&]()
                {
                    m_objContext.RunPendingOperationIfPossible();
                });
    }
    else
    {
        m_objContext.RunPendingOperationIfPossible();
        // TODO: b/388210264 - common fix is required.
        // A pending operation can delete EstablishedState.
        // So, do not add any operation after this line.
    }
}

PUBLIC VIRTUAL CallStateName EstablishedState::Hold(IN MediaInfo& objMediaInfo)
{
    if (ShouldPendOperation())
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [objMediaInfo](IMtcCallState* pState) mutable
                {
                    return pState->Hold(objMediaInfo);
                });
        return GetStateName();
    }

    const MediaInfo& objOldMediaInfo = m_objContext.GetMediaManager().GetMediaInfo();
    if (objOldMediaInfo.eAudioDirection == DIRECTION_INACTIVE)
    {
        m_objContext.SetHeldByMe(IMS_TRUE);
        m_objContext.GetUiNotifier().SendHeld();
        return GetStateName();
    }

    if (!UpdatingInfo::IsValidHoldDirection(
                objOldMediaInfo.eAudioDirection, objMediaInfo.eAudioDirection) ||
            HandleUpdate(UpdateType::HOLD, m_objContext.GetSession()->GetCallType(),
                    objMediaInfo) == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendHoldFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED));
        return GetStateName();
    }
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Resume(IN MediaInfo& objMediaInfo)
{
    if (ShouldPendOperation())
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [objMediaInfo](IMtcCallState* pState) mutable
                {
                    return pState->Resume(objMediaInfo);
                });
        return GetStateName();
    }

    const MediaInfo& objOldMediaInfo = m_objContext.GetMediaManager().GetMediaInfo();
    if (!UpdatingInfo::IsValidResumeDirection(
                objOldMediaInfo.eAudioDirection, objMediaInfo.eAudioDirection) ||
            HandleUpdate(UpdateType::RESUME, m_objContext.GetSession()->GetCallType(),
                    objMediaInfo) == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendResumeFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED));
        return GetStateName();
    }
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Update(
        IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    if (ShouldPendOperation())
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [eCallType, objMediaInfo](IMtcCallState* pState) mutable
                {
                    return pState->Update(eCallType, objMediaInfo);
                });
        return GetStateName();
    }

    if (HandleUpdate(UpdateType::SESSION, eCallType, objMediaInfo) == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendUpdateFailed(
                CallReasonInfo(CODE_SESSION_MODIFICATION_FAILED));
        return GetStateName();
    }
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::Terminate(IN const CallReasonInfo& objReason)
{
    // SetTerminateCodeForInvitedSessionToConf

    const CallReasonInfo objTerminateReason = GetAudioInactivityReasonOnTermination(objReason);

    HandleTerminate(objTerminateReason);
    m_objContext.GetUiNotifier().SendTerminated(objTerminateReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SessionTerminated(IN ISession* piSession)
{
    m_objContext.GetUiNotifier().SendTerminated(IsConferenceCallParticipant()
                    ? CallReasonInfo(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE)
                    : TerminationHandler(m_objContext).Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SessionUpdateReceived(IN ISession* piSession)
{
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_UPDATE);

    IMtcSession* pSession = m_objContext.GetSession();
    pSession->HandleRequest(RequestType::UPDATE, *piMessage);
    m_objContext.GetUpdatingInfo().GetOriginalInfo() =
            m_objContext.GetMediaManager().GetMediaInfo();
    m_objContext.GetUpdatingInfo().SetTargetCallType(
            m_objContext.GetMessageUtils().GetCallType(piMessage, piSession, IMS_TRUE));

    // TODO, conference

    CallStateName eStateName = CallStateName::UPDATING;
    CallReasonInfo objResultReason(CODE_NONE);
    if (m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        auto pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
                m_objContext.CreateBlockChecker(GetCallUpdateBlockRules()));
        IMtcBlockChecker::Result objResult = pBlockChecker->Check();

        if (objResult.eStatus == IMtcBlockChecker::Result::Status::UNBLOCKED)
        {
            objResultReason = HandleReceivedUpdate(eStateName);
        }
        else
        {
            objResultReason = objResult.objReason;
        }
    }
    else
    {
        objResultReason = HandleReceivedUpdateWithoutOffer(eStateName);
    }

    if (objResultReason.nCode != CODE_NONE)
    {
        // Restore the CallType that was changed by MtcSession#HandleRequest.
        pSession->SetCallType(pSession->GetPreviousCallType());
        pSession->Reject(objResultReason);
        m_objContext.GetMediaManager().FinalizeSdp(piSession);
        eStateName = CallStateName::ESTABLISHED;
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

    m_objContext.GetUiNotifier().SendTerminated(
            TerminationHandler(m_objContext).Handle(*piSession));

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::SendUssd(IN const AString& strUssd)
{
    IMS_TRACE_D("SendUssi", 0, 0, 0);

    SendInfoForUssi(strUssd);
    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::UssiInfoReceived(
        IN ISession* /*piSession*/, IN ISipServerConnection* piSipServerConnection)
{
    IMS_TRACE_D("UssiInfoReceived", 0, 0, 0);
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

PUBLIC VIRTUAL CallStateName EstablishedState::Refresh_NotifyCompleted(
        IN ISipClientConnection* /*piScc*/)
{
    m_objContext.RunAsyncOperation(this,
            [&]()
            {
                m_objContext.RunPendingOperationIfPossible();
            });

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnReceivingMediaDataFailed(
        IN IMS_UINT32 eMediaType, IN [[maybe_unused]] IMS_UINT32 eProtocolType)
{
    if (eMediaType == MEDIATYPE_AUDIO)
    {
        IMS_SINT32 nReasonCode =
                m_objContext.GetService().IsWlanIpCanType() ? CODE_WIFI_LOST : CODE_MEDIA_NO_DATA;
        CallReasonInfo objReason(nReasonCode);
        HandleTerminate(objReason);
        m_objContext.GetUiNotifier().SendTerminated(objReason);
        return CallStateName::TERMINATING;
    }

    CallType eCallType = m_objContext.GetSession()->GetCallType();
    if ((eMediaType == MEDIATYPE_VIDEO && eCallType == CallType::VT) ||
            (eMediaType == MEDIATYPE_TEXT && eCallType == CallType::RTT))
    {
        return Downgrade(CallType::VOIP);
    }
    // TODO: check VIDEO_RTT case

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnVideoLowestBitRate()
{
    CallType eCallType = m_objContext.GetSession()->GetCallType();
    if (eCallType == CallType::VT)
    {
        return Downgrade(CallType::VOIP);
    }

    if (eCallType == CallType::VIDEO_RTT)
    {
        return Downgrade(CallType::RTT);
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnReceivingNetworkToneStarted()
{
    m_objContext.GetUiNotifier().SendHeldBy();

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnReceivingNetworkToneFailed()
{
    m_objContext.GetUiNotifier().SendHeldBy();

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    HandleTerminate(objReason);
    m_objContext.GetUiNotifier().SendTerminated(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::QosReserveFailed(
        IN ISession* /* piSession */, IN QosLossPolicy eNextAction)
{
    if (eNextAction == QosLossPolicy::RELEASE)
    {
        CallReasonInfo objReason(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
        HandleTerminate(objReason);
        m_objContext.GetUiNotifier().SendTerminated(objReason);

        return CallStateName::TERMINATING;
    }

    // For the case that QosReserveFailed() is called by MtcPendingOperationHolder after downgraded.
    if (eNextAction == QosLossPolicy::MODIFY &&
            m_objContext.GetSession()->GetCallType() != CallType::VOIP)
    {
        return Downgrade(CallType::VOIP);
    }

    return GetStateName();
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnIpcanChanged(IN IMS_UINT32 eIpcan)
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_ENABLE_SEND_REINVITE_ON_RAT_CHANGE_BOOL))
    {
        return GetStateName();
    }

    if (ShouldPendOperation())
    {
        m_objContext.GetPendingOperationHolder().PushPendingOperation(
                [eIpcan](IMtcCallState* pState)
                {
                    return pState->OnIpcanChanged(eIpcan);
                });
        return GetStateName();
    }

    if (HandleUpdate(UpdateType::SESSION, m_objContext.GetSession()->GetCallType(),
                m_objContext.GetMediaManager().GetMediaInfo()) == IMS_FAILURE)
    {
        return GetStateName();
    }
    return CallStateName::UPDATING;
}

PUBLIC VIRTUAL CallStateName EstablishedState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_DELAY_UPDATE_AFTER_CONNECTED:
            m_objContext.RunAsyncOperation(this,
                    [&]()
                    {
                        m_objContext.RunPendingOperationIfPossible();
                    });
            break;
        default:
            break;
    }

    return GetStateName();
}

PROTECTED VIRTUAL CallStateName EstablishedState::SendUpdateBySrvcc(IN UpdateType eType)
{
    IMtcSession* piMtcSession = m_objContext.GetSession();
    if (piMtcSession == IMS_NULL)
    {
        return GetStateName();
    }

    // TODO: check ShouldPendOperation in MtcCallState#OnSrvccStateUpdated?
    // Handling in UpdatingState is also needed.
    if (HandleUpdate(eType, m_objContext.GetSession()->GetCallType(),
                m_objContext.GetMediaManager().GetMediaInfo()) == IMS_FAILURE)
    {
        return GetStateName();
    }
    return CallStateName::UPDATING;
}

PRIVATE
IMS_RESULT EstablishedState::HandleUpdate(
        IN UpdateType eUpdateType, IN CallType eCallType, IN const MediaInfo& objMediaInfo)
{
    IMS_TRACE_D("HandleUpdate Type[%s]", MtcCallStringUtils::ConvertUpdateType(eUpdateType), 0, 0);
    m_objContext.GetUpdatingInfo().SetTargetCallType(eCallType);
    m_objContext.GetUpdatingInfo().SetModifier();
    m_objContext.GetUpdatingInfo().SetRequestingType(eUpdateType);

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    m_objContext.GetUpdatingInfo().GetOriginalInfo() = objMediaManager.GetMediaInfo();
    objMediaManager.SetMediaInfo(objMediaInfo);

    IMtcSession* pSession = m_objContext.GetSession();
    pSession->SetCallType(eCallType);

    m_objContext.GetUpdatingInfo().GetModifyingInfo() = objMediaManager.GetMediaInfo();

    if (pSession->Update(eUpdateType, IMS_FALSE, SipMethod::INVITE) == IMS_FAILURE)
    {
        pSession->SetCallType(pSession->GetPreviousCallType());
        m_objContext.GetMediaManager().RestoreSdp(&pSession->GetISession());
        m_objContext.DeleteUpdatingInfo();
        return IMS_FAILURE;
    }

    if (!m_objContext.GetTimer().IsActive(TIMER_CONVERT_REMOTE_RESPONSE))
    {
        m_objContext.GetTimer().Start(TIMER_CONVERT_REMOTE_RESPONSE,
                m_objContext.GetConfigurationProxy().GetInt(
                        ConfigVt::KEY_CONVERT_REMOTE_RESPONSE_TIMER_MILLIS_INT));
    }

    return IMS_SUCCESS;
}

PRIVATE
CallReasonInfo EstablishedState::HandleReceivedUpdate(OUT CallStateName& eStateName)
{
    IMS_TRACE_D("HandleReceivedUpdate", 0, 0, 0);
    IMtcSession* pMtcSession = m_objContext.GetSession();
    ISession& objSession = pMtcSession->GetISession();
    NegotiationResult eNegoResult = m_objContext.GetMediaManager().NegotiateSdp(&objSession);
    if (eNegoResult != NegotiationResult::NO_ERROR)
    {
        return CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE, eNegoResult);
    }

    m_objContext.GetUpdatingInfo().GetAlertingInfo() =
            m_objContext.GetMediaManager().GetMediaInfo();
    m_objContext.GetPreconditionManager().OnSdpReceived(&objSession);

    eStateName = CallStateName::UPDATING;

    if (m_objContext.GetUpdatingInfo().IsNeedToAlert())
    {
        if (UpdatingState::IsPreconditionRequired(
                    m_objContext.GetConfigurationProxy(), m_objContext.GetUpdatingInfo()))
        {
            // re-INVITE for update call type is just received.
            m_objContext.GetSession()->SendProvisionalResponse(IMS_FALSE, IsRprRequired());

            // No QoS wait timer is used for Upgrade media.
            // And, TIMER_CONVERT_USER_RESPONSE is started when the precondition negotiation is
            // done. So, to guarantee there is at least one timer to limit UpdatingState, the timer
            // is started here.
            // Once the incoming upgrade is notified to the user, this timer will re-start.
            m_objContext.GetTimer().Start(TIMER_CONVERT_USER_RESPONSE,
                    m_objContext.GetConfigurationProxy().GetInt(
                            ConfigVt::KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT));
        }
        else
        {
            SendIncomingUpdateToUi(
                    m_objContext.GetMediaManager().GetNegotiatedCallType(&objSession));
        }

        return CallReasonInfo(CODE_NONE);
    }

    if (m_objContext.GetUpdatingInfo().IsResumedBy() &&
            m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_CHECK_UI_CONDITION_FOR_INCOMING_RESUME_BOOL))
    {
        m_objContext.GetUiNotifier().SendIncomingResume();
        m_objContext.GetTimer().Start(TIMER_CONVERT_USER_RESPONSE,
                m_objContext.GetConfigurationProxy().GetInt(
                        ConfigVt::KEY_CONVERT_USER_RESPONSE_TIMER_MILLIS_INT));

        return CallReasonInfo(CODE_NONE);
    }

    IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    if (piMessage != IMS_NULL && piMessage->GetMethod().Equals(SipMethod::UPDATE))
    {
        eStateName = CallStateName::ESTABLISHED;
    }

    m_objContext.GetMediaManager().AdjustDirectionForAutoAnswer();
    m_objContext.GetUpdatingInfo().GetModifiedInfo() =
            m_objContext.GetMediaManager().GetMediaInfo();

    if (pMtcSession->AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo EstablishedState::HandleReceivedUpdateWithoutOffer(OUT CallStateName& eStateName)
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
        m_objContext.GetMediaManager().AdjustDirectionForAutoOffer(
                m_objContext.GetSession()->GetCallType());
        m_objContext.GetUpdatingInfo().GetModifyingInfo() =
                m_objContext.GetMediaManager().GetMediaInfo();
    }

    if (m_objContext.GetSession()->AcceptUpdate() == IMS_FAILURE)
    {
        // TODO
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
IMS_BOOL EstablishedState::IsConferenceCallParticipant() const
{
    ImsList<IMtcCall*> objConfCalls = m_objContext.GetCallManager().GetCallsInConference();
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
ImsList<IMtcBlockRule*> EstablishedState::GetCallUpdateBlockRules() const
{
    // No pending rules
    ImsList<IMtcBlockRule*> lstRules;

    lstRules.Append(new CallTypeBlockRule(m_objContext));
    lstRules.Append(new SrvccBlockRule(m_objContext.GetService().GetSrvccState()));
    return lstRules;
}

PRIVATE
CallStateName EstablishedState::Downgrade(IN CallType eCallType)
{
    IMS_TRACE_I("Downgrade [%s]", MtcCallStringUtils::ConvertCallType(eCallType), 0, 0);

    MediaInfo objNewMediaInfo = m_objContext.GetMediaManager().GetMediaInfo();

    // Assumption : no case to downgrade from VIDEO_RTT to VOIP directly
    if (eCallType == CallType::VOIP || eCallType == CallType::VT)
    {
        objNewMediaInfo.eTextDirection = DIRECTION_INVALID;
        objNewMediaInfo.eGttMode = GTT_MODE_INVALID;
    }

    if (eCallType == CallType::VOIP || eCallType == CallType::RTT)
    {
        objNewMediaInfo.eVideoDirection = DIRECTION_INVALID;
        objNewMediaInfo.eVideoQuality = VIDEO_QUALITY_NONE;
    }

    return Update(eCallType, objNewMediaInfo);
}

PRIVATE
IMS_BOOL EstablishedState::ShouldPendOperation() const
{
    IMtcSession* piMtcSession = m_objContext.GetSession();
    return (piMtcSession && piMtcSession->GetISession().IsSessionRefreshInProgress()) ||
            m_objContext.GetTimer().IsActive(TIMER_DELAY_UPDATE_AFTER_CONNECTED);
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
