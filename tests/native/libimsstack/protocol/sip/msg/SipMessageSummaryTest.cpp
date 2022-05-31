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

class SipMessageSummaryTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipMessageSummaryTest, DecodeAndEncodeMessageSummary)
{
    SipMessageSummary* pMessageSummary = new SipMessageSummary();
    ASSERT_TRUE(pMessageSummary != nullptr);

    char* pSimpleMsgSummary = (char*)"Messages-Waiting: No\r\n\
Message-Account: sip:abcd@ims.com\r\n\
Fax-Message: 2/4\r\n\
Voice-Message: 1/3 (0/1)\r\n\
\r\n\
header1: value1\r\n\
header2: value2\r\n";

    int nLen = strlen(pSimpleMsgSummary);

    EXPECT_EQ(SIP_TRUE,
            pMessageSummary->DecodeMessageSummary(pSimpleMsgSummary, pSimpleMsgSummary + nLen));

    SipMessageSummary* pCopyMessageSummary = new SipMessageSummary(*pMessageSummary);
    ASSERT_TRUE(pCopyMessageSummary != nullptr);

    pMessageSummary->SipDelete();

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pCopyMessageSummary->EncodeMessageSummary(&pBuff));

    EXPECT_STREQ(pSimpleMsgSummary, &(aBuffer[0]));

    pCopyMessageSummary->SipDelete();

    pMessageSummary = new SipMessageSummary();
    ASSERT_TRUE(pMessageSummary != nullptr);

    pSimpleMsgSummary = (char*)"Messages-Waiting: No\r\n\
Fax-Message: 2/4\r\n\
Voice-Message: 1/3 (0/1)\r\n\
\r\n\
header1: value1\r\n\
header2: value2\r\n";

    nLen = strlen(pSimpleMsgSummary);

    EXPECT_EQ(SIP_TRUE,
            pMessageSummary->DecodeMessageSummary(pSimpleMsgSummary, pSimpleMsgSummary + nLen));

    pCopyMessageSummary = new SipMessageSummary(*pMessageSummary);
    ASSERT_TRUE(pCopyMessageSummary != nullptr);

    pMessageSummary->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessageSummary->EncodeMessageSummary(&pBuff));

    EXPECT_STREQ(pSimpleMsgSummary, &(aBuffer[0]));

    pCopyMessageSummary->SipDelete();

    pMessageSummary = new SipMessageSummary();
    ASSERT_TRUE(pMessageSummary != nullptr);

    pSimpleMsgSummary = (char*)"Messages-Waiting: No\r\n\
Message-Account: sip:abcd@ims.com\r\n\
\r\n\
header1: value1\r\n\
header2: value2\r\n";

    nLen = strlen(pSimpleMsgSummary);

    EXPECT_EQ(SIP_TRUE,
            pMessageSummary->DecodeMessageSummary(pSimpleMsgSummary, pSimpleMsgSummary + nLen));

    pCopyMessageSummary = new SipMessageSummary(*pMessageSummary);
    ASSERT_TRUE(pCopyMessageSummary != nullptr);

    pMessageSummary->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessageSummary->EncodeMessageSummary(&pBuff));

    EXPECT_STREQ(pSimpleMsgSummary, &(aBuffer[0]));

    pCopyMessageSummary->SipDelete();

    pMessageSummary = new SipMessageSummary();
    ASSERT_TRUE(pMessageSummary != nullptr);

    pSimpleMsgSummary = (char*)"Messages-Waiting: Yes\r\n\
\r\n\
header1: value1\r\n\
header2: value2\r\n";

    nLen = strlen(pSimpleMsgSummary);

    EXPECT_EQ(SIP_TRUE,
            pMessageSummary->DecodeMessageSummary(pSimpleMsgSummary, pSimpleMsgSummary + nLen));

    pCopyMessageSummary = new SipMessageSummary(*pMessageSummary);
    ASSERT_TRUE(pCopyMessageSummary != nullptr);

    pMessageSummary->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessageSummary->EncodeMessageSummary(&pBuff));

    EXPECT_STREQ(pSimpleMsgSummary, &(aBuffer[0]));

    pCopyMessageSummary->SipDelete();

    pMessageSummary = new SipMessageSummary();
    ASSERT_TRUE(pMessageSummary != nullptr);

    pSimpleMsgSummary = (char*)"Messages-Waiting: Yes\r\n";

    nLen = strlen(pSimpleMsgSummary);

    EXPECT_EQ(SIP_TRUE,
            pMessageSummary->DecodeMessageSummary(pSimpleMsgSummary, pSimpleMsgSummary + nLen));

    pCopyMessageSummary = new SipMessageSummary(*pMessageSummary);
    ASSERT_TRUE(pCopyMessageSummary != nullptr);

    pMessageSummary->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessageSummary->EncodeMessageSummary(&pBuff));

    EXPECT_STREQ(pSimpleMsgSummary, &(aBuffer[0]));

    pCopyMessageSummary->SipDelete();
}

}  // namespace android