/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "PlatformContext.h"

#include "CarrierConfig.h"
#include "private/ConfigurationManager.h"
#include "private/SipConfig.h"

#include "Sip.h"
#include "SipSocket.h"
#include "SipSocketAddress.h"

#include "MockINetworkConnection.h"
#include "MockISocket.h"
#include "TestConfigService.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::StrEq;

namespace android
{

class TestSipSocket : public SipSocket
{
public:
    TestSipSocket(IMS_SINT32 nSlotId, IMS_SINT32 nType) :
            SipSocket(nSlotId, nType)
    {
    }
    ~TestSipSocket() override = default;

    inline void SetSocketOptionForTcpMaxSegEx(
            IN const INetworkConnection* piConnection, IN const IpAddress& objLocalIp)
    {
        SetSocketOptionForTcpMaxSeg(piConnection, objLocalIp);
    }

    inline void SetSocket(ISocket* piSocket) { m_piSocket = piSocket; }
};

class SipSocketTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_piConfigService = new TestConfigService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_piConfigService);

        ConfigurationManager::GetInstance()->Initialize();
    }

    void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        if (m_piConfigService != IMS_NULL)
        {
            delete m_piConfigService;
        }
    }

protected:
    TestConfigService* m_piConfigService;
};

TEST_F(SipSocketTest, SetSocketOptionForTcpMaxSeg_IPv6_WfcEnabled)
{
    // WFC enabled
    ON_CALL(m_piConfigService->GetMockCarrierConfig(),
            GetBoolean(StrEq(CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL), _))
            .WillByDefault(Return(IMS_TRUE));

    TestSipSocket objSipSocket(IMS_SLOT_0, SipSocketAddress::SOCKET_TCP_CLIENT);
    MockISocket objMockSocket;
    objSipSocket.SetSocket(&objMockSocket);

    MockINetworkConnection objMockConnection;
    IpAddress objLocalIp("2001:db8::1");
    ON_CALL(objMockConnection, GetMtu()).WillByDefault(Return(Sip::MTU_IPV4));

    // Overhead: 60 (ESP) + 20 (TCP) + 20 (BUFFER) + 40 (IPv6) + 100 (WFC) = 240
    // MSS: 1280 - 240 = 1040
    IMS_SINT32 nOverhead = Sip::PACKET_OVERHEAD_ESP;
    nOverhead += Sip::PACKET_OVERHEAD_TCP;
    nOverhead += Sip::PACKET_EXTRA_BUFFER;
    nOverhead += Sip::PACKET_OVERHEAD_IPV6;
    nOverhead += Sip::PACKET_OVERHEAD_EPDG;
    IMS_SINT32 nMss = Sip::MTU_IPV6 - nOverhead;
    EXPECT_CALL(objMockSocket, SetOption(ISocket::OPT_TCP_MAXSEG, Eq(nMss)))
            .WillOnce(Return(IMS_TRUE));

    objSipSocket.SetSocketOptionForTcpMaxSegEx(&objMockConnection, objLocalIp);
    objSipSocket.SetSocket(IMS_NULL);
}

TEST_F(SipSocketTest, SetSocketOptionForTcpMaxSeg_IPv4_WfcDisabled)
{
    TestSipSocket objSipSocket(IMS_SLOT_0, SipSocketAddress::SOCKET_TCP_CLIENT);
    MockISocket objMockSocket;
    objSipSocket.SetSocket(&objMockSocket);

    MockINetworkConnection objMockConnection;
    IpAddress objLocalIp("192.168.1.1");
    IMS_SINT32 nNetworkMtu = Sip::MTU_IPV4;
    ON_CALL(objMockConnection, GetMtu()).WillByDefault(Return(nNetworkMtu));

    // Overhead: 60 (ESP) + 20 (TCP) + 20 (BUFFER) + 20 (IPv4) + 0 (WFC) = 120
    // MSS: 1500 - 120 = 1380
    IMS_SINT32 nOverhead = Sip::PACKET_OVERHEAD_ESP;
    nOverhead += Sip::PACKET_OVERHEAD_TCP;
    nOverhead += Sip::PACKET_EXTRA_BUFFER;
    nOverhead += Sip::PACKET_OVERHEAD_IPV4;
    IMS_SINT32 nMss = nNetworkMtu - nOverhead;

    EXPECT_CALL(objMockSocket, SetOption(ISocket::OPT_TCP_MAXSEG, Eq(nMss)))
            .WillOnce(Return(IMS_TRUE));

    objSipSocket.SetSocketOptionForTcpMaxSegEx(&objMockConnection, objLocalIp);
    objSipSocket.SetSocket(IMS_NULL);
}

TEST_F(SipSocketTest, SetSocketOptionForTcpMaxSeg_WithMaxAllowedNetworkMtu)
{
    IMS_SINT32 nMaxAllowedNetworkMtu = 1100;
    ON_CALL(m_piConfigService->GetMockCarrierConfig(),
            GetInt(StrEq(CarrierConfig::Ims::KEY_MAX_ALLOWED_NETWORK_MTU_INT), _))
            .WillByDefault(Return(nMaxAllowedNetworkMtu));
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(IMS_SLOT_0);
    ASSERT_NE(pSipConfig, nullptr);
    const_cast<SipConfig*>(pSipConfig)->Refresh();

    TestSipSocket objSipSocket(IMS_SLOT_0, SipSocketAddress::SOCKET_TCP_CLIENT);
    MockISocket objMockSocket;
    objSipSocket.SetSocket(&objMockSocket);

    MockINetworkConnection objMockConnection;
    IpAddress objLocalIp("192.168.1.1");
    ON_CALL(objMockConnection, GetMtu()).WillByDefault(Return(Sip::MTU_IPV4));

    // MaxAllowedNetworkMtu: 1100, NetworkMtu: 1500
    // MSS: 1100 (since 1100 < 1500)
    // Overhead: 60 (ESP) + 20 (TCP) + 20 (BUFFER) + 20 (IPv4) + 0 (WFC) = 120
    // Final MSS: 1100 - 120 = 980
    IMS_SINT32 nOverhead = Sip::PACKET_OVERHEAD_ESP;
    nOverhead += Sip::PACKET_OVERHEAD_TCP;
    nOverhead += Sip::PACKET_EXTRA_BUFFER;
    nOverhead += Sip::PACKET_OVERHEAD_IPV4;
    IMS_SINT32 nMss = nMaxAllowedNetworkMtu - nOverhead;

    EXPECT_CALL(objMockSocket, SetOption(ISocket::OPT_TCP_MAXSEG, Eq(nMss)))
            .WillOnce(Return(IMS_TRUE));

    objSipSocket.SetSocketOptionForTcpMaxSegEx(&objMockConnection, objLocalIp);
    objSipSocket.SetSocket(IMS_NULL);
}

}  // namespace android
