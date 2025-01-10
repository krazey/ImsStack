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

    FRIEND_TEST(AosTransactionTest, NullListenerIsNotAddedToTheList);
    FRIEND_TEST(AosTransactionTest, AddListOfTheTypeToTheMapIfDifferentTypeOfListener);
    FRIEND_TEST(AosTransactionTest, SameListenerIsNotAddedToTheList);
    FRIEND_TEST(AosTransactionTest, DifferentListenerIsAddedToTheList);
    FRIEND_TEST(AosTransactionTest, DoNothingIfListenerIsNullWhenRemoveListener);
    FRIEND_TEST(AosTransactionTest, DoNothingIfNoListenerForTheTypeWhenRemoveListener);
    FRIEND_TEST(AosTransactionTest, TransactionIsNotAllowedIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, CallIsImsTrafficAllowedWithRegType);
    FRIEND_TEST(AosTransactionTest, CallIsImsTrafficAllowedWithEmergencyType);
    FRIEND_TEST(AosTransactionTest, StartTrafficReturnTrueIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, StartTrafficReturnTrueIfAlreadyStarted);
    FRIEND_TEST(AosTransactionTest, StartTrafficStopsTimerIfStartUpdatedAndStopTimerIsRunning);
    FRIEND_TEST(AosTransactionTest,
            AddTheTrafficTypeToTheWaitingListIfStartUpdatedAndTrafficResponseWaiting);
    FRIEND_TEST(AosTransactionTest,
            DoNothingIfStartUpdatedButTrafficResponseWaitingWhenTrafficIsStarted);
    FRIEND_TEST(
            AosTransactionTest, ShouldNotStartImsTrafficIfNoTrafficInstanceWhenTrafficIsStarted);
    FRIEND_TEST(AosTransactionTest, StartImsTrafficIfStartNotUpdated);
    FRIEND_TEST(AosTransactionTest, EmergencyTrafficIsNotStartedIfImsRadioIsNull);
    FRIEND_TEST(AosTransactionTest, EmergencyTrafficIsNotStartedIfNoTrafficInstance);
    FRIEND_TEST(AosTransactionTest, StartingEmergencyTrafficIsNotDuplicated);
    FRIEND_TEST(AosTransactionTest, TrafficCannotBeStoppedIfNotStarted);
    FRIEND_TEST(AosTransactionTest, StopTrafficForTheTypeAndStartTimerIfTrafficStarted);
    FRIEND_TEST(AosTransactionTest, EmergencyTrafficCannotBeStoppedIfNotStarted);
    FRIEND_TEST(AosTransactionTest, EmergencyTrafficCanBeStoppedIfStarted);
    FRIEND_TEST(AosTransactionTest, CannotSetWlanIfImsTrafficIsNull);
    FRIEND_TEST(AosTransactionTest, SetWlanWithGivenValue);
    FRIEND_TEST(AosTransactionTest, NrIsConvertedToNgran);
    FRIEND_TEST(AosTransactionTest, LteIsConvertedToEutran);
    FRIEND_TEST(AosTransactionTest, EhrpdIsConvertedToUtran);
    FRIEND_TEST(AosTransactionTest, WcdmaIsConvertedToUtran);
    FRIEND_TEST(AosTransactionTest, HspaIsConvertedToUtran);
    FRIEND_TEST(AosTransactionTest, WlanIsConvertedToIwlan);
    FRIEND_TEST(AosTransactionTest, OtherRatsAreConvertedToUnknown);
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
    void SetUp() override
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

    void TearDown() override
    {
        if (m_pAosTransaction)
        {
            m_pAosTransaction->SetOriginRadio();
            delete m_pAosTransaction;
            m_pAosTransaction = IMS_NULL;
        }
    }
};

TEST_F(AosTransactionTest, NullListenerIsNotAddedToTheList)
{
    // GIVEN
    IMS_SINT32 nCountBefore = m_pAosTransaction->GetListeners().GetSize();

    // WHEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, nullptr);

    // THEN
    IMS_SINT32 nCountAfter = m_pAosTransaction->GetListeners().GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore);
}

