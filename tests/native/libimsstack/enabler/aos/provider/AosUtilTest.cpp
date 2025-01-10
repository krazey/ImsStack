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
#include "msg/SipMsgUtil.h"
#include "SipMessageBodyPart.h"
#include "interface/IAosBlock.h"
#include "provider/AosUtil.h"

using ::testing::Return;
using ::testing::ReturnNull;

const IMS_SINT32 SLOT_ID = 0;
const IMS_UINT32 BASE_TIME = 30;
const IMS_UINT32 MAX_TIME = 1800;

class AosUtilTest : public ::testing::Test
{
public:
    AosUtil* m_pAosUtil;

    MockIRegistration m_objMockIRegistration;
    MockISipMessage m_objMockISipMsg;

    enum
    {
        FEATURE_NONE = 0x0,
        FEATURE_SUBSCRIPTION = 0x01,
        FEATURE_IPSEC = 0x02
    };

protected:
    void SetUp() override
    {
        ON_CALL(m_objMockIRegistration, GetPreviousResponse())
                .WillByDefault(Return(&m_objMockISipMsg));

        m_pAosUtil = new AosUtil();
        ASSERT_TRUE(m_pAosUtil != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosUtil)
        {
            delete m_pAosUtil;
        }
    }
};

/// GetResponseCode()
TEST_F(AosUtilTest, ReturnInvalidValueWhenSipMsgIsNullInGetResponseCode)
{
    EXPECT_EQ(m_pAosUtil->GetResponseCode(IMS_NULL), -1);
}

TEST_F(AosUtilTest, ReturnStatusCodeWhenGetResponseCodeIsCalled)
{
    // 403 return value
    EXPECT_CALL(m_objMockISipMsg, GetStatusCode()).WillOnce(Return(403));

    EXPECT_EQ(m_pAosUtil->GetResponseCode(&m_objMockISipMsg), 403);
}

/// GetRetryAfterValue()
TEST_F(AosUtilTest, ReturnZeroValueWhenSipMsgIsNullInGetRetryAfterValue)
{
    ON_CALL(m_objMockIRegistration, GetPreviousResponse()).WillByDefault(ReturnNull());

    EXPECT_EQ(m_pAosUtil->GetRetryAfterValue(&m_objMockIRegistration), 0);
}

