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

#include "msg/SipTimeStampHeader.h"

namespace android
{

class SipTimeStampHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipTimeStampHeaderTest, EncodeAndEncodeHdr)
{
    SipTimeStampHeader* pHeader = reinterpret_cast<SipTimeStampHeader*>(
            SipTimeStampHeader::GetNewObj(SipHeaderBase::TIMESTAMP, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    pHeader->SetTimeVal("12.56");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));

    EXPECT_STREQ("12.56", objValue.GetCharString());
    EXPECT_STREQ("12.56", &(aBuffer[0]));

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->SetDelay("1.30");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));

    EXPECT_STREQ("12.56 1.30", objValue.GetCharString());
    EXPECT_STREQ("12.56 1.30", &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipTimeStampHeaderTest, DecodeHdr)
{
    SipTimeStampHeader* pHeader = reinterpret_cast<SipTimeStampHeader*>(
            SipTimeStampHeader::GetNewObj(SipHeaderBase::TIMESTAMP, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("", 0));

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("12.56", 5));

    EXPECT_STREQ("12.56", pHeader->GetTimeVal());
    EXPECT_EQ(nullptr, pHeader->GetDelay());

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTimeStampHeader*>(
            SipTimeStampHeader::GetNewObj(SipHeaderBase::TIMESTAMP, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("12.56 1.30", 10));

    EXPECT_STREQ("12.56", pHeader->GetTimeVal());
    EXPECT_STREQ("1.30", pHeader->GetDelay());

    SipTimeStampHeader* pCopyHeader = reinterpret_cast<SipTimeStampHeader*>(
            SipTimeStampHeader::GetNewObj(SipHeaderBase::TIMESTAMP, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objValue, SIP_FALSE));

    EXPECT_STREQ("12.56 1.30", objValue.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android