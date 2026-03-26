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

#include "AString.h"
#include "IpAddress.h"
#include "PlatformContext.h"

#include "CarrierConfig.h"
#include "private/ConfigurationManager.h"
#include "private/SipConfig.h"

#include "msg/SipHeaderBase.h"
#include "msg/SipMessage.h"

#include "SipClientTransport.h"
#include "SipProfile.h"
#include "SipStack.h"

#include "MockINetworkConnection.h"
#include "TestConfigService.h"
#include "TestNetworkService.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::Unused;

namespace android
{

class SipClientTransportTest : public ::testing::Test
{
public:
    inline SipClientTransportTest() :
            m_pTransport(IMS_NULL)
    {
    }
    ~SipClientTransportTest() override = default;

protected:
    void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &m_objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &m_objNetworkService);

        ConfigurationManager::GetInstance()->Initialize();

        m_pTransport = new SipClientTransport(IMS_SLOT_0);
    }

    void TearDown() override
    {
        if (m_pTransport != IMS_NULL)
        {
            delete m_pTransport;
            m_pTransport = IMS_NULL;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    }

protected:
    TestConfigService m_objConfigService;
    TestNetworkService m_objNetworkService;
    SipClientTransport* m_pTransport;
};

TEST_F(SipClientTransportTest, UpdateDestinationInfo)
{
    ::SipMessage* pSipMsg = new ::SipMessage(::SipMessage::REQ_TYPE);

    // "lr" and no Route header
    EXPECT_FALSE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    // "sr" and no Request-URI
    EXPECT_FALSE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_FALSE, IMS_NULL, IMS_NULL));

    // "lr" and Route header
    IMS_SINT32 nPort = 5080;
    AString strHostIp("192.168.0.1");
    AString strUri;
    strUri.Sprintf("sip:%s:%d", strHostIp.GetStr(), nPort);
    SipHeaderBase* pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    EXPECT_TRUE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_UDP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    // "sr" and Request-URI
    nPort += 1;
    strHostIp = "192.168.0.2";
    strUri.Sprintf("sip:%s:%d", strHostIp.GetStr(), nPort);
    SipStack::SetRequestLine("INVITE", strUri, pSipMsg);

    EXPECT_TRUE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_FALSE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_UDP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    // "lr" and "udp" transport parameter
    SipStack::RemoveHeader(SipHeaderBase::ROUTE, pSipMsg);
    nPort += 1;
    strHostIp = "192.168.0.3";
    strUri.Sprintf("<sip:%s:%d;transport=udp>", strHostIp.GetStr(), nPort);
    pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    EXPECT_TRUE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_UDP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    // "lr" and "tcp" transport parameter
    SipStack::RemoveHeader(SipHeaderBase::ROUTE, pSipMsg);
    nPort += 1;
    strHostIp = "192.168.0.4";
    strUri.Sprintf("<sip:%s:%d;transport=tcp>", strHostIp.GetStr(), nPort);
    pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    EXPECT_TRUE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_TCP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    // "lr" and "tls" transport parameter
    SipStack::RemoveHeader(SipHeaderBase::ROUTE, pSipMsg);
    nPort += 1;
    strHostIp = "192.168.0.5";
    strUri.Sprintf("<sip:%s:%d;transport=tls>", strHostIp.GetStr(), nPort);
    pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    EXPECT_TRUE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_TLS, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    // "lr" and "sips" URI scheme
    SipStack::RemoveHeader(SipHeaderBase::ROUTE, pSipMsg);
    nPort += 1;
    strHostIp = "192.168.0.6";
    strUri.Sprintf("sips:%s:%d", strHostIp.GetStr(), nPort);
    pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    EXPECT_TRUE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_UDP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    SipStack::FreeMessage(pSipMsg);
}

TEST_F(SipClientTransportTest, UpdateDestinationInfoWithImplicitRoute)
{
    ::SipMessage* pSipMsg = new ::SipMessage(::SipMessage::REQ_TYPE);

    IMS_SINT32 nPort = 5080;
    AString strHostIp("192.168.0.1");
    AString strUri;
    strUri.Sprintf("sip:%s:%d;transport=tcp", strHostIp.GetStr(), nPort);
    SipStack::SetRequestLine("INVITE", strUri, pSipMsg);

    SipAddrSpec* pImplicitRoute = SipStack::DecodeAddrSpec(strUri);

    EXPECT_TRUE(m_pTransport->UpdateDestinationInfo(
            pSipMsg, IMS_NULL, IMS_FALSE, pImplicitRoute, IMS_NULL));

    SipStack::FreeAddrSpec(pImplicitRoute);

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_TCP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    SipStack::FreeMessage(pSipMsg);
}

