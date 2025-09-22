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

#include "AString.h"
#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IImsAosInfo.h"
#include "IMessage.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsVector.h"
#include "IuMtcService.h"
#include "MediaDef.h"
#include "MtcDef.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/IMtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/block/CallCountBlockRule.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/CallWaitingBlockRule.h"
#include "call/block/CsCallBlockRule.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/IncomingCallBarringBlockRule.h"
#include "call/block/LocationBlockRule.h"
#include "call/block/ProcessingCallBlockRule.h"
#include "call/block/RadioBlockRule.h"
#include "call/block/RetryAfterBlockRule.h"
#include "call/block/ServiceBlockRule.h"
#include "call/block/SsacBlockRule.h"
#include "call/block/VopsBlockRule.h"
#include "call/block/WfcBlockRule.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/IdleState.h"
#include "call/termination/CancelHandler.h"
#include "conferencecall/ConferenceDef.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "dialogevent/IMultiEndpointManager.h"
#include "helper/IMtcAosConnector.h"
#include "helper/LastComeFirstServedHelper.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "precondition/IMtcPreconditionManager.h"
#include "ussi/UssiController.h"
#include "utility/IMessageUtils.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
IdleState::IdleState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::IDLE, objContext),
        m_pBlockChecker(nullptr),
        m_objOperationAfterBlockCheck(nullptr),
        m_pConfUsers(ImsList<std::shared_ptr<ConfUser>>())
{
}

PUBLIC VIRTUAL IdleState::~IdleState() {}

