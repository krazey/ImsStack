/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "MockIMtcService.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/PassiveTimerHolder.h"
#include <gtest/gtest.h>

#define ANY_TIMER_TYPE     IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER
#define ANY_TIMER_DURATION 10000

class PassiveTimerHolderTest : public ::testing::Test
{
public:
    inline PassiveTimerHolderTest() :
            objService(),
            objTimerService(),
            objTimer(objTimerService.GetMockTimer()),
            pPassiveTimerHolder(new PassiveTimerHolder())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
    }

    inline ~PassiveTimerHolderTest()
    {
        delete pPassiveTimerHolder;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

protected:
    MockIMtcService objService;
    TestTimerService objTimerService;
    MockITimer& objTimer;

    PassiveTimerHolder* pPassiveTimerHolder;
};

TEST_F(PassiveTimerHolderTest, IsActiveReturnsFalseInitially)
{
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, AddTimerCreatesTimerAndLetsIsActiveReturnTrue)
{
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    EXPECT_TRUE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, AddTimerWithSameTimeDoesNothingIfNotAllowsReset)
{
    EXPECT_CALL(objTimer, SetTimer(ANY_TIMER_DURATION, pPassiveTimerHolder)).Times(1);

    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    EXPECT_TRUE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    EXPECT_TRUE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, AddTimerWithSameTimeResetsTimerIfAllowsReset)
{
    EXPECT_CALL(objTimer, SetTimer(ANY_TIMER_DURATION, pPassiveTimerHolder)).Times(2);

    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_TRUE);
    EXPECT_TRUE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_TRUE);
    EXPECT_TRUE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, IsActiveReturnsFalseAfterAosDisconnected)
{
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, SerNormalServiceAddsAosStateListener)
{
    EXPECT_CALL(objService, AddAosStateListener(pPassiveTimerHolder));
    pPassiveTimerHolder->SetNormalService(&objService);
    EXPECT_CALL(objService, RemoveAosStateListener(pPassiveTimerHolder));
}

TEST_F(PassiveTimerHolderTest, TimerExpiredReleasesTimer)
{
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, InvalidTimerExpiredInvokesNothing)
{
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    MockITimer* pDiffTimer = new MockITimer();
    pPassiveTimerHolder->Timer_TimerExpired(pDiffTimer);
    EXPECT_TRUE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));

    delete pDiffTimer;
}
