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

#include "INetworkPing.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "condition/AosServiceAvailableWifi.h"
#include "provider/AosProvider.h"

#include "../../../platform/interface/MockIPhoneInfoLocation.h"
#include "../../../platform/interface/MockIWifiWatcher.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosNConfiguration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrEq;

#define DECLARE_USING(Base)                     \
    using Base::WifiWatcher_NotifyStateChanged; \
    using Base::NetworkPing_NotifyResult;       \
    using Base::HandleCallStateChanged;         \
    using Base::HandleAirplaneModeChanged;      \
    using Base::HandleWifiConnectionChanged;    \
    using Base::HandleLocationInfoChanged;      \
    using Base::ProcessBadConnectionReported;   \
    using Base::RequestNetPing;                 \
    using Base::PingResultToString;

class TestAosServiceAvailableWifi : public AosServiceAvailableWifi
{
public:
    DECLARE_USING(AosServiceAvailableWifi)

    inline explicit TestAosServiceAvailableWifi() :
            AosServiceAvailableWifi()
    {
    }

    inline void SetBlock(IN IAosBlock* piBlock) { m_piBlock = piBlock; }

    inline void SetCallTracker(IN IAosCallTracker* pIAosCallTracker)
    {
        m_piCallTracker = pIAosCallTracker;
    }

    inline void SetRegistration(IN IAosRegistration* pIAosRegistration)
    {
        m_piRegistration = pIAosRegistration;
    }

    inline void SetConnection(IN IAosConnection* pIAosConnection)
    {
        m_piConnection = pIAosConnection;
    }

    inline void SetCountry(IN const AString& strCountry) { m_strCountry = strCountry; }

    inline IMS_UINT32 GetBadNetworkState() { return m_nBadNetworkState; }

    inline void SetBadNetworkState(IN IMS_UINT32 nState) { m_nBadNetworkState = nState; }

    inline IMS_BOOL GetWifiState() { return m_bWifiState; }

    inline void SetWifiState(IN IMS_BOOL bState) { m_bWifiState = bState; }

    inline void SetTestLocation(IN ILocationProperties* piTestLocation)
    {
        m_piTestLocation = piTestLocation;
    }
};