PUBLIC VIRTUAL CallStateName IdleState::Start(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices)
{
    IMS_TRACE_D("Start [%s]", strTarget.GetStr(), 0, 0);
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);
    MediaInfo objMediaInfoToStart;
    if (IsCallPull())
    {
        if (HandleCallPull(objMediaInfoToStart) == IMS_FAILURE)
        {
            return CallStateName::TERMINATING;
        }
    }
    else
    {
        m_objContext.GetCallInfo().eInitialCallType = eCallType;
        objMediaInfoToStart = objMediaInfo;
    }

    m_objContext.GetCallInfo().ePeerType = PeerType::MO;

    if (m_objContext.IsUssi())
    {
        m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
        m_objOperationAfterBlockCheck = [&, objMediaInfoToStart]()
        {
            return ContinueStartUssi(objMediaInfoToStart);
        };
    }
    else
    {
        m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(
                RemoveCallerIdServiceCodeAndUpdateSuppService(strTarget));
        m_objOperationAfterBlockCheck = [&, objMediaInfoToStart]()
        {
            return ContinueStart(objMediaInfoToStart);
        };
    }

    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(IN CallType eCallType,
        IN const AString& strTarget, IN MediaInfo& objMediaInfo,
        IN const ImsList<SuppService*>& objSuppServices, IN const ImsList<ConfUser*>& lstUsers)
{
    m_objContext.GetSupplementaryService().UpdateOutgoingServices(objSuppServices);
    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetCallInfo().bConference = IMS_TRUE;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);

    CopyConfUserListForAsynchronousHandling(lstUsers);

    m_objOperationAfterBlockCheck = [&, objMediaInfo]()
    {
        return ContinueConference(objMediaInfo);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::StartConference(
        IN CallType eCallType, IN const AString& strTarget, IN const ImsList<ConfUser*>& lstUsers)
{
    m_objContext.GetCallInfo().eInitialCallType = eCallType;
    m_objContext.GetCallInfo().ePeerType = PeerType::MO;
    m_objContext.GetCallInfo().bConference = IMS_TRUE;
    m_objContext.GetParticipantInfo().UpdateFromRemoteNumber(strTarget);
    IMS_SINT32 nVideoDirection = DIRECTION_INVALID;
    if (eCallType == CallType::VT)
    {
        nVideoDirection = DIRECTION_SEND_RECEIVE;
    }
    MediaInfo objMediaInfo(DIRECTION_SEND_RECEIVE, nVideoDirection, DIRECTION_INVALID,
            AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID);

    CopyConfUserListForAsynchronousHandling(lstUsers);

    m_objOperationAfterBlockCheck = [&, objMediaInfo]()
    {
        return ContinueConference(objMediaInfo);
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetOutgoingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::HandleIncoming(IN ISession* piSession)
{
    m_objContext.GetCallInfo().eInitialCallType = CallType::UNKNOWN;
    m_objContext.GetCallInfo().ePeerType = PeerType::MT;

    IMtcSession* pSession = m_objContext.CreateSession(piSession);
    if (pSession == IMS_NULL)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
    }

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
    pSession->HandleRequest(RequestType::START, *piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(RequestType::START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);

    AString strNotSupportedExtension;
    if (!pSession->GetExtensionSet().IsSupportRequiredExtensions(
                *piMessage, strNotSupportedExtension))
    {
        return RejectIncomingAndToTerminating(
                CallReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strNotSupportedExtension));
    }

    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL) &&
            !m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    StartTimer(TIMER_MT_ALERTING);

    if (LastComeFirstServedHelper::IsSupported(m_objContext.GetConfigurationProxy()))
    {
        m_objContext.GetLastComeFirstServedHelper().OnCallReceived(m_objContext.GetCallKey());
    }

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueHandleIncoming();
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetIncomingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::Terminate(IN const CallReasonInfo& objReason)
{
    m_objContext.GetUiNotifier().SendStartFailed(objReason);

    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IdleState::SessionTerminated(IN ISession* piSession)
{
    const IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_TERMINATE);

    CallReasonInfo objReason = piMessage ? CancelHandler(m_objContext).Handle(*piMessage)
                                         : CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR);

    m_objContext.GetUiNotifier().SendIncomingCallRejected(objReason);
    return CallStateName::TERMINATING;
}

PUBLIC VIRTUAL CallStateName IdleState::OnBlockChecked(IN IMtcBlockChecker::Result objResult)
{
    switch (objResult.eStatus)
    {
        case IMtcBlockChecker::Result::Status::UNBLOCKED:
            m_pBlockChecker.reset();
            return m_objOperationAfterBlockCheck();

        case IMtcBlockChecker::Result::Status::BLOCKED:
            m_pBlockChecker.reset();

            if (EpsFallbackTrigger::ShouldTriggerByReasonInfo(m_objContext, objResult.objReason))
            {
                m_objContext.GetEpsFallbackTrigger().TriggerEpsFallback(
                        EpsFallbackReason::RADIO_CHECK_BLOCK);
                return GetStateName();
            }

            if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
            {
                m_objContext.GetSession()->Reject(objResult.objReason);
                m_objContext.GetUiNotifier().SendIncomingCallRejected(objResult.objReason);
            }
            else
            {
                m_objContext.GetUiNotifier().SendStartFailed(
                        m_objContext.GetCallInfo().IsEmergency()
                                ? CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                                          EXTRA_CODE_CALL_RETRY_EMERGENCY)
                                : objResult.objReason.ConvertFromInternal());
            }

            StopTimer(TIMER_MT_ALERTING);
            return CallStateName::TERMINATING;

        default:  // IMtcBlockChecker::Result::Status::PENDING:
            return GetStateName();
    }
}

PUBLIC VIRTUAL CallStateName IdleState::OnAttached()
{
    ISession* piSession = GetISession();
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    InitMediaSession(MediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_INVALID, DIRECTION_INVALID,
            AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID));
    m_objContext.GetPreconditionManager().CreateQos(piSession);

    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(eCallReason));
    }

    if (m_objContext.GetMessageUtils().HasSdp(piMessage) == IMS_FALSE)
    {
        m_objContext.GetMediaManager().AdjustDirectionForAutoOffer(
                piSession, m_objContext.GetSession()->GetCallType());
    }

    m_objContext.GetPreconditionManager().OnMessageReceived(piSession, piMessage);

    if (m_objContext.GetSession()->GetExtensionSet().IsAvailableOnBoth(
                MtcExtensionSet::OPTION_TAG_RPR))
    {
        if (m_objContext.GetSession()->SendProvisionalResponse(IMS_FALSE, IMS_TRUE) == IMS_FAILURE)
        {
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        }
        StartTimer(TIMER_MT_PRACK_WAIT);
    }
    else
    {
        return OnReadyToAlert();
    }

    StartEpsFallbackWatchdogIfNeeded(*piSession->GetPreviousResponse(IMessage::SESSION_START));
    return CallStateName::INCOMING;
}

