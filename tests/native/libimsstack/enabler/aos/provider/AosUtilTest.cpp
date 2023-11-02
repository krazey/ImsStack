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
#include <gmock/gmock.h>

#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"

#include "ImsList.h"
#include "INetworkWatcher.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "msg/SipMessage.h"
#include "SipMessageBodyPart.h"
#include "interface/IAosBlock.h"
#include "provider/AosUtil.h"

using ::testing::Return;
using ::testing::ReturnNull;

const IMS_SINT32 SLOT_ID = 0;

class AosUtilTest : public ::testing::Test
{
public:
    AosUtil* pAosUtil;

    enum
    {
        FEATURE_NONE = 0x0,
        FEATURE_SUBSCRIPTION = 0x01,
        FEATURE_IPSEC = 0x02
    };

protected:
    virtual void SetUp() override
    {
        pAosUtil = new AosUtil();
        ASSERT_TRUE(pAosUtil != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosUtil)
        {
            delete pAosUtil;
        }
    }
};

TEST_F(AosUtilTest, GetResponseCode)
{
    EXPECT_EQ(-1, pAosUtil->GetResponseCode(IMS_NULL));

    MockISipMessage objMockSipMsg;

    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(403));
    EXPECT_EQ(403, pAosUtil->GetResponseCode(static_cast<ISipMessage*>(&objMockSipMsg)));

    EXPECT_CALL(objMockSipMsg, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_EQ(200, pAosUtil->GetResponseCode(static_cast<ISipMessage*>(&objMockSipMsg)));
}

TEST_F(AosUtilTest, GetRetryAfterValue)
{
    MockISipMessage objMockSipMsg;
    MockIRegistration objMockIRegistration;
    EXPECT_CALL(objMockIRegistration, GetPreviousResponse())
            .WillOnce(ReturnNull())
            .WillRepeatedly(Return(static_cast<ISipMessage*>(&objMockSipMsg)));

    AString strHeader = "";
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strHeader));

    EXPECT_EQ(0, pAosUtil->GetRetryAfterValue(static_cast<IRegistration*>(&objMockIRegistration)));
    EXPECT_EQ(0, pAosUtil->GetRetryAfterValue(static_cast<IRegistration*>(&objMockIRegistration)));

    strHeader.Append("60");
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillOnce(Return(strHeader));

    EXPECT_EQ(60, pAosUtil->GetRetryAfterValue(static_cast<IRegistration*>(&objMockIRegistration)));
}

TEST_F(AosUtilTest, GetMinExpiresValue)
{
    EXPECT_EQ(-1, pAosUtil->GetMinExpiresValue(IMS_NULL));

    MockISipMessage objMockSipMsg;
    AString strHeader = "";
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillOnce(Return(strHeader));
    EXPECT_EQ(-1, pAosUtil->GetMinExpiresValue(static_cast<ISipMessage*>(&objMockSipMsg)));

    strHeader.Append("600");
    EXPECT_CALL(objMockSipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillOnce(Return(strHeader));
    EXPECT_EQ(600, pAosUtil->GetMinExpiresValue(static_cast<ISipMessage*>(&objMockSipMsg)));
}

TEST_F(AosUtilTest, IsInitialRegistrationRequired)
{
    EXPECT_FALSE(pAosUtil->IsInitialRegistrationRequired(IMS_NULL));

    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Clear();

    MockISipMessage objMockSipMsg;
    EXPECT_CALL(objMockSipMsg, GetBodyParts()).WillOnce(Return(objBodyParts));
    EXPECT_FALSE(
            pAosUtil->IsInitialRegistrationRequired(static_cast<ISipMessage*>(&objMockSipMsg)));

    SipMessageBodyPart objBodyPart;
    ISipMessageBodyPart* piBodyPart = static_cast<ISipMessageBodyPart*>(&objBodyPart);

    AString strContent = "";
    strContent.Append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    strContent.Append("<ims-3gpp version=\"1\"><alternative-service>");
    strContent.Append("<type>restoration</type>");
    strContent.Append("<reason></reason>");
    strContent.Append("<action>initial-registration</action>");
    strContent.Append("</alternative-service></ims-3gpp>");
    ByteArray objContent(strContent);

    piBodyPart->SetContent(objContent);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, "application/3gpp-ims+xml");

    objBodyParts.Append(piBodyPart);

    EXPECT_CALL(objMockSipMsg, GetBodyParts()).WillOnce(Return(objBodyParts));
    EXPECT_TRUE(pAosUtil->IsInitialRegistrationRequired(static_cast<ISipMessage*>(&objMockSipMsg)));
}

