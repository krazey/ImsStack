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

    SipMsgBody* pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer((char*)"This is body1", 13));
    EXPECT_EQ(SIP_TRUE, pList->AddBody(pMessageBody));

    pMessageBody->SipDelete();

    unsigned int nLen = 0;

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen));

    EXPECT_STREQ("This is body1", &(aBuffer[0]));
    EXPECT_EQ(13, nLen);

    pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer((char*)"This is body2", 13));
    EXPECT_EQ(SIP_TRUE, pList->AddBody(pMessageBody));

    pMessageBody->SipDelete();

    nLen = 0;
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pList->GetEncodedMessageBody(&pBuff, nLen, (char*)"unique-boundary-1"));

    char* pMsg = (char*)"--unique-boundary-1\r\n\
\r\n\
This is body1\r\n\
--unique-boundary-1\r\n\
\r\n\
This is body2\r\n\
--unique-boundary-1--\r\n";

    EXPECT_STREQ(pMsg, &(aBuffer[0]));
    EXPECT_EQ(nLen, strlen(pMsg));

    pList->SipDelete();
}

TEST_F(SipMsgBodyListTest, DecodeSingleBody)
{
    SipMsgBodyList* pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    char* pSingleBody = (char*)"This is a single body,\r\n\
and no headers and boundary present\r\n";
    int nLen = strlen(pSingleBody);

    EXPECT_EQ(SIP_TRUE, pList->DecodeSingleBody(pSingleBody, pSingleBody + nLen));

    EXPECT_EQ(1, pList->GetMsgBodyCount());

    SipMsgBody* pMessageBody = pList->GetBodyByIndex(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_STREQ(pSingleBody, pMessageBody->GetBuffer());

    pMessageBody->SipDelete();
}

TEST_F(SipMsgBodyListTest, DecodeMessageSummaryBody)
{
    SipMsgBodyList* pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    char* pMessageSummary = (char*)"Messages-Waiting: No\r\n";
    int nLen = strlen(pMessageSummary);

    EXPECT_EQ(SIP_TRUE, pList->DecodeMessageSummaryBody(pMessageSummary, pMessageSummary + nLen));

    EXPECT_EQ(1, pList->GetMsgBodyCount());

    SipMsgBody* pMessageBody = pList->GetBodyByIndex(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    pMessageBody->SipDelete();
}

TEST_F(SipMsgBodyListTest, DecodeMIMEBody)
{
    SipMsgBodyList* pList = new SipMsgBodyList();
    ASSERT_TRUE(pList != nullptr);

    EXPECT_EQ(SIP_FALSE, pList->DecodeMIMEBody(nullptr, nullptr, nullptr));

    char* pMsg = (char*)"--unique-boundary\r\n\
Content-Type: application/body1\r\n\
\r\n\
Test body 1\r\n\
--unique-boundary\r\n\
Content-Type: application/body2\r\n\
\r\n\
Test body 2\r\n\
--unique-boundary--\r\n";

    unsigned int nLen = strlen(pMsg);

    EXPECT_EQ(SIP_TRUE, pList->DecodeMIMEBody(pMsg, pMsg + nLen, (char*)"unique-boundary"));

    EXPECT_EQ(2, pList->GetMsgBodyCount());

    SipMsgBody* pMessageBody = pList->GetBodyByIndex(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_STREQ("Test body 1", pMessageBody->GetBuffer());

    pMessageBody->SipDelete();

    pMessageBody = pList->GetBodyByIndex(1);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_STREQ("Test body 2", pMessageBody->GetBuffer());

    pMessageBody->SipDelete();
}

}  // namespace android