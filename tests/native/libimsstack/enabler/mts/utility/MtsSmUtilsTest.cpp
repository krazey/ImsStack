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
#include "utility/MtsSmUtils.h"

namespace android
{

class MtsSmUtilsTest : public ::testing::Test
{
public:
    MtsSmUtils* pMtsSmUtils;

protected:
    virtual void SetUp() override { pMtsSmUtils = new MtsSmUtils(); }

    virtual void TearDown() override { delete pMtsSmUtils; }
};

TEST_F(MtsSmUtilsTest, Constructor)
{
    ASSERT_NE(pMtsSmUtils, nullptr);
}

TEST_F(MtsSmUtilsTest, GetMtiStringFrom3gpp)
{
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_MS),
            "MTS_3GPP_MTI_RP_DATA_From_MS");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_N),
            "MTS_3GPP_MTI_RP_DATA_From_N");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_MS),
            "MTS_3GPP_MTI_RP_ACK_From_MS");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_N),
            "MTS_3GPP_MTI_RP_ACK_From_N");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS),
            "MTS_3GPP_MTI_RP_ERROR_From_MS");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_N),
            "MTS_3GPP_MTI_RP_ERROR_From_N");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_3GPP_MTI_RP_SMMA),
            "MTS_3GPP_MTI_RP_SMMA");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp(MtsSmUtils::MTS_SMS_MTI_NONE),
            "SMS 3GPP MTI INFO INVALID");
}

TEST_F(MtsSmUtilsTest, GetMtiStringFrom3gpp2)
{
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp2(MtsSmUtils::MTS_3GPP2_MTI_SMS_POINT_TO_POINT),
            "MTS_3GPP2_MTI_SMS_POINT_TO_POINT");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp2(MtsSmUtils::MTS_3GPP2_MTI_SMS_BROADCAST),
            "MTS_3GPP2_MTI_SMS_BROADCAST");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp2(MtsSmUtils::MTS_3GPP2_MTI_SMS_ACKNOWLEDGE),
            "MTS_3GPP2_MTI_SMS_ACKNOWLEDGE");
    EXPECT_STREQ(pMtsSmUtils->GetMtiStringFrom3gpp2(MtsSmUtils::MTS_SMS_MTI_NONE),
            "SMS 3GPP2 MTI INFO INVALID");
}

}  // namespace android
