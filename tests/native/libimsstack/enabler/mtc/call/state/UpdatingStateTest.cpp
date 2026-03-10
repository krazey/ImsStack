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
#include "Engine.h"
#include "IConfiguration.h"
#include "ISession.h"
#include "ImsVector.h"
#include "MediaDef.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MtcDef.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/UpdatingInfo.h"
#include "call/state/MtcCallState.h"
#include "call/state/UpdatingState.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <initializer_list>

using ::testing::_;
using ::testing::InSequence;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

MATCHER_P(IsEqualCallReason, reason, "")
{
    return arg == reason;
}

class UpdatingStateTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockMtcConfigurationProxy* pConfigurationProxy;
    UpdatingInfo* pUpdatingInfo;
    UpdatingState* pUpdatingState;
    CallInfo objCallInfo;
    MockIMtcMediaManager objMediaManager;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMtcUiNotifier objUiNotifier;
    MtcSupplementaryService* pMtcSupplementaryService;
    MockIMtcPreconditionManager objMtcPreconditionManager;
    MockIMessage objMessage;
    MockMtcTimerWrapper objTimer;
    TestMtcPendingOperationHolder objPendingOperationHolder;
    MockIMessageUtils objMessageUtils;
    MediaInfo objMediaInfo;
    SipMethod* pSipMethod;
    ImsVector<AString> objActionSets;
    MockIMtcService objMtcService;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        pMtcSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        ON_CALL(objContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));

        pUpdatingInfo = new UpdatingInfo(objContext);
        ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMediaManager, GetMediaInfo(Ref(objSession)))
                .WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetSession(&objSession)).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetSupplementaryService())
                .WillByDefault(ReturnRef(*pMtcSupplementaryService));
        ON_CALL(objContext, GetPreconditionManager())
                .WillByDefault(ReturnRef(objMtcPreconditionManager));
        ON_CALL(objContext, GetTimer).WillByDefault(ReturnRef(objTimer));

        ON_CALL(objContext, GetPendingOperationHolder)
                .WillByDefault(ReturnRef(objPendingOperationHolder));

        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMtcService));

        pSipMethod = new SipMethod(SipMethod::ACK);

        pUpdatingState = new UpdatingState(objContext);
    }

    virtual void TearDown() override
    {
        delete pUpdatingState;
        delete pUpdatingInfo;
        delete pConfigurationProxy;
        delete pMtcSupplementaryService;
        delete pSipMethod;
    }

    void SetUpOnSdpReceivedFailed()
    {
        // SetUp OnSdpReceived failed
        ON_CALL(objMediaManager, GetNegotiationState(_))
                .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
        ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(*pSipMethod));
    }

    void SetNegotiateSdpFailure(
            IN MockIMessage& objMessage, IN const SdpNegotiationResult& objNegoResult)
    {
        ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(objMediaManager, GetNegotiationState(&objSession))
                .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
        ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(*pSipMethod));
        ON_CALL(objMediaManager, NegotiateSdp(&objSession)).WillByDefault(Return(objNegoResult));
    }

    void SetActionConfigs(IN IMS_SINT32 nStatusCode, std::initializer_list<IMS_SINT32> objActions)
    {
        AString strActionSet;
        strActionSet.SetNumber(nStatusCode);
        strActionSet += ":";

        bool bFirst = true;
        for (IMS_SINT32 nAction : objActions)
        {
            if (!bFirst)
            {
                strActionSet += ",";
            }
            AString strAction;
            strAction.SetNumber(nAction);
            strActionSet += strAction;
            bFirst = false;
        }

        objActionSets.Add(strActionSet);
        ON_CALL(*pConfigurationProxy,
                GetStringArray(ConfigVoice::KEY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY))
                .WillByDefault(Return(objActionSets));
    }
};

TEST_F(UpdatingStateTest, OnExitDoesNotSendUpdateIfPendingUpdateIsNotSet)
{
    EXPECT_CALL(objMtcSession, Update(_, _, _)).Times(0);

    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitSendsUpdateIfPendingUpdateIsSet)
{
    EXPECT_CALL(objMtcSession, Update(UpdateType::REFRESH, IMS_FALSE, SipMethod::INVALID)).Times(1);

    pUpdatingInfo->SetPendingUpdate();
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitStopsTimerWhenModifier)
{
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_RETRY_UPDATE));

    pUpdatingInfo->SetModifier();
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitStopsTimerWhenAlerted)
{
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE));

    pUpdatingInfo->SetAlerted();
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, HoldPushesPendingOperation)
{
    EXPECT_CALL(objPendingOperationHolder.GetMock(), Hold(objMediaInfo));

    pUpdatingState->Hold(objMediaInfo);
}

TEST_F(UpdatingStateTest, ResumePushesPendingOperation)
{
    EXPECT_CALL(objPendingOperationHolder.GetMock(), Resume(objMediaInfo));

    pUpdatingState->Resume(objMediaInfo);
}

