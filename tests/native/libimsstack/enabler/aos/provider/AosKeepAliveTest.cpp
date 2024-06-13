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

#include "../../../engine/interface/sipcore/MockISipKeepAliveHelper.h"
#include "provider/AosKeepAlive.h"
#include "provider/MockAosKeepAlive.h"

using ::testing::_;

const IMS_SINT32 SLOT_ID = 0;

#define DECLARE_USING(Base)                   \
    using Base::SendPing;                     \
    using Base::SetCheckingPong;              \
    using Base::IsPongChecked;                \
    using Base::ProcessKeepAliveTimerExpired; \
    using Base::ProcessPongWaitTimerExpired;  \
    using Base::StartTimer;                   \
    using Base::StopTimer;                    \
    using Base::ClearTimer;                   \
    using Base::KeepAliveHelper_PongReceived; \
    using Base::Timer_TimerExpired;           \
    using Base::TimerToString;

class TestAosKeepAlive : public AosKeepAlive
{
public:
    DECLARE_USING(AosKeepAlive)

    inline explicit TestAosKeepAlive(IN IMS_SINT32 nSlotId) :
            AosKeepAlive(nSlotId)
    {
    }

    inline void SetKeepAliveHelper(IN ISipKeepAliveHelper* piKeepAliveHelper)
    {
        m_piKeepAliveHelper = piKeepAliveHelper;
    }

    inline IAosKeepAliveListener* GetListener() { return m_piListener; }

    inline ITimer* GetKeepAliveTimer() { return m_piKeepAliveTimer; }

    inline ITimer* GetPongWaitTimer() { return m_piPongWaitTimer; }

    inline IMS_UINT32 GetKeepAliveTime() { return m_nKeepAliveTime; }

    inline void SetKeepAliveTime(IN IMS_UINT32 nTime) { m_nKeepAliveTime = nTime; }

    static const IMS_UINT32 GRATER_TIME_THAN_POING_WAIT = PONG_WAIT_TIME_MILLIS + 10000;
    static const IMS_UINT32 SMALLER_TIME_THAN_POING_WAIT = PONG_WAIT_TIME_MILLIS - 10000;
};

class AosKeepAliveTest : public ::testing::Test
{
public:
    TestAosKeepAlive* m_pAosKeepAlive;
    MockISipKeepAliveHelper m_objMockSipKeepAliveHelper;
    MockIAosKeepAliveListener m_objMockIAosKeepAliveListener;

protected:
    virtual void SetUp() override
    {
        m_pAosKeepAlive = new TestAosKeepAlive(SLOT_ID);
        ASSERT_TRUE(m_pAosKeepAlive != nullptr);

        m_pAosKeepAlive->SetKeepAliveHelper(&m_objMockSipKeepAliveHelper);
    }

    virtual void TearDown() override
    {
        if (m_pAosKeepAlive)
        {
            delete m_pAosKeepAlive;
        }
    }
};

TEST_F(AosKeepAliveTest, SucceedsSetListener)
{
    // GIVEN
    EXPECT_EQ(m_pAosKeepAlive->GetListener(), nullptr);

    // WHEN
    m_pAosKeepAlive->SetListener(&m_objMockIAosKeepAliveListener);

    // THEN
    EXPECT_NE(m_pAosKeepAlive->GetListener(), nullptr);
}

TEST_F(AosKeepAliveTest, SucceedsSetCheckingPongTrueWhenStartWithTrue)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_FALSE);

    // WHEN
    m_pAosKeepAlive->Start(TestAosKeepAlive::GRATER_TIME_THAN_POING_WAIT, IMS_TRUE);

    // THEN
    EXPECT_TRUE(m_pAosKeepAlive->IsPongChecked());
}

TEST_F(AosKeepAliveTest, SucceedsSetCheckingPongFalseWhenStartWithFalse)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_TRUE);

    // WHEN
    m_pAosKeepAlive->Start(TestAosKeepAlive::GRATER_TIME_THAN_POING_WAIT, IMS_FALSE);

    // THEN
    EXPECT_FALSE(m_pAosKeepAlive->IsPongChecked());
}

TEST_F(AosKeepAliveTest, FailsSetCheckingPongTrueWhenStartWithShortRepeatTime)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_FALSE);

    // WHEN
    m_pAosKeepAlive->Start(TestAosKeepAlive::SMALLER_TIME_THAN_POING_WAIT, IMS_TRUE);

    // THEN
    EXPECT_FALSE(m_pAosKeepAlive->IsPongChecked());
}

TEST_F(AosKeepAliveTest, SucceedsClearKeepAliveTimeWhenStop)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetListener(_));

    m_pAosKeepAlive->SetKeepAliveTime(2000);

    // WHEN
    m_pAosKeepAlive->Stop();

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->GetKeepAliveTime(), 0);
}

TEST_F(AosKeepAliveTest, SucceedsSetTransport)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetTransportTupleS(_, _, _));
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetTransportTupleD(_, _));

    // WHEN
    m_pAosKeepAlive->SetTransport(IpAddress("127.0.0.1"), 1234, IpAddress("127.0.0.2"), 5678, 1);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, SucceedsSendPing)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_));

    // WHEN
    m_pAosKeepAlive->SendPing();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, StartPongWaitTimerWhenPoingChecked)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_TRUE);
    m_pAosKeepAlive->ClearTimer();
    m_pAosKeepAlive->SetKeepAliveTime(2000);

    // WHEN
    m_pAosKeepAlive->ProcessKeepAliveTimerExpired();

    // THEN
    EXPECT_TRUE(m_pAosKeepAlive->GetKeepAliveTimer());
    EXPECT_TRUE(m_pAosKeepAlive->GetPongWaitTimer());
}