TEST_F(AosTransactionTest, AddListOfTheTypeToTheMapIfDifferentTypeOfListener)
{
    // GIVEN
    MockIAosTransactionListener objMockIAosTransactionListenerForReg;
    MockIAosTransactionListener objMockIAosTransactionListenerForSub;
    MockIAosTransactionListener objMockIAosTransactionListenerForEmergency;
    IMS_SINT32 nCountBefore = m_pAosTransaction->GetListeners().GetSize();

    // WHEN
    m_pAosTransaction->SetListener(
            IAosTransaction::TYPE_REG, &objMockIAosTransactionListenerForReg);
    m_pAosTransaction->SetListener(
            IAosTransaction::TYPE_SUB, &objMockIAosTransactionListenerForSub);
    m_pAosTransaction->SetListener(
            IAosTransaction::TYPE_EMERGENCY, &objMockIAosTransactionListenerForEmergency);

    // THEN
    IMS_SINT32 nCountAfter = m_pAosTransaction->GetListeners().GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore + 3);
}

TEST_F(AosTransactionTest, SameListenerIsNotAddedToTheList)
{
    // GIVEN
    MockIAosTransactionListener objMockIAosTransactionListener;
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);
    IMS_SINT32 nCountBefore =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();

    // WHEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);

    // THEN
    IMS_SINT32 nCountAfter =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();
    EXPECT_EQ(nCountAfter, nCountBefore);
}

TEST_F(AosTransactionTest, DifferentListenerIsAddedToTheList)
{
    // GIVEN
    MockIAosTransactionListener objMockIAosTransactionListener;
    MockIAosTransactionListener objMockIAosTransactionListener2;
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener);
    IMS_SINT32 nCountBefore =
            m_pAosTransaction->GetListeners().GetValue(IAosTransaction::TYPE_REG).GetSize();

    // WHEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &objMockIAosTransactionListener2);

    // THEN
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

TEST_F(AosTransactionTest, TransactionIsNotAllowedIfImsRadioIsNull)
{
    // GIVEN
    m_pAosTransaction->m_piImsRadio = nullptr;

    // WHEN
    EXPECT_FALSE(m_pAosTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG));

    // THEN: Return false
}

TEST_F(AosTransactionTest, CallIsImsTrafficAllowedWithRegType)
{
    // GIVEN
    EXPECT_CALL(m_objMockIImsRadio, IsImsTrafficAllowed(IImsRadio::TRAFFIC_TYPE_REGISTRATION));

    // WHEN
    m_pAosTransaction->IsTransactionAllowed(IAosTransaction::TYPE_REG);

    // THEN: Given condition should be met
}

TEST_F(AosTransactionTest, CallIsImsTrafficAllowedWithEmergencyType)
{
    // GIVEN
    EXPECT_CALL(m_objMockIImsRadio, IsImsTrafficAllowed(IImsRadio::TRAFFIC_TYPE_EMERGENCY));

    // WHEN
    m_pAosTransaction->IsTransactionAllowed(IAosTransaction::TYPE_EMERGENCY);

    // THEN: Given condition should be met
}

TEST_F(AosTransactionTest, StartTrafficReturnTrueIfImsRadioIsNull)
{
    // GIVEN
    m_pAosTransaction->m_piImsRadio = nullptr;

    // WHEN
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));

    // THEN: Return true
}

TEST_F(AosTransactionTest, StartTrafficReturnTrueIfAlreadyStarted)
{
    // GIVEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));

    // WHEN
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));

    // THEN: Return true
}

TEST_F(AosTransactionTest, StartTrafficStopsTimerIfStartUpdatedAndStopTimerIsRunning)
{
    // GIVEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);
    m_pAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);

    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());
    EXPECT_TRUE(m_pAosTransaction->IsTimerRunning());

    // WHEN
    EXPECT_TRUE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));

    // THEN
    EXPECT_FALSE(m_pAosTransaction->IsTimerRunning());
}

