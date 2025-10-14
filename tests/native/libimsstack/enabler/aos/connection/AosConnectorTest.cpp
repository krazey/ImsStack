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

#include "IIpcan.h"
#include "ServiceNetworkPolicy.h"
#include "connection/AosConnector.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "TestUtilService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosConnectorListener.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistration.h"
#include "../../interface/aos/MockIAosService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;

#define DECLARE_USING(Base)                         \
    using Base::IsCrossSimConnected;                \
    using Base::SetState;                           \
    using Base::SetDataConnected;                   \
    using Base::SetEmergencyType;                   \
    using Base::IsDataConnected;                    \
    using Base::IsTimerRunning;                     \
    using Base::CheckIpChangedForEmergency;         \
    using Base::CheckIpaAndProcessReadyRecovery;    \
    using Base::HandleInvalidPcscfAddress;          \
    using Base::SelectIpVersion;                    \
    using Base::Notify;                             \
    using Base::ProcessCheckingPcscfAndIpa;         \
    using Base::StartTimer;                         \
    using Base::StopTimer;                          \
    using Base::AosConnection_StateChanged;         \
    using Base::AosConnection_IpChanged;            \
    using Base::AosConnection_IpcanCatChanged;      \
    using Base::AosConnection_PcscfChanged;         \
    using Base::AosConnection_ConnectionFailed;     \
    using Base::Pcscf_NotifyResult;                 \
    using Base::ServicePhone_PcoValueChanged;       \
    using Base::ServicePhone_CrossSimStatusChanged; \
    using Base::CleanUp;                            \
    using Base::TimerToString;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");
const IMS_UINT32 TIMER_DURATION_MILLIS = 3000;

class TestAosConnector : public AosConnector
{
public:
    DECLARE_USING(AosConnector)

    explicit inline TestAosConnector(IN IAosAppContext* piAppContext) :
            AosConnector(piAppContext)
    {
    }

    inline IMS_UINT32 GetState() { return m_nState; }
    inline void SetPendingFeature(IN IMS_UINT32 nFeature) { m_nPendingFeature = nFeature; }
    inline IMS_UINT32 GetPendingFeature() { return m_nPendingFeature; }
    inline IMS_UINT32 GetReadyRecoveryCount() { return m_nReadyRecoveryCount; }
    inline void SetReadyRecoveryCount(IN IMS_UINT32 nCount) { m_nReadyRecoveryCount = nCount; }
    inline void SetPcscfChangeIgnored(IN IMS_BOOL bIsIgnored)
    {
        m_bIsPcscfChangeIgnored = bIsIgnored;
    }
    inline IMS_BOOL GetPcscfChangeIgnored() { return m_bIsPcscfChangeIgnored; }

    void NotifyTimerExpired(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosConnector::TIMER_IPV6:
                Timer_TimerExpired(m_piIpv6Timer);
                break;

            case AosConnector::TIMER_STOP_DELAY:
                Timer_TimerExpired(m_piStopDelayTimer);
                break;

            case AosConnector::TIMER_READY_RECOVERY:
                Timer_TimerExpired(m_piReadyRecoveryTimer);
                break;

            case AosConnector::TIMER_PCO_WAITING:
                Timer_TimerExpired(m_piPcoWaitingTimer);
                break;

            default:
                Timer_TimerExpired(IMS_NULL);
        }
    }

    void ClearAllTimers()
    {
        StopTimer(TIMER_IPV6);
        StopTimer(TIMER_STOP_DELAY);
        StopTimer(TIMER_READY_RECOVERY);
        StopTimer(TIMER_PCO_WAITING);
    }
};

