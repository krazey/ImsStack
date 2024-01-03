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
#include "ServiceTimer.h"
#include "call/state/MtcCallState.h"
#include "helper/IMtcTimerListener.h"
#include "helper/MtcTimerWrapper.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsMutexForMtc : public ImsMutex
{
public:
    inline TestImsMutexForMtc() {}
    inline virtual ~TestImsMutexForMtc() {}

public:
    void Lock() override {}
    void Unlock() override {}
};

class TestImsTimerForMtc : public ImsTimer
{
public:
    inline TestImsTimerForMtc() {}
    inline ~TestImsTimerForMtc() {}

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

class MtcTimerWrapperTest : public ::testing::Test, public IMtcTimerListener
{
public:
    void OnTimerExpired(IN IMS_SINT32 /*nType*/) override { bTimerExpired = IMS_TRUE; }

    MockIOsFactory objMockIOsFactory;
    IOsFactory* pOldIOsFactory;
    MtcTimerWrapper objMtcTimerWrapper;
    TestImsTimerForMtc* pTestImsTimerForMtc1;
    TestImsTimerForMtc* pTestImsTimerForMtc2;
    IMS_BOOL bTimerExpired;
    IMS_UINT32 nAnyDuration = 10000;
    TimerService* pTimerService;
    PlatformService* pOldTimerService;
    ImsMutex* pImsMutex;

protected:
    virtual void SetUp() override
    {
        pOldIOsFactory = PlatformContext::GetInstance()->SetOsFactory(&objMockIOsFactory);

        objMtcTimerWrapper.SetListener(this);
        bTimerExpired = IMS_FALSE;

        // Mutex will be deleted by test TimerService deleting.
        pImsMutex = new TestImsMutexForMtc();
        ON_CALL(objMockIOsFactory, CreateMutex(_)).WillByDefault(Return(pImsMutex));

        pTimerService = new TimerService();
        pOldTimerService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, pTimerService);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetOsFactory(pOldIOsFactory);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, pOldTimerService);
        pTimerService->Destroy();
    }
};

TEST_F(MtcTimerWrapperTest, StartStopInvokesAddingRemovingTimer)
{
    pTestImsTimerForMtc1 = new TestImsTimerForMtc();
    EXPECT_CALL(objMockIOsFactory, CreateTimer()).Times(1).WillOnce(Return(pTestImsTimerForMtc1));

    objMtcTimerWrapper.Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, nAnyDuration);

    EXPECT_TRUE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));

    objMtcTimerWrapper.Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT);

    EXPECT_FALSE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
}

TEST_F(MtcTimerWrapperTest, StopAllRemovesAllTimer)
{
    pTestImsTimerForMtc1 = new TestImsTimerForMtc();
    pTestImsTimerForMtc2 = new TestImsTimerForMtc();
    EXPECT_CALL(objMockIOsFactory, CreateTimer())
            .Times(2)
            .WillOnce(Return(pTestImsTimerForMtc1))
            .WillOnce(Return(pTestImsTimerForMtc2));

    objMtcTimerWrapper.Start(MtcCallState::TimerType::TIMER_E911_LTE_OPEN, nAnyDuration);
    objMtcTimerWrapper.Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, nAnyDuration);

    EXPECT_TRUE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_E911_LTE_OPEN));
    EXPECT_TRUE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT));

    objMtcTimerWrapper.StopAll();

    EXPECT_FALSE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_E911_LTE_OPEN));
    EXPECT_FALSE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
}

TEST_F(MtcTimerWrapperTest, Timer_TimerExpiredRemovesTimerAndNotifies)
{
    pTestImsTimerForMtc1 = new TestImsTimerForMtc();
    EXPECT_CALL(objMockIOsFactory, CreateTimer()).Times(1).WillOnce(Return(pTestImsTimerForMtc1));

    objMtcTimerWrapper.Start(MtcCallState::TimerType::TIMER_MO_100_WAIT, nAnyDuration);

    EXPECT_TRUE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
    EXPECT_FALSE(bTimerExpired);

    objMtcTimerWrapper.Timer_TimerExpired(pTestImsTimerForMtc1);

    EXPECT_FALSE(objMtcTimerWrapper.IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT));
    EXPECT_TRUE(bTimerExpired);
}

}  // namespace android