TEST_F(AosTransactionTest, AddTheTrafficTypeToTheWaitingListIfStartUpdatedAndTrafficResponseWaiting)
{
    // GIVEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);

    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());
    EXPECT_TRUE(m_pAosTransaction->IsTrafficResponseWaiting());
    EXPECT_FALSE(m_pAosTransaction->IsResponseWaiting(IAosTransaction::TYPE_REG));

    // WHEN
    EXPECT_FALSE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_SUB, NW_REPORT_RADIO_LTE));

    // THEN
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

TEST_F(AosTransactionTest, StartImsTrafficIfStartNotUpdated)
{
    // GIVEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    // WHEN
    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_REGISTRATION, _, IImsRadio::DIRECTION_MO, _));

    // THEN
    EXPECT_FALSE(m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE));
}

TEST_F(AosTransactionTest, EmergencyTrafficIsNotStartedIfImsRadioIsNull)
{
    // GIVEN
    m_pAosTransaction->m_piImsRadio = nullptr;
    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, _, IImsRadio::DIRECTION_MO, _))
            .Times(0);

    // WHEN
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);

    // THEN: Do nothing
}

TEST_F(AosTransactionTest, EmergencyTrafficIsNotStartedIfNoTrafficInstance)
{
    // GIVEN
    m_pAosTransaction->ClearTraffics();

    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, _, IImsRadio::DIRECTION_MO, _))
            .Times(0);

    // WHEN
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);

    // THEN: Do nothing
}

TEST_F(AosTransactionTest, StartingEmergencyTrafficIsNotDuplicated)
{
    // GIVEN
    EXPECT_CALL(m_objMockIImsRadio,
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);

    // WHEN
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);

    // THEN: The GIVEN condition should be met
}

TEST_F(AosTransactionTest, TrafficCannotBeStoppedIfNotStarted)
{
    // GIVEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));

    // WHEN
    m_pAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);

    // THEN: Do nothing
}

TEST_F(AosTransactionTest, StopTrafficForTheTypeAndStartTimerIfTrafficStarted)
{
    // GIVEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_FALSE(m_pAosTransaction->IsStartUpdated());

    m_pAosTransaction->StartTraffic(IAosTransaction::TYPE_REG, NW_REPORT_RADIO_LTE);

    EXPECT_TRUE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_TRUE(m_pAosTransaction->IsStartUpdated());

    // WHEN
    m_pAosTransaction->StopTraffic(IAosTransaction::TYPE_REG);

    // THEN
    EXPECT_FALSE(m_pAosTransaction->IsStarted(IAosTransaction::TYPE_REG));
    EXPECT_TRUE(m_pAosTransaction->IsTimerRunning());

    // Cleaning up the timer
    m_pAosTransaction->InvokeTimerExpired();
    EXPECT_FALSE(m_pAosTransaction->IsTimerRunning());
}

TEST_F(AosTransactionTest, EmergencyTrafficCannotBeStoppedIfNotStarted)
{
    // GIVEN
    EXPECT_CALL(m_objMockIImsRadio, StopImsTraffic(_)).Times(0);

    // WHEN
    m_pAosTransaction->StopEmergencyTraffic();

    // THEN: Do nothing
}

TEST_F(AosTransactionTest, EmergencyTrafficCanBeStoppedIfStarted)
{
    // GIVEN
    m_pAosTransaction->StartEmergencyTraffic(IAosTransaction::TYPE_REG);
    EXPECT_CALL(m_objMockIImsRadio, StopImsTraffic(_)).Times(1);

    // WHEN
    m_pAosTransaction->StopEmergencyTraffic();

    // THEN: The GIVEN condition should be met
}

TEST_F(AosTransactionTest, CannotSetWlanIfImsTrafficIsNull)
{
    // GIVEN
    TestImsRadioService objTestImsRadioService;
    PlatformService* pOrigService = PlatformContext::GetInstance()->SetService(
            PlatformContext::SERVICE_RADIO, &objTestImsRadioService);

    objTestImsRadioService.SetImsTraffic(IMS_NULL);
    EXPECT_CALL(objTestImsRadioService.GetMockImsTraffic(), SetWlan(_, _)).Times(0);

    // WHEN
    m_pAosTransaction->SetWlan(IMS_TRUE);

    // THEN: The GIVEN condition should be met (no call SetWlan)

    // Cleaning up (restore original service)
    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, pOrigService);
}

TEST_F(AosTransactionTest, SetWlanWithGivenValue)
{
    // GIVEN
    TestImsRadioService objTestImsRadioService;
    PlatformService* pOrigService = PlatformContext::GetInstance()->SetService(
            PlatformContext::SERVICE_RADIO, &objTestImsRadioService);

    EXPECT_CALL(objTestImsRadioService.GetMockImsTraffic(), SetWlan(_, _)).Times(2);

    // WHEN
    m_pAosTransaction->SetWlan(IMS_TRUE);
    m_pAosTransaction->SetWlan(IMS_FALSE);

    // THEN: The GIVEN condition should be met (Call SetWlan)

    // Cleaning up (restore original service)
    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, pOrigService);
}

TEST_F(AosTransactionTest, NrIsConvertedToNgran)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_NR),
            IImsRadio::ACCESS_NETWORK_TYPE_NGRAN);
}

TEST_F(AosTransactionTest, LteIsConvertedToEutran)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_LTE),
            IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN);
}

TEST_F(AosTransactionTest, EhrpdIsConvertedToUtran)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_EHRPD),
            IImsRadio::ACCESS_NETWORK_TYPE_UTRAN);
}

TEST_F(AosTransactionTest, WcdmaIsConvertedToUtran)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_WCDMA),
            IImsRadio::ACCESS_NETWORK_TYPE_UTRAN);
}

TEST_F(AosTransactionTest, HspaIsConvertedToUtran)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_HSPA),
            IImsRadio::ACCESS_NETWORK_TYPE_UTRAN);
}

TEST_F(AosTransactionTest, WlanIsConvertedToIwlan)
{
    EXPECT_EQ(m_pAosTransaction->GetAccessNetworkType(NW_REPORT_RADIO_WLAN),
            IImsRadio::ACCESS_NETWORK_TYPE_IWLAN);
}

TEST_F(AosTransactionTest, OtherRatsAreConvertedToUnknown)
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

    // THEN: The GIVEN condition should be met
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

    // THEN: The GIVEN condition should be met
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

    // THEN: The GIVEN condition should be met
}

TEST_F(AosTransactionTest, ShouldNotifyToTheListenersWhenTrafficConnectionSetupIsPrepared)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnConnectionSetupPrepared()).Times(1);

    // WHEN
    m_pAosTransaction->Traffic_OnConnectionSetupPrepared(IAosTransaction::TYPE_REG);

    // THEN: The GIVEN condition should be met
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

    // THEN: The GIVEN condition should be met
}

TEST_F(AosTransactionTest, ShouldNotifyToOneListenerWhenTrafficPriorityChanged)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnTrafficPriorityChanged()).Times(1);

    // WHEN
    m_pAosTransaction->ImsRadio_OnTrafficPriorityChanged();

    // THEN: The GIVEN condition should be met
}

TEST_F(AosTransactionTest, ShouldNotifyToTwoListenersWhenTrafficPriorityChanged)
{
    // GIVEN
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_REG, &m_objMockIAosTransactionListener);
    m_pAosTransaction->SetListener(IAosTransaction::TYPE_SUB, &m_objMockIAosTransactionListener);
    EXPECT_CALL(m_objMockIAosTransactionListener, Transaction_OnTrafficPriorityChanged()).Times(2);

    // WHEN
    m_pAosTransaction->ImsRadio_OnTrafficPriorityChanged();

    // THEN: The GIVEN condition should be met
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
