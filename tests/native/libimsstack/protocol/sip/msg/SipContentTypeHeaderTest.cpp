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

TEST_F(SipContentTypeHeaderTest, EncodeAndEncodeHdr)
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
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    /* Only mediaType present, fail */
    pHeader->SetMediaType("mediaType");
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* Only media subType present, fail */
    pHeader->SetSubMediaType("mediaSubType");
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* mediaType and media subType present, success */
    pHeader->SetSubMediaType("mediaSubType");
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

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value", &(aBuffer[0]));
    EXPECT_STREQ("mediaType/mediaSubType;param-name=param-value", objBuffer.GetCharString());

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipContentTypeHeaderTest, DecodeHdr)
{
    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header allowed for accept*/
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode only value and no parameter */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("application/sdp", 15));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    /* Decode only value and parameters, with double quote included for param
     * value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("application/sdp;boundary=\"unique-boundary-1\"", 44));
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
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("application/sdp;boundary=unique-boundary-2", 42));
    EXPECT_STREQ("application", pHeader->GetMediaType());
    EXPECT_STREQ("sdp", pHeader->GetSubMediaType());

    EXPECT_EQ(1, pHeader->GetParamCount());
    pBoundary = pHeader->GetBoundary();
    ASSERT_TRUE(pBoundary != nullptr);

    EXPECT_STREQ("unique-boundary-2", pBoundary);

    delete[] pBoundary;
    pHeader->SipDelete();
}

}  // namespace android