class AosConnectorTest : public ::testing::Test
{
public:
    inline AosConnectorTest() :
            m_pAosConnector(IMS_NULL)
    {
        m_objTimerService.SetTimer(&m_objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);
        m_piAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(&m_objMockIAosService, SLOT_ID);
    }
    inline virtual ~AosConnectorTest()
    {
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    TestAosConnector* m_pAosConnector;
    TestTimerService m_objTimerService;
    TestUtilService m_objUtilService;

    AStringArray m_objPcscfs;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosService* m_piAosService;
    IAosCallTracker* m_piAosCallTracker;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosCallTracker m_objMockIAosCallTracker;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosConnectorListener m_objMockIAosConnectorListener;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistration m_objMockIAosRegistration;
    MockIAosService m_objMockIAosService;
    MockITimer m_objMockITimer;

protected:
    void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetPcscf()).WillByDefault(Return(&m_objMockIAosPcscf));
        ON_CALL(m_objMockIAosAppContext, GetApp()).WillByDefault(Return(&m_objMockIAosApplication));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&m_objMockIAosRegistration));
        ON_CALL(m_objMockIAosConnection, GetConnectionType())
                .WillByDefault(Return(NetworkPolicy::APN_IMS));
        ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
                .WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
        ON_CALL(m_objMockIAosPcscf, GetPcscfs()).WillByDefault(ReturnRef(m_objPcscfs));

        m_pAosConnector = new TestAosConnector(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosConnector != nullptr);

        m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker, 0);
    }

    void TearDown() override
    {
        m_objPcscfs.RemoveAllElements();
        if (m_pAosConnector)
        {
            m_pAosConnector->ClearAllTimers();
            delete m_pAosConnector;
            m_pAosConnector = IMS_NULL;
        }

        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker, 0);
    }
};

TEST_F(AosConnectorTest, NotifyActivatedIfAlreadyInReadyStateWhenStart)
{
    m_pAosConnector->SetState(AosConnector::STATE_READY);

    // notify Activated without invoking Activate because already ready
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pAosConnector->Start();
}

TEST_F(AosConnectorTest, UpdatePcscfIfPcscfChangeIgnoredWhenStart)
{
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    m_pAosConnector->SetPcscfChangeIgnored(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco());

    m_pAosConnector->Start();
}

TEST_F(AosConnectorTest, NotInvokeActivateIfPendingOperationExistWhenStart)
{
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_IPV6_DELAY);

    // not invoke Activate because connector is now pending
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);

    m_pAosConnector->Start();
}

TEST_F(AosConnectorTest, InvokeActivateWhenStart)
{
    // invoke Activate normally
    EXPECT_CALL(m_objMockIAosConnection, Activate());

    m_pAosConnector->Start();
}

TEST_F(AosConnectorTest, InvokeDeactivateAndCleanAllWhenStop)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);
    m_pAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pAosConnector->StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_IPV6_DELAY);
    m_pAosConnector->SetState(AosConnector::STATE_READY);

    // invoke Deactivate normally and notifies Deactivated
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(AnyNumber());

    m_pAosConnector->Stop();

    // timer, pending feature and state is cleaned during Stop
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, InvokeStopImmediatelyWhenStopWithoutDelay)
{
    // invoke Deactivate without TIMER_STOP_DELAY
    EXPECT_CALL(m_objMockIAosConnection, Deactivate());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pAosConnector->Stop(0);

    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, RunStopDelayTimerWhenStopWithDelay)
{
    // not invoke Deactivate but run TIMER_STOP_DELAY instead
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);

    m_pAosConnector->Stop(10);

    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, KeepStopDelayTimerIfAlreadyExistWhenStopWithDelay)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);

    // not invoke Deactivate and keep TIMER_STOP_DELAY
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    m_pAosConnector->Stop(10);

    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, CheckWhetherIsReady)
{
    // state is not STATE_READY
    EXPECT_FALSE(m_pAosConnector->IsReady());

    // state is changed to STATE_READY
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    EXPECT_TRUE(m_pAosConnector->IsReady());

    // if TIMER_STOP_DELAY is running, it should not be treated as STATE_READY
    m_pAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);
    EXPECT_FALSE(m_pAosConnector->IsReady());
}

TEST_F(AosConnectorTest, ClearCountAndTimerWhenResettingReadyRecovery)
{
    m_pAosConnector->SetReadyRecoveryCount(3);
    m_pAosConnector->StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_MILLIS);

    m_pAosConnector->ResetReadyRecovery();

    EXPECT_EQ(m_pAosConnector->GetReadyRecoveryCount(), 0);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, UpdateCrossSimStatusWhenCrossSimStatusChanged)
{
    m_pAosConnector->ServicePhone_CrossSimStatusChanged(IMS_TRUE);

    EXPECT_TRUE(m_pAosConnector->IsCrossSimConnected());
}

