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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "provider/AosKeepAlive.h"
#include "provider/MockAosKeepAlive.h"

const IMS_SINT32 SLOT_ID = 0;
enum
{
    TIMER_KEEP_ALIVE = 0,
    TIMER_PONG_WAIT
};

class AosKeepAliveTest : public ::testing::Test
{
public:
    AosKeepAlive* m_pAosKeepAlive;

protected:
    virtual void SetUp() override
    {
        m_pAosKeepAlive = new AosKeepAlive(SLOT_ID);
        ASSERT_TRUE(m_pAosKeepAlive != nullptr);

        SetKeepAliveHelper(IMS_NULL);
    }

    virtual void TearDown() override
    {
        if (m_pAosKeepAlive)
        {
            delete m_pAosKeepAlive;
        }
    }

    IMS_UINT32 GetKeepAliveTime() { return m_pAosKeepAlive->m_nKeepAliveTime; }

    void SetKeepAliveHelper(IN ISipKeepAliveHelper* piKeepAliveHelper)
    {
        m_pAosKeepAlive->m_piKeepAliveHelper = piKeepAliveHelper;
    }

    IMS_BOOL IsActive_KeepAliveTimer() { return (m_pAosKeepAlive->m_piKeepAliveTimer != IMS_NULL); }

    IMS_BOOL IsActive_PongWaitTimer() { return (m_pAosKeepAlive->m_piPongWaitTimer != IMS_NULL); }

    ITimer* GetKeepAliverTimer() { return m_pAosKeepAlive->m_piKeepAliveTimer; }

    ITimer* GetPongWaitTimer() { return m_pAosKeepAlive->m_piPongWaitTimer; }

    void ProcessPongWaitTimerExpired() { m_pAosKeepAlive->ProcessPongWaitTimerExpired(); }

    void SetCheckingPong(IN IMS_BOOL bCheck) { m_pAosKeepAlive->SetCheckingPong(bCheck); }

    IMS_BOOL IsPongChecked() { return m_pAosKeepAlive->IsPongChecked(); }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        m_pAosKeepAlive->StartTimer(nType, nDuration);
    }

    void ClearTimer() { m_pAosKeepAlive->ClearTimer(); }

    void ProcessKeepAliveTimerExpired() { m_pAosKeepAlive->ProcessKeepAliveTimerExpired(); }

    void KeepAliveHelper_PongReceived() { m_pAosKeepAlive->KeepAliveHelper_PongReceived(); }

    void Timer_TimerExpired(IN ITimer* piTimer) { m_pAosKeepAlive->Timer_TimerExpired(piTimer); }

    const IMS_CHAR* TimerToString(IN IMS_UINT32 nType)
    {
        return m_pAosKeepAlive->TimerToString(nType);
    }
};

TEST_F(AosKeepAliveTest, SetListener)
{
    MockIAosKeepAliveListener objMockIAosKeepAliveListener;
    EXPECT_CALL(objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(1);

    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&objMockIAosKeepAliveListener));

    ProcessPongWaitTimerExpired();
}

TEST_F(AosKeepAliveTest, Start_CheckingPongTrue)
{
    SetCheckingPong(IMS_FALSE);
    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    EXPECT_TRUE(IsPongChecked());
}

TEST_F(AosKeepAliveTest, Start_CheckingPongFalse)
{
    SetCheckingPong(IMS_TRUE);
    m_pAosKeepAlive->Start(20000, IMS_FALSE);

    EXPECT_FALSE(IsPongChecked());
}

TEST_F(AosKeepAliveTest, Stop)
{
    SetCheckingPong(IMS_FALSE);
    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    EXPECT_EQ(GetKeepAliveTime(), 20000);
    m_pAosKeepAlive->Stop();
    EXPECT_EQ(GetKeepAliveTime(), 0);
}

TEST_F(AosKeepAliveTest, SetTransport_KeepAliveHelperIsNull)
{
    EXPECT_FALSE(m_pAosKeepAlive->SetTransport(
            IpAddress("127.0.0.1"), 1234, IpAddress("127.0.0.2"), 5678, 1));
}

TEST_F(AosKeepAliveTest, ProcessKeepAliveTimerExpired_IsPongChecked)
{
    m_pAosKeepAlive->Start(20000, IMS_TRUE);
    ClearTimer();

    EXPECT_FALSE(IsActive_KeepAliveTimer());
    EXPECT_FALSE(IsActive_PongWaitTimer());
    ProcessKeepAliveTimerExpired();

    EXPECT_TRUE(IsActive_KeepAliveTimer());
    EXPECT_TRUE(IsActive_PongWaitTimer());
}

TEST_F(AosKeepAliveTest, ProcessKeepAliveTimerExpired_IsNotPongChecked)
{
    m_pAosKeepAlive->Start(20000, IMS_FALSE);
    ClearTimer();

    EXPECT_FALSE(IsActive_KeepAliveTimer());
    EXPECT_FALSE(IsActive_PongWaitTimer());
    ProcessKeepAliveTimerExpired();

    EXPECT_TRUE(IsActive_KeepAliveTimer());
    EXPECT_FALSE(IsActive_PongWaitTimer());
}

TEST_F(AosKeepAliveTest, KeepAliveHelper_PongReceived)
{
    StartTimer(TIMER_PONG_WAIT, 1000);
    EXPECT_TRUE(IsActive_PongWaitTimer());

    KeepAliveHelper_PongReceived();
    EXPECT_FALSE(IsActive_PongWaitTimer());
}

TEST_F(AosKeepAliveTest, Timer_TimerExpired_KeepAliveTimer)
{
    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    Timer_TimerExpired(GetKeepAliverTimer());

    EXPECT_TRUE(IsActive_KeepAliveTimer());
    EXPECT_TRUE(IsActive_PongWaitTimer());
}

TEST_F(AosKeepAliveTest, Timer_TimerExpired_PongWaitTimer)
{
    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    MockIAosKeepAliveListener objMockIAosKeepAliveListener;
    EXPECT_CALL(objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(1);

    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&objMockIAosKeepAliveListener));

    Timer_TimerExpired(GetPongWaitTimer());

    EXPECT_FALSE(IsActive_KeepAliveTimer());
    EXPECT_FALSE(IsActive_PongWaitTimer());
}

TEST_F(AosKeepAliveTest, TimerToString)
{
    EXPECT_STREQ(TimerToString(TIMER_KEEP_ALIVE), "TIMER_KEEP_ALIVE");
    EXPECT_STREQ(TimerToString(TIMER_PONG_WAIT), "TIMER_PONG_WAIT");
    EXPECT_STREQ(TimerToString(-1), "__INVALID__");
}