PUBLIC VIRTUAL CallStateName IdleState::OnTimerExpired(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case TIMER_MT_ALERTING:
            IMS_TRACE_D("TIMER_MT_ALERTING expires in IdleState", 0, 0, 0);
            return RejectIncomingAndToTerminating(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR));
        default:
            break;
    }

    return GetStateName();
}

PROTECTED VIRTUAL CallStateName IdleState::HandleAosConnected()
{
    if (m_objContext.GetEpsFallbackTrigger().IsWaitingRegistration())
    {
        m_objContext.GetEpsFallbackTrigger().OnEpsFallbackCompleted();

        m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
                m_objContext.CreateBlockChecker(GetBlockRulesAfterEpsFallback()));
        return OnBlockChecked(m_pBlockChecker->Check());
    }

    return GetStateName();
}

PROTECTED VIRTUAL const CallReasonInfo IdleState::GetCallReasonInfoByAosDisconnection(
        IN IMS_UINT32 nAosReason, IN IMS_SINT32 nDataFailureReason) const
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY,
                EXTRA_MESSAGE_AOS_DISCONNECTED);
    }

    return MtcCallState::GetCallReasonInfoByAosDisconnection(nAosReason, nDataFailureReason);
}

PUBLIC VIRTUAL CallStateName IdleState::HandleIncomingUssi(IN ISession* piSession)
{
    IMS_TRACE_D("HandleIncomingUssi", 0, 0, 0);

    m_objContext.GetCallInfo().ePeerType = PeerType::MT;
    m_objContext.GetCallInfo().bUssi = IMS_TRUE;
    m_objContext.CreateSession(piSession);

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_REJECT_OFFERLESS_INVITE_BOOL) &&
            !m_objContext.GetMessageUtils().HasSdp(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE));
    }

    if (!m_objContext.GetUssiController()->HasValidXmlBodyForNetworkInitiatedUssi(piMessage))
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(CODE_REJECT_UNKNOWN));
    }

    m_objOperationAfterBlockCheck = [&]()
    {
        return ContinueHandleIncoming();
    };
    m_pBlockChecker = std::unique_ptr<IMtcBlockChecker>(
            m_objContext.CreateBlockChecker(GetIncomingCallBlockRules()));
    return OnBlockChecked(m_pBlockChecker->Check());
}

PUBLIC VIRTUAL CallStateName IdleState::OnUssiAttached()
{
    IMS_TRACE_D("OnUssiAttached", 0, 0, 0);

    ISession* piSession = GetISession();

    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);

    // TODO: need to check
    m_objContext.GetSession()->HandleRequest(RequestType::START, *piMessage);
    m_objContext.GetSupplementaryService().UpdateIncomingServices(piMessage);
    m_objContext.GetParticipantInfo().HandleRequest(RequestType::START, *piMessage);

    AString strNotSupportedExtension;
    if (!m_objContext.GetSession()->GetExtensionSet().IsSupportRequiredExtensions(
                *piMessage, strNotSupportedExtension))
    {
        return RejectIncomingAndToTerminating(
                CallReasonInfo(CODE_REJECT_UNSUPPORTED_SIP_HEADERS, -1, strNotSupportedExtension));
    }

    m_objContext.GetCallInfo().eInitialCallType = m_objContext.GetSession()->GetCallType();
    InitMediaSession(MediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_INVALID, DIRECTION_INVALID,
            AUDIO_QUALITY_NONE, VIDEO_QUALITY_NONE, GTT_MODE_INVALID));

    IMS_SINT32 eCallReason = HandleReceivedSdp(piSession, piMessage);
    if (eCallReason != CODE_NONE)
    {
        return RejectIncomingAndToTerminating(CallReasonInfo(eCallReason));
    }

    return OnReadyToAlert();
}

