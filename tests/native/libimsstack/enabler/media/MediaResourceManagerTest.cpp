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
#include <IJniMedia.h>
#include <MediaResourceManager.h>
#include <MockMediaNetworkConnectionWatcher.h>
#include <config/MockMediaConfiguration.h>

const IMS_UINT32 RTP_PORT_START = 10000;
const IMS_UINT32 RTP_PORT_END = 20000;
const AString LOCAL_IP = "127.0.0.1";

using ::testing::Return;

class FakeMediaResourceManager : public MediaResourceManager
{
public:
    void SetNetworkType(IN IMS_SINT32 nNetworkType) { m_nNetworkType = nNetworkType; }

    void SetMtu(IN IMS_SINT32 nMtu) { m_nMtu = nMtu; }

    void SetIsIpv6(IN IMS_BOOL bIsIpv6) { m_bIsIpv6 = bIsIpv6; }

    void SetMediaNetworkConnectionWatcher(
            IN MediaNetworkConnectionWatcher* pNetworkConnectionWatcher)
    {
        m_pNetworkConnectionWatcher = pNetworkConnectionWatcher;
    }
};

class MediaResourceManagerTest : public ::testing::Test
{
public:
    FakeMediaResourceManager* m_pManager;
    MockMediaConfiguration* m_pMediaConfiguration;

protected:
    virtual void SetUp() override
    {
        m_pManager = new FakeMediaResourceManager();
        m_pMediaConfiguration = new MockMediaConfiguration(MEDIA_TYPE_AUDIO);
    }

    virtual void TearDown() override
    {
        if (m_pManager != IMS_NULL)
        {
            delete m_pManager;
        }

        if (m_pMediaConfiguration != IMS_NULL)
        {
            delete m_pMediaConfiguration;
        }
    }
};

TEST_F(MediaResourceManagerTest, testAcquireReleasePort)
{
    ON_CALL(*m_pMediaConfiguration, GetPortRtp()).WillByDefault(Return(RTP_PORT_START));
    ON_CALL(*m_pMediaConfiguration, GetPortRtpEnd()).WillByDefault(Return(RTP_PORT_END));

    IMS_UINT32 nPort = m_pManager->AcquireRtpPort(m_pMediaConfiguration);
    EXPECT_TRUE((nPort >= RTP_PORT_START) && (nPort <= RTP_PORT_END));

    EXPECT_EQ(m_pManager->ReleaseRtpPort(nPort), IMS_TRUE);
    EXPECT_EQ(m_pManager->ReleaseRtpPort(30000), IMS_FALSE);
}

TEST_F(MediaResourceManagerTest, testUpdatePdn)
{
    IpAddress objIpAddr = IpAddress(LOCAL_IP);
    EXPECT_EQ(m_pManager->UpdatePdn(MediaResourceManager::PDN_IMS, objIpAddr), IMS_TRUE);
}

TEST_F(MediaResourceManagerTest, testGetRtpFragmentSize)
{
    IMS_SINT32 nDefaultMtu = 1300;
    IMS_SINT32 nMtuFromConnectionWatcher = 14200;
    IMS_SINT32 MTU_MOBILE = 1500;
    IMS_SINT32 MTU_EPDG = 1280;
    IMS_SINT32 SIZE_OF_IP_SEC = 60;
    IMS_SINT32 SIZE_OF_IPV6 = 60;
    IMS_SINT32 SIZE_OF_IPV4 = 40;
    IMS_SINT32 SIZE_OF_RTP = 20 + 8;

    MockMediaNetworkConnectionWatcher* m_pWatcher =
            new MockMediaNetworkConnectionWatcher(IpAddress(LOCAL_IP));
    EXPECT_CALL(*m_pWatcher, GetMtu())
            .WillOnce(Return(nMtuFromConnectionWatcher))
            .WillOnce(Return(nMtuFromConnectionWatcher))
            .WillRepeatedly(Return(0));
    m_pManager->SetMediaNetworkConnectionWatcher(
            static_cast<MediaNetworkConnectionWatcher*>(m_pWatcher));

    m_pManager->SetMtu(nDefaultMtu);
    m_pManager->SetIsIpv6(IMS_TRUE);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            nDefaultMtu - SIZE_OF_IP_SEC - SIZE_OF_IPV6 - SIZE_OF_RTP);

    m_pManager->SetIsIpv6(IMS_FALSE);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            nDefaultMtu - SIZE_OF_IP_SEC - SIZE_OF_IPV4 - SIZE_OF_RTP);

    m_pManager->SetMtu(0);
    m_pManager->SetIsIpv6(IMS_TRUE);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            nMtuFromConnectionWatcher - SIZE_OF_IP_SEC - SIZE_OF_IPV6 - SIZE_OF_RTP);

    m_pManager->SetIsIpv6(IMS_FALSE);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            nMtuFromConnectionWatcher - SIZE_OF_IP_SEC - SIZE_OF_IPV4 - SIZE_OF_RTP);

    m_pManager->SetIsIpv6(IMS_TRUE);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            MTU_MOBILE - SIZE_OF_IP_SEC - SIZE_OF_IPV6 - SIZE_OF_RTP);

    m_pManager->SetNetworkType(MediaNetworkConnectionWatcher::IWLAN);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            MTU_EPDG - SIZE_OF_IP_SEC - SIZE_OF_IPV6 - SIZE_OF_RTP);

    m_pManager->SetIsIpv6(IMS_FALSE);
    EXPECT_EQ(m_pManager->GetRtpFragmentSize(),
            MTU_EPDG - SIZE_OF_IP_SEC - SIZE_OF_IPV4 - SIZE_OF_RTP);
}
