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
#include "PlatformContext.h"

#include "provider/AosTransaction.h"

#include "MockIImsRadio.h"
#include "MockIImsTraffic.h"
#include "interface/MockIAosTransactionListener.h"

#include "../../../platform/interface/TestImsRadioService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

const IMS_SINT32 SLOT_ID = 0;

class TestAosTransaction : public AosTransaction
{
public:
    inline explicit TestAosTransaction(IN IMS_SINT32 nSlotId) :
            AosTransaction(nSlotId),
            piOriginRadio(IMS_NULL)
    {
    }

    FRIEND_TEST(AosTransactionTest, SetListener_NotAddedToTheListIfListenerIsNull);
    FRIEND_TEST(AosTransactionTest, SetListener_AddToTheMapIfDifferentTypeOfListener);
    FRIEND_TEST(AosTransactionTest, SetListener_NotAddedToTheListIfSameListener);
    FRIEND_TEST(AosTransactionTest, SetListener_AddToTheListIfDifferentListener);
    FRIEND_TEST(AosTransactionTest, IsTransactionAllowed_ReturnFalseIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, IsTransactionAllowed_CallIsImsTrafficAllowed);
    FRIEND_TEST(AosTransactionTest, StartTraffic_ReturnTrueIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, StartTraffic_ReturnTrueIfAlreadyStarted);
    FRIEND_TEST(AosTransactionTest, StartTraffic_StopsTimerIfStartUpdatedAndStopTimerIsRunning);
    FRIEND_TEST(AosTransactionTest,
            StartTraffic_AddsTypeToWaitingListIfStartUpdatedAndTrafficResponseWaiting);
    FRIEND_TEST(AosTransactionTest, StartTraffic_StartImsTrafficIfStartNotUpdated);
    FRIEND_TEST(AosTransactionTest, StartEmergencyTraffic_DoNothingIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, StartEmergencyTraffic_StartImsTrafficCalledOnlyOneTime);
    FRIEND_TEST(AosTransactionTest, StopTraffic_DoNothingIfNotStarted);
    FRIEND_TEST(AosTransactionTest, StopTraffic_StopForTheTypeAndStartTimerIfStarted);
    FRIEND_TEST(AosTransactionTest, StopEmergencyTraffic_DoNothingIfNotStarted);
    FRIEND_TEST(AosTransactionTest, StopEmergencyTraffic_StopIfStarted);
    FRIEND_TEST(AosTransactionTest, SetWlan_DoNothingIfImsTrafficIsNull);
    FRIEND_TEST(AosTransactionTest, SetWlan_CallSetWlan);

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
    TestAosTransaction* m_pAosTransaction;

    MockIImsRadio m_objMockIImsRadio;

protected:
    virtual void SetUp() override
    {
        m_pAosTransaction = new TestAosTransaction(SLOT_ID);

        m_pAosTransaction->SetMockIImsRadio(static_cast<IImsRadio*>(&m_objMockIImsRadio));

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
        if (m_pAosTransaction)
        {
            m_pAosTransaction->SetOriginRadio();
            delete m_pAosTransaction;
            m_pAosTransaction = IMS_NULL;
        }
    }
};

TEST_F(AosTransactionTest, SetListener_NotAddedToTheListIfListenerIsNull)
{
    IMS_SINT32 nCountBefore = m_pAosTransaction->GetListeners().GetSize();

    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, nullptr);

    IMS_SINT32 nCountAfter = m_pAosTransaction->GetListeners().GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore);
}

TEST_F(AosTransactionTest, SetListener_AddToTheMapIfDifferentTypeOfListener)
{
    MockIAosTransactionListener objMockIAosTransactionListenerForReg;
    MockIAosTransactionListener objMockIAosTransactionListenerForSub;
    MockIAosTransactionListener objMockIAosTransactionListenerForEmergency;
    IMS_SINT32 nCountBefore = m_pAosTransaction->GetListeners().GetSize();

    m_pAosTransaction->SetListener(
            IAosTransaction::TYPE_REG, &objMockIAosTransactionListenerForReg);
    m_pAosTransaction->SetListener(
            IAosTransaction::TYPE_SUB, &objMockIAosTransactionListenerForSub);
    m_pAosTransaction->SetListener(
            IAosTransaction::TYPE_EMERGENCY, &objMockIAosTransactionListenerForEmergency);

    IMS_SINT32 nCountAfter = m_pAosTransaction->GetListeners().GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore + 3);
}

TEST_F(AosTransactionTest, SetListener_NotAddedToTheListIfSameListener)
{
    MockIAosTransactionListener objMockIAosTransactionListener;
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);
    IMS_SINT32 nCountBefore =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();

    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);

    IMS_SINT32 nCountAfter =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore);
}

