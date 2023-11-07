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
#include "TestUtilService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
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

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");
const IMS_UINT32 TIMER_DURATION_MILLIS = 3000;

class TestAosConnector : public AosConnector
{
public:
    TestAosConnector(IN IAosAppContext* piAppContext) :
            AosConnector(piAppContext)
    {
    }

    FRIEND_TEST(AosConnectorTest, StartWhenPcscfChangeNotIgnored_NotifyActivatedIfReadyState);
    FRIEND_TEST(AosConnectorTest, StartWhenPcscfChangeIgnored_NotifyActivatedIfReadyState);
    FRIEND_TEST(AosConnectorTest, Start_NotInvokeActivateIfPendingProcessExist);
    FRIEND_TEST(AosConnectorTest, Start_InvokeActivate);
    FRIEND_TEST(AosConnectorTest, Stop_InvokeDeactivateAndCleanAll);
    FRIEND_TEST(AosConnectorTest, StopWithZeroDelay_InvokeStopImmediately);
    FRIEND_TEST(AosConnectorTest, StopWithDelay_RunStopDelayTimer);
    FRIEND_TEST(AosConnectorTest, StopWithDelay_KeepStopDelayTimerIfStopDelayTimerExist);
    FRIEND_TEST(AosConnectorTest, CheckWhetherIsReady);
    FRIEND_TEST(AosConnectorTest, CheckWhetherIsPdnDeactivateRequired);
    FRIEND_TEST(AosConnectorTest, StateChangedToActive_IgnoreIfReadyState);
    FRIEND_TEST(AosConnectorTest, StateChangedToActive_WaitForIpv6);
    FRIEND_TEST(AosConnectorTest, StateChangedToActive_WaitForPco);
    FRIEND_TEST(AosConnectorTest, StateChangedToActive_WaitForPendingProcess);
    FRIEND_TEST(AosConnectorTest, StateChangedToActive_WaitForPcscfConfigureIfPcscfConfigureFail);
    FRIEND_TEST(AosConnectorTest, StateChangedToActive_Activated);
    FRIEND_TEST(AosConnectorTest, StateChangedToIdle_Deactivated);
    FRIEND_TEST(AosConnectorTest, IpChangedInReadyStateForEmergency_IgnoreIfSame);
    FRIEND_TEST(AosConnectorTest, IpChangedInReadyState_UpdateIfIpVersionMatch);
    FRIEND_TEST(AosConnectorTest, IpChangedInReadyState_DeactivateIfInvalidIpAddress);
    FRIEND_TEST(AosConnectorTest, IpChangedInIdleState_WaitForIpv6);
    FRIEND_TEST(AosConnectorTest, IpChangedInIdleState_WaitForPendingProcess);
    FRIEND_TEST(AosConnectorTest, IpChangedInIdleState_WaitForPcscfConfigureIfPcscfConfigureFail);
    FRIEND_TEST(AosConnectorTest, IpChangedInIdleState_Activated);
    FRIEND_TEST(AosConnectorTest, IpcanCatChanged_Updated);
    FRIEND_TEST(AosConnectorTest, PcscfChanged_IgnoreIfInvalidState);
    FRIEND_TEST(AosConnectorTest, PcscfChanged_IgnoreIfConfiguredToNotInitReg);
    FRIEND_TEST(AosConnectorTest, PcscfChanged_IgnoreIfUnavailablePcscf);
    FRIEND_TEST(AosConnectorTest, PcscfChanged_Notified);
    FRIEND_TEST(AosConnectorTest, ConnectionFailed_Deactivated);
    FRIEND_TEST(AosConnectorTest, PcscfNotifyResult_WaitForPendingProcess);
    FRIEND_TEST(AosConnectorTest, PcscfNotifyResult_Activated);
    FRIEND_TEST(AosConnectorTest, PcoValueChanged_UpdatePcoWhenNotReadyState);
    FRIEND_TEST(AosConnectorTest, PcoValueChanged_UpdatePcoWhenReadyStateButDeregistering);
    FRIEND_TEST(AosConnectorTest, PcoValueChanged_UpdatePcoWhenReadyStateAndLimitedModeNotChanged);
    FRIEND_TEST(AosConnectorTest, PcoValueChanged_UpdatePcoAndDeactivatedWhenLimitedModeChanged);
    FRIEND_TEST(AosConnectorTest, Ipv6TimerExpired_WaitForPco);
    FRIEND_TEST(AosConnectorTest, Ipv6TimerExpired_WaitForDataConnection);
    FRIEND_TEST(AosConnectorTest, Ipv6TimerExpired_ProcessCheckingPcscfAndIpa);
    FRIEND_TEST(AosConnectorTest, StopDelayTimerExpired_InvokeStop);
    FRIEND_TEST(AosConnectorTest, ReadyRecoveryTimerExpired_InvokeStopIfNoDelayTimerExist);
    FRIEND_TEST(AosConnectorTest, ReadyRecoveryTimerExpired_WaitForStopDelayTime);
    FRIEND_TEST(AosConnectorTest, PcoWaitingTimerExpired_ProcessCheckingPcscfAndIpa);
    FRIEND_TEST(AosConnectorTest, CheckPcscfAndIpa_EmergencyTypeDeactivatedIfPcscfConfigureFail);
    FRIEND_TEST(AosConnectorTest, CheckIpChangedForEmergency_ReturnFalseIfOfflineRegState);
    FRIEND_TEST(AosConnectorTest, CheckIpChangedForEmergency_ReturnFalseIfUnknownIpAddress);
    FRIEND_TEST(AosConnectorTest, CheckIpaAndProcessReadyRecovery_ReturnFalseIfFailParsePcscf);
    FRIEND_TEST(AosConnectorTest, CheckIpaAndProcessReadyRecovery_ReturnFalseIfNonIpAddress);
    FRIEND_TEST(AosConnectorTest, SelectIpVersion_FailIfNoPcscf);
    FRIEND_TEST(AosConnectorTest, SelectIpVersion_FailIfPcscfAddressIsEmpty);
    FRIEND_TEST(AosConnectorTest, SelectIpVersion_FailIfInvalidPcscfAddress);
    FRIEND_TEST(AosConnectorTest, SelectIpVersion_FailIfAddressVersionNotMatch);
    FRIEND_TEST(AosConnectorTest, Notify_IgnoredIfNullListener);
    FRIEND_TEST(AosConnectorTest, Notify_IgnoredIfUnknownListenerType);
    FRIEND_TEST(AosConnectorTest, CleanUp);
    FRIEND_TEST(AosConnectorTest, InvalidTimer_Ignored);

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
            m_pTestUtilService(new TestUtilService())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, m_pTestUtilService);

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration), SLOT_ID);
        m_piAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockIAosService), SLOT_ID);
    }
    inline virtual ~AosConnectorTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
    }

    TestAosConnector* m_pTestAosConnector;
    TestUtilService* m_pTestUtilService;

    AStringArray m_objPcscfs;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosService* m_piAosService;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosConnectorListener m_objMockIAosConnectorListener;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistration m_objMockIAosRegistration;
    MockIAosService m_objMockIAosService;