TEST_F(AosConnectorTest, NotUpdateCrossSimStatusForEmergencyTypeWhenCrossSimStatusChanged)
{
    m_pAosConnector->SetEmergencyType(IMS_TRUE);

    m_pAosConnector->ServicePhone_CrossSimStatusChanged(IMS_TRUE);

    EXPECT_FALSE(m_pAosConnector->IsCrossSimConnected());
}

TEST_F(AosConnectorTest, DoNothingIfAlreadyInReadyStateWhenStateChangedToActive)
{
    m_pAosConnector->SetState(AosConnector::STATE_READY);

    // not handle STATE_ACTIVE data state because already STATE_READY
    EXPECT_CALL(m_objMockIAosConnection, IsIpv6Preferred()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory()).Times(0);

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectorTest, WaitForIpv6IfRequiredWhenStateChangedToActive)
{
    m_objPcscfs.AddElement(AString("2001::1"));
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(m_objPcscfs));
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_TRUE));

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    // run TIMER_IPV6 and wait for IPv6 address
    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, DoNotWaitForIpv6IfCanNotOptainIpv6PcscfWhenStateChangedToActive)
{
    m_objPcscfs.AddElement(AString("::"));
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(m_objPcscfs));
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue())
            .WillByDefault(Return(AosConnector::PCO_INVALID_VALUE));

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, WaitForIpv6IfPcscfDiscoveryOtherThanPcoIsPossibleWhenStateChangedToActive)
{
    m_objPcscfs.AddElement(AString("::"));
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(m_objPcscfs));
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_FALSE));

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, WaitForPcoIfRequiredWhenStateChangedToActive)
{
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue())
            .WillByDefault(Return(AosConnector::PCO_INVALID_VALUE));

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    // run TIMER_PCO_WAITING and wait for PCO
    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_PCO_WAITING);
    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, WaitForPendingOperationCompleteWhenStateChangedToActive)
{
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));

    // not trying configure PCSCF and wait for pending feature processing
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectorTest, WaitForPcscfConfigureIfPcscfConfigureFailWhenStateChangedToActive)
{
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxRetryCnt()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryWaitTime()).WillByDefault(Return(20));

    // trying configure PCSCF but fails
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    // run TIMER_READY_RECOVERY and wait for PCSCF configuration
    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, NotifyActivatedIfPcscfConfigureSucceedWhenStateChangedToActive)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // configure PCSCF and notify Activated
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    EXPECT_TRUE(m_pAosConnector->IsDataConnected());
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, NotifyDeactivatedWhenStateChangedToIdle)
{
    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_DISCONNECTED));

    m_pAosConnector->AosConnection_StateChanged(IAosConnection::STATE_IDLE);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsDataConnected());
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, DoNothingIfIpNotChangedForEmergencyTypeWhenIpChangedInReadyState)
{
    m_pAosConnector->SetEmergencyType(IMS_TRUE);
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));
    AString strIpa = AString("10.10.10.10");
    const IpAddress objCurrIpa = IpAddress(strIpa);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillByDefault(ReturnRef(objCurrIpa));
    ON_CALL(m_objMockIAosRegistration, GetPropertyInternal(_, _, _))
            .WillByDefault(DoAll(SetArgPointee<2>(strIpa), Return(0)));

    // not handle IP change
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IP_CHANGED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(0);

    m_pAosConnector->AosConnection_IpChanged();
}

TEST_F(AosConnectorTest, NotifyIpChangedIfAvailableAddressExistWhenIpChangedInReadyState)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // notify IP is changed
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IP_CHANGED));

    m_pAosConnector->AosConnection_IpChanged();
}

