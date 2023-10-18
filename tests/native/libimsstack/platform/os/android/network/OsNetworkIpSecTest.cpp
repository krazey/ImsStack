/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "ImsIpSecType.h"
#include "MockISystem.h"
#include "MockISocket.h"
#include "PlatformContext.h"
#include "TestNetworkService.h"
#include "TestThreadService.h"
#include "network/OsIpSecPolicy.h"
#include "network/OsIpSecSa.h"
#include "network/OsIpSecSp.h"
#include "network/OsNetworkIpSec.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{

class OsNetworkIpSecTest : public ::testing::Test
{
protected:
    inline OsNetworkIpSecTest() :
            m_nSpi(0),
            m_nMode(0),
            m_objLocalAddress(AString("192.168.2.3"), 5060),
            m_objRemoteAddress(AString("192.168.5.9"), 5080)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_nSpi = 21;
        m_nMode = IpSecSaParameter::Policy::MODE_TUNNEL;

        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetSystem(&m_objSystem);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &m_objNetworkService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }

    void SetIpsecSpTransport(IIpSecSp* pIpSecSp)
    {
        IMS_UINT32 nTransportProtocol = IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP;
        IMS_UINT32 nAction = IpSecType::ACTION_APPLY;
        IMS_UINT32 nDirection = IpSecSaParameter::Policy::DIRECTION_OUT;

        static_cast<OsIpSecSp*>(pIpSecSp)->SetTransportInfo(m_objLocalAddress.GetAddress(),
                m_objLocalAddress.GetPort(), m_objRemoteAddress.GetAddress(),
                m_objRemoteAddress.GetPort(), nTransportProtocol, nAction, nDirection, m_nSpi,
                m_nMode);
    }

    void SetIpsecSa(IIpSecSa* pIpSecSa)
    {
        IMS_UINT32 nSecurityProtocol = IpSecSaParameter::SECURITY_PROTOCOL_ESP;
        IMS_UINT32 nAuthAlgorithm = IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96;
        IMS_UINT32 nEncryptionAlgorithm = IpSecSaParameter::ENCRYPTION_ALG_AES_CBC;
        ByteArray objAuthKey(AString("12345"));
        ByteArray objEncryptionKey(AString("67890"));

        static_cast<OsIpSecSa*>(pIpSecSa)->SetSa(m_objLocalAddress.GetAddress(),
                m_objLocalAddress.GetPort(), m_objRemoteAddress.GetAddress(),
                m_objRemoteAddress.GetPort(), nSecurityProtocol, m_nSpi, m_nMode, nAuthAlgorithm,
                nEncryptionAlgorithm, objAuthKey, objEncryptionKey);
    }

protected:
    MockISystem m_objSystem;
    MockIThread m_objMockThread;
    TestNetworkService m_objNetworkService;
    TestThreadService m_objThreadService;
    IMS_UINT32 m_nSpi;
    IMS_UINT32 m_nMode;
    SocketAddress m_objLocalAddress;
    SocketAddress m_objRemoteAddress;
};

TEST_F(OsNetworkIpSecTest, CreatePolicy)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);
}

TEST_F(OsNetworkIpSecTest, DestroyPolicy)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IMS_SINT32 nId = pIpSecPolicy->GetId();

    pIpSecPolicy = objOsNetworkIpSec.GetPolicy(nId);
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    objOsNetworkIpSec.DestroyPolicy(pIpSecPolicy);

    pIpSecPolicy = objOsNetworkIpSec.GetPolicy(nId);
    ASSERT_TRUE(pIpSecPolicy == nullptr);
}

TEST_F(OsNetworkIpSecTest, DestroyAllPolicies)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IMS_SINT32 nId1 = pIpSecPolicy->GetId();
    pIpSecPolicy = nullptr;

    pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IMS_SINT32 nId2 = pIpSecPolicy->GetId();

    pIpSecPolicy = objOsNetworkIpSec.GetPolicy(nId1);
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    pIpSecPolicy = objOsNetworkIpSec.GetPolicy(nId2);
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    EXPECT_CALL(m_objSystem, RemoveIpSecSaParameter(_, _)).Times(0);
    objOsNetworkIpSec.DestroyAllPolicies();

    pIpSecPolicy = objOsNetworkIpSec.GetPolicy(nId1);
    ASSERT_TRUE(pIpSecPolicy == nullptr);

    pIpSecPolicy = objOsNetworkIpSec.GetPolicy(nId2);
    ASSERT_TRUE(pIpSecPolicy == nullptr);
}

