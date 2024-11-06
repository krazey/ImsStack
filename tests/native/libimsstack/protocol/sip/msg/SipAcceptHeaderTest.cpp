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

    pHeader->SetMediaType("mediaType");
    pHeader->SetMediaSubType("mediaSubType");

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
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Only media subType present, fail */
    pHeader->SetMediaSubType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present and are *'s, success */
    pHeader->SetMediaType("*");
    pHeader->SetMediaSubType("*");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present and are not *'s, success */
    pHeader->SetMediaType("mediaType");
    pHeader->SetMediaSubType("mediaSubType");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType is '*' and media subType is not '*', fail */
    pHeader->SetMediaType("*");
    pHeader->SetMediaSubType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType is not '*' and media subType is '*', success */
    pHeader->SetMediaType("mediaType");
    pHeader->SetMediaSubType("*");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipAcceptHeaderTest, EncodeAndEncodeHdr)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    SipAcceptHeader* pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, empty accept allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    /* Only mediaType present, fail */
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Only media subType present, fail */
    pHeader->SetMediaSubType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present, success */
    pHeader->SetMediaSubType("mediaSubType");
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("mediaType/mediaSubType", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    /* Encode accept with value and parameters */
    pHeader->AddParam("param-name", "param-value");
    pHeader->AddParam("q", "0.1");

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value;q=0.1", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value;q=0.1", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("mediaType/mediaSubType", objBuffer.GetCharString());

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
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("application/sdp", 15));
    EXPECT_STREQ("application", pHeader->GetMType());
    EXPECT_STREQ("sdp", pHeader->GetMSubType());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Decode only value and parameters */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("application/sdp;q=0.4;level=1", 29));
    EXPECT_STREQ("application", pHeader->GetMType());
    EXPECT_STREQ("sdp", pHeader->GetMSubType());

    EXPECT_EQ(2, pHeader->GetParamCount());

    SipNameValue* pNameVal = pHeader->GetParam(0);
    EXPECT_STREQ("q", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("0.4", pNameVal->m_valueList.GetAt(0));

    pNameVal = pHeader->GetParam(1);
    EXPECT_STREQ("level", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("1", pNameVal->m_valueList.GetAt(0));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipAcceptHeader*>(
            SipAcceptHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Decode invalid media range - MType is '*' and MSubType is not '*' */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("*/sdp", 5));
    pHeader->SipDelete();
}

}  // namespace android