TEST_F(AosConnectorTest, NotifyDeactivateIfIpVerisonChangedWhenIpChangedInReadyState)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::IPv6LOOPBACK));

    // notify Deactivated with IP changed
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_IP_CHANGED));

    m_pAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, NotifyDeactivateIfNoAvailableAddressExistWhenIpChangedInReadyState)
{
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    const IpAddress objInvalidAddress = IpAddress(AString("IpAddress"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(objInvalidAddress));

    // notify Deactivated with IP changed
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_IP_CHANGED));

    m_pAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, WaitForIpv6IfRequiredWhenIpChangedInIdleState)
{
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_IPV6_DELAY);
    m_pAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, WaitForPendingOperationCompleteWhenIpChangedInIdleState)
{
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    m_pAosConnector->SetPendingFeature(
            AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY);
    m_pAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::IPv6LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillByDefault(Return(IMS_TRUE));

    m_pAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, WaitForPcscfConfigureIfPcscfConfigureFailWhenIpChangedInIdleState)
{
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxRetryCnt()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryWaitTime()).WillByDefault(Return(20));

    m_pAosConnector->AosConnection_IpChanged();

    // run TIMER_READY_RECOVERY and wait for PCSCF configuration
    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, NotifyActivatedWhenIpChangedInIdleState)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, NotifyChangeWhenIpcanCategoryChanged)
{
    // notify IP CAN category is changed
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Updated(AosConnector::REASON_IPCAN_CAT_CHANGED));

    m_pAosConnector->AosConnection_IpcanCatChanged();
}

TEST_F(AosConnectorTest, DoNothingIfInvalidAppStateWhenPcscfChanged)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_NOTREADY));

    // not handle PCSCF change because the app state is not available
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pAosConnector->AosConnection_PcscfChanged();
}

TEST_F(AosConnectorTest, DoNothingIfKeepExistingPcscfOnPcscfChangeDuringTheCallIsTrueWhenPcscfChangeDuringTheCall)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosApplication, IsOn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // not handle PCSCF change because KeepExistingPcscfOnPcscfChangeDuringTheCall is True
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pAosConnector->AosConnection_PcscfChanged();

    EXPECT_TRUE(m_pAosConnector->GetPcscfChangeIgnored());
}

TEST_F(AosConnectorTest, NotifyChangeWhenPcscfChangedInIdleIfKeepExistingPcscfOnPcscfChangeDuringTheCallIsTrue)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosApplication, IsOn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_TRUE));

    // handle PCSCF change
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED));

    m_pAosConnector->AosConnection_PcscfChanged();

    EXPECT_FALSE(m_pAosConnector->GetPcscfChangeIgnored());
}

TEST_F(AosConnectorTest, NotifyChangeWhenPcscfChangedIfKeepExistingPcscfOnPcscfChangeDuringTheCallIsFalse)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosApplication, IsOn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_TRUE));

    // handle PCSCF change
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED));

    m_pAosConnector->AosConnection_PcscfChanged();

    EXPECT_FALSE(m_pAosConnector->GetPcscfChangeIgnored());
}

TEST_F(AosConnectorTest, DoNothingIfNoAvailablePcscfExistWhenPcscfChanged)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosApplication, IsOn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillByDefault(Return(IMS_FALSE));

    // not handle PCSCF change because the PCSCF address is not available
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pAosConnector->AosConnection_PcscfChanged();

    EXPECT_FALSE(m_pAosConnector->GetPcscfChangeIgnored());
}

TEST_F(AosConnectorTest, NotifyChangeWhenPcscfChanged)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosNConfiguration, ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall())
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_TRUE));

    // handle PCSCF change
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED));

    m_pAosConnector->AosConnection_PcscfChanged();
}

TEST_F(AosConnectorTest, NotifyDeactivatedWhenConnectionFailed)
{
    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_PERMANENTLY_FAILED));

    m_pAosConnector->AosConnection_ConnectionFailed();
}

