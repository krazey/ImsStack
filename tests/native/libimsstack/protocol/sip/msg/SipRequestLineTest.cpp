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
#include "sip_abnfUtil.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipRequestLine.h"

namespace android {

class SipRequestLineTest : public ::testing::Test {

public:

protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};

TEST_F(SipRequestLineTest, CopyConstructor) {
    SipRequestLine *pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    pRequestLine->SetSipVersion(SIP_SIPVER);
    pRequestLine->SetMethod("INVITE");

    SipAddrSpec *pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec((char*)"sip:user@host", 13));

    pRequestLine->SetReqUri(pSipAddrSpec);
    pSipAddrSpec->SipDelete();

    SipRequestLine *pCopyRequestLine = new SipRequestLine(*pRequestLine);
    ASSERT_TRUE(pCopyRequestLine != nullptr);

    pSipAddrSpec->SipDelete();
    pRequestLine->SipDelete();

    pSipAddrSpec = pCopyRequestLine->GetReqUri();
    SipUri *pSipUri = pSipAddrSpec->GetSipUri();
    EXPECT_STREQ("user", pSipUri->GetUser());
    EXPECT_STREQ("host", pSipUri->GetHost());

    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();
    pCopyRequestLine->SipDelete();
}

TEST_F(SipRequestLineTest, IsValidComponent) {
    SipRequestLine *pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_EQ(SIP_FALSE, pRequestLine->IsValidComponent(nullptr));

    pRequestLine->SipDelete();
    pRequestLine = nullptr;

    pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_USER));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_PASSWORD));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_HOST));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_PORT));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_USER_PRM));
    EXPECT_EQ(SIP_FALSE, pRequestLine->IsValidComponent(SIP_METHOD));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_MADDR_PRM));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_TTL_PRM));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_TRNSPORT_PRM));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_LR_PRM));
    EXPECT_EQ(SIP_TRUE, pRequestLine->IsValidComponent(SIP_OTHER_PRM));
    EXPECT_EQ(SIP_FALSE, pRequestLine->IsValidComponent(SIP_HEADERS));

    pRequestLine->SipDelete();
    pRequestLine = nullptr;
}

TEST_F(SipRequestLineTest, EncodeRequestLine) {
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {0, };
    char *pBuff = &(aBuffer[0]);

    SipRequestLine *pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    /* Empty request line, fail */
    EXPECT_EQ(SIP_FALSE, pRequestLine->EncodeRequestLine(&pBuff));

    /* Only method set and no addrspec and sip version, fail */
    pRequestLine->SetMethod("INVITE");
    EXPECT_EQ(SIP_FALSE, pRequestLine->EncodeRequestLine(&pBuff));

    SipAddrSpec *pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    /* method and addrspec present and no sip version, fail */
    pRequestLine->SetReqUri(pSipAddrSpec);
    EXPECT_EQ(SIP_FALSE, pRequestLine->EncodeRequestLine(&pBuff));

    pRequestLine->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec((char*)"sip:user@host", 13));

    pRequestLine = new SipRequestLine((char*)"INVITE", pSipAddrSpec, SIP_SIPVER);
    ASSERT_TRUE(pRequestLine != nullptr);

    /* method, addrspec and sip version present, success */
    EXPECT_EQ(SIP_TRUE, pRequestLine->EncodeRequestLine(&pBuff));
    EXPECT_STREQ("INVITE sip:user@host SIP/2.0", &(aBuffer[0]));

    pRequestLine->SipDelete();
}

TEST_F(SipRequestLineTest, DecodeRequestLine) {
    SipRequestLine *pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_EQ(SIP_FALSE, pRequestLine->DecodeRequestLine((char*)"", 0));

    EXPECT_EQ(SIP_FALSE, pRequestLine->DecodeRequestLine((char*)"MESSAGE", 7));

    EXPECT_EQ(SIP_FALSE, pRequestLine->DecodeRequestLine((char*)"MESSAGE sip:abcd", 16));

    pRequestLine->SipDelete();

    pRequestLine = new SipRequestLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_EQ(SIP_TRUE, pRequestLine->DecodeRequestLine((char*)"MESSAGE sip:abcd SIP/2.0", 24));

    pRequestLine->SipDelete();
}

} // namespace android
