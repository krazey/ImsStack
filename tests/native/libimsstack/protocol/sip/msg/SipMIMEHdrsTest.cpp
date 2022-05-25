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
#include "msg/SipUnknownHeader.h"
#include "msg/SipMsgBody.h"

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
    EXPECT_TRUE(pMimeHeaders->getMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE) == nullptr);
    EXPECT_TRUE(pMimeHeaders->getMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING) == nullptr);
    EXPECT_TRUE(pMimeHeaders->getMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION) == nullptr);
    EXPECT_TRUE(pMimeHeaders->getMimeHdrObj(SipMIMEHdrs::UNKNOWN) == nullptr);
    EXPECT_TRUE(pMimeHeaders->getMimeHdrObj(SipMIMEHdrs::INVALID) == nullptr);

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
            SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr));
    ASSERT_TRUE(pContentDospositionHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentDospositionHeader));
    pContentDospositionHeader->SipDelete();

    SipHeaderBase* pContentEncodingHeader = reinterpret_cast<SipHeaderBase*>(
            SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_ENCODING, nullptr));
    ASSERT_TRUE(pContentEncodingHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentEncodingHeader));
    pContentEncodingHeader->SipDelete();

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

    pHeader = pCopyMimeHeaders->getMimeHdrObj(SipMIMEHdrs::CONTENT_TYPE);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->getMimeHdrObj(SipMIMEHdrs::CONTENT_ENCODING);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->getMimeHdrObj(SipMIMEHdrs::CONTENT_DISPOSITION);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pHeader = pCopyMimeHeaders->getMimeHdrObj(SipMIMEHdrs::UNKNOWN);
    EXPECT_TRUE(pHeader != nullptr);
    pHeader->SipDelete();

    pCopyMimeHeaders->SipDelete();

    /* Get new mime header with getNewMIMEHdrObj */
    pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::CONTENT_TYPE);
    EXPECT_TRUE(pHeader != nullptr);

    /* Content type header already created, should not create new header */
    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::CONTENT_TYPE);
    EXPECT_TRUE(pHeader != nullptr);

    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::CONTENT_ENCODING);
    EXPECT_TRUE(pHeader != nullptr);

    /* Content encoding header already created, should not create new header */
    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::CONTENT_ENCODING);
    EXPECT_TRUE(pHeader != nullptr);

    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::CONTENT_DISPOSITION);
    EXPECT_TRUE(pHeader != nullptr);

    /* Content disposition header already created, should not create new header */
    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::CONTENT_DISPOSITION);
    EXPECT_TRUE(pHeader != nullptr);

    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::UNKNOWN);
    EXPECT_TRUE(pHeader != nullptr);

    /* Unknowd header already created, should return same header */
    pHeader = pMimeHeaders->getNewMIMEHdrObj(SipHeaderBase::UNKNOWN);
    EXPECT_TRUE(pHeader != nullptr);

    pMimeHeaders->SipDelete();
}

TEST_F(SipMIMEHdrsTest, EncodeMIMEHdrs)
{
    SipMIMEHdrs* pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->EncodeMIMEHdrs(&pBuff));

    EXPECT_STREQ("", &(aBuffer[0]));

    SipContentTypeHeader* pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pContentTypeHeader->SetMediaType("mediaType"));
    EXPECT_EQ(SIP_TRUE, pContentTypeHeader->SetSubMediaType("mediaSubType"));

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pContentTypeHeader));

    pContentTypeHeader->SipDelete();

    SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipUnknownHeader::GetNewObj(SipHeaderBase::UNKNOWN, nullptr));
    ASSERT_TRUE(pUnknownHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderName("UnknownHeaderName1"));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderValue("UnknownHeaderValue1"));

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pUnknownHeader));

    pUnknownHeader->SipDelete();

    pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipUnknownHeader::GetNewObj(SipHeaderBase::UNKNOWN, nullptr));
    ASSERT_TRUE(pUnknownHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderName("UnknownHeaderName2"));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderValue("UnknownHeaderValue2"));

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->SetMimeHdrs(pUnknownHeader));

    pUnknownHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->EncodeMIMEHdrs(&pBuff));

    char* pMimeHdrs = (char*)"Content-Type: mediaType/mediaSubType\r\n\
UnknownHeaderName1: UnknownHeaderValue1\r\nUnknownHeaderName2: UnknownHeaderValue2\r\n";

    EXPECT_STREQ(pMimeHdrs, &(aBuffer[0]));

    pMimeHeaders->SipDelete();
}

TEST_F(SipMIMEHdrsTest, DecodeMIMEHdrs)
{
    SipMIMEHdrs* pMimeHeaders = new SipMIMEHdrs();
    ASSERT_TRUE(pMimeHeaders != nullptr);

    EXPECT_EQ(SIP_FALSE, pMimeHeaders->DecodeMIMEHdrs((char*)"", 0));

    char* pMimeHeader = (char*)"Content-Type: mediaType/mediaSubType";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->DecodeMIMEHdrs(pMimeHeader, strlen(pMimeHeader)));

    pMimeHeader = (char*)"UnknownHeaderName1: UnknownHeaderValue1";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->DecodeMIMEHdrs(pMimeHeader, strlen(pMimeHeader)));

    pMimeHeader = (char*)"UnknownHeaderName2: UnknownHeaderValue2";
    EXPECT_EQ(SIP_TRUE, pMimeHeaders->DecodeMIMEHdrs(pMimeHeader, strlen(pMimeHeader)));

    char* pMimeHdrs = (char*)"Content-Type: mediaType/mediaSubType\r\n\
UnknownHeaderName1: UnknownHeaderValue1\r\nUnknownHeaderName2: UnknownHeaderValue2\r\n";

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pMimeHeaders->EncodeMIMEHdrs(&pBuff));
    EXPECT_STREQ(pMimeHdrs, &(aBuffer[0]));

    pMimeHeaders->SipDelete();
}

}  // namespace android