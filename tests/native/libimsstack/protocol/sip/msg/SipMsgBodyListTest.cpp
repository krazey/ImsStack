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

#include "msg/SipMsgBody.h"

namespace android
{

class SipMsgBodyListTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipMsgBodyListTest, GetEncodedMessageBody)
{
    SipMsgBodyList* pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    unsigned int nLen = 0;

    const int BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    /* No message body, success */
    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen, nullptr));

    SipMsgBody* pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer("This is body1", 13));
    EXPECT_EQ(SIP_TRUE, pList->AddBody(pMessageBody));

    pMessageBody->SipDelete();

    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen));

    EXPECT_STREQ("This is body1", &(aBuffer[0]));
    EXPECT_EQ(13, nLen);

    pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer("This is body2", 13));
    EXPECT_EQ(SIP_TRUE, pList->AddBody(pMessageBody));

    pMessageBody->SipDelete();

    nLen = 0;
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen, "unique-boundary-1"));

    const SIP_CHAR* pMsg = "--unique-boundary-1\r\n\
\r\n\
This is body1\r\n\
--unique-boundary-1\r\n\
\r\n\
This is body2\r\n\
--unique-boundary-1--\r\n";

    EXPECT_STREQ(pMsg, &(aBuffer[0]));
    EXPECT_EQ(nLen, strlen(pMsg));

    pList->SipDelete();

    // Message body with null character inside buffer (same buffer as previous example with extra
    // '\0')
    pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer("This is body1", 13));
    EXPECT_EQ(SIP_TRUE, pList->AddBody(pMessageBody));

    pMessageBody->SipDelete();

    pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer("This is \0body2", 14));  // '\0' introduces
    EXPECT_EQ(SIP_TRUE, pList->AddBody(pMessageBody));

    pMessageBody->SipDelete();

    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen));

    nLen = 0;
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen, "unique-boundary-1"));

    const SIP_CHAR* pNullCharMsg = "--unique-boundary-1\r\n\
\r\n\
This is body1\r\n\
--unique-boundary-1\r\n\
\r\n\
This is \0body2\r\n\
--unique-boundary-1--\r\n";

    // pNullCharMsg length = pMsg + 1 (extra '\0' character)
    EXPECT_TRUE(memcmp(pNullCharMsg, &(aBuffer[0]), strlen(pMsg) + 1) == 0);
    EXPECT_EQ(nLen, strlen(pMsg) + 1);
    EXPECT_TRUE(nLen > strlen(&(aBuffer[0])));  // null character present check

    pList->SipDelete();
}

TEST_F(SipMsgBodyListTest, DecodeSingleBody)
{
    SipMsgBodyList* pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    const SIP_CHAR* pSingleBody = "This is a single body,\r\n\
and no headers and boundary present\r\n";
    int nLen = strlen(pSingleBody);

    EXPECT_EQ(SIP_TRUE, pList->DecodeSingleBody(pSingleBody, pSingleBody + nLen));

    EXPECT_EQ(1, pList->GetMsgBodyCount());

    SipMsgBody* pMessageBody = pList->GetBodyByIndex(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_STREQ(pSingleBody, pMessageBody->GetBuffer());

    pMessageBody->SipDelete();

    pMessageBody = pList->GetBodyByIndex(1);
    ASSERT_TRUE(pMessageBody == nullptr);
}

TEST_F(SipMsgBodyListTest, DecodeMIMEBody)
{
    SipMsgBodyList* pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    EXPECT_EQ(SIP_FALSE, pList->DecodeMIMEBody(nullptr, nullptr, nullptr));

    const SIP_CHAR* pMsg = "\r\n--unique-boundary\r\n\
Content-Type: application/body1\r\n\
\r\n\
Test body 1\r\n\
--unique-boundary\r\n\
Content-Type: application/body2\r\n\
\r\n\
Test body 2\r\n\
--unique-boundary--\r\n";

    unsigned int nLen = strlen(pMsg);

    EXPECT_EQ(SIP_TRUE, pList->DecodeMIMEBody(pMsg, pMsg + nLen, "unique-boundary"));

    EXPECT_EQ(2, pList->GetMsgBodyCount());

    SipMsgBody* pMessageBody = pList->GetBodyByIndex(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_STREQ("Test body 1", pMessageBody->GetBuffer());

    pMessageBody->SipDelete();

    pMessageBody = pList->GetBodyByIndex(1);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_STREQ("Test body 2", pMessageBody->GetBuffer());

    pMessageBody->SipDelete();
    pList->SipDelete();

    /* boundary mismatch, fail */
    pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    pMsg = "--unique-boundary\r\n\
Content-Type: application/body1\r\n\
\r\n\
Test body 1\r\n\
--unique-boundary\r\n\
Content-Type: application/body2\r\n\
\r\n\
Test body 2\r\n\
--unique-boundary--\r\n";

    nLen = strlen(pMsg);

    EXPECT_EQ(SIP_FALSE, pList->DecodeMIMEBody(pMsg, pMsg + nLen, "mismatch-boundary"));

    pList->SipDelete();

    /* No CRLF present, fail */
    pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    pMsg = "--unique-boundary";

    nLen = strlen(pMsg);

    EXPECT_EQ(SIP_FALSE, pList->DecodeMIMEBody(pMsg, pMsg + nLen, "unique-boundary"));

    pList->SipDelete();
}

}  // namespace android