TEST_F(AosTransactionTest, SetListener_AddToTheListIfDifferentListener)
{
    MockIAosTransactionListener objMockIAosTransactionListener;
    MockIAosTransactionListener objMockIAosTransactionListener2;
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);
    IMS_SINT32 nCountBefore =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();

    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener2);

    IMS_SINT32 nCountAfter =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore + 1);
}

TEST_F(AosTransactionTest, IsTransactionAllowed_ReturnFalseIfImsRadioIsNull)
{
    m_pAosTransaction->m_piImsRadio = nullptr;
    EXPECT_FALSE(m_pAosTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG));
}

TEST_F(AosTransactionTest, IsTransactionAllowed_CallIsImsTrafficAllowed)
{
    EXPECT_CALL(m_objMockIImsRadio, IsImsTrafficAllowed(IImsRadio::TRAFFIC_TYPE_REGISTRATION));
    m_pAosTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG);
    EXPECT_CALL(m_objMockIImsRadio, IsImsTrafficAllowed(IImsRadio::TRAFFIC_TYPE_EMERGENCY));
    m_pAosTransaction->IsTransactionAllowed(IAosTransaction::TYPE_EMERGENCY);
}

TEST_F(AosTransactionTest, StartTraffic_ReturnTrueIfImsRadioIsNull)
{
    m_pAosTransaction->m_piImsRadio = nullptr;
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));
}

TEST_F(AosTransactionTest, StartTraffic_ReturnTrueIfAlreadyStarted)
{
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));

    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_NR));
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_EHRPD));
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_WLAN));
}

TEST_F(AosTransactionTest, StartTraffic_StopsTimerIfStartUpdatedAndStopTimerIsRunning)
{
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);

    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());
    EXPECT_TRUE(m_pAosTransaction->IsTimerRunning());

    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(m_pAosTransaction->IsTimerRunning());
}

TEST_F(AosTransactionTest,
        StartTraffic_AddsTypeToWaitingListIfStartUpdatedAndTrafficResponseWaiting)
{
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);

    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());
    EXPECT_TRUE(m_pAosTransaction->IsTrafficResponseWaiting());
    EXPECT_FALSE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_REG));

    EXPECT_FALSE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_SUB));
}

TEST_F(AosTransactionTest, StartTraffic_StartImsTrafficIfStartNotUpdated)
{
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION, _, IImsRadio::DIRECTION_MO, _));
    EXPECT_FALSE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));
}

TEST_F(AosTransactionTest, StartEmergencyTraffic_DoNothingIfImsRadioIsNull)
{
    m_pAosTransaction->m_piImsRadio = nullptr;
    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, _, IImsRadio::DIRECTION_MO, _))
            .Times(0);
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
}

TEST_F(AosTransactionTest, StartEmergencyTraffic_StartImsTrafficCalledOnlyOneTime)
{
    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
}

TEST_F(AosTransactionTest, StopTraffic_DoNothingIfNotStarted)
{
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    m_pAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);
}

TEST_F(AosTransactionTest, StopTraffic_StopForTheTypeAndStartTimerIfStarted)
{
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);

    EXPECT_TRUE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_TRUE(m_pAosTransaction->IsTimerRunning());

    m_pAosTransaction->InvokeTimerExpired();
    EXPECT_FALSE(m_pAosTransaction->IsTimerRunning());
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());
}

TEST_F(AosTransactionTest, StopEmergencyTraffic_DoNothingIfNotStarted)
{
    EXPECT_CALL(m_objMockIImsRadio, StopImsTraffic(_)).Times(0);
    m_pAosTransaction->StopEmergencyTraffic();
}

TEST_F(AosTransactionTest, StopEmergencyTraffic_StopIfStarted)
{
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
    EXPECT_CALL(m_objMockIImsRadio, StopImsTraffic(_)).Times(1);
    m_pAosTransaction->StopEmergencyTraffic();
}

TEST_F(AosTransactionTest, SetWlan_DoNothingIfImsTrafficIsNull)
{
    TestImsRadioService objTestImsRadioService;
    PlatformService* pOrigService = PlatformContext::GetInstance()->SetService(
            PlatformContext::SERVICE_RADIO, &objTestImsRadioService);

    objTestImsRadioService.SetImsTraffic(IMS_NULL);
    EXPECT_CALL(objTestImsRadioService.GetMockImsTraffic(), SetWlan(_, _)).Times(0);
    m_pAosTransaction->SetWlan(IMS_TRUE);

    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, pOrigService);
}

TEST_F(AosTransactionTest, SetWlan_CallSetWlan)
{
    TestImsRadioService objTestImsRadioService;
    PlatformService* pOrigService = PlatformContext::GetInstance()->SetService(
            PlatformContext::SERVICE_RADIO, &objTestImsRadioService);

    EXPECT_CALL(objTestImsRadioService.GetMockImsTraffic(), SetWlan(_, _)).Times(2);
    m_pAosTransaction->SetWlan(IMS_TRUE);
    m_pAosTransaction->SetWlan(IMS_FALSE);

    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, pOrigService);
}