TEST_F(UpdatingStateTest, UpdatePushesPendingOperation)
{
    const CallType eAnyType = CallType::VT;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), Update(eAnyType, objMediaInfo));

    pUpdatingState->Update(eAnyType, objMediaInfo);
}

TEST_F(UpdatingStateTest, AcceptUpdateReturnsEstablishedWhenISessionStateEstablished)
{
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMtcSession, SetCallType(CallType::VIDEO_RTT)).Times(1);
    EXPECT_CALL(objMediaManager, Run(&objSession, _, IMS_FALSE)).Times(1);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), _)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession)).Times(1);
    EXPECT_CALL(objUiNotifier, SendUpdated).Times(1);

    EXPECT_EQ(CallStateName::ESTABLISHED,
            pUpdatingState->AcceptUpdate(CallType::VIDEO_RTT, objMediaInfo));
}

TEST_F(UpdatingStateTest, AcceptUpdateReturnsEstablishedWhenPreviousRequestIsUpdate)
{
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), _)).Times(1);
    EXPECT_CALL(objMediaManager, AdjustDirectionForAutoAnswer(Ref(objSession))).Times(1);
    EXPECT_CALL(objMtcSession, SetCallType(_)).Times(1);
    EXPECT_CALL(objMtcSession, AcceptUpdate()).Times(1);
    EXPECT_CALL(objSession, GetPreviousRequest(_)).Times(1).WillOnce(Return(&objMessage));
    SipMethod objSipMethod(SipMethod::UPDATE);
    EXPECT_CALL(objMessage, GetMethod()).Times(1).WillOnce(ReturnRef(objSipMethod));
    EXPECT_CALL(objUiNotifier, SendUpdated).Times(1);

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->AcceptUpdate(CallType::VOIP, objMediaInfo));
}

TEST_F(UpdatingStateTest, AcceptUpdateReturnsUpdating)
{
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    EXPECT_CALL(objMediaManager, AdjustDirectionForAutoAnswer(Ref(objSession))).Times(1);
    EXPECT_CALL(objSession, GetPreviousRequest(_)).Times(1).WillOnce(Return(&objMessage));
    SipMethod objSipMethod(SipMethod::INVITE);
    EXPECT_CALL(objMessage, GetMethod()).Times(1).WillOnce(ReturnRef(objSipMethod));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->AcceptUpdate(CallType::VOIP, objMediaInfo));
}

TEST_F(UpdatingStateTest, RejectUpdateInvokesMtcSessionRejectIfMediaAlreadyNegotiated)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_NEGOTIATING));

    const CallReasonInfo objInfo(CODE_UNSPECIFIED);
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE));
    EXPECT_CALL(objMtcSession, Reject(Ref(objInfo)));
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->RejectUpdate(objInfo));
}

TEST_F(UpdatingStateTest, RejectUpdateInvokesMtcSessionUpdateIfSessionAlreadyEstablished)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));

    const CallReasonInfo objInfo(CODE_UNSPECIFIED);
    EXPECT_CALL(objMtcSession, GetPreviousCallType());
    EXPECT_CALL(objMtcSession, SetCallType(_));
    EXPECT_CALL(objMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE));
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->RejectUpdate(objInfo));
}

TEST_F(UpdatingStateTest, RejectUpdateInvokesMtcSessionRejectIfRejectCodeIsNot200)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(603));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_NEGOTIATING));

    const CallReasonInfo objInfo(CODE_UNSPECIFIED);
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE));
    EXPECT_CALL(objMtcSession, Reject(Ref(objInfo)));
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->RejectUpdate(objInfo));
}

TEST_F(UpdatingStateTest, RejectUpdateInvokesAcceptUpdateIfRejectCodeIs200)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(200));

    ON_CALL(objMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VOIP));
    objContext.GetUpdatingInfo().GetOriginalInfo() = objMediaInfo;

    // Copied from AcceptUpdateReturnsEstablishedWhenPreviousRequestIsUpdate
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), _)).Times(1);
    EXPECT_CALL(objMediaManager, AdjustDirectionForAutoAnswer(Ref(objSession))).Times(1);
    EXPECT_CALL(objMtcSession, SetCallType(_)).Times(1);
    EXPECT_CALL(objMtcSession, AcceptUpdate()).Times(1);
    EXPECT_CALL(objSession, GetPreviousRequest(_)).Times(1).WillOnce(Return(&objMessage));
    SipMethod objSipMethod(SipMethod::UPDATE);
    EXPECT_CALL(objMessage, GetMethod()).Times(1).WillOnce(ReturnRef(objSipMethod));
    EXPECT_CALL(objUiNotifier, SendUpdated).Times(1);

    const CallReasonInfo objInfo(CODE_UNSPECIFIED);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->RejectUpdate(objInfo));
}

TEST_F(UpdatingStateTest, CancelUpdateSendsCancelUpdate)
{
    const CallReasonInfo objReason(CODE_USER_IGNORE);

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE)).Times(1);
    EXPECT_CALL(objMtcSession, CancelUpdate(objReason)).Times(1);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->CancelUpdate(objReason));
}

