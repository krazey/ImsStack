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

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestSystemTimeService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/SsacBlockRule.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MockISsacTimerHandler.h"
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class SsacBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    CallInfo objCallInfo;
    TestImsRadioService objImsRadioService;
    TestSystemTimeService objSystemTimeService;
    MockIMtcService objService;
    MockIMtcBlockRuleCheckListener objBlockRuleCheckListener;
    SsacInfo objSsacInfo;
    MockISsacTimerHandler objSsacTimerHandler;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_SYSTEM_TIME, &objSystemTimeService);

        objSsacInfo.nBarringFactorForVoice = 100;
        objSsacInfo.nBarringFactorForVideo = 100;
        ON_CALL(objImsRadioService.GetMockImsRadio(), GetSsacInfo())
                .WillByDefault(ReturnRef(objSsacInfo));

        objCallInfo.ePeerType = PeerType::MO;
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

        ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objService, GetSsacTimerHandler).WillByDefault(ReturnRef(objSsacTimerHandler));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_SYSTEM_TIME, IMS_NULL);
    }

    void SetSsacBarred(IN IMS_BOOL bVoiceBarred, IN IMS_BOOL bVideoBarred)
    {
        ON_CALL(objSsacTimerHandler, IsSsacTimerRunning(CallType::VOIP))
                .WillByDefault(Return(bVoiceBarred));
        ON_CALL(objSsacTimerHandler, IsSsacTimerRunning(CallType::RTT))
                .WillByDefault(Return(bVoiceBarred));
        ON_CALL(objSsacTimerHandler, IsSsacTimerRunning(CallType::VT))
                .WillByDefault(Return(bVideoBarred));
        ON_CALL(objSsacTimerHandler, IsSsacTimerRunning(CallType::VIDEO_RTT))
                .WillByDefault(Return(bVideoBarred));
    }
};

TEST_F(SsacBlockRuleTest, CheckNotChecksSsacWhenNr)
{
    SetSsacBarred(IMS_TRUE, IMS_TRUE);
    objCallInfo.ePeerType = PeerType::MT;

    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::UNBLOCKED), objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckNotChecksSsacWhenMt)
{
    SetSsacBarred(IMS_TRUE, IMS_TRUE);
    objCallInfo.ePeerType = PeerType::MT;

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::UNBLOCKED), objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckNotChecksSsacWhenEmergency)
{
    SetSsacBarred(IMS_TRUE, IMS_TRUE);
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::UNBLOCKED), objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckNotChecksSsacWhenWifi)
{
    SetSsacBarred(IMS_TRUE, IMS_TRUE);
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::UNBLOCKED), objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsBlockedForVoipCallWhenVoiceSsacTimerIsRunning)
{
    SetSsacBarred(IMS_TRUE, IMS_FALSE);

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)),
            objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsUnBlockedForVoipCallWhenVideoSsacTimerIsRunning)
{
    SetSsacBarred(IMS_FALSE, IMS_TRUE);

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::UNBLOCKED), objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsBlockedForVideoCallWhenVideoSsacTimerIsRunning)
{
    SetSsacBarred(IMS_FALSE, IMS_TRUE);

    SsacBlockRule objRule(objContext, CallType::VT);
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)),
            objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsBlockedForVideoRttCallWhenVideoSsacTimerIsRunning)
{
    SetSsacBarred(IMS_FALSE, IMS_TRUE);

    SsacBlockRule objRule(objContext, CallType::VIDEO_RTT);
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)),
            objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsBlockedWithStartingTimer)
{
    SetSsacBarred(IMS_FALSE, IMS_FALSE);

    objSsacInfo.nBarringFactorForVoice = 90;

    EXPECT_CALL(objSystemTimeService.GetMockSystemTime(), GetRandom(_))
            .Times(1)
            .WillOnce(Return(99));

    EXPECT_CALL(objSsacTimerHandler, StartBarringTimer(CallType::VOIP));

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)),
            objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsUnblockedWhenRandomValueIsSmallerThanFactor)
{
    SetSsacBarred(IMS_FALSE, IMS_FALSE);

    objSsacInfo.nBarringFactorForVoice = 90;

    EXPECT_CALL(objSystemTimeService.GetMockSystemTime(), GetRandom(_))
            .Times(1)
            .WillOnce(Return(50));

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::UNBLOCKED), objRule.Check(objBlockRuleCheckListener));
}

TEST_F(SsacBlockRuleTest, CheckReturnsBlockedWhenFactorIsZero)
{
    SetSsacBarred(IMS_FALSE, IMS_FALSE);

    objSsacInfo.nBarringFactorForVoice = 0;

    EXPECT_CALL(objSystemTimeService.GetMockSystemTime(), GetRandom(_))
            .Times(1)
            .WillOnce(Return(0));

    SsacBlockRule objRule(objContext, CallType::VOIP);
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)),
            objRule.Check(objBlockRuleCheckListener));
}
