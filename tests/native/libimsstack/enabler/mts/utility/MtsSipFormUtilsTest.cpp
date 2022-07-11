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
#include "MtsApp.h"
#include "utility/MtsSipFormUtils.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

namespace android
{

class MtsSipFormUtilsTest : public ::testing::Test
{
public:
    MtsSipFormUtils* pMtsSipFormUtils;

protected:
    virtual void SetUp() override { pMtsSipFormUtils = new MtsSipFormUtils(SLOT_ID); }

    virtual void TearDown() override { delete pMtsSipFormUtils; }
};

TEST_F(MtsSipFormUtilsTest, Constructor)
{
    ASSERT_NE(pMtsSipFormUtils, nullptr);
}

TEST_F(MtsSipFormUtilsTest, TestFormDestination)
{
    AString strTargetAddress = "tel:+12345678901";
    AString strLastIpSmgw = "tel:+12345678902";
    AString strResultAddress;

    pMtsSipFormUtils->FormDestination(strTargetAddress, IMS_FALSE, strLastIpSmgw, strResultAddress);
    EXPECT_STREQ(strTargetAddress.GetStr(), strResultAddress.GetStr());

    pMtsSipFormUtils->FormDestination(strTargetAddress, IMS_TRUE, strLastIpSmgw, strResultAddress);
    EXPECT_STREQ(strLastIpSmgw.GetStr(), strResultAddress.GetStr());
}

TEST_F(MtsSipFormUtilsTest, TestFormContentTypeEnumToStr)
{
    EXPECT_STREQ(pMtsSipFormUtils->FormContentTypeEnumToStr(MtsApp::SMSFORMAT_3GPP).GetStr(),
            "application/vnd.3gpp.sms");
    EXPECT_STREQ(pMtsSipFormUtils->FormContentTypeEnumToStr(MtsApp::SMSFORMAT_3GPP2).GetStr(),
            "application/vnd.3gpp2.sms");
}

TEST_F(MtsSipFormUtilsTest, TestFormContentTypeStrToEnum)
{
    AString strTestContentType = "application/vnd.3gpp.sms";
    EXPECT_EQ(
            pMtsSipFormUtils->FormContentTypeStrToEnum(strTestContentType), MtsApp::SMSFORMAT_3GPP);

    strTestContentType = "application/vnd.3gpp2.sms";
    EXPECT_EQ(pMtsSipFormUtils->FormContentTypeStrToEnum(strTestContentType),
            MtsApp::SMSFORMAT_3GPP2);

    strTestContentType = "application/reginfo+xml";
    EXPECT_EQ(pMtsSipFormUtils->FormContentTypeStrToEnum(strTestContentType),
            MtsApp::SMSFORMAT_INVALID);
}

TEST_F(MtsSipFormUtilsTest, TestIsTelUrlParam)
{
    AString strTestUrl = "phone-context";
    EXPECT_TRUE(pMtsSipFormUtils->IsTelUrlParam(strTestUrl) == IMS_TRUE);

    strTestUrl = "user";
    EXPECT_TRUE(pMtsSipFormUtils->IsTelUrlParam(strTestUrl) == IMS_FALSE);
}

TEST_F(MtsSipFormUtilsTest, TestIsNumberFormat)
{
    AString strTestDial = "+";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_FALSE);

    strTestDial = "+12345678901";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_TRUE);

    strTestDial = "+12345678901@google.com";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_FALSE);

    strTestDial = "#32#1234567890";
    EXPECT_TRUE(pMtsSipFormUtils->IsNumberFormat(strTestDial) == IMS_TRUE);
}

TEST_F(MtsSipFormUtilsTest, TestIsIpAddress)
{
    AString strTestIpAddress = "168.0.0.1";
    EXPECT_TRUE(pMtsSipFormUtils->IsIpAddress(strTestIpAddress) == IMS_TRUE);
}

TEST_F(MtsSipFormUtilsTest, TestCheckScheme)
{
    AString strTestScheme = "sip:";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == MtsSipFormUtils::SCHEME_SIP);

    strTestScheme = "tel:";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == MtsSipFormUtils::SCHEME_TEL);

    strTestScheme = "sips:";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == MtsSipFormUtils::SCHEME_SIPS);

    strTestScheme = "+12345678901";
    EXPECT_TRUE(pMtsSipFormUtils->CheckScheme(strTestScheme) == MtsSipFormUtils::SCHEME_UNKNOWN);
}

}  // namespace android