TEST_F(OsNetworkIpSecTest, AddPolicy)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IIpSecSp* pIpSecSp = pIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    IIpSecSa* pIpSecSa = pIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    EXPECT_CALL(m_objSystem, AddIpSecSaParameter(_, _)).Times(1).WillRepeatedly(Return(1));
    objOsNetworkIpSec.AddPolicy(pIpSecPolicy);

    EXPECT_CALL(m_objSystem, RemoveIpSecSaParameter(_, _)).Times(1);
    objOsNetworkIpSec.DestroyAllPolicies();
    objOsNetworkIpSec.DestroyAllPolicies();
}

TEST_F(OsNetworkIpSecTest, DeletePolicy)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IIpSecSp* pIpSecSp = pIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    IIpSecSa* pIpSecSa = pIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    EXPECT_CALL(m_objSystem, AddIpSecSaParameter(_, _)).Times(1).WillRepeatedly(Return(1));
    objOsNetworkIpSec.AddPolicy(pIpSecPolicy);

    EXPECT_CALL(m_objSystem, RemoveIpSecSaParameter(_, _)).Times(1);
    objOsNetworkIpSec.DeletePolicy(pIpSecPolicy);

    // Already policy deleted, should not call RemoveIpSecSaParameter
    EXPECT_CALL(m_objSystem, RemoveIpSecSaParameter(_, _)).Times(0);
    objOsNetworkIpSec.DeletePolicy(pIpSecPolicy);

    objOsNetworkIpSec.DestroyAllPolicies();
}

TEST_F(OsNetworkIpSecTest, ApplyIpSecTransform)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IIpSecSp* pIpSecSp = pIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    SetIpsecSpTransport(pIpSecSp);

    IIpSecSa* pIpSecSa = pIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    SetIpsecSa(pIpSecSa);

    EXPECT_CALL(m_objSystem, AddIpSecSaParameter(_, _)).Times(1).WillRepeatedly(Return(1));

    objOsNetworkIpSec.AddPolicy(pIpSecPolicy);

    MockISocket objMockSocket;

    EXPECT_CALL(objMockSocket, GetSocketType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::TYPE_STREAM));
    EXPECT_CALL(objMockSocket, GetSocketId()).Times(AnyNumber()).WillRepeatedly(Return(1));

    // One policy present
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    EXPECT_EQ(IMS_TRUE,
            objOsNetworkIpSec.ApplyIpSecTransform(
                    &objMockSocket, m_objLocalAddress, &m_objRemoteAddress));

    // Policy already applied and socket id set, should not re-apply
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(0);
    objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, m_objLocalAddress);

    // delete the applied policy
    EXPECT_CALL(m_objSystem, RemoveIpSecSaParameter(_, _)).Times(1);
    objOsNetworkIpSec.DeletePolicy(pIpSecPolicy);

    EXPECT_CALL(m_objSystem, AddIpSecSaParameter(_, _)).Times(1).WillRepeatedly(Return(1));
    objOsNetworkIpSec.AddPolicy(pIpSecPolicy);

    // ApplyIpSecSa fail
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(0));
    EXPECT_EQ(IMS_FALSE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, m_objLocalAddress));

    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    // Remote address null
    EXPECT_EQ(IMS_TRUE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, m_objLocalAddress));

    EXPECT_CALL(m_objSystem, RemoveIpSecSaParameter(_, _)).Times(1);
    objOsNetworkIpSec.DeletePolicy(pIpSecPolicy);

    objOsNetworkIpSec.DestroyAllPolicies();
}

