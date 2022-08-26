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
#include "sip_abnfUtil.h"
#include "msg/SipMessage.h"
#include "msg/sip_msgutil.h"

namespace android
{

class SipMessageTest : public ::testing::Test
{
public:
    SipMessage* pMessage;
    SipHeaderBase* pViaHeader;
    SipHeaderBase* pCallIdHeader;
    SipHeaderBase* pCSeqHeader;
    SipHeaderBase* pFromHeader;
    SipHeaderBase* pToHeader;

protected:
    virtual void SetUp() override
    {
        pMessage = new SipMessage();
        ASSERT_TRUE(pMessage != nullptr);
    }

    virtual void TearDown() override { pMessage->SipDelete(); }

    void FillRequestLine()
    {
        SipAddrSpec* pSipAddrSpec = new SipAddrSpec();
        ASSERT_TRUE(pSipAddrSpec != nullptr);

        EXPECT_EQ(SIP_TRUE, pSipAddrSpec->DecodeAddrSpec((char*)"sip:user@host", 13));

        SipRequestLine* pRequestLine =
                new SipRequestLine((char*)"INVITE", pSipAddrSpec, SIP_SIPVER);
        ASSERT_TRUE(pRequestLine != nullptr);

        pMessage->SetRequestline(pRequestLine);
    }

    void FillStatusLine()
    {
        SipStatusLine* pStatusLine = new SipStatusLine(SIP_SIPVER, "180", "Ringing");
        ASSERT_TRUE(pStatusLine != nullptr);

        EXPECT_EQ(SIP_TRUE, pMessage->SetStatusLine(pStatusLine));
    }

    void FillMandatoryHeaders()
    {
        pViaHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
        ASSERT_TRUE(pViaHeader != nullptr);

        pCallIdHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CALL_ID);
        ASSERT_TRUE(pCallIdHeader != nullptr);

        pCSeqHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CSEQ);
        ASSERT_TRUE(pCSeqHeader != nullptr);

        pFromHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::FROM);
        ASSERT_TRUE(pFromHeader != nullptr);

        pToHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TO);
        ASSERT_TRUE(pToHeader != nullptr);

        char* via = (char*)"SIP/2.0/TCP "
                           "[2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;branch="
                           "z9hG4bK1422bd448-755bfe94";
        char* callid = (char*)"13217132a-3c0d31f9@2409:4031:241d:5ff5:b54d:c29a:ecea:88b8";
        char* cseq = (char*)"3 INVITE";
        char* from = (char*)"<sip:405861079851317@ims.mnc861.mcc405.3gppnetwork.org>;tag="
                            "544671422bd42c-2899e679";
        char* to = (char*)"<sip:user@host>";

        EXPECT_EQ(SIP_TRUE, pViaHeader->DecodeHdr(via, strlen(via)));
        EXPECT_EQ(SIP_TRUE, pCallIdHeader->DecodeHdr(callid, strlen(callid)));
        EXPECT_EQ(SIP_TRUE, pCSeqHeader->DecodeHdr(cseq, strlen(cseq)));
        EXPECT_EQ(SIP_TRUE, pFromHeader->DecodeHdr(from, strlen(from)));
        EXPECT_EQ(SIP_TRUE, pToHeader->DecodeHdr(to, strlen(to)));

        pMessage->SetHeader(pViaHeader);
        pMessage->SetHeader(pCallIdHeader);
        pMessage->SetHeader(pCSeqHeader);
        pMessage->SetHeader(pFromHeader);
        pMessage->SetHeader(pToHeader);

        pViaHeader->SipDelete();
        pCallIdHeader->SipDelete();
        pCSeqHeader->SipDelete();
        pFromHeader->SipDelete();
        pToHeader->SipDelete();
    }
};

TEST_F(SipMessageTest, SetRequestline)
{
    FillRequestLine();

    SipRequestLine* pRequestLine = pMessage->GetReqLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_STREQ("INVITE", pRequestLine->GetMethod());

    SipAddrSpec* pSipAddrSpec = pRequestLine->GetReqUri();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    pSipAddrSpec->SipDelete();
    pRequestLine->SipDelete();

    pRequestLine = new SipRequestLine((char*)"TEST-METHOD", nullptr, SIP_SIPVER);
    ASSERT_TRUE(pRequestLine != nullptr);

    pMessage->SetRequestline(pRequestLine);

    pRequestLine = nullptr;

    pRequestLine = pMessage->GetReqLine();
    ASSERT_TRUE(pRequestLine != nullptr);

    EXPECT_STREQ("TEST-METHOD", pRequestLine->GetMethod());
    EXPECT_TRUE(pRequestLine->GetReqUri() == nullptr);

    pRequestLine->SipDelete();
}

