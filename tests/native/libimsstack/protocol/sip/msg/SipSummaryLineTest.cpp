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

class SipSummaryLineTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipSummaryLineTest, DecodeAndEncodeSummaryLine)
{
    SipSummaryLine* pSummaryLine = new SipSummaryLine();
    ASSERT_TRUE(pSummaryLine != nullptr);

    char* pSummary = (char*)"voice-message: 1/3 (0/1)";
    int nLen = strlen(pSummary);

    EXPECT_EQ(SIP_TRUE, pSummaryLine->DecodeSummaryLine(pSummary, pSummary + nLen));

    EXPECT_STREQ("voice-message", pSummaryLine->GetMedia());
    EXPECT_EQ(1, pSummaryLine->GetNewMessages());
    EXPECT_EQ(3, pSummaryLine->GetOldMessages());
    EXPECT_EQ(0, pSummaryLine->GetNewUrgentMessages());
    EXPECT_EQ(1, pSummaryLine->GetOldUrgentMessages());

    SipSummaryLine* pCopySummaryLine = new SipSummaryLine(*pSummaryLine);
    ASSERT_TRUE(pCopySummaryLine != nullptr);

    pSummaryLine->SipDelete();

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pCopySummaryLine->EncodeSummaryLine(&pBuff));

    EXPECT_STREQ(pSummary, &(aBuffer[0]));

    pCopySummaryLine->SipDelete();

    pSummaryLine = new SipSummaryLine();
    ASSERT_TRUE(pSummaryLine != nullptr);

    pSummary = (char*)"fax-message: 3/4";
    nLen = strlen(pSummary);

    EXPECT_EQ(SIP_TRUE, pSummaryLine->DecodeSummaryLine(pSummary, pSummary + nLen));

    EXPECT_STREQ("fax-message", pSummaryLine->GetMedia());
    EXPECT_EQ(3, pSummaryLine->GetNewMessages());
    EXPECT_EQ(4, pSummaryLine->GetOldMessages());
    EXPECT_EQ(0, pSummaryLine->GetNewUrgentMessages());
    EXPECT_EQ(0, pSummaryLine->GetOldUrgentMessages());

    pCopySummaryLine = new SipSummaryLine(*pSummaryLine);
    ASSERT_TRUE(pCopySummaryLine != nullptr);

    pSummaryLine->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopySummaryLine->EncodeSummaryLine(&pBuff));

    EXPECT_STREQ(pSummary, &(aBuffer[0]));

    pCopySummaryLine->SipDelete();
}

}  // namespace android