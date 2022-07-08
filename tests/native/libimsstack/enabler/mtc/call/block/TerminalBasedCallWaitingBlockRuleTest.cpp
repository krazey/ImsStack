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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "MockIMtcService.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/TerminalBasedCallWaitingBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class TerminalBasedCallWaitingBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    CallInfo objCallInfo;
    MockIMtcBlockRuleCheckListener objListener;
    IMSList<IMtcCall*> lstOtherCalls;
    TerminalBasedCallWaitingBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService)
                .WillByDefault(ReturnRef(objService));

        objCallInfo.ePeerType = PeerType::MT;   // MT usage only
        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));

        pBlockRule = new TerminalBasedCallWaitingBlockRule(objContext);
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

        ON_CALL(*pCall, GetState)
                .WillByDefault(Return(eState));

        return pCall;
    }
};

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsUnblockedIfNoOtherCalls)
{
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .Times(0);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsUnblockedIfNoOtherActiveCalls)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::TERMINATING));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .Times(0);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsUnblockedIfActiveCallExistsAndCwEnabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::OUTGOING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::INCOMING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ALERTING));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_TRUE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsBlockedIfIdleCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsBlockedIfOutgoingCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::OUTGOING));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsBlockedIfIncomingCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::INCOMING));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsBlockedIfAlertingCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ALERTING));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsBlockedIfEstablishedCallExistsAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}

TEST_F(TerminalBasedCallWaitingBlockRuleTest, CheckReturnsBlockedIfHasUpdatingCallAndCwDisabled)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(objService, IsTerminalBasedCallWaitingEnabled)
            .WillByDefault(Return(IMS_FALSE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED), objResult.objReason);
}
