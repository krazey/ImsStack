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

#include "msg/SipWarningHeader.h"

namespace android
{

class SipWarningHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipWarningHeaderTest, Encode)
{
    SipWarningHeader* pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetWarnCode(99);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetWarnCode(1000);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetWarnCode(300);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetWarnAgent("Warn-Agent");

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetWarnText("\"Warn-Text\"");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("300 Warn-Agent \"Warn-Text\"", objValue.GetCharString());
    EXPECT_STREQ("300 Warn-Agent \"Warn-Text\"", &(aBuffer[0]));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetWarnCode(300);
    pHeader->SetWarnAgent("Warn-Agent");
    pHeader->SetWarnText("\"Warn Text\"");

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("300 Warn-Agent \"Warn Text\"", objValue.GetCharString());
    EXPECT_STREQ("300 Warn-Agent \"Warn Text\"", &(aBuffer[0]));

    pHeader->SetWarnText("Warn Text");

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("300 Warn-Agent \"Warn Text\"", objValue.GetCharString());
    EXPECT_STREQ("300 Warn-Agent \"Warn Text\"", &(aBuffer[0]));

    pHeader->SetWarnText("W");

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("300 Warn-Agent \"W\"", objValue.GetCharString());
    EXPECT_STREQ("300 Warn-Agent \"W\"", &(aBuffer[0]));

    pHeader->SetWarnText("");

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("300 Warn-Agent \"\"", objValue.GetCharString());
    EXPECT_STREQ("300 Warn-Agent \"\"", &(aBuffer[0]));

    pHeader->SetWarnText(SIP_NULL);

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("300 Warn-Agent \"\"", objValue.GetCharString());
    EXPECT_STREQ("300 Warn-Agent \"\"", &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipWarningHeaderTest, Decode)
{
    SipWarningHeader* pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("no-delimiter-space", 18));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("99 code-less-than-3digit", 24));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("1234 code-more-than-3digit", 26));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("301 no-warn-text", 16));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("301 warn-agent wrong-warn-test", 30));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("301 warn-agent \"wrong-warn-test", 31));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("301 warn-agent wrong-warn-test\"", 31));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipWarningHeader*>(
            SipWarningHeader::GetNewObj(SipHeaderBase::WARNING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("301 warn-agent \"warn-text\"", 26));

    EXPECT_EQ(301, pHeader->GetWarnCode());
    EXPECT_STREQ("warn-agent", pHeader->GetWarnAgent());
    EXPECT_STREQ("\"warn-text\"", pHeader->GetWarnText());

    pHeader->SipDelete();
}

}  // namespace android
