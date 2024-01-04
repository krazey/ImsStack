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
#include "MockITimer.h"
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
    FRIEND_TEST(AosTransactionTest, DoNothingIfListenerIsNullWhenRemoveListener);
    FRIEND_TEST(AosTransactionTest, DoNothingIfNoListenerForTheTypeWhenRemoveListener);
    FRIEND_TEST(AosTransactionTest, IsTransactionAllowed_ReturnFalseIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, IsTransactionAllowed_CallIsImsTrafficAllowed);
    FRIEND_TEST(AosTransactionTest, StartTraffic_ReturnTrueIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, StartTraffic_ReturnTrueIfAlreadyStarted);
    FRIEND_TEST(AosTransactionTest, StartTraffic_StopsTimerIfStartUpdatedAndStopTimerIsRunning);
    FRIEND_TEST(AosTransactionTest,
            StartTraffic_AddsTypeToWaitingListIfStartUpdatedAndTrafficResponseWaiting);
    FRIEND_TEST(AosTransactionTest,
            DoNothingIfStartUpdatedButTrafficResponseWaitingWhenTrafficIsStarted);
    FRIEND_TEST(
            AosTransactionTest, ShouldNotStartImsTrafficIfNoTrafficInstanceWhenTrafficIsStarted);
    FRIEND_TEST(AosTransactionTest, StartTraffic_StartImsTrafficIfStartNotUpdated);
    FRIEND_TEST(AosTransactionTest, StartEmergencyTraffic_DoNothingIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, DoNothingIfNoTrafficInstanceWhenEmergencyTrafficIsStarted);
    FRIEND_TEST(AosTransactionTest, StartEmergencyTraffic_StartImsTrafficCalledOnlyOneTime);
    FRIEND_TEST(AosTransactionTest, StopTraffic_DoNothingIfNotStarted);
    FRIEND_TEST(AosTransactionTest, StopTraffic_StopForTheTypeAndStartTimerIfStarted);
    FRIEND_TEST(AosTransactionTest, StopEmergencyTraffic_DoNothingIfNotStarted);
    FRIEND_TEST(AosTransactionTest, StopEmergencyTraffic_StopIfStarted);
    FRIEND_TEST(AosTransactionTest, SetWlan_DoNothingIfImsTrafficIsNull);
    FRIEND_TEST(AosTransactionTest, SetWlan_CallSetWlan);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnNgranIfNr);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnEutranIfLte);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnUtranIfEhrpd);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnUtranIfWcdma);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnUtranIfHspa);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnIwlanIfWlan);
    FRIEND_TEST(AosTransactionTest, GetAccessNetworkType_ReturnUnknownIfNotConsideringRat);
    FRIEND_TEST(AosTransactionTest, ShouldNotNotifyIfNoListenerWhenTrafficConnectionIsFailed);
    FRIEND_TEST(AosTransactionTest, ShouldNotifyToTheListenersWhenTrafficConnectionIsFailed);
    FRIEND_TEST(AosTransactionTest,
            ShouldNotifyToTheListenerForTypeRegWhenTrafficConnectionIsFailedForTypeReg);
    FRIEND_TEST(AosTransactionTest,
            ShouldNotifyToTheListenerForTypeSubWhenTrafficConnectionIsFailedForTypeSub);
    FRIEND_TEST(
            AosTransactionTest, ShouldNotNotifyIfNoListenerWhenTrafficConnectionSetupIsPrepared);
    FRIEND_TEST(AosTransactionTest, ShouldNotifyToTheListenersWhenTrafficConnectionSetupIsPrepared);
    FRIEND_TEST(AosTransactionTest,
            ShouldNotifyToTheListenerForTypeRegWhenTrafficConnectionSetupIsPreparedForTypeReg);
    FRIEND_TEST(AosTransactionTest,
            ShouldNotifyToTheListenerForTypeSubWhenTrafficConnectionSetupIsPreparedForTypeSub);
    FRIEND_TEST(AosTransactionTest, ShouldNotNotifyIfNoListenerWhenTrafficPriorityChanged);
    FRIEND_TEST(AosTransactionTest, ShouldNotifyToOneListenerWhenTrafficPriorityChanged);
    FRIEND_TEST(AosTransactionTest, ShouldNotifyToTwoListenersWhenTrafficPriorityChanged);
    FRIEND_TEST(AosTransactionTest, DoNothingIfTheTimerIsNullWhenTimerIsExpired);
    FRIEND_TEST(AosTransactionTest, DoNothingIfInvalidTimerWhenTimerIsExpired);

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

    MockIAosTransactionListener m_objMockIAosTransactionListener;
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

TEST_F(AosTransactionTest, DoNothingIfListenerIsNullWhenRemoveListener)
{
    m_pAosTransaction->RemoveListener(IAosTransaction::TYPE_REG, nullptr);
}

TEST_F(AosTransactionTest, DoNothingIfNoListenerForTheTypeWhenRemoveListener)
{
    // GIVEN
    MockIAosTransactionListener objMockIAosTransactionListener;
    m_pAosTransaction->ClearListeners();

    // WHEN
    m_pAosTransaction->RemoveListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);

    // THEN: Do nothing
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

TEST_F(AosTransactionTest, DoNothingIfStartUpdatedButTrafficResponseWaitingWhenTrafficIsStarted)
{
    // GIVEN
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->Traffic_OnConnectionSetupPrepared(IAosTransaction::TYPE_REG);
    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());
    EXPECT_FALSE(m_pAosTransaction->IsTrafficResponseWaiting());

    // WHEN
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE));

    // THEN: Do nothing and return true in WHEN part
}

