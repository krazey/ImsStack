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
#include "msg/SipAddrSpec.h"
#include "msg/SipRequestLine.h"

namespace android
{

class SipRequestLineTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipRequestLineTest, CopyConstructor)
{
    SipRequestLine* pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    pRequestLine->SetSipVersion(SIP_SIPVER);
    pRequestLine->SetMethod("INVITE");

    SipAddrSpec* pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec("sip:user@host", 13));

    pRequestLine->SetReqUri(pSipAddrSpec);
    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec("sip:user@host", 13));

    /* Calling SetReqUri again to verify already set uri is deleted and assigned new uri */
    pRequestLine->SetReqUri(pSipAddrSpec);
    pSipAddrSpec->SipDelete();

    SipRequestLine* pCopyRequestLine = new SipRequestLine(*pRequestLine);
    ASSERT_TRUE(pCopyRequestLine != nullptr);

    pRequestLine->SipDelete();

    pSipAddrSpec = pCopyRequestLine->GetReqUri();
    SipUri* pSipUri = pSipAddrSpec->GetSipUri();
    EXPECT_STREQ("user", pSipUri->GetUser());
    EXPECT_STREQ("host", pSipUri->GetHost());

    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();
    pCopyRequestLine->SipDelete();
}

TEST_F(SipRequestLineTest, EncodeRequestLine)
{
    const int BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    SipRequestLine* pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    /* Empty request line, fail */
    EXPECT_EQ(SIP_FALSE, pRequestLine->EncodeRequestLine(&pBuff));

    /* Only method set and no addrspec and sip version, fail */
    pRequestLine->SetMethod("INVITE");
    EXPECT_EQ(SIP_FALSE, pRequestLine->EncodeRequestLine(&pBuff));

    SipAddrSpec* pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    /* method and addrspec present and no sip version, fail */
    pRequestLine->SetReqUri(pSipAddrSpec);
    EXPECT_EQ(SIP_FALSE, pRequestLine->EncodeRequestLine(&pBuff));

    pRequestLine->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec("sip:user@host", 13));

    pRequestLine = new SipRequestLine("INVITE", pSipAddrSpec, SIP_SIPVER);
    ASSERT_TRUE(pRequestLine != nullptr);

    /* method, addrspec and sip version present, success */
    EXPECT_EQ(SIP_TRUE, pRequestLine->EncodeRequestLine(&pBuff));
    EXPECT_STREQ("INVITE sip:user@host SIP/2.0", &(aBuffer[0]));

    pRequestLine->SipDelete();
}

TEST_F(SipRequestLineTest, DecodeRequestLine)
{
    SipRequestLine* pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_EQ(SIP_FALSE, pRequestLine->DecodeRequestLine("", 0));

    EXPECT_EQ(SIP_FALSE, pRequestLine->DecodeRequestLine("MESSAGE", 7));

    EXPECT_EQ(SIP_FALSE, pRequestLine->DecodeRequestLine("MESSAGE sip:abcd", 16));

    pRequestLine->SipDelete();

    pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_EQ(SIP_TRUE, pRequestLine->DecodeRequestLine("MESSAGE sip:abcd SIP/2.0", 24));

    pRequestLine->SipDelete();
}

}  // namespace android
