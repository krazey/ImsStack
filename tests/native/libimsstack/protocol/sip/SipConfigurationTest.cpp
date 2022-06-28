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
#include "SipConfiguration.h"

namespace android
{

class SipConfigurationTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipConfigurationTest, SipConfigurationSetGetTest)
{
    SipConfiguration* pConfiguration = SipConfiguration::GetInstance();
    ASSERT_TRUE(pConfiguration != nullptr);

    /* Default values check */
    EXPECT_EQ(SIP_FALSE, pConfiguration->IsPANIHeaderReqdForACK());

    unsigned int nOptions = pConfiguration->GetStackSettings();
    EXPECT_EQ(0, nOptions & SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE);
    EXPECT_EQ(0, nOptions & SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM);
    EXPECT_EQ(0, nOptions & SipConfiguration::MSG_OPT_DECODE_STRICT);

    EXPECT_EQ(2000, pConfiguration->GetT1());
    EXPECT_EQ(16000, pConfiguration->GetT2());
    EXPECT_EQ(17000, pConfiguration->GetT4());
    EXPECT_EQ(128000, pConfiguration->GetTimerB());
    EXPECT_EQ(180000, pConfiguration->GetTimerC());
    EXPECT_EQ(180000, pConfiguration->GetTimerCr());
    EXPECT_EQ(128000, pConfiguration->GetTimerD());
    EXPECT_EQ(128000, pConfiguration->GetTimerF());
    EXPECT_EQ(128000, pConfiguration->GetTimerH());
    EXPECT_EQ(17000, pConfiguration->GetTimerI());
    EXPECT_EQ(128000, pConfiguration->GetTimerJ());
    EXPECT_EQ(17000, pConfiguration->GetTimerK());

    /* Set values and check */
    pConfiguration->SetMultiLineEncoding(SIP_TRUE);
    pConfiguration->SetShortFormEncoding(SIP_TRUE);
    pConfiguration->SetDecodeStrictness(SIP_TRUE);

    nOptions = pConfiguration->GetStackSettings();
    EXPECT_EQ(SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE,
            nOptions & SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE);
    EXPECT_EQ(SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM,
            nOptions & SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM);
    EXPECT_EQ(SipConfiguration::MSG_OPT_DECODE_STRICT,
            nOptions & SipConfiguration::MSG_OPT_DECODE_STRICT);

    pConfiguration->SetMultiLineEncoding(SIP_FALSE);
    pConfiguration->SetShortFormEncoding(SIP_FALSE);
    pConfiguration->SetDecodeStrictness(SIP_FALSE);

    nOptions = pConfiguration->GetStackSettings();
    EXPECT_EQ(0, nOptions & SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE);
    EXPECT_EQ(0, nOptions & SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM);
    EXPECT_EQ(0, nOptions & SipConfiguration::MSG_OPT_DECODE_STRICT);

    pConfiguration->SetT1(500);
    EXPECT_EQ(500, pConfiguration->GetT1());

    pConfiguration->SetT2(20500);
    EXPECT_EQ(20500, pConfiguration->GetT2());

    pConfiguration->SetT4(18500);
    EXPECT_EQ(18500, pConfiguration->GetT4());

    pConfiguration->SetTimerB(132000);
    EXPECT_EQ(132000, pConfiguration->GetTimerB());

    pConfiguration->SetTimerC(190000);
    EXPECT_EQ(190000, pConfiguration->GetTimerC());

    pConfiguration->SetTimerCr(190000);
    EXPECT_EQ(190000, pConfiguration->GetTimerCr());

    pConfiguration->SetTimerD(132000);
    EXPECT_EQ(132000, pConfiguration->GetTimerD());

    pConfiguration->SetTimerF(132000);
    EXPECT_EQ(132000, pConfiguration->GetTimerF());

    pConfiguration->SetTimerH(132000);
    EXPECT_EQ(132000, pConfiguration->GetTimerH());

    pConfiguration->SetTimerI(18500);
    EXPECT_EQ(18500, pConfiguration->GetTimerI());

    pConfiguration->SetTimerJ(132000);
    EXPECT_EQ(132000, pConfiguration->GetTimerJ());

    pConfiguration->SetTimerK(18500);
    EXPECT_EQ(18500, pConfiguration->GetTimerK());

    SipConfiguration::DestroyInstance();
}

}  // namespace android