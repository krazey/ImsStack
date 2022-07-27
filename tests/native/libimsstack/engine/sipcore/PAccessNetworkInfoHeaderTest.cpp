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

#include "ImsNew.h"
#include "MockINetworkConnection.h"
#include "MockIOsFactory.h"
#include "MockIPhoneInfoLocation.h"
#include "MockISystem.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"

#include "ISipConfig.h"

#include "PAccessNetworkInfoHeader.h"
#include "SipProfile.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StartsWith;

namespace android
{

class PAccessNetworkInfoHeaderTest : public ::testing::Test
{
public:
    inline PAccessNetworkInfoHeaderTest() :
            m_objMethod(SipMethod::INVITE),
            m_pSipProfile(new SipProfile()),
            m_pPhoneInfoService(new TestPhoneInfoService())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, m_pPhoneInfoService);
    }
    inline virtual ~PAccessNetworkInfoHeaderTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        if (m_pPhoneInfoService != IMS_NULL)
        {
            m_pPhoneInfoService->Destroy();
        }
    }

protected:
    virtual void SetUp() override
    {
        // To disable all the features, set an arbitrary SIP feature
        m_pSipProfile->SetSipFeatureCaps(ISipConfig::SIP_FEATURE_CAPS_IPSEC);

        SetAccessNetworkInfoForDefault();

        ON_CALL(m_objNetworkConnection, GetAccessNetworkInfo(_))
                .WillByDefault(Invoke(this, &PAccessNetworkInfoHeaderTest::GetAccessNetworkInfo));
    }

    virtual void TearDown() override {}

    void GetAccessNetworkInfo(AccessNetworkInfo& objAni)
    {
        IMS_MEM_Memcpy(&objAni, &m_objAni, sizeof(AccessNetworkInfo));
    }

    void SetAccessNetworkInfoForDefault() { m_objAni = AccessNetworkInfo(); }

    void SetAccessNetworkInfoForWifi()
    {
        m_objAni.nType = AccessNetworkInfo::TYPE_IEEE_802_11;
        m_objAni.nClass = AccessNetworkInfo::CLASS_NONE;

        for (IMS_SINT32 i = 0; i < ANI_WLAN_MAX_MAC; ++i)
        {
            m_objAni.uniAI.i_wlan_node_id.aMAC[i] = static_cast<IMS_UINT8>(0xFF & (i + 1));
        }
    }

protected:
    MockINetworkConnection m_objNetworkConnection;

    SipMethod m_objMethod;
    RcPtr<SipProfile> m_pSipProfile;
    AccessNetworkInfo m_objAni;
    TestPhoneInfoService* m_pPhoneInfoService;
};

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingNetworkConnectionWithInvalidTypeOrClass)
{
    AString strHeader;

    EXPECT_FALSE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, IMS_NULL, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_FALSE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, &m_objNetworkConnection, m_objMethod, m_pSipProfile.Get(), strHeader));
}

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingNetworkConnectionWithIwlan)
{
    AString strHeader;
    AString strExpected("IEEE-802.11;i-wlan-node-id=010203040506");

    SetAccessNetworkInfoForWifi();
    ASSERT_TRUE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, &m_objNetworkConnection, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_EQ(strHeader, strExpected);
}

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingNetworkConnectionWithIwlanInvalidMacAddress)
{
    AString strHeader;
    AString strExpected("IEEE-802.11;i-wlan-node-id=000000000000");

    m_pSipProfile->SetSipFeatureCaps(m_pSipProfile->GetSipFeatureCaps() |
            ISipConfig::SIP_FEATURE_CAPS_HIDE_MAC_ADDRESS_IN_PANI_HEADER);
    SetAccessNetworkInfoForWifi();
    ASSERT_TRUE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, &m_objNetworkConnection, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_EQ(strHeader, strExpected);
}

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingNetworkConnectionWithIwlanLocalTimezone)
{
    AString strHeader;
    AString strExpected("IEEE-802.11;i-wlan-node-id=010203040506;local-time-zone=");

    m_pSipProfile->SetSipFeatureCaps(m_pSipProfile->GetSipFeatureCaps() |
            ISipConfig::SIP_FEATURE_CAPS_LOCAL_TIMEZONE_PARAM_IN_PANI_HEADER);
    SetAccessNetworkInfoForWifi();
    ASSERT_TRUE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, &m_objNetworkConnection, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_THAT(strHeader.GetStr(), StartsWith(strExpected.GetStr()));
}

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingNetworkConnectionWithIwlanCountry)
{
    MockILocationProperties objLocationProperties;
    AString strCountry("KR");

    ON_CALL(m_pPhoneInfoService->GetMockLocationInfo(),
            GetLocationProperties(Eq(ILocationInfo::LOCATION_POSITION_N_COUNTRY)))
            .WillByDefault(Return(&objLocationProperties));
    ON_CALL(objLocationProperties, GetCountry()).WillByDefault(ReturnRef(strCountry));

    AString strHeader;
    AString strExpected("IEEE-802.11;i-wlan-node-id=010203040506;country=KR");

    m_pSipProfile->SetSipFeatureCaps(m_pSipProfile->GetSipFeatureCaps() |
            ISipConfig::SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER);
    SetAccessNetworkInfoForWifi();
    ASSERT_TRUE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, &m_objNetworkConnection, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_EQ(strHeader, strExpected);
}

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingNetworkConnectionWithIwlanInvalidCountry)
{
    MockILocationProperties objLocationProperties;
    AString strCountry("ZZ");

    ON_CALL(m_pPhoneInfoService->GetMockLocationInfo(),
            GetLocationProperties(Eq(ILocationInfo::LOCATION_POSITION_N_COUNTRY)))
            .WillByDefault(Return(&objLocationProperties));
    ON_CALL(objLocationProperties, GetCountry()).WillByDefault(ReturnRef(strCountry));

    AString strHeader;
    AString strExpected("IEEE-802.11;i-wlan-node-id=010203040506");

    m_pSipProfile->SetSipFeatureCaps(m_pSipProfile->GetSipFeatureCaps() |
            ISipConfig::SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER);
    SetAccessNetworkInfoForWifi();
    ASSERT_TRUE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, &m_objNetworkConnection, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_EQ(strHeader, strExpected);
    EXPECT_EQ(strCountry, objLocationProperties.GetCountry());
}

}  // namespace android
