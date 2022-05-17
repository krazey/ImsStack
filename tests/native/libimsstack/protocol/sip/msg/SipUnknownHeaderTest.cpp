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
#include "msg/SipHeaders.h"
#include "msg/SipUnknownHeader.h"

namespace android
{

class SipUnknownHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipUnknownHeaderTest, EncodeHdr)
{
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pUnknownHeader->SetHeaderName(SIP_NULL));
    EXPECT_EQ(SIP_FALSE, pUnknownHeader->SetHeaderValue(SIP_NULL));

    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderValue("UnknownHeaderValue"));
    EXPECT_EQ(SIP_FALSE, pUnknownHeader->EncodeHdr(&pBuff));

    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderName("UnknownHeaderName"));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("UnknownHeaderName: UnknownHeaderValue", &(aBuffer[0]));
    pUnknownHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderName("UnknownHeaderName"));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("UnknownHeaderName: ", &(aBuffer[0]));
    pUnknownHeader->SipDelete();
}

}  // namespace android
