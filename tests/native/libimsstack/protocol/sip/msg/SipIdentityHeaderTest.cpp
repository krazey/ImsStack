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
#include "msg/SipIdentityHeader.h"

namespace android
{

class SipIdentityHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipIdentityHeaderTest, Encode)
{
    SipIdentityHeader* pHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    /* Empty header, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetSignedIdentityDigest("signed.digest");

    /* Only signed identity digest present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetInfo("InfoValue");

    /* Only info present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetSignedIdentityDigest("signed.digest");

    /* Signed identity digest and info present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("signed.digest;info=<InfoValue>", objValue.GetCharString());
    EXPECT_STREQ("signed.digest;info=<InfoValue>", &(aBuffer[0]));

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->AddParam("alg", "ES256");

    /* Signed identity digest, info and parameters present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("signed.digest;info=<InfoValue>;alg=ES256", objValue.GetCharString());
    EXPECT_STREQ("signed.digest;info=<InfoValue>;alg=ES256", &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipIdentityHeaderTest, Decode)
{
    SipIdentityHeader* pHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty headers, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    /* Only signed digest, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("eyJhbGci.VyaSI6WyJ-zJ6F1VOg", 27));

    /* info not present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("eyJhbGci.VyaSI6WyJ-zJ6F1VOg;NoInfo=abcd", 39));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* signed digest, info present with value not enclosed in LA/RA Quotes, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("eyJhbGci.VyaSI6WyJ-zJ6F1VOg;info=abcd", 37));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* signed digest, info present and no parameters, success */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("eyJhbGci.VyaSI6WyJ-zJ6F1VOg;info=<abcd>", 39));
    EXPECT_STREQ("eyJhbGci.VyaSI6WyJ-zJ6F1VOg", pHeader->GetSignedIdentityDigest());
    EXPECT_STREQ("abcd", pHeader->GetInfo());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* signed digest, info present and parameters present, success */
    EXPECT_EQ(SIP_TRUE,
            pHeader->Decode("eyJhbGci.VyaSI6WyJ-zJ6F1VOg;info=<1234>;alg=ES256;ppt=shaken", 60));
    EXPECT_STREQ("eyJhbGci.VyaSI6WyJ-zJ6F1VOg", pHeader->GetSignedIdentityDigest());
    EXPECT_STREQ("1234", pHeader->GetInfo());

    EXPECT_EQ(2, pHeader->GetParamCount());

    SipIdentityHeader* pCopyHeader = reinterpret_cast<SipIdentityHeader*>(
            SipIdentityHeader::GetNewObj(SipHeaderBase::IDENTITY, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->IsValidHeader());

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objValue, SIP_TRUE));

    EXPECT_STREQ("eyJhbGci.VyaSI6WyJ-zJ6F1VOg;info=<1234>;alg=ES256;ppt=shaken",
            objValue.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android
