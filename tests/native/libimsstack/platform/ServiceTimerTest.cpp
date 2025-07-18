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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsMutex.h"
#include "ImsTimer.h"
#include "MockIOsFactory.h"
#include "OsMutex.h"
#include "PlatformContext.h"
#include "ServiceTimer.h"

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsMutex : public ImsMutex
{
public:
    TestImsMutex() = default;
    ~TestImsMutex() override = default;

public:
    void Lock() override {}
    void Unlock() override {}
};

class TestImsTimer : public ImsTimer
{
public:
    inline TestImsTimer() :
            m_bServiceMsgDispatched(IMS_FALSE)
    {
    }

    ~TestImsTimer() override = default;

public:
    IMS_BOOL Equals(IN const ITimer* /*piTimer*/) const override { return IMS_TRUE; }

    IMS_UINTP SetTimer(IN IMS_UINT32 /*nDuration*/, IN ITimerListener* /*piListener*/) override
    {
        return reinterpret_cast<IMS_UINTP>(this);
    }

    void KillTimer() override {}

    IMS_UINTP GetTimerId() const override { return reinterpret_cast<IMS_UINTP>(this); }

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override
    {
        m_bServiceMsgDispatched = IMS_TRUE;
    }

    IMS_BOOL IsServiceMessageDispatched() { return m_bServiceMsgDispatched; }

private:
    IMS_BOOL m_bServiceMsgDispatched;
};

class TimerServiceTest : public ::testing::Test
{
public:
    inline TimerServiceTest() :
            m_pTimerService(IMS_NULL),
            m_pOldTimerService(IMS_NULL),
            m_pOldIOsFactory(IMS_NULL),
            m_pTestImsTimer(IMS_NULL),
            m_pImsMutex(IMS_NULL)
    {
    }

    inline ~TimerServiceTest() {}

protected:
    virtual void SetUp() override
    {
        m_pOldIOsFactory = PlatformContext::GetInstance()->SetOsFactory(&m_objMockIOsFactory);

        m_pImsMutex = new TestImsMutex();

        EXPECT_CALL(m_objMockIOsFactory, CreateMutex(_)).Times(1).WillOnce(Return(m_pImsMutex));

        m_pTimerService = new TimerService();
        m_pTestImsTimer = new TestImsTimer();
        m_pOldTimerService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, m_pTimerService);
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetOsFactory(m_pOldIOsFactory);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, m_pOldTimerService);
        m_pTimerService->Destroy();
    }

protected:
    TimerService* m_pTimerService;
    PlatformService* m_pOldTimerService;
    MockIOsFactory m_objMockIOsFactory;
    IOsFactory* m_pOldIOsFactory;
    TestImsTimer* m_pTestImsTimer;
    ImsMutex* m_pImsMutex;
};

TEST_F(TimerServiceTest, CreateTimer)
{
    TimerService* pTimerService = TimerService::GetTimerService();

    EXPECT_CALL(m_objMockIOsFactory, CreateTimer()).Times(1).WillOnce(Return(m_pTestImsTimer));

    ImsTimer* pImsTimer = static_cast<ImsTimer*>(pTimerService->CreateTimer());

    ASSERT_TRUE(pImsTimer != IMS_NULL);

    EXPECT_TRUE(m_pTestImsTimer->GetTimerId() == pImsTimer->GetTimerId());

    m_pTestImsTimer->Destroy();
}

TEST_F(TimerServiceTest, DestroyTimer)
{
    TimerService* pTimerService = TimerService::GetTimerService();

    EXPECT_CALL(m_objMockIOsFactory, CreateTimer()).Times(1).WillOnce(Return(m_pTestImsTimer));

    ITimer* pITimer = pTimerService->CreateTimer();

    ASSERT_TRUE(pITimer != IMS_NULL);

    pTimerService->DestroyTimer(pITimer);

    EXPECT_TRUE(pITimer == IMS_NULL);
}

TEST_F(TimerServiceTest, DispatchServiceMessage)
{
    TimerService* pTimerService = TimerService::GetTimerService();

    EXPECT_CALL(m_objMockIOsFactory, CreateTimer()).Times(1).WillOnce(Return(m_pTestImsTimer));

    ITimer* pITimer = pTimerService->CreateTimer();

    ASSERT_TRUE(pITimer != IMS_NULL);

    EXPECT_FALSE(m_pTestImsTimer->IsServiceMessageDispatched());

    ImsMessage objMessage(IMS_MSG_TIMER, reinterpret_cast<IMS_UINTP>(m_pTestImsTimer), 1);

    pTimerService->DispatchServiceMessage(objMessage);

    EXPECT_TRUE(m_pTestImsTimer->IsServiceMessageDispatched());

    ImsMessage objMessage1(IMS_MSG_CONFIGURATION, reinterpret_cast<IMS_UINTP>(m_pTestImsTimer),
            ImsTimer::MSG_PARAM_DESTROY);

    // m_pTestImsTimer gets deleted
    pTimerService->DispatchServiceMessage(objMessage1);
}

}  // namespace android