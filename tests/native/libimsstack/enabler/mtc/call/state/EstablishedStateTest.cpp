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
#include "ImsTypeDef.h"
#include "MtcContextRepository.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/UpdatingInfo.h"
#include "call/state/EstablishedState.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
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
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcSession objMockMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockISession objMockISession;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMockMtcSession));
        ON_CALL(objMockMtcSession, GetISession).WillByDefault(ReturnRef(objMockISession));

        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        pEstablishedState = new EstablishedState(objMockCallContext);
    }

    virtual void TearDown() override { delete pEstablishedState; }
};

TEST_F(EstablishedStateTest, TerminateByUserActionWhenNoReceivingAudioPackets)
{
    CallInfo objCallInfo;
    ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_CALL(objMockMediaManager, IsAudioInactive)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Terminate(_, _)).Times(2);

    CallReasonInfo objReason(CODE_USER_TERMINATED);
    pEstablishedState->Terminate(objReason);
    pEstablishedState->Terminate(objReason);
}

TEST_F(EstablishedStateTest, OnMediaFailed)
{
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_MEDIA_INIT_FAILED)))
            .Times(1);

    pEstablishedState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByCanceled)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_CANCEL, IMS_FALSE, SipMethod::UPDATE))
            .Times(1);

    EXPECT_EQ(
            CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByFailed)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_FAILURE, IMS_FALSE, SipMethod::UPDATE))
            .Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(EstablishedStateTest, SendOfferWithFullCapaOnResponseToReInvite)
{
    MockIMessage* piMessage = new MockIMessage();
    ON_CALL(objMockISession, GetPreviousRequest(_)).WillByDefault(Return(piMessage));

    UpdatingInfo* pUpdatingInfo = new UpdatingInfo(objMockCallContext);
    ON_CALL(objMockCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objMockCallContext);

    MockIMessageUtils objMessageUtils;
    ON_CALL(objMockCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));

    ON_CALL(objMessageUtils, HasSdp(piMessage)).WillByDefault(Return(IMS_FALSE));

    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(*piMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));

    ON_CALL(objMockCallContext, IsHeldByMe).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMockMediaManager, FormSdp(&objMockISession, CallType::VOIP, IMS_TRUE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    MockIMtcPreconditionManager objMockPreconditionManager;
    ON_CALL(objMockCallContext, GetPreconditionManager)
            .WillByDefault(ReturnRef(objMockPreconditionManager));
    EXPECT_CALL(objMockPreconditionManager, FormPreconditionSdp(_, IMS_FALSE)).Times(1);

    EXPECT_CALL(objMockMtcSession, AcceptUpdate()).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));

    delete piMessage;
    delete pUpdatingInfo;
}

}  // namespace android
