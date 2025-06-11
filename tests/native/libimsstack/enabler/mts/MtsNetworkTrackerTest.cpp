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

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "MtsNetworkTracker.h"
#include "MockIMtsContext.h"
#include <gtest/gtest.h>

namespace android
{

const IMS_SINT32 INVALID_EVENT = -1;

class MtsNetworkTrackerTest : public ::testing::Test
{
public:
    MockIMtsContext objContext;
    MtsNetworkTracker* pNetworkTracker;

protected:
    virtual void SetUp() override { pNetworkTracker = new MtsNetworkTracker(objContext); }

    virtual void TearDown() override { delete pNetworkTracker; }
};

TEST_F(MtsNetworkTrackerTest, Constructor)
{
    ASSERT_NE(pNetworkTracker, nullptr);
}

TEST_F(MtsNetworkTrackerTest, GetNetworkStatusAfterInvalidEventReturnsInitialValues)
{
    pNetworkTracker->Event_NotifyEvent(INVALID_EVENT, 1, 2);

    EXPECT_EQ(IMS_LTE_INFO_UNKNOWN, pNetworkTracker->GetLteAttachState());
    EXPECT_EQ(IMS_FALSE, pNetworkTracker->IsInRoamingState());
}

TEST_F(MtsNetworkTrackerTest, Event_NotifyEventAndRoamingStateChanged)
{
    EXPECT_EQ(IMS_FALSE, pNetworkTracker->IsInRoamingState());

    pNetworkTracker->Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, 1, 0);
    EXPECT_EQ(IMS_TRUE, pNetworkTracker->IsInRoamingState());

    pNetworkTracker->Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, 0, 0);
    EXPECT_EQ(IMS_FALSE, pNetworkTracker->IsInRoamingState());
}

TEST_F(MtsNetworkTrackerTest, Event_NotifyEventAndLteAttachTypeChanged)
{
    EXPECT_EQ(IMS_LTE_INFO_UNKNOWN, pNetworkTracker->GetLteAttachState());

    pNetworkTracker->Event_NotifyEvent(IMS_EVENT_LTE_INFO, 1, 2);
    EXPECT_EQ(IMS_LTE_INFO_EPS_ONLY_ATTACHED, pNetworkTracker->GetLteAttachState());

    pNetworkTracker->Event_NotifyEvent(IMS_EVENT_LTE_INFO, 1, 2);
    EXPECT_EQ(IMS_LTE_INFO_EPS_ONLY_ATTACHED, pNetworkTracker->GetLteAttachState());

    pNetworkTracker->Event_NotifyEvent(IMS_EVENT_LTE_INFO, 2, 2);
    EXPECT_EQ(IMS_LTE_INFO_COMBINED_ATTACHED, pNetworkTracker->GetLteAttachState());
}

}  // namespace android