protected:
    virtual void SetUp() override
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

        m_pTestAosConnector =
                new TestAosConnector(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        m_pTestAosConnector->SetListener(&m_objMockIAosConnectorListener);
    }

    virtual void TearDown() override
    {
        m_objPcscfs.RemoveAllElements();
        m_pTestAosConnector->ClearAllTimers();
        if (m_pTestAosConnector)
        {
            delete m_pTestAosConnector;
        }
    }
};

TEST_F(AosConnectorTest, StartWhenPcscfChangeNotIgnored_NotifyActivatedIfReadyState)
{
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);

    // notify Activated without invoking Activate because already ready
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).Times(0);

    EXPECT_FALSE(m_pTestAosConnector->Start());
}

TEST_F(AosConnectorTest, StartWhenPcscfChangeIgnored_NotifyActivatedIfReadyState)
{
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
    m_pTestAosConnector->m_bIsPcscfChangeIgnored = IMS_TRUE;

    // notify Activated without invoking Activate because already ready
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco());

    EXPECT_FALSE(m_pTestAosConnector->Start());
}

TEST_F(AosConnectorTest, Start_NotInvokeActivateIfPendingProcessExist)
{
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_IPV6_DELAY;

    // not invoke Activate because connector is now pending
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);

    EXPECT_FALSE(m_pTestAosConnector->Start());
}

