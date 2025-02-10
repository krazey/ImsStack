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
#include "msg/SipAuthBase.h"

namespace android
{

class SipAuthBaseTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipAuthBaseTest, IsValidHeader)
{
    SipAuthBase* pHeader = reinterpret_cast<SipAuthBase*>(
            SipAuthBase::GetNewObj(SipHeaderBase::AUTHORIZATION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    pHeader->SetValue("Digest");

    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    pHeader->SetParams("realm", "abcd.example.com", SIP_FALSE);

    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    pHeader->SipDelete();
}

TEST_F(SipAuthBaseTest, Encode)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    AStringBuffer objBuffer(64);

    SipAuthBase* pHeader = reinterpret_cast<SipAuthBase*>(
            SipAuthBase::GetNewObj(SipHeaderBase::AUTHORIZATION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetValue("Digest");

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetParams("realm", "abcd.example.com", SIP_FALSE);
    pHeader->SetParams("uri", "abc@xyz.com", SIP_TRUE);

    SipAuthBase* pCopyHeader = reinterpret_cast<SipAuthBase*>(
            SipAuthBase::GetNewObj(SipHeaderBase::AUTHORIZATION, pHeader));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("Digest", pCopyHeader->GetValue());
    EXPECT_STREQ("Digest", pHeader->GetValue());

    SIP_CHAR* pRealm = pCopyHeader->GetAuthValue("realm");
    ASSERT_TRUE(pRealm != nullptr);
    EXPECT_STREQ("abcd.example.com", pRealm);
    delete[] pRealm;

    SIP_CHAR* pUri = pCopyHeader->GetAuthValue("uri");
    ASSERT_TRUE(pUri != nullptr);
    EXPECT_STREQ("\"abc@xyz.com\"", pUri);
    delete[] pUri;

    pRealm = pHeader->GetAuthValue("realm");
    ASSERT_TRUE(pRealm != nullptr);
    EXPECT_STREQ("abcd.example.com", pRealm);
    delete[] pRealm;

    pUri = pHeader->GetAuthValue("uri");
    ASSERT_TRUE(pUri != nullptr);
    EXPECT_STREQ("\"abc@xyz.com\"", pUri);
    delete[] pUri;

    EXPECT_STREQ("Digest realm=abcd.example.com,uri=\"abc@xyz.com\"", objBuffer.GetCharString());
    EXPECT_STREQ("Digest realm=abcd.example.com,uri=\"abc@xyz.com\"", &(aBuffer[0]));

    pHeader->SipDelete();
    pCopyHeader->SipDelete();
}

TEST_F(SipAuthBaseTest, Decode)
{
    SipAuthBase* pHeader = reinterpret_cast<SipAuthBase*>(
            SipAuthBase::GetNewObj(SipHeaderBase::AUTHORIZATION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));
    EXPECT_EQ(nullptr, pHeader->GetValue());
    EXPECT_EQ(nullptr, pHeader->GetAuthValue(nullptr));
    EXPECT_EQ(nullptr, pHeader->GetAuthValue("atlanta"));

    EXPECT_EQ(SIP_TRUE,
            pHeader->Decode("Digest realm=\"atlanta.example.com\",\
qop=\"auth\",nonce=\"f84f1cec41e6cbe5aea9c8e88d359\",opaque=\"\", stale=FALSE, algorithm=MD5",
                    121));

    EXPECT_STREQ("Digest", pHeader->GetValue());

    SIP_CHAR* pValue = pHeader->GetAuthValue("qop");
    EXPECT_STREQ("\"auth\"", pValue);

    delete[] pValue;

    pValue = pHeader->GetAuthValue("opaque");
    EXPECT_STREQ("\"\"", pValue);

    delete[] pValue;

    pValue = pHeader->GetAuthValue("algorithm");
    EXPECT_STREQ("MD5", pValue);

    delete[] pValue;

    EXPECT_EQ(nullptr, pHeader->GetAuthValue("atlanta"));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipAuthBase*>(
            SipAuthBase::GetNewObj(SipHeaderBase::AUTHORIZATION, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("InValidHeaderValue", 18));

    pHeader->SipDelete();
}

}  // namespace android
