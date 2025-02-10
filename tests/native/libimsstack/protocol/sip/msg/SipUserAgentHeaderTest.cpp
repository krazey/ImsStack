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
#include "msg/SipUserAgentHeader.h"

namespace android
{

class SipUserAgentHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipUserAgentHeaderTest, Encode)
{
    SipUserAgentHeader* pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("ims (comment) abcd", 18));

    AStringBuffer objValue(64);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));

    const AString& headerValue = objValue.GetString();

    EXPECT_EQ(SIP_TRUE, headerValue.Equals("ims (comment) abcd"));

    pHeader->SipDelete();
}

TEST_F(SipUserAgentHeaderTest, EncodeAndDecode)
{
    SipUserAgentHeader* pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    /* Empty buffer, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    /* no comment and only value, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("ims", 3));

    SipUserAgentHeader* pCopyHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->IsValidHeader());

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("ims", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* value and comment present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("ims (comment message)", 21));

    pCopyHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("ims (comment message)", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* only comment present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("(comment message)", 17));

    pCopyHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("(comment message)", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* value, comment and value present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("ims (comment message) 2.0", 25));

    pCopyHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("ims (comment message) 2.0", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* value, empty comment and value present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("ims () user-agent", 17));

    pCopyHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_STREQ("ims () user-agent", &(aBuffer[0]));

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* value, comment not properly closed, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("ims (comment not closed", 23));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipUserAgentHeader*>(
            SipUserAgentHeader::GetNewObj(SipHeaderBase::USER_AGENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* value, comment with no opening parenthesis, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("ims comment with no opening parenthesis)", 40));

    pHeader->SipDelete();
}

}  // namespace android
