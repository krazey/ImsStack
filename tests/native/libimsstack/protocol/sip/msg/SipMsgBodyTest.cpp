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

#include "msg/SipMsgBody.h"
#include "msg/SipUnknownHeader.h"

namespace android
{

class SipMsgBodyTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipMsgBodyTest, EncodeSingleMsgBody)
{
    SipMsgBody* pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_FALSE, pMessageBody->EncodeSingleMsgBody(&pBuff));

    EXPECT_EQ(SIP_FALSE, pMessageBody->SetMsgBuffer(nullptr, 0));

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer("This is single body", 19));

    EXPECT_EQ(SIP_TRUE, pMessageBody->EncodeSingleMsgBody(&pBuff));

    EXPECT_STREQ("This is single body", &(aBuffer[0]));

    pMessageBody->SipDelete();

    pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer("", 0));

    EXPECT_EQ(SIP_TRUE, pMessageBody->EncodeSingleMsgBody(&pBuff));

    EXPECT_STREQ("", &(aBuffer[0]));

    pMessageBody->SipDelete();
}

TEST_F(SipMsgBodyTest, DecodeSingleMsgBody)
{
    SipMsgBody* pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    char* pDecodeBody = SIP_NULL;

    EXPECT_EQ(SIP_FALSE, pMessageBody->GetMsgBuffer(&pDecodeBody));

    char* pSingleBody = (char*)"This is a single body,\r\n\
and no headers and boundary present\r\n";
    int nLen = strlen(pSingleBody);

    EXPECT_EQ(SIP_TRUE, pMessageBody->DecodeSingleMsgBody(pSingleBody, pSingleBody + nLen));
    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pDecodeBody));
    EXPECT_EQ(nLen, pMessageBody->GetBufferLength());

    EXPECT_STREQ(pSingleBody, pDecodeBody);

    delete[] pDecodeBody;
    pMessageBody->SipDelete();
}

