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
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrEq;

class TestAosServiceAvailableWifi : public AosServiceAvailableWifi
{
public:
    inline explicit TestAosServiceAvailableWifi() :
            AosServiceAvailableWifi()
    {
    }

    FRIEND_TEST(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_CallTrackerNull);
    FRIEND_TEST(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_EmergencyActive);
    FRIEND_TEST(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationNull);
    FRIEND_TEST(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationRegistered);
    FRIEND_TEST(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_BadNetworkState);
    FRIEND_TEST(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_NormalCallActive);
    FRIEND_TEST(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkNone);
    FRIEND_TEST(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkDetected);
    FRIEND_TEST(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkChecking);
    FRIEND_TEST(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_IWifiWatcherIsNull);
    FRIEND_TEST(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherConnected);
    FRIEND_TEST(
            AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherDisconnected);
    FRIEND_TEST(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateDeadPeer);
    FRIEND_TEST(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateTimedout);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeNormal_Idle);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeCs_Idle);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeEmergency_Idle);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleAirplaneModeChanged_ReturnByConfig);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleAirplaneModeChanged_AirplaneModeTrue);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleAirplaneModeChanged_AirplaneModeFalse);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleWifiConnectionChanged);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleLocationInfoChanged_TestLocationIsNull);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountryNotSame);
    FRIEND_TEST(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountrySame);
    FRIEND_TEST(AosServiceAvailableWifiTest, ProcessBadConnectionReported);
    FRIEND_TEST(AosServiceAvailableWifiTest, PingResultToString);
};

class AosServiceAvailableWifiTest : public ::testing::Test
{
public:
    TestAosServiceAvailableWifi* m_pAosServiceAvailableWifi;
    IAosNConfiguration* m_piOriginConfiguration;

protected:
    virtual void SetUp() override
    {
        m_pAosServiceAvailableWifi = new TestAosServiceAvailableWifi();
        ASSERT_TRUE(m_pAosServiceAvailableWifi != nullptr);

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    }

    virtual void TearDown() override
    {
        if (m_pAosServiceAvailableWifi)
        {
            delete m_pAosServiceAvailableWifi;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);
    }
};

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_CallTrackerNull)
{
    m_pAosServiceAvailableWifi->m_piCallTracker = IMS_NULL;
    EXPECT_EQ(m_pAosServiceAvailableWifi->m_piCallTracker, nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_EmergencyActive)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosServiceAvailableWifi->m_piCallTracker = &objMockIAosCallTracker;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piCallTracker, nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationNull)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosServiceAvailableWifi->m_piCallTracker = &objMockIAosCallTracker;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piCallTracker, nullptr);

    m_pAosServiceAvailableWifi->m_piRegistration = IMS_NULL;
    EXPECT_EQ(m_pAosServiceAvailableWifi->m_piRegistration, nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationRegistered)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosServiceAvailableWifi->m_piCallTracker = &objMockIAosCallTracker;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piCallTracker, nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosServiceAvailableWifi->m_piRegistration = &objMockIAosRegistration;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piRegistration, nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_BadNetworkState)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosServiceAvailableWifi->m_piCallTracker = &objMockIAosCallTracker;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piCallTracker, nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosServiceAvailableWifi->m_piRegistration = &objMockIAosRegistration;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piRegistration, nullptr);

    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());

    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED;
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_NormalCallActive)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosServiceAvailableWifi->m_piCallTracker = &objMockIAosCallTracker;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piCallTracker, nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(_, _, _)).Times(AnyNumber());

    m_pAosServiceAvailableWifi->m_piRegistration = &objMockIAosRegistration;
    EXPECT_NE(m_pAosServiceAvailableWifi->m_piRegistration, nullptr);

    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE;

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    m_pAosServiceAvailableWifi->m_piConnection = &objMockIAosConnection;

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
    EXPECT_TRUE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkNone)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE;
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkDetected)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED;
    EXPECT_TRUE(m_pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkChecking)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_IWifiWatcherIsNull)
{
    // Test : IWifiWatcher is NULL
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_TRUE;
    m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(IMS_NULL);
    EXPECT_TRUE(m_pAosServiceAvailableWifi->m_bWifiState);

    m_pAosServiceAvailableWifi->m_bWifiState = IMS_FALSE;
    m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(IMS_NULL);
    EXPECT_FALSE(m_pAosServiceAvailableWifi->m_bWifiState);
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherConnected)
{
    MockIWifiWatcher objMockIWifiWatcher;
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    // Test1 : m_bWifiState is IMS_FALSE
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_FALSE;
    m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_TRUE(m_pAosServiceAvailableWifi->m_bWifiState);

    // Test2 : m_bWifiState is IMS_TRUE
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_TRUE;
    m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_TRUE(m_pAosServiceAvailableWifi->m_bWifiState);
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherDisconnected)
{
    MockIWifiWatcher objMockIWifiWatcher;
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    // Test1 : m_bWifiState is IMS_TRUE
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_TRUE;
    m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->m_bWifiState);

    // Test2 : m_bWifiState is IMS_FALSE
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_FALSE;
    m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(&objMockIWifiWatcher);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->m_bWifiState);
}

