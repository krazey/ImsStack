/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ImsAosParameter.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/ServiceBlockRule.h"
#include "helper/MockIMtcAosConnector.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class ServiceBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcService objService;
    MockIMtcCallContext objContext;
    MockIMtcAosConnector objAosConnector;
    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    }
};

TEST_F(ServiceBlockRuleTest, CheckReturnsUnblockedIfMmtelFeatureAvailable)
{
    ON_CALL(objAosConnector, IsFeatureConnected(ImsAosFeature::MMTEL))
            .WillByDefault(Return(IMS_TRUE));

    Result objResult = ServiceBlockRule(objContext, CallType::VOIP).Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(ServiceBlockRuleTest, CheckReturnsBlockedIfMmtelFeatureUnavailableForCsfbAvailableMoCall)
{
    ON_CALL(objAosConnector, IsFeatureConnected(ImsAosFeature::MMTEL))
            .WillByDefault(Return(IMS_FALSE));
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objContext, IsCsfbAvailable).WillByDefault(Return(IMS_TRUE));

    Result objResult = ServiceBlockRule(objContext, CallType::VOIP).Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL),
            objResult.objReason);
}

TEST_F(ServiceBlockRuleTest, CheckReturnsBlockedIfMmtelFeatureUnavailableForCsfbUnavailableMoCall)
{
    ON_CALL(objAosConnector, IsFeatureConnected(ImsAosFeature::MMTEL))
            .WillByDefault(Return(IMS_FALSE));
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objContext, IsCsfbAvailable).WillByDefault(Return(IMS_FALSE));

    Result objResult = ServiceBlockRule(objContext, CallType::VOIP).Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE), objResult.objReason);
}

TEST_F(ServiceBlockRuleTest, CheckReturnsBlockedIfMmtelFeatureUnavailableForMtCall)
{
    ON_CALL(objAosConnector, IsFeatureConnected(ImsAosFeature::MMTEL))
            .WillByDefault(Return(IMS_FALSE));
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objContext, IsCsfbAvailable).WillByDefault(Return(IMS_FALSE));

    Result objResult = ServiceBlockRule(objContext, CallType::VOIP).Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_SIP_488),
            objResult.objReason);
}

TEST_F(ServiceBlockRuleTest, CheckReturnsUnblockedIfOnlyVideoFeatureAvailableForVideoCall)
{
    ON_CALL(objAosConnector, IsFeatureConnected(ImsAosFeature::MMTEL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objAosConnector, IsFeatureConnected(ImsAosFeature::VIDEO))
            .WillByDefault(Return(IMS_TRUE));

    Result objResult = ServiceBlockRule(objContext, CallType::VT).Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}
