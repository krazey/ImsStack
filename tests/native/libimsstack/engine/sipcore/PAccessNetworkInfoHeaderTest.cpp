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
#include "TestNetworkService.h"

#include "ISipConfig.h"

#include "ISipRtConfigHelper.h"
#include "MockISipMessage.h"
#include "PAccessNetworkInfoHeader.h"
#include "SipFactory.h"
#include "SipHeaderName.h"
#include "SipProfile.h"
#include "SipRtConfig.h"

using ::testing::_;
using ::testing::AnyNumber;
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
            m_pPhoneInfoService(new TestPhoneInfoService()),
            m_pNetworkService(new TestNetworkService())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, m_pPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, m_pNetworkService);
    }

    inline virtual ~PAccessNetworkInfoHeaderTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        if (m_pPhoneInfoService != IMS_NULL)
        {
            m_pPhoneInfoService->Destroy();
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);

        if (m_pNetworkService != IMS_NULL)
        {
            delete m_pNetworkService;
        }
    }

protected:
    virtual void SetUp() override
    {
        m_pNetworkService->SetConnection(&m_objNetworkConnection);

        // To disable all the features, set an arbitrary SIP feature
        m_pSipProfile->SetSipFeatureCaps(ISipConfig::SIP_FEATURE_CAPS_IPSEC);
        // Sets the policy to show MAC address in PANI header.
        m_pSipProfile->SetHideMacInPaniHeaderPolicy(ISipConfig::SHOW_MAC_IN_PANI);

        SetAccessNetworkInfoForDefault();

        ON_CALL(m_objNetworkConnection, GetAccessNetworkInfo(_))
                .WillByDefault(Invoke(this, &PAccessNetworkInfoHeaderTest::GetAccessNetworkInfo));
        ON_CALL(m_objNetworkConnection, GetLastAccessNetworkInfo(_, _, _))
                .WillByDefault(
                        Invoke(this, &PAccessNetworkInfoHeaderTest::GetLastAccessNetworkInfo));
    }

    virtual void TearDown() override {}

    void GetAccessNetworkInfo(AccessNetworkInfo& objAni)
    {
        IMS_MEM_Memcpy(&objAni, &m_objAni, sizeof(AccessNetworkInfo));
    }

    void GetLastAccessNetworkInfo(OUT AccessNetworkInfo& objAccessNetInfo,
            OUT AString& strTimestamp, OUT AString& strCellInfoAge)
    {
        IMS_MEM_Memcpy(&objAccessNetInfo, &m_objAni, sizeof(AccessNetworkInfo));
        strTimestamp = "08:56:31Z";
        strCellInfoAge = "60";
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
    TestNetworkService* m_pNetworkService;
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

    m_pSipProfile->SetHideMacInPaniHeaderPolicy(ISipConfig::HIDE_MAC_IN_PANI);
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

TEST_F(PAccessNetworkInfoHeaderTest, FormHeaderUsingIpAddress)
{
    IpAddress objIpAddress(AString("192.168.1.3"));

    AString strHeader;
    AString strExpected("IEEE-802.11;i-wlan-node-id=010203040506");

    SetAccessNetworkInfoForWifi();

    m_pNetworkService->SetConnection(IMS_NULL);

    EXPECT_FALSE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, objIpAddress, m_objMethod, m_pSipProfile.Get(), strHeader));

    m_pNetworkService->SetConnection(&m_objNetworkConnection);

    ASSERT_TRUE(PAccessNetworkInfoHeader::FormHeader(
            IMS_SLOT_0, objIpAddress, m_objMethod, m_pSipProfile.Get(), strHeader));
    EXPECT_EQ(strHeader, strExpected);
}

TEST_F(PAccessNetworkInfoHeaderTest, SetHeader)
{
    IpAddress objIpAddress(AString("192.168.1.3"));
    ISipMessage* piSipMsg = IMS_NULL;

    // Passing sip message as null, no action
    PAccessNetworkInfoHeader::SetHeader(IMS_SLOT_0, objIpAddress, m_pSipProfile.Get(), piSipMsg);

    m_pNetworkService->SetConnection(IMS_NULL);

    MockISipMessage objMockISipMessage;
    piSipMsg = &objMockISipMessage;

    // Connection is null, no action
    PAccessNetworkInfoHeader::SetHeader(IMS_SLOT_0, objIpAddress, m_pSipProfile.Get(), piSipMsg);

    m_pNetworkService->SetConnection(&m_objNetworkConnection);

    SipRtConfig::Header objPlciHeader;

    objPlciHeader.strName = "P-Last-Cell-ID";
    objPlciHeader.strParameter = "\\2023-03-05T13%3A15%3A30Z\\";

    ISipRtConfigHelper* piConfHelper = SipFactory::GetRtConfigHelper(IMS_SLOT_0);
    piConfHelper->SetConfig(SipRtConfig::CONFIG_I_SIP_HEADER, &objPlciHeader);

    SipRtConfig::Header objCniHeader;

    objCniHeader.strName = SipHeaderName::CELLULAR_NETWORK_INFO;
    objCniHeader.strParameter = "\\2023-03-05T13%3A15%3A32Z\\";

    piConfHelper->SetConfig(SipRtConfig::CONFIG_I_SIP_HEADER, &objCniHeader);

    SipRtConfig::Header objPlaniHeader;

    objPlaniHeader.strName = "P-Last-Access-Network-Info";
    objPlaniHeader.strParameter = "\\2023-03-05T13%3A15%3A10Z\\";

    piConfHelper->SetConfig(SipRtConfig::CONFIG_I_SIP_HEADER, &objPlaniHeader);

    EXPECT_CALL(m_objNetworkConnection, IsePDGEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_pPhoneInfoService->GetMockNetworkWatcher(), GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));
    EXPECT_CALL(objMockISipMessage, GetMethod()).Times(4);
    EXPECT_CALL(m_objNetworkConnection, GetExtraInfo(_, _)).Times(AnyNumber());

    ON_CALL(objMockISipMessage, GetMethod()).WillByDefault(ReturnRef(m_objMethod));

    ON_CALL(m_objNetworkConnection, GetExtraInfo(_, _))
            .WillByDefault(Invoke(
                    [](IN const AString&, OUT AString& strInfo)
                    {
                        strInfo = "mobile_ims";
                        return IMS_TRUE;
                    }));

    EXPECT_CALL(m_objNetworkConnection, GetAccessNetworkInfo(_)).Times(1);
    EXPECT_CALL(m_objNetworkConnection, GetLastAccessNetworkInfo(_, _, _)).Times(3);
    EXPECT_CALL(objMockISipMessage, SetHeader(ISipHeader::UNKNOWN, _, _))
            .Times(3)
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockISipMessage, SetHeader(ISipHeader::P_ACCESS_NETWORK_INFO, _, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_SUCCESS));

    SetAccessNetworkInfoForWifi();

    PAccessNetworkInfoHeader::SetHeader(IMS_SLOT_0, objIpAddress, m_pSipProfile.Get(), piSipMsg);

    piConfHelper->RemoveConfig(SipRtConfig::CONFIG_I_SIP_HEADER, &objPlciHeader);
    piConfHelper->RemoveConfig(SipRtConfig::CONFIG_I_SIP_HEADER, &objCniHeader);
    piConfHelper->RemoveConfig(SipRtConfig::CONFIG_I_SIP_HEADER, &objPlaniHeader);
}

}  // namespace android
