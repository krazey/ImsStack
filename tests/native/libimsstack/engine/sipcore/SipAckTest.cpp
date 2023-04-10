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
#include <gtest/gtest.h>

#include "SipAck.h"

#include "PlatformContext.h"
#include "TestTimerService.h"

using ::testing::_;
using ::testing::Invoke;

class TestSipClientTransactionState : public SipClientTransactionState
{
public:
    inline TestSipClientTransactionState() :
            SipClientTransactionState(IMS_SLOT_0),
            m_bRetransmitted(IMS_FALSE)
    {
    }

    IMS_RESULT RetransmitMessage() override
    {
        m_bRetransmitted = IMS_TRUE;
        return IMS_SUCCESS;
    }

    IMS_BOOL IsMessageRetransmitted() { return m_bRetransmitted; }

private:
    IMS_BOOL m_bRetransmitted;
};

namespace android
{

class SipAckTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pTimerService = new TestTimerService();
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, m_pTimerService);

        m_pTestSipClientTransactionState = new TestSipClientTransactionState();
        m_pTestSipClientTransactionState->AddReference();
    }
    virtual void TearDown() override
    {
        delete m_pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);

        delete m_pTestSipClientTransactionState;
    }

public:
    TestTimerService* m_pTimerService;
    TestSipClientTransactionState* m_pTestSipClientTransactionState;
};

TEST_F(SipAckTest, Constructor)
{
    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(1);
    EXPECT_CALL(objMockTimer, KillTimer()).Times(1);

    // Alive interval is zero, timer should not start
    SipAck objSipAck(IMS_NULL, 0);

    // Alive interval is non zero, timer should start
    SipAck objSipAck1(m_pTestSipClientTransactionState, 10);
}

TEST_F(SipAckTest, IsSameTransaction)
{
    SipAck objSipAck(IMS_NULL, 10);

    // No client transaction object set, returns false
    EXPECT_FALSE(objSipAck.IsSameTransaction(IMS_NULL));

    SipAck objSipAck1(m_pTestSipClientTransactionState, 10);

    // client transaction object set, txn keys are null and returns false
    EXPECT_FALSE(objSipAck1.IsSameTransaction(IMS_NULL));
}

TEST_F(SipAckTest, RetransmitMessage)
{
    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(2);
    EXPECT_CALL(objMockTimer, KillTimer()).Times(2);

    SipAck objSipAck(IMS_NULL, 10);

    // No client transaction object set, no retransmission
    objSipAck.RetransmitMessage();

    EXPECT_FALSE(m_pTestSipClientTransactionState->IsMessageRetransmitted());

    SipAck objSipAck1(m_pTestSipClientTransactionState, 10);

    // client transaction object set, message retransmission success
    objSipAck1.RetransmitMessage();

    EXPECT_TRUE(m_pTestSipClientTransactionState->IsMessageRetransmitted());
}

TEST_F(SipAckTest, Timer_TimerExpired)
{
    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();

    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(1);
    EXPECT_CALL(objMockTimer, KillTimer()).Times(1);

    ITimerListener* pAckTimerListener = IMS_NULL;
    IMS_UINT32 nTimerDuration = 0;

    ON_CALL(objMockTimer, SetTimer)
            .WillByDefault(Invoke(
                    [&](IMS_UINT32 nDuration, ITimerListener* pTimerListener)
                    {
                        nTimerDuration = nDuration;
                        pAckTimerListener = pTimerListener;
                        return 1;
                    }));

    // Alive interval is non zero, timer should start
    SipAck objSipAck(m_pTestSipClientTransactionState, 10);

    EXPECT_FALSE(objSipAck.IsStrayAck());

    ASSERT_NE(pAckTimerListener, nullptr);

    MockITimer objDiffMockTimer;

    // different time object, simply return
    pAckTimerListener->Timer_TimerExpired(&objDiffMockTimer);

    EXPECT_FALSE(objSipAck.IsStrayAck());

    // Valid timer object passed, success
    pAckTimerListener->Timer_TimerExpired(&objMockTimer);

    EXPECT_TRUE(objSipAck.IsStrayAck());
}

}  // namespace android