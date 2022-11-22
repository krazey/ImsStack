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

#include "INetworkWatcher.h"

#include "provider/AosTransaction.h"

#include "MockIImsRadio.h"
#include "interface/MockIAosTransaction.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class TestAosTransaction : public AosTransaction
{
    inline explicit TestAosTransaction(IN IMS_SINT32 nSlotId) :
            AosTransaction(nSlotId),
            piOriginRadio(IMS_NULL)
    {
    }

    friend class AosTransactionTest;
    FRIEND_TEST(AosTransactionTest, StartAndStopTraffic);

public:
    inline void SetMockIImsRadio(IN IImsRadio* piImsRadio)
    {
        piOriginRadio = m_piImsRadio;
        m_piImsRadio = piImsRadio;
    }

    inline void SetOriginRadio() { m_piImsRadio = piOriginRadio; }

    inline void InvokeTimerExpired()
    {
        if (m_piStopTimer != IMS_NULL)
        {
            Timer_TimerExpired(m_piStopTimer);
        }
    }

private:
    IImsRadio* piOriginRadio;
};

class AosTransactionTest : public ::testing::Test
{
public:
    TestAosTransaction* m_pTestAosTransaction;

    MockIImsRadio m_objMockIImsRadio;

protected:
    virtual void SetUp() override
    {
        m_pTestAosTransaction = new TestAosTransaction(SLOT_ID);

        m_pTestAosTransaction->SetMockIImsRadio(static_cast<IImsRadio*>(&m_objMockIImsRadio));

        EXPECT_CALL(m_objMockIImsRadio, IsImsTrafficAllowed(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIImsRadio, StartImsTraffic(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIImsRadio, StopImsTraffic(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIImsRadio, AddListenerForTrafficPriority(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIImsRadio, RemoveListenerForTrafficPriority(_)).Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        if (m_pTestAosTransaction)
        {
            m_pTestAosTransaction->SetOriginRadio();
            delete m_pTestAosTransaction;
            m_pTestAosTransaction = IMS_NULL;
        }
    }
};

TEST_F(AosTransactionTest, StartAndStopTraffic)
{
    /* TEST_F : IsStarted(_), Start, IsStartUpdated, */
    EXPECT_FALSE(
            m_pTestAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));

    EXPECT_TRUE(m_pTestAosTransaction->IsStartUpdated());
    EXPECT_TRUE(m_pTestAosTransaction->IsTrafficResponseWaiting());

    /*
        TEST_F : IsStarted(_), Stop, RemoveForWaitingResponse, IsStartUpdated, IsStarted,
                StartTimer
    */
    m_pTestAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);
    EXPECT_FALSE(m_pTestAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_TRUE(m_pTestAosTransaction->IsTimerRunning());

    /* TEST_F : StopTimer, ProcessTimerExpired */
    m_pTestAosTransaction->InvokeTimerExpired();
    EXPECT_FALSE(m_pTestAosTransaction->IsTimerRunning());
    EXPECT_FALSE(m_pTestAosTransaction->IsStartUpdated());
}
