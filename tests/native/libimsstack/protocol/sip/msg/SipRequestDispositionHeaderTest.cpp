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

#include "msg/SipRequestDispositionHeader.h"
#include "platform/SipString.h"

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

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("proxy", 5));

    SipRequestDispositionHeader* pCopyHeader = reinterpret_cast<SipRequestDispositionHeader*>(
            SipRequestDispositionHeader::GetNewObj(SipHeaderBase::REQUEST_DISPOSITION, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_STREQ("proxy", pCopyHeader->GetValue());
    pCopyHeader->SipDelete();
}

TEST_F(SipRequestDispositionHeaderTest, Decode)
{
    SipRequestDispositionHeader* pHeader = reinterpret_cast<SipRequestDispositionHeader*>(
            SipRequestDispositionHeader::GetNewObj(SipHeaderBase::REQUEST_DISPOSITION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty headers, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    /* invalid value, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("InvalidValue", 12));
    pHeader->SipDelete();

    /* Check all possible valid values, success */
    const SIP_CHAR* pDirectiveStr;

    for (SIP_UINT16 nCnt = 0; nCnt < SipRequestDispositionHeader::GetDirectiveSize(); nCnt++)
    {
        pHeader = reinterpret_cast<SipRequestDispositionHeader*>(
                SipRequestDispositionHeader::GetNewObj(
                        SipHeaderBase::REQUEST_DISPOSITION, nullptr));
        ASSERT_TRUE(pHeader != nullptr);

        pDirectiveStr = SipRequestDispositionHeader::GetDirectiveString(nCnt);
        EXPECT_EQ(SIP_TRUE, pHeader->Decode(pDirectiveStr, SipPf_Strlen(pDirectiveStr)));
        pHeader->SipDelete();
    }
}

}  // namespace android
