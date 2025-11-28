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

#include "AString.h"

#include "Sdp.h"
#include "SdpOrigin.h"

namespace android
{

class TestOrigin
{
public:
    inline TestOrigin() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_strUsername(AString::ConstNull()),
            m_strSessionId(AString::ConstNull()),
            m_strSessionVersion(AString::ConstNull()),
            m_nAddrType(Sdp::ADDR_TYPE_IP4),
            m_strAddrTypeString(AString::ConstNull()),
            m_strAddress(AString::ConstNull())
    {
    }
    inline TestOrigin(const AString& strSdpFullLine, const AString& strSdpLine,
            const AString& strUsername, const AString& strSessionId,
            const AString& strSessionVersion, IMS_SINT32 nAddrType,
            const AString& strAddrTypeString, const AString& strAddress) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_strUsername(strUsername),
            m_strSessionId(strSessionId),
            m_strSessionVersion(strSessionVersion),
            m_nAddrType(nAddrType),
            m_strAddrTypeString(strAddrTypeString),
            m_strAddress(strAddress)
    {
    }
    inline TestOrigin(const TestOrigin& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_strUsername(other.m_strUsername),
            m_strSessionId(other.m_strSessionId),
            m_strSessionVersion(other.m_strSessionVersion),
            m_nAddrType(other.m_nAddrType),
            m_strAddrTypeString(other.m_strAddrTypeString),
            m_strAddress(other.m_strAddress)
    {
    }
    inline ~TestOrigin() {}

    inline TestOrigin& operator=(const TestOrigin& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_strUsername = other.m_strUsername;
            m_strSessionId = other.m_strSessionId;
            m_strSessionVersion = other.m_strSessionVersion;
            m_nAddrType = other.m_nAddrType;
            m_strAddrTypeString = other.m_strAddrTypeString;
            m_strAddress = other.m_strAddress;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;
    AString m_strSdpLine;
    AString m_strUsername;
    AString m_strSessionId;
    AString m_strSessionVersion;
    IMS_SINT32 m_nAddrType;
    AString m_strAddrTypeString;
    AString m_strAddress;
};

class SdpOriginTest : public ::testing::Test
{
public:
    SdpOriginTest();

protected:
    TestOrigin m_objTestOrigin;
    ImsVector<TestOrigin> m_objNormalTestOrigins;
    ImsVector<TestOrigin> m_objAbnormalTestOrigins;
};

SdpOriginTest::SdpOriginTest() :
        m_objTestOrigin(TestOrigin("o=ims 2890844526 2890842807 IN IP4 10.47.16.5\r\n",
                "ims 2890844526 2890842807 IN IP4 10.47.16.5", "ims", "2890844526", "2890842807",
                Sdp::ADDR_TYPE_IP4, "IP4", "10.47.16.5"))
{
    // clang-format off
    m_objNormalTestOrigins.Add(m_objTestOrigin);
    m_objNormalTestOrigins.Add(TestOrigin(
            "o=ims 3866718292 3866718292 IN IP6 fc01:abab:cdcd:efe0::1\r\n",
            "ims 3866718292 3866718292 IN IP6 fc01:abab:cdcd:efe0::1",
            "ims", "3866718292", "3866718292", Sdp::ADDR_TYPE_IP6, "IP6",
            "fc01:abab:cdcd:efe0::1"));
    m_objNormalTestOrigins.Add(TestOrigin(
            "o=ims 1866718292 1866718292 IN FQDN ims.example.com\r\n",
            "ims 1866718292 1866718292 IN FQDN ims.example.com",
            "ims", "1866718292", "1866718292", Sdp::ADDR_TYPE_OTHER, "FQDN",
            "ims.example.com"));

    // Invalid username
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=i\rms 3866718292 3866718292 IN IP6 fc01:abab:cdcd:efe0::1\r\n",
            "i\rms 3866718292 3866718292 IN IP6 fc01:abab:cdcd:efe0::1",
            "i\rms", "3866718292", "3866718292", Sdp::ADDR_TYPE_IP6, "IP6",
            "fc01:abab:cdcd:efe0::1"));
    // Invalid session-id
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=ims 386671829a 3866718292 IN IP6 fc01:abab:cdcd:efe0::1\r\n",
            "ims 386671829a 3866718292 IN IP6 fc01:abab:cdcd:efe0::1",
            "ims", "386671829a", "3866718292", Sdp::ADDR_TYPE_IP6, "IP6",
            "fc01:abab:cdcd:efe0::1"));
    // Invalid session-version
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=ims 3866718292 386671829b IN IP6 fc01:abab:cdcd:efe0::1\r\n",
            "ims 3866718292 386671829b IN IP6 fc01:abab:cdcd:efe0::1",
            "ims", "3866718292", "386671829b", Sdp::ADDR_TYPE_IP6, "IP6",
            "fc01:abab:cdcd:efe0::1"));
    // Invalid address type
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=ims 3866718292 3866718292 IN ATM/6 fc01:abab:cdcd:efe0::1\r\n",
            "ims 3866718292 3866718292 IN ATM/6 fc01:abab:cdcd:efe0::1",
            "ims", "3866718292", "3866718292", Sdp::ADDR_TYPE_OTHER, "ATM/6",
            "fc01:abab:cdcd:efe0::1"));
    // Invalid address
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=ims 3866718292 3866718292 IN IP6 10.47.16.5\r\n",
            "ims 3866718292 3866718292 IN IP6 10.47.16.5",
            "ims", "3866718292", "3866718292", Sdp::ADDR_TYPE_IP6, "IP6",
            "10.47.16.5"));
    // Invalid address
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=ims 3866718292 3866718292 IN IP4 fc01:abab:cdcd:efe0::1\r\n",
            "ims 3866718292 3866718292 IN IP4 fc01:abab:cdcd:efe0::1",
            "ims", "3866718292", "3866718292", Sdp::ADDR_TYPE_IP4, "IP6",
            "fc01:abab:cdcd:efe0::1"));
    // Invalid address
    m_objAbnormalTestOrigins.Add(TestOrigin(
            "o=ims 3866718292 3866718292 IN FQDN ims.\rexample.com\r\n",
            "ims 3866718292 3866718292 IN FQDN ims.\rexample.com",
            "ims", "3866718292", "3866718292", Sdp::ADDR_TYPE_OTHER, "FQDN",
            "ims.\rexample.com"));
    // clang-format on
}