class AosServiceAvailableWifiTest : public ::testing::Test
{
public:
    TestAosServiceAvailableWifi* m_pServiceAvailableWifi;
    IAosNConfiguration* m_piOriginConfiguration;
    NiceMock<MockIAosCallTracker> m_objMockIAosCallTracker;
    NiceMock<MockIAosRegistration> m_objMockIAosRegistration;
    NiceMock<MockIAosConnection> m_objMockIAosConnection;

protected:
    virtual void SetUp() override
    {
        m_pServiceAvailableWifi = new TestAosServiceAvailableWifi();
        ASSERT_TRUE(m_pServiceAvailableWifi != nullptr);

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();

        ON_CALL(m_objMockIAosCallTracker, IsEmergencyCallActive()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));
        m_pServiceAvailableWifi->SetCallTracker(&m_objMockIAosCallTracker);

        ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_TRUE));
        m_pServiceAvailableWifi->SetRegistration(&m_objMockIAosRegistration);

        ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));
        m_pServiceAvailableWifi->SetConnection(&m_objMockIAosConnection);

        m_pServiceAvailableWifi->SetBadNetworkState(
                TestAosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
    }

    virtual void TearDown() override
    {
        if (m_pServiceAvailableWifi)
        {
            delete m_pServiceAvailableWifi;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);
    }
};

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWithoutCallTracker)
{
    // GIVEN
    m_pServiceAvailableWifi->SetCallTracker(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWhenEmergencyCallActive)
{
    // GIVEN
    ON_CALL(m_objMockIAosCallTracker, IsEmergencyCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWithoutIRegistration)
{
    // GIVE
    m_pServiceAvailableWifi->SetRegistration(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWhenNotRegistered)
{
    // GIVEN
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWhenBadNetworkChecking)
{
    // GIVEN
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWhenBadNetworkDetected)
{
    // GIVEN
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, FailsStartToCheckNetworkConnectionWhenEpdgDisabled)
{
    // GIVEN
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, SucceedsStartToCheckNetworkConnectionWhenEpdgEnabled)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pServiceAvailableWifi->StartToCheckNetworkConnection();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkNone)
{
    m_pServiceAvailableWifi->SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
    EXPECT_FALSE(m_pServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkDetected)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
    EXPECT_TRUE(m_pServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkChecking)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    EXPECT_FALSE(m_pServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_IWifiWatcherIsNull)
{
    // Test : IWifiWatcher is NULL
    m_pServiceAvailableWifi->SetWifiState(IMS_TRUE);
    m_pServiceAvailableWifi->WifiWatcher_NotifyStateChanged(IMS_NULL);
    EXPECT_TRUE(m_pServiceAvailableWifi->GetWifiState());

    m_pServiceAvailableWifi->SetWifiState(IMS_FALSE);
    m_pServiceAvailableWifi->WifiWatcher_NotifyStateChanged(IMS_NULL);
    EXPECT_FALSE(m_pServiceAvailableWifi->GetWifiState());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherConnected)
{
    MockIWifiWatcher objMockIWifiWatcher;
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    // Test1 : m_bWifiState is IMS_FALSE
    m_pServiceAvailableWifi->SetWifiState(IMS_FALSE);
    m_pServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_TRUE(m_pServiceAvailableWifi->GetWifiState());

    // Test2 : m_bWifiState is IMS_TRUE
    m_pServiceAvailableWifi->SetWifiState(IMS_TRUE);
    m_pServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_TRUE(m_pServiceAvailableWifi->GetWifiState());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherDisconnected)
{
    MockIWifiWatcher objMockIWifiWatcher;
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    // Test1 : m_bWifiState is IMS_TRUE
    m_pServiceAvailableWifi->SetWifiState(IMS_TRUE);
    m_pServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_FALSE(m_pServiceAvailableWifi->GetWifiState());

    // Test2 : m_bWifiState is IMS_FALSE
    m_pServiceAvailableWifi->SetWifiState(IMS_FALSE);
    m_pServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_FALSE(m_pServiceAvailableWifi->GetWifiState());
}

TEST_F(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateDeadPeer)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    m_pServiceAvailableWifi->NetworkPing_NotifyResult(
            IMS_NULL, INetworkPing::PING_STATUS_DEAD_PEER);

    EXPECT_EQ(m_pServiceAvailableWifi->GetBadNetworkState(),
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateTimedout)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    m_pServiceAvailableWifi->NetworkPing_NotifyResult(IMS_NULL, INetworkPing::PING_STATUS_TIMEDOUT);

    EXPECT_EQ(m_pServiceAvailableWifi->GetBadNetworkState(),
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeNormal_Idle)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    m_pServiceAvailableWifi->HandleCallStateChanged(
            IAosCallTracker::TYPE_NORMAL, static_cast<IMS_SINT32>(CallState::IDLE));
    EXPECT_EQ(m_pServiceAvailableWifi->GetBadNetworkState(),
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeCs_Idle)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    m_pServiceAvailableWifi->HandleCallStateChanged(
            IAosCallTracker::TYPE_CS, static_cast<IMS_SINT32>(CallState::IDLE));
    EXPECT_NE(m_pServiceAvailableWifi->GetBadNetworkState(),
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeEmergency_Idle)
{
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    m_pServiceAvailableWifi->HandleCallStateChanged(
            IAosCallTracker::TYPE_EMERGENCY, static_cast<IMS_SINT32>(CallState::IDLE));
    EXPECT_NE(m_pServiceAvailableWifi->GetBadNetworkState(),
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleAirplaneModeChanged_ReturnByConfig)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsRequiredWfcBlockByAirplaneMode())
            .WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock);

    m_pServiceAvailableWifi->HandleAirplaneModeChanged(0);
    m_pServiceAvailableWifi->HandleAirplaneModeChanged(1);
}

TEST_F(AosServiceAvailableWifiTest, HandleAirplaneModeChanged_AirplaneModeTrue)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsRequiredWfcBlockByAirplaneMode())
            .WillRepeatedly(Return(IMS_TRUE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock);

    m_pServiceAvailableWifi->HandleAirplaneModeChanged(1);
}

