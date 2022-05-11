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
#include "msg/SipAcceptHeader.h"

namespace android
{

class SipAcceptHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipAcceptHeaderTest, CopyConstructor)
{
    SipAcceptHeader* pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("mediaSubType"));

    SipAcceptHeader* pCopyHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_STREQ("mediaType", pCopyHeader->GetMType());
    EXPECT_STREQ("mediaSubType", pCopyHeader->GetMSubType());

    pCopyHeader->SipDelete();
}

TEST_F(SipAcceptHeaderTest, IsValidHeader)
{
    SipAcceptHeader* pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, empty accept allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    /* Only mediaType present, fail */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Only media subType present, fail */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("mediaSubType"));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present and are *'s, success */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("*"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("*"));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present and are not *'s, success */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("mediaSubType"));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType is '*' and media subType is not '*', fail */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("*"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("mediaSubType"));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType is not '*' and media subType is '*', success */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("*"));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipAcceptHeaderTest, EncodeHdr)
{
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    SipAcceptHeader* pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, empty accept allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));

    /* Only mediaType present, fail */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Only media subType present, fail */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("mediaSubType"));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaSubType("mediaSubType"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("mediaType/mediaSubType", &(aBuffer[0]));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Encode accept with value and parameters */
    pHeader->InitParameters(SIP_NULL);
    SipParameters* pParameters = pHeader->GetParameters();
    pParameters->AddParam("param-name", "param-value");
    pParameters->AddParam("q", "0.1");

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value;q=0.1", &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipAcceptHeaderTest, DecodeHdr)
{
    SipAcceptHeader* pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header allowed for accept*/
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode only value and no parameter */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"application/sdp", 15));
    EXPECT_STREQ("application", pHeader->GetMType());
    EXPECT_STREQ("sdp", pHeader->GetMSubType());
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Decode only value and parameters */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"application/sdp;q=0.4;level=1", 29));
    EXPECT_STREQ("application", pHeader->GetMType());
    EXPECT_STREQ("sdp", pHeader->GetMSubType());

    SipParameters* pParameters = pHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList* pSipParameterList = pParameters->GetParameterList();
    ASSERT_TRUE(pSipParameterList != nullptr);

    EXPECT_EQ(2, pSipParameterList->GetCount());

    SipNameValue* pNameVal = pSipParameterList->GetNameValNode(0);
    EXPECT_STREQ("q", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("0.4", pNameVal->m_valueList.GetAt(0));

    pNameVal = pSipParameterList->GetNameValNode(1);
    EXPECT_STREQ("level", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("1", pNameVal->m_valueList.GetAt(0));

    pHeader->SipDelete();
    pHeader = SIP_NULL;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Decode invalid media range - MType is '*' and MSubType is not '*' */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)"*/sdp", 5));
    pHeader->SipDelete();
    pHeader = SIP_NULL;
}

}  // namespace android
