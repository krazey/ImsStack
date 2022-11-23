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

#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/MockMtcPendingOperationHolder.h"
#include "call/UpdatingInfo.h"
#include "call/state/MtcCallState.h"
#include "call/state/UpdatingState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/ISession.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "sipcore/SipMethod.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class UpdatingStateTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    UpdatingInfo* pUpdatingInfo;
    UpdatingState* pUpdatingState;
    CallInfo objCallInfo;
    MockIMtcMediaManager objMediaManager;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMtcUiNotifier objUiNotifier;
    MtcSupplementaryService* pMtcSupplementaryService;
    MockIMtcPreconditionManager objMtcPreconditionManager;
    MockIMessage* pMessage;
    MockMtcTimerWrapper objTimer;
    MockMtcPendingOperationHolder objPendingOperationHolder;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        pMtcSupplementaryService = new MtcSupplementaryService(*pConfigurationProxy);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        ON_CALL(objContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));

        pUpdatingInfo = new UpdatingInfo(objContext);
        ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetSupplementaryService())
                .WillByDefault(ReturnRef(*pMtcSupplementaryService));
        ON_CALL(objContext, GetPreconditionManager())
                .WillByDefault(ReturnRef(objMtcPreconditionManager));
        ON_CALL(objContext, GetTimer).WillByDefault(ReturnRef(objTimer));

        ON_CALL(objContext, GetPendingOperationHolder)
                .WillByDefault(ReturnRef(objPendingOperationHolder));

        pMessage = new MockIMessage();
        pUpdatingState = new UpdatingState(objContext);
    }

    virtual void TearDown() override
    {
        delete pUpdatingState;
        delete pMessage;
        delete pUpdatingInfo;
        delete pConfigurationProxy;
        delete pMtcSupplementaryService;
    }
};

TEST_F(UpdatingStateTest, OnExitDoesntSendUpdateIfUpdatingInfoHasPendingUpdateAsDefaultValue)
{
    EXPECT_CALL(objMtcSession, Update(_, _, _)).Times(0);

    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitDoesntSendUpdateIfUpdatingInfoDoesntHavePendingUpdate)
{
    EXPECT_CALL(objMtcSession, Update(_, _, _)).Times(0);

    pUpdatingInfo->SetPendingUpdate(IMS_FALSE);
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitSendsUpdateIfUpdatingInfoHasPendingUpdate)
{
    EXPECT_CALL(objMtcSession, Update(UpdateType::REFRESH, IMS_FALSE, SipMethod::INVALID)).Times(1);

    pUpdatingInfo->SetPendingUpdate(IMS_TRUE);
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, HoldPushesPendingOperation)
{
    MediaInfo objMediaInfo;
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

    pUpdatingState->Hold(objMediaInfo);
}

TEST_F(UpdatingStateTest, ResumePushesPendingOperation)
{
    MediaInfo objMediaInfo;
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

    pUpdatingState->Resume(objMediaInfo);
}

TEST_F(UpdatingStateTest, UpdatePushesPendingOperation)
{
    MediaInfo objMediaInfo;
    CallType eAnyType = CallType::VT;
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

    pUpdatingState->Update(eAnyType, objMediaInfo);
}

TEST_F(UpdatingStateTest, AcceptUpdateReturnsEstablishedWhenISessionStateEstablished)
{
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_ESTABLISHED));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    MediaInfo objMediaInfo;
    EXPECT_CALL(objMediaManager, GetMediaInfo()).Times(1).WillOnce(ReturnRef(objMediaInfo));
    EXPECT_CALL(objUiNotifier, SendUpdated(_, _, _)).Times(1);

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->AcceptUpdate(CallType::VOIP, objMediaInfo));
}

TEST_F(UpdatingStateTest, AcceptUpdateReturnsEstablishedWhenPreviousRequestIsUpdate)
{
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    pUpdatingInfo->GetAlertingInfo().eAudioDirection = DIRECTION_SEND;

    EXPECT_CALL(objTimer, Stop(MtcCallState::TIMER_CONVERT_USER_RESPONSE)).Times(1);
    EXPECT_CALL(objMediaManager, SetMediaInfo(_)).Times(1);
    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(1);
    EXPECT_CALL(objMtcPreconditionManager, FormPreconditionSdp(_, _)).Times(1);
    EXPECT_CALL(objMediaManager, GetMediaInfo()).Times(1).WillOnce(ReturnRef(objMediaInfo));
    EXPECT_CALL(objMtcSession, AcceptUpdate()).Times(1);
    EXPECT_CALL(objSession, GetPreviousRequest(_)).Times(1).WillOnce(Return(pMessage));
    SipMethod objSipMethod(SipMethod::UPDATE);
    EXPECT_CALL(*pMessage, GetMethod()).Times(1).WillOnce(ReturnRef(objSipMethod));
    EXPECT_CALL(objUiNotifier, SendUpdated(_, _, _)).Times(1);

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pUpdatingState->AcceptUpdate(CallType::VOIP, objMediaInfo));
    EXPECT_EQ(DIRECTION_SEND, objMediaInfo.eAudioDirection);
}

TEST_F(UpdatingStateTest, AcceptUpdateReturnsUpdating)
{
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));
    EXPECT_CALL(objSession, GetPreviousRequest(_)).Times(1).WillOnce(Return(pMessage));
    SipMethod objSipMethod(SipMethod::INVITE);
    EXPECT_CALL(*pMessage, GetMethod()).Times(1).WillOnce(ReturnRef(objSipMethod));
    MediaInfo objMediaInfo;
    EXPECT_CALL(objMediaManager, GetMediaInfo()).Times(1).WillOnce(ReturnRef(objMediaInfo));

    EXPECT_EQ(CallStateName::UPDATING, pUpdatingState->AcceptUpdate(CallType::VOIP, objMediaInfo));
}

TEST_F(UpdatingStateTest, OnUserResponseTimerExpiredCallsReject)
{
    MockISession objSession;
    ON_CALL(objSession, GetState()).WillByDefault(Return(ISession::STATE_RENEGOTIATING));

    ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));

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

TEST_F(UpdatingStateTest, OnReceivingMediaDataFailedWithVideoPushesPendingOperation)
{
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::VT));
    pUpdatingState->OnReceivingMediaDataFailed(MEDIATYPE_VIDEO, MEDIA_PROTOCOL_RTP);
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
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

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
