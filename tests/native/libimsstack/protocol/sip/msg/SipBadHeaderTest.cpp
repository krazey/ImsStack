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

#include "msg/SipBadHeader.h"

namespace android
{

class SipBadHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipBadHeaderTest, CopyConstructor)
{
    SipBadHeader* pHeader = new SipBadHeader();
    ASSERT_TRUE(pHeader != nullptr);

    SipBadHeader* pCopyHeader = new SipBadHeader(*pHeader);
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipBadHeaderTest, EncodeHdr)
{
    SipBadHeader* pHeader = new SipBadHeader();
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetHeaderName("Name");
    EXPECT_STREQ("Name", pHeader->GetHeaderName());
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));

    AStringBuffer objBuffer(512);
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));

    pHeader->SipDelete();
}

TEST_F(SipBadHeaderTest, DecodeHdr)
{
    SipBadHeader* pHeader = new SipBadHeader();
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(SIP_NULL, 0));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pHeader->SipDelete();
}

}  // namespace android