TEST_F(OsNetworkIpSecTest, ApplyIpSecTransformWithArgsSockets)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    MockISocket objMockSocket;
    MockISocket objMockServerSocket;

    EXPECT_CALL(objMockSocket, GetSocketType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::TYPE_STREAM));
    EXPECT_CALL(objMockSocket, GetSocketId()).Times(AnyNumber()).WillRepeatedly(Return(1));
    EXPECT_CALL(objMockServerSocket, GetSocketType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::TYPE_STREAM));
    EXPECT_CALL(objMockServerSocket, GetSocketId()).Times(AnyNumber()).WillRepeatedly(Return(2));

    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(0);
    EXPECT_EQ(IMS_FALSE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, IMS_NULL));
    EXPECT_EQ(IMS_FALSE, objOsNetworkIpSec.ApplyIpSecTransform(IMS_NULL, &objMockServerSocket));

    // no policy present
    EXPECT_EQ(
            IMS_TRUE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, &objMockServerSocket));

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IIpSecSp* pIpSecSp = pIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    SetIpsecSpTransport(pIpSecSp);

    IIpSecSa* pIpSecSa = pIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    SetIpsecSa(pIpSecSa);

    EXPECT_CALL(m_objSystem, AddIpSecSaParameter(_, _)).Times(1).WillRepeatedly(Return(1));

    objOsNetworkIpSec.AddPolicy(pIpSecPolicy);

    // server Socket ID not yet updated with Ipsec transform
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(0);
    EXPECT_EQ(
            IMS_TRUE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, &objMockServerSocket));

    // One policy present
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    EXPECT_EQ(IMS_TRUE,
            objOsNetworkIpSec.ApplyIpSecTransform(
                    &objMockServerSocket, m_objLocalAddress, &m_objRemoteAddress));

    // ApplyIpSecSa fail
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(0));
    EXPECT_EQ(
            IMS_FALSE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, &objMockServerSocket));

    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    EXPECT_EQ(
            IMS_TRUE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, &objMockServerSocket));
}

TEST_F(OsNetworkIpSecTest, RemoveIpSecTransforms)
{
    OsNetworkIpSec objOsNetworkIpSec(IMS_SLOT_0);

    MockISocket objMockSocket;

    EXPECT_CALL(objMockSocket, GetSocketType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::TYPE_STREAM));
    EXPECT_CALL(objMockSocket, GetSocketId()).Times(AnyNumber()).WillRepeatedly(Return(1));

    EXPECT_CALL(m_objSystem, RemoveIpSecSa(_, _, _, _)).Times(0);
    objOsNetworkIpSec.RemoveIpSecTransforms(objMockSocket.GetSocketId());

    IIpSecPolicy* pIpSecPolicy = objOsNetworkIpSec.CreatePolicy();
    ASSERT_TRUE(pIpSecPolicy != nullptr);

    IIpSecSp* pIpSecSp = pIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    SetIpsecSpTransport(pIpSecSp);

    IIpSecSa* pIpSecSa = pIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    SetIpsecSa(pIpSecSa);

    EXPECT_CALL(m_objSystem, AddIpSecSaParameter(_, _)).Times(1).WillRepeatedly(Return(1));

    objOsNetworkIpSec.AddPolicy(pIpSecPolicy);

    // One policy present
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    EXPECT_EQ(IMS_TRUE,
            objOsNetworkIpSec.ApplyIpSecTransform(
                    &objMockSocket, m_objLocalAddress, &m_objRemoteAddress));

    // One policy present
    EXPECT_CALL(m_objSystem, RemoveIpSecSa(_, _, _, _)).Times(1);
    objOsNetworkIpSec.RemoveIpSecTransforms(objMockSocket.GetSocketId());

    MockISocket objMockServerSocket;

    EXPECT_CALL(objMockServerSocket, GetSocketType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::TYPE_STREAM));
    EXPECT_CALL(objMockServerSocket, GetSocketId()).Times(AnyNumber()).WillRepeatedly(Return(2));

    // One policy present
    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    EXPECT_EQ(IMS_TRUE,
            objOsNetworkIpSec.ApplyIpSecTransform(
                    &objMockServerSocket, m_objLocalAddress, &m_objRemoteAddress));

    EXPECT_CALL(m_objSystem, ApplyIpSecSa(_, _, _, _)).Times(1).WillRepeatedly(Return(1));
    EXPECT_EQ(
            IMS_TRUE, objOsNetworkIpSec.ApplyIpSecTransform(&objMockSocket, &objMockServerSocket));

    EXPECT_CALL(m_objSystem, RemoveIpSecSa(_, _, _, _)).Times(2);
    objOsNetworkIpSec.RemoveIpSecTransforms(objMockServerSocket.GetSocketId());
    objOsNetworkIpSec.RemoveIpSecTransforms(objMockSocket.GetSocketId());
}

}  // namespace android