TEST_F(SdpOriginTest, Constructor)
{
    SdpOrigin objOrigin;
    EXPECT_EQ(objOrigin.GetUsername(), AString::ConstNull());
    EXPECT_EQ(objOrigin.GetSessionId(), AString::ConstNull());
    EXPECT_EQ(objOrigin.GetSessionVersion(), AString::ConstNull());
    EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP6);
    EXPECT_EQ(objOrigin.GetAddressTypeToString(), AString(Sdp::STR_ADDR_TYPE_IP6));
    EXPECT_EQ(objOrigin.GetAddress(), AString::ConstNull());
}

TEST_F(SdpOriginTest, CopyConstructor)
{
    SdpOrigin objOrigin;
    ASSERT_TRUE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strAddress));

    SdpOrigin objNewOrigin(objOrigin);
    EXPECT_EQ(objNewOrigin.GetUsername(), objOrigin.GetUsername());
    EXPECT_EQ(objNewOrigin.GetSessionId(), objOrigin.GetSessionId());
    EXPECT_EQ(objNewOrigin.GetSessionVersion(), objOrigin.GetSessionVersion());
    EXPECT_EQ(objNewOrigin.GetAddressType(), objOrigin.GetAddressType());
    EXPECT_EQ(objNewOrigin.GetAddressTypeToString(), objOrigin.GetAddressTypeToString());
    EXPECT_EQ(objNewOrigin.GetAddress(), objOrigin.GetAddress());
}

TEST_F(SdpOriginTest, OperatorAssignment)
{
    SdpOrigin objOrigin;
    ASSERT_TRUE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strSessionId,
            m_objTestOrigin.m_strSessionVersion, m_objTestOrigin.m_nAddrType,
            m_objTestOrigin.m_strAddress, m_objTestOrigin.m_strAddrTypeString));

    SdpOrigin objNewOrigin;
    objNewOrigin = objOrigin;
    EXPECT_EQ(objNewOrigin.GetUsername(), m_objTestOrigin.m_strUsername);
    EXPECT_EQ(objNewOrigin.GetSessionId(), m_objTestOrigin.m_strSessionId);
    EXPECT_EQ(objNewOrigin.GetSessionVersion(), m_objTestOrigin.m_strSessionVersion);
    EXPECT_EQ(objNewOrigin.GetAddressType(), m_objTestOrigin.m_nAddrType);
    EXPECT_EQ(objNewOrigin.GetAddressTypeToString(), m_objTestOrigin.m_strAddrTypeString);
    EXPECT_EQ(objNewOrigin.GetAddress(), m_objTestOrigin.m_strAddress);
}

TEST_F(SdpOriginTest, Decode)
{
    SdpOrigin objOrigin;
    IMS_UINT32 nTestCount = m_objNormalTestOrigins.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestOrigin& objTestOrigin = m_objNormalTestOrigins.GetAt(i);

        ASSERT_TRUE(objOrigin.Decode(objTestOrigin.m_strSdpLine));
        EXPECT_EQ(objOrigin.GetUsername(), objTestOrigin.m_strUsername);
        EXPECT_EQ(objOrigin.GetSessionId(), objTestOrigin.m_strSessionId);
        EXPECT_EQ(objOrigin.GetSessionVersion(), objTestOrigin.m_strSessionVersion);
        EXPECT_EQ(objOrigin.GetAddressType(), objTestOrigin.m_nAddrType);
        EXPECT_EQ(objOrigin.GetAddressTypeToString(), objTestOrigin.m_strAddrTypeString);
        EXPECT_EQ(objOrigin.GetAddress(), objTestOrigin.m_strAddress);
    }

    nTestCount = m_objAbnormalTestOrigins.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestOrigin& objTestOrigin = m_objAbnormalTestOrigins.GetAt(i);

        EXPECT_FALSE(objOrigin.Decode(objTestOrigin.m_strSdpLine));
    }

    EXPECT_FALSE(objOrigin.Decode(AString::ConstNull()));
    EXPECT_FALSE(objOrigin.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objOrigin.Decode(AString("3866718292 3866718292 IN IP6 fc01:abab:cdcd:efe0::1")));
    EXPECT_FALSE(objOrigin.Decode(
            AString("ims 3866718292 3866718292 IN IP6 fc01:abab:cdcd:efe0::1 test")));
}

TEST_F(SdpOriginTest, Encode)
{
    SdpOrigin objOrigin;
    AString strEncoded = objOrigin.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    IMS_UINT32 nTestCount = m_objNormalTestOrigins.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestOrigin& objTestOrigin = m_objNormalTestOrigins.GetAt(i);

        ASSERT_TRUE(objOrigin.SetValue(objTestOrigin.m_strUsername, objTestOrigin.m_strSessionId,
                objTestOrigin.m_strSessionVersion, objTestOrigin.m_nAddrType,
                objTestOrigin.m_strAddress, objTestOrigin.m_strAddrTypeString));
        strEncoded = objOrigin.Encode();
        EXPECT_EQ(objTestOrigin.m_strSdpFullLine, strEncoded);
    }
}

TEST_F(SdpOriginTest, IncreaseSessionVersion)
{
    AString strTestSessionVersion = "1866718299";
    AString strExpectedSessionVersion = "1866718300";
    SdpOrigin objOrigin;

    ASSERT_TRUE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strSessionId,
            strTestSessionVersion, m_objTestOrigin.m_nAddrType, m_objTestOrigin.m_strAddress,
            m_objTestOrigin.m_strAddrTypeString));
    EXPECT_EQ(objOrigin.GetSessionVersion(), strTestSessionVersion);
    objOrigin.IncreaseSessionVersion();
    EXPECT_EQ(objOrigin.GetSessionVersion(), strExpectedSessionVersion);
}