TEST_F(AosConnectorTest, Start_InvokeActivate)
{
    // invoke Activate normally
    EXPECT_CALL(m_objMockIAosConnection, Activate());

    EXPECT_TRUE(m_pTestAosConnector->Start());
}

TEST_F(AosConnectorTest, Stop_InvokeDeactivateAndCleanAll)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_IPV6_DELAY;
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);

    // invoke Deactivate normally and notifies Deactivated
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(AnyNumber());

    m_pTestAosConnector->Stop();

    // timer, pending feature and state is cleaned during Stop
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StopWithZeroDelay_InvokeStopImmediately)
{
    // invoke Deactivate without TIMER_STOP_DELAY
    EXPECT_CALL(m_objMockIAosConnection, Deactivate());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pTestAosConnector->Stop(0);

    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, StopWithDelay_RunStopDelayTimer)
{
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));

    // not invoke Deactivate but run TIMER_STOP_DELAY instead
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);

    m_pTestAosConnector->Stop(1);

    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, StopWithDelay_KeepStopDelayTimerIfStopDelayTimerExist)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);

    // not invoke Deactivate and keep TIMER_STOP_DELAY
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);

    m_pTestAosConnector->Stop(1);

    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, CheckWhetherIsReady)
{
    // state is not STATE_READY
    EXPECT_FALSE(m_pTestAosConnector->IsReady());

    // state is changed to STATE_READY
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
    EXPECT_TRUE(m_pTestAosConnector->IsReady());

    // if TIMER_STOP_DELAY is running, it should not be treated as STATE_READY
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);
    EXPECT_FALSE(m_pTestAosConnector->IsReady());
}

TEST_F(AosConnectorTest, CheckWhetherIsPdnDeactivateRequired)
{
    // PdnDeactivateRequired is false
    EXPECT_FALSE(m_pTestAosConnector->IsPdnDeactivationRequired());

    m_pTestAosConnector->SetPdnDeactivationRequired(IMS_TRUE);

    // PdnDeactivateRequired is changed to true
    EXPECT_TRUE(m_pTestAosConnector->IsPdnDeactivationRequired());
}

TEST_F(AosConnectorTest, StateChangedToActive_IgnoreIfReadyState)
{
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);

    // not handle STATE_ACTIVE data state because already STATE_READY
    EXPECT_CALL(m_objMockIAosConnection, IsIpv6Preferred()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory()).Times(0);

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectorTest, StateChangedToActive_WaitForIpv6)
{
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    // run TIMER_IPV6 and wait for IPv6 address
    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, StateChangedToActive_WaitForPco)
{
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue())
            .WillByDefault(Return(TestAosConnector::PCO_INVALID_VALUE));

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    // run TIMER_PCO_WAITING and wait for PCO
    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_PCO_WAITING);
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, StateChangedToActive_WaitForPendingProcess)
{
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCSCF_CONFIG_READY;
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));

    // not trying configure PCSCF and wait for pending feature processing
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectorTest, StateChangedToActive_WaitForPcscfConfigureIfPcscfConfigureFail)
{
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillByDefault(Return(IMS_FALSE));

    // trying configure PCSCF but fails
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    // run TIMER_READY_RECOVERY and wait for PCSCF configuration
    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, StateChangedToActive_Activated)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    ON_CALL(m_objMockIAosConnection, IsIpv6Preferred()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // configure PCSCF and notify Activated
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);

    EXPECT_TRUE(m_pTestAosConnector->IsDataConnected());
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, StateChangedToIdle_Deactivated)
{
    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_DISCONNECTED));

    m_pTestAosConnector->AosConnection_StateChanged(IAosConnection::STATE_IDLE);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsDataConnected());
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChangedInReadyStateForEmergency_IgnoreIfSame)
{
    m_pTestAosConnector->SetEmergencyType(IMS_TRUE);
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
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

    m_pTestAosConnector->AosConnection_IpChanged();
}