TEST_F(SipClientTransportTest, UpdateDestinationInfoWithImplicitDstAddress)
{
    ::SipMessage* pSipMsg = new ::SipMessage(::SipMessage::REQ_TYPE);

    IMS_SINT32 nPort = 5080;
    AString strHostIp("192.168.0.1");
    AString strUri;
    strUri.Sprintf("sip:%s:%d;transport=tcp", strHostIp.GetStr(), nPort);
    SipStack::SetRequestLine("INVITE", strUri, pSipMsg);

    AString strHostFqdn("test.ims.com");
    strUri.Sprintf("<sip:%s:%d;lr>", strHostFqdn.GetStr(), nPort);
    SipHeaderBase* pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    // No implicit destination address: no DNS query result
    EXPECT_FALSE(
            m_pTransport->UpdateDestinationInfo(pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, IMS_NULL));

    // Implicit destination address: non-numeric IP address (FQDN) & no DNS query result
    AString strImplicitHostIp("192.168.0.2");
    IpAddress objImplicitDstAddress(strImplicitHostIp);
    EXPECT_TRUE(m_pTransport->UpdateDestinationInfo(
            pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, &objImplicitDstAddress));

    EXPECT_EQ(strImplicitHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_UDP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    // Implicit destination address: non-numeric IP address (FQDN) & successful DNS query result
    IpAddress objDnsResolvedIpAddr("192.168.0.3");
    MockINetworkConnection& objMockConnection = m_objNetworkService.GetMockConnection();
    ON_CALL(objMockConnection, GetHostByName)
            .WillByDefault(Invoke(
                    [&](Unused, OUT ImsList<IpAddress>& objIpAddrs, Unused)
                    {
                        objIpAddrs.Append(objDnsResolvedIpAddr);
                        return 1;
                    }));

    EXPECT_TRUE(m_pTransport->UpdateDestinationInfo(
            pSipMsg, IMS_NULL, IMS_TRUE, IMS_NULL, &objImplicitDstAddress));

    EXPECT_EQ(objDnsResolvedIpAddr, m_pTransport->GetIpAddress(SipTransport::TA_FAR));
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_UDP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    SipStack::FreeMessage(pSipMsg);
}

TEST_F(SipClientTransportTest, UpdateDestinationInfo_IgnoreUdpTransportParameter)
{
    ON_CALL(m_objConfigService.GetMockCarrierConfig(),
            GetInt(StrEq(CarrierConfig::Ims::KEY_SIP_PREFERRED_TRANSPORT_INT), _))
            .WillByDefault(Return(SipConfig::TRANSPORT_TYPE_TCP));
    const SipConfig* pSipConfig = ConfigurationManager::GetInstance()->GetSipConfig(IMS_SLOT_0);
    ASSERT_NE(pSipConfig, IMS_NULL);
    const_cast<SipConfig*>(pSipConfig)->Refresh();

    ::SipMessage* pSipMsg = new ::SipMessage(::SipMessage::REQ_TYPE);

    IMS_SINT32 nPort = 5080;
    const AString strHostIp("192.168.0.1");
    AString strUri;
    strUri.Sprintf("sip:%s:%d", strHostIp.GetStr(), nPort);
    SipStack::SetRequestLine("INVITE", strUri, pSipMsg);

    strUri.Sprintf("<sip:%s:%d;lr;transport=udp>", strHostIp.GetStr(), nPort);
    SipHeaderBase* pRouteHeader = SipStack::DecodeHeader(SipHeaderBase::ROUTE, strUri);
    SipStack::SetHeader(pRouteHeader, pSipMsg);
    SipStack::FreeHeader(pRouteHeader);

    SipProfile objSipProfile;
    objSipProfile.SetSipFeatureCaps(
            ISipConfig::SIP_FEATURE_CAPS_IGNORE_UDP_TRANSPORT_PARAMETER_FOR_OUTGOING_REQUEST);

    EXPECT_TRUE(m_pTransport->UpdateDestinationInfo(
            pSipMsg, &objSipProfile, IMS_TRUE, IMS_NULL, IMS_NULL));

    EXPECT_EQ(strHostIp, m_pTransport->GetIpAddress(SipTransport::TA_FAR).ToString());
    EXPECT_EQ(nPort, m_pTransport->GetPort(SipTransport::TA_FAR));
    EXPECT_EQ(SipTransportAddress::PROTOCOL_TCP, m_pTransport->GetProtocol(SipTransport::TA_FAR));

    SipStack::FreeMessage(pSipMsg);
}

}  // namespace android
