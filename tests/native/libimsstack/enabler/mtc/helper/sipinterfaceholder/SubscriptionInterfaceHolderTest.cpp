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

#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockISession.h"
#include "MockISubscription.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class SubscriptionInterfaceHolderTest : public ::testing::Test
{
public:
    SubscriptionInterfaceHolder* pHolder;
    MockIInterfaceHolderListener objListener;
    MockISubscription objMockISubscription;
    MockISession objMockISession;
    MockICoreService objMockICoreService;
    TestTimerService objTimerService;
    MockITimer objMockITimer;

protected:
    virtual void SetUp() override
    {
        objTimerService.SetTimer(&objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
        ON_CALL(objMockISession, CreateSubscription(_))
                .WillByDefault(Return(&objMockISubscription));
        ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
                .WillByDefault(Return(&objMockISubscription));
        pHolder = new SubscriptionInterfaceHolder(objListener);
    }

    virtual void TearDown() override
    {
        delete pHolder;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(SubscriptionInterfaceHolderTest, HolderDoesNothingForSubscriptionListenerExceptTerminated)
{
    MockIMessage objIMessage;

    pHolder->SubscriptionForkedNotify(&objMockISubscription, &objMockISubscription);
    pHolder->SubscriptionNotify(&objMockISubscription, &objIMessage);
    pHolder->SubscriptionStarted(&objMockISubscription);
    pHolder->SubscriptionStartFailed(&objMockISubscription);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
}

TEST_F(SubscriptionInterfaceHolderTest,
        CreateSubscriptionByISessionAndSubscriptionTerminatedStopsTimer)
{
    EXPECT_CALL(objMockISubscription, Destroy()).Times(1);

    pHolder->GetISubscription(&objMockISession, "someEvent");
    pHolder->SubscriptionTerminated(&objMockISubscription);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);
}

TEST_F(SubscriptionInterfaceHolderTest,
        CreateSubscriptionByICoreServiceAndSubscriptionTerminatedStopsTimer)
{
    EXPECT_CALL(objMockISubscription, Destroy()).Times(1);

    pHolder->GetISubscription(&objMockICoreService, "sip:fromUri", "sip:toUri", "someEvent");
    pHolder->SubscriptionTerminated(&objMockISubscription);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);
}

TEST_F(SubscriptionInterfaceHolderTest, AddAndReleaseStartsTimer)
{
    ON_CALL(objMockISubscription, GetState).WillByDefault(Return(ISubscription::STATE_ACTIVE));
    EXPECT_CALL(objMockISubscription, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);

    pHolder->GetISubscription(&objMockISession, "someEvent");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 1);

    pHolder->ReleaseISubscription(&objMockISubscription, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISubscription));
}

TEST_F(SubscriptionInterfaceHolderTest, AddAndReleaseWithTimerExpiredStopsTimer)
{
    ON_CALL(objMockISubscription, GetState).WillByDefault(Return(ISubscription::STATE_ACTIVE));
    EXPECT_CALL(objMockISubscription, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);

    pHolder->GetISubscription(&objMockISession, "someEvent");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 1);

    pHolder->ReleaseISubscription(&objMockISubscription, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISubscription));

    pHolder->Timer_TimerExpired(&objMockITimer);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);
}

TEST_F(SubscriptionInterfaceHolderTest, AddAndReleaseWithTerminatedStopsTimer)
{
    ON_CALL(objMockISubscription, GetState).WillByDefault(Return(ISubscription::STATE_ACTIVE));
    EXPECT_CALL(objMockISubscription, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);

    pHolder->GetISubscription(&objMockISession, "someEvent");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 1);

    pHolder->ReleaseISubscription(&objMockISubscription, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISubscription));

    pHolder->ReleaseISubscription(&objMockISubscription, IMS_TRUE);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);
}

}  // namespace android
