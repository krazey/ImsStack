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

#include "AString.h"
#include "ImsIdentity.h"
#include "helper/dialing/MtsDialingPlan.h"
#include <gtest/gtest.h>

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const AString TEL_URI_SCHEME("tel");
const AString SIP_URI_SCHEME("sip");
const AString SIPS_URI_SCHEME("sips");

class MtsDialingPlanTest : public ::testing::Test
{
public:
    MtsDialingPlan* pMtsDialingPlan;

protected:
    virtual void SetUp() override
    {
        pMtsDialingPlan =
                new MtsDialingPlan(SLOT_ID, TEL_URI_SCHEME, ImsIdentity::DIALING_POLICY_HOME_LOCAL);
    }

    virtual void TearDown() override { delete pMtsDialingPlan; }
};

TEST_F(MtsDialingPlanTest, Constructor)
{
    ASSERT_NE(pMtsDialingPlan, nullptr);
}

TEST_F(MtsDialingPlanTest, Translate)
{
    AString strTargetAddress = "tel:+12345678901";
    AString strResult;

    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    EXPECT_STREQ(strResult.GetStr(), strTargetAddress.GetStr());

    strTargetAddress = "sip:+12063130004@msg.pc.t-mobile.com;user=phone";
    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    strTargetAddress.Prepend("<");
    strTargetAddress.Append(">");
    EXPECT_STREQ(strResult.GetStr(), strTargetAddress.GetStr());

    strTargetAddress = "+12345678901";
    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    EXPECT_TRUE(strResult.Contains(TEL_URI_SCHEME));

    strTargetAddress = "2345678901";
    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    EXPECT_TRUE(strResult.Contains(";phone-context="));

    strTargetAddress = "ipsmgw.lte-lguplus.co.kr";
    pMtsDialingPlan->SetScheme(SIP_URI_SCHEME.GetStr());
    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    EXPECT_TRUE(strResult.Contains(SIP_URI_SCHEME));

    strTargetAddress = "";
    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    EXPECT_STREQ(strResult.GetStr(), strTargetAddress.GetStr());

    strTargetAddress = "<sip:+12063130004@msg.pc.t-mobile.com;user=phone>";
    strResult = pMtsDialingPlan->Translate(strTargetAddress);
    EXPECT_STREQ(strResult.GetStr(), strTargetAddress.GetStr());
}

TEST_F(MtsDialingPlanTest, TranslateFormNonTelUriWithSomeMoreParameters)
{
    AString strTargetAddress = "#31#2345678901";
    AString strResult;

    pMtsDialingPlan->SetScheme(SIP_URI_SCHEME);
    strResult = pMtsDialingPlan->Translate(strTargetAddress, IMS_TRUE);
    EXPECT_TRUE(strResult.Contains(SIP_URI_SCHEME));
    EXPECT_TRUE(strResult.Contains(";phone-context="));
    EXPECT_TRUE(strResult.Contains(";user="));
}

TEST_F(MtsDialingPlanTest, TranslateFormNonTelUriWithSpecificTargetAddress)
{
    AString strTargetAddress = "+";
    AString strResult;

    pMtsDialingPlan->SetScheme(SIP_URI_SCHEME);

    strResult = pMtsDialingPlan->Translate(strTargetAddress, IMS_TRUE);
    EXPECT_TRUE(strResult.Contains(SIP_URI_SCHEME + ":+"));
}

TEST_F(MtsDialingPlanTest, TranslateFormNonTelUriWithUnexpectedTargetAddress)
{
    AString strTargetAddress = "+12345678901_";
    AString strResult;

    pMtsDialingPlan->SetScheme(SIP_URI_SCHEME);
    strResult = pMtsDialingPlan->Translate(strTargetAddress, IMS_TRUE);
    EXPECT_TRUE(strResult.Contains(SIP_URI_SCHEME + ":+12345678901_"));
    EXPECT_FALSE(strResult.Contains(";phone-context="));
    EXPECT_FALSE(strResult.Contains(";user="));
}

TEST_F(MtsDialingPlanTest, TranslateFormNonTelUriWithGlobalNumberFormat)
{
    AString strTargetAddress = "+12345678901";
    AString strResult;

    pMtsDialingPlan->SetScheme(SIP_URI_SCHEME);
    strResult = pMtsDialingPlan->Translate(strTargetAddress, IMS_TRUE);
    EXPECT_TRUE(strResult.Contains(SIP_URI_SCHEME + ":+12345678901"));
    EXPECT_FALSE(strResult.Contains(";phone-context="));
    EXPECT_TRUE(strResult.Contains(";user="));
}

TEST_F(MtsDialingPlanTest, GetterSetterNetworkProfile)
{
    AString strNetworkProfile = "Sms over IP";
    pMtsDialingPlan->SetNetworkProfile(strNetworkProfile);
    EXPECT_STREQ(strNetworkProfile.GetStr(), pMtsDialingPlan->GetNetworkProfile().GetStr());
}

TEST_F(MtsDialingPlanTest, GetterSetterDialingPolicy)
{
    pMtsDialingPlan->SetDialingPolicy(ImsIdentity::DIALING_POLICY_GEO_LOCAL);
    EXPECT_EQ(ImsIdentity::DIALING_POLICY_GEO_LOCAL, pMtsDialingPlan->GetDialingPolicy());
}

}  // namespace android
