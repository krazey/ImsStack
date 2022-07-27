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

#include <gtest/gtest.h>
#include "call/state/UpdatingState.h"
#include "call/MockIMtcCallContext.h"
#include "call/UpdatingInfo.h"
#include "call/MockIMtcSession.h"
#include "call/state/MtcCallState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/ISession.h"
#include "core/MockISession.h"
#include "helper/MtcTimerWrapper.h"
#include "sipcore/SipMethod.h"

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

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));

        pUpdatingInfo = new UpdatingInfo();
        ON_CALL(objContext, GetUpdatingInfo)
                .WillByDefault(ReturnRef(*pUpdatingInfo));

        pUpdatingState = new UpdatingState(objContext);
    }

    virtual void TearDown() override
    {
        delete pUpdatingState;
        delete pUpdatingInfo;
    }
};

TEST_F(UpdatingStateTest, OnExitDoesntSendUpdateIfUpdatingInfoHasPendingUpdateAsDefaultValue)
{
    MockIMtcSession objMtcSession;
    EXPECT_CALL(objMtcSession, Update(_, _, _)).Times(0);

    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitDoesntSendUpdateIfUpdatingInfoDoesntHavePendingUpdate)
{
    MockIMtcSession objMtcSession;
    EXPECT_CALL(objMtcSession, Update(_, _, _)).Times(0);

    pUpdatingInfo->SetPendingUpdate(IMS_FALSE);
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitSendsUpdateIfUpdatingInfoHasPendingUpdate)
{
    MockIMtcSession objMtcSession;
    ON_CALL(objContext, GetSession())
            .WillByDefault(Return(&objMtcSession));

    EXPECT_CALL(objMtcSession, Update(UpdateType::REFRESH, IMS_FALSE, SipMethod::INVALID)).Times(1);

    pUpdatingInfo->SetPendingUpdate(IMS_TRUE);
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnUserResponseTimerExpiredCallsReject)
{
    MtcTimerWrapper objTimer;
    ON_CALL(objContext, GetTimer).WillByDefault(ReturnRef(objTimer));
    MockISession objSession;
    ON_CALL(objSession, GetState())
            .WillByDefault(Return(ISession::STATE_RENEGOTIATING));

    MockIMtcSession objMtcSession;
    ON_CALL(objMtcSession, GetISession())
            .WillByDefault(ReturnRef(objSession));
    ON_CALL(objContext, GetSession())
            .WillByDefault(Return(&objMtcSession));

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE))).Times(1);

    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_CONVERT_USER_RESPONSE);
}

TEST_F(UpdatingStateTest, OnRemoteResponseTimerExpiredCallsCancelUpdate)
{
    MtcTimerWrapper objTimer;
    ON_CALL(objContext, GetTimer).WillByDefault(ReturnRef(objTimer));

    MockIMtcSession objMtcSession;
    ON_CALL(objContext, GetSession())
            .WillByDefault(Return(&objMtcSession));

    EXPECT_CALL(objMtcSession, CancelUpdate(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE)))
            .Times(1);

    pUpdatingState->OnTimerExpired(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE);
}
