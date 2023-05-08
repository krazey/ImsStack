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

#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "MockIMtcService.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/UpdatingInfo.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/MockIMtcBlockChecker.h"
#include "call/state/EstablishedState.h"
#include "call/state/MtcCallState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "sipcore/ISipClientConnection.h"
#include "sipcore/SipMethod.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class EstablishedStateTest : public ::testing::Test
{
public:
    EstablishedState* pEstablishedState;
    MockIMtcCallContext objMockCallContext;
    MockIMtcService objService;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcSession objMockMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockISession objMockISession;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MockIMtcPreconditionManager objMockPreconditionManager;
    MockMtcTimerWrapper objTimerWrapper;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;
    MockIMtcBlockChecker* pBlockChecker;
    MtcConfigurationProxy* pConfigurationProxy;
    UpdatingInfo* pUpdatingInfo;
    MediaInfo objMediaInfo;
    CallInfo objCallInfo;
    MtcSupplementaryService* pSupplementaryService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));
        ON_CALL(objMockMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objMockCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMockMtcSession));
        ON_CALL(objMockMtcSession, GetISession).WillByDefault(ReturnRef(objMockISession));
        ON_CALL(objMockISession, GetPreviousRequest(_)).WillByDefault(Return(&objMessage));

        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objMockCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pUpdatingInfo = new UpdatingInfo(objMockCallContext);
        ON_CALL(objMockCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
        ON_CALL(objMockCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objMockPreconditionManager));
        ON_CALL(objMockMediaManager, FormSdp(&objMockISession, CallType::VOIP, IMS_TRUE))
                .WillByDefault(Return(IMS_SUCCESS));

        ON_CALL(objMockCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMockPreconditionManager, FormPreconditionSdp(_, _)).WillByDefault(Return());

        ON_CALL(objMockCallContext, GetTimer).WillByDefault(ReturnRef(objTimerWrapper));
        pSupplementaryService =
                new MtcSupplementaryService(objMockCallContext, *pConfigurationProxy);
        ON_CALL(objMockCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        // This will be deleted by EstablishedState.
        pBlockChecker = new MockIMtcBlockChecker();
        ON_CALL(objMockCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));

        pEstablishedState = new EstablishedState(objMockCallContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pUpdatingInfo;
        delete pEstablishedState;
        delete pSupplementaryService;
    }
};

TEST_F(EstablishedStateTest, OnEnterRunsPendingOperationSynchronously)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockCallContext, RunPendingOperationIfPossible());
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest, OnEnterRunsPendingOperationAsynchronouslyIfOnRefreshing)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMockCallContext, GetAsyncRunner(_));
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest, TerminateByUserActionWhenNoReceivingAudioPackets)
{
    EXPECT_CALL(objMockMediaManager, IsAudioInactive)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Terminate(_, _)).Times(2);

    CallReasonInfo objReason(CODE_USER_TERMINATED);
    pEstablishedState->Terminate(objReason);
    pEstablishedState->Terminate(objReason);
}

TEST_F(EstablishedStateTest, RefreshNotifyCompletedRunsPendingOperationAsynchronously)
{
    EXPECT_CALL(objMockCallContext, GetAsyncRunner(_));

    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    pEstablishedState->Refresh_NotifyCompleted(piFakeConnection);
}

TEST_F(EstablishedStateTest, OnReceivingMediaDataFailedWithVideoInvokesDowngrade)
{
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .Times(2)
            .WillOnce(Return(IMS_SUCCESS))
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(2);

    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));
    pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_VIDEO, MEDIA_PROTOCOL_RTP);

    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::RTT));
    pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_TEXT, MEDIA_PROTOCOL_RTP);
}

TEST_F(EstablishedStateTest, OnVideoLowestBitRateInvokesDowngradeIfCallTypeIsVt)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VOIP));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->OnVideoLowestBitRate();
}

TEST_F(EstablishedStateTest, OnVideoLowestBitRateInvokesDowngradeIfCallTypeIsVideoRtt)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VIDEO_RTT));

    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::RTT));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->OnVideoLowestBitRate();
}

TEST_F(EstablishedStateTest, OnMediaFailed)
{
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_MEDIA_INIT_FAILED)))
            .Times(1);

    pEstablishedState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(EstablishedStateTest, OnIpcanChangedNotHandledIfConfigurationIsOff)
{
    ON_CALL(*pMockConfigurationManager, IsEnableSendReinviteOnRatChange)
            .WillByDefault(Return(IMS_FALSE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(_)).Times(0);

    IMS_UINT32 eIpcan = 1;
    pEstablishedState->OnIpcanChanged(eIpcan);
}

TEST_F(EstablishedStateTest, OnIpcanChangedDoesNotPushPendingOperationIfNoSessionRefreshing)
{
    ON_CALL(*pMockConfigurationManager, IsEnableSendReinviteOnRatChange)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(_)).Times(0);
    EXPECT_CALL(objMockMtcSession, Update(_, _, _));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _));

    IMS_UINT32 eIpcan = 1;
    pEstablishedState->OnIpcanChanged(eIpcan);
}

TEST_F(EstablishedStateTest, OnIpcanChangedPushesPendingOperation)
{
    ON_CALL(*pMockConfigurationManager, IsEnableSendReinviteOnRatChange)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_TRUE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    const IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(eIpcan));

    pEstablishedState->OnIpcanChanged(eIpcan);
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByCanceled)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_CANCEL, IMS_FALSE, SipMethod::INVITE))
            .Times(1);

    EXPECT_EQ(
            CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByFailed)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_FAILURE, IMS_FALSE, SipMethod::INVITE))
            .Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(EstablishedStateTest, SendOfferWithFullCapaOnResponseToReInvite)
{
    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objMockCallContext, IsHeldByMe).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMockMtcSession, AcceptUpdate()).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendIncomingUpdateIsInvokedIfUpdateNeedsToAlert)
{
    ON_CALL(*pMockConfigurationManager, GetPolicyForCheckingQosWhileCallUpgrading)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));

    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendProvisionalResponseIsInvokedIfPreconditionIsSupported)
{
    ON_CALL(*pMockConfigurationManager, GetPolicyForCheckingQosWhileCallUpgrading)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));

    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_CALL(objMockMtcSession, SendProvisionalResponse(IMS_FALSE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedRejectsIfBlocked)
{
    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    CallReasonInfo objAnyReason(CODE_UNSPECIFIED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objAnyReason)));

    CallType eAnyCallType = CallType::VIDEO_RTT;
    EXPECT_CALL(objMockMtcSession, GetPreviousCallType()).WillOnce(Return(eAnyCallType));
    EXPECT_CALL(objMockMtcSession, SetCallType(eAnyCallType));
    EXPECT_CALL(objMockMtcSession, Reject(objAnyReason));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedInvokesSendIncomingResume)
{
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VOIP));
    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    ON_CALL(*pMockConfigurationManager, IsCheckUiConditionForIncomingResume)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objUiNotifier, SendIncomingResume).Times(1);
    EXPECT_CALL(objTimerWrapper, Start(_, _)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, OnReceivingNetworkToneStartedAndFailedInvokesSendHeldBy)
{
    EXPECT_CALL(objUiNotifier, SendHeldBy).Times(2);
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnReceivingNetworkToneStarted());
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnReceivingNetworkToneFailed());
}

TEST_F(EstablishedStateTest, UpdateDoesNotRefreshConvertRemoteResponseTimer)
{
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .Times(3)
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
}

}  // namespace android
