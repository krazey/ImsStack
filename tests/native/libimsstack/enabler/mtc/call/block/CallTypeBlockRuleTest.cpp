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
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class CallTypeBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcBlockRuleCheckListener objListener;
    CallInfo objCallInfo;
    IMSList<IMtcCall*> lstOtherCalls;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);

        ON_CALL(objContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;

        for (IMS_UINT32 nIndex = 0; nIndex < lstOtherCalls.GetSize(); nIndex++)
        {
            delete lstOtherCalls.GetAt(nIndex);
        }
        lstOtherCalls.Clear();
    }

    MockIMtcCall* CreateMockIMtcCall(CallType eCallType)
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetCallType)
                .WillByDefault(Return(eCallType));

        return pCall;
    }
};

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVideoRttIfNotAllowed)
{
    ON_CALL(*pConfigurationManager, IsAllowTextWithVideo)
            .WillByDefault(Return(IMS_FALSE));

    CallTypeBlockRule objBlockRule(objContext, CallType::VIDEO_RTT);
    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE), objResult.objReason);
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsUnblockedIfAllowed)
{
    ON_CALL(*pConfigurationManager, IsAllowTextWithVideo)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsAllowMultipleCallIncludingVideoCall)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objContext, GetOtherCalls)
            .Times(0);

    CallTypeBlockRule objBlockRuleForVoip(objContext, CallType::VOIP);
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVoip.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForVt(objContext, CallType::VT);
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVt.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForRtt(objContext, CallType::RTT);
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForRtt.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForVideoRtt(objContext, CallType::VIDEO_RTT);
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVideoRtt.Check(objListener).eStatus);
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVideoCallIfVoipExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VOIP));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationManager, IsAllowTextWithVideo)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsAllowMultipleCallIncludingVideoCall)
            .WillByDefault(Return(IMS_FALSE));

    {
        objCallInfo.ePeerType = PeerType::MO;

        CallTypeBlockRule objBlockRuleForVt(objContext, CallType::VT);
        Result objResultForVt = objBlockRuleForVt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVt.objReason);

        CallTypeBlockRule objBlockRuleForVideoRtt(objContext, CallType::VIDEO_RTT);
        Result objResultForVideoRtt = objBlockRuleForVideoRtt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVideoRtt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVideoRtt.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        CallTypeBlockRule objBlockRuleForVt(objContext, CallType::VT);
        Result objResultForVt = objBlockRuleForVt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVt.objReason);

        CallTypeBlockRule objBlockRuleForVideoRtt(objContext, CallType::VIDEO_RTT);
        Result objResultForVideoRtt = objBlockRuleForVideoRtt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVideoRtt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED),
                objResultForVideoRtt.objReason);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVoipIfVtExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VT));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationManager, IsAllowTextWithVideo)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsAllowMultipleCallIncludingVideoCall)
            .WillByDefault(Return(IMS_FALSE));

    {
        objCallInfo.ePeerType = PeerType::MO;

        CallTypeBlockRule objBlockRule(objContext, CallType::VOIP);
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVt.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        CallTypeBlockRule objBlockRule(objContext, CallType::VOIP);
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVt.objReason);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVoipIfVideoRttExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VIDEO_RTT));
    ON_CALL(objContext, GetOtherCalls)
            .WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationManager, IsAllowTextWithVideo)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, IsAllowMultipleCallIncludingVideoCall)
            .WillByDefault(Return(IMS_FALSE));

    {
        objCallInfo.ePeerType = PeerType::MO;

        CallTypeBlockRule objBlockRule(objContext, CallType::VOIP);
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVt.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        CallTypeBlockRule objBlockRule(objContext, CallType::VOIP);
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVt.objReason);
    }
}