TEST_F(AosConnectorTest, WaitForPendingOperationCompleteWhenPcscfNotifyResult)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    m_pAosConnector->SetPendingFeature(
            AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // not handle notified PCSCF result because there is still pending process
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pAosConnector->Pcscf_NotifyResult(IMS_TRUE);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_IPV6_DELAY);
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, NotifyActivatedWhenPcscfNotifyResult)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pAosConnector->Pcscf_NotifyResult(IMS_TRUE);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, NotNotifyDeactivatedIfInNotReadyStateWhenPcoValueChanged)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCO_WAITING);
    m_pAosConnector->SetState(AosConnector::STATE_IDLE);
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO))
            .Times(0);

    m_pAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, NotNotifyDeactivatedIfWhileInDeregisteringWhenPcoValueChanged)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCO_WAITING);
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_DEREGISTERING));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO))
            .Times(0);

    m_pAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, NotNotifyDeactivatedIfLimitedModeNotChangedWhenPcoValueChanged)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCO_WAITING);
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));
    ON_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_LIMITED));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO))
            .Times(0);

    m_pAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, NotifyDeactivatedIfLimitedModeChangedWhenPcoValueChanged)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCO_WAITING);
    m_pAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));
    ON_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_NORMAL));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO));

    m_pAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, WaitForPcoIfRequiredWhenIpv6TimerExpired)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_IPV6_DELAY);
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue())
            .WillByDefault(Return(AosConnector::PCO_INVALID_VALUE));

    m_pAosConnector->NotifyTimerExpired(AosConnector::TIMER_IPV6);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_PCO_WAITING);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, WaitForDataConnectionWhenIpv6TimerExpired)
{
    m_pAosConnector->SetDataConnected(IMS_FALSE);
    m_pAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_IPV6_DELAY);
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    m_pAosConnector->NotifyTimerExpired(AosConnector::TIMER_IPV6);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, NotifyActivatedIfPcscfAndLocalAddressIsAvailableWhenIpv6TimerExpired)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    m_pAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_IPV6_DELAY);
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pAosConnector->NotifyTimerExpired(AosConnector::TIMER_IPV6);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, InvokeStopWhenStopDelayTimerExpired)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pAosConnector->NotifyTimerExpired(AosConnector::TIMER_STOP_DELAY);

    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, InvokeStopIfNoDelayTimerExistWhenReadyRecoveryTimerExpired)
{
    m_pAosConnector->StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_MILLIS);

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pAosConnector->NotifyTimerExpired(AosConnector::TIMER_READY_RECOVERY);
}

TEST_F(AosConnectorTest, NotifyActivatedIfPcscfAndLocalAddressIsAvailableWhenPcoWaitingTimerExpired)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pAosConnector->SetDataConnected(IMS_TRUE);
    m_pAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pAosConnector->SetPendingFeature(AosConnector::PENDING_PCO_WAITING);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pAosConnector->NotifyTimerExpired(AosConnector::TIMER_PCO_WAITING);

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, DoNotHandleInvalidPcscfWhenFailToGetConfiguration)
{
    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL, SLOT_ID);

    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(_)).Times(0);

    m_pAosConnector->HandleInvalidPcscfAddress();

    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, StartTimerWithConfiguredValueWhenHandlesInvalidPcscfUnderMaxCount)
{
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxRetryCnt()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryWaitTime()).WillByDefault(Return(20));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryBaseTime()).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxTime()).Times(0);

    m_pAosConnector->HandleInvalidPcscfAddress();

    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, NotifyAsDeactivatedIfBaseTimeConfiguredZeroWhenHandlesInvalidPcscf)
{
    m_pAosConnector->SetReadyRecoveryCount(3);
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxRetryCnt()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryWaitTime()).WillByDefault(Return(20));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryBaseTime()).WillByDefault(Return(0));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxTime()).WillByDefault(Return(0));

    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_PCSCF_DISCOVERY_FAILED));

    m_pAosConnector->HandleInvalidPcscfAddress();
}

TEST_F(AosConnectorTest, StartTimerWithCalculatedValueWhenHandlesInvalidPcscfOverMaxCount)
{
    m_pAosConnector->SetReadyRecoveryCount(3);
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxRetryCnt()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryWaitTime()).WillByDefault(Return(20));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryBaseTime()).WillOnce(Return(20));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfRecoveryMaxTime()).WillOnce(Return(1800));

    m_pAosConnector->HandleInvalidPcscfAddress();

    EXPECT_TRUE(m_pAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, NotifyDeactivatedForEmergencyTypeWhenPcscfConfigureFail)
{
    m_pAosConnector->SetEmergencyType(IMS_TRUE);
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_FALSE));

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED));

    m_pAosConnector->ProcessCheckingPcscfAndIpa();

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
}

TEST_F(AosConnectorTest, ReturnFalseIfOfflineRegStateWhenCheckIpChangedForEmergency)
{
    m_pAosConnector->SetEmergencyType(IMS_TRUE);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_OFFLINE));

    IMS_BOOL bResult = m_pAosConnector->CheckIpChangedForEmergency();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfUnknownIpAddressWhenCheckIpChangedForEmergency)
{
    m_pAosConnector->SetEmergencyType(IMS_TRUE);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));
    AString strInvalidAddress = AString("IpAddress");
    ON_CALL(m_objMockIAosRegistration, GetPropertyInternal(_, _, _))
            .WillByDefault(DoAll(SetArgPointee<2>(strInvalidAddress), Return(0)));

    IMS_BOOL bResult = m_pAosConnector->CheckIpChangedForEmergency();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfFailToParsePcscfWhenCheckIpaAndProcessReadyRecovery)
{
    // calculate waiting time through the WaitTimeForFlowRecovery()
    m_pAosConnector->SetReadyRecoveryCount(3);
    m_objPcscfs.AddElement(AString("PcscfAddress"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_BOOL bResult = m_pAosConnector->CheckIpaAndProcessReadyRecovery();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfNonIpAddressWhenCheckIpaAndProcessReadyRecovery)
{
    m_objPcscfs.AddElement(AString("1:1:1:1"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillByDefault(ReturnRef(IpAddress::NONE));

    IMS_BOOL bResult = m_pAosConnector->CheckIpaAndProcessReadyRecovery();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfNoPcscfExistWhenSelectIpVersion)
{
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_BOOL bResult = m_pAosConnector->SelectIpVersion();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfPcscfAddressIsEmptyWhenSelectIpVersion)
{
    m_objPcscfs.AddElement(AString(""));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_BOOL bResult = m_pAosConnector->SelectIpVersion();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfInvalidPcscfAddressWhenSelectIpVersion)
{
    m_objPcscfs.AddElement(AString("PcscfAddress"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_BOOL bResult = m_pAosConnector->SelectIpVersion();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, ReturnFalseIfAddressVersionNotMatchWhenSelectIpVersion)
{
    m_objPcscfs.AddElement(AString("fe80::1"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_BOOL bResult = m_pAosConnector->SelectIpVersion();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectorTest, DoNothingIfListenerIsNullWhenNotify)
{
    m_pAosConnector->SetListener(IMS_NULL);

    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pAosConnector->Notify(AosConnector::LISTENER_TYPE_ACTIVATED);
}

TEST_F(AosConnectorTest, DoNothingIfListenerTypeIsUnknownWhenNotify)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);

    IMS_SINT32 nUnknownListenerType = -1;
    m_pAosConnector->Notify(nUnknownListenerType);
}

TEST_F(AosConnectorTest, RemoveListenersWhenCleanUp)
{
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosConnector)));
    EXPECT_CALL(m_objMockIAosPcscf, SetListener(IMS_NULL));
    EXPECT_CALL(m_objMockIAosConnection, RemoveListener(_));

    m_pAosConnector->CleanUp();

    EXPECT_EQ(m_pAosConnector->GetPendingFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pAosConnector->GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StopAndStartAgainIfTimerIsAlreadyExistWhenStartTimer)
{
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(2);
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(2);

    m_pAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);
    m_pAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);
}

TEST_F(AosConnectorTest, NotHandleInvalidTimer)
{
    IMS_SINT32 nInvalidTimer = -1;

    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    m_pAosConnector->StartTimer(nInvalidTimer, TIMER_DURATION_MILLIS);

    EXPECT_STREQ(m_pAosConnector->TimerToString(nInvalidTimer), "__INVALID__");

    EXPECT_FALSE(m_pAosConnector->IsTimerRunning(nInvalidTimer));

    m_pAosConnector->NotifyTimerExpired(nInvalidTimer);

    m_pAosConnector->StopTimer(nInvalidTimer);
}

TEST_F(AosConnectorTest, NotifyChangeWhenPcscfChangedIfProcessPendingPcscfChangeIsTrue)
{
    m_pAosConnector->SetPcscfChangeIgnored(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_TRUE));
    // handle PCSCF change
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED));

    m_pAosConnector->ProcessPendingPcscfChange();
    EXPECT_FALSE(m_pAosConnector->GetPcscfChangeIgnored());
}

TEST_F(AosConnectorTest, DoNothingIfNoAvailablePcscfExistWhenProcessPendingPcscfChange)
{
    m_pAosConnector->SetPcscfChangeIgnored(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_FALSE));

    // not handle PCSCF change because the PCSCF address is not available
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pAosConnector->ProcessPendingPcscfChange();
    EXPECT_FALSE(m_pAosConnector->GetPcscfChangeIgnored());
}