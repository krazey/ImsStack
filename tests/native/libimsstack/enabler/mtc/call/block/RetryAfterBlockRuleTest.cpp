/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include "ImsList.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/RetryAfterBlockRule.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MockIPassiveTimerHolder.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using Result = IMtcBlockRule::Result;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class RetryAfterBlockRuleTest : public ::testing::Test
{
public:
    inline explicit RetryAfterBlockRuleTest() {}

    MockIMtcCallContext objContext;
    MockIMtcCallManager objCallManager;
    CallInfo objCallInfo;
    MockIMtcBlockRuleCheckListener objListener;
    MockIPassiveTimerHolder objPassiveTimerHolder;
    RetryAfterBlockRule* pBlockRule;
    MockIMtcService objService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimerHolder));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

        pBlockRule = new RetryAfterBlockRule(objContext);
    }

    virtual void TearDown() override { delete pBlockRule; }
};

TEST_F(RetryAfterBlockRuleTest, CheckReturnsUnblockedIfCallIsEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_CALL(
            objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .Times(0);

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(RetryAfterBlockRuleTest, CheckReturnsUnblockedIfRetryAfterBlockingTimerIsNotActive)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(RetryAfterBlockRuleTest, CheckReturnsUnblockedIfActiveCallExists)
{
    ImsList<IMtcCall*> objCalls;
    MockIMtcCall objCall;
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .WillByDefault(Return(IMS_TRUE));
    objCalls.Append(&objCall);
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(RetryAfterBlockRuleTest, CheckReturnsPendingIfNotEpsCombinedAttach)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsEpsCombinedAttach).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objPassiveTimerHolder,
            AddListener(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _))
            .Times(1);

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::PENDING, objResult.eStatus);
}

TEST_F(RetryAfterBlockRuleTest, CheckReturnsBlockedIfCsfbRequired)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsCsfbAvailable).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objPassiveTimerHolder,
            AddListener(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _))
            .Times(0);

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(objResult.objReason,
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(RetryAfterBlockRuleTest, OnPassiveTimerExpiredInvokesOnBlockRuleChecked)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsCsfbAvailable).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objPassiveTimerHolder,
            AddListener(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _))
            .Times(1);

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::PENDING, objResult.eStatus);

    EXPECT_CALL(objListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED))).Times(1);
    pBlockRule->OnPassiveTimerExpired(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER);
}
