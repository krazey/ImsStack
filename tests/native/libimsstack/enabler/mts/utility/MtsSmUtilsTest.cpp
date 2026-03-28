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
#include "ByteArray.h"
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

TEST_F(MtsSmUtilsTest, GetRpMrWithNull)
{
    const IMS_BYTE* pbyContent = IMS_NULL;
    EXPECT_EQ(pMtsSmUtils->GetRpMr(pbyContent), -1);

    const ByteArray* pContent = new ByteArray();
    EXPECT_EQ(pMtsSmUtils->GetRpMr(*pContent), -1);
    delete pContent;
}

TEST_F(MtsSmUtilsTest, GetMtiWithNull)
{
    const IMS_BYTE* pbyContent = IMS_NULL;
    EXPECT_EQ(pMtsSmUtils->GetMti(SmsFormatType::SMSFORMAT_3GPP, pbyContent), -1);

    const ByteArray* pContent = new ByteArray();
    EXPECT_EQ(pMtsSmUtils->GetMti(SmsFormatType::SMSFORMAT_3GPP, *pContent), -1);
    delete pContent;
}

TEST_F(MtsSmUtilsTest, IsSmsRpAckOrErrorReturnsTrueForAckOrError)
{
    const ByteArray objAckContent(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_ACK_FROM_MS));
    EXPECT_TRUE(MtsSmUtils::IsSmsRpAckOrError(SmsFormatType::SMSFORMAT_3GPP, objAckContent));

    const ByteArray objErrorContent(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_ERROR_FROM_MS));
    EXPECT_TRUE(MtsSmUtils::IsSmsRpAckOrError(SmsFormatType::SMSFORMAT_3GPP, objErrorContent));
}

TEST_F(MtsSmUtilsTest, IsSmsRpAckOrErrorReturnsFalseForOtherTypesOrNull)
{
    const ByteArray objDataContent(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    EXPECT_FALSE(MtsSmUtils::IsSmsRpAckOrError(SmsFormatType::SMSFORMAT_3GPP, objDataContent));

    const ByteArray objAckContent(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_ACK_FROM_MS));
    EXPECT_FALSE(MtsSmUtils::IsSmsRpAckOrError(SmsFormatType::SMSFORMAT_3GPP2, objAckContent));
}

}  // namespace android
