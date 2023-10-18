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

#include "ImsIpSecType.h"
#include "IpSecSaParameter.h"
#include "network/OsIpSecSp.h"

namespace android
{

class OsIpSecSpTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}

protected:
    OsIpSecSp m_objOsIpSecSp;
};

TEST_F(OsIpSecSpTest, SetTransportInfoAndCreatePolicy)
{
    SocketAddress objLocalAddress(AString("192.168.2.3"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5080);
    IMS_UINT32 nTransportProtocol = IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_UDP;
    IMS_UINT32 nAction = IpSecType::ACTION_APPLY;  // Seems unused member
    IMS_UINT32 nDirection = IpSecSaParameter::Policy::DIRECTION_OUT;
    IMS_UINT32 nSpi = 21;
    IMS_UINT32 nMode = IpSecSaParameter::Policy::MODE_TUNNEL;

    m_objOsIpSecSp.SetTransportInfo(objLocalAddress.GetAddress(), objLocalAddress.GetPort(),
            objRemoteAddress.GetAddress(), objRemoteAddress.GetPort(), nTransportProtocol, nAction,
            nDirection, nSpi, nMode);

    IpSecSaParameter::Policy objOsIpSecSpPolicy = m_objOsIpSecSp.CreateSaPolicy();

    EXPECT_EQ(21, objOsIpSecSpPolicy.GetSpi());
    EXPECT_EQ(IpSecSaParameter::Policy::DIRECTION_OUT, objOsIpSecSpPolicy.GetDirection());
    EXPECT_EQ(IpSecSaParameter::Policy::MODE_TUNNEL, objOsIpSecSpPolicy.GetMode());
    EXPECT_EQ(IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_UDP,
            objOsIpSecSpPolicy.GetTransportProtocol());
    EXPECT_TRUE(objOsIpSecSpPolicy.GetLocalAddress().Equals(objLocalAddress));
    EXPECT_TRUE(objOsIpSecSpPolicy.GetRemoteAddress().Equals(objRemoteAddress));
}

}  // namespace android
