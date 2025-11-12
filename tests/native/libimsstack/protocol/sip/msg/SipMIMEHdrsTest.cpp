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

#include "msg/SipAuthBase.h"
#include "msg/SipMsgBody.h"
#include "msg/SipUnknownHeader.h"
#include "platform/SipString.h"

namespace android
{

class SipMIMEHdrsTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipMIMEHdrsTest, SetAndGetHeaders)
{
    SipMIMEHdrs* pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    EXPECT_EQ(0, pMimeHeaders->GetUnknownHdrCount());

    /* All mime headers nnull in new empty object */
    EXPECT_TRUE(pMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE) == nullptr);
    EXPECT_TRUE(pMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING) == nullptr);
    EXPECT_TRUE(pMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION) == nullptr);
    EXPECT_TRUE(pMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::UNKNOWN) == nullptr);
    EXPECT_TRUE(pMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::INVALID) == nullptr);

    EXPECT_EQ(SIP_FALSE, pMimeHeaders->SetMimeHdrs(nullptr));

    /* Setting other than mime header, fail */
    SipAuthBase* pAuthHeader = reinterpret_cast<SipAuthBase*>(
            SipAuthBase::GetNewObj(SipHeaderBase::AUTHORIZATION, nullptr));
    ASSERT_TRUE(pAuthHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pMimeHeaders->SetMimeHdrs(pAuthHeader));
    pAuthHeader->SipDelete();

    /* Set all mime headers with SetMimeHdrs and get all headers, success */
    SipContentTypeHeader* pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentTypeHeader));
    pContentTypeHeader->SipDelete();

    SipHeaderBase* pContentDospositionHeader = reinterpret_cast<SipHeaderBase*>(
            SipHeaderBase::CreateGenericHeader(SipHeaderBase::CONTENT_DISPOSITION, nullptr));
    ASSERT_TRUE(pContentDospositionHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentDospositionHeader));
    pContentDospositionHeader->SipDelete();

    SipHeaderBase* pContentEncodingHeader = reinterpret_cast<SipHeaderBase*>(
            SipHeaderBase::CreateGenericHeader(SipHeaderBase::CONTENT_ENCODING, nullptr));
    ASSERT_TRUE(pContentEncodingHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentEncodingHeader));
    pContentEncodingHeader->SipDelete();

    ASSERT_TRUE(nullptr == pMimeHeaders->GetUnknownHdr(0));

    SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipUnknownHeader::GetNewObj(SipHeaderBase::UNKNOWN, nullptr));
    ASSERT_TRUE(pUnknownHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pUnknownHeader));
    pUnknownHeader->SipDelete();

    SipMIMEHdrs* pCopyMimeHeaders = new SipMIMEHdrs(*pMimeHeaders);
    ASSERT_TRUE(pCopyMimeHeaders != nullptr);

    pMimeHeaders->SipDelete();

    SipHeaderBase* pHeader = pCopyMimeHeaders->GetUnknownHdr(0);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->GetUnknownHdr(1);
    EXPECT_TRUE(pHeader == nullptr);

    pHeader = pCopyMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->GetMimeHdrObj(SipMIMEHdrs::UNKNOWN);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pCopyMimeHeaders->SipDelete();

    /* Get new mime header with GetNewMIMEHdrObj */
    pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::CONTENT_TYPE);
    EXPECT_TRUE(pHeader != nullptr);

    /* Content type header already created, should not create new header */
    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::CONTENT_TYPE);
    EXPECT_TRUE(pHeader != nullptr);

    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::CONTENT_ENCODING);
    EXPECT_TRUE(pHeader != nullptr);

    /* Content encoding header already created, should not create new header */
    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::CONTENT_ENCODING);
    EXPECT_TRUE(pHeader != nullptr);

    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::CONTENT_DISPOSITION);
    EXPECT_TRUE(pHeader != nullptr);

    /* Content disposition header already created, should not create new header */
    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::CONTENT_DISPOSITION);
    EXPECT_TRUE(pHeader != nullptr);

    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::UNKNOWN);
    EXPECT_TRUE(pHeader != nullptr);

    /* Unknowd header already created, should return same header */
    pHeader = pMimeHeaders->GetNewMIMEHdrObj(SipHeaderBase::UNKNOWN);
    EXPECT_TRUE(pHeader != nullptr);

    pMimeHeaders->SipDelete();
}

