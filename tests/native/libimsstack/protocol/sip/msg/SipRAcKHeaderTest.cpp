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

#include "msg/SipRAcKHeader.h"

namespace android
{

class SipRAcKHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipRAcKHeaderTest, IsValidHeader)
{
    SipRAcKHeader* pHeader = reinterpret_cast<SipRAcKHeader*>(
            SipRAcKHeader::GetNewObj(SipHeaderBase::RACK, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    pHeader->SetMethod("REGISTER");

    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pHeader->SipDelete();
}

TEST_F(SipRAcKHeaderTest, EncodeHdrAndDecodeHdr)
{
    SipRAcKHeader* pHeader = reinterpret_cast<SipRAcKHeader*>(
            SipRAcKHeader::GetNewObj(SipHeaderBase::RACK, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("", 0));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("2", 1));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("2 ", 2));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(" 2", 2));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("INVITE", 6));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(" INVITE", 7));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("99 INVITE", 9));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(" 99 INVITE", 10));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("212 7", 5));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("212 7 ", 6));

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("7183 1 INVITE", 13));

    SipRAcKHeader* pCopyHeader = reinterpret_cast<SipRAcKHeader*>(
            SipRAcKHeader::GetNewObj(SipHeaderBase::RACK, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);
    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("7183 1 INVITE", &(aBuffer[0]));
    EXPECT_STREQ("7183 1 INVITE", objBuffer.GetCharString());

    EXPECT_EQ(7183, pCopyHeader->GetResponseNum());
    EXPECT_EQ(1, pCopyHeader->GetCSeqNum());
    EXPECT_STREQ("INVITE", pCopyHeader->GetMethod());
    pCopyHeader->SipDelete();
}

}  // namespace android
