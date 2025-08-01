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
    MockIMtcBlockRuleCheckListener objListener;
    IncomingCallBarringBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        pBlockRule = new IncomingCallBarringBlockRule(objContext, CallType::RTT);
    }

    virtual void TearDown() override { delete pBlockRule; }
};

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForAllCalls)
{
    ON_CALL(objService, IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ALL_VOICE))
            .WillByDefault(Return(IMS_TRUE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedWhenRoaming)
{
    ON_CALL(objService,
            IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsRoaming).WillByDefault(Return(IMS_TRUE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST_F(IncomingCallBarringBlockRuleTest, CheckReturnsBlockedForAnonymousCalls)
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

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}