TEST_F(AosUtilTest, IsParameterIncluded3)
{
    IMS_SINT32 nHeaderType = 0;
    AString strParameter = "test";
    EXPECT_FALSE(pAosUtil->IsParameterIncluded(IMS_NULL, nHeaderType, strParameter));

    MockISipMessage objMockSipMsg;
    ImsList<AString> objHeaders;
    objHeaders.Clear();
    EXPECT_CALL(objMockSipMsg, GetHeaders(nHeaderType, (AString::ConstNull())))
            .WillOnce(Return(objHeaders));
    EXPECT_FALSE(pAosUtil->IsParameterIncluded(
            static_cast<ISipMessage*>(&objMockSipMsg), nHeaderType, strParameter));

    AString strHeader = "";
    objHeaders.Append(strHeader);
    EXPECT_CALL(objMockSipMsg, GetHeaders(nHeaderType, (AString::ConstNull())))
            .WillOnce(Return(objHeaders));
    EXPECT_FALSE(pAosUtil->IsParameterIncluded(
            static_cast<ISipMessage*>(&objMockSipMsg), nHeaderType, strParameter));

    strHeader = "test";
    objHeaders.Append(strHeader);
    EXPECT_CALL(objMockSipMsg, GetHeaders(nHeaderType, (AString::ConstNull())))
            .WillOnce(Return(objHeaders));
    EXPECT_TRUE(pAosUtil->IsParameterIncluded(
            static_cast<ISipMessage*>(&objMockSipMsg), nHeaderType, strParameter));
}

TEST_F(AosUtilTest, IsParameterIncluded4)
{
    IMS_SINT32 nHeaderType = 0;
    AString strName = "name";
    AString strParameter = "test";
    EXPECT_FALSE(pAosUtil->IsParameterIncluded(IMS_NULL, nHeaderType, strName, strParameter));

    MockISipMessage objMockSipMsg;
    ImsList<AString> objHeaders;
    objHeaders.Clear();
    EXPECT_CALL(objMockSipMsg, GetHeaders(nHeaderType, strName)).WillOnce(Return(objHeaders));
    EXPECT_FALSE(pAosUtil->IsParameterIncluded(
            static_cast<ISipMessage*>(&objMockSipMsg), nHeaderType, strName, strParameter));

    AString strHeader = "";
    objHeaders.Append(strHeader);
    strHeader = "test";
    objHeaders.Append(strHeader);
    EXPECT_CALL(objMockSipMsg, GetHeaders(nHeaderType, strName)).WillOnce(Return(objHeaders));
    EXPECT_TRUE(pAosUtil->IsParameterIncluded(
            static_cast<ISipMessage*>(&objMockSipMsg), nHeaderType, strName, strParameter));
}

TEST_F(AosUtilTest, GetLocalPort)
{
    EXPECT_EQ(-1, pAosUtil->GetLocalPort(SLOT_ID));
}

TEST_F(AosUtilTest, CheckFeature)
{
    IMS_UINT32 nFeatures = 0;
    pAosUtil->AddFeature(FEATURE_SUBSCRIPTION, nFeatures);
    EXPECT_TRUE(pAosUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, nFeatures));

    pAosUtil->AddFeature(FEATURE_IPSEC, nFeatures);
    EXPECT_TRUE(pAosUtil->IsFeatureOn(FEATURE_IPSEC, nFeatures));

    pAosUtil->RemoveFeature(FEATURE_IPSEC, nFeatures);
    EXPECT_FALSE(pAosUtil->IsFeatureOn(FEATURE_IPSEC, nFeatures));

    pAosUtil->ClearFeature(nFeatures);
}

