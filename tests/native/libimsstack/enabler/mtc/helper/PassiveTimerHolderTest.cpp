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
#include "helper/MockIPassiveTimerListener.h"
#include "helper/PassiveTimerHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Invoke;

LOCAL const IPassiveTimerHolder::Type ANY_TIMER_TYPE =
        IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER;
LOCAL const IMS_SINT32 ANY_TIMER_DURATION = 10000;

class PassiveTimerHolderTest : public ::testing::Test
{
public:
    inline PassiveTimerHolderTest() :
            objService(),
            objTimerService(),
            objTimer(objTimerService.GetMockTimer()),
            objListener(),
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
    MockIPassiveTimerListener objListener;

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

TEST_F(PassiveTimerHolderTest, AddTimerWithSameTimeDoesNothingIfDurationIsNegative)
{
    EXPECT_CALL(objTimer, SetTimer(_, _)).Times(0);

    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, -1, IMS_TRUE);
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
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

TEST_F(PassiveTimerHolderTest, RemoveTimerReleasesTimer)
{
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->RemoveTimer(ANY_TIMER_TYPE);
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, RemoveTimerDoesNothingIfNoMatchingTimer)
{
    pPassiveTimerHolder->RemoveTimer(ANY_TIMER_TYPE);
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, IsActiveReturnsFalseAfterAosDisconnected)
{
    pPassiveTimerHolder->AddTimer(ANY_TIMER_TYPE, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
    EXPECT_FALSE(pPassiveTimerHolder->IsActive(ANY_TIMER_TYPE));
}

TEST_F(PassiveTimerHolderTest, NotNotifyingListenerAfterAosDisconnected)
{
    pPassiveTimerHolder->AddTimer(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
    pPassiveTimerHolder->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);

    EXPECT_CALL(objListener, OnPassiveTimerExpired).Times(0);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);
}

TEST_F(PassiveTimerHolderTest, SetNormalServiceAddsAosStateListener)
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

TEST_F(PassiveTimerHolderTest, AddListenerAndTimerExpiringInvokesNotifyingListener)
{
    pPassiveTimerHolder->AddTimer(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
    EXPECT_CALL(objListener, OnPassiveTimerExpired).Times(1);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);
}

TEST_F(PassiveTimerHolderTest,
        AddListenerAndRemoveListenerAndTimerExpiringNotInvokesNotifyingListener)
{
    pPassiveTimerHolder->AddTimer(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
    MockIPassiveTimerListener objListener2;
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener2);
    pPassiveTimerHolder->RemoveListener(
            IPassiveTimerHolder::Type::SSAC_VOICE_BARRING, &objListener);

    EXPECT_CALL(objListener, OnPassiveTimerExpired).Times(1);
    EXPECT_CALL(objListener2, OnPassiveTimerExpired).Times(1);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);

    pPassiveTimerHolder->AddTimer(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener2);
    pPassiveTimerHolder->RemoveListener(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);

    EXPECT_CALL(objListener, OnPassiveTimerExpired).Times(0);
    EXPECT_CALL(objListener2, OnPassiveTimerExpired).Times(1);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);
}

TEST_F(PassiveTimerHolderTest, AddListenerAndRemoveListenerDoesNothingWithNoTimer)
{
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);

    EXPECT_CALL(objListener, OnPassiveTimerExpired).Times(0);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);

    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
    pPassiveTimerHolder->RemoveListener(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);

    EXPECT_CALL(objListener, OnPassiveTimerExpired).Times(0);
    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);
}

TEST_F(PassiveTimerHolderTest,
        AddListenerAndTimerExpiringAndRemoveListenerRightAfterThatIgnoresRemoveListener)
{
    pPassiveTimerHolder->AddTimer(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, ANY_TIMER_DURATION, IMS_FALSE);
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
    MockIPassiveTimerListener objListener2;
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener2);

    MockITimer objTimer2;
    objTimerService.SetTimer(&objTimer2);
    pPassiveTimerHolder->AddTimer(
            IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING, ANY_TIMER_DURATION, IMS_FALSE);
    MockIPassiveTimerListener objListener3;
    pPassiveTimerHolder->AddListener(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING, &objListener3);

    EXPECT_CALL(objListener, OnPassiveTimerExpired)
            .Times(1)
            .WillRepeatedly(Invoke(
                    [&]()
                    {
                        pPassiveTimerHolder->RemoveListener(
                                IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, &objListener);
                        pPassiveTimerHolder->RemoveListener(
                                IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING, &objListener3);
                    }));
    EXPECT_CALL(objListener2, OnPassiveTimerExpired).Times(1);

    pPassiveTimerHolder->Timer_TimerExpired(&objTimer);

    EXPECT_CALL(objListener3, OnPassiveTimerExpired).Times(0);

    pPassiveTimerHolder->Timer_TimerExpired(&objTimer2);
}