TEST_F(UpdatingStateTest, AcceptResumeReturnsEstablishedWhenPreviousRequestIsUpdate)
{
    // AcceptResume codes are almost same with AcceptUpdate, so simply checks tiny differences.
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetAlertingInfo().eAudioDirection = DIRECTION_SEND;

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), _)).Times(1);
    EXPECT_CALL(objMtcSession, SetCallType(_)).Times(1);
    EXPECT_CALL(objMtcSession, AcceptUpdate()).Times(1);
    EXPECT_CALL(objSession, GetPreviousRequest(_)).Times(1).WillOnce(Return(&objMessage));
    SipMethod objSipMethod(SipMethod::UPDATE);
    EXPECT_CALL(objMessage, GetMethod()).Times(1).WillOnce(ReturnRef(objSipMethod));
    EXPECT_CALL(objUiNotifier, SendResumedBy).Times(1);

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->AcceptResume(CallType::VOIP, objMediaInfo));
    EXPECT_EQ(DIRECTION_SEND_RECEIVE, objMediaInfo.eAudioDirection);
}

TEST_F(UpdatingStateTest, AcceptResumeReturnsUpdatingWhenPreviousRequestIsNotUpdate)
{
    // AcceptResume codes are almost same with AcceptUpdate, so simply checks tiny differences.
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetAlertingInfo().eAudioDirection = DIRECTION_SEND;

    ON_CALL(objSession, GetPreviousRequest(_)).WillByDefault(Return(&objMessage));
    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMediaManager, SetMediaInfo(Ref(objSession), objMediaInfo)).Times(1);
    EXPECT_CALL(objMtcSession, SetCallType(CallType::VOIP)).Times(1);
    EXPECT_CALL(objMtcSession, AcceptUpdate()).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->AcceptResume(CallType::VOIP, objMediaInfo));
    EXPECT_EQ(DIRECTION_SEND_RECEIVE, objMediaInfo.eAudioDirection);
}

TEST_F(UpdatingStateTest, RejectResumeInvokesReject)
{
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    const CallReasonInfo objInfo(CODE_UNSPECIFIED);
    EXPECT_CALL(objMtcSession, Reject(objInfo)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->RejectResume(objInfo));
}

TEST_F(UpdatingStateTest, OnUserResponseTimerExpiredCallsReject)
{
    MockISession objSession;
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE))).Times(1);

    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_CONVERT_USER_RESPONSE);
}

TEST_F(UpdatingStateTest, OnRemoteResponseTimerExpiredCallsCancelUpdate)
{
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE)).Times(1);
    EXPECT_CALL(objMtcSession, CancelUpdate(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE)))
            .Times(1);

    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE);
}

TEST_F(UpdatingStateTest, TerminateByUserActionWhenNoReceivingAudioPackets)
{
    EXPECT_CALL(objMediaManager, IsAudioInactive).Times(1).WillOnce(Return(IMS_TRUE));

    CallReasonInfo objTerminateReason(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_RTP_TIMEOUT);
    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objTerminateReason)).Times(1);

    CallReasonInfo objReason(CODE_USER_TERMINATED);
    pUpdatingState->Terminate(objReason);

    EXPECT_CALL(objMediaManager, IsAudioInactive).Times(1).WillOnce(Return(IMS_FALSE));

    objTerminateReason.nExtraCode = -1;
    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objTerminateReason)).Times(1);

    pUpdatingState->Terminate(objReason);
}

TEST_F(UpdatingStateTest, SessionTerminatedTerminatesCall)
{
    EXPECT_CALL(objUiNotifier, SendTerminated(_));
    EXPECT_EQ(CallStateName::TERMINATING, pUpdatingState->SessionTerminated(&objSession));
}

TEST_F(UpdatingStateTest, OnReceivingMediaDataFailedWithAudioTerminatesCall)
{
    EXPECT_CALL(objMtcService, IsWlanIpCanType)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))))
            .Times(2);
    EXPECT_CALL(
            objUiNotifier, SendTerminated(IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))))
            .Times(2);

    EXPECT_EQ(CallStateName::TERMINATING,
            pUpdatingState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
    EXPECT_EQ(CallStateName::TERMINATING,
            pUpdatingState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
}

TEST_F(UpdatingStateTest, OnReceivingMediaDataFailedWithAudioOverridesReasonHeader)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigWfc::KEY_OVERRIDE_MEDIA_INACTIVITY_TO_WIFI_LOST_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcService, IsWlanIpCanType)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(
            objMtcSession, Terminate(IMS_TRUE, IsEqualCallReason(CallReasonInfo(CODE_WIFI_LOST))));
    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))));
    EXPECT_CALL(objUiNotifier, SendTerminated(IsEqualCallReason(CallReasonInfo(CODE_WIFI_LOST))));
    EXPECT_CALL(
            objUiNotifier, SendTerminated(IsEqualCallReason(CallReasonInfo(CODE_MEDIA_NO_DATA))));

    EXPECT_EQ(CallStateName::TERMINATING,
            pUpdatingState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
    EXPECT_EQ(CallStateName::TERMINATING,
            pUpdatingState->OnReceivingMediaDataFailed(MEDIATYPE_AUDIO, MEDIA_PROTOCOL_RTP));
}

