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

#include "IMtcService.h"
#include "ImsList.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/CallWaitingBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class CallWaitingBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    CallInfo objCallInfo;
    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    ImsList<IMtcCall*> lstOtherCalls;
    CallWaitingBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

        objCallInfo.ePeerType = PeerType::MT;  // MT usage only
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        pBlockRule = new CallWaitingBlockRule(objContext);
    }

    virtual void TearDown() override
    {
        delete pBlockRule;

        for (IMS_UINT32 nIndex = 0; nIndex < lstOtherCalls.GetSize(); nIndex++)
        {
            delete lstOtherCalls.GetAt(nIndex);
        }
        lstOtherCalls.Clear();
    }

    MockIMtcCall* CreateMockIMtcCall(IMtcCall::State eState)
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetState).WillByDefault(Return(eState));

        return pCall;
    }
};

TEST_F(CallWaitingBlockRuleTest, CheckReturnsUnblockedIfNoOtherCalls)
{
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_CALL(objService, GetTbcwStatus).Times(0);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsUnblockedIfNoOtherActiveCalls)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::TERMINATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_CALL(objService, GetTbcwStatus).Times(0);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsUnblockedIfActiveCallExistsAndCwUnprovisioned)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::OUTGOING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::INCOMING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ALERTING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::UNPROVISIONED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsUnblockedIfActiveCallExistsAndCwEnabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::OUTGOING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::INCOMING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ALERTING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_ENABLED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsBlockedIfIdleCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_DISABLED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsBlockedIfOutgoingCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::OUTGOING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_DISABLED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsBlockedIfIncomingCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::INCOMING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_DISABLED));
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsBlockedIfAlertingCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ALERTING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_DISABLED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsBlockedIfEstablishedCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_DISABLED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(CallWaitingBlockRuleTest, CheckReturnsBlockedIfHasUpdatingCallAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, GetTbcwStatus).WillByDefault(Return(SuppStatus::PROVISIONED_DISABLED));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}