TEST_F(AosKeepAliveTest, DoesNotStartPongWaitTimerWhenPoingNotChecked)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_FALSE);
    m_pAosKeepAlive->ClearTimer();
    m_pAosKeepAlive->SetKeepAliveTime(2000);

    // WHEN
    m_pAosKeepAlive->ProcessKeepAliveTimerExpired();

    // THEN
    EXPECT_TRUE(m_pAosKeepAlive->GetKeepAliveTimer());
    EXPECT_FALSE(m_pAosKeepAlive->GetPongWaitTimer());
}

TEST_F(AosKeepAliveTest, InvokesDetectedFlowFailed)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed());

    // WHEN
    m_pAosKeepAlive->ProcessPongWaitTimerExpired();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, DoesNotInvokesDetectedFlowFailedWhenListenerIsNull)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(IMS_NULL);

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(0);

    // WHEN
    m_pAosKeepAlive->ProcessPongWaitTimerExpired();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, StartKeepAliveTimer)
{
    // GIVEN

    // WHEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE, 1000);

    // THEN
    EXPECT_NE(m_pAosKeepAlive->GetKeepAliveTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, StartPongWaitTimer)
{
    // GIVEN

    // WHEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // THEN
    EXPECT_NE(m_pAosKeepAlive->GetPongWaitTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, ReturnsFalseWhenDurationIsZero)
{
    // GIVEN

    // WHEN
    IMS_BOOL bResult = m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE, 0);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosKeepAliveTest, StartTimerAfterStopWhenSameTimerIsRunning)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE, 1000);

    // WHEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE, 1000);

    // THEN
    EXPECT_NE(m_pAosKeepAlive->GetKeepAliveTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, ReturnsFalseWhenStartTimerWithInvalidTimer)
{
    // GIVEN
    IMS_UINT32 TIMER_INVALID_TYPE = 999;

    // WHEN
    IMS_BOOL bResult = m_pAosKeepAlive->StartTimer(TIMER_INVALID_TYPE, 1000);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosKeepAliveTest, StopKeepAliveTimer)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE, 1000);

    // WHEN
    m_pAosKeepAlive->StopTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE);

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->GetKeepAliveTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, StopPongWaitTimer)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // WHEN
    m_pAosKeepAlive->StopTimer(TestAosKeepAlive::TIMER_PONG_WAIT);

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->GetPongWaitTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, ReturnsFalseWhenStopTimerWithoutTimer)
{
    // GIVEN

    // WHEN
    IMS_BOOL bResult = m_pAosKeepAlive->StopTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosKeepAliveTest, ReturnsFalseWhenStopTimerWithInvalidTimer)
{
    // GIVEN
    IMS_UINT32 TIMER_INVALID_TYPE = 999;

    // WHEN
    IMS_BOOL bResult = m_pAosKeepAlive->StopTimer(TIMER_INVALID_TYPE);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosKeepAliveTest, ClearTimerWhenTimersAreSet)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_KEEP_ALIVE, 1000);
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // WHEN
    m_pAosKeepAlive->ClearTimer();

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->GetKeepAliveTimer(), nullptr);
    EXPECT_EQ(m_pAosKeepAlive->GetPongWaitTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, StopPongWaitTimerWhenPongReceived)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // WHEN
    m_pAosKeepAlive->KeepAliveHelper_PongReceived();

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->GetPongWaitTimer(), nullptr);
}

TEST_F(AosKeepAliveTest, InvokesSendPacketWhenKeepaliveTimerExpired)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(2);

    m_pAosKeepAlive->Start(TestAosKeepAlive::GRATER_TIME_THAN_POING_WAIT, IMS_TRUE);

    // WHEN
    m_pAosKeepAlive->Timer_TimerExpired(m_pAosKeepAlive->GetKeepAliveTimer());

    // THEN : GIVEN conditions should be met.
    //     If SendPacket(_) is not invoked as a result of calling Timer_TimerExpired,
    //     it occurs only once.
}

TEST_F(AosKeepAliveTest, InvokesDetectedFlowFailedWhenPongWaitTimerExpired)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed());

    m_pAosKeepAlive->Start(TestAosKeepAlive::GRATER_TIME_THAN_POING_WAIT, IMS_TRUE);

    // WHEN
    m_pAosKeepAlive->Timer_TimerExpired(m_pAosKeepAlive->GetPongWaitTimer());

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, DoesNotInvokesAnyTimerExpiredProcessWhenTimerIsNull)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(0);

    m_pAosKeepAlive->Start(TestAosKeepAlive::GRATER_TIME_THAN_POING_WAIT, IMS_TRUE);

    // WHEN
    m_pAosKeepAlive->Timer_TimerExpired(IMS_NULL);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, ReturnsTimerString)
{
    EXPECT_STREQ(
            m_pAosKeepAlive->TimerToString(TestAosKeepAlive::TIMER_KEEP_ALIVE), "TIMER_KEEP_ALIVE");
    EXPECT_STREQ(
            m_pAosKeepAlive->TimerToString(TestAosKeepAlive::TIMER_PONG_WAIT), "TIMER_PONG_WAIT");
    EXPECT_STREQ(m_pAosKeepAlive->TimerToString(-1), "__INVALID__");
}