TEST_F(SipMessageTest, SetHeader)
{
    EXPECT_EQ(SIP_FALSE, pMessage->SetHeader(nullptr));

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    /* Empty Via, fail */
    EXPECT_EQ(SIP_FALSE, pMessage->SetHeader(pViaHdr));

    char* pViaValue = (char*)"SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
                             branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pMessage->SetHeader(pViaHdr));

    ASSERT_TRUE(pMessage->GetMsgHdrs() != nullptr);

    pViaHdr->SipDelete();

    EXPECT_TRUE(pMessage->GetHdrObj(SipHeaderBase::CALL_ID) == nullptr);

    SipHeaderBase* pHdr = pMessage->GetHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pHdr != nullptr);
    pHdr->SipDelete();

    SipUnknownHeader* pUnknownHdr = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHdr != nullptr);

    pUnknownHdr->SetHeaderName("UnknownHeaderName");
    pUnknownHdr->SetHeaderValue("UnknownHeaderValue");

    EXPECT_EQ(SIP_TRUE, pMessage->SetHeader(pUnknownHdr));

    pUnknownHdr->SipDelete();

    EXPECT_TRUE(pMessage->GetUnknownHdrObj("abc") == nullptr);

    pUnknownHdr = pMessage->GetUnknownHdrObj("UnknownHeaderName");
    ASSERT_TRUE(pUnknownHdr != nullptr);

    pUnknownHdr->SipDelete();
}

TEST_F(SipMessageTest, SetMessageBody)
{
    EXPECT_EQ(0, pMessage->GetMsgBodyCount());

    SipMsgBody* pSipMsgBody = new SipMsgBody();
    EXPECT_EQ(SIP_TRUE, pMessage->SetMessageBody(pSipMsgBody));

    pSipMsgBody->SipDelete();

    EXPECT_EQ(1, pMessage->GetMsgBodyCount());
    ASSERT_TRUE(pMessage->GetMsgBodyList() != nullptr);
}

TEST_F(SipMessageTest, SetStatusLine)
{
    SipStatusLine* pStatusLine = new SipStatusLine(SIP_SIPVER, "180", "Ringing");
    ASSERT_TRUE(pStatusLine != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessage->SetStatusLine(pStatusLine));

    pStatusLine = new SipStatusLine(SIP_SIPVER, "100", "Trying");
    ASSERT_TRUE(pStatusLine != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessage->SetStatusLine(pStatusLine));

    pStatusLine = pMessage->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);

    EXPECT_EQ(100, pStatusLine->GetStatusCodeAsInt());
    EXPECT_STREQ("SIP/2.0", pStatusLine->GetSipVersion());
    EXPECT_STREQ("Trying", pStatusLine->GetRsnPhrase());

    pStatusLine->SipDelete();
}

TEST_F(SipMessageTest, AppendHeader)
{
    EXPECT_EQ(SIP_FALSE, pMessage->AppendHeader(nullptr));

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    char* pViaValue = (char*)"SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
                             branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pMessage->AppendHeader(pViaHdr));

    pViaHdr->SipDelete();

    SipHeaderBase* pHdr = pMessage->GetHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pHdr != nullptr);
    pHdr->SipDelete();
}

TEST_F(SipMessageTest, InsertHeader)
{
    EXPECT_EQ(SIP_FALSE, pMessage->InsertHeader(nullptr, 0));

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    char* pViaValue = (char*)"SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
                             branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pMessage->InsertHeader(pViaHdr, 0));

    pViaHdr->SipDelete();

    SipHeaderBase* pHdr = pMessage->GetHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pHdr != nullptr);
    pHdr->SipDelete();
}