TEST_F(AosConnectorTest, IpChangedInReadyState_UpdateIfIpVersionMatch)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // notify IP is changed
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IP_CHANGED));
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(0);

    m_pTestAosConnector->AosConnection_IpChanged();
}

TEST_F(AosConnectorTest, IpChangedInReadyState_DeactivateIfInvalidIpAddress)
{
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
    const IpAddress objInvalidAddress = IpAddress(AString("IpAddress"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(objInvalidAddress));

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IP_CHANGED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED));

    m_pTestAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChangedInIdleState_WaitForIpv6)
{
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_IPV6_DELAY;
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    m_pTestAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChangedInIdleState_WaitForPendingProcess)
{
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    m_pTestAosConnector->m_nPendingFeature =
            (AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::IPv6LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillByDefault(Return(IMS_TRUE));

    m_pTestAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChangedInIdleState_WaitForPcscfConfigureIfPcscfConfigureFail)
{
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillByDefault(Return(IMS_FALSE));

    m_pTestAosConnector->AosConnection_IpChanged();

    // run TIMER_READY_RECOVERY and wait for PCSCF configuration
    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, IpChangedInIdleState_Activated)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pTestAosConnector->AosConnection_IpChanged();

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, IpcanCatChanged_Updated)
{
    // notify IP CAN category is changed
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Updated(AosConnector::REASON_IPCAN_CAT_CHANGED));

    m_pTestAosConnector->AosConnection_IpcanCatChanged();
}

TEST_F(AosConnectorTest, PcscfChanged_IgnoreIfInvalidState)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_NOTREADY));

    // not handle PCSCF change because the app state is not available
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pTestAosConnector->AosConnection_PcscfChanged();
}
TEST_F(AosConnectorTest, PcscfChanged_IgnoreIfConfiguredToNotInitReg)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosApplication, IsOn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsNoInitRegOnPcscfChange())
            .WillByDefault(Return(IMS_TRUE));

    // not handle PCSCF change because configured to not initialize registration
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pTestAosConnector->AosConnection_PcscfChanged();

    EXPECT_TRUE(m_pTestAosConnector->m_bIsPcscfChangeIgnored);
}

TEST_F(AosConnectorTest, PcscfChanged_IgnoreIfUnavailablePcscf)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosNConfiguration, IsNoInitRegOnPcscfChange())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillByDefault(Return(IMS_FALSE));

    // not handle PCSCF change because the PCSCF address is not available
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);

    m_pTestAosConnector->AosConnection_PcscfChanged();
}

TEST_F(AosConnectorTest, PcscfChanged_Notified)
{
    ON_CALL(m_objMockIAosApplication, GetAppState())
            .WillByDefault(Return(IAosApplication::STATE_CONNECTING));
    ON_CALL(m_objMockIAosNConfiguration, IsNoInitRegOnPcscfChange())
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_TRUE));

    // handle PCSCF change
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED));

    m_pTestAosConnector->AosConnection_PcscfChanged();
}

TEST_F(AosConnectorTest, ConnectionFailed_Deactivated)
{
    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_PERMANENTLY_FAILED));

    m_pTestAosConnector->AosConnection_ConnectionFailed();
}