TEST_F(AosUtilTest, SetRetryTimeDuration)
{
    EXPECT_EQ(30, pAosUtil->CalculateUpperBoundTime(30, 1800, 0));
    EXPECT_LE(30, pAosUtil->CalculateUpperBoundTime(30, 1800, 1));
    EXPECT_GE(1800, pAosUtil->CalculateUpperBoundTime(30, 1800, 1));

    EXPECT_LE(30, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 1));
    EXPECT_LE(60, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 2));
    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 1));
}

TEST_F(AosUtilTest, SetRetryTimeDurationExecption)
{
    EXPECT_EQ(1800, pAosUtil->CalculateUpperBoundTime(30, 1800, 30));
    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(30, 1800, 30));

    EXPECT_EQ(1800, pAosUtil->CalculateUpperBoundTime(1800, 1800, 1));
    EXPECT_EQ(1800, pAosUtil->CalculateUpperBoundTime(1800, 1800, 30));

    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(1800, 1800, 1));
    EXPECT_GE(1800, pAosUtil->WaitTimeForFlowRecovery(1800, 1800, 30));
}

TEST_F(AosUtilTest, CompareList)
{
    AStringArray leftArray;
    leftArray.AddElement("1234@ims.google.com");
    leftArray.AddElement("sip:1234@ims.google.com");
    leftArray.AddElement("tel:+1234@ims.google.com");

    AStringArray leftArrayOutOfOrder;
    leftArrayOutOfOrder.AddElement("tel:+1234@ims.google.com");
    leftArrayOutOfOrder.AddElement("1234@ims.google.com");
    leftArrayOutOfOrder.AddElement("sip:1234@ims.google.com");

    AStringArray leftArrayAllDiff;
    leftArrayAllDiff.AddElement("1234@google.com");
    leftArrayAllDiff.AddElement("sip:4567@ims.google.com");
    leftArrayAllDiff.AddElement("tel:+8901@ims.google.com");

    AString leftArrayExist = "tel:+1234@ims.google.com";
    AString leftArrayNotExist = "sip:4567@ims.google.com";

    AStringArray rightArray;
    rightArray.AddElement("1234@ims.google.com");
    rightArray.AddElement("sip:1234@ims.google.com");
    rightArray.AddElement("tel:+1234@ims.google.com");

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_FALSE));

    leftArray.AddElement("+1234@ims.google.com");
    EXPECT_FALSE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_FALSE));

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArrayOutOfOrder, rightArray, IMS_FALSE));

    EXPECT_FALSE(pAosUtil->IsListEqual(leftArrayAllDiff, rightArray, IMS_FALSE));

    EXPECT_TRUE(pAosUtil->IsStrExistInList(leftArrayExist, rightArray, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsStrExistInList(leftArrayNotExist, rightArray, IMS_FALSE));
}