TEST_F(SipMessageTest, GetMethodType)
{
    EXPECT_EQ(SipMessage::METHOD_INVALID, pMessage->GetMethodType());

    FillStatusLine();

    EXPECT_EQ(SipMessage::METHOD_INVALID, pMessage->GetMethodType());

    pCSeqHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CSEQ);
    ASSERT_TRUE(pCSeqHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pCSeqHeader->DecodeHdr((char*)"3 INVALIDMETHOD", 10));

    pMessage->SetHeader(pCSeqHeader);
    pCSeqHeader->SipDelete();

    EXPECT_EQ(SipMessage::METHOD_UNKNOWN, pMessage->GetMethodType());

    pCSeqHeader = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CSEQ);
    ASSERT_TRUE(pCSeqHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pCSeqHeader->DecodeHdr((char*)"3 REGISTER", 10));

    pMessage->SetHeader(pCSeqHeader);
    pCSeqHeader->SipDelete();

    EXPECT_EQ(SipMessage::METHOD_REGISTER, pMessage->GetMethodType());

    pMessage->SetMessageType(SipMessage::REQ_TYPE);

    EXPECT_EQ(SipMessage::METHOD_INVALID, pMessage->GetMethodType());

    FillRequestLine();

    EXPECT_EQ(SipMessage::METHOD_INVITE, pMessage->GetMethodType());
}

TEST_F(SipMessageTest, HasHeader)
{
    FillMandatoryHeaders();

    EXPECT_EQ(SIP_TRUE, pMessage->HasHeader(SipHeaderBase::CSEQ));
    EXPECT_EQ(SIP_FALSE, pMessage->HasHeader(SipHeaderBase::SUPPORTED));
}

TEST_F(SipMessageTest, IsReqLineExists)
{
    EXPECT_EQ(SIP_FALSE, pMessage->IsReqLineExists());

    FillRequestLine();

    EXPECT_EQ(SIP_TRUE, pMessage->IsReqLineExists());
}

TEST_F(SipMessageTest, IsStatusLineExists)
{
    EXPECT_EQ(SIP_FALSE, pMessage->IsStatusLineExists());

    FillStatusLine();

    EXPECT_EQ(SIP_TRUE, pMessage->IsStatusLineExists());
}

TEST_F(SipMessageTest, HasMIMEMessageBody)
{
    EXPECT_EQ(SIP_FALSE, pMessage->HasMIMEMessageBody());

    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"multipart/mixed;boundary=\"boundary1\"", 36));
    pMessage->SetHeader(pHeader);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pMessage->HasMIMEMessageBody());

    EXPECT_EQ(SIP_TRUE, pMessage->RemoveHdr(SipHeaderBase::CONTENT_TYPE));

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"application/sdp", 15));
    pMessage->SetHeader(pHeader);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_FALSE, pMessage->HasMIMEMessageBody());
}

TEST_F(SipMessageTest, HasSDPMessageBody)
{
    EXPECT_EQ(SIP_FALSE, pMessage->HasSDPMessageBody());

    SipContentTypeHeader* pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"multipart/mixed;boundary=\"boundary1\"", 36));
    pMessage->SetHeader(pHeader);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_FALSE, pMessage->HasSDPMessageBody());

    EXPECT_EQ(SIP_TRUE, pMessage->RemoveHdr(SipHeaderBase::CONTENT_TYPE));

    pHeader = reinterpret_cast<SipContentTypeHeader*>(
            SipContentTypeHeader::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"application/sdp", 15));
    pMessage->SetHeader(pHeader);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pMessage->HasSDPMessageBody());
}

TEST_F(SipMessageTest, SetHdrList_GetHdrList)
{
    EXPECT_TRUE(pMessage->GetHdrList(SipHeaderBase::VIA) == nullptr);

    FillMandatoryHeaders();

    SipHeaderList* pHeaderList = pMessage->GetHdrList(SipHeaderBase::VIA);
    ASSERT_TRUE(pHeaderList != nullptr);

    EXPECT_EQ(1, pHeaderList->GetSize());

    pHeaderList->SipDelete();

    pHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::VIA, nullptr));

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    char* pViaValue = (char*)"SIP/2.0/TLS [2001::4]:8090;\
                             branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, strlen(pViaValue)));

    EXPECT_EQ(SIP_TRUE, pHeaderList->AddHeader(pViaHdr));

    pViaHdr->SipDelete();

    EXPECT_EQ(SIP_TRUE, pMessage->SetHdrList(pHeaderList));

    pHeaderList->SipDelete();

    pHeaderList = pMessage->GetHdrList(SipHeaderBase::VIA);
    ASSERT_TRUE(pHeaderList != nullptr);

    EXPECT_EQ(2, pHeaderList->GetSize());

    pHeaderList->SipDelete();
}

