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

class AosServiceAvailableWifiTest : public ::testing::Test
{
public:
    AosServiceAvailableWifi* m_pAosServiceAvailableWifi;
    IAosNConfiguration* m_piOriginConfiguration;

protected:
    virtual void SetUp() override
    {
        m_pAosServiceAvailableWifi = new AosServiceAvailableWifi();
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

    void SetCallTracker(IN IAosCallTracker* piCallTracker)
    {
        m_pAosServiceAvailableWifi->m_piCallTracker = piCallTracker;
    }

    IAosCallTracker* GetCallTracker() { return m_pAosServiceAvailableWifi->m_piCallTracker; }

    void SetRegistration(IN IAosRegistration* piRegistration)
    {
        m_pAosServiceAvailableWifi->m_piRegistration = piRegistration;
    }

    IAosRegistration* GetRegistration() { return m_pAosServiceAvailableWifi->m_piRegistration; }

    void SetConnection(IN IAosConnection* piConnection)
    {
        m_pAosServiceAvailableWifi->m_piConnection = piConnection;
    }

    IAosConnection* GetConnection() { return m_pAosServiceAvailableWifi->m_piConnection; }

    void SetBadNetworkState(IN IMS_UINT32 nState)
    {
        m_pAosServiceAvailableWifi->m_nBadNetworkState = nState;
    }

    IMS_UINT32 GetBadNetworkState() { return m_pAosServiceAvailableWifi->m_nBadNetworkState; }

    void SetWifiState(IN IMS_BOOL bState) { m_pAosServiceAvailableWifi->m_bWifiState = bState; }

    IMS_BOOL GetWifiState() { return m_pAosServiceAvailableWifi->m_bWifiState; }

    void SetCountry(IN const AString& strCountry)
    {
        m_pAosServiceAvailableWifi->m_strCountry = strCountry;
    }

    AString& GetCountry() { return m_pAosServiceAvailableWifi->m_strCountry; }

    void SetAosBlock(IN IAosBlock* piBlock) { m_pAosServiceAvailableWifi->m_piBlock = piBlock; }

    void SetTestLocation(IN ILocationProperties* piTestLocation)
    {
        m_pAosServiceAvailableWifi->m_piTestLocation = piTestLocation;
    }

    void WifiWatcher_NotifyStateChanged(IN IWifiWatcher* pIWifiWatcher)
    {
        m_pAosServiceAvailableWifi->WifiWatcher_NotifyStateChanged(pIWifiWatcher);
    }

    void NetworkPing_NotifyResult(IN INetworkPing* piPing, IN IMS_SINT32 nResult)
    {
        m_pAosServiceAvailableWifi->NetworkPing_NotifyResult(piPing, nResult);
    }

    void HandleCallStateChanged(IN IMS_UINT32 nState, IN CallState eStateEx)
    {
        m_pAosServiceAvailableWifi->HandleCallStateChanged(
                nState, static_cast<IMS_SINT32>(eStateEx));
    }

    void HandleRoamingChanged(IN IMS_UINT32 nState)
    {
        m_pAosServiceAvailableWifi->HandleRoamingChanged(nState);
    }

    void HandleAirplaneModeChanged(IN IMS_UINT32 nState)
    {
        m_pAosServiceAvailableWifi->HandleAirplaneModeChanged(nState);
    }

    void HandleWifiConnectionChanged()
    {
        m_pAosServiceAvailableWifi->HandleWifiConnectionChanged();
    }

    void ProcessBadConnectionReported()
    {
        m_pAosServiceAvailableWifi->ProcessBadConnectionReported();
    }

    const IMS_CHAR* PingResultToString(IN IMS_SINT32 nResult)
    {
        return m_pAosServiceAvailableWifi->PingResultToString(nResult);
    }

    void HandleLocationInfoChanged() { m_pAosServiceAvailableWifi->HandleLocationInfoChanged(); }
};

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_CallTrackerNull)
{
    SetCallTracker(IMS_NULL);
    EXPECT_EQ(GetCallTracker(), nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_EmergencyActive)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationNull)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    SetRegistration(IMS_NULL);
    EXPECT_EQ(GetRegistration(), nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationRegistered)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetRegistration(static_cast<IAosRegistration*>(&objMockIAosRegistration));
    EXPECT_NE(GetRegistration(), nullptr);

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_BadNetworkState)
{
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetRegistration(static_cast<IAosRegistration*>(&objMockIAosRegistration));
    EXPECT_NE(GetRegistration(), nullptr);

    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());

    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
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

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(_, _, _)).Times(AnyNumber());

    SetRegistration(static_cast<IAosRegistration*>(&objMockIAosRegistration));
    EXPECT_NE(GetRegistration(), nullptr);

    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    SetConnection(static_cast<IAosConnection*>(&objMockIAosConnection));

    EXPECT_FALSE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
    EXPECT_TRUE(m_pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkNone)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkDetected)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
    EXPECT_TRUE(m_pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkChecking)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    EXPECT_FALSE(m_pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_IWifiWatcherIsNull)
{
    // Test : IWifiWatcher is NULL
    SetWifiState(IMS_TRUE);
    WifiWatcher_NotifyStateChanged(IMS_NULL);
    EXPECT_TRUE(GetWifiState());

    SetWifiState(IMS_FALSE);
    WifiWatcher_NotifyStateChanged(IMS_NULL);
    EXPECT_FALSE(GetWifiState());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherConnected)
{
    MockIWifiWatcher objMockIWifiWatcher;
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    // Test1 : m_bWifiState is IMS_FALSE
    SetWifiState(IMS_FALSE);
    WifiWatcher_NotifyStateChanged(static_cast<IWifiWatcher*>(&objMockIWifiWatcher));

    EXPECT_TRUE(GetWifiState());

    // Test2 : m_bWifiState is IMS_TRUE
    SetWifiState(IMS_TRUE);
    WifiWatcher_NotifyStateChanged(static_cast<IWifiWatcher*>(&objMockIWifiWatcher));

    EXPECT_TRUE(GetWifiState());
}

