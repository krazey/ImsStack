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
#include "MockIMtcService.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/VopsBlockRule.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class VopsBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcService objService;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockIMtcCallContext objContext;
    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    VopsBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));

        pBlockRule = new VopsBlockRule(objContext);
    }

    virtual void TearDown() override { delete pBlockRule; }
};

TEST_F(VopsBlockRuleTest, CheckReturnsUnblockedForWfc)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE)).Times(0);

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(VopsBlockRuleTest, CheckReturnsUnblockedIfVopsSupported)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_SUPPORTED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(VopsBlockRuleTest, CheckReturnsUnblockedIfVopsUnknown)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMtcImsEventReceiver::UNKNOWN_VALUE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(VopsBlockRuleTest, CheckReturnsBlockedIfVopsNotSupported)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_NOT_SUPPORTED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_CALL_TYPE_NOT_ALLOWED), objResult.objReason);
}
