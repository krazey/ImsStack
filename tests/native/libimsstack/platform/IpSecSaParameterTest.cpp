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

#include "IpSecSaParameter.h"

namespace android
{

class IpSecSaParameterTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(IpSecSaParameterTest, Policy_DefaultConstructor)
{
    IpSecSaParameter::Policy objPolicy;

    EXPECT_EQ(0, objPolicy.GetSpi());
    EXPECT_EQ(IpSecSaParameter::Policy::DIRECTION_OUT, objPolicy.GetDirection());
    EXPECT_EQ(IpSecSaParameter::Policy::MODE_TRANSPORT, objPolicy.GetMode());
    EXPECT_EQ(IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_UDP, objPolicy.GetTransportProtocol());

    EXPECT_TRUE(objPolicy.GetLocalAddress().Equals(SocketAddress()));
    EXPECT_TRUE(objPolicy.GetRemoteAddress().Equals(SocketAddress()));

    EXPECT_EQ(IpSecSaParameter::Policy::SOCKET_NOT_SET, objPolicy.GetSocketId());
}

TEST_F(IpSecSaParameterTest, Policy_OverloadedConstructorWithoutPortParams)
{
    SocketAddress objLocalAddress(AString("192.168.2.5"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5061);

    IpSecSaParameter::Policy objPolicy(12, IpSecSaParameter::Policy::DIRECTION_IN,
            IpSecSaParameter::Policy::MODE_TUNNEL, IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objLocalAddress, objRemoteAddress);

    EXPECT_EQ(12, objPolicy.GetSpi());
    EXPECT_EQ(IpSecSaParameter::Policy::DIRECTION_IN, objPolicy.GetDirection());
    EXPECT_EQ(IpSecSaParameter::Policy::MODE_TUNNEL, objPolicy.GetMode());
    EXPECT_EQ(IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP, objPolicy.GetTransportProtocol());

    EXPECT_TRUE(objPolicy.GetLocalAddress().Equals(objLocalAddress));
    EXPECT_TRUE(objPolicy.GetRemoteAddress().Equals(objRemoteAddress));

    EXPECT_EQ(IpSecSaParameter::Policy::SOCKET_NOT_SET, objPolicy.GetSocketId());
}

TEST_F(IpSecSaParameterTest, Policy_OverloadedConstructorWithPortParams)
{
    SocketAddress objLocalAddress(AString("192.168.2.5"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5061);

    IpSecSaParameter::Policy objPolicy(12, IpSecSaParameter::Policy::DIRECTION_IN,
            IpSecSaParameter::Policy::MODE_TUNNEL, IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objLocalAddress.GetAddress(), objLocalAddress.GetPort(), objRemoteAddress.GetAddress(),
            objRemoteAddress.GetPort());

    EXPECT_EQ(12, objPolicy.GetSpi());
    EXPECT_EQ(IpSecSaParameter::Policy::DIRECTION_IN, objPolicy.GetDirection());
    EXPECT_EQ(IpSecSaParameter::Policy::MODE_TUNNEL, objPolicy.GetMode());
    EXPECT_EQ(IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP, objPolicy.GetTransportProtocol());

    EXPECT_TRUE(objPolicy.GetLocalAddress().Equals(objLocalAddress));
    EXPECT_TRUE(objPolicy.GetRemoteAddress().Equals(objRemoteAddress));

    EXPECT_EQ(IpSecSaParameter::Policy::SOCKET_NOT_SET, objPolicy.GetSocketId());
}

TEST_F(IpSecSaParameterTest, Policy_CopyConstructor)
{
    SocketAddress objLocalAddress(AString("192.168.2.5"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5061);

    IpSecSaParameter::Policy objPolicy(12, IpSecSaParameter::Policy::DIRECTION_IN,
            IpSecSaParameter::Policy::MODE_TUNNEL, IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objLocalAddress.GetAddress(), objLocalAddress.GetPort(), objRemoteAddress.GetAddress(),
            objRemoteAddress.GetPort());

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    IpSecSaParameter::Policy objCopiedPolicy(objPolicy);

    EXPECT_EQ(12, objCopiedPolicy.GetSpi());
    EXPECT_EQ(IpSecSaParameter::Policy::DIRECTION_IN, objCopiedPolicy.GetDirection());
    EXPECT_EQ(IpSecSaParameter::Policy::MODE_TUNNEL, objCopiedPolicy.GetMode());
    EXPECT_EQ(IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objCopiedPolicy.GetTransportProtocol());

    EXPECT_TRUE(objCopiedPolicy.GetLocalAddress().Equals(objLocalAddress));
    EXPECT_TRUE(objCopiedPolicy.GetRemoteAddress().Equals(objRemoteAddress));

    EXPECT_EQ(IpSecSaParameter::Policy::SOCKET_NOT_SET, objPolicy.GetSocketId());
}

TEST_F(IpSecSaParameterTest, Policy_OverloadedAssignmentOperator)
{
    SocketAddress objLocalAddress(AString("192.168.2.5"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5061);

    IpSecSaParameter::Policy objPolicy(12, IpSecSaParameter::Policy::DIRECTION_IN,
            IpSecSaParameter::Policy::MODE_TUNNEL, IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objLocalAddress.GetAddress(), objLocalAddress.GetPort(), objRemoteAddress.GetAddress(),
            objRemoteAddress.GetPort());

    IpSecSaParameter::Policy objCopiedPolicy;
    objCopiedPolicy = objPolicy;

    EXPECT_EQ(12, objCopiedPolicy.GetSpi());
    EXPECT_EQ(IpSecSaParameter::Policy::DIRECTION_IN, objCopiedPolicy.GetDirection());
    EXPECT_EQ(IpSecSaParameter::Policy::MODE_TUNNEL, objCopiedPolicy.GetMode());
    EXPECT_EQ(IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objCopiedPolicy.GetTransportProtocol());

    EXPECT_TRUE(objCopiedPolicy.GetLocalAddress().Equals(objLocalAddress));
    EXPECT_TRUE(objCopiedPolicy.GetRemoteAddress().Equals(objRemoteAddress));

    EXPECT_EQ(IpSecSaParameter::Policy::SOCKET_NOT_SET, objPolicy.GetSocketId());
}

TEST_F(IpSecSaParameterTest, Policy_SetGetSocketID)
{
    IpSecSaParameter::Policy objPolicy;
    EXPECT_EQ(IpSecSaParameter::Policy::SOCKET_NOT_SET, objPolicy.GetSocketId());
    objPolicy.SetSocketId(12345);
    EXPECT_EQ(12345, objPolicy.GetSocketId());
}

TEST_F(IpSecSaParameterTest, Policy_Add_Remove_AcceptedSocketId)
{
    IpSecSaParameter::Policy objPolicy;
    EXPECT_FALSE(objPolicy.HasAcceptedSocketId(IpSecSaParameter::Policy::SOCKET_NOT_SET));
    EXPECT_FALSE(objPolicy.HasAcceptedSocketId(12345));
    objPolicy.AddAcceptedSocketId(12345);
    EXPECT_TRUE(objPolicy.HasAcceptedSocketId(12345));
    EXPECT_FALSE(objPolicy.HasAcceptedSocketId(54321));
    objPolicy.AddAcceptedSocketId(54321);
    EXPECT_TRUE(objPolicy.HasAcceptedSocketId(54321));
    objPolicy.RemoveAcceptedSocketId(1);
    EXPECT_TRUE(objPolicy.HasAcceptedSocketId(12345));
    EXPECT_TRUE(objPolicy.HasAcceptedSocketId(54321));
    objPolicy.RemoveAcceptedSocketId(12345);
    objPolicy.RemoveAcceptedSocketId(54321);
    EXPECT_FALSE(objPolicy.HasAcceptedSocketId(12345));
    EXPECT_FALSE(objPolicy.HasAcceptedSocketId(54321));
}

TEST_F(IpSecSaParameterTest, Policy_ToString)
{
    IpSecSaParameter::Policy objPolicy;

    EXPECT_FALSE(objPolicy.ToString().IsEmpty());

    IpAddress objLocalAddress(AString("192.168.2.5"));
    IpAddress objRemoteAddress(AString("192.168.5.9"));

    IpSecSaParameter::Policy objPolicyIPv4(12, IpSecSaParameter::Policy::DIRECTION_IN,
            IpSecSaParameter::Policy::MODE_TUNNEL, IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP,
            objLocalAddress, 7090, objRemoteAddress, 7091);

    EXPECT_FALSE(objPolicyIPv4.ToString().IsEmpty());
}

TEST_F(IpSecSaParameterTest, Policy_DirectionToString)
{
    EXPECT_STREQ("IN",
            IpSecSaParameter::Policy::DirectionToString(IpSecSaParameter::Policy::DIRECTION_IN));
    EXPECT_STREQ("OUT",
            IpSecSaParameter::Policy::DirectionToString(IpSecSaParameter::Policy::DIRECTION_OUT));
    EXPECT_STREQ("INVALID", IpSecSaParameter::Policy::DirectionToString(20));
}

TEST_F(IpSecSaParameterTest, Policy_ModeToString)
{
    EXPECT_STREQ("TRANSPORT",
            IpSecSaParameter::Policy::ModeToString(IpSecSaParameter::Policy::MODE_TRANSPORT));
    EXPECT_STREQ("TUNNEL",
            IpSecSaParameter::Policy::ModeToString(IpSecSaParameter::Policy::MODE_TUNNEL));
    EXPECT_STREQ("UNKNOWN", IpSecSaParameter::Policy::ModeToString(20));
}

TEST_F(IpSecSaParameterTest, Policy_TransportProtocolToString)
{
    EXPECT_STREQ("UDP",
            IpSecSaParameter::Policy::TransportProtocolToString(
                    IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_UDP));
    EXPECT_STREQ("TCP",
            IpSecSaParameter::Policy::TransportProtocolToString(
                    IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP));
    EXPECT_STREQ("UNKNOWN", IpSecSaParameter::Policy::TransportProtocolToString(20));
}

TEST_F(IpSecSaParameterTest, DefaultConstructor)
{
    IpSecSaParameter objIpSecSaParameter;

    EXPECT_EQ(-1, objIpSecSaParameter.GetIpSecId());
    EXPECT_EQ(IpSecSaParameter::SECURITY_PROTOCOL_ESP, objIpSecSaParameter.GetSecurityProtocol());
    EXPECT_EQ(IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96,
            objIpSecSaParameter.GetIntegrityAlgorithm());
    EXPECT_EQ(
            IpSecSaParameter::ENCRYPTION_ALG_AES_CBC, objIpSecSaParameter.GetEncryptionAlgorithm());
    EXPECT_TRUE(objIpSecSaParameter.GetIk().IsNull());
    EXPECT_TRUE(objIpSecSaParameter.GetCk().IsNull());
    EXPECT_EQ(0, objIpSecSaParameter.GetPolicys().GetSize());
}

TEST_F(IpSecSaParameterTest, OverloadedConstructor)
{
    AString strIk("12345");
    AString strCk("67890");

    IpSecSaParameter objIpSecSaParameter(90, IpSecSaParameter::SECURITY_PROTOCOL_ESP,
            IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96, IpSecSaParameter::ENCRYPTION_ALG_AES_CBC,
            ByteArray(strIk), ByteArray(strCk));

    EXPECT_EQ(90, objIpSecSaParameter.GetIpSecId());
    EXPECT_EQ(IpSecSaParameter::SECURITY_PROTOCOL_ESP, objIpSecSaParameter.GetSecurityProtocol());
    EXPECT_EQ(IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96,
            objIpSecSaParameter.GetIntegrityAlgorithm());
    EXPECT_EQ(
            IpSecSaParameter::ENCRYPTION_ALG_AES_CBC, objIpSecSaParameter.GetEncryptionAlgorithm());

    EXPECT_EQ(strIk, objIpSecSaParameter.GetIk().ToString());

    EXPECT_EQ(strCk, objIpSecSaParameter.GetCk().ToString());

    EXPECT_EQ(0, objIpSecSaParameter.GetPolicys().GetSize());
}

TEST_F(IpSecSaParameterTest, CopyConstructor)
{
    AString strIk("12345");
    AString strCk("67890");

    IpSecSaParameter objIpSecSaParameter(90, IpSecSaParameter::SECURITY_PROTOCOL_ESP,
            IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96, IpSecSaParameter::ENCRYPTION_ALG_AES_CBC,
            ByteArray(strIk), ByteArray(strCk));

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    IpSecSaParameter objCopiedIpSecSaParameter(objIpSecSaParameter);

    EXPECT_EQ(90, objCopiedIpSecSaParameter.GetIpSecId());
    EXPECT_EQ(IpSecSaParameter::SECURITY_PROTOCOL_ESP,
            objCopiedIpSecSaParameter.GetSecurityProtocol());
    EXPECT_EQ(IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96,
            objCopiedIpSecSaParameter.GetIntegrityAlgorithm());
    EXPECT_EQ(IpSecSaParameter::ENCRYPTION_ALG_AES_CBC,
            objCopiedIpSecSaParameter.GetEncryptionAlgorithm());

    EXPECT_EQ(strIk, objIpSecSaParameter.GetIk().ToString());

    EXPECT_EQ(strCk, objIpSecSaParameter.GetCk().ToString());

    EXPECT_EQ(0, objCopiedIpSecSaParameter.GetPolicys().GetSize());
}

TEST_F(IpSecSaParameterTest, OverloadedAssignmentOperator)
{
    AString strIk("12345");
    AString strCk("67890");

    IpSecSaParameter objIpSecSaParameter(90, IpSecSaParameter::SECURITY_PROTOCOL_ESP,
            IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96, IpSecSaParameter::ENCRYPTION_ALG_AES_CBC,
            ByteArray(strIk), ByteArray(strCk));

    IpSecSaParameter objCopiedIpSecSaParameter;
    objCopiedIpSecSaParameter = objIpSecSaParameter;

    EXPECT_EQ(90, objCopiedIpSecSaParameter.GetIpSecId());
    EXPECT_EQ(IpSecSaParameter::SECURITY_PROTOCOL_ESP,
            objCopiedIpSecSaParameter.GetSecurityProtocol());
    EXPECT_EQ(IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96,
            objCopiedIpSecSaParameter.GetIntegrityAlgorithm());
    EXPECT_EQ(IpSecSaParameter::ENCRYPTION_ALG_AES_CBC,
            objCopiedIpSecSaParameter.GetEncryptionAlgorithm());

    EXPECT_EQ(strIk, objIpSecSaParameter.GetIk().ToString());

    EXPECT_EQ(strCk, objIpSecSaParameter.GetCk().ToString());

    EXPECT_EQ(0, objCopiedIpSecSaParameter.GetPolicys().GetSize());
}

TEST_F(IpSecSaParameterTest, ToString)
{
    IpSecSaParameter objIpSecSaParameter;

    EXPECT_FALSE(objIpSecSaParameter.ToString().IsEmpty());

    IpSecSaParameter objIpSecSaParameter1(90, IpSecSaParameter::SECURITY_PROTOCOL_ESP,
            IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96, IpSecSaParameter::ENCRYPTION_ALG_AES_CBC,
            ByteArray(AString("12345")), ByteArray(AString("67890")));

    ImsList<IpSecSaParameter::Policy> objPolicys;
    IpSecSaParameter::Policy objPolicy;
    objPolicys.Append(objPolicy);

    // Add policy list
    objIpSecSaParameter1.AddPolicys(objPolicys);

    EXPECT_FALSE(objIpSecSaParameter1.ToString().IsEmpty());

    objIpSecSaParameter1.RemoveAllPolicys();

    EXPECT_FALSE(objIpSecSaParameter1.ToString().IsEmpty());

    // Add policy
    objIpSecSaParameter1.AddPolicy(objPolicy);

    EXPECT_FALSE(objIpSecSaParameter1.ToString().IsEmpty());
}

TEST_F(IpSecSaParameterTest, SecurityProtocolToString)
{
    EXPECT_STREQ("AH",
            IpSecSaParameter::SecurityProtocolToString(IpSecSaParameter::SECURITY_PROTOCOL_AH));
    EXPECT_STREQ("ESP",
            IpSecSaParameter::SecurityProtocolToString(IpSecSaParameter::SECURITY_PROTOCOL_ESP));
    EXPECT_STREQ("UNKNOWN", IpSecSaParameter::SecurityProtocolToString(12));
}

TEST_F(IpSecSaParameterTest, IntegrityAlgToString)
{
    EXPECT_STREQ("HMAC_MD5_96",
            IpSecSaParameter::IntegrityAlgToString(IpSecSaParameter::INTEGRITY_ALG_HMAC_MD5_96));
    EXPECT_STREQ("HMAC_SHA1_96",
            IpSecSaParameter::IntegrityAlgToString(IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96));
    EXPECT_STREQ("UNKNOWN",
            IpSecSaParameter::IntegrityAlgToString(IpSecSaParameter::INTEGRITY_ALG_AES_GMAC));
    EXPECT_STREQ("UNKNOWN",
            IpSecSaParameter::IntegrityAlgToString(IpSecSaParameter::INTEGRITY_ALG_NULL));
}

TEST_F(IpSecSaParameterTest, EncryptionAlgToString)
{
    EXPECT_STREQ("DES_EDE3_CBC",
            IpSecSaParameter::EncryptionAlgToString(IpSecSaParameter::ENCRYPTION_ALG_DES_EDE3_CBC));
    EXPECT_STREQ("AES_CBC",
            IpSecSaParameter::EncryptionAlgToString(IpSecSaParameter::ENCRYPTION_ALG_AES_CBC));
    EXPECT_STREQ(
            "NULL", IpSecSaParameter::EncryptionAlgToString(IpSecSaParameter::ENCRYPTION_ALG_NULL));
    EXPECT_STREQ("UNKNOWN",
            IpSecSaParameter::EncryptionAlgToString(IpSecSaParameter::ENCRYPTION_ALG_AES_GCM));
}
}  // namespace android