TEST_F(SipMessageTest, DecMultiPartBody)
{
    EXPECT_EQ(SIP_FALSE, pMessage->DecMultiPartBody(nullptr, nullptr, 0));

    FillMandatoryHeaders();

    char* pContentTypeValue = (char*)"multipart/mixed;boundary=\"abcxz\"";

    SipContentTypeHeader* pContentTypeHdr = new SipContentTypeHeader();
    ASSERT_TRUE(pContentTypeHdr != nullptr);

    EXPECT_EQ(SIP_TRUE, pContentTypeHdr->DecodeHdr(pContentTypeValue, strlen(pContentTypeValue)));

    pMessage->SetHeader(pContentTypeHdr);

    pContentTypeHdr->SipDelete();

    char* pMimeBody = (char*)"--abcxz\r\n\
Content-Type: application/body1\r\n\
\r\n\
Test message body 1\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body2\r\n\
\r\n\
Test message body 2\r\n\
\r\n\
--abcxz--\r\n";

    unsigned int nLen = strlen(pMimeBody);

    EXPECT_EQ(SIP_TRUE, pMessage->DecMultiPartBody(pMimeBody, pMimeBody + nLen, nLen));
    EXPECT_EQ(2, pMessage->GetMsgBodyCount());

    pMessage->RemoveAllMessageBodies();
    EXPECT_EQ(0, pMessage->GetMsgBodyCount());
}

TEST_F(SipMessageTest, EncodeMsgAndDecCompleteMessage)
{
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);
    SIP_UINT32 sipMsgLength = 0;

    /* No request line or status line, fail */
    EXPECT_EQ(SIP_FALSE, pMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    FillRequestLine();

    /* Invalid message type, fail */
    EXPECT_EQ(SIP_FALSE, pMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    pMessage->SetMessageType(SipMessage::REQ_TYPE);

    /* No headers, fail */
    EXPECT_EQ(SIP_FALSE, pMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    EXPECT_EQ(SIP_FALSE, pMessage->AppendMessageBody(nullptr));

    FillMandatoryHeaders();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Valid msg type and request line, mandatory headers included. success */
    EXPECT_EQ(SIP_TRUE, pMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    SipMessage* pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    /* Empty buffer, fail */
    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(nullptr, 0));

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pBuff, sipMsgLength));

    pDecodeMessage->SipDelete();

    /* With single message body and mandatory headers, success */
    char* pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: application/simple-message-example\r\n\
Content-Length: 23\r\n\
\r\n\
Messages-Waiting: yes\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->HasMandatoryHdrs());

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->HasMandatoryHdrs());

    SipMessage* pCopyMessage = new SipMessage(*pDecodeMessage);
    ASSERT_TRUE(pCopyMessage != nullptr);

    pDecodeMessage->SipDelete();

    SipMsgBody* pMessageBody = pCopyMessage->GetMsgBody(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    char* pMsgBuffer = nullptr;
    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pMsgBuffer));
    ASSERT_TRUE(pMsgBuffer != nullptr);

    EXPECT_STREQ("Messages-Waiting: yes\r\n", pMsgBuffer);

    delete[] pMsgBuffer;
    pMessageBody->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    pCopyMessage->SipDelete();

    /* With gzip message body and mandatory headers, success */
    pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: application/pidf+xml\r\n\
Content-Encoding: gzip\r\n\
Content-Length: 19\r\n\
\r\n\
gzip message body\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->HasMandatoryHdrs());

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->HasMandatoryHdrs());

    pCopyMessage = new SipMessage(*pDecodeMessage);
    ASSERT_TRUE(pCopyMessage != nullptr);

    pDecodeMessage->SipDelete();

    pMessageBody = pCopyMessage->GetMsgBody(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pMsgBuffer));
    ASSERT_TRUE(pMsgBuffer != nullptr);

    EXPECT_STREQ("gzip message body\r\n", pMsgBuffer);

    delete[] pMsgBuffer;
    pMessageBody->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    pCopyMessage->SipDelete();

    /* With message summary message body and mandatory headers, success */
    pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: application/simple-message-summary\r\n\
