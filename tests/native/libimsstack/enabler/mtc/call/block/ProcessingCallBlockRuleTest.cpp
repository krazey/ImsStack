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
#include "MtcDef.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/UpdatingInfo.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/ProcessingCallBlockRule.h"
#include "call/state/UpdatingState.h"
#include "emergency/MockIMtcEmergencyServiceManager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class ProcessingCallBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcCallContext objEmergencyCallContext;
    MockIMtcEmergencyServiceManager objEmergencyServiceManager;
    CallInfo objCallInfo;
    CallInfo objEmergencyCallInfo;
    UpdatingInfo* pUpdatingInfo;

    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    ImsList<IMtcCall*> lstOtherCalls;
    ProcessingCallBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetEmergencyServiceManager)
                .WillByDefault(ReturnRef(objEmergencyServiceManager));

        objEmergencyCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
        ON_CALL(objEmergencyCallContext, GetCallInfo)
                .WillByDefault(ReturnRef(objEmergencyCallInfo));

        pBlockRule = new ProcessingCallBlockRule(objContext);
        pUpdatingInfo = new UpdatingInfo(objContext);
        ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
    }

    virtual void TearDown() override
    {
        delete pBlockRule;

        for (IMS_UINT32 nIndex = 0; nIndex < lstOtherCalls.GetSize(); nIndex++)
        {
            delete lstOtherCalls.GetAt(nIndex);
        }
        lstOtherCalls.Clear();
        delete pUpdatingInfo;
    }

    MockIMtcCall* CreateMockIMtcCall(IMtcCall::State eState)
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetCallContext).WillByDefault(ReturnRef(objContext));
        ON_CALL(*pCall, GetState).WillByDefault(Return(eState));

        return pCall;
    }

    MockIMtcCall* CreateMockIMtcCallEmergency()
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetCallContext).WillByDefault(ReturnRef(objEmergencyCallContext));

        return pCall;
    }
};

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsUnblockedIfEmergencyRoutingCall)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsUnblockedIfNormalRoutingCall)
{
    objCallInfo.eEmergencyType = EmergencyType::NORMAL_ROUTING;
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedIfIdleCallExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::IDLE));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedIfIncomingCallExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::INCOMING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedIfAlertingCallExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ALERTING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedIfOutgoingCallExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::OUTGOING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsUnblockedForMoIfConvertingCallExists)
{
    objCallInfo.ePeerType = PeerType::MO;
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedForMtIfUpdatingCallExists)
{
    objCallInfo.ePeerType = PeerType::MT;
    objContext.GetUpdatingInfo().SetRequestingType(UpdateType::SESSION);
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsUnblockedForMtIfHoldUpdatingCallExists)
{
    objCallInfo.ePeerType = PeerType::MT;
    objContext.GetUpdatingInfo().SetRequestingType(UpdateType::HOLD);
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::UPDATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedForMoIfEmergencyServiceIsOpening)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objEmergencyServiceManager, GetState)
            .WillByDefault(Return(IEmergencyServiceController::State::OPENING));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedForMtIfEmergencyServiceIsOpening)
{
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objEmergencyServiceManager, GetState)
            .WillByDefault(Return(IEmergencyServiceController::State::OPENING));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_E911_CALL), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedForMoIfEmergencyCallExists)
{
    objCallInfo.ePeerType = PeerType::MO;

    lstOtherCalls.Append(CreateMockIMtcCallEmergency());
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsBlockedForMtIfEmergencyCallExists)
{
    objCallInfo.ePeerType = PeerType::MT;

    lstOtherCalls.Append(CreateMockIMtcCallEmergency());
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_REJECT_ONGOING_E911_CALL), objResult.objReason);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsUnblockedIfNoProcessingCallExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::ESTABLISHED));
    lstOtherCalls.Append(CreateMockIMtcCall(IMtcCall::State::TERMINATING));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(ProcessingCallBlockRuleTest, CheckReturnsUnblockedIfNoCallExists)
{
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}