TEST_F(UpdatingStateTest, OnReceivingMediaDataFailedWithVideoPushesPendingOperation)
{
    EXPECT_CALL(objPendingOperationHolder.GetMock(),
            OnReceivingMediaDataFailed(MEDIATYPE_VIDEO, MEDIA_PROTOCOL_RTP));

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::VT));
    pUpdatingState->OnReceivingMediaDataFailed(MEDIATYPE_VIDEO, MEDIA_PROTOCOL_RTP);
}

TEST_F(UpdatingStateTest, OnVideoLowestBitRatePushesPendingOperation)
{
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnVideoLowestBitRate());
    pUpdatingState->OnVideoLowestBitRate();
}

TEST_F(UpdatingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_MEDIA_INIT_FAILED)))
            .Times(1);

    pUpdatingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(UpdatingStateTest, OnIpcanChangedPushesPendingOperation)
{
    IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(eIpcan));

    pUpdatingState->OnIpcanChanged(eIpcan);
}

TEST_F(UpdatingStateTest, HandleSrvccStartedAsModifier)
{
    pUpdatingInfo->SetModifier();

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE)).Times(1);
    const CallReasonInfo objReason(CODE_LOCAL_CALL_VCC_ON_PROGRESSING);
    EXPECT_CALL(objMtcSession, CancelUpdate(objReason)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->OnSrvccStateUpdated(SrvccState::STARTED));
}

TEST_F(UpdatingStateTest, HandleSrvccStartedAsNotModifier)
{
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_REESTABLISHING));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    const CallReasonInfo objReason(CODE_LOCAL_CALL_VCC_ON_PROGRESSING);
    EXPECT_CALL(objMtcSession, Reject(objReason)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->OnSrvccStateUpdated(SrvccState::STARTED));
}

TEST_F(UpdatingStateTest, OnNormalReasonCodeTerminatesCall)
{
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    EXPECT_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillOnce(Return(nullptr));

    EXPECT_CALL(objUiNotifier, SendTerminated(_));
    EXPECT_EQ(CallStateName::TERMINATING, pUpdatingState->SessionUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, OnInternalRetryUpdateStartsRetryUpdateTimer)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    EXPECT_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillOnce(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    SetActionConfigs(SipStatusCode::SC_491, {ConfigVoice::UPDATE_ERROR_ACTION_GLARE_CONDITION});

    EXPECT_CALL(objMediaManager, RestoreSdp(_));
    EXPECT_CALL(objTimer, Start(MtcCallState::TIMER_RETRY_UPDATE, _));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, OnInternalRetryUpdateStartsRetryUpdateTimerWithStaleUpdateCase)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    EXPECT_CALL(
            objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_STALE_UPDATE, _))
            .WillOnce(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    SetActionConfigs(SipStatusCode::SC_491, {ConfigVoice::UPDATE_ERROR_ACTION_GLARE_CONDITION});

    EXPECT_CALL(objMediaManager, RestoreSdp(_));
    EXPECT_CALL(objTimer, Start(MtcCallState::TIMER_RETRY_UPDATE, _));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, OnCodeUnspecifiedReturnsToEstablished)
{
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    EXPECT_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillOnce(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_403));
    SetActionConfigs(SipStatusCode::SC_403, {});

    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, Refresh_NotifyTimerExpiredSetsPendingUpdate)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    EXPECT_EQ(CallStateName::UPDATING,
            pUpdatingState->Refresh_NotifyTimerExpired(bDoImplicitRefresh));

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_TRUE(pUpdatingInfo->HasPendingUpdate());
}

TEST_F(UpdatingStateTest, OnGlareConditionTimerExpiredRetriesUpdate)
{
    EXPECT_CALL(objPendingOperationHolder.GetMock(), Hold(pUpdatingInfo->GetModifyingInfo()));
    pUpdatingInfo->SetRequestingType(UpdateType::HOLD);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Resume(pUpdatingInfo->GetModifyingInfo()));
    pUpdatingInfo->SetRequestingType(UpdateType::RESUME);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);

    EXPECT_CALL(objPendingOperationHolder.GetMock(),
            Update(pUpdatingInfo->GetTargetCallType(), pUpdatingInfo->GetModifyingInfo()));
    pUpdatingInfo->SetRequestingType(UpdateType::SESSION);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnSrvccStateUpdated(SrvccState::CANCELED));
    pUpdatingInfo->SetRequestingType(UpdateType::SRVCC_RECOVERED_CANCEL);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnSrvccStateUpdated(SrvccState::FAILED));
    pUpdatingInfo->SetRequestingType(UpdateType::SRVCC_RECOVERED_FAILURE);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);
}

