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

#include "msg/SipCSeqHeader.h"

namespace android
{

class SipCSeqHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipCSeqHeaderTest, IsValidHeader)
{
    SipCSeqHeader* pHeader = reinterpret_cast<SipCSeqHeader*>(
            SipCSeqHeader::GetNewObj(SipHeaderBase::CSEQ, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    pHeader->SetMethod("REGISTER");

    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pHeader->SetSeq(99);

    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pHeader->SipDelete();
}

TEST_F(SipCSeqHeaderTest, EncodeAndDecode)
{
    SipCSeqHeader* pHeader = reinterpret_cast<SipCSeqHeader*>(
            SipCSeqHeader::GetNewObj(SipHeaderBase::CSEQ, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("2", 1));
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("2 ", 2));
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(" 2", 2));
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("INVITE", 6));
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(" INVITE", 7));

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("0 INVITE", 8));

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("1 \r\n INVITE", 11));
    EXPECT_STREQ("INVITE", pHeader->GetMethod());

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("1 INVITE", 8));

    SipCSeqHeader* pCopyHeader = reinterpret_cast<SipCSeqHeader*>(
            SipCSeqHeader::GetNewObj(SipHeaderBase::CSEQ, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);
    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("1 INVITE", &(aBuffer[0]));
    EXPECT_STREQ("1 INVITE", objBuffer.GetCharString());
    pCopyHeader->SipDelete();
}

}  // namespace android
