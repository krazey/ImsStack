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

#include "AStringBuffer.h"
#include "msg/SipInfoBase.h"

namespace android
{

class SipInfoBaseTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipInfoBaseTest, EncodeAndDecode)
{
    SipInfoBase* pHeader = reinterpret_cast<SipInfoBase*>(
            SipInfoBase::GetNewObj(SipHeaderBase::ALERT_INFO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    AStringBuffer objBuffer(64);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    /* only value and no params, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("<http://www.example.com/audio/xyz.wav>", 38));

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("<http://www.example.com/audio/xyz.wav>", &(aBuffer[0]));

    SipInfoBase* pCopyHeader = reinterpret_cast<SipInfoBase*>(
            SipInfoBase::GetNewObj(SipHeaderBase::ALERT_INFO, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("<http://www.example.com/audio/xyz.wav>", objBuffer.GetCharString());

    pCopyHeader->SipDelete();

    pHeader = reinterpret_cast<SipInfoBase*>(
            SipInfoBase::GetNewObj(SipHeaderBase::ALERT_INFO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* value and params, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("<http://www.example.com/audio/xyz.wav>;appearance=2", 51));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("<http://www.example.com/audio/xyz.wav>;appearance=2", &(aBuffer[0]));

    pCopyHeader = reinterpret_cast<SipInfoBase*>(
            SipInfoBase::GetNewObj(SipHeaderBase::ALERT_INFO, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_TRUE));

    EXPECT_STREQ("<http://www.example.com/audio/xyz.wav>;appearance=2", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("<http://www.example.com/audio/xyz.wav>", objBuffer.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android
