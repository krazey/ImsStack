/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "call/block/MockIMtcBlockRule.h"
#include "call/block/TimerBlockRule.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MockIPassiveTimerHolder.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using Result = IMtcBlockRule::Result;
using ::testing::Return;

class TimerBlockRuleTest : public ::testing::Test
{
public:
    inline explicit TimerBlockRuleTest() :
            objListener(),
            objPassiveTimerHolder(),
            pBlockRule(IMS_NULL)
    {
    }
    inline ~TimerBlockRuleTest() { delete pBlockRule; }

protected:
    MockIMtcBlockRuleCheckListener objListener;
    MockIPassiveTimerHolder objPassiveTimerHolder;
    TimerBlockRule* pBlockRule;
};

TEST_F(TimerBlockRuleTest, CheckReturnsUnblockedIfCallIsEmergency)
{
    pBlockRule = new TimerBlockRule(objPassiveTimerHolder, IMS_TRUE);
    EXPECT_CALL(
            objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .Times(0);
    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(TimerBlockRuleTest, CheckReturnsBlockedIfRetryAfterBlockingTimerIsActive)
{
    pBlockRule = new TimerBlockRule(objPassiveTimerHolder, IMS_FALSE);
    EXPECT_CALL(
            objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(objResult.objReason,
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(TimerBlockRuleTest, CheckReturnsUnblockedIfRetryAfterBlockingTimerIsNotActive)
{
    pBlockRule = new TimerBlockRule(objPassiveTimerHolder, IMS_FALSE);
    EXPECT_CALL(
            objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}
