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
#include "msg/SipPChargingFunctionAddressesHeader.h"

namespace android
{

class SipPChargingFunctionAddressesHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipPChargingFunctionAddressesHeaderTest, CopyConstructor)
{
    SipPChargingFunctionAddressesHeader* pHeader =
            reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
                    SipPChargingFunctionAddressesHeader::GetNewObj(
                            SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(const_cast<char*>("ccf=192.0.8.1"), 13));

    SipPChargingFunctionAddressesHeader* pCopyHeader =
            reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
                    SipPChargingFunctionAddressesHeader::GetNewObj(
                            SipHeaderBase::P_CHRG_FUN_ADDR, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipPChargingFunctionAddressesHeaderTest, EncodeHdr_Null)
{
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    SipPChargingFunctionAddressesHeader* pHeader =
            reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
                    SipPChargingFunctionAddressesHeader::GetNewObj(
                            SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    pHeader->SipDelete();
}

TEST_F(SipPChargingFunctionAddressesHeaderTest, Encode)
{
    AStringBuffer objBuffer(512);

    SipPChargingFunctionAddressesHeader* pHeader =
            reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
                    SipPChargingFunctionAddressesHeader::GetNewObj(
                            SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
            SipPChargingFunctionAddressesHeader::GetNewObj(
                    SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));

    /* Invalid value */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(const_cast<char*>("=value"), strlen("=value")));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
            SipPChargingFunctionAddressesHeader::GetNewObj(
                    SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));

    /* Valid value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(const_cast<char*>("ccf=192.0.8.1"), 13));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("ccf=192.0.8.1", objBuffer.GetCharString());

    pHeader->SipDelete();
}

TEST_F(SipPChargingFunctionAddressesHeaderTest, Encode_DecodeHdr)
{
    SipPChargingFunctionAddressesHeader* pHeader =
            reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
                    SipPChargingFunctionAddressesHeader::GetNewObj(
                            SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
            SipPChargingFunctionAddressesHeader::GetNewObj(
                    SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    /* Invalid value*/
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(const_cast<char*>("=invalid"), strlen("=invalid")));

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
            SipPChargingFunctionAddressesHeader::GetNewObj(
                    SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    /* Decode invalid value */
    SIP_CHAR* pValue = const_cast<char*>("ccf;");
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(pValue, strlen(pValue)));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
            SipPChargingFunctionAddressesHeader::GetNewObj(
                    SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));

    /* Decode valid value */
    pValue = const_cast<char*>("ccf=192.0.8.1;ecf=192.0.8.3");
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, strlen(pValue)));

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingFunctionAddressesHeader*>(
            SipPChargingFunctionAddressesHeader::GetNewObj(
                    SipHeaderBase::P_CHRG_FUN_ADDR, nullptr));
    /* Decode valid value */
    pValue = const_cast<char*>("ccf-2=192.0.8.1;ecf-2=192.0.8.3");
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, strlen(pValue)));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();
}

}  // namespace android