TEST_F(SipMsgBodyTest, EncodeMIMEMsgBody)
{
    SipMsgBody* pEmptyMessageBody = new SipMsgBody();
    ASSERT_TRUE(pEmptyMessageBody != nullptr);

    /* Empty message body, fail */
    EXPECT_EQ(SIP_FALSE, pEmptyMessageBody->EncodeMIMEMsgBody(nullptr));

    pEmptyMessageBody->SipDelete();

    /* Level - 1 Start */
    SipMsgBody* pMessageBodyLevel1 = new SipMsgBody(SipMsgBody::MULTI_PART_MIXED_BODY);
    ASSERT_TRUE(pMessageBodyLevel1 != nullptr);

    /* Level - 1 Headers */
    SipContentTypeHeader* pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pContentTypeHeader->DecodeHdr((char*)"multipart/mixed;boundary=abcxz", 30));
    EXPECT_EQ(SIP_TRUE, pMessageBodyLevel1->SetMimeHdr(pContentTypeHeader));
    pContentTypeHeader->SipDelete();

    /* Level - 2 Start */
    SipMsgBody* pMessageBodyLevel2 = new SipMsgBody(SipMsgBody::MULTI_PART_MIXED_BODY);
    ASSERT_TRUE(pMessageBodyLevel2 != nullptr);

    /* Level - 2 Headers */
    pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);
    EXPECT_EQ(
            SIP_TRUE, pContentTypeHeader->DecodeHdr((char*)"application/test;boundary=12345", 31));
    EXPECT_EQ(SIP_TRUE, pMessageBodyLevel2->SetMimeHdr(pContentTypeHeader));
    pContentTypeHeader->SipDelete();

    /* Level - 2 Message body 1 */
    SipMsgBody* pMessageLevel2 = new SipMsgBody();
    ASSERT_TRUE(pMessageLevel2 != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessageLevel2->SetMsgBuffer((char*)"level2 - message body 1", 23));
    SipMsgBodyList* pMsgBobyList = pMessageBodyLevel2->GetMessageBodyList();
    ASSERT_TRUE(pMsgBobyList != nullptr);
    EXPECT_EQ(SIP_TRUE, pMsgBobyList->AddBody(pMessageLevel2));
    pMessageLevel2->SipDelete();

    /* Level - 2 Message body 2 */
    pMessageLevel2 = new SipMsgBody();
    ASSERT_TRUE(pMessageLevel2 != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessageLevel2->SetMsgBuffer((char*)"level2 - message body 2", 23));
    EXPECT_EQ(SIP_TRUE, pMsgBobyList->AddBody(pMessageLevel2));
    pMessageLevel2->SipDelete();
    pMsgBobyList->SipDelete();

    /* Set level - 2 message body to Level - 1 message body */
    pMsgBobyList = pMessageBodyLevel1->GetMessageBodyList();
    EXPECT_EQ(SIP_TRUE, pMsgBobyList->AddBody(pMessageBodyLevel2));
    /* Level - 2 End */

    /* Level - 1 Message body 1 */
    SipMsgBody* pMessageLevel1 = new SipMsgBody();
    ASSERT_TRUE(pMessageLevel1 != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessageLevel1->SetMsgBuffer((char*)"level1 - message body 1", 23));
    pMsgBobyList = pMessageBodyLevel1->GetMessageBodyList();
    ASSERT_TRUE(pMsgBobyList != nullptr);
    EXPECT_EQ(SIP_TRUE, pMsgBobyList->AddBody(pMessageLevel1));
    pMessageLevel1->SipDelete();

    /* Level - 1 Message body 2 */
    pMessageLevel1 = new SipMsgBody();
    ASSERT_TRUE(pMessageLevel1 != nullptr);

    SipUnknownHeader* pUnknownHeader = reinterpret_cast<SipUnknownHeader*>(
            SipUnknownHeader::GetNewObj(SipHeaderBase::UNKNOWN, nullptr));
    ASSERT_TRUE(pUnknownHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderName("UnknownHeaderName1"));
    EXPECT_EQ(SIP_TRUE, pUnknownHeader->SetHeaderValue("UnknownHeaderValue1"));
    EXPECT_EQ(SIP_TRUE, pMessageLevel1->SetMimeHdr(pUnknownHeader));
    pUnknownHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pMessageLevel1->SetMsgBuffer((char*)"level1 - message body 2", 23));
    pMsgBobyList = pMessageBodyLevel1->GetMessageBodyList();
    ASSERT_TRUE(pMsgBobyList != nullptr);
    EXPECT_EQ(SIP_TRUE, pMsgBobyList->AddBody(pMessageLevel1));
    pMessageLevel1->SipDelete();
    pMsgBobyList->SipDelete();

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pMessageBodyLevel1->EncodeMIMEMsgBody(&pBuff));

    char* pMessage = (char*)"--abcxz\r\n\
Content-Type: application/test;boundary=12345\r\n\
\r\n\
--12345\r\n\
\r\n\
level2 - message body 1\r\n\
--12345\r\n\
\r\n\
level2 - message body 2\r\n\
--12345--\r\n\
\r\n\
--abcxz\r\n\
\r\n\
level1 - message body 1\r\n\
--abcxz\r\n\
UnknownHeaderName1: UnknownHeaderValue1\r\n\
\r\n\
level1 - message body 2\r\n\
--abcxz--\r\n";

    EXPECT_STREQ(pMessage, &(aBuffer[0]));

    pMessageBodyLevel1->SipDelete();
}

TEST_F(SipMsgBodyTest, EncodeBody)
{
    SipMsgBody* pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    /* Empty message body, fail */
    EXPECT_EQ(SIP_FALSE, pMessageBody->EncodeBody(&pBuff));

    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMsgBuffer((char*)"single body", 11));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pMessageBody->EncodeBody(&pBuff));

    EXPECT_STREQ("single body", pMessageBody->GetBuffer());
    pMessageBody->SipDelete();

    pMessageBody = new SipMsgBody(SipMsgBody::MULTI_PART_MIXED_BODY);
    ASSERT_TRUE(pMessageBody != nullptr);

    SipContentTypeHeader* pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pContentTypeHeader->DecodeHdr((char*)"multipart/mixed;boundary=abcxz", 30));
    EXPECT_EQ(SIP_TRUE, pMessageBody->SetMimeHdr(pContentTypeHeader));
    pContentTypeHeader->SipDelete();

    SipMsgBody* pBody1 = new SipMsgBody();
    ASSERT_TRUE(pBody1 != nullptr);

    pContentTypeHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pContentTypeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pContentTypeHeader->DecodeHdr((char*)"application/sdp", 15));
    EXPECT_EQ(SIP_TRUE, pBody1->SetMimeHdr(pContentTypeHeader));
    pContentTypeHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pBody1->IsMessageBodySDP());

    EXPECT_EQ(SIP_TRUE, pBody1->SetMsgBuffer((char*)"sdp message body", 16));

    SipMsgBodyList* pMsgBobyList = pMessageBody->GetMessageBodyList();
    ASSERT_TRUE(pMsgBobyList != nullptr);
    EXPECT_EQ(SIP_TRUE, pMsgBobyList->AddBody(pBody1));
    pBody1->SipDelete();
    pMsgBobyList->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pMessageBody->EncodeBody(&pBuff));

    char* pMsg = (char*)"\r\nContent-Type: multipart/mixed;boundary=abcxz\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/sdp\r\n\
\r\n\
sdp message body\r\n\
--abcxz--\r\n";

    EXPECT_STREQ(pMsg, &(aBuffer[0]));

    pMessageBody->SipDelete();
}

TEST_F(SipMsgBodyTest, DecodeMIMEMsgBody)
{
    SipMsgBody* pMessageBody = new SipMsgBody();
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SIP_FALSE, pMessageBody->DecodeMIMEMsgBody(nullptr, nullptr));

    char* pMessage = (char*)"Content-Type: multipart/mixed;boundary=abcxz\r\n\
\r\n\
--abcxz\r\n\
Content-Type: multipart/mixed;boundary=12345\r\n\
\r\n\
--12345\r\n\
Content-Type: application/test\r\n\
\r\n\
level2 - message body 1\r\n\
--12345\r\n\
Content-Type: application/sdp\r\n\
\r\n\
level2 - message body 2\r\n\
--12345--\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/sdp\r\n\
Content-Encoding: en\r\n\
Content-Disposition: alert\r\n\
\r\n\
level1 - message body 1\r\n\
--abcxz\r\n\
Content-Type: application/sdp\r\n\
UnknownHeaderName1: UnknownHeaderValue1\r\n\
\r\n\
level1 - message body 2\r\n\
--abcxz--\r\n";
    int nLen = strlen(pMessage);

    EXPECT_EQ(SIP_TRUE, pMessageBody->DecodeMIMEMsgBody(pMessage, pMessage + nLen));

    SipContentTypeHeader* pContentTypeHeader = pMessageBody->GetContentType();

    ASSERT_TRUE(pContentTypeHeader != nullptr);

    EXPECT_STREQ("multipart", pContentTypeHeader->GetMediaType());
    EXPECT_STREQ("mixed", pContentTypeHeader->GetSubMediaType());

    char* pBoundary = pContentTypeHeader->GetBoundary();
    EXPECT_STREQ("abcxz", pBoundary);
    delete[] pBoundary;

    pContentTypeHeader->SipDelete();

    SipMsgBodyList* pMsgBobyList = pMessageBody->GetMessageBodyList();

    EXPECT_EQ(3, pMsgBobyList->GetMsgBodyCount());

    /* level - 2, boundary 12345, start */
    SipMsgBody* pBody1 = pMsgBobyList->GetBodyByIndex(0);
    ASSERT_TRUE(pBody1 != nullptr);

    pContentTypeHeader = pBody1->GetContentType();

    ASSERT_TRUE(pContentTypeHeader != nullptr);

    EXPECT_STREQ("multipart", pContentTypeHeader->GetMediaType());
    EXPECT_STREQ("mixed", pContentTypeHeader->GetSubMediaType());

    pBoundary = pContentTypeHeader->GetBoundary();
    EXPECT_STREQ("12345", pBoundary);
    delete[] pBoundary;

    pContentTypeHeader->SipDelete();

    SipMsgBodyList* pBobyList1 = pBody1->GetMessageBodyList();
    ASSERT_TRUE(pBobyList1 != nullptr);

    EXPECT_EQ(2, pBobyList1->GetMsgBodyCount());

    SipMsgBody* pInnerBody1 = pBobyList1->GetBodyByIndex(0);
    ASSERT_TRUE(pInnerBody1 != nullptr);

    EXPECT_STREQ("level2 - message body 1", pInnerBody1->GetBuffer());
    pInnerBody1->SipDelete();

    SipMsgBody* pInnerBody2 = pBobyList1->GetBodyByIndex(1);
    ASSERT_TRUE(pInnerBody2 != nullptr);

    EXPECT_STREQ("level2 - message body 2", pInnerBody2->GetBuffer());
    pInnerBody2->SipDelete();
    pBobyList1->SipDelete();
    pBody1->SipDelete();
    /* level - 2, boundary 12345, end */

    SipMsgBody* pBody2 = pMsgBobyList->GetBodyByIndex(1);
    ASSERT_TRUE(pBody2 != nullptr);

    SipMsgBodyList* pBobyList2 = pBody2->GetMessageBodyList();
    ASSERT_TRUE(pBobyList2 == nullptr);

    SipHeaderBase* pContentEncodingHeader = pBody2->GetContentEncoding();
    ASSERT_TRUE(pContentEncodingHeader != nullptr);
    EXPECT_STREQ("en", pContentEncodingHeader->GetValue());
    pContentEncodingHeader->SipDelete();

    SipHeaderBase* pContentDispositionHeader = pBody2->GetContentDisposition();
    ASSERT_TRUE(pContentDispositionHeader != nullptr);
    EXPECT_STREQ("alert", pContentDispositionHeader->GetValue());
    pContentDispositionHeader->SipDelete();

    EXPECT_STREQ("level1 - message body 1", pBody2->GetBuffer());
    pBody2->SipDelete();

    SipMsgBody* pBody3 = pMsgBobyList->GetBodyByIndex(2);
    ASSERT_TRUE(pBody3 != nullptr);

    SipMsgBodyList* pBobyList3 = pBody3->GetMessageBodyList();
    ASSERT_TRUE(pBobyList3 == nullptr);

    EXPECT_STREQ("level1 - message body 2", pBody3->GetBuffer());

    EXPECT_EQ(1, pBody3->GetUnknownHdrCount());

    SipUnknownHeader* pUnknownHdr =
            reinterpret_cast<SipUnknownHeader*>(pBody3->GetMimeHdr(SipHeaderBase::UNKNOWN, 0));

    EXPECT_STREQ("UnknownHeaderName1", pUnknownHdr->GetHeaderName());
    EXPECT_STREQ("UnknownHeaderValue1", pUnknownHdr->GetHeaderValue());

    pUnknownHdr->SipDelete();

    pBody3->SipDelete();

    pMessageBody->SipDelete();
}

TEST_F(SipMsgBodyTest, DecodeAndEncodeMessageSummaryMsgBody)
{
    SipMsgBody* pEmptyMessageBody = new SipMsgBody();
    ASSERT_TRUE(pEmptyMessageBody != nullptr);

    /* Empty message body, fail */
    EXPECT_EQ(SIP_FALSE, pEmptyMessageBody->EncodeMessageSummaryMsgBody(nullptr));

    pEmptyMessageBody->SipDelete();

    SipMsgBody* pMessageBody = new SipMsgBody(SipMsgBody::MESSAGE_SUMMARY_BODY);
    ASSERT_TRUE(pMessageBody != nullptr);

    char* pMessageSummary = (char*)"Messages-Waiting: No\r\n";
    int nLen = strlen(pMessageSummary);

    EXPECT_EQ(SIP_TRUE,
            pMessageBody->DecodeMessageSummaryMsgBody(pMessageSummary, pMessageSummary + nLen - 1));

    SipMsgBody* pCopyMessageBody = new SipMsgBody(*pMessageBody);
    ASSERT_TRUE(pCopyMessageBody != nullptr);

    pMessageBody->SipDelete();

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pCopyMessageBody->EncodeMessageSummaryMsgBody(&pBuff));

    EXPECT_STREQ("Messages-Waiting: No\r\n", &(aBuffer[0]));

    pCopyMessageBody->SipDelete();
}

}  // namespace android