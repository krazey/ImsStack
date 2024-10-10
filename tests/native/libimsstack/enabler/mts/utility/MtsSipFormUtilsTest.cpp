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

#include "MtsDef.h"
#include "utility/MtsSipFormUtils.h"
#include <gtest/gtest.h>

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsSipFormUtilsTest : public ::testing::Test
{
public:
    MtsSipFormUtils* pMtsSipFormUtils;
    AString strTargetAddress = "tel:+12345678901";
    AString strLastIpSmgw = "tel:+12345678902";
    AString str3gppFormat = "application/vnd.3gpp.sms";
    AString str3gpp2Format = "application/vnd.3gpp2.sms";
    AString strRegiInfoContentType = "application/reginfo+xml";

protected:
    virtual void SetUp() override { pMtsSipFormUtils = new MtsSipFormUtils(SLOT_ID); }

    virtual void TearDown() override { delete pMtsSipFormUtils; }
};

TEST_F(MtsSipFormUtilsTest, Constructor)
{
    ASSERT_NE(pMtsSipFormUtils, nullptr);
}

TEST_F(MtsSipFormUtilsTest, FormDestination)
{
    AString strResultAddress;

    pMtsSipFormUtils->FormDestination(strTargetAddress, IMS_FALSE, strLastIpSmgw, strResultAddress);
    EXPECT_STREQ(strTargetAddress.GetStr(), strResultAddress.GetStr());

    pMtsSipFormUtils->FormDestination(strTargetAddress, IMS_TRUE, strLastIpSmgw, strResultAddress);
    EXPECT_STREQ(strLastIpSmgw.GetStr(), strResultAddress.GetStr());
}

TEST_F(MtsSipFormUtilsTest, FormDestinationWithNoTargetAddress)
{
    AString strResultAddress;
    AString strNoTargetAddress;

    EXPECT_FALSE(pMtsSipFormUtils->FormDestination(
            strNoTargetAddress, IMS_FALSE, strLastIpSmgw, strResultAddress));
}

TEST_F(MtsSipFormUtilsTest, FormContentTypeEnumToStr)
{
    EXPECT_STREQ(pMtsSipFormUtils->FormContentTypeEnumToStr(SmsFormatType::SMSFORMAT_3GPP).GetStr(),
            str3gppFormat.GetStr());
    EXPECT_STREQ(
            pMtsSipFormUtils->FormContentTypeEnumToStr(SmsFormatType::SMSFORMAT_3GPP2).GetStr(),
            str3gpp2Format.GetStr());
    EXPECT_STREQ(
            pMtsSipFormUtils->FormContentTypeEnumToStr(SmsFormatType::SMSFORMAT_INVALID).GetStr(),
            "");
}

TEST_F(MtsSipFormUtilsTest, FormContentTypeStrToEnum)
{
    EXPECT_EQ(pMtsSipFormUtils->FormContentTypeStrToEnum(str3gppFormat),
            SmsFormatType::SMSFORMAT_3GPP);
    EXPECT_EQ(pMtsSipFormUtils->FormContentTypeStrToEnum(str3gpp2Format),
            SmsFormatType::SMSFORMAT_3GPP2);
    EXPECT_EQ(pMtsSipFormUtils->FormContentTypeStrToEnum(strRegiInfoContentType),
            SmsFormatType::SMSFORMAT_INVALID);
}

TEST_F(MtsSipFormUtilsTest, IsTelUrlParam)
{
    AString strTestUrl = "phone-context";
    EXPECT_TRUE(pMtsSipFormUtils->IsTelUrlParam(strTestUrl) == IMS_TRUE);

    strTestUrl = "user";
    EXPECT_TRUE(pMtsSipFormUtils->IsTelUrlParam(strTestUrl) == IMS_FALSE);
}

TEST_F(MtsSipFormUtilsTest, IsNumberFormat)
{
    AString strTestDial = "+";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_FALSE);

    strTestDial = "+12345678901";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_TRUE);

    strTestDial = "+12345678901@ims.google.com";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_FALSE);

    strTestDial = "#32#1234567890";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_TRUE);
}

TEST_F(MtsSipFormUtilsTest, IsIpAddress)
{
    AString strTestIpAddress = "168.0.0.1";
    EXPECT_TRUE(pMtsSipFormUtils->IsIpAddress(strTestIpAddress) == IMS_TRUE);
}

TEST_F(MtsSipFormUtilsTest, CheckScheme)
{
    AString strTestScheme = "sip:";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == URI_SCHEME_SIP);

    strTestScheme = "tel:";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == URI_SCHEME_TEL);

    strTestScheme = "sips:";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == URI_SCHEME_SIPS);

    strTestScheme = "+12345678901";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == URI_SCHEME_UNKNOWN);
}

}  // namespace android
