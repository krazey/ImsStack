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

#include "msg/SipGeolocationRoutingHeader.h"

namespace android
{

class SipGeolocationRoutingHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipGeolocationRoutingHeaderTest, DecodeAndEncode)
{
    SipGeolocationRoutingHeader* pHeader = reinterpret_cast<SipGeolocationRoutingHeader*>(
            SipGeolocationRoutingHeader::GetNewObj(SipHeaderBase::GEOLOCATION_ROUTING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    const SIP_INT32 BUFFER_SIZE = 64;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(64);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("no", 2));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("no", &(aBuffer[0]));
    EXPECT_STREQ("no", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipGeolocationRoutingHeader*>(
            SipGeolocationRoutingHeader::GetNewObj(SipHeaderBase::GEOLOCATION_ROUTING, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("name=value", 10));

    SipGeolocationRoutingHeader* pCopyHeader = reinterpret_cast<SipGeolocationRoutingHeader*>(
            SipGeolocationRoutingHeader::GetNewObj(SipHeaderBase::GEOLOCATION_ROUTING, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(&pBuff));

    EXPECT_STREQ("name=value", &(aBuffer[0]));
    EXPECT_STREQ("name=value", objBuffer.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android
