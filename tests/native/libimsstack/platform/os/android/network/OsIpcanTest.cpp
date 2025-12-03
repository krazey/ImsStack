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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockISystem.h"
#include "PlatformContext.h"
#include "network/OsIpcan.h"

using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{

class OsIpcanTest : public ::testing::Test
{
public:
    inline OsIpcanTest() :
            m_piIpcan(IMS_NULL),
            m_piOldSystem(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_piOldSystem = PlatformContext::GetInstance()->SetSystem(&m_objSystem);
        m_piIpcan = &m_objIpcan;
    }

    virtual void TearDown() override { PlatformContext::GetInstance()->SetSystem(m_piOldSystem); }

protected:
    OsIpcan m_objIpcan;
    IIpcan* m_piIpcan;
    ISystem* m_piOldSystem;
    MockISystem m_objSystem;
};

TEST_F(OsIpcanTest, GetAccessInfoForWiFi)
{
    AccessNetworkInfo objANInfo;

    EXPECT_CALL(m_objSystem, GetWifiBssId())
            .Times(AnyNumber())
            .WillOnce(Return(AString("8c:3b:ad:8c:31:d0")))
            .WillOnce(Return(AString(":::::")))
            .WillOnce(Return(AString("8c:3b:ad:8c:31:d0:8c:40")))
            .WillOnce(Return(AString::ConstNull()));

    // Valid MAC address.
    m_piIpcan->GetAccessInfoForWiFi(objANInfo);

    EXPECT_EQ(objANInfo.nType, AccessNetworkInfo::TYPE_IEEE_802_11);
    EXPECT_EQ(objANInfo.nClass, AccessNetworkInfo::CLASS_NONE);
    EXPECT_STREQ(reinterpret_cast<IMS_CHAR*>(&objANInfo.uniAI.i_wlan_node_id.aMAC[0]),
            "\x8C\x3B\xAD\x8C\x31\xD0");

    // Valid MAC address & empty values.
    m_piIpcan->GetAccessInfoForWiFi(objANInfo);

    EXPECT_EQ(objANInfo.nType, AccessNetworkInfo::TYPE_IEEE_802_11);
    EXPECT_EQ(objANInfo.nClass, AccessNetworkInfo::CLASS_NONE);
    EXPECT_STREQ(reinterpret_cast<IMS_CHAR*>(&objANInfo.uniAI.i_wlan_node_id.aMAC[0]),
            "\xFF\xFF\xFF\xFF\xFF\xFF");

    // Invalid MAC address - invalid format.
    m_piIpcan->GetAccessInfoForWiFi(objANInfo);

    EXPECT_EQ(objANInfo.nType, AccessNetworkInfo::TYPE_NONE);
    EXPECT_EQ(objANInfo.nClass, AccessNetworkInfo::CLASS_NONE);

    // Invalid MAC address - null or empty string.
    m_piIpcan->GetAccessInfoForWiFi(objANInfo);

    EXPECT_EQ(objANInfo.nType, AccessNetworkInfo::TYPE_IEEE_802_11);
    EXPECT_EQ(objANInfo.nClass, AccessNetworkInfo::CLASS_NONE);
    EXPECT_STREQ(reinterpret_cast<IMS_CHAR*>(&objANInfo.uniAI.i_wlan_node_id.aMAC[0]),
            "\x00\x00\x00\x00\x00\x00");
}

}  // namespace android