TEST_F(AosUtilTest, ReturnZeroValueWhenThereIsNoRetryAfterValue)
{
    AString strHeader = "";
    ON_CALL(m_objMockISipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillByDefault(Return(strHeader));

    EXPECT_EQ(m_pAosUtil->GetRetryAfterValue(&m_objMockIRegistration), 0);
}

TEST_F(AosUtilTest, ReturnRetryAfterValueWhenRetryAfterValueIsNormal)
{
    AString strHeader = "60";
    ON_CALL(m_objMockISipMsg, GetHeader(ISipHeader::RETRY_AFTER_SEC, 0, AString::ConstNull()))
            .WillByDefault(Return(strHeader));

    EXPECT_EQ(m_pAosUtil->GetRetryAfterValue(&m_objMockIRegistration), 60);
}

/// GetMinExpiresValue()
TEST_F(AosUtilTest, ReturnInvalidValueWhenSipMsgIsNullInGetMinExpiresValue)
{
    EXPECT_EQ(m_pAosUtil->GetMinExpiresValue(IMS_NULL), -1);
}

TEST_F(AosUtilTest, ReturnInvalidValueWhenMinExpiresHeaderIsNone)
{
    AString strHeader = "";
    ON_CALL(m_objMockISipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillByDefault(Return(strHeader));

    EXPECT_EQ(m_pAosUtil->GetMinExpiresValue(&m_objMockISipMsg), -1);
}

TEST_F(AosUtilTest, ReturnMinExpiresValueWhenThereIsMinExpiresHeader)
{
    AString strHeader = "600";
    ON_CALL(m_objMockISipMsg, GetHeader(ISipHeader::MIN_EXPIRES, 0, AString::ConstNull()))
            .WillByDefault(Return(strHeader));

    EXPECT_EQ(m_pAosUtil->GetMinExpiresValue(&m_objMockISipMsg), 600);
}

/// IsInitialRegistrationRequired()
TEST_F(AosUtilTest, ReturnFalseWhenSipMsgIsNullInInitRegRequired)
{
    EXPECT_FALSE(m_pAosUtil->IsInitialRegistrationRequired(IMS_NULL));
}

TEST_F(AosUtilTest, ReturnFalseWhenThereIsNoBodyInInitRegRequired)
{
    ImsList<ISipMessageBodyPart*> objBodyParts;
    ON_CALL(m_objMockISipMsg, GetBodyParts()).WillByDefault(Return(objBodyParts));

    EXPECT_FALSE(m_pAosUtil->IsInitialRegistrationRequired(&m_objMockISipMsg));
}

TEST_F(AosUtilTest, ReturnTrueWhenBodyHasActionWithInitialRegistration)
{
    ImsList<ISipMessageBodyPart*> objBodyParts;
    SipMessageBodyPart objBodyPart;
    ISipMessageBodyPart* piBodyPart = &objBodyPart;

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
    ON_CALL(m_objMockISipMsg, GetBodyParts()).WillByDefault(Return(objBodyParts));

    EXPECT_TRUE(m_pAosUtil->IsInitialRegistrationRequired(&m_objMockISipMsg));
}

/// IsParameterIncluded() - three factors
TEST_F(AosUtilTest, ReturnFalseWhenSipMsgIsNullInThreeFactorsParameterChecked)
{
    EXPECT_FALSE(m_pAosUtil->IsParameterIncluded(IMS_NULL, SipHeaderBase::ALLOW, AString("test")));
}

TEST_F(AosUtilTest, ReturnFalseWhenHeaderIsEmptyInThreeFactorsParameterChecked)
{
    ON_CALL(m_objMockISipMsg, GetHeaders(SipHeaderBase::ALLOW, (AString::ConstNull())))
            .WillByDefault(Return(ImsList<AString>()));

    EXPECT_FALSE(m_pAosUtil->IsParameterIncluded(
            &m_objMockISipMsg, SipHeaderBase::ALLOW, AString("test")));
}

TEST_F(AosUtilTest, ReturnFalseWhenThereIsEmptyParameterInThreeFactorsParameterChecked)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(AString(" "));
    ON_CALL(m_objMockISipMsg, GetHeaders(SipHeaderBase::ALLOW, (AString::ConstNull())))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(m_pAosUtil->IsParameterIncluded(
            &m_objMockISipMsg, SipHeaderBase::ALLOW, AString("test")));
}

TEST_F(AosUtilTest, ReturnTrueWhenThereIsParameterInThreeFactorsParameterChecked)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(AString("test"));
    ON_CALL(m_objMockISipMsg, GetHeaders(SipHeaderBase::ALLOW, (AString::ConstNull())))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(m_pAosUtil->IsParameterIncluded(
            &m_objMockISipMsg, SipHeaderBase::ALLOW, AString("test")));
}

/// IsParameterIncluded() - four factors
TEST_F(AosUtilTest, ReturnFalseWhenSipMsgIsNullInFourFactorsParameterChecked)
{
    EXPECT_FALSE(m_pAosUtil->IsParameterIncluded(IMS_NULL, 0, AString("name"), AString("test")));
}

TEST_F(AosUtilTest, ReturnFalseWhenHeaderIsEmptyInFourFactorsParameterChecked)
{
    AString strName = "name";

    ON_CALL(m_objMockISipMsg, GetHeaders(SipHeaderBase::ALLOW, strName))
            .WillByDefault(Return(ImsList<AString>()));

    EXPECT_FALSE(m_pAosUtil->IsParameterIncluded(
            &m_objMockISipMsg, SipHeaderBase::ALLOW, strName, AString("test")));
}

TEST_F(AosUtilTest, ReturnTrueWhenThereIsParameterInFourFactorsParameterChecked)
{
    AString strName = "name";

    ImsList<AString> objHeaders;
    objHeaders.Append(AString(" test "));
    ON_CALL(m_objMockISipMsg, GetHeaders(SipHeaderBase::ALLOW, strName))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(m_pAosUtil->IsParameterIncluded(
            &m_objMockISipMsg, SipHeaderBase::ALLOW, strName, AString("test")));
}

TEST_F(AosUtilTest, ReturnInitValueWhenGetLocalPort)
{
    EXPECT_EQ(-1, m_pAosUtil->GetLocalPort(SLOT_ID));
}

TEST_F(AosUtilTest, VerifyAddedFeatureWhenSomeFeaturesAdded)
{
    IMS_UINT32 nFeatures = 0;
    m_pAosUtil->AddFeature(FEATURE_SUBSCRIPTION, nFeatures);
    m_pAosUtil->AddFeature(FEATURE_IPSEC, nFeatures);

    EXPECT_TRUE(m_pAosUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, nFeatures));
    EXPECT_TRUE(m_pAosUtil->IsFeatureOn(FEATURE_IPSEC, nFeatures));
}

TEST_F(AosUtilTest, VerifyRemovedFeatureWhenSomeFeaturesRemoved)
{
    IMS_UINT32 nFeatures = 0;
    m_pAosUtil->AddFeature(FEATURE_SUBSCRIPTION, nFeatures);
    m_pAosUtil->AddFeature(FEATURE_IPSEC, nFeatures);
    m_pAosUtil->RemoveFeature(FEATURE_IPSEC, nFeatures);

    EXPECT_TRUE(m_pAosUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, nFeatures));
    EXPECT_FALSE(m_pAosUtil->IsFeatureOn(FEATURE_IPSEC, nFeatures));
}

TEST_F(AosUtilTest, VerifyFeatureIsClearWhenClearFeatureCalled)
{
    IMS_UINT32 nFeatures = 0;
    m_pAosUtil->AddFeature(FEATURE_SUBSCRIPTION, nFeatures);
    m_pAosUtil->AddFeature(FEATURE_IPSEC, nFeatures);

    m_pAosUtil->ClearFeature(nFeatures);

    EXPECT_FALSE(m_pAosUtil->IsFeatureOn(FEATURE_SUBSCRIPTION, nFeatures));
    EXPECT_FALSE(m_pAosUtil->IsFeatureOn(FEATURE_IPSEC, nFeatures));
}

TEST_F(AosUtilTest, UpperBoundTimeIsBaseTimeWhenFailCountIsZero)
{
    const IMS_UINT32 nFailCount = 0;

    EXPECT_EQ(m_pAosUtil->CalculateUpperBoundTime(BASE_TIME, MAX_TIME, nFailCount), BASE_TIME);
}

TEST_F(AosUtilTest, UpperBoundTimeIsBetweenBaseAndMaxWhenFailCountIsOne)
{
    const IMS_UINT32 nFailCount = 1;

    EXPECT_GE(m_pAosUtil->CalculateUpperBoundTime(BASE_TIME, MAX_TIME, nFailCount), BASE_TIME);
    EXPECT_LE(m_pAosUtil->CalculateUpperBoundTime(BASE_TIME, MAX_TIME, nFailCount), MAX_TIME);
}

TEST_F(AosUtilTest, UpperBoundTimeIsMaxWhenFailCountIsOverMax)
{
    const IMS_UINT32 nFailCount = 30;

    // REASONABLE_MAX_FAILURE_COUNT = 24
    EXPECT_EQ(m_pAosUtil->CalculateUpperBoundTime(BASE_TIME, MAX_TIME, nFailCount), MAX_TIME);
}

TEST_F(AosUtilTest, UpperBoundTimeIsTheSameValueWhenBaseAndMaxAreTheSame)
{
    const IMS_UINT32 nBaseTime = MAX_TIME;
    const IMS_UINT32 nFailCount = 1;

    EXPECT_EQ(1800, m_pAosUtil->CalculateUpperBoundTime(nBaseTime, MAX_TIME, nFailCount));
}

TEST_F(AosUtilTest, WaitTimeIsBetweenBaseAndMaxWhenFailureCountIsOne)
{
    const IMS_UINT32 nFailCount = 1;

    EXPECT_GE(m_pAosUtil->WaitTimeForFlowRecovery(BASE_TIME, MAX_TIME, nFailCount), BASE_TIME);
    EXPECT_LE(m_pAosUtil->WaitTimeForFlowRecovery(BASE_TIME, MAX_TIME, nFailCount), MAX_TIME);
}

TEST_F(AosUtilTest, WaitTimeIsBetweenTwiceBaseAndMaxWhenFailureCountIsTwo)
{
    const IMS_UINT32 nFailCount = 2;

    EXPECT_GE(m_pAosUtil->WaitTimeForFlowRecovery(BASE_TIME, MAX_TIME, nFailCount), 2 * BASE_TIME);
    EXPECT_LE(m_pAosUtil->WaitTimeForFlowRecovery(BASE_TIME, MAX_TIME, nFailCount), MAX_TIME);
}

TEST_F(AosUtilTest, WaitTimeIsLessAndEqualMaxWhenFailCountIsOverMax)
{
    const IMS_UINT32 nFailCount = 30;

    // REASONABLE_MAX_FAILURE_COUNT = 24
    EXPECT_LE(m_pAosUtil->WaitTimeForFlowRecovery(BASE_TIME, MAX_TIME, nFailCount), MAX_TIME);
}

TEST_F(AosUtilTest, WaitTimeIsLessAndEqualTheSameValueWhenBaseAndMaxAreTheSame)
{
    const IMS_UINT32 nBaseTime = MAX_TIME;
    IMS_UINT32 nFailCount = 1;

    EXPECT_LE(m_pAosUtil->WaitTimeForFlowRecovery(nBaseTime, MAX_TIME, nFailCount), MAX_TIME);

    nFailCount = 30;
    EXPECT_LE(m_pAosUtil->WaitTimeForFlowRecovery(nBaseTime, MAX_TIME, nFailCount), MAX_TIME);
}

/// IsListEqual()
TEST_F(AosUtilTest, ArraysAreEqualWhenTwoArraysAreTheSame)
{
    AStringArray objLeft;
    objLeft.AddElement("1234@ims.google.com");
    objLeft.AddElement("sip:1234@ims.google.com");
    objLeft.AddElement("tel:+1234@ims.google.com");

    AStringArray objRight;
    objRight.AddElement("1234@ims.google.com");
    objRight.AddElement("sip:1234@ims.google.com");
    objRight.AddElement("tel:+1234@ims.google.com");

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_FALSE));
}

TEST_F(AosUtilTest, ArraysAreEqualWhenTwoArraysAreInDifferentOrder)
{
    AStringArray objLeft;
    objLeft.AddElement("tel:+1234@ims.google.com");
    objLeft.AddElement("1234@ims.google.com");
    objLeft.AddElement("sip:1234@ims.google.com");

    AStringArray objRight;
    objRight.AddElement("1234@ims.google.com");
    objRight.AddElement("sip:1234@ims.google.com");
    objRight.AddElement("tel:+1234@ims.google.com");

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_FALSE));
}

TEST_F(AosUtilTest, ArraysAreNotEqualWhenTwoArraysAreSlightlyDifferent)
{
    AStringArray objLeft;
    objLeft.AddElement("1234@ims.google.com");
    objLeft.AddElement("sip:1234@ims.google.com");
    objLeft.AddElement("tel:+1234@ims.google.com");
    objLeft.AddElement("+1234@ims.google.com");

    AStringArray objRight;
    objRight.AddElement("1234@ims.google.com");
    objRight.AddElement("sip:1234@ims.google.com");
    objRight.AddElement("tel:+1234@ims.google.com");

    EXPECT_FALSE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_FALSE));
}

TEST_F(AosUtilTest, ArraysAreNotEqualWhenTwoArraysAreDifferent)
{
    AStringArray objLeft;
    objLeft.AddElement("1234@google.com");
    objLeft.AddElement("sip:4567@ims.google.com");
    objLeft.AddElement("tel:+8901@ims.google.com");

    AStringArray objRight;
    objRight.AddElement("1234@ims.google.com");
    objRight.AddElement("sip:1234@ims.google.com");
    objRight.AddElement("tel:+1234@ims.google.com");

    EXPECT_FALSE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_FALSE));
}

/// IsStrExistInList()
TEST_F(AosUtilTest, ExpectTrueWhenLeftValueIsExistInRightArray)
{
    AString strLeft = "tel:+1234@ims.google.com";

    AStringArray objRight;
    objRight.AddElement("1234@ims.google.com");
    objRight.AddElement("sip:1234@ims.google.com");
    objRight.AddElement("tel:+1234@ims.google.com");

    EXPECT_TRUE(m_pAosUtil->IsStrExistInList(strLeft, objRight, IMS_FALSE));
}

TEST_F(AosUtilTest, ExpectFalseWhenLeftValueIsNotExistInRightArray)
{
    AString strLeft = "sip:4567@ims.google.com";

    AStringArray objRight;
    objRight.AddElement("1234@ims.google.com");
    objRight.AddElement("sip:1234@ims.google.com");
    objRight.AddElement("tel:+1234@ims.google.com");

    EXPECT_FALSE(m_pAosUtil->IsStrExistInList(strLeft, objRight, IMS_FALSE));
}

/// IsListEqual() for Ip Address
TEST_F(AosUtilTest, ArraysAreEqualWhenTwoIpv4ArraysAreTheSame)
{
    AStringArray objLeft;
    objLeft.AddElement("10.168.219.102");
    objLeft.AddElement("10.168.219.104");
    objLeft.AddElement("10.168.219.106");

    AStringArray objRight;
    objRight.AddElement("10.168.219.102");
    objRight.AddElement("10.168.219.104");
    objRight.AddElement("10.168.219.106");

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_TRUE));
}

