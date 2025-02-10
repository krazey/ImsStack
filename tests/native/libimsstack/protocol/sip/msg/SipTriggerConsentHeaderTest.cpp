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

#include "msg/SipTriggerConsentHeader.h"
#include "platform/SipString.h"

namespace android
{

class SipTriggerConsentHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipTriggerConsentHeaderTest, CopyConstructor)
{
    SipTriggerConsentHeader* pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipUri* pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    SipTriggerConsentHeader* pCopyHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipTriggerConsentHeaderTest, SetSipUri)
{
    SipTriggerConsentHeader* pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->SetSipUri(SIP_NULL));

    /* Create sipuri */
    SipUri* pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipTriggerConsentHeaderTest, GetSipUri)
{
    SipTriggerConsentHeader* pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* NULL SipUri */
    EXPECT_EQ(SIP_NULL, pHeader->GetSipUri());

    /* Set & Get SipUri value */
    SipUri* pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    pSipUri = pHeader->GetSipUri();
    ASSERT_TRUE(pSipUri != nullptr);
    pSipUri->SipDelete();
    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipTriggerConsentHeaderTest, EncodeHdr)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    SipTriggerConsentHeader* pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));

    /* Empty SipUri not allowed */
    SipUri* pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));

    pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    /* Valid value */
    const SIP_CHAR* pValue = "sip:123@example.com";

    pSipUri->Decode(pValue, SipPf_Strlen(pValue));

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipTriggerConsentHeaderTest, Encode)
{
    AStringBuffer objBuffer(512);

    SipTriggerConsentHeader* pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));

    /* Valid value */
    SipUri* pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));

    pSipUri = new SipUri();
    EXPECT_EQ(SIP_TRUE, pHeader->SetSipUri(pSipUri));
    pSipUri->SipDelete();

    /* Valid value */
    const SIP_CHAR* pValue = "sip:123@example.com";
    pSipUri->Decode(pValue, SipPf_Strlen(pValue));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ(pValue, objBuffer.GetCharString());
    pHeader->SipDelete();
}

TEST_F(SipTriggerConsentHeaderTest, Decode)
{
    SipTriggerConsentHeader* pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(SIP_NULL, 0));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));

    /* Invalid sipuri value*/
    EXPECT_EQ(SIP_FALSE,
            pHeader->Decode(
                    "urii:value@example.com:[789", SipPf_Strlen("urii:value@example.com:[789")));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    /* Decode invalid value */
    const SIP_CHAR* pValue = "sip:1111@example.com;=\"sip:friends@";
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(pValue, SipPf_Strlen(pValue)));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));

    /* Decode valid value */
    pValue = "sip:1111@example.com;target-uri=\"sip:friends@example.com\"";
    EXPECT_EQ(SIP_TRUE, pHeader->Decode(pValue, SipPf_Strlen(pValue)));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ(pValue + 4, &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipTriggerConsentHeader*>(
            SipTriggerConsentHeader::GetNewObj(SipHeaderBase::TRIGGER_CONSENT, nullptr));
    /* Decode valid value */
    pValue = "<sip:1111@example.com>";
    EXPECT_EQ(SIP_TRUE, pHeader->Decode(pValue, SipPf_Strlen(pValue)));
    SipUri* pSipUri = pHeader->GetSipUri();
    ASSERT_TRUE(pSipUri != nullptr);
    pSipUri->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("1111@example.com", &(aBuffer[0]));
    pHeader->SipDelete();
}

}  // namespace android
