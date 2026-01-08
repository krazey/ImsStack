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

TEST_F(SipUnknownHeaderTest, Constructor)
{
    SipUnknownHeader* pUnknownHeader = new SipUnknownHeader();
    ASSERT_TRUE(pUnknownHeader != nullptr);

    pUnknownHeader->SetHdrType(SipHeaderBase::UNKNOWN);
    EXPECT_EQ(SipHeaderBase::UNKNOWN, pUnknownHeader->GetHdrType());

    pUnknownHeader->SipDelete();
}
TEST_F(SipUnknownHeaderTest, Encode)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHeader != nullptr);

    pUnknownHeader->SetHeaderValue("UnknownHeaderValue");
    EXPECT_EQ(SIP_FALSE, pUnknownHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pUnknownHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pUnknownHeader->IsValidHeader());

    pUnknownHeader->SetHeaderName("UnknownHeaderName");
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("UnknownHeaderName: UnknownHeaderValue", &(aBuffer[0]));
    EXPECT_STREQ("UnknownHeaderName: UnknownHeaderValue", objBuffer.GetCharString());
    pUnknownHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHeader != nullptr);
    pUnknownHeader->SetHeaderName("UnknownHeaderName");
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("UnknownHeaderName: ", &(aBuffer[0]));
    EXPECT_STREQ("UnknownHeaderName: ", objBuffer.GetCharString());
    pUnknownHeader->SipDelete();
}

}  // namespace android
