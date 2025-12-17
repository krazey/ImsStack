/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/block/IncomingCallBarringBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class IncomingCallBarringBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    CallInfo objCallInfo;
    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    IncomingCallBarringBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
    }

    virtual void TearDown() override { delete pBlockRule; }
};

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForAllVoiceCalls)
{
    ON_CALL(objService, IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE))
            .WillByDefault(Return(IMS_TRUE));

    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::VOIP);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForVoiceCallsWhenRoaming)
{
    ON_CALL(objService,
            IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsRoaming).WillByDefault(Return(IMS_TRUE));

    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::RTT);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForAnonymousVoiceCalls)
{
    ON_CALL(objService,
            IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VOICE))
            .WillByDefault(Return(IMS_TRUE));
    MockMtcConfigurationProxy objConfigProxy;
    MtcSupplementaryService objSuppService(objContext, objConfigProxy);
    objSuppService.Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(OipType::RESTRICTED));
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSuppService));
    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(objParticipantInfo));

    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::VOIP);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForAllVideoCalls)
{
    ON_CALL(objService, IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ALL_VIDEO))
            .WillByDefault(Return(IMS_TRUE));

    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::VT);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForVideoCallsWhenRoaming)
{
    ON_CALL(objService,
            IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ROAMING_VIDEO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsRoaming).WillByDefault(Return(IMS_TRUE));

    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::VIDEO_RTT);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForAnonymousVideoCalls)
{
    ON_CALL(objService,
            IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VIDEO))
            .WillByDefault(Return(IMS_TRUE));
    MockMtcConfigurationProxy objConfigProxy;
    MtcSupplementaryService objSuppService(objContext, objConfigProxy);
    objSuppService.Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(OipType::RESTRICTED));
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSuppService));
    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(objParticipantInfo));

    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::VT);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsUnblockedForUnknownCallType)
{
    pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::UNKNOWN);
    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}
