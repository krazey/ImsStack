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

#include "SipAbnfUtil.h"
#include "msg/SipMsgUtil.h"
#include "msg/SipStatusLine.h"

namespace android
{

class SipStatusLineTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipStatusLineTest, EncodeStatusLine)
{
    SipStatusLine* pStatusLine = new SipStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    /* Empty object, fail */
    EXPECT_EQ(SIP_FALSE, pStatusLine->EncodeStatusLine(&pBuff));

    /* Only status code present, fail */
    pStatusLine->SetStatusCode("183");

    EXPECT_EQ(SIP_FALSE, pStatusLine->EncodeStatusLine(&pBuff));

    /* Status code, reason phrase present and sip version absent, fail */
    pStatusLine->SetReasonPhrase("Session Progress");

    EXPECT_EQ(SIP_FALSE, pStatusLine->EncodeStatusLine(&pBuff));

    pStatusLine->SetSipVersion(SIP_SIPVER);

    /* sip version,status code and reason phrase present, success */
    EXPECT_EQ(SIP_TRUE, pStatusLine->EncodeStatusLine(&pBuff));

    EXPECT_STREQ("SIP/2.0 183 Session Progress", &(aBuffer[0]));

    pStatusLine->SipDelete();

    pStatusLine = new SipStatusLine(SIP_SIPVER, "180", "Ringing");
    ASSERT_TRUE(pStatusLine != nullptr);

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    /* sip version,status code and reason phrase present, success */
    EXPECT_EQ(SIP_TRUE, pStatusLine->EncodeStatusLine(&pBuff));

    EXPECT_STREQ("SIP/2.0 180 Ringing", &(aBuffer[0]));

    pStatusLine->SipDelete();

    pStatusLine = new SipStatusLine("100", "Trying");
    ASSERT_TRUE(pStatusLine != nullptr);

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    /* sip version,status code and reason phrase present, success */
    EXPECT_EQ(SIP_TRUE, pStatusLine->EncodeStatusLine(&pBuff));

    EXPECT_STREQ("SIP/2.0 100 Trying", &(aBuffer[0]));

    pStatusLine->SipDelete();
}

TEST_F(SipStatusLineTest, DecodeStatusLine)
{
    SipStatusLine* pStatusLine = new SipStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);

    /* Empty buffer, fail */
    EXPECT_EQ(SIP_FALSE, pStatusLine->DecodeStatusLine("", 0));
    /* only sip version present, fail */
    EXPECT_EQ(SIP_FALSE, pStatusLine->DecodeStatusLine("SIP/2.0", 7));
    EXPECT_EQ(SIP_SC_INVALID, pStatusLine->GetStatusCodeAsInt());
    SIP_INT16 nStatusCode;
    EXPECT_FALSE(pStatusLine->GetStatusCode(&nStatusCode));
    EXPECT_EQ(0, nStatusCode);

    /* reason phrase missing, fail */
    EXPECT_EQ(SIP_FALSE, pStatusLine->DecodeStatusLine("SIP/2.0 480", 11));
    pStatusLine->SipDelete();

    pStatusLine = new SipStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    /* sip version, status code and reason phrase present, success */
    EXPECT_EQ(SIP_TRUE, pStatusLine->DecodeStatusLine("SIP/2.0 480 Unavailable", 23));
    EXPECT_EQ(480, pStatusLine->GetStatusCodeAsInt());
    EXPECT_STREQ("480", pStatusLine->GetStatusCode());
    EXPECT_STREQ("SIP/2.0", pStatusLine->GetSipVersion());
    EXPECT_STREQ("Unavailable", pStatusLine->GetReasonPhrase());
    pStatusLine->SipDelete();
}

}  // namespace android