PRIVATE
CallStateName IdleState::ContinueStart(IN const MediaInfo& objMediaInfo)
{
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(GetInternalErrorReason());
        return CallStateName::TERMINATING;
    }

    InitMediaSession(objMediaInfo);

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(GetInternalErrorReason());
        return CallStateName::TERMINATING;
    }

    m_objContext.GetUiNotifier().SendInitiating();
    StartTimer(TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        if (!m_objContext.GetTimer().IsActive(TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
        {
            StartTimer(TIMER_MO_CALL_INITIATION_TO_18X_WAIT);
        }
    }

    return CallStateName::OUTGOING;
}

PRIVATE
CallStateName IdleState::ContinueConference(IN const MediaInfo& objMediaInfo)
{
    IMS_TRACE_D("ContinueConference UserSize[%d]", m_pConfUsers.GetSize(), 0, 0);
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }
    SetResourceListForConference(*GetISession()->GetNextRequest());

    InitMediaSession(objMediaInfo);

    m_objContext.GetPreconditionManager().CreateQos(GetISession());

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    m_objContext.GetUiNotifier().SendInitiating();
    StartTimer(TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);

    return CallStateName::OUTGOING;
}

PRIVATE
CallStateName IdleState::ContinueHandleIncoming()
{
    IMS_TRACE_D("ContinueHandleIncoming", 0, 0, 0);

    m_objContext.GetUiNotifier().SendPreIncomingCallReceived();

    return GetStateName();
}

PRIVATE
CallStateName IdleState::ContinueStartUssi(IN const MediaInfo& objMediaInfo)
{
    IMS_TRACE_D("ContinueStartUssi", 0, 0, 0);
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    if (m_objContext.CreateSession() == IMS_NULL)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    InitMediaSession(objMediaInfo);

    if (m_objContext.GetUssiController()->FormStartUssiRequest(
                m_objContext.GetParticipantInfo().GetRemoteNumber()) == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    IMS_UINT32 eMediaTypes =
            MtcMediaUtil::GetMediaTypesFromCallType(m_objContext.GetCallInfo().eInitialCallType);
    objMediaManager.SetRtpPort(GetISession(), eMediaTypes, 0);

    if (m_objContext.GetSession()->Start() == IMS_FAILURE)
    {
        m_objContext.GetUiNotifier().SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR));
        return CallStateName::TERMINATING;
    }

    m_objContext.GetUiNotifier().SendInitiating();
    StartTimer(TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    return CallStateName::OUTGOING;
}

PRIVATE
void IdleState::SetResourceListForConference(IN_OUT IMessage& objMessage)
{
    if (m_pConfUsers.GetSize() == 0)
    {
        return;
    }
    objMessage.AddHeader(SipHeaderName::CONTENT_TYPE, "multipart/mixed");

    ImsList<ConfUser*> objUsers;
    for (IMS_UINT32 i = 0; i < m_pConfUsers.GetSize(); i++)
    {
        objUsers.Append(m_pConfUsers.GetAt(i).get());
    }

    // TODO: LGU needs to set false the 5th param.
    m_objContext.GetMessageUtils().SetResourceList(
            &objMessage, m_objContext, objUsers, IMS_TRUE, IMS_TRUE);
}

PRIVATE
ImsList<IMtcBlockRule*> IdleState::GetIncomingCallBlockRules()
{
    CallType eCallType = m_objContext.GetSession()->GetCallType();

    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(new IncomingCallBarringBlockRule(m_objContext, eCallType));
    lstRules.Append(new VopsBlockRule(m_objContext));
    lstRules.Append(new WfcBlockRule(m_objContext, eCallType));
    lstRules.Append(new ServiceBlockRule(m_objContext, eCallType));
    lstRules.Append(new ProcessingCallBlockRule(m_objContext));
    lstRules.Append(new CsCallBlockRule(m_objContext));
    lstRules.Append(new CallCountBlockRule(m_objContext));
    lstRules.Append(new CallTypeBlockRule(m_objContext, eCallType));
    lstRules.Append(new CallWaitingBlockRule(m_objContext));
    lstRules.Append(new SsacBlockRule(m_objContext, eCallType));
    lstRules.Append(new RadioBlockRule(m_objContext, eCallType));
    return lstRules;
}

PRIVATE
ImsList<IMtcBlockRule*> IdleState::GetOutgoingCallBlockRules()
{
    CallType eCallType = m_objContext.GetCallInfo().eInitialCallType;

    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(new RadioBlockRule(m_objContext, eCallType));
    lstRules.Append(new LocationBlockRule(m_objContext));
    lstRules.Append(new RetryAfterBlockRule(m_objContext));
    lstRules.Append(new ServiceBlockRule(m_objContext, eCallType));
    lstRules.Append(new ProcessingCallBlockRule(m_objContext));
    lstRules.Append(new CsCallBlockRule(m_objContext));
    lstRules.Append(new CallCountBlockRule(m_objContext));
    lstRules.Append(new SsacBlockRule(m_objContext, eCallType));
    return lstRules;
}

PRIVATE
ImsList<IMtcBlockRule*> IdleState::GetBlockRulesAfterEpsFallback()
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(new SsacBlockRule(m_objContext, m_objContext.GetCallInfo().eInitialCallType));
    lstRules.Append(new RadioBlockRule(m_objContext, m_objContext.GetCallInfo().eInitialCallType));
    return lstRules;
}

PRIVATE
IMS_BOOL IdleState::IsCallPull() const
{
    const SuppService* pSs = m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL);
    return pSs && pSs->nValue > 0;
}

PRIVATE
IMS_RESULT IdleState::HandleCallPull(OUT MediaInfo& objMediaInfo)
{
    const IMultiEndpointManager* piMultiEndpointManager = m_objContext.GetMultiEndpointManager();
    if (!piMultiEndpointManager)
    {
        Terminate(CallReasonInfo(CODE_MULTIENDPOINT_NOT_SUPPORTED));
        return IMS_FAILURE;
    }

    IMultiEndpointManager::PullingDialogInfo objDialogInfo = piMultiEndpointManager->GetDialogInfo(
            m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL)->nValue);

    if (objDialogInfo.strCallId.GetLength() <= 0 || objDialogInfo.strLocalTag.GetLength() <= 0 ||
            objDialogInfo.strRemoteTag.GetLength() <= 0)
    {
        Terminate(CallReasonInfo(CODE_CALL_PULL_OUT_OF_SYNC));
        return IMS_FAILURE;
    }

    m_objContext.GetCallInfo().eInitialCallType = objDialogInfo.eCallType;
    objMediaInfo = *objDialogInfo.pMediaInfo;

    return IMS_SUCCESS;
}

PRIVATE
void IdleState::CopyConfUserListForAsynchronousHandling(const ImsList<ConfUser*> objUsers)
{
    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        m_pConfUsers.Append(std::make_shared<ConfUser>(*objUsers.GetAt(i)));
    }
}

PRIVATE
AString IdleState::RemoveCallerIdServiceCodeAndUpdateSuppService(IN const AString& strTarget)
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        // TODO: b/382332088
        // imsemergency.caller_id_service_codes_for_restriction_string_array for JP carriers.
        return strTarget;
    }

    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_INCLUDE_CALLER_ID_SERVICE_CODES_IN_SIP_INVITE_BOOL))
    {
        return strTarget;
    }

    ImsVector<AString> objCodeRestricted = m_objContext.GetConfigurationProxy().GetStringArray(
            ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_RESTRICTION_STRING_ARRAY);
    for (IMS_UINT32 i = 0; i < objCodeRestricted.GetSize(); i++)
    {
        if (strTarget.StartsWith(objCodeRestricted.GetAt(i)))
        {
            IMS_TRACE_D("dialed number includes caller id for restriction : ", 0, 0, 0);
            m_objContext.GetSupplementaryService().Add(SuppType::CALLER_ID, CALLERID_RESTRICTED);
            return AString(strTarget.GetSubStr(objCodeRestricted.GetAt(i).GetLength()));
        }
    }

    ImsVector<AString> objCodeIdentity = m_objContext.GetConfigurationProxy().GetStringArray(
            ConfigVoice::KEY_CALLER_ID_SERVICE_CODES_FOR_IDENTITY_STRING_ARRAY);
    for (IMS_UINT32 i = 0; i < objCodeIdentity.GetSize(); i++)
    {
        if (strTarget.StartsWith(objCodeIdentity.GetAt(i)))
        {
            IMS_TRACE_D("dialed number includes caller id for identity : ", 0, 0, 0);
            m_objContext.GetSupplementaryService().Add(SuppType::CALLER_ID, CALLERID_IDENTITY);
            return AString(strTarget.GetSubStr(objCodeIdentity.GetAt(i).GetLength()));
        }
    }

    return strTarget;
}

PRIVATE
const CallReasonInfo IdleState::GetInternalErrorReason() const
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
    }
    else
    {
        return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    }
}
