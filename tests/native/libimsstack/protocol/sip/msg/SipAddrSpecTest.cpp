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
#include "msg/SipAddrSpec.h"

namespace android
{

class SipAddrSpecTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipAddrSpecTest, CopyConstructor)
{
    SipAddrSpec* pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    SipAddrSpec* pCopySipAddrSpec = new SipAddrSpec(*pSipAddrSpec);
    ASSERT_TRUE(pCopySipAddrSpec != nullptr);

    pSipAddrSpec->SipDelete();
    pCopySipAddrSpec->SipDelete();
}

TEST_F(SipAddrSpecTest, EncodeAndDecodeAddrSpec)
{
    SipAddrSpec* pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_FALSE, pSipAddrSpec->EncodeAddrSpec(&pBuff));

    EXPECT_EQ(SIP_FALSE, pSipAddrSpec->DecodeAddrSpec(nullptr, 0));

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec((char*)"http://absoluteuri.addrspec", 27));

    EXPECT_EQ(SipUri::SCHEME_ABS, pSipAddrSpec->GetUriScheme());
    EXPECT_STREQ("http://absoluteuri.addrspec", pSipAddrSpec->GetAbsUri());

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_STREQ("http://absoluteuri.addrspec", &(aBuffer[0]));

    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec((char*)"sip:192.168.2.2:9090", 22));

    EXPECT_EQ(SipUri::SCHEME_SIP, pSipAddrSpec->GetUriScheme());

    SipUri* pSipUri = pSipAddrSpec->GetSipUri();
    EXPECT_STREQ("192.168.2.2", pSipUri->GetHost());
    EXPECT_EQ(9090, pSipUri->GetPort());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_STREQ("sip:192.168.2.2:9090", &(aBuffer[0]));

    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec((char*)"sips:192.168.2.8:9091", 22));

    EXPECT_EQ(SipUri::SCHEME_SIPS, pSipAddrSpec->GetUriScheme());

    pSipUri = pSipAddrSpec->GetSipUri();
    EXPECT_STREQ("192.168.2.8", pSipUri->GetHost());
    EXPECT_EQ(9091, pSipUri->GetPort());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_STREQ("sips:192.168.2.8:9091", &(aBuffer[0]));

    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();
}

}  // namespace android