TEST_F(UpdatingStateTest, OnGlareConditionTimerExpiredDoesNotRetryUpdateIfInvalidUpdateType)
{
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnTimerExpired(_)).Times(0);

    pUpdatingInfo->SetRequestingType(UpdateType::NORMAL);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);

    pUpdatingInfo->SetRequestingType(UpdateType::REFRESH);
    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE);
}

TEST_F(UpdatingStateTest, OnInvalidTimerExpiredDoesNothing)
{
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->OnTimerExpired(-1));
}

TEST_F(UpdatingStateTest, SessionUpdateReceivedReturnsEstablishedIfGlareTimerActive)
{
    ON_CALL(objTimer, IsActive(MtcCallState::TIMER_RETRY_UPDATE)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objPendingOperationHolder.GetMock(), SessionUpdateReceived(&objSession)).Times(3);

    // No Held / Resumed case.
    EXPECT_CALL(objUiNotifier, SendUpdateFailed(_));
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdateReceived(&objSession));

    // Held case
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND;
    EXPECT_CALL(objUiNotifier, SendHoldFailed(_));
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdateReceived(&objSession));

    // Resumed case
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    EXPECT_CALL(objUiNotifier, SendResumeFailed(_));
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdateReceivedRejectsIfGlareTimerInActive)
{
    ON_CALL(objTimer, IsActive(MtcCallState::TIMER_RETRY_UPDATE)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_SIP_REQUEST_PENDING)));
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedNotifiesHeld)
{
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND;

    EXPECT_CALL(objUiNotifier, SendHeld);
    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedNotifiesResumed)
{
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_CALL(objUiNotifier, SendResumed);
    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedNotifiesHeldBy)
{
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_CALL(objUiNotifier, SendHeldBy);
    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedNotifiesResumedBy)
{
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_RECEIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_CALL(objUiNotifier, SendResumedBy);
    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedSetsConferenceCallToMediaManagerIfConferenceCall)
{
    objCallInfo.bConference = IMS_TRUE;

    EXPECT_CALL(objMediaManager, SetConferenceCall());
    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedNotSetConferenceCallToMediaManagerIfNotConferenceCall)
{
    objCallInfo.bConference = IMS_FALSE;

    EXPECT_CALL(objMediaManager, SetConferenceCall()).Times(0);
    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest,
        SessionUpdatedInvokesSendIncomingUpdateIfModificationOccurredWithoutIntentionAsModifier)
{
    delete pUpdatingInfo;
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objContext);
    ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    pUpdatingInfo->SetModifier();
    pUpdatingInfo->SetTargetCallType(CallType::VOIP);
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::ACCEPT_UPDATE, _)).Times(1);
    EXPECT_CALL(objMtcSession, SendAck()).Times(1);
    EXPECT_CALL(objMediaManager, NegotiateSdp(_)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, OnSdpReceived(_)).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(1);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedDoesNotInvokeSendIncomingUpdateIfImplicitlyDowngradedBefore)
{
    // AcceptResume changes call type to VOIP from VT - implicitly downgraded
    delete pUpdatingInfo;
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objContext);
    pUpdatingInfo->SetTargetCallType(CallType::VT);
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_RECEIVE;
    pUpdatingInfo->GetOriginalInfo().eVideoDirection = DIRECTION_INACTIVE;
    pUpdatingInfo->GetAlertingInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetAlertingInfo().eVideoDirection = DIRECTION_RECEIVE;
    ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objMediaInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;
    ON_CALL(objSession, GetPreviousRequest(_)).WillByDefault(Return(&objMessage));
    SipMethod objInviteMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objInviteMethod));
    pUpdatingState->AcceptResume(CallType::VOIP, objMediaInfo);

    // Receives ACK
    SipMethod objAckMethod(SipMethod::ACK);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objAckMethod));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMediaManager, Run(_, _, IMS_FALSE));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(_));
    EXPECT_CALL(objUiNotifier, SendResumedBy());
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedReturnsEstablishedStateIfResumedAsModifier)
{
    delete pUpdatingInfo;
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objContext);
    ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    pUpdatingInfo->SetModifier();
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->SetTargetCallType(CallType::VOIP);
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::ACCEPT_UPDATE, _)).Times(1);
    EXPECT_CALL(objMtcSession, SendAck()).Times(1);
    EXPECT_CALL(objMediaManager, NegotiateSdp(_)).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesSendUpdatedAsModifierWithSdp)
{
    pUpdatingInfo->SetModifier();
    pUpdatingInfo->SetTargetCallType(CallType::VOIP);
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::ACCEPT_UPDATE, _)).Times(1);
    EXPECT_CALL(objMtcSession, SendAck()).Times(1);
    EXPECT_CALL(objMediaManager, NegotiateSdp(_)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, OnSdpReceived(_)).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdated()).Times(1);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesSendUpdatedAsModifierWithoutSdp)
{
    pUpdatingInfo->SetModifier();
    pUpdatingInfo->SetTargetCallType(CallType::RTT);
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::ACCEPT_UPDATE, _)).Times(1);
    EXPECT_CALL(objMtcSession, SendAck()).Times(1);
    EXPECT_CALL(objMediaManager, NegotiateSdp(_)).Times(0);
    EXPECT_CALL(objMtcPreconditionManager, OnSdpReceived(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdated()).Times(1);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesOnMessageReceivedIfModified)
{
    delete pUpdatingInfo;
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objContext);
    ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    pUpdatingInfo->SetModifier();

    // SetUp IsModified() true
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    EXPECT_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillRepeatedly(Return(&objMessage));

    EXPECT_CALL(objMtcPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesOnMessageReceivedIfModifiedWithStaleUpdate)
{
    delete pUpdatingInfo;
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objContext);
    ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    pUpdatingInfo->SetModifier();

    // SetUp IsModified() true
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    EXPECT_CALL(
            objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_STALE_UPDATE, _))
            .WillOnce(Return(&objMessage))
            .WillOnce(Return(&objMessage));

    EXPECT_CALL(objMtcPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedReturnsEstablishedIfTargetCallTypeIsUnknown)
{
    pUpdatingInfo->SetTargetCallType(CallType::UNKNOWN);

    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::ACK, _)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, OnMessageReceived(&objSession, &objMessage)).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdated).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdatedBy).Times(0);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesSendUpdatedIfAlerted)
{
    pUpdatingInfo->SetAlerted();
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMtcSession, SetCallType(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdatedBy).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdated()).Times(1);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesSendIncomingUpdateIfNotAlertedAndModified)
{
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_UPDATE))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    SipMethod objMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMtcSession, SendAck()).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdatedBy).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdated()).Times(0);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(1);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesSendUpdatedBy)
{
    pUpdatingInfo->SetTargetCallType(CallType::VOIP);
    EXPECT_CALL(objMtcSession, HandleRequest(RequestType::ACK, _)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, OnMessageReceived(&objSession, &objMessage)).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdated).Times(0);
    EXPECT_CALL(objUiNotifier, SendUpdatedBy).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedInvokesSendUpdatedAndSendHeldByInOrder)
{
    pUpdatingInfo->SetAlerted();
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_RECEIVE;

    {
        InSequence s;

        EXPECT_CALL(objUiNotifier, SendUpdated).Times(1);
        EXPECT_CALL(objUiNotifier, SendHeldBy).Times(1);
    }

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionUpdatedTerminatesCallIfNoSdpAnswerForOfferlessInvite)
{
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    pUpdatingInfo->SetTargetCallType(CallType::UNKNOWN);
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    const CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objReason));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pUpdatingState->SessionUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionCancelDeliveryFailedReturnsUpdatingState)
{
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionCancelDeliveryFailed(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdatedInvokesMtcSessionHandleResponse)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(
            objMtcSession, HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, Ref(objMessage)));
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdatedReturnsEstablishedStateIfNegoFailure)
{
    SetUpOnSdpReceivedFailed();

    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdateFailed)
{
    // Expect CancelUpdate to be called
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE));
    EXPECT_CALL(objMtcSession, CancelUpdate(CallReasonInfo(CODE_SESSION_MODIFICATION_FAILED)));

    // Expect RecoverModificationFailure to be called
    EXPECT_CALL(objMtcSession, GetPreviousCallType());
    EXPECT_CALL(objMtcSession, SetCallType(_));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    // Expect NotifyFailure to be called (default case)
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdateFailedNotifiesHoldFailed)
{
    // Set up conditions for IsHeld() to be true
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND;

    // Expect CancelUpdate to be called
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE));
    EXPECT_CALL(objMtcSession, CancelUpdate(CallReasonInfo(CODE_SESSION_MODIFICATION_FAILED)));

    // Expect RecoverModificationFailure to be called
    EXPECT_CALL(objMtcSession, GetPreviousCallType());
    EXPECT_CALL(objMtcSession, SetCallType(_));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    // Expect NotifyFailure to call SendHoldFailed
    EXPECT_CALL(objUiNotifier, SendHoldFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED)));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdateFailedNotifiesResumeFailed)
{
    // Set up conditions for IsResumed() to be true
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;

    // Expect CancelUpdate to be called
    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE));
    EXPECT_CALL(objMtcSession, CancelUpdate(CallReasonInfo(CODE_SESSION_MODIFICATION_FAILED)));

    // Expect RecoverModificationFailure to be called
    EXPECT_CALL(objMtcSession, GetPreviousCallType());
    EXPECT_CALL(objMtcSession, SetCallType(_));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    // Expect NotifyFailure to call SendResumeFailed
    EXPECT_CALL(objUiNotifier, SendResumeFailed(CallReasonInfo(CODE_SUPP_SVC_FAILED)));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdateReceivedNotifiesIncomingUpdateIfPreconditionMet)
{
    ON_CALL(objMtcPreconditionManager, IsAvailableToAlertUser(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate);

    EXPECT_EQ(
            CallStateName::UPDATING, pUpdatingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest,
        SessionEarlyMediaUpdateReceivedDoesNotNotifyIncomingUpdateIfPreconditionDidNotMeet)
{
    ON_CALL(objMtcPreconditionManager, IsAvailableToAlertUser(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate).Times(0);

    EXPECT_EQ(
            CallStateName::UPDATING, pUpdatingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdateReceivedRejectsIfNegoFailed)
{
    SetUpOnSdpReceivedFailed();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    EXPECT_EQ(CallStateName::ESTABLISHED,
            pUpdatingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest,
        SessionEarlyMediaUpdateReceivedRejectsIfNegoFailedUsingSessionRejectingIf488RejectFailed)
{
    SetUpOnSdpReceivedFailed();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(603));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_NEGOTIATING));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objMtcSession, Reject(_));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    EXPECT_EQ(CallStateName::ESTABLISHED,
            pUpdatingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionEarlyMediaUpdateReceivedRejectsIfResponseFailed)
{
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(603));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_NEGOTIATING));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objMtcSession, Reject(_));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    EXPECT_EQ(
            CallStateName::UPDATING, pUpdatingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackDeliveredReturnsUpdatingAndDoesNothingIfNoMessageFound)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(nullptr));
    EXPECT_CALL(objMtcSession, HandleResponse(_, _)).Times(0);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackDelivered(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackDeliveredInvokesSendEarlyUpdate)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PRACK_RESPONSE, Ref(objMessage)));

    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackDelivered(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackDeliveredReturnsEstablishedStateIfEarlyUpdateFailed)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PRACK_RESPONSE, Ref(objMessage)));

    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionPrackDelivered(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackDeliveredDoesNothingIfLocalResourceConfirmationNotRequired)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PRACK_RESPONSE, Ref(objMessage)));

    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackDelivered(&objSession));
}