TEST_F(AosTransactionTest, ShouldNotStartImsTrafficIfNoTrafficInstanceWhenTrafficIsStarted)
{
    // GIVEN
    m_pAosTransaction->ClearTraffics();
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());

    EXPECT_CALL(m_objMockIImsRadio, StartImsTraffic(_, _, _, _)).Times(0);

    // WHEN
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));

    // THEN: StartImsTraffic should not be called as the GIVEN part and return true in WHEN part
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

TEST_F(AosTransactionTest, DoNothingIfNoTrafficInstanceWhenEmergencyTrafficIsStarted)
{
    // GIVEN
    m_pAosTransaction->ClearTraffics();

    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, _, IImsRadio::DIRECTION_MO, _))
            .Times(0);

    // WHEN
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);

    // THEN: The GIVEN condition should be met.
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

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnNgranIfNr)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_NR),
            IImsRadio::ACCESS_NETWORK_TYPE_NGRAN);
}

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnEutranIfLte)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_LTE),
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN);
}

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnUtranIfEhrpd)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_EHRPD),
            IImsRadio::ACCESS_NETWORK_TYPE_UTRAN);
}

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnUtranIfWcdma)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_WCDMA),
            IImsRadio::ACCESS_NETWORK_TYPE_UTRAN);
}

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnUtranIfHspa)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_HSPA),
            IImsRadio::ACCESS_NETWORK_TYPE_UTRAN);
}

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnIwlanIfWlan)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_WLAN),
            IImsRadio::ACCESS_NETWORK_TYPE_IWLAN);
}

TEST_F(AosTransactionTest, GetAccessNetworkType_ReturnUnknownIfNotConsideringRat)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_INVALID),
            IImsRadio::ACCESS_NETWORK_TYPE_UNKNOWN);
}

TEST_F(AosTransactionTest, ShouldNotNotifyIfNoListenerWhenTrafficConnectionIsFailed)
{
    // GIVEN
    m_pAosTransaction->ClearListeners();
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnConnectionFailed(_, _, _)).Times(0);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionFailed(
            IAosTransaction::TYPE_REG, IImsRadio::REASON_ACCESS_DENIED, 0, 1000);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest, ShouldNotifyToTheListenersWhenTrafficConnectionIsFailed)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener,
            Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 1000))
            .Times(1);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionFailed(
            IAosTransaction::TYPE_REG, IImsRadio::REASON_ACCESS_DENIED, 0, 1000);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest,
        ShouldNotifyToTheListenerForTypeRegWhenTrafficConnectionIsFailedForTypeReg)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_REG));
    EXPECT_CALL(m_objMockIAosTransactionListener,
            Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 1000))
            .Times(2);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionFailed(
            IAosTransaction::TYPE_REG, IImsRadio::REASON_ACCESS_DENIED, 0, 1000);

    // THEN
    EXPECT_FALSE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_REG));
}

TEST_F(AosTransactionTest,
        ShouldNotifyToTheListenerForTypeSubWhenTrafficConnectionIsFailedForTypeSub)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_SUB, &m_objMockIAosTransactionListener);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_SUB));
    EXPECT_CALL(m_objMockIAosTransactionListener,
            Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 1000))
            .Times(2);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionFailed(
            IAosTransaction::TYPE_SUB, IImsRadio::REASON_ACCESS_DENIED, 0, 1000);

    // THEN
    EXPECT_FALSE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_SUB));
}

TEST_F(AosTransactionTest, ShouldNotNotifyIfNoListenerWhenTrafficConnectionSetupIsPrepared)
{
    // GIVEN
    m_pAosTransaction->ClearListeners();
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnConnectionSetupPrepared()).Times(0);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionSetupPrepared(IAosTransaction::TYPE_REG);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest, ShouldNotifyToTheListenersWhenTrafficConnectionSetupIsPrepared)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnConnectionSetupPrepared()).Times(1);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionSetupPrepared(IAosTransaction::TYPE_REG);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest,
        ShouldNotifyToTheListenerForTypeRegWhenTrafficConnectionSetupIsPreparedForTypeReg)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_REG));
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnConnectionSetupPrepared()).Times(2);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionSetupPrepared(IAosTransaction::TYPE_REG);

    // THEN
    EXPECT_FALSE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_REG));
}

TEST_F(AosTransactionTest,
        ShouldNotifyToTheListenerForTypeSubWhenTrafficConnectionSetupIsPreparedForTypeSub)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_SUB, &m_objMockIAosTransactionListener);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_SUB));
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnConnectionSetupPrepared()).Times(2);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionSetupPrepared(IAosTransaction::TYPE_SUB);

    // THEN
    EXPECT_FALSE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_SUB));
}

TEST_F(AosTransactionTest, ShouldNotNotifyIfNoListenerWhenTrafficPriorityChanged)
{
    // GIVEN
    m_pAosTransaction->ClearListeners();
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnTrafficPriorityChanged()).Times(0);

    // WHEN
    m_pAosTransaction->ImsRadio_OnTrafficPriorityChanged();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest, ShouldNotifyToOneListenerWhenTrafficPriorityChanged)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnTrafficPriorityChanged()).Times(1);

    // WHEN
    m_pAosTransaction->ImsRadio_OnTrafficPriorityChanged();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest, ShouldNotifyToTwoListenersWhenTrafficPriorityChanged)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_SUB, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnTrafficPriorityChanged()).Times(2);

    // WHEN
    m_pAosTransaction->ImsRadio_OnTrafficPriorityChanged();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosTransactionTest, DoNothingIfTheTimerIsNullWhenTimerIsExpired)
{
    m_pAosTransaction->Timer_TimerExpired(nullptr);
}

TEST_F(AosTransactionTest, DoNothingIfInvalidTimerWhenTimerIsExpired)
{
    MockITimer objMockITimer;
    m_pAosTransaction->Timer_TimerExpired(&objMockITimer);
}
