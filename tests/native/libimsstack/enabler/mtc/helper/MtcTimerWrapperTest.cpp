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

#include "ITimer.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "call/state/MtcCallState.h"
#include "helper/IMtcTimerListener.h"
#include "helper/MtcTimerWrapper.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class MtcTimerWrapperTest : public ::testing::Test, public IMtcTimerListener
{
public:
    TestTimerService objTimerService;
    IMS_BOOL bTimerExpired;
    IMS_UINT32 nAnyDuration = 10000;
    MtcTimerWrapper* pMtcTimerWrapper;

    void OnTimerExpired(IN IMS_SINT32 /*nType*/) override { bTimerExpired = IMS_TRUE; }

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);

        pMtcTimerWrapper = new MtcTimerWrapper();
        pMtcTimerWrapper->SetListener(this);
        bTimerExpired = IMS_FALSE;
    }

    virtual void TearDown() override
    {
        delete pMtcTimerWrapper;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(MtcTimerWrapperTest, StartStopInvokesAddingRemovingTimer)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(nAnyDuration, pMtcTimerWrapper));
    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, nAnyDuration);
    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));

    EXPECT_CALL(objTimerService.GetMockTimer(), KillTimer);
    pMtcTimerWrapper->Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT);
    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
}

TEST_F(MtcTimerWrapperTest, StartDoesNothingWithInvalidDuration)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(nAnyDuration, pMtcTimerWrapper)).Times(0);

    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, -1);

    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
}

TEST_F(MtcTimerWrapperTest, StartInvokesStopIfExistedType)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(nAnyDuration, pMtcTimerWrapper));
    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, nAnyDuration);
    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));

    EXPECT_CALL(objTimerService.GetMockTimer(), KillTimer);
    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, -1);
    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
}

TEST_F(MtcTimerWrapperTest, StartInvokesAddingTimerWithDuration0)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(0, pMtcTimerWrapper));

    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, 0);

    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
}

TEST_F(MtcTimerWrapperTest, StopDoesNothingWithNotExistedType)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(nAnyDuration, pMtcTimerWrapper));

    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_E911_LTE_OPEN, nAnyDuration);

    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_E911_LTE_OPEN));
    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT));

    pMtcTimerWrapper->Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT);

    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_E911_LTE_OPEN));
    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
}

TEST_F(MtcTimerWrapperTest, StopAllRemovesAllTimer)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(nAnyDuration, pMtcTimerWrapper)).Times(2);

    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_E911_LTE_OPEN, nAnyDuration);
    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, nAnyDuration);

    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_E911_LTE_OPEN));
    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
    EXPECT_CALL(objTimerService.GetMockTimer(), KillTimer).Times(2);

    pMtcTimerWrapper->StopAll();

    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_E911_LTE_OPEN));
    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
}

TEST_F(MtcTimerWrapperTest, TimerTimerExpiredRemovesTimerAndNotifies)
{
    EXPECT_CALL(objTimerService.GetMockTimer(), SetTimer(nAnyDuration, pMtcTimerWrapper));

    pMtcTimerWrapper->Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, nAnyDuration);

    EXPECT_TRUE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
    EXPECT_FALSE(bTimerExpired);

    EXPECT_CALL(objTimerService.GetMockTimer(), KillTimer);

    // This pointer is actually member variable of `TestTimerService`. So, it will naturally be
    // released.
    ITimer* piTimer = objTimerService.CreateTimer();
    pMtcTimerWrapper->Timer_TimerExpired(piTimer);

    EXPECT_FALSE(pMtcTimerWrapper->IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
    EXPECT_TRUE(bTimerExpired);
}

}  // namespace android