TEST_F(AosUtilTest, CompareListIPv4)
{
    AStringArray leftArray;
    leftArray.AddElement("10.168.219.102");
    leftArray.AddElement("10.168.219.104");
    leftArray.AddElement("10.168.219.106");

    AStringArray leftArrayOutOfOrder;
    leftArrayOutOfOrder.AddElement("10.168.219.104");
    leftArrayOutOfOrder.AddElement("10.168.219.106");
    leftArrayOutOfOrder.AddElement("10.168.219.102");

    AStringArray leftArrayAllDiff;
    leftArrayAllDiff.AddElement("10.168.216.104");
    leftArrayAllDiff.AddElement("10.168.216.106");
    leftArrayAllDiff.AddElement("10.168.216.102");

    AString leftArrayExist = "10.168.219.106";
    AString leftArrayNotExist = "10.168.216.102";

    AStringArray rightArray;
    rightArray.AddElement("10.168.219.102");
    rightArray.AddElement("10.168.219.104");
    rightArray.AddElement("10.168.219.106");

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArrayOutOfOrder, rightArray, IMS_TRUE));

    EXPECT_FALSE(pAosUtil->IsListEqual(leftArrayAllDiff, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsStrExistInList(leftArrayExist, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsStrExistInList(leftArrayNotExist, rightArray, IMS_TRUE));
}

TEST_F(AosUtilTest, CompareListIPv6)
{
    AStringArray leftArray;
    leftArray.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    leftArray.AddElement("240a:3:4400:3420::7");
    leftArray.AddElement("fc01:abab:cdcd:6fee::1");

    AStringArray leftArrayOutOfOrder;
    leftArrayOutOfOrder.AddElement("fc01:abab:cdcd:6fee::1");
    leftArrayOutOfOrder.AddElement("240a:3:4400:3420::7");
    leftArrayOutOfOrder.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");

    AStringArray leftArrayAllDiff;
    leftArrayAllDiff.AddElement("240a:3:4400:3420::6");
    leftArrayAllDiff.AddElement("fc02:abab:cdcd:6fee::1");
    leftArrayAllDiff.AddElement("fd29:cc42:7fb9:2:20c:29ff:fe66:b4c7");

    AString leftArrayExist = "240a:3:4400:3420::7";
    AString leftArrayNotExist = "fd29:cc42:7fb9:2:20c:29ff:fe66:b4c7";

    AStringArray rightArray;
    rightArray.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    rightArray.AddElement("240a:3:4400:3420:0:0:0:7");
    rightArray.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArray, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsListEqual(leftArrayOutOfOrder, rightArray, IMS_TRUE));

    EXPECT_FALSE(pAosUtil->IsListEqual(leftArrayAllDiff, rightArray, IMS_TRUE));

    EXPECT_TRUE(pAosUtil->IsStrExistInList(leftArrayExist, rightArray, IMS_TRUE));
    EXPECT_FALSE(pAosUtil->IsStrExistInList(leftArrayNotExist, rightArray, IMS_TRUE));
}

TEST_F(AosUtilTest, ManageIntList)
{
    ImsList<IMS_UINT32> reasons;
    ImsList<IMS_UINT32> compareReasons;
    reasons.Clear();
    compareReasons.Clear();

    pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, reasons);
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);

    pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, compareReasons);
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, compareReasons);

    EXPECT_TRUE(pAosUtil->IsListEqual(reasons, compareReasons, IMS_TRUE));
    EXPECT_TRUE(pAosUtil->IsElementExistInList(compareReasons, reasons));

    reasons.Clear();
    pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, reasons);
    pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, reasons);
    EXPECT_TRUE(pAosUtil->IsListEqual(reasons, compareReasons, IMS_FALSE));
    EXPECT_FALSE(pAosUtil->IsListEqual(reasons, compareReasons, IMS_TRUE));

    pAosUtil->AddElementToList(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, reasons);
    EXPECT_TRUE(pAosUtil->IsElementExistInList(compareReasons, reasons));
    EXPECT_FALSE(pAosUtil->IsListEqual(reasons, compareReasons, IMS_FALSE));

    reasons.Clear();
    pAosUtil->AddElementToList(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, reasons);
    pAosUtil->AddElementToList(BLOCK_CELLULAR_VOPS_OFF, reasons);
    EXPECT_FALSE(pAosUtil->IsElementExistInList(compareReasons, reasons));
}

TEST_F(AosUtilTest, checkNetworkType)
{
    EXPECT_TRUE(pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_WCDMA));

    EXPECT_TRUE(pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_NR));
    EXPECT_FALSE(pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_GSM));
}

TEST_F(AosUtilTest, checkSet)
{
    pAosUtil->SetWifiTest(IMS_TRUE);
    EXPECT_TRUE(pAosUtil->IsWifiTest());
    pAosUtil->SetWifiTest(IMS_FALSE);
    EXPECT_FALSE(pAosUtil->IsWifiTest());
}