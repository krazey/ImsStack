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

#include "SipConfiguration.h"
#include "msg/SipEventHeader.h"

namespace android
{

class SipEventHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipEventHeaderTest, EncodeAndDecode)
{
    SipHeaderBase* pHeader = SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    EXPECT_EQ(SIP_FALSE,
            pHeader->Encode(&pBuff, SIP_TRUE, SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("event-package", 13));

    SipHeaderBase* pCopyHeader = SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, pHeader);
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("event-package", &(aBuffer[0]));
    EXPECT_STREQ("event-package", objBuffer.GetCharString());

    pCopyHeader->SipDelete();

    pHeader = SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("event-package.event-template", 28));

    pCopyHeader = SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, pHeader);
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("event-package.event-template", &(aBuffer[0]));
    EXPECT_STREQ("event-package.event-template", objBuffer.GetCharString());

    pCopyHeader->SipDelete();

    pHeader = SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    // Consecutive DOTS, fail.
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("event-package.event-template1..event-template2", 46));

    pHeader->SipDelete();

    pHeader = SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    // Ending with DOT, fail.
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("event-package.event-template1.event-template2.", 46));

    pHeader->SipDelete();

    SipEventHeader* pEventsHeader = reinterpret_cast<SipEventHeader*>(
            SipEventHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr));
    ASSERT_TRUE(pEventsHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pEventsHeader->Decode("event-package.event-template1.event-template2", 45));

    EXPECT_EQ(2, pEventsHeader->GetTemplateCount());

    EXPECT_EQ(SIP_TRUE, pEventsHeader->IsTemplatePresent("event-template1"));
    EXPECT_EQ(SIP_FALSE, pEventsHeader->IsTemplatePresent("event-template3"));

    EXPECT_EQ(0, pEventsHeader->GetTemplateIndex("event-template1"));
    EXPECT_EQ(-1, pEventsHeader->GetTemplateIndex("event-template4"));

    EXPECT_STREQ("event-template2", pEventsHeader->GetTemplate(1));

    pEventsHeader->RemoveTemplate("event-template2");
    pEventsHeader->AddTemplate("event-template5");

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pEventsHeader->Encode(&pBuff));
    EXPECT_STREQ("event-package.event-template1.event-template5", &(aBuffer[0]));

    pEventsHeader->SipDelete();

    // Event header
    pHeader = SipEventHeader::GetNewObj(SipHeaderBase::EVENT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE,
            pHeader->Decode("event-package.event-template;event-param-name=event-param-value", 63));

    pCopyHeader = SipEventHeader::GetNewObj(SipHeaderBase::EVENT, pHeader);
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("event-package.event-template;event-param-name=event-param-value", &(aBuffer[0]));
    EXPECT_STREQ("event-package.event-template;event-param-name=event-param-value",
            objBuffer.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android
