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

#include "msg/SipMsgUtil.h"
#include "msg/SipRequestDispositionHeader.h"
#include "platform/SipString.h"

extern SIP_CHAR gaszDirectivesArray[SipRequestDispositionHeader::MAX_DIRECTIVE_SIZE]
                                   [SipRequestDispositionHeader::MAX_DIRECTIVE_LEN];

namespace android
{

class SipRequestDispositionHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipRequestDispositionHeaderTest, CopyConstructor)
{
    SipRequestDispositionHeader* pHeader = reinterpret_cast<SipRequestDispositionHeader*>(
            SipRequestDispositionHeader::GetNewObj(SipHeaderBase::REQUEST_DISPOSITION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("proxy", 5));

    SipRequestDispositionHeader* pCopyHeader = reinterpret_cast<SipRequestDispositionHeader*>(
            SipRequestDispositionHeader::GetNewObj(SipHeaderBase::REQUEST_DISPOSITION, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_STREQ("proxy", pCopyHeader->GetValue());
    pCopyHeader->SipDelete();
}

TEST_F(SipRequestDispositionHeaderTest, DecodeHdr)
{
    SipRequestDispositionHeader* pHeader = reinterpret_cast<SipRequestDispositionHeader*>(
            SipRequestDispositionHeader::GetNewObj(SipHeaderBase::REQUEST_DISPOSITION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty headers, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("", 0));

    /* invalid value, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("InvalidValue", 12));
    pHeader->SipDelete();

    /* Check all possible valid values, success */
    for (SIP_UINT16 nCnt = 0; nCnt < SipRequestDispositionHeader::MAX_DIRECTIVE_SIZE; nCnt++)
    {
        pHeader = reinterpret_cast<SipRequestDispositionHeader*>(
                SipRequestDispositionHeader::GetNewObj(
                        SipHeaderBase::REQUEST_DISPOSITION, nullptr));
        ASSERT_TRUE(pHeader != nullptr);

        EXPECT_EQ(SIP_TRUE,
                pHeader->DecodeHdr(
                        gaszDirectivesArray[nCnt], SipPf_Strlen(gaszDirectivesArray[nCnt])));
        pHeader->SipDelete();
    }
}

}  // namespace android
