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

#include "SocketAddress.h"

namespace android
{

class SocketAddressTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SocketAddressTest, Constructor)
{
    SocketAddress objSa;

    EXPECT_EQ(0, objSa.GetPort());
    ASSERT_TRUE(IpAddress::IPv6NONE.Equals(objSa.GetAddress()));

    IpAddress objIpAddr("192.168.0.1");
    SocketAddress objSa2(objIpAddr, 5060);

    EXPECT_EQ(5060, objSa2.GetPort());
    ASSERT_TRUE(objIpAddr.Equals(objSa2.GetAddress()));

    AString strIpAddr("[2001:cafe:0:1::2002]");
    IpAddress objIpAddr1(strIpAddr);
    SocketAddress objSa2_1(strIpAddr, 5060);

    EXPECT_EQ(5060, objSa2_1.GetPort());
    ASSERT_TRUE(objIpAddr1.Equals(objSa2_1.GetAddress()));

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    SocketAddress objSa3(objSa2_1);

    EXPECT_EQ(5060, objSa3.GetPort());
    ASSERT_TRUE(objIpAddr1.Equals(objSa3.GetAddress()));
}

TEST_F(SocketAddressTest, OperatorAssignment)
{
    IpAddress objIpAddr("[2001:cafe:0:1::2002]");
    SocketAddress objSa(objIpAddr, 5060);

    EXPECT_EQ(5060, objSa.GetPort());
    ASSERT_TRUE(objIpAddr.Equals(objSa.GetAddress()));

    SocketAddress objSa1;
    objSa1 = objSa;

    EXPECT_EQ(5060, objSa1.GetPort());
    ASSERT_TRUE(objIpAddr.Equals(objSa1.GetAddress()));
}

TEST_F(SocketAddressTest, Setter)
{
    IpAddress objIpAddr("[2001:cafe:0:1::2002]");
    SocketAddress objSa;

    objSa.SetPort(5060);
    objSa.SetAddress(objIpAddr);

    EXPECT_EQ(5060, objSa.GetPort());
    ASSERT_TRUE(objIpAddr.Equals(objSa.GetAddress()));

    IpAddress objIpAddr1("192.168.0.1");
    objSa.SetPort(49152);
    objSa.SetAddress(objIpAddr1);

    EXPECT_EQ(49152, objSa.GetPort());
    ASSERT_TRUE(objIpAddr1.Equals(objSa.GetAddress()));
}

TEST_F(SocketAddressTest, Equals)
{
    IpAddress objIpAddr("[2001:cafe:0:1::2002]");
    SocketAddress objSa1(objIpAddr, 5060);
    SocketAddress objSa2(objIpAddr, 5060);

    ASSERT_TRUE(objSa1.Equals(objSa2));

    // Different Port
    objSa1.SetPort(49152);
    ASSERT_FALSE(objSa1.Equals(objSa2));

    // Different IP Address
    objSa1.SetPort(5060);
    objSa1.SetAddress(IpAddress("192.168.0.1"));

    ASSERT_FALSE(objSa1.Equals(objSa2));
}

TEST_F(SocketAddressTest, ToString)
{
    IpAddress objIpAddr6("[2001:cafe:0:1::2002]");
    SocketAddress objSaIpv6(objIpAddr6, 5060);

    EXPECT_STREQ("[2001:cafe:0:1::2002]:5060", objSaIpv6.ToString().GetStr());

    IpAddress objIpAddr4("192.168.0.1");
    SocketAddress objSaIpv4(objIpAddr4, 5060);
    EXPECT_STREQ("192.168.0.1:5060", objSaIpv4.ToString().GetStr());
}

}  // namespace android
