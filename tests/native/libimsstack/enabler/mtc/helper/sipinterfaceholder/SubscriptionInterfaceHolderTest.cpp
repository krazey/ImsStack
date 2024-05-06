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

#include "MockIOsFactory.h"
#include "PlatformContext.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "core/MockISubscription.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsTimerForSubsInterfaceHolder : public ImsTimer
{
public:
    inline TestImsTimerForSubsInterfaceHolder() {}
    inline ~TestImsTimerForSubsInterfaceHolder() {}

public:
    IMS_BOOL Equals(IN const ITimer* piTimer) const override
    {
        ImsTimer* pTimer = DYNAMIC_CAST(ImsTimer*, piTimer);
        return pTimer->GetTimerId() == reinterpret_cast<IMS_UINTP>(this);
    }

    IMS_UINTP SetTimer(IN IMS_UINT32 /*nDuration*/, IN ITimerListener* /*piListener*/) override
    {
        return reinterpret_cast<IMS_UINTP>(this);
    }

    void KillTimer() override {}

    IMS_UINTP GetTimerId() const override { return reinterpret_cast<IMS_UINTP>(this); }

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override {}
};

class SubscriptionInterfaceHolderTest : public ::testing::Test
{
public:
    MockIOsFactory objMockIOsFactory;
    IOsFactory* piOldOsFactory;
    TestImsTimerForSubsInterfaceHolder* pTestImsTimerForSubsInterfaceHolder;
    SubscriptionInterfaceHolder* pHolder;
    MockIInterfaceHolderListener objListener;
    MockISubscription objMockISubscription;
    MockISession objMockISession;
    MockICoreService objMockICoreService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockISession, CreateSubscription(_))
                .WillByDefault(Return(&objMockISubscription));
        ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
                .WillByDefault(Return(&objMockISubscription));
        pHolder = new SubscriptionInterfaceHolder(objListener);
    }

    virtual void TearDown() override { delete pHolder; }
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
    piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(&objMockIOsFactory);

    // will be deleted in the TimerService::DestroyTimer.
    pTestImsTimerForSubsInterfaceHolder = new TestImsTimerForSubsInterfaceHolder();
    EXPECT_CALL(objMockIOsFactory, CreateTimer())
            .Times(1)
            .WillOnce(Return(pTestImsTimerForSubsInterfaceHolder));

    ON_CALL(objMockISubscription, GetState).WillByDefault(Return(ISubscription::STATE_ACTIVE));
    EXPECT_CALL(objMockISubscription, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);

    pHolder->GetISubscription(&objMockISession, "someEvent");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 1);

    pHolder->ReleaseISubscription(&objMockISubscription, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISubscription));

    pHolder->Timer_TimerExpired(pTestImsTimerForSubsInterfaceHolder);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISubscription));
    EXPECT_EQ(pHolder->GetSubscriptionCount(), 0);

    PlatformContext::GetInstance()->SetOsFactory(piOldOsFactory);
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