TEST_F(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateDeadPeer)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;
    m_pAosServiceAvailableWifi->NetworkPing_NotifyResult(
            IMS_NULL, INetworkPing::PING_STATUS_DEAD_PEER);

    EXPECT_EQ(m_pAosServiceAvailableWifi->m_nBadNetworkState,
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateTimedout)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;
    m_pAosServiceAvailableWifi->NetworkPing_NotifyResult(
            IMS_NULL, INetworkPing::PING_STATUS_TIMEDOUT);

    EXPECT_EQ(m_pAosServiceAvailableWifi->m_nBadNetworkState,
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeNormal_Idle)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;

    m_pAosServiceAvailableWifi->HandleCallStateChanged(
            IAosCallTracker::TYPE_NORMAL, static_cast<IMS_SINT32>(CallState::IDLE));
    EXPECT_EQ(m_pAosServiceAvailableWifi->m_nBadNetworkState,
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeCs_Idle)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;

    m_pAosServiceAvailableWifi->HandleCallStateChanged(
            IAosCallTracker::TYPE_CS, static_cast<IMS_SINT32>(CallState::IDLE));
    EXPECT_NE(m_pAosServiceAvailableWifi->m_nBadNetworkState,
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeEmergency_Idle)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING;

    m_pAosServiceAvailableWifi->HandleCallStateChanged(
            IAosCallTracker::TYPE_EMERGENCY, static_cast<IMS_SINT32>(CallState::IDLE));
    EXPECT_NE(m_pAosServiceAvailableWifi->m_nBadNetworkState,
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

    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableWifi->HandleAirplaneModeChanged(0);
    m_pAosServiceAvailableWifi->HandleAirplaneModeChanged(1);
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

    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableWifi->HandleAirplaneModeChanged(1);
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

    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableWifi->HandleAirplaneModeChanged(0);
}

TEST_F(AosServiceAvailableWifiTest, HandleWifiConnectionChanged)
{
    // Test1 : WifiState is IMS_FALSE, BadNetworkState is STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock1;
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_FALSE;
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED;

    EXPECT_CALL(objMockIAosBlock1, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock1;

    m_pAosServiceAvailableWifi->HandleWifiConnectionChanged();

    // Test2 : WifiState is IMS_TRUE, BadNetworkState is not STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock2;
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_TRUE;
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE;

    EXPECT_CALL(objMockIAosBlock2, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock2;

    m_pAosServiceAvailableWifi->HandleWifiConnectionChanged();

    // Test3 : WifiState is IMS_FALSE, BadNetworkState is not STATE_BAD_NETWORK_DETECTED
    MockIAosBlock objMockIAosBlock3;
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_FALSE;
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE;

    EXPECT_CALL(objMockIAosBlock3, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock3;

    m_pAosServiceAvailableWifi->HandleWifiConnectionChanged();

    // Test4 : WifiState is IMS_TRUE, BadNetworkState is STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock4;
    m_pAosServiceAvailableWifi->m_bWifiState = IMS_TRUE;
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED;

    EXPECT_CALL(objMockIAosBlock4, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock4;

    m_pAosServiceAvailableWifi->HandleWifiConnectionChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_TestLocationIsNull)
{
    m_pAosServiceAvailableWifi->m_piTestLocation = IMS_NULL;

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);
    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableWifi->HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountryNotSame)
{
    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pAosServiceAvailableWifi->m_piTestLocation = &objMockILocationProperties;

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock;

    // Set Country
    AString strCountry = AString("test_country");
    m_pAosServiceAvailableWifi->m_strCountry = strCountry;

    m_pAosServiceAvailableWifi->HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountrySame)
{
    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pAosServiceAvailableWifi->m_piTestLocation = &objMockILocationProperties;

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosServiceAvailableWifi->m_piBlock = &objMockIAosBlock;

    // Set Country
    AString strCountry = AString("test_country");
    m_pAosServiceAvailableWifi->m_strCountry = strCountry;

    m_pAosServiceAvailableWifi->HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, ProcessBadConnectionReported)
{
    m_pAosServiceAvailableWifi->m_nBadNetworkState =
            AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE;

    m_pAosServiceAvailableWifi->ProcessBadConnectionReported();
    EXPECT_EQ(m_pAosServiceAvailableWifi->m_nBadNetworkState,
            AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, PingResultToString)
{
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_OK),
            StrEq("PING_STATUS_OK"));
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_PENDING),
            StrEq("PING_STATUS_PENDING"));
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_NOK),
            StrEq("PING_STATUS_NOK"));
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_DEAD_PEER),
            StrEq("PING_STATUS_DEAD_PEER"));
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(INetworkPing::PING_STATUS_TIMEDOUT),
            StrEq("PING_STATUS_TIMEDOUT"));
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(-1), StrEq("__INVALID__"));
    EXPECT_THAT(m_pAosServiceAvailableWifi->PingResultToString(5), StrEq("__INVALID__"));
}
