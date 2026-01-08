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

#include "IMtsContext.h"
#include "INetworkWatcher.h"
#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "MtsNetworkTracker.h"
#include "MockIMtsContext.h"
#include "MockINetworkWatcher.h"
#include <gtest/gtest.h>

using ::testing::Return;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 INVALID_EVENT = -1;

class TestMtsNetworkTracker : public MtsNetworkTracker
{
public:
    explicit TestMtsNetworkTracker(IN IMtsContext& objContext) :
            MtsNetworkTracker(objContext)
    {
    }
    virtual ~TestMtsNetworkTracker() override {}

    inline void SetNetworkWatcher(IN INetworkWatcher* piNw) { m_piNetWatcherInfo = piNw; }
};

class MtsNetworkTrackerTest : public ::testing::Test
{
public:
    MockIMtsContext objContext;
    MockINetworkWatcher objMockNetworkWatcher;
    TestMtsNetworkTracker* pNetworkTracker;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        pNetworkTracker = new TestMtsNetworkTracker(objContext);
        pNetworkTracker->SetNetworkWatcher(&objMockNetworkWatcher);
    }

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

TEST_F(MtsNetworkTrackerTest, GetNetworkTypeWhenNetworkWatcherIsNull)
{
    pNetworkTracker->SetNetworkWatcher(IMS_NULL);

    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkTracker->GetNetworkType());
}

TEST_F(MtsNetworkTrackerTest, GetNetworkTypeInVarietyOfNetwork)
{
    pNetworkTracker->SetNetworkWatcher(&objMockNetworkWatcher);
    EXPECT_CALL(objMockNetworkWatcher, GetNetworkType())
            .Times(4)
            .WillOnce(Return(INetworkWatcher::RADIOTECH_TYPE_NR))
            .WillOnce(Return(INetworkWatcher::RADIOTECH_TYPE_LTE_CA))
            .WillOnce(Return(INetworkWatcher::RADIOTECH_TYPE_IWLAN))
            .WillOnce(Return(INetworkWatcher::RADIOTECH_TYPE_UMTS));

    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkTracker->GetNetworkType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE_CA, pNetworkTracker->GetNetworkType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkTracker->GetNetworkType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_UMTS, pNetworkTracker->GetNetworkType());
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
