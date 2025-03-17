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

#include "AString.h"
#include "AStringArray.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoDevice.h"
#include "MockISubscriberConfig.h"
#include "PlatformContext.h"
#include "TestNetworkService.h"
#include "TestPhoneInfoService.h"
#include "call/MockIMtcCallContext.h"
#include "call/message/TemplateFormatter.h"
#include "device/OsLocationInfo.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Unused;

LOCAL const IMS_SINT32 SLOT_ID = 0;

namespace android
{

class TemplateFormatterTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    TestPhoneInfoService objPhoneInfoService;
    TestNetworkService objNetworkService;
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    MockISubscriberConfig objSubscriberConfig;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetAosConnector(_)).WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetSubscriberConfig).WillByDefault(Return(&objSubscriberConfig));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &objNetworkService);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
    }
};

TEST_F(TemplateFormatterTest, FormatWithPlainText)
{
    EXPECT_STREQ("text", TemplateFormatter::Format("text", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithImei)
{
    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strDeviceId)
                    {
                        strDeviceId = "123456789012345";
                        return IMS_TRUE;
                    }));

    EXPECT_STREQ("<123456789012340>", TemplateFormatter::Format("<#IMEI#>", objContext).GetStr());

    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strDeviceId)
                    {
                        strDeviceId = "1234567890";
                        return IMS_TRUE;
                    }));

    EXPECT_STREQ("<1234567890>", TemplateFormatter::Format("<#IMEI#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithImsi)
{
    ON_CALL(objPhoneInfoService.GetMockSubscriberInfo(), GetSubscriberId(_))
            .WillByDefault(Invoke(
                    [](OUT AString& strImsi)
                    {
                        strImsi = "123456789012345";
                        return IMS_TRUE;
                    }));

    EXPECT_STREQ("<123456789012345>", TemplateFormatter::Format("<#IMSI#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithMacAddress)
{
    ON_CALL(objNetworkService.GetMockConnection(), GetExtraInfo(AString("mac_address"), _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strInfo)
                    {
                        strInfo = "XX-XX-XX-XX-XX-XX";
                        return IMS_TRUE;
                    }));

    EXPECT_STREQ("<XX-XX-XX-XX-XX-XX>", TemplateFormatter::Format("<#MAC#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithIpv4Address)
{
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(AString("127.0.0.1")));

    EXPECT_STREQ("<127.0.0.1>", TemplateFormatter::Format("<#IP#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithIpv6Address)
{
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(AString("::1")));

    EXPECT_STREQ("<[::1]>", TemplateFormatter::Format("<#IP#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithPort)
{
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(5060));

    EXPECT_STREQ("<5060>", TemplateFormatter::Format("<#PORT#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithPublicUserId)
{
    AStringArray lstPuids;
    lstPuids.AddElement("sip:0000@ims.mnc0.mcc0.3gppnetwork.org");
    lstPuids.AddElement("sip:+0000@abc.com");
    lstPuids.AddElement("tel:0000");

    ON_CALL(objSubscriberConfig, GetPublicUserIds).WillByDefault(ReturnRef(lstPuids));

    EXPECT_STREQ("<sip:0000@ims.mnc0.mcc0.3gppnetwork.org>",
            TemplateFormatter::Format("<#PUID#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithAid)
{
    const AString strCountry("KR");
    ON_CALL(objPhoneInfoService.GetMockLocationInfo(), GetLastKnownCountry)
            .WillByDefault(ReturnRef(strCountry));
    ON_CALL(objPhoneInfoService.GetMockCallInfo(), GetWifiCallingAddressId)
            .WillByDefault(Return(AString("some_aid")));

    EXPECT_STREQ("<some_aid>", TemplateFormatter::Format("<#AID#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithNoAid)
{
    const AString strCountry("KR");
    ON_CALL(objPhoneInfoService.GetMockLocationInfo(), GetLastKnownCountry)
            .WillByDefault(ReturnRef(strCountry));
    AString strAid(AString::ConstNull());
    ON_CALL(objPhoneInfoService.GetMockCallInfo(), GetWifiCallingAddressId)
            .WillByDefault(Return(strAid));

    EXPECT_STREQ(
            "<0000:0000:0000:0000>", TemplateFormatter::Format("<#AID#>", objContext).GetStr());
}

TEST_F(TemplateFormatterTest, FormatWithAidAndNoCountry)
{
    const AString strNoCountry(OsLocationInfo::COUNTRY_ISO_UNKNOWN);
    ON_CALL(objPhoneInfoService.GetMockLocationInfo(), GetLastKnownCountry)
            .WillByDefault(ReturnRef(strNoCountry));
    EXPECT_CALL(objPhoneInfoService.GetMockCallInfo(), GetWifiCallingAddressId).Times(0);

    EXPECT_STREQ(
            "<0000:0000:0000:0000>", TemplateFormatter::Format("<#AID#>", objContext).GetStr());
}

}  // namespace android
