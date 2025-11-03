/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "IMediaNetworkConnectionListener.h"
#include "MediaNetworkConnectionWatcher.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "PlatformContext.h"

#include "MockINetworkConnection.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;

namespace
{

// Mock for IMediaNetworkConnectionListener
class MockMediaNetworkConnectionListener : public IMediaNetworkConnectionListener
{
public:
    MOCK_METHOD(void, OnNetworkConnectionChanged, (IN const IMS_SINT32 nRatType), (override));
    MOCK_METHOD(void, OnMediaMtuChanged, (IN const IMS_UINT32 nMtu), (override));
};

// Mock for NetworkService
class MockNetworkService : public NetworkService
{
public:
    MOCK_METHOD(INetworkConnection*, FindConnection, (IN const IpAddress& objIpAddr), (override));
    MOCK_METHOD(INetworkConnection*, FindConnection,
            (IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId), (override));
};
}  // namespace

class MediaNetworkConnectionWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_pMockNetworkService = new NiceMock<MockNetworkService>();
        m_pMockNetworkConnection = new NiceMock<MockINetworkConnection>();
        m_pMockListener = new NiceMock<MockMediaNetworkConnectionListener>();

        // Replace the global NetworkService instance with our mock
        m_pOriginalNetworkService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, m_pMockNetworkService);
    }

    void TearDown() override
    {
        // Restore the original NetworkService instance
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, m_pOriginalNetworkService);

        delete m_pMockNetworkConnection;
        delete m_pMockListener;
        // m_pMockNetworkService is deleted by PlatformContext when service is replaced.
    }

    NiceMock<MockNetworkService>* m_pMockNetworkService;
    NiceMock<MockINetworkConnection>* m_pMockNetworkConnection;
    NiceMock<MockMediaNetworkConnectionListener>* m_pMockListener;
    PlatformService* m_pOriginalNetworkService;
};

TEST_F(MediaNetworkConnectionWatcherTest, ConstructorFindConnectionByIpSuccess)
{
    IpAddress testIp("192.168.1.1");
    EXPECT_CALL(*m_pMockNetworkService, FindConnection(testIp))
            .WillOnce(Return(m_pMockNetworkConnection));
    EXPECT_CALL(*m_pMockNetworkService, FindConnection(_, _)).Times(0);
    EXPECT_CALL(*m_pMockNetworkConnection, AddReferenceListener(_)).Times(1);

    MediaNetworkConnectionWatcher objWatcher(testIp);
}

TEST_F(MediaNetworkConnectionWatcherTest, ConstructorFindConnectionFail)
{
    IpAddress testIp("192.168.1.1");
    EXPECT_CALL(*m_pMockNetworkService, FindConnection(testIp)).WillOnce(Return(nullptr));
    EXPECT_CALL(*m_pMockNetworkService, FindConnection(NetworkPolicy::APN_IMS, 0))
            .WillOnce(Return(nullptr));
    EXPECT_CALL(*m_pMockNetworkConnection, AddReferenceListener(_)).Times(0);

    MediaNetworkConnectionWatcher objWatcher(testIp);
    EXPECT_EQ(objWatcher.GetNetworkType(), MediaNetworkConnectionWatcher::UNKNOWN);
    EXPECT_EQ(objWatcher.GetMtu(), 0);
}

TEST_F(MediaNetworkConnectionWatcherTest, OnNetworkConnectionChanged_NotifiesListener)
{
    IpAddress testIp("192.168.1.1");
    AString ratLte("LTE");
    AString ratWifi("WiFi");

    EXPECT_CALL(*m_pMockNetworkService, FindConnection(testIp))
            .WillOnce(Return(m_pMockNetworkConnection));
    EXPECT_CALL(*m_pMockNetworkConnection, GetExtraInfo(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(ratLte), Return(IMS_TRUE)))
            .WillOnce(DoAll(SetArgReferee<1>(ratWifi), Return(IMS_TRUE)));
    EXPECT_CALL(*m_pMockNetworkConnection, IsePDGEnabled()).WillRepeatedly(Return(IMS_FALSE));

    MediaNetworkConnectionWatcher objWatcher(testIp);
    objWatcher.SetListener(m_pMockListener);

    // Initial state
    EXPECT_EQ(objWatcher.GetNetworkType(), MediaNetworkConnectionWatcher::EUTRAN);

    // Expect listener to be called on change
    EXPECT_CALL(*m_pMockListener, OnNetworkConnectionChanged(MediaNetworkConnectionWatcher::IWLAN))
            .Times(1);
    objWatcher.NetworkConnection_OnIpcanChanged(m_pMockNetworkConnection);

    EXPECT_EQ(objWatcher.GetNetworkType(), MediaNetworkConnectionWatcher::IWLAN);
}

TEST_F(MediaNetworkConnectionWatcherTest, OnMtuChanged_NotifiesListener)
{
    IpAddress testIp("192.168.1.1");

    EXPECT_CALL(*m_pMockNetworkService, FindConnection(testIp))
            .WillOnce(Return(m_pMockNetworkConnection));
    EXPECT_CALL(*m_pMockNetworkConnection, GetMtu()).WillOnce(Return(1500)).WillOnce(Return(1400));

    MediaNetworkConnectionWatcher objWatcher(testIp);
    objWatcher.SetListener(m_pMockListener);

    // Initial state
    EXPECT_EQ(objWatcher.GetMtu(), 1500);

    // Expect listener to be called on change
    EXPECT_CALL(*m_pMockListener, OnMediaMtuChanged(1400)).Times(1);
    objWatcher.NetworkConnection_OnIpChanged(m_pMockNetworkConnection);

    EXPECT_EQ(objWatcher.GetMtu(), 1400);
}

TEST_F(MediaNetworkConnectionWatcherTest, NoListenerChangeOnSameValues)
{
    IpAddress testIp("192.168.1.1");
    AString ratLte("LTE");

    EXPECT_CALL(*m_pMockNetworkService, FindConnection(testIp))
            .WillOnce(Return(m_pMockNetworkConnection));
    EXPECT_CALL(*m_pMockNetworkConnection, GetExtraInfo(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(ratLte), Return(IMS_TRUE)));
    EXPECT_CALL(*m_pMockNetworkConnection, GetMtu()).WillRepeatedly(Return(1500));

    MediaNetworkConnectionWatcher objWatcher(testIp);
    objWatcher.SetListener(m_pMockListener);

    // No listener calls should happen if values don't change
    EXPECT_CALL(*m_pMockListener, OnNetworkConnectionChanged(_)).Times(0);
    EXPECT_CALL(*m_pMockListener, OnMediaMtuChanged(_)).Times(0);

    objWatcher.NetworkConnection_OnIpChanged(m_pMockNetworkConnection);
}