TEST_F(SipMIMEHdrsTest, Encode)
{
    SipMIMEHdrs* pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Encode(&pBuff));

    EXPECT_STREQ("", &(aBuffer[0]));

    SipContentTypeHeader* pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);

    pContentTypeHeader->SetMediaType("mediaType");
    pContentTypeHeader->SetSubMediaType("mediaSubType");

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentTypeHeader));

    pContentTypeHeader->SipDelete();

    SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipUnknownHeader::GetNewObj(SipHeaderBase::UNKNOWN, nullptr));
    ASSERT_TRUE(pUnknownHeader != nullptr);

    pUnknownHeader->SetHeaderName("UnknownHeaderName1");
    pUnknownHeader->SetHeaderValue("UnknownHeaderValue1");

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pUnknownHeader));

    pUnknownHeader->SipDelete();

    pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipUnknownHeader::GetNewObj(SipHeaderBase::UNKNOWN, nullptr));
    ASSERT_TRUE(pUnknownHeader != nullptr);

    pUnknownHeader->SetHeaderName("UnknownHeaderName2");
    pUnknownHeader->SetHeaderValue("UnknownHeaderValue2");

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pUnknownHeader));

    pUnknownHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Encode(&pBuff));

    const SIP_CHAR* pMimeHdrs = "Content-Type: mediaType/mediaSubType\r\n\
UnknownHeaderName1: UnknownHeaderValue1\r\nUnknownHeaderName2: UnknownHeaderValue2\r\n";

    EXPECT_STREQ(pMimeHdrs, &(aBuffer[0]));

    pMimeHeaders->SipDelete();
}

TEST_F(SipMIMEHdrsTest, Decode)
{
    SipMIMEHdrs* pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    EXPECT_EQ(SIP_FALSE, pMimeHeaders->Decode("", 0));

    const SIP_CHAR* pMimeHeader = "Content-Length: 33";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Decode(pMimeHeader, SipPf_Strlen(pMimeHeader)));

    EXPECT_EQ(SIP_FALSE, pMimeHeaders->Decode("c: text", 7));

    pMimeHeader = "Content-Language: fr";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Decode(pMimeHeader, SipPf_Strlen(pMimeHeader)));
    pMimeHeaders->SipDelete();

    pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    pMimeHeader = "Content-Transfer-Encoding: base64";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Decode(pMimeHeader, SipPf_Strlen(pMimeHeader)));

    pMimeHeaders->SipDelete();

    pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    pMimeHeader = "Content-Type: mediaType/mediaSubType";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Decode(pMimeHeader, SipPf_Strlen(pMimeHeader)));

    pMimeHeader = "UnknownHeaderName1: UnknownHeaderValue1";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Decode(pMimeHeader, SipPf_Strlen(pMimeHeader)));

    pMimeHeader = "UnknownHeaderName2: UnknownHeaderValue2";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Decode(pMimeHeader, SipPf_Strlen(pMimeHeader)));

    const SIP_CHAR* pMimeHdrs = "Content-Type: mediaType/mediaSubType\r\n\
UnknownHeaderName1: UnknownHeaderValue1\r\nUnknownHeaderName2: UnknownHeaderValue2\r\n";

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->Encode(&pBuff));
    EXPECT_STREQ(pMimeHdrs, &(aBuffer[0]));

    pMimeHeaders->SipDelete();

    /* Invalid header with no colon, fail */
    pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    EXPECT_EQ(SIP_FALSE, pMimeHeaders->Decode("Content-Type", 12));
    pMimeHeaders->SipDelete();
}

}  // namespace android