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

#include "MockIPhoneInfoLocation.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/LocationBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MtcLocationRefresher.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using Result = IMtcBlockRule::Result;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class LocationBlockRuleTest : public ::testing::Test
{
public:
    inline explicit LocationBlockRuleTest() {}

    MockIMtcCallContext objContext;
    CallInfo objCallInfo;
    MockIMtcBlockRuleCheckListener objListener;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockILocationInfo objLocationInfo;
    MtcLocationRefresher* pLocationRefresher;

    LocationBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        pLocationRefresher = new MtcLocationRefresher(objLocationInfo);
        ON_CALL(objContext, GetLocationRefresher).WillByDefault(ReturnRef(*pLocationRefresher));

        pBlockRule = new LocationBlockRule(objContext);
    }

    virtual void TearDown() override
    {
        delete pBlockRule;
        delete pLocationRefresher;
    }
};

TEST_F(LocationBlockRuleTest, CheckReturnsUnblockedIfNotEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(1000));

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, CheckReturnsUnblockedIfLocationRefreshTimerIsNotSet)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(0));

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, CheckReturnsPendingIfLocationRefreshing)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(1000));

    pLocationRefresher->RequestUpdate(1000);

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::PENDING, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, CheckReturnsUnblockedIfLocationRefreshIsCompleted)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(1000));

    pLocationRefresher->RequestUpdate(1000);
    pLocationRefresher->LocationUpdate_OnCompleted();

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, NotifyListenerUnblockedWhenLocationIsUpdated)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(1000));

    pLocationRefresher->RequestUpdate(1000);
    pBlockRule->Check(objListener);
    pLocationRefresher->LocationUpdate_OnCompleted();

    EXPECT_CALL(objListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED)));
    pBlockRule->LocationUpdate_OnCompleted();
}
