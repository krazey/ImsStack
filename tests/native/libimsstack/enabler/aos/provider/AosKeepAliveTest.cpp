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

class TestAosKeepAlive : public AosKeepAlive
{
public:
    inline explicit TestAosKeepAlive(IN IMS_SINT32 nSlotId) :
            AosKeepAlive(nSlotId)
    {
    }

    inline void SetKeepAliveHelper(IN ISipKeepAliveHelper* piKeepAliveHelper)
    {
        m_piKeepAliveHelper = piKeepAliveHelper;
    }

    // TEST : SetListener
    FRIEND_TEST(AosKeepAliveTest, SucceedsSetListener);
    // TEST : Start
    FRIEND_TEST(AosKeepAliveTest, SetCheckingPongTrue);
    FRIEND_TEST(AosKeepAliveTest, SetCheckingPongFalse);
    FRIEND_TEST(AosKeepAliveTest, SetCheckingPongFalseWhenShortRepeatTime);
    FRIEND_TEST(AosKeepAliveTest, StartWithKeepAliveHelper);
    FRIEND_TEST(AosKeepAliveTest, StartWithoutKeepAliveHelper);
    // TEST : Stop
    FRIEND_TEST(AosKeepAliveTest, SucceedsClearKeepAliveTime);
    // TEST : SetTransport
    FRIEND_TEST(AosKeepAliveTest, SucceedsSetTransport);
    // TEST : SendPing
    FRIEND_TEST(AosKeepAliveTest, SucceedsSendPing);
    // TEST : ProcessKeepAliveTimerExpired
    FRIEND_TEST(AosKeepAliveTest, StartPongWaitTimerWhenPoingChecked);
    FRIEND_TEST(AosKeepAliveTest, DoesNotStartPongWaitTimerWhenPoingNotChecked);
    // TEST : ProcessPongWaitTimerExpired
    FRIEND_TEST(AosKeepAliveTest, InvokesDetectedFlowFailed);
    FRIEND_TEST(AosKeepAliveTest, DoesNotInvokesDetectedFlowFailedWhenListenerIsNull);
    // TEST : StartTimer
    FRIEND_TEST(AosKeepAliveTest, StartKeepAliveTimer);
    FRIEND_TEST(AosKeepAliveTest, StartPongWaitTimer);
    FRIEND_TEST(AosKeepAliveTest, ReturnsFalseWhenDurationIsZero);
    FRIEND_TEST(AosKeepAliveTest, StartTimerAfterStopWhenSameTimerIsRunning);
    FRIEND_TEST(AosKeepAliveTest, ReturnsFalseWhenStartTimerWithInvalidTimer);
    // TEST : StopTimer
    FRIEND_TEST(AosKeepAliveTest, StopKeepAliveTimer);
    FRIEND_TEST(AosKeepAliveTest, StopPongWaitTimer);
    FRIEND_TEST(AosKeepAliveTest, ReturnsFalseWhenStopTimerWithoutTimer);
    FRIEND_TEST(AosKeepAliveTest, ReturnsFalseWhenStopTimerWithInvalidTimer);
    // TEST : ClearTimer
    FRIEND_TEST(AosKeepAliveTest, ClearTimerWhenTimersAreSet);
    // TEST : KeepAliveHelper_PongReceived
    FRIEND_TEST(AosKeepAliveTest, StopPongWaitTimerWhenPongReceived);
    // TEST : Timer_TimerExpired
    FRIEND_TEST(AosKeepAliveTest, InvokesSendPacketWhenKeepaliveTimerExpired);
    FRIEND_TEST(AosKeepAliveTest, InvokesDetectedFlowFailedWhenPongWaitTimerExpired);
    FRIEND_TEST(AosKeepAliveTest, DoesNotInvokesAnyTimerExpiredProcessWhenTimerIsNull);
    // TEST : TimerToString
    FRIEND_TEST(AosKeepAliveTest, ReturnsTimerString);
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
    EXPECT_EQ(m_pAosKeepAlive->m_piListener, nullptr);

    // WHEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    // THEN
    EXPECT_NE(m_pAosKeepAlive->m_piListener, nullptr);
}

TEST_F(AosKeepAliveTest, SetCheckingPongTrue)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_FALSE);
    EXPECT_FALSE(m_pAosKeepAlive->IsPongChecked());

    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, Destroy()).Times(1);

    // WHEN
    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    // THEN
    EXPECT_TRUE(m_pAosKeepAlive->IsPongChecked());
}

TEST_F(AosKeepAliveTest, SetCheckingPongFalse)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_TRUE);
    EXPECT_TRUE(m_pAosKeepAlive->IsPongChecked());

    // WHEN
    m_pAosKeepAlive->Start(20000, IMS_FALSE);

    // THEN
    EXPECT_FALSE(m_pAosKeepAlive->IsPongChecked());
}

TEST_F(AosKeepAliveTest, SetCheckingPongFalseWhenShortRepeatTime)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_FALSE);
    EXPECT_FALSE(m_pAosKeepAlive->IsPongChecked());

    // WHEN
    m_pAosKeepAlive->Start(5000, IMS_TRUE);

    // THEN
    EXPECT_FALSE(m_pAosKeepAlive->IsPongChecked());
}