TEST_F(AosServiceAvailableWifiTest, HandleAirplaneModeChanged_AirplaneModeFalse)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsRequiredWfcBlockByAirplaneMode())
            .WillRepeatedly(Return(IMS_TRUE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock);

    m_pServiceAvailableWifi->HandleAirplaneModeChanged(0);
}

TEST_F(AosServiceAvailableWifiTest, HandleWifiConnectionChanged)
{
    // Test1 : WifiState is IMS_FALSE, BadNetworkState is STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock1;
    m_pServiceAvailableWifi->SetWifiState(IMS_FALSE);
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);

    EXPECT_CALL(objMockIAosBlock1, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock1);

    m_pServiceAvailableWifi->HandleWifiConnectionChanged();

    // Test2 : WifiState is IMS_TRUE, BadNetworkState is not STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock2;
    m_pServiceAvailableWifi->SetWifiState(IMS_TRUE);
    m_pServiceAvailableWifi->SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    EXPECT_CALL(objMockIAosBlock2, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock2);

    m_pServiceAvailableWifi->HandleWifiConnectionChanged();

    // Test3 : WifiState is IMS_FALSE, BadNetworkState is not STATE_BAD_NETWORK_DETECTED
    MockIAosBlock objMockIAosBlock3;
    m_pServiceAvailableWifi->SetWifiState(IMS_FALSE);
    m_pServiceAvailableWifi->SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    EXPECT_CALL(objMockIAosBlock3, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock3);

    m_pServiceAvailableWifi->HandleWifiConnectionChanged();

    // Test4 : WifiState is IMS_TRUE, BadNetworkState is STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock4;
    m_pServiceAvailableWifi->SetWifiState(IMS_TRUE);
    m_pServiceAvailableWifi->SetBadNetworkState(
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);

    EXPECT_CALL(objMockIAosBlock4, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock4);

    m_pServiceAvailableWifi->HandleWifiConnectionChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_TestLocationIsNull)
{
    m_pServiceAvailableWifi->SetTestLocation(IMS_NULL);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);
    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock);

    m_pServiceAvailableWifi->HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountryNotSame)
{
    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pServiceAvailableWifi->SetTestLocation(&objMockILocationProperties);

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock);

    // Set Country
    AString strCountry = AString("test_country");
    m_pServiceAvailableWifi->SetCountry(strCountry);

    m_pServiceAvailableWifi->HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountrySame)
{
    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pServiceAvailableWifi->SetTestLocation(&objMockILocationProperties);

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pServiceAvailableWifi->SetBlock(&objMockIAosBlock);

    // Set Country
    AString strCountry = AString("test_country");
    m_pServiceAvailableWifi->SetCountry(strCountry);

    m_pServiceAvailableWifi->HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, ProcessBadConnectionReported)
{
    m_pServiceAvailableWifi->SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    m_pServiceAvailableWifi->ProcessBadConnectionReported();
    EXPECT_EQ(m_pServiceAvailableWifi->GetBadNetworkState(),
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, PingResultToString)
{
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_OK),
            StrEq("PING_STATUS_OK"));
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_PENDING),
            StrEq("PING_STATUS_PENDING"));
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_NOK),
            StrEq("PING_STATUS_NOK"));
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_DEAD_PEER),
            StrEq("PING_STATUS_DEAD_PEER"));
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_TIMEDOUT),
            StrEq("PING_STATUS_TIMEDOUT"));
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(-1), StrEq("__INVALID__"));
    EXPECT_THAT(m_pServiceAvailableWifi->PingResultToString(5), StrEq("__INVALID__"));
}