TEST_F(AosUtilTest, ArraysAreEqualWhenTwoIpv4ArraysAreInDifferentOrder)
{
    AStringArray objLeft;
    objLeft.AddElement("10.168.219.104");
    objLeft.AddElement("10.168.219.106");
    objLeft.AddElement("10.168.219.102");

    AStringArray objRight;
    objRight.AddElement("10.168.219.102");
    objRight.AddElement("10.168.219.104");
    objRight.AddElement("10.168.219.106");

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_TRUE));
}

TEST_F(AosUtilTest, ArraysAreNotEqualWhenTwoIpv4ArraysAreDifferent)
{
    AStringArray objLeft;
    objLeft.AddElement("10.168.216.104");
    objLeft.AddElement("10.168.216.106");
    objLeft.AddElement("10.168.216.102");

    AStringArray objRight;
    objRight.AddElement("10.168.219.102");
    objRight.AddElement("10.168.219.104");
    objRight.AddElement("10.168.219.106");

    EXPECT_FALSE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_TRUE));
}

/// IsStrExistInList()
TEST_F(AosUtilTest, ExpectTrueWhenLeftIpv4ValueIsExistInRightArray)
{
    AString strLeft = "10.168.219.106";

    AStringArray objRight;
    objRight.AddElement("10.168.219.102");
    objRight.AddElement("10.168.219.104");
    objRight.AddElement("10.168.219.106");

    EXPECT_TRUE(m_pAosUtil->IsStrExistInList(strLeft, objRight, IMS_TRUE));
}

TEST_F(AosUtilTest, ExpectFalseWhenLeftIpv4ValueIsNotExistInRightArray)
{
    AString strLeft = "10.168.216.102";

    AStringArray objRight;
    objRight.AddElement("10.168.219.102");
    objRight.AddElement("10.168.219.104");
    objRight.AddElement("10.168.219.106");

    EXPECT_FALSE(m_pAosUtil->IsStrExistInList(strLeft, objRight, IMS_TRUE));
}

/// IsListEqual() for Ip Address
TEST_F(AosUtilTest, ArraysAreEqualWhenTwoIpv6ArraysAreTheSame)
{
    AStringArray objLeft;
    objLeft.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    objLeft.AddElement("240a:3:4400:3420::7");
    objLeft.AddElement("fc01:abab:cdcd:6fee::1");

    AStringArray objRight;
    objRight.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    objRight.AddElement("240a:3:4400:3420:0:0:0:7");
    objRight.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_TRUE));
}

TEST_F(AosUtilTest, ArraysAreEqualWhenTwoIpv6ArraysAreInDifferentOrder)
{
    AStringArray objLeft;
    objLeft.AddElement("fc01:abab:cdcd:6fee::1");
    objLeft.AddElement("240a:3:4400:3420::7");
    objLeft.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");

    AStringArray objRight;
    objRight.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    objRight.AddElement("240a:3:4400:3420:0:0:0:7");
    objRight.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_TRUE));
}

TEST_F(AosUtilTest, ArraysAreNotEqualWhenTwoIpv6ArraysAreDifferent)
{
    AStringArray objLeft;
    objLeft.AddElement("240a:3:4400:3420::6");
    objLeft.AddElement("fc02:abab:cdcd:6fee::1");
    objLeft.AddElement("fd29:cc42:7fb9:2:20c:29ff:fe66:b4c7");

    AStringArray objRight;
    objRight.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    objRight.AddElement("240a:3:4400:3420:0:0:0:7");
    objRight.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_FALSE(m_pAosUtil->IsListEqual(objLeft, objRight, IMS_TRUE));
}

/// IsStrExistInList()
TEST_F(AosUtilTest, ExpectTrueWhenLeftIpv6ValueIsExistInRightArray)
{
    AString strLeft = "240a:3:4400:3420::7";

    AStringArray objRight;
    objRight.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    objRight.AddElement("240a:3:4400:3420:0:0:0:7");
    objRight.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_TRUE(m_pAosUtil->IsStrExistInList(strLeft, objRight, IMS_TRUE));
}

TEST_F(AosUtilTest, ExpectFalseWhenLeftIpv6ValueIsNotExistInRightArray)
{
    AString strLeft = "fd29:cc42:7fb9:2:20c:29ff:fe66:b4c7";

    AStringArray objRight;
    objRight.AddElement("fd29:cc43:7fb9:2:20c:29ff:fe66:b4c7");
    objRight.AddElement("240a:3:4400:3420:0:0:0:7");
    objRight.AddElement("fc01:abab:cdcd:6fee::1");

    EXPECT_FALSE(m_pAosUtil->IsStrExistInList(strLeft, objRight, IMS_TRUE));
}

// IsListEqual() for ImsList<IMS_UINT32>
TEST_F(AosUtilTest, ListsAreEqualWhenIntListsAreTheSame)
{
    ImsList<IMS_UINT32> objReasons;
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objReasons);
    // Try adding duplicate list
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objReasons);

    ImsList<IMS_UINT32> objCompareReasons;
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objCompareReasons);
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objCompareReasons);

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objReasons, objCompareReasons, IMS_TRUE));
}

TEST_F(AosUtilTest, ListsAreEqualWhenIntListsAreInDifferentOrderAndOrderCheckIsNotRequired)
{
    ImsList<IMS_UINT32> objReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objReasons);

    ImsList<IMS_UINT32> objCompareReasons;
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objCompareReasons);
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objCompareReasons);

    EXPECT_TRUE(m_pAosUtil->IsListEqual(objReasons, objCompareReasons, IMS_FALSE));
}

TEST_F(AosUtilTest, ListsAreNotEqualWhenIntListsAreInDifferentOrderAndOrderCheckIsRequired)
{
    ImsList<IMS_UINT32> objReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objReasons);

    ImsList<IMS_UINT32> objCompareReasons;
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objCompareReasons);
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objCompareReasons);

    EXPECT_FALSE(m_pAosUtil->IsListEqual(objReasons, objCompareReasons, IMS_TRUE));
}

TEST_F(AosUtilTest, ListsAreNotEqualWhenIntListsAreDifferent)
{
    ImsList<IMS_UINT32> objReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, objReasons);

    ImsList<IMS_UINT32> objCompareReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objCompareReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objCompareReasons);

    EXPECT_FALSE(m_pAosUtil->IsListEqual(objReasons, objCompareReasons, IMS_FALSE));
}

/// IsElementExistInList()
TEST_F(AosUtilTest, ExpectTrueWhenLeftListIsExistInRightList)
{
    ImsList<IMS_UINT32> objReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objReasons);

    ImsList<IMS_UINT32> objCompareReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objCompareReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objCompareReasons);

    EXPECT_TRUE(m_pAosUtil->IsElementExistInList(objCompareReasons, objReasons));
}

TEST_F(AosUtilTest, ExpectFalseWhenLeftListIsNotExistInRightList)
{
    ImsList<IMS_UINT32> objReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, objReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_VOPS_OFF, objReasons);

    ImsList<IMS_UINT32> objCompareReasons;
    m_pAosUtil->AddElementToList(BLOCK_WIFI_AIRPLANE_MODE_ON, objCompareReasons);
    m_pAosUtil->AddElementToList(BLOCK_CELLULAR_AIRPLANE_MODE_ON, objCompareReasons);

    EXPECT_FALSE(m_pAosUtil->IsElementExistInList(objCompareReasons, objReasons));
}

TEST_F(AosUtilTest, ReturnTrueWhenCheckingSupportedNetworkType)
{
    EXPECT_TRUE(m_pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(m_pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_WLAN));
}

TEST_F(AosUtilTest, ReturnFalseWhenCheckingUnsupportedNetworkType)
{
    EXPECT_FALSE(m_pAosUtil->IsSupportedNetworkType(NW_REPORT_RADIO_WCDMA));
}

TEST_F(AosUtilTest, ReturnTrueWhenCheckingSupportedNetworkTypeForCellular)
{
    EXPECT_TRUE(m_pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_NR));
}

TEST_F(AosUtilTest, ReturnFalseWhenCheckingUnsupportedNetworkTypeForCellular)
{
    EXPECT_FALSE(m_pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(m_pAosUtil->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_GSM));
}

TEST_F(AosUtilTest, WifiTestIsTrueWhenTurningOnWifiTest)
{
    m_pAosUtil->SetWifiTest(IMS_TRUE);
    EXPECT_TRUE(m_pAosUtil->IsWifiTest());
}

TEST_F(AosUtilTest, WifiTestIsFalseWhenTurningOffWifiTest)
{
    m_pAosUtil->SetWifiTest(IMS_FALSE);
    EXPECT_FALSE(m_pAosUtil->IsWifiTest());
}