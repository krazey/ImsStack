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
#include "call/block/LocationBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
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

    MockIMtcBlockRuleCheckListener objListener;
    MockILocationInfo objLocationInfo;
    MtcLocationRefresher* pLocationRefresher;

    LocationBlockRule* pBlockRule;

protected:
    virtual void SetUp() override
    {
        pLocationRefresher = new MtcLocationRefresher(objLocationInfo);
        pBlockRule = new LocationBlockRule(*pLocationRefresher);
    }

    virtual void TearDown() override
    {
        delete pBlockRule;
        delete pLocationRefresher;
    }
};

TEST_F(LocationBlockRuleTest, CheckReturnsUnblockedIfNoLocationRefreshHaveBeenRequested)
{
    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, CheckReturnsPendingIfLocationRefreshing)
{
    pLocationRefresher->RequestUpdate(1000);

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::PENDING, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, CheckReturnsUnblockedIfLocationRefreshIsCompleted)
{
    pLocationRefresher->RequestUpdate(1000);
    pLocationRefresher->LocationUpdate_OnCompleted();

    Result objResult = pBlockRule->Check(objListener);
    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(LocationBlockRuleTest, NotifyListenerUnblockedWhenLocationIsUpdated)
{
    pLocationRefresher->RequestUpdate(1000);
    pBlockRule->Check(objListener);
    pLocationRefresher->LocationUpdate_OnCompleted();

    EXPECT_CALL(objListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED)));
    pBlockRule->LocationUpdate_OnCompleted();
}
