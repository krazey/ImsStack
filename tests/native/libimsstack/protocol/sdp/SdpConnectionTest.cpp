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

#include "Sdp.h"
#include "SdpConnection.h"

namespace android
{

class TestConnection
{
public:
    inline TestConnection() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_nAddrType(Sdp::ADDR_TYPE_IP4),
            m_strAddrType(AString::ConstNull()),
            m_strAddress(AString::ConstNull())
    {
    }
    inline TestConnection(const AString& strSdpFullLine, const AString& strSdpLine,
            IMS_SINT32 nAddrType, const AString& strAddrType, const AString& strAddress) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_nAddrType(nAddrType),
            m_strAddrType(strAddrType),
            m_strAddress(strAddress)
    {
    }
    inline TestConnection(const TestConnection& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_nAddrType(other.m_nAddrType),
            m_strAddrType(other.m_strAddrType),
            m_strAddress(other.m_strAddress)
    {
    }
    inline ~TestConnection() {}

    inline TestConnection& operator=(const TestConnection& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_nAddrType = other.m_nAddrType;
            m_strAddrType = other.m_strAddrType;
            m_strAddress = other.m_strAddress;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;
    AString m_strSdpLine;
    IMS_SINT32 m_nAddrType;
    AString m_strAddrType;
    AString m_strAddress;
};

class SdpConnectionTest : public ::testing::Test
{
public:
    SdpConnectionTest();

protected:
    TestConnection m_objTestConnection;
    ImsVector<TestConnection> m_objNormalTestConnections;
    ImsVector<TestConnection> m_objAbnormalTestConnections;
};

SdpConnectionTest::SdpConnectionTest() :
        m_objTestConnection(TestConnection("c=IN IP4 10.47.16.5\r\n", "IN IP4 10.47.16.5",
                Sdp::ADDR_TYPE_IP4, "IP4", "10.47.16.5"))
{
    // clang-format off
    m_objNormalTestConnections.Add(m_objTestConnection);
    m_objNormalTestConnections.Add(TestConnection(
            "c=IN IP6 fc01:abab:cdcd:efe0::1\r\n",
            "IN IP6 fc01:abab:cdcd:efe0::1", Sdp::ADDR_TYPE_IP6, "IP6", "fc01:abab:cdcd:efe0::1"));
    m_objNormalTestConnections.Add(TestConnection(
            "c=IN FQDN ims.example.com\r\n",
            "IN FQDN ims.example.com", Sdp::ADDR_TYPE_OTHER, "FQDN", "ims.example.com"));

    // Invalid address type
    m_objAbnormalTestConnections.Add(TestConnection(
            "c=IN ATM/6 fc01:abab:cdcd:efe0::1\r\n",
            "IN ATM/6 fc01:abab:cdcd:efe0::1", Sdp::ADDR_TYPE_OTHER, "ATM/6",
            "fc01:abab:cdcd:efe0::1"));
    // Invalid address
    m_objAbnormalTestConnections.Add(TestConnection(
            "c=IN IP6 10.47.16.5\r\n",
            "IN IP6 10.47.16.5", Sdp::ADDR_TYPE_IP6, "IP6", "10.47.16.5"));
    // Invalid address
    m_objAbnormalTestConnections.Add(TestConnection(
            "c=IN IP4 fc01:abab:cdcd:efe0::1\r\n",
            "IN IP4 fc01:abab:cdcd:efe0::1", Sdp::ADDR_TYPE_IP4, "IP6", "fc01:abab:cdcd:efe0::1"));
    // Invalid address
    m_objAbnormalTestConnections.Add(TestConnection(
            "c=IN FQDN ims.\rexample.com\r\n",
            "IN FQDN ims.\rexample.com", Sdp::ADDR_TYPE_OTHER, "FQDN", "ims.\rexample.com"));
    // clang-format on
}

TEST_F(SdpConnectionTest, Constructor)
{
    SdpConnection objConnection;
    EXPECT_EQ(objConnection.GetAddressType(), AString(Sdp::STR_ADDR_TYPE_IP6));
    EXPECT_EQ(objConnection.GetAddress(), AString::ConstNull());
    EXPECT_EQ(objConnection.GetAddresses().GetSize(), 1);
}

TEST_F(SdpConnectionTest, CopyConstructor)
{
    SdpConnection objConnection;
    ASSERT_TRUE(objConnection.SetValue(
            m_objTestConnection.m_nAddrType, m_objTestConnection.m_strAddress));

    SdpConnection objNewConnection(objConnection);
    EXPECT_EQ(objNewConnection.GetAddressType(), objConnection.GetAddressType());
    EXPECT_EQ(objNewConnection.GetAddress(), objConnection.GetAddress());
}

TEST_F(SdpConnectionTest, OperatorAssignment)
{
    SdpConnection objConnection;
    ASSERT_TRUE(objConnection.SetValue(
            m_objTestConnection.m_nAddrType, m_objTestConnection.m_strAddress));

    SdpConnection objNewConnection;
    objNewConnection = objConnection;
    // cppcheck-suppress knownConditionTrueFalse
    EXPECT_EQ(objNewConnection.GetAddressType(), objConnection.GetAddressType());
    // cppcheck-suppress knownConditionTrueFalse
    EXPECT_EQ(objNewConnection.GetAddress(), objConnection.GetAddress());
}

TEST_F(SdpConnectionTest, Decode)
{
    SdpConnection objConnection;
    IMS_UINT32 nTestCount = m_objNormalTestConnections.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestConnection& objTestConnection = m_objNormalTestConnections.GetAt(i);

        ASSERT_TRUE(objConnection.Decode(objTestConnection.m_strSdpLine));
        EXPECT_EQ(objConnection.GetAddressType(), objTestConnection.m_strAddrType);
        EXPECT_EQ(objConnection.GetAddress(), objTestConnection.m_strAddress);
    }

    nTestCount = m_objAbnormalTestConnections.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestConnection& objTestConnection = m_objAbnormalTestConnections.GetAt(i);

        EXPECT_FALSE(objConnection.Decode(objTestConnection.m_strSdpLine));
    }