Content-Length: 22\r\n\r\n\
Messages-Waiting: No\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pCopyMessage = new SipMessage(*pDecodeMessage);
    ASSERT_TRUE(pCopyMessage != nullptr);

    pDecodeMessage->SipDelete();

    pMessageBody = pCopyMessage->GetMsgBody(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    pMessageBody->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    EXPECT_STREQ(pBuff, pMsg);

    pCopyMessage->SipDelete();

    /* With multipart message body and mandatory headers, success */
    pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 145\r\n\
UnknownName: UnknownValue\r\n\
Content-Type: multipart/mixed;boundary=\"abcxz\"\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body1\r\n\
\r\n\
Test message body 1\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body2\r\n\
\r\n\
Test message body 2\r\n\
\r\n\
--abcxz--\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pCopyMessage = new SipMessage(*pDecodeMessage);
    ASSERT_TRUE(pCopyMessage != nullptr);

    pDecodeMessage->SipDelete();

    pMessageBody = pCopyMessage->GetMsgBody(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    pMsgBuffer = nullptr;
    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pMsgBuffer));
    ASSERT_TRUE(pMsgBuffer != nullptr);

    EXPECT_STREQ("Test message body 1\r\n", pMsgBuffer);

    delete[] pMsgBuffer;
    pMessageBody->SipDelete();

    pMessageBody = pCopyMessage->GetMsgBody(1);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    pMsgBuffer = nullptr;
    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pMsgBuffer));
    ASSERT_TRUE(pMsgBuffer != nullptr);

    EXPECT_STREQ("Test message body 2\r\n", pMsgBuffer);

    delete[] pMsgBuffer;
    pMessageBody->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    char* pContentLengthLastHdr = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: multipart/mixed;boundary=\"abcxz\"\r\n\
UnknownName: UnknownValue\r\n\
Content-Length: 145\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body1\r\n\
\r\n\
Test message body 1\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body2\r\n\
\r\n\
Test message body 2\r\n\
\r\n\
--abcxz--\r\n";

    EXPECT_STREQ(pBuff, pContentLengthLastHdr);

    pCopyMessage->SipDelete();

    /* response type messagewith multipart message body and mandatory headers, success */
    pMsg = (char*)"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: multipart/mixed;boundary=\"abcxz\"\r\n\
Content-Length: 145\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body1\r\n\
\r\nTest message body 5\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body2\r\n\
\r\nTest message body 6\r\n\
\r\n\
--abcxz--\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    EXPECT_EQ(200, pDecodeMessage->GetStatusCode());
    EXPECT_EQ(200, pDecodeMessage->GetStatusCode());

    pCopyMessage = new SipMessage(*pDecodeMessage);
    ASSERT_TRUE(pCopyMessage != nullptr);

    pDecodeMessage->SipDelete();

    pMessageBody = pCopyMessage->GetMsgBody(0);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    pMsgBuffer = nullptr;
    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pMsgBuffer));
    ASSERT_TRUE(pMsgBuffer != nullptr);

    EXPECT_STREQ("Test message body 5\r\n", pMsgBuffer);

    delete[] pMsgBuffer;
    pMessageBody->SipDelete();

    pMessageBody = pCopyMessage->GetMsgBody(1);
    ASSERT_TRUE(pMessageBody != nullptr);

    EXPECT_EQ(SipMsgBody::SINGLE_BODY, pMessageBody->GetBodyType());

    pMsgBuffer = nullptr;
    EXPECT_EQ(SIP_TRUE, pMessageBody->GetMsgBuffer(&pMsgBuffer));
    ASSERT_TRUE(pMsgBuffer != nullptr);

    EXPECT_STREQ("Test message body 6\r\n", pMsgBuffer);

    delete[] pMsgBuffer;
    pMessageBody->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyMessage->EncodeMsg(&pBuff, &sipMsgLength, 0));

    EXPECT_STREQ(pBuff, pMsg);

    pCopyMessage->SipDelete();

    /* With bad headers and buffer statrs with \r\n, success */
    pMsg = (char*)"\r\nINVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