TEST_F(SdpOriginTest, SetAddress)
{
    SdpOrigin objOrigin;
    ASSERT_TRUE(objOrigin.Decode(m_objTestOrigin.m_strSdpLine));
    EXPECT_EQ(objOrigin.GetUsername(), m_objTestOrigin.m_strUsername);
    EXPECT_EQ(objOrigin.GetSessionId(), m_objTestOrigin.m_strSessionId);
    EXPECT_EQ(objOrigin.GetSessionVersion(), m_objTestOrigin.m_strSessionVersion);
    EXPECT_EQ(objOrigin.GetAddressType(), m_objTestOrigin.m_nAddrType);
    EXPECT_EQ(objOrigin.GetAddressTypeToString(), m_objTestOrigin.m_strAddrTypeString);
    EXPECT_EQ(objOrigin.GetAddress(), m_objTestOrigin.m_strAddress);

    AString strTestIp6 = "fc01:abab:cdcd:efe0::2";
    AString strTestIp4 = "10.47.16.6";

    if (objOrigin.GetAddressType() == Sdp::ADDR_TYPE_IP4)
    {
        ASSERT_TRUE(objOrigin.SetAddress(strTestIp6));
        EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP6);
        EXPECT_EQ(objOrigin.GetAddress(), strTestIp6);

        ASSERT_TRUE(objOrigin.SetAddress(strTestIp4));
        EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP4);
        EXPECT_EQ(objOrigin.GetAddress(), strTestIp4);
    }
    else if (objOrigin.GetAddressType() == Sdp::ADDR_TYPE_IP6)
    {
        ASSERT_TRUE(objOrigin.SetAddress(strTestIp4));
        EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP4);
        EXPECT_EQ(objOrigin.GetAddress(), strTestIp4);

        ASSERT_TRUE(objOrigin.SetAddress(strTestIp6));
        EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP6);
        EXPECT_EQ(objOrigin.GetAddress(), strTestIp6);
    }

    AString strFqdn = "ims.example.com";
    ASSERT_TRUE(objOrigin.SetAddress(strFqdn));
    EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP6);
    EXPECT_EQ(objOrigin.GetAddress(), strFqdn);

    AString strInvalidAddress = "i\rms.example.com";
    ASSERT_FALSE(objOrigin.SetAddress(strInvalidAddress));
    ASSERT_FALSE(objOrigin.SetAddress(AString::ConstNull()));
    ASSERT_FALSE(objOrigin.SetAddress(AString::ConstEmpty()));
}

TEST_F(SdpOriginTest, SetValue)
{
    SdpOrigin objOrigin;
    IMS_UINT32 nTestCount = m_objNormalTestOrigins.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestOrigin& objTestOrigin = m_objNormalTestOrigins.GetAt(i);

        ASSERT_TRUE(objOrigin.SetValue(objTestOrigin.m_strUsername, objTestOrigin.m_strAddress));
        EXPECT_EQ(objOrigin.GetUsername(), objTestOrigin.m_strUsername);
        EXPECT_NE(objOrigin.GetSessionId().GetLength(), 0);
        EXPECT_NE(objOrigin.GetSessionVersion().GetLength(), 0);

        if (objTestOrigin.m_nAddrType == Sdp::ADDR_TYPE_IP4 ||
                objTestOrigin.m_nAddrType == Sdp::ADDR_TYPE_IP6)
        {
            EXPECT_EQ(objOrigin.GetAddressType(), objTestOrigin.m_nAddrType);
            EXPECT_EQ(objOrigin.GetAddressTypeToString(), objTestOrigin.m_strAddrTypeString);
        }
        else
        {
            EXPECT_EQ(objOrigin.GetAddressType(), Sdp::ADDR_TYPE_IP6);
            EXPECT_EQ(objOrigin.GetAddressTypeToString(), AString(Sdp::STR_ADDR_TYPE_IP6));
        }
        EXPECT_EQ(objOrigin.GetAddress(), objTestOrigin.m_strAddress);
    }

    AString strInvalidUsername = "i\rms";
    ASSERT_FALSE(objOrigin.SetValue(strInvalidUsername, m_objTestOrigin.m_strAddress));
    ASSERT_FALSE(objOrigin.SetValue(AString::ConstNull(), m_objTestOrigin.m_strAddress));
    ASSERT_FALSE(objOrigin.SetValue(AString::ConstEmpty(), m_objTestOrigin.m_strAddress));
    AString strInvalidAddress = "i\rms.example.com";
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, strInvalidAddress));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, AString::ConstNull()));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, AString::ConstEmpty()));
}