    EXPECT_FALSE(objConnection.Decode(AString::ConstNull()));
    EXPECT_FALSE(objConnection.Decode(AString::ConstEmpty()));
}

TEST_F(SdpConnectionTest, Encode)
{
    SdpConnection objConnection;
    AString strEncoded = objConnection.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    IMS_UINT32 nTestCount = m_objNormalTestConnections.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestConnection& objTestConnection = m_objNormalTestConnections.GetAt(i);

        ASSERT_TRUE(objConnection.SetValue(objTestConnection.m_nAddrType,
                objTestConnection.m_strAddress, objTestConnection.m_strAddrType));
        strEncoded = objConnection.Encode();
        EXPECT_EQ(objTestConnection.m_strSdpFullLine, strEncoded);
    }
}

TEST_F(SdpConnectionTest, SetValue)
{
    SdpConnection objConnection;
    IMS_UINT32 nTestCount = m_objNormalTestConnections.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestConnection& objTestConnection = m_objNormalTestConnections.GetAt(i);

        ASSERT_TRUE(objConnection.SetValue(objTestConnection.m_nAddrType,
                objTestConnection.m_strAddress, objTestConnection.m_strAddrType));
        EXPECT_EQ(objConnection.GetAddressType(), objTestConnection.m_strAddrType);
        EXPECT_EQ(objConnection.GetAddress(), objTestConnection.m_strAddress);
        EXPECT_EQ(objConnection.GetValue(), objTestConnection.m_strSdpLine);
    }

    ASSERT_FALSE(objConnection.SetValue(Sdp::ADDR_TYPE_OTHER + 1, m_objTestConnection.m_strAddress,
            m_objTestConnection.m_strAddrType));
    ASSERT_FALSE(objConnection.SetValue(m_objTestConnection.m_nAddrType, AString::ConstNull(),
            m_objTestConnection.m_strAddrType));
    ASSERT_FALSE(objConnection.SetValue(
            Sdp::ADDR_TYPE_OTHER, m_objTestConnection.m_strAddress, AString::ConstNull()));
}

}  // namespace android
