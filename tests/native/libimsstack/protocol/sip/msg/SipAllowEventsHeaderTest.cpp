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
#include "msg/SipAllowEventsHeader.h"
#include "SipConfiguration.h"

namespace android
{

class SipAllowEventsHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipAllowEventsHeaderTest, EncodeHdrAndDecodeHdr)
{
    SipHeaderBase* pHeader = SipAllowEventsHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    EXPECT_EQ(SIP_FALSE,
            pHeader->EncodeHdr(&pBuff, SIP_TRUE, SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)"", 0));

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"event-package", 13));

    SipHeaderBase* pCopyHeader =
            SipAllowEventsHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, pHeader);
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("event-package", &(aBuffer[0]));
    EXPECT_STREQ("event-package", objBuffer.GetCharString());

    pCopyHeader->SipDelete();

    pHeader = SipAllowEventsHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"event-package.event-template", 28));

    pCopyHeader = SipAllowEventsHeader::GetNewObj(SipHeaderBase::ALLOW_EVENTS, pHeader);
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("event-package.event-template", &(aBuffer[0]));
    EXPECT_STREQ("event-package.event-template", objBuffer.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android
