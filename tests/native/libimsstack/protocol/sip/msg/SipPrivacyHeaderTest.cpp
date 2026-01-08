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

#include "msg/SipPrivacyHeader.h"

namespace android
{

class SipPrivacyHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipPrivacyHeaderTest, CopyConstructor)
{
    SipPrivacyHeader* pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("id"));

    SipPrivacyHeader* pCopyHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipPrivacyHeaderTest, IsValidHeader)
{
    SipPrivacyHeader* pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* empty Privacy not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    /* Add Privacy value */
    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("user"));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
}

TEST_F(SipPrivacyHeaderTest, AddPrivacy)
{
    SipPrivacyHeader* pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* empty PRIVACY not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->AddPrivacy(SIP_NULL));

    /* Add Privacy value */
    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("none"));

    pHeader->SipDelete();
}

TEST_F(SipPrivacyHeaderTest, EncodeHdr)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    SipPrivacyHeader* pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty privacy value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    /* Encode valid value */
    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("user"));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("user", &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("id"));
    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("session"));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("id;session", &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipPrivacyHeaderTest, Encode)
{
    AStringBuffer objBuffer(512);

    SipPrivacyHeader* pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty privacy value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));

    /* Encode valid value */
    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("user"));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("user", objBuffer.GetCharString());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("id"));
    EXPECT_EQ(SIP_TRUE, pHeader->AddPrivacy("session"));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("id;session", objBuffer.GetCharString());

    pHeader->SipDelete();
}

TEST_F(SipPrivacyHeaderTest, Decode)
{
    SipPrivacyHeader* pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed for privacy*/
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(SIP_NULL, 0));

    /* Empty value after ; not allowed for privacy*/
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("user;", 5));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    /* Decode ; value */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode(";", 1));
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));

    /* Decode valid value */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("history", 7));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("history", &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPrivacyHeader*>(
            SipPrivacyHeader::GetNewObj(SipHeaderBase::PRIVACY, nullptr));
    /* Decode more than one value */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("user;header;id", 14));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("user;header;id", &(aBuffer[0]));
    pHeader->SipDelete();
}

}  // namespace android
