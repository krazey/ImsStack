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
#include "IuMtsService.h"
#include "MtsStringDef.h"

namespace android
{

class MtsStringDefTest : public ::testing::Test
{
public:
    MtsStringDef* pMtsStringDef;

protected:
    virtual void SetUp() override { pMtsStringDef = new MtsStringDef(); }

    virtual void TearDown() override { delete pMtsStringDef; }
};

TEST_F(MtsStringDefTest, Constructor)
{
    ASSERT_NE(pMtsStringDef, nullptr);
}

TEST_F(MtsStringDefTest, PS_SmsFormatType)
{
    EXPECT_STREQ(PS_SmsFormatType(SmsFormatType::SMSFORMAT_3GPP), "3GPP");
    EXPECT_STREQ(PS_SmsFormatType(SmsFormatType::SMSFORMAT_3GPP2), "3GPP2");
    EXPECT_STREQ(PS_SmsFormatType(SmsFormatType::SMSFORMAT_INVALID), "invalid");
}

TEST_F(MtsStringDefTest, PS_MtiStringFrom3gpp)
{
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_DATA_FROM_MS), "SMS_3GPP_MTI_RP_DATA_FROM_MS");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_DATA_FROM_N), "SMS_3GPP_MTI_RP_DATA_FROM_N");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ACK_FROM_MS), "SMS_3GPP_MTI_RP_ACK_FROM_MS");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ACK_FROM_N), "SMS_3GPP_MTI_RP_ACK_FROM_N");
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ERROR_FROM_MS), "SMS_3GPP_MTI_RP_ERROR_FROM_MS");
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_ERROR_FROM_N), "SMS_3GPP_MTI_RP_ERROR_FROM_N");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_3GPP_MTI_RP_SMMA), "SMS_3GPP_MTI_RP_SMMA");
    EXPECT_STREQ(PS_MtiStringFrom3gpp(SMS_MTI_NONE), "SMS 3GPP MTI INFO INVALID");
}

TEST_F(MtsStringDefTest, PS_MtiStringFrom3gpp2)
{
    EXPECT_STREQ(
            PS_MtiStringFrom3gpp2(SMS_3GPP2_MTI_POINT_TO_POINT), "SMS_3GPP2_MTI_POINT_TO_POINT");
    EXPECT_STREQ(PS_MtiStringFrom3gpp2(SMS_3GPP2_MTI_BROADCAST), "SMS_3GPP2_MTI_BROADCAST");
    EXPECT_STREQ(PS_MtiStringFrom3gpp2(SMS_3GPP2_MTI_ACKNOWLEDGE), "SMS_3GPP2_MTI_ACKNOWLEDGE");
    EXPECT_STREQ(PS_MtiStringFrom3gpp2(SMS_MTI_NONE), "SMS 3GPP2 MTI INFO INVALID");
}

TEST_F(MtsStringDefTest, PS_MoStatus)
{
    EXPECT_STREQ(PS_MoStatus(MO_SUCCESS), "MO_SUCCESS");
    EXPECT_STREQ(PS_MoStatus(MO_IMS_TEMP_FAILURE), "MO_IMS_TEMP_FAILURE");
    EXPECT_STREQ(PS_MoStatus(MO_IMS_PERM_FAILURE), "MO_IMS_PERM_FAILURE");
    EXPECT_STREQ(PS_MoStatus(MO_IMS_LIMITEDSMSSVCREGI), "MO_IMS_LIMITEDSMSSVCREGI");
    EXPECT_STREQ(PS_MoStatus(MO_RETRY_CS), "MO_RETRY_CS");
    EXPECT_STREQ(PS_MoStatus(MO_RETRY_CS_OR_SGS), "MO_RETRY_CS_OR_SGS");
    EXPECT_STREQ(PS_MoStatus(MO_INVALID), "MO_INVALID");
}

TEST_F(MtsStringDefTest, PS_CallState)
{
    EXPECT_STREQ(PS_CallState(CALL_STATE_IDLE), "CALL_STATE_IDLE");
    EXPECT_STREQ(PS_CallState(CALL_STATE_TERMINATING), "CALL_STATE_TERMINATING");
    EXPECT_STREQ(PS_CallState(CALL_STATE_RINGBACK), "CALL_STATE_RINGBACK");
    EXPECT_STREQ(PS_CallState(CALL_STATE_RINGING), "CALL_STATE_RINGING");
    EXPECT_STREQ(PS_CallState(CALL_STATE_ALERTING), "CALL_STATE_ALERTING");
    EXPECT_STREQ(PS_CallState(CALL_STATE_OFFHOOK), "CALL_STATE_OFFHOOK");
    EXPECT_STREQ(PS_CallState(CALL_STATE_UNKNOWN), "__INVALID__");
}

}  // namespace android
