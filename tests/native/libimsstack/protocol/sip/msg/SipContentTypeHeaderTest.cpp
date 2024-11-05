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

#include "msg/SipContentTypeHeader.h"

namespace android
{

class SipContentTypeHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipContentTypeHeaderTest, CopyConstructor)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetMediaType("mediaType");
    pHeader->SetSubMediaType("mediaSubType");

    SipContentTypeHeader* pCopyHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_STREQ("mediaType", pCopyHeader->GetMediaType());
    EXPECT_STREQ("mediaSubType", pCopyHeader->GetSubMediaType());

    pCopyHeader->SipDelete();
}

TEST_F(SipContentTypeHeaderTest, IsValidHeader)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, empty accept allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());

    /* Only mediaType present, fail */
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* Only media subType present, fail */
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* mediaType and media subType present, success */
    pHeader->SetMediaType("mediaType");
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipContentTypeHeaderTest, Encode)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    /* Only mediaType present, fail */
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* Only media subType present, fail */
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* mediaType and media subType present, success */
    pHeader->SetSubMediaType("mediaSubType");
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("mediaType/mediaSubType", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    /* Encode accept with value and parameters */
    pHeader->AddParam("param-name", "param-value");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value", objBuffer.GetCharString());

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipContentTypeHeaderTest, Decode)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header allowed for accept*/
    EXPECT_EQ(SIP_FALSE, pHeader->Decode(SIP_NULL, 0));

    /* Decode only value and no parameter */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("application/sdp", 15));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* Decode only value and parameters, with double quote included for param
     * value */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("application/sdp;boundary=\"unique-boundary-1\"", 44));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());

    EXPECT_EQ(1, pHeader->GetParamCount());
    SIP_CHAR* pBoundary = pHeader->GetBoundary();
    ASSERT_TRUE(pBoundary != nullptr);

    EXPECT_STREQ("unique-boundary-1", pBoundary);

    delete[] pBoundary;
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* Decode only value and parameters, without double quote included for param
     * value */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("application/sdp;boundary=unique-boundary-2", 42));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());

    EXPECT_EQ(1, pHeader->GetParamCount());
    pBoundary = pHeader->GetBoundary();
    ASSERT_TRUE(pBoundary != nullptr);

    EXPECT_STREQ("unique-boundary-2", pBoundary);

    delete[] pBoundary;
    pHeader->SipDelete();
}

TEST_F(SipContentTypeHeaderTest, AcceptHeader_CopyConstructor)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetMediaType("mediaType");
    pHeader->SetSubMediaType("mediaSubType");

    SipContentTypeHeader* pCopyHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_STREQ("mediaType", pCopyHeader->GetMediaType());
    EXPECT_STREQ("mediaSubType", pCopyHeader->GetSubMediaType());

    pCopyHeader->SipDelete();
}

TEST_F(SipContentTypeHeaderTest, AcceptHeader_IsValidHeader)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, empty accept allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    /* Only mediaType present, fail */
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Only media subType present, fail */
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present and are *'s, success */
    pHeader->SetMediaType("*");
    pHeader->SetSubMediaType("*");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present and are not *'s, success */
    pHeader->SetMediaType("mediaType");
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType is '*' and media subType is not '*', fail */
    pHeader->SetMediaType("*");
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType is not '*' and media subType is '*', success */
    pHeader->SetMediaType("mediaType");
    pHeader->SetSubMediaType("*");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipContentTypeHeaderTest, AcceptHeader_Encode)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* No MediaType, media subType and parameters, empty accept allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    /* Only mediaType present, fail */
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Only media subType present, fail */
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* mediaType and media subType present, success */
    pHeader->SetSubMediaType("mediaSubType");
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("mediaType/mediaSubType", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    /* Encode accept with value and parameters */
    pHeader->AddParam("param-name", "param-value");
    pHeader->AddParam("q", "0.1");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value;q=0.1", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value;q=0.1", objBuffer.GetCharString());

    objBuffer = AString::ConstNull();
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("mediaType/mediaSubType", objBuffer.GetCharString());

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipContentTypeHeaderTest, AcceptHeader_Decode)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header allowed for accept*/
    EXPECT_EQ(SIP_TRUE, pHeader->Decode(SIP_NULL, 0));

    /* Decode only value and no parameter */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("application/sdp", 15));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Decode only value and parameters */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("application/sdp;q=0.4;level=1", 29));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());

    EXPECT_EQ(2, pHeader->GetParamCount());

    SipNameValue* pNameVal = pHeader->GetParam(0);
    EXPECT_STREQ("q", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("0.4", pNameVal->m_objValueList.GetAt(0));

    pNameVal = pHeader->GetParam(1);
    EXPECT_STREQ("level", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("1", pNameVal->m_objValueList.GetAt(0));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::ACCEPT, nullptr));
    /* Decode invalid media range - MType is '*' and MSubType is not '*' */
    EXPECT_EQ(SIP_FALSE, pHeader->Decode("*/sdp", 5));
    pHeader->SipDelete();
}

}  // namespace android