TEST_F(UpdatingStateTest,
        SessionPrackDeliveredDoesNothingIfNotAvailableToSendLocalResourceConfirmation)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PRACK_RESPONSE, Ref(objMessage)));

    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackDelivered(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackDeliveredDoesNothingIfNotNegotiated)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PRACK_RESPONSE, Ref(objMessage)));

    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);
    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackDelivered(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackDeliveryFailedReturnsEstablishedState)
{
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackReceivedNotifiesIncomingUpdateIfPreconditionMet)
{
    ON_CALL(objMtcPreconditionManager, IsAvailableToAlertUser(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackReceivedDoesNotNotifyIncomingUpdateIfPreconditionDidNotMeet)
{
    ON_CALL(objMtcPreconditionManager, IsAvailableToAlertUser(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionPrackReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackReceivedRejectsIfNegoFailed)
{
    SetUpOnSdpReceivedFailed();
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionPrackReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackReceivedRejectsIfNoCodecMatched)
{
    const SdpNegotiationResult objNegoResult(MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);
    SetNegotiateSdpFailure(objMessage, objNegoResult);
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objMtcSession,
            Reject(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE, MEDIA_NEGO_ERROR_NO_CODEC_MATCHED)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionPrackReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackReceivedRejectsIfInvalidDescriptor)
{
    const SdpNegotiationResult objNegoResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    SetNegotiateSdpFailure(objMessage, objNegoResult);
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objMtcSession,
            Reject(CallReasonInfo(
                    CODE_REJECT_UNSUPPORTED_SDP_HEADERS, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionPrackReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionPrackReceivedRejectsIfResponseFailed)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT))
            .WillByDefault(Return(603));
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_NEGOTIATING));
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objMtcSession, Reject(_));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionPrackReceived(&objSession));
}

TEST_F(UpdatingStateTest, SessionRprDeliveryFailedReturnsEstablishedState)
{
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionRprDeliveryFailed(&objSession));
}

TEST_F(UpdatingStateTest,
        SessionRprReceivedInvokesSendLocalResourceConfirmationWithPrackIfResourceReserved)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PROVISIONAL_RESPONSE, Ref(objMessage)));
    EXPECT_CALL(objMtcSession, SendPrack(IMS_TRUE)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionRprReceived(&objSession, -1));
}

TEST_F(UpdatingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfNotSupported)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));

    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .Times(0);
    EXPECT_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .Times(0);

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PROVISIONAL_RESPONSE, Ref(objMessage)));
    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionRprReceived(&objSession, -1));
}

TEST_F(UpdatingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfNotRequired)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PROVISIONAL_RESPONSE, Ref(objMessage)));
    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionRprReceived(&objSession, -1));
}

TEST_F(UpdatingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfResourceNotReserved)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PROVISIONAL_RESPONSE, Ref(objMessage)));
    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->SessionRprReceived(&objSession, -1));
}

TEST_F(UpdatingStateTest, SessionRprReceivedNotifiesFailureIfSdpNegoFailed)
{
    SetUpOnSdpReceivedFailed();
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PROVISIONAL_RESPONSE, Ref(objMessage)));
    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).Times(0);
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionRprReceived(&objSession, -1));
}