Via: BadVia\r\n\
Accept: BadAccept\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: application/simple-message-example\r\n\
Content-Length: 0\r\n\
\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(0, pDecodeMessage->GetBadHeaderCount());

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    EXPECT_EQ(2, pDecodeMessage->GetBadHeaderCount());

    SipHeaderList* pBadHeaders = pDecodeMessage->GetBadHdrs();
    ASSERT_TRUE(pBadHeaders != nullptr);

    SipBadHeader* pBadHeader = reinterpret_cast<SipBadHeader*>(pBadHeaders->GetObj(0));

    EXPECT_STREQ("Via", pBadHeader->GetHeaderName());

    pBadHeader->SipDelete();

    pBadHeader = reinterpret_cast<SipBadHeader*>(pBadHeaders->GetObj(1));

    EXPECT_STREQ("Accept", pBadHeader->GetHeaderName());

    pBadHeader->SipDelete();

    pDecodeMessage->DeleteBadHdrList();

    EXPECT_EQ(0, pDecodeMessage->GetBadHeaderCount());

    pDecodeMessage->SipDelete();

    /* Invalid message with no terminating CRLF, fail */
    pMsg = (char*)"Invalid Message";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* Invalid message with terminating CRLF and no SPACE in request line, fail */
    pMsg = (char*)"InvalidMessage\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* Invalid message with terminating CRLF and invalid request line, fail */
    pMsg = (char*)"Invalid RequestLine\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* Invalid message with terminating CRLF and invalid status line, fail */
    pMsg = (char*)"SIP/2.0 InvalidRequestLine\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* No proper header end with CRLF, fail */
    pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* No header end in message with 2 CRLF's, fail */
    pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* CSeq and Request line method mismatch, fail */
    pMsg = (char*)"\r\nINVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
Via: BadVia\r\n\
Accept: BadAccept\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Type: application/simple-message-example\r\n\
Content-Length: 0\r\n\
\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* Content length larger than message body, fail */
    pMsg = (char*)"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: multipart/mixed;boundary=\"abcxz\"\r\n\
Content-Length: 945\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body1\r\n\
\r\nTest message body 5\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body2\r\n\
\r\nTest message body 6\r\n\
\r\n\
--abcxz--\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* multipart body and content type not having boundary, fail */
    pMsg = (char*)"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Type: multipart/mixed\r\n\
Content-Length: 145\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body1\r\n\
\r\nTest message body 5\r\n\
\r\n\
--abcxz\r\n\
Content-Type: application/body2\r\n\
\r\nTest message body 6\r\n\
\r\n\
--abcxz--\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* PRACK with no RAck header - decode success but hasMandatoryHeaders fail, fail */
    pMsg = (char*)"PRACK sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
Accept: BadAccept\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=abcd\r\n\
Call-ID: callid\r\n\
CSeq: 3 PRACK\r\n\
Content-Length: 0\r\n\
\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));
    EXPECT_EQ(SIP_FALSE, pDecodeMessage->HasMandatoryHdrs());

    pDecodeMessage->SipDelete();

    /* With message body and no content-type header, fail */
    pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 23\r\n\
\r\n\
Messages-Waiting: yes\r\n";

    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_FALSE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* With compact form of header name & few other headers, success */
    pMsg = (char*)"SIP/2.0 500 Internal Server Error\r\n\
v: SIP/2.0/TCP host;branch=test-br\r\n\
f: <sip:user@host>;tag=abcd\r\n\
t: <sip:userA@host>\r\n\
i: callid\r\n\
CSeq: 1 INVITE\r\n\
Expires: 1344\r\n\
Feature-Caps: *;+g.3gpp.atcf=\"<tel:9999>\"\r\n\
Retry-After: 5\r\n\
z: unknown\r\n\
qwertyuioasdfghjklzxcvbnmqwetyydfgh: badheader\r\n\
l: 0\r\n\
\r\n";
    pDecodeMessage = new SipMessage();
    ASSERT_TRUE(pDecodeMessage != nullptr);

    EXPECT_EQ(SIP_TRUE, pDecodeMessage->DecCompleteMsg(pMsg, strlen(pMsg)));

    pDecodeMessage->SipDelete();

    /* Calling GetHdrType with null header name, fail */
    EXPECT_EQ(SipHeaderBase::TYPE_INVALID, SipGetHdrType(SIP_NULL));
}

}  // namespace android
