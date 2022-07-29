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
#include "MockINetworkWatcher.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/NetworkBlockRule.h"

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class NetworkBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcService objService;
    MockIMtcCallContext objContext;
    MockINetworkWatcher objNetworkWatcher;
    MockIMtcBlockRuleCheckListener objListener;
    NetworkBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService)
                .WillByDefault(ReturnRef(objService));

        pBlockRule = new NetworkBlockRule(objContext, objNetworkWatcher);
    }

    virtual void TearDown() override
    {
        delete pBlockRule;
    }
};

TEST_F(NetworkBlockRuleTest, CheckReturnsUnblockedIfInEpdg)
{
    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_TRUE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(NetworkBlockRuleTest, CheckReturnsUnblockedIfWifiRegistered)
{
    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsWifiRegistered)
            .WillByDefault(Return(IMS_TRUE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(NetworkBlockRuleTest, CheckReturnsUnblockedIfLte)
{
    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsWifiRegistered)
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(NetworkBlockRuleTest, CheckReturnsUnblockedIfNr)
{
    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsWifiRegistered)
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(NetworkBlockRuleTest, CheckReturnsBlockedIfNotSupportedNetwork)
{
    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsWifiRegistered)
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objService, GetAosConnector)
            .WillByDefault(Return(nullptr));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_INVALID));

    Result objResult = pBlockRule->Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE), objResult.objReason);
}