TEST_F(AosKeepAliveTest, SucceedsClearKeepAliveTime)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetListener(_)).Times(1);

    m_pAosKeepAlive->m_nKeepAliveTime = 2000;

    // WHEN
    m_pAosKeepAlive->Stop();

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->m_nKeepAliveTime, 0);
}

TEST_F(AosKeepAliveTest, SucceedsSetTransport)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetTransportTupleS(_, _, _)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetTransportTupleD(_, _)).Times(1);

    // WHEN
    m_pAosKeepAlive->SetTransport(IpAddress("127.0.0.1"), 1234, IpAddress("127.0.0.2"), 5678, 1);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, SucceedsSendPing)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(1);

    // WHEN
    m_pAosKeepAlive->SendPing();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, StartPongWaitTimerWhenPoingChecked)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_TRUE);
    m_pAosKeepAlive->ClearTimer();
    m_pAosKeepAlive->m_nKeepAliveTime = 2000;

    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, Destroy()).Times(1);

    // WHEN
    m_pAosKeepAlive->ProcessKeepAliveTimerExpired();

    // THEN
    EXPECT_TRUE(m_pAosKeepAlive->m_piKeepAliveTimer);
    EXPECT_TRUE(m_pAosKeepAlive->m_piPongWaitTimer);
}

TEST_F(AosKeepAliveTest, DoesNotStartPongWaitTimerWhenPoingNotChecked)
{
    // GIVEN
    m_pAosKeepAlive->SetCheckingPong(IMS_FALSE);
    m_pAosKeepAlive->ClearTimer();
    m_pAosKeepAlive->m_nKeepAliveTime = 2000;

    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, Destroy()).Times(1);

    // WHEN
    m_pAosKeepAlive->ProcessKeepAliveTimerExpired();

    // THEN
    EXPECT_TRUE(m_pAosKeepAlive->m_piKeepAliveTimer);
    EXPECT_FALSE(m_pAosKeepAlive->m_piPongWaitTimer);
}

TEST_F(AosKeepAliveTest, InvokesDetectedFlowFailed)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(1);

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
    EXPECT_NE(m_pAosKeepAlive->m_piKeepAliveTimer, nullptr);
}

TEST_F(AosKeepAliveTest, StartPongWaitTimer)
{
    // GIVEN

    // WHEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // THEN
    EXPECT_NE(m_pAosKeepAlive->m_piPongWaitTimer, nullptr);
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
    EXPECT_NE(m_pAosKeepAlive->m_piKeepAliveTimer, nullptr);
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
    EXPECT_EQ(m_pAosKeepAlive->m_piKeepAliveTimer, nullptr);
}

TEST_F(AosKeepAliveTest, StopPongWaitTimer)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // WHEN
    m_pAosKeepAlive->StopTimer(TestAosKeepAlive::TIMER_PONG_WAIT);

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->m_piPongWaitTimer, nullptr);
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
    EXPECT_EQ(m_pAosKeepAlive->m_piKeepAliveTimer, nullptr);
    EXPECT_EQ(m_pAosKeepAlive->m_piPongWaitTimer, nullptr);
}

TEST_F(AosKeepAliveTest, StopPongWaitTimerWhenPongReceived)
{
    // GIVEN
    m_pAosKeepAlive->StartTimer(TestAosKeepAlive::TIMER_PONG_WAIT, 1000);

    // WHEN
    m_pAosKeepAlive->KeepAliveHelper_PongReceived();

    // THEN
    EXPECT_EQ(m_pAosKeepAlive->m_piPongWaitTimer, nullptr);
}

TEST_F(AosKeepAliveTest, InvokesSendPacketWhenKeepaliveTimerExpired)
{
    // GIVEN
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(2);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, Destroy()).Times(1);

    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    // WHEN
    m_pAosKeepAlive->Timer_TimerExpired(m_pAosKeepAlive->m_piKeepAliveTimer);

    // THEN : GIVEN conditions should be met.
    //     If SendPacket(_) is not invoked as a result of calling Timer_TimerExpired,
    //     it occurs only once.
}

TEST_F(AosKeepAliveTest, InvokesDetectedFlowFailedWhenPongWaitTimerExpired)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(1);

    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, Destroy()).Times(1);

    m_pAosKeepAlive->Start(20000, IMS_TRUE);

    // WHEN
    m_pAosKeepAlive->Timer_TimerExpired(m_pAosKeepAlive->m_piPongWaitTimer);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosKeepAliveTest, DoesNotInvokesAnyTimerExpiredProcessWhenTimerIsNull)
{
    // GIVEN
    m_pAosKeepAlive->SetListener(
            static_cast<IAosKeepAliveListener*>(&m_objMockIAosKeepAliveListener));

    EXPECT_CALL(m_objMockIAosKeepAliveListener, KeepAlive_DetectedFlowFailed()).Times(0);

    EXPECT_CALL(m_objMockSipKeepAliveHelper, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, SendPacket(_)).Times(1);
    EXPECT_CALL(m_objMockSipKeepAliveHelper, Destroy()).Times(1);

    m_pAosKeepAlive->Start(20000, IMS_TRUE);

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
