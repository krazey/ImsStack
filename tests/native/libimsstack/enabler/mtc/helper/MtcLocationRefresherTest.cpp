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
#include "helper/MtcLocationRefresher.h"
#include <gtest/gtest.h>

using LocationRefreshState = MtcLocationRefresher::LocationRefreshState;

namespace android
{

class MtcLocationRefresherTest : public ::testing::Test
{
public:
    MockILocationInfo objLocationInfo;
    MtcLocationRefresher* pLocationRefresher;

protected:
    virtual void SetUp() override
    {
        pLocationRefresher = new MtcLocationRefresher(objLocationInfo);
    }

    virtual void TearDown() override { delete pLocationRefresher; }
};

TEST_F(MtcLocationRefresherTest, RequestLocationUpdate)
{
    EXPECT_CALL(objLocationInfo, RequestLocationUpdate(1000, pLocationRefresher));
    pLocationRefresher->RequestUpdate(1000);
    EXPECT_EQ(LocationRefreshState::REFRESHING, pLocationRefresher->GetState());
}

TEST_F(MtcLocationRefresherTest, RequestLocationUpdateAgainAfterUpdated)
{
    EXPECT_CALL(objLocationInfo, RequestLocationUpdate(1000, pLocationRefresher));
    pLocationRefresher->RequestUpdate(1000);
    pLocationRefresher->LocationUpdate_OnCompleted();

    EXPECT_CALL(objLocationInfo, RequestLocationUpdate(1000, pLocationRefresher));
    pLocationRefresher->RequestUpdate(1000);
}

TEST_F(MtcLocationRefresherTest, DoesNotRequestLocationUpdateIfRefreshing)
{
    EXPECT_CALL(objLocationInfo, RequestLocationUpdate(1000, pLocationRefresher)).Times(1);
    pLocationRefresher->RequestUpdate(1000);
    pLocationRefresher->RequestUpdate(2000);
}

TEST_F(MtcLocationRefresherTest, NotifyListenersWhenUpdated)
{
    MockILocationUpdateListener objListener1;
    MockILocationUpdateListener objListener2;

    EXPECT_CALL(objListener1, LocationUpdate_OnCompleted());
    EXPECT_CALL(objListener2, LocationUpdate_OnCompleted());

    pLocationRefresher->AddListener(objListener1);
    pLocationRefresher->AddListener(objListener2);
    pLocationRefresher->LocationUpdate_OnCompleted();
}

TEST_F(MtcLocationRefresherTest, DoesNotNotifyRemovedListeners)
{
    MockILocationUpdateListener objListener1;
    MockILocationUpdateListener objListener2;

    EXPECT_CALL(objListener1, LocationUpdate_OnCompleted()).Times(0);
    EXPECT_CALL(objListener2, LocationUpdate_OnCompleted());

    pLocationRefresher->AddListener(objListener1);
    pLocationRefresher->AddListener(objListener2);
    pLocationRefresher->RemoveListener(objListener1);
    pLocationRefresher->LocationUpdate_OnCompleted();
}

TEST_F(MtcLocationRefresherTest, GetStateReturnsIdleInitially)
{
    EXPECT_EQ(LocationRefreshState::IDLE, pLocationRefresher->GetState());
}

TEST_F(MtcLocationRefresherTest, GetStateReturnsRefreshingAfterRequesting)
{
    pLocationRefresher->RequestUpdate(1000);

    EXPECT_EQ(LocationRefreshState::REFRESHING, pLocationRefresher->GetState());
}

TEST_F(MtcLocationRefresherTest, GetStateReturnsIdleAfterUpdate)
{
    pLocationRefresher->RequestUpdate(1000);
    pLocationRefresher->LocationUpdate_OnCompleted();

    EXPECT_EQ(LocationRefreshState::IDLE, pLocationRefresher->GetState());
}

}  // namespace android