TEST_F(AosServiceAvailableWifiTest, WifiWatcher_NotifyStateChanged_WifiWatcherDisconnected)
{
    MockIWifiWatcher objMockIWifiWatcher;
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    // Test1 : m_bWifiState is IMS_TRUE
    SetWifiState(IMS_TRUE);
    WifiWatcher_NotifyStateChanged(static_cast<IWifiWatcher*>(&objMockIWifiWatcher));

    EXPECT_FALSE(GetWifiState());

    // Test2 : m_bWifiState is IMS_FALSE
    SetWifiState(IMS_FALSE);
    WifiWatcher_NotifyStateChanged(static_cast<IWifiWatcher*>(&objMockIWifiWatcher));

    EXPECT_FALSE(GetWifiState());
}

TEST_F(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateDeadPeer)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    NetworkPing_NotifyResult(IMS_NULL, INetworkPing::PING_STATUS_DEAD_PEER);

    EXPECT_EQ(GetBadNetworkState(), AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, NetworkPing_NotifyResult_PingStateTimedout)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    NetworkPing_NotifyResult(IMS_NULL, INetworkPing::PING_STATUS_TIMEDOUT);

    EXPECT_EQ(GetBadNetworkState(), AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeNormal_Idle)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    HandleCallStateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_EQ(GetBadNetworkState(), AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeCs_Idle)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    HandleCallStateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE);
    EXPECT_NE(GetBadNetworkState(), AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
}

TEST_F(AosServiceAvailableWifiTest, HandleCallStateChanged_CallTypeEmergency_Idle)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);

    HandleCallStateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_NE(GetBadNetworkState(), AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
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

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    HandleAirplaneModeChanged(0);
    HandleAirplaneModeChanged(1);
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

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    HandleAirplaneModeChanged(1);
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

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    HandleAirplaneModeChanged(0);
}

TEST_F(AosServiceAvailableWifiTest, HandleWifiConnectionChanged)
{
    // Test1 : WifiState is IMS_FALSE, BadNetworkState is STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock1;
    SetWifiState(IMS_FALSE);
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);

    EXPECT_CALL(objMockIAosBlock1, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock1));

    HandleWifiConnectionChanged();

    // Test2 : WifiState is IMS_TRUE, BadNetworkState is not STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock2;
    SetWifiState(IMS_TRUE);
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    EXPECT_CALL(objMockIAosBlock2, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock2));

    HandleWifiConnectionChanged();

    // Test3 : WifiState is IMS_FALSE, BadNetworkState is not STATE_BAD_NETWORK_DETECTED
    MockIAosBlock objMockIAosBlock3;
    SetWifiState(IMS_FALSE);
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    EXPECT_CALL(objMockIAosBlock3, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock3));

    HandleWifiConnectionChanged();

    // Test4 : WifiState is IMS_TRUE, BadNetworkState is STATE_BAD_NETWORK_DETECTED.
    MockIAosBlock objMockIAosBlock4;
    SetWifiState(IMS_TRUE);
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);

    EXPECT_CALL(objMockIAosBlock4, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _)).Times(1);
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock4));

    HandleWifiConnectionChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_TestLocationIsNull)
{
    SetTestLocation(IMS_NULL);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountryNotSame)
{
    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    SetTestLocation(static_cast<ILocationProperties*>(&objMockILocationProperties));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    // Set Country
    AString strCountry = AString("test_country");
    SetCountry(strCountry);

    HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, HandleLocationInfoChanged_CountrySame)
{
    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    SetTestLocation(static_cast<ILocationProperties*>(&objMockILocationProperties));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    // Set Country
    AString strCountry = AString("test_country");
    SetCountry(strCountry);

    HandleLocationInfoChanged();
}

TEST_F(AosServiceAvailableWifiTest, ProcessBadConnectionReported)
{
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    ProcessBadConnectionReported();
    EXPECT_EQ(GetBadNetworkState(), AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
}

TEST_F(AosServiceAvailableWifiTest, PingResultToString)
{
    EXPECT_THAT(PingResultToString(INetworkPing::PING_STATUS_OK), StrEq("PING_STATUS_OK"));
    EXPECT_THAT(
            PingResultToString(INetworkPing::PING_STATUS_PENDING), StrEq("PING_STATUS_PENDING"));
    EXPECT_THAT(PingResultToString(INetworkPing::PING_STATUS_NOK), StrEq("PING_STATUS_NOK"));
    EXPECT_THAT(PingResultToString(INetworkPing::PING_STATUS_DEAD_PEER),
            StrEq("PING_STATUS_DEAD_PEER"));
    EXPECT_THAT(
            PingResultToString(INetworkPing::PING_STATUS_TIMEDOUT), StrEq("PING_STATUS_TIMEDOUT"));
    EXPECT_THAT(PingResultToString(-1), StrEq("__INVALID__"));
    EXPECT_THAT(PingResultToString(5), StrEq("__INVALID__"));
}