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

#include "ImsList.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/block/CallCountBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class CallCountBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcCallManager objCallManager;
    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    CallInfo objCallInfo;
    MockMtcConfigurationProxy* pConfigurationProxy;
    CallCountBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        pBlockRule = new CallCountBlockRule(objContext);
    }

    virtual void TearDown() override
    {
        delete pBlockRule;
        delete pConfigurationProxy;
    }

    MockIMtcCall* CreateMockIMtcCall(IMtcCall::State eState)
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetState).WillByDefault(Return(eState));

        return pCall;
    }
};

TEST_F(CallCountBlockRuleTest, CheckReturnsUnblockedForMoConference)
{
    objCallInfo.bConference = IMS_TRUE;
    objCallInfo.ePeerType = PeerType::MO;

    EXPECT_CALL(objCallManager, GetCalls).Times(0);

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallCountBlockRuleTest, CheckReturnsUnblockedIfNoCallExists)
{
    objCallInfo.bConference = IMS_FALSE;

    ImsList<IMtcCall*> lstCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(lstCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CALL_MAX_COUNT_INT))
            .WillByDefault(Return(1));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallCountBlockRuleTest, CheckReturnsUnblockedIfMaxTerminatingCallExists)
{
    objCallInfo.bConference = IMS_FALSE;

    ImsList<IMtcCall*> lstCalls;
    lstCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));         // Call to check
    lstCalls.Append(CreateMockIMtcCall(IMtcCall::State::TERMINATING));  // Ongoing call
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(lstCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CALL_MAX_COUNT_INT))
            .WillByDefault(Return(1));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallCountBlockRuleTest, CheckReturnsBlockedIfMaxCallExists)
{
    objCallInfo.bConference = IMS_FALSE;

    ImsList<IMtcCall*> lstCalls;
    lstCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));         // Call to check
    lstCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));  // Ongoing call
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(lstCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CALL_MAX_COUNT_INT))
            .WillByDefault(Return(1));

    {
        objCallInfo.ePeerType = PeerType::MO;

        Result objResult = pBlockRule->Check(objListener);

        EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResult.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        Result objResult = pBlockRule->Check(objListener);

        EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResult.objReason);
    }
}
