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

#include "msg/SipReferSubHeader.h"

namespace android
{

class SipReferSubHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipReferSubHeaderTest, CopyConstructor)
{
    SipReferSubHeader* pHeader = reinterpret_cast<SipReferSubHeader*>(
            SipReferSubHeader::GetNewObj(SipHeaderBase::REFER_SUB, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipReferSubHeader* pCopyHeader = reinterpret_cast<SipReferSubHeader*>(
            SipReferSubHeader::GetNewObj(SipHeaderBase::REFER_SUB, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipReferSubHeaderTest, Decode)
{
    SipReferSubHeader* pHeader = reinterpret_cast<SipReferSubHeader*>(
            SipReferSubHeader::GetNewObj(SipHeaderBase::REFER_SUB, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed for refer-sub*/
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(SIP_NULL, 0));

    /* ; value */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(";", 1));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipReferSubHeader*>(
            SipReferSubHeader::GetNewObj(SipHeaderBase::REFER_SUB, nullptr));

    /* any value other than true/false*/
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("*", 1));
    pHeader->SipDelete();

    /* Decode true valid value */
    pHeader = reinterpret_cast<SipReferSubHeader*>(
            SipReferSubHeader::GetNewObj(SipHeaderBase::REFER_SUB, nullptr));
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("true", 4));

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("true", &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipReferSubHeader*>(
            SipReferSubHeader::GetNewObj(SipHeaderBase::REFER_SUB, nullptr));

    /* Decode false valid value */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("false", 5));
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("false", &(aBuffer[0]));
    pHeader->SipDelete();
}

}  // namespace android