TEST_F(AosConnectorTest, PcscfNotifyResult_WaitForPendingProcess)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    m_pTestAosConnector->m_nPendingFeature =
            AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY;
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // not handle notified PCSCF result because there is still pending process
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pTestAosConnector->Pcscf_NotifyResult(IMS_TRUE);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_IPV6_DELAY);
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, PcscfNotifyResult_Activated)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCSCF_CONFIG_READY;
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    m_pTestAosConnector->Pcscf_NotifyResult(IMS_TRUE);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, PcoValueChanged_UpdatePcoWhenNotReadyState)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCO_WAITING;
    m_pTestAosConnector->SetState(AosConnector::STATE_IDLE);
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistration, GetMode()).Times(0);

    m_pTestAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, PcoValueChanged_UpdatePcoWhenReadyStateButDeregistering)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCO_WAITING;
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_DEREGISTERING));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistration, GetMode()).Times(0);

    m_pTestAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, PcoValueChanged_UpdatePcoWhenReadyStateAndLimitedModeNotChanged)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCO_WAITING;
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
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

    m_pTestAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, PcoValueChanged_UpdatePcoAndDeactivatedWhenLimitedModeChanged)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCO_WAITING;
    m_pTestAosConnector->SetState(AosConnector::STATE_READY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));
    ON_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_NORMAL));

    IMS_UINT32 nPcoValue = 5;
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(nPcoValue));
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO));

    m_pTestAosConnector->ServicePhone_PcoValueChanged(nPcoValue);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_WaitForPco)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_IPV6_DELAY;
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue())
            .WillByDefault(Return(TestAosConnector::PCO_INVALID_VALUE));

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_IPV6);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_PCO_WAITING);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_WaitForDataConnection)
{
    m_pTestAosConnector->SetDataConnected(IMS_FALSE);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_IPV6_DELAY;
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_IPV6);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_ProcessCheckingPcscfAndIpa)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_IPV6_DELAY;
    ON_CALL(m_pTestUtilService->GetMockPrivateProperty(),
            GetPersistentInt(
                    Eq(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_IPV6);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, StopDelayTimerExpired_InvokeStop)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_STOP_DELAY);

    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, ReadyRecoveryTimerExpired_InvokeStopIfNoDelayTimerExist)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_MILLIS);

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_READY_RECOVERY);

    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, ReadyRecoveryTimerExpired_WaitForStopDelayTime)
{
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_MILLIS);

    // wait for stop delay time to expire
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(0);

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_READY_RECOVERY);

    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_TRUE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, PcoWaitingTimerExpired_ProcessCheckingPcscfAndIpa)
{
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    m_pTestAosConnector->SetDataConnected(IMS_TRUE);
    m_pTestAosConnector->StartTimer(AosConnector::TIMER_PCO_WAITING, TIMER_DURATION_MILLIS);
    m_pTestAosConnector->m_nPendingFeature = AosConnector::PENDING_PCO_WAITING;
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));

    m_pTestAosConnector->NotifyTimerExpired(AosConnector::TIMER_PCO_WAITING);

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
}

TEST_F(AosConnectorTest, CheckPcscfAndIpa_EmergencyTypeDeactivatedIfPcscfConfigureFail)
{
    m_pTestAosConnector->SetEmergencyType(IMS_TRUE);
    ON_CALL(m_objMockIAosPcscf, IsConfigured()).WillByDefault(Return(IMS_FALSE));

    // notify Deactivated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED));

    m_pTestAosConnector->ProcessCheckingPcscfAndIpa();

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
}

TEST_F(AosConnectorTest, CheckIpChangedForEmergency_ReturnFalseIfOfflineRegState)
{
    m_pTestAosConnector->SetEmergencyType(IMS_TRUE);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_OFFLINE));

    EXPECT_FALSE(m_pTestAosConnector->CheckIpChangedForEmergency());
}

TEST_F(AosConnectorTest, CheckIpChangedForEmergency_ReturnFalseIfUnknownIpAddress)
{
    m_pTestAosConnector->SetEmergencyType(IMS_TRUE);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));
    AString strInvalidAddress = AString("IpAddress");
    ON_CALL(m_objMockIAosRegistration, GetPropertyInternal(_, _, _))
            .WillByDefault(DoAll(SetArgPointee<2>(strInvalidAddress), Return(0)));

    EXPECT_FALSE(m_pTestAosConnector->CheckIpChangedForEmergency());
}

TEST_F(AosConnectorTest, CheckIpaAndProcessReadyRecovery_ReturnFalseIfFailParsePcscf)
{
    // calculate waiting time through the WaitTimeForFlowRecovery()
    m_pTestAosConnector->m_nReadyRecoveryCount = TestAosConnector::READY_RECOVERY_DEFAULT_COUNT;
    m_objPcscfs.AddElement(AString("PcscfAddress"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_FALSE(m_pTestAosConnector->CheckIpaAndProcessReadyRecovery());
}

TEST_F(AosConnectorTest, CheckIpaAndProcessReadyRecovery_ReturnFalseIfNonIpAddress)
{
    m_objPcscfs.AddElement(AString("1:1:1:1"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillByDefault(ReturnRef(IpAddress::NONE));

    EXPECT_FALSE(m_pTestAosConnector->CheckIpaAndProcessReadyRecovery());
}

TEST_F(AosConnectorTest, SelectIpVersion_FailIfNoPcscf)
{
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_FALSE(m_pTestAosConnector->SelectIpVersion());
}

TEST_F(AosConnectorTest, SelectIpVersion_FailIfPcscfAddressIsEmpty)
{
    m_objPcscfs.AddElement(AString(""));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_FALSE(m_pTestAosConnector->SelectIpVersion());
}

TEST_F(AosConnectorTest, SelectIpVersion_FailIfInvalidPcscfAddress)
{
    m_objPcscfs.AddElement(AString("PcscfAddress"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_FALSE(m_pTestAosConnector->SelectIpVersion());
}

TEST_F(AosConnectorTest, SelectIpVersion_FailIfAddressVersionNotMatch)
{
    m_objPcscfs.AddElement(AString("fe80::1"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_FALSE(m_pTestAosConnector->SelectIpVersion());
}

TEST_F(AosConnectorTest, Notify_IgnoredIfNullListener)
{
    m_pTestAosConnector->SetListener(IMS_NULL);

    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pTestAosConnector->Notify(AosConnector::LISTENER_TYPE_ACTIVATED);
}

TEST_F(AosConnectorTest, Notify_IgnoredIfUnknownListenerType)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);

    IMS_SINT32 nUnknownListenerType = -1;
    m_pTestAosConnector->Notify(nUnknownListenerType);
}

TEST_F(AosConnectorTest, CleanUp)
{
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pTestAosConnector)));
    EXPECT_CALL(m_objMockIAosPcscf, SetListener(IMS_NULL));
    EXPECT_CALL(m_objMockIAosConnection, RemoveListener(_));

    m_pTestAosConnector->CleanUp();

    EXPECT_EQ(m_pTestAosConnector->m_nPendingFeature, AosConnector::PENDING_NONE);
    EXPECT_EQ(m_pTestAosConnector->m_nState, AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, InvalidTimer_Ignored)
{
    IMS_SINT32 nInvalidTimer = -1;
    m_pTestAosConnector->StartTimer(nInvalidTimer, TIMER_DURATION_MILLIS);
    EXPECT_FALSE(m_pTestAosConnector->IsTimerRunning(nInvalidTimer));
    m_pTestAosConnector->NotifyTimerExpired(nInvalidTimer);
    m_pTestAosConnector->StopTimer(nInvalidTimer);
}