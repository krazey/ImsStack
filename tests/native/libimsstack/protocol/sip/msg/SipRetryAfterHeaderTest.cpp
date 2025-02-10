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

#include "AStringBuffer.h"
#include "msg/SipRetryAfterHeader.h"

namespace android
{

class SipRetryAfterHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipRetryAfterHeaderTest, Encode)
{
    SipRetryAfterHeader* pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetDeltaSec(120);
    pHeader->SetComment("server message");

    AStringBuffer objBuffer(64);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("120(server message)", objBuffer.GetCharString());

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("360(sip server overloaded);duration=7200", 40));

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));

    EXPECT_STREQ("360(sip server overloaded);duration=7200", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("360(sip server overloaded)", objBuffer.GetCharString());

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("360 (sip server overloaded);duration=7200", 41));

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));

    EXPECT_STREQ("360(sip server overloaded);duration=7200", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("360(sip server overloaded)", objBuffer.GetCharString());

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("360 ( sip server overloaded ) ;duration=7200", 44));

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));

    EXPECT_STREQ("360( sip server overloaded );duration=7200", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("360( sip server overloaded )", objBuffer.GetCharString());

    pHeader->SipDelete();
}

TEST_F(SipRetryAfterHeaderTest, EncodeAndDecode)
{
    SipRetryAfterHeader* pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty buffer, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    /* only delta seconds to retry present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("20", 2));
    EXPECT_EQ(20, pHeader->GetDeltaSec());

    SipRetryAfterHeader* pCopyHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("20", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds to retry and comment present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("30(sip server overloaded)", 25));

    pCopyHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("30(sip server overloaded)", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds to retry,comment and extra parameter present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("15(sip server overloaded);duration=4800", 39));
    EXPECT_STREQ("sip server overloaded", pHeader->GetComment());

    pCopyHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("15(sip server overloaded);duration=4800", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds and space to retry,comment and extra parameter present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("15 (sip server overloaded);duration=4800", 40));
    EXPECT_STREQ("sip server overloaded", pHeader->GetComment());

    pCopyHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("15(sip server overloaded);duration=4800", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds and space to retry,comment with space and extra parameter present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("15 ( sip server overloaded ) ;duration=4800", 43));
    EXPECT_STREQ(" sip server overloaded ", pHeader->GetComment());

    pCopyHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("15( sip server overloaded );duration=4800", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds to retry,empty comment and extra parameter present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("15();duration=1800", 18));
    EXPECT_STREQ("", pHeader->GetComment());
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pCopyHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("15();duration=1800", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds to retry and comment present with no closing parenthesis, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("30(sip server overloaded", 24));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* delta seconds to retry and comment present with no opening parenthesis, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("30 sip server overloaded)", 25));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipRetryAfterHeader*>(
            SipRetryAfterHeader::GetNewObj(SipHeaderBase::RETRY_AFTER_SEC, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* extra invalid string after comment, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("30(sip server overloaded)InvalidString", 38));

    pHeader->SipDelete();
}

}  // namespace android
