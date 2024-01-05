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
#include "precondition/IQosTimerListener.h"
#include "precondition/QosTimer.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsMutexForQos : public ImsMutex
{
public:
    inline TestImsMutexForQos() {}
    inline virtual ~TestImsMutexForQos() {}

public:
    void Lock() override {}
    void Unlock() override {}
};

class TestImsTimerForQos : public ImsTimer
{
public:
    inline TestImsTimerForQos() {}
    inline ~TestImsTimerForQos() {}

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

class QosTimerTest : public ::testing::Test, public IQosTimerListener
{
public:
    MockIOsFactory objMockIOsFactory;
    IOsFactory* pOldIOsFactory;
    TimerService* pTimerService;
    TestImsTimerForQos* pTestImsTimerForQos;
    PlatformService* pOldTimerService;
    ImsMutex* pImsMutex;
    IMS_BOOL bTimerExpired;
    IMS_UINT32 nAnyDuration = 10000;
    QosTimer* pQosTimer;

    void OnTimerExpired(IN QosTimer* /*pTimer*/, IN QosTimerType /*eType*/) override
    {
        bTimerExpired = IMS_TRUE;
    }

protected:
    virtual void SetUp() override
    {
        pOldIOsFactory = PlatformContext::GetInstance()->SetOsFactory(&objMockIOsFactory);

        // Mutex will be deleted by test TimerService deleting.
        pImsMutex = new TestImsMutexForQos();
        ON_CALL(objMockIOsFactory, CreateMutex(_)).WillByDefault(Return(pImsMutex));

        pTimerService = new TimerService();
        pOldTimerService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, pTimerService);
        // TestImsTimerForQos will be deleted by QosTimer.
        pTestImsTimerForQos = new TestImsTimerForQos();
        ON_CALL(objMockIOsFactory, CreateTimer()).WillByDefault(Return(pTestImsTimerForQos));

        bTimerExpired = IMS_FALSE;
        pQosTimer = new QosTimer(this);
    }

    virtual void TearDown() override
    {
        delete pQosTimer;

        PlatformContext::GetInstance()->SetOsFactory(pOldIOsFactory);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, pOldTimerService);
        pTimerService->Destroy();
    }
};

TEST_F(QosTimerTest, StartQosTimerStopQosTimerAddsRemovesTimer)
{
    pQosTimer->StartQosTimer(QosTimerType::GUARD_AVAILABLE, nAnyDuration);

    EXPECT_TRUE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));

    pQosTimer->StopQosTimer(QosTimerType::GUARD_AVAILABLE);

    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));
}

TEST_F(QosTimerTest, TimerTimerExpiredRemovesTimerAndNotifies)
{
    pQosTimer->StartQosTimer(QosTimerType::GUARD_AVAILABLE, nAnyDuration);

    EXPECT_TRUE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));

    pQosTimer->Timer_TimerExpired(pTestImsTimerForQos);

    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));
    EXPECT_TRUE(bTimerExpired);
}

}  // namespace android
