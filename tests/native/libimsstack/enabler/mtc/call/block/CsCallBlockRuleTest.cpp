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

#include "IMtcImsEventReceiver.h"
#include "ImsEventDef.h"
#include "MockIMtcImsEventReceiver.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/CsCallBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class CsCallBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockIMtcCallContext objContext;
    MockIMtcBlockRuleCheckListener objListener;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    }
};

TEST_F(CsCallBlockRuleTest, CheckReturnsUnblockedForEmergencyCall)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    CsCallBlockRule objBlockRule(objContext);

    EXPECT_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_CSCALL_STATE)).Times(0);

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CsCallBlockRuleTest, CheckReturnsUnblockedIfCsCallStateIdle)
{
    CsCallBlockRule objBlockRule(objContext);
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_CSCALL_STATE))
            .WillByDefault(Return(IMS_CSCALL_STATE_IDLE));

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CsCallBlockRuleTest, CheckReturnsUnblockedIfCsCallStateUnknown)
{
    CsCallBlockRule objBlockRule(objContext);
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_CSCALL_STATE))
            .WillByDefault(Return(IMtcImsEventReceiver::UNKNOWN_VALUE));

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CsCallBlockRuleTest, CheckReturnsBlockedIfCsCallIncoming)
{
    CsCallBlockRule objBlockRule(objContext);
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_CSCALL_STATE))
            .WillByDefault(Return(IMS_CSCALL_STATE_INCOMING));

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CS_CALL), objResult.objReason);
}

TEST_F(CsCallBlockRuleTest, CheckReturnsBlockedIfCsCallActive)
{
    CsCallBlockRule objBlockRule(objContext);
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_CSCALL_STATE))
            .WillByDefault(Return(IMS_CSCALL_STATE_ACTIVE));

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CS_CALL), objResult.objReason);
}

TEST_F(CsCallBlockRuleTest, CheckReturnsBlockedIfCsCallActiveE911)
{
    CsCallBlockRule objBlockRule(objContext);
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_CSCALL_STATE))
            .WillByDefault(Return(IMS_CSCALL_STATE_ACTIVE_E911));

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CS_CALL), objResult.objReason);
}