TEST_F(UpdatingStateTest, SessionRprReceivedNotifiesFailureIfSendPrackFailed)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::PROVISIONAL_RESPONSE, Ref(objMessage)));
    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession));
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession));
    EXPECT_CALL(objUiNotifier,
            SendUpdateFailed(CallReasonInfo(CODE_USER_REJECTED_SESSION_MODIFICATION)));

    EXPECT_EQ(CallStateName::ESTABLISHED, pUpdatingState->SessionRprReceived(&objSession, -1));
}

TEST_F(UpdatingStateTest, QosReservedDoesNothingIfResponseNotSuccessful)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->QosReserved(&objSession, 0));
}

TEST_F(UpdatingStateTest, QosReservedInvokesDoesNothingIfLocalResourceConfirmationNotRequired)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    pUpdatingInfo->SetModifier();
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->QosReserved(&objSession, 0));
}

TEST_F(UpdatingStateTest,
        QosReservedInvokesDoesNothingIfNotAvailableToSendLocalResourceConfirmation)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    pUpdatingInfo->SetModifier();
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->QosReserved(&objSession, 0));
}

TEST_F(UpdatingStateTest, QosReservedInvokesSendEarlyUpdateIfPreconditionReady)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    pUpdatingInfo->SetModifier();
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->QosReserved(&objSession, 0));
}

TEST_F(UpdatingStateTest, QosReservedInvokesRecoverModificationIfSendEarlyUpdateFail)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    pUpdatingInfo->SetModifier();
    ON_CALL(objMtcPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objMediaManager, RestoreSdp(&objSession)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, OnCallModified(&objSession)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->QosReserved(&objSession, 0));
}

TEST_F(UpdatingStateTest, QosReservedInvokesSendIncomingUpdateIfPreconditionReady)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    ON_CALL(objMtcPreconditionManager, IsAvailableToAlertUser(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->QosReserved(&objSession, 0));
}

TEST_F(UpdatingStateTest, QosReserveFailedMaintainsCall)
{
    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendTerminated(_)).Times(0);

    EXPECT_EQ(CallStateName::UPDATING,
            pUpdatingState->QosReserveFailed(&objSession, QosLossPolicy::MAINTAIN));
}

TEST_F(UpdatingStateTest, QosReserveFailedReleasesCall)
{
    const CallReasonInfo objReason(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED);
    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objReason));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReason));

    EXPECT_EQ(CallStateName::TERMINATING,
            pUpdatingState->QosReserveFailed(&objSession, QosLossPolicy::RELEASE));
}

TEST_F(UpdatingStateTest, QosReserveFailedModifiesCall)
{
    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendTerminated(_)).Times(0);
    EXPECT_CALL(objPendingOperationHolder.GetMock(),
            QosReserveFailed(&objSession, QosLossPolicy::MODIFY));

    EXPECT_EQ(CallStateName::UPDATING,
            pUpdatingState->QosReserveFailed(&objSession, QosLossPolicy::MODIFY));
}

TEST_F(UpdatingStateTest, IsPreconditionRequiredReturnsTrueIfConditionsAreMet)
{
    ON_CALL(objMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(
                    Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));

    EXPECT_TRUE(UpdatingState::IsPreconditionRequired(*pConfigurationProxy, *pUpdatingInfo));
}

TEST_F(UpdatingStateTest, IsPreconditionRequiredReturnsFalseIfConfigIsOff)
{
    ON_CALL(objMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_CHECKING_QOS_WHILE_CALL_UPGRADING_INT))
            .WillByDefault(Return(ConfigVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));

    EXPECT_FALSE(UpdatingState::IsPreconditionRequired(*pConfigurationProxy, *pUpdatingInfo));
}

TEST_F(UpdatingStateTest, SessionUpdatedSetsUnconfirmedRemoteHold)
{
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_SEND;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_CALL(objContext, SetUnconfirmedRemoteHold(IMS_TRUE));

    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedResetsUnconfirmedRemoteHoldIfResumedBy)
{
    ON_CALL(objContext, IsOnUnconfirmedRemoteHold).WillByDefault(Return(IMS_TRUE));
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_INACTIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_SEND;

    EXPECT_CALL(objContext, SetUnconfirmedRemoteHold(IMS_FALSE));

    pUpdatingState->SessionUpdated(&objSession);
}

TEST_F(UpdatingStateTest, SessionUpdatedRecoversFromUnconfirmedRemoteHoldAfterResume)
{
    ON_CALL(objContext, IsOnUnconfirmedRemoteHold).WillByDefault(Return(IMS_TRUE));
    pUpdatingInfo->GetOriginalInfo().eAudioDirection = DIRECTION_INACTIVE;
    pUpdatingInfo->GetModifyingInfo().eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Resume(_));
    EXPECT_CALL(objContext, SetUnconfirmedRemoteHold(IMS_FALSE));

    pUpdatingState->SessionUpdated(&objSession);
}