TEST_F(SdpOriginTest, SetValueWithAllArguments)
{
    SdpOrigin objOrigin;
    IMS_UINT32 nTestCount = m_objNormalTestOrigins.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestOrigin& objTestOrigin = m_objNormalTestOrigins.GetAt(i);

        ASSERT_TRUE(objOrigin.SetValue(objTestOrigin.m_strUsername, objTestOrigin.m_strSessionId,
                objTestOrigin.m_strSessionVersion, objTestOrigin.m_nAddrType,
                objTestOrigin.m_strAddress, objTestOrigin.m_strAddrTypeString));
        EXPECT_EQ(objOrigin.GetUsername(), objTestOrigin.m_strUsername);
        EXPECT_EQ(objOrigin.GetSessionId(), objTestOrigin.m_strSessionId);
        EXPECT_EQ(objOrigin.GetSessionVersion(), objTestOrigin.m_strSessionVersion);
        EXPECT_EQ(objOrigin.GetAddressType(), objTestOrigin.m_nAddrType);
        EXPECT_EQ(objOrigin.GetAddressTypeToString(), objTestOrigin.m_strAddrTypeString);
        EXPECT_EQ(objOrigin.GetAddress(), objTestOrigin.m_strAddress);
        EXPECT_EQ(objOrigin.GetValue(), objTestOrigin.m_strSdpLine);
    }

    nTestCount = m_objAbnormalTestOrigins.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestOrigin& objTestOrigin = m_objAbnormalTestOrigins.GetAt(i);

        ASSERT_FALSE(objOrigin.SetValue(objTestOrigin.m_strUsername, objTestOrigin.m_strSessionId,
                objTestOrigin.m_strSessionVersion, objTestOrigin.m_nAddrType,
                objTestOrigin.m_strAddress, objTestOrigin.m_strAddrTypeString));
    }

    ASSERT_FALSE(objOrigin.SetValue(AString::ConstNull(), m_objTestOrigin.m_strSessionId,
            m_objTestOrigin.m_strSessionVersion, m_objTestOrigin.m_nAddrType,
            m_objTestOrigin.m_strAddress, m_objTestOrigin.m_strAddrTypeString));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, AString::ConstNull(),
            m_objTestOrigin.m_strSessionVersion, m_objTestOrigin.m_nAddrType,
            m_objTestOrigin.m_strAddress, m_objTestOrigin.m_strAddrTypeString));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strSessionId,
            AString::ConstNull(), m_objTestOrigin.m_nAddrType, m_objTestOrigin.m_strAddress,
            m_objTestOrigin.m_strAddrTypeString));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strSessionId,
            m_objTestOrigin.m_strSessionVersion, m_objTestOrigin.m_nAddrType, AString::ConstNull(),
            m_objTestOrigin.m_strAddrTypeString));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strSessionId,
            m_objTestOrigin.m_strSessionVersion, Sdp::ADDR_TYPE_OTHER, m_objTestOrigin.m_strAddress,
            AString::ConstNull()));
    ASSERT_FALSE(objOrigin.SetValue(m_objTestOrigin.m_strUsername, m_objTestOrigin.m_strSessionId,
            m_objTestOrigin.m_strSessionVersion, Sdp::ADDR_TYPE_OTHER + 1,
            m_objTestOrigin.m_strAddress, m_objTestOrigin.m_strAddrTypeString));
}

}  // namespace android
