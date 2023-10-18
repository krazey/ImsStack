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

    AStringBuffer objBuffer(256);

    EXPECT_EQ(SIP_FALSE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_EQ(SIP_FALSE, pSipAddrSpec->Encode(objBuffer, SIP_FALSE));

    EXPECT_EQ(SIP_FALSE, pSipAddrSpec->DecodeAddrSpec(nullptr, 0));

    EXPECT_EQ(SIP_TRUE,
            pSipAddrSpec->DecodeAddrSpec(const_cast<char*>("http://absoluteuri.addrspec"), 27));

    EXPECT_EQ(SipUri::SCHEME_ABS, pSipAddrSpec->GetUriScheme());
    EXPECT_STREQ("http://absoluteuri.addrspec", pSipAddrSpec->GetAbsUri());

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("http://absoluteuri.addrspec", &(aBuffer[0]));
    EXPECT_STREQ("http://absoluteuri.addrspec", objBuffer.GetCharString());

    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    char* pUri = const_cast<char*>("sip:192.168.2.2:9090;user=phone");
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec(pUri, strlen(pUri)));

    EXPECT_EQ(SipUri::SCHEME_SIP, pSipAddrSpec->GetUriScheme());

    SipUri* pSipUri = pSipAddrSpec->GetSipUri();
    EXPECT_STREQ("192.168.2.2", pSipUri->GetHost());
    EXPECT_EQ(9090, pSipUri->GetPort());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ(pUri, &(aBuffer[0]));
    EXPECT_STREQ(pUri, objBuffer.GetCharString());

    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    EXPECT_EQ(
            SIP_TRUE, pSipAddrSpec->DecodeAddrSpec(const_cast<char*>("sips:192.168.2.8:9091"), 21));

    EXPECT_EQ(SipUri::SCHEME_SIPS, pSipAddrSpec->GetUriScheme());

    pSipUri = pSipAddrSpec->GetSipUri();
    EXPECT_STREQ("192.168.2.8", pSipUri->GetHost());
    EXPECT_EQ(9091, pSipUri->GetPort());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("sips:192.168.2.8:9091", &(aBuffer[0]));
    EXPECT_STREQ("sips:192.168.2.8:9091", objBuffer.GetCharString());

    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);
    pUri = const_cast<char*>("sip:alice@atlanta.com;maddr=239.255.255.1;ttl=15");
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec(pUri, strlen(pUri)));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("sip:alice@atlanta.com;maddr=239.255.255.1;ttl=15", &(aBuffer[0]));
    EXPECT_STREQ("sip:alice@atlanta.com;maddr=239.255.255.1;ttl=15", objBuffer.GetCharString());

    pSipAddrSpec->SipDelete();

    pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);
    pUri = const_cast<char*>("sip:AAuser:$=+,%3AB@host;method=REGISTER?to=proxy");
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec(pUri, strlen(pUri)));

    pSipUri = pSipAddrSpec->GetSipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    pSipUri->SetUser("%AAuser");
    pSipUri->SetPassword("$=+,%3AB");

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->EncodeAddrSpec(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipAddrSpec->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("sip:%AAuser:$=+,%3AB@host;method=REGISTER?to=proxy", &(aBuffer[0]));
    EXPECT_STREQ("sip:%AAuser:$=+,%3AB@host;method=REGISTER?to=proxy", objBuffer.GetCharString());
    pSipUri->SipDelete();
    pSipAddrSpec->SipDelete();
}

}  // namespace android
