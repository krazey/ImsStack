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

#include "SipStackCallback.h"
#include "platform/SipString.h"
#include "transport/SipTransportHandler.h"

namespace android
{

SipTxn* pTxn = SIP_NULL;

SIP_BOOL FetchTxn_Fail(SIP_VOID* /*pvTxnKey*/, SIP_INT32 /*nOption*/, SIP_VOID** /*ppvOutTxnKey*/,
        SIP_VOID** ppvTxn)
{
    *ppvTxn = SIP_NULL;
    return SIP_TRUE;
}

SIP_BOOL FetchTxn_NotExists(SIP_VOID* /*pvTxnKey*/, SIP_INT32 /*nOption*/,
        SIP_VOID** /*ppvOutTxnKey*/, SIP_VOID** /*ppvTxn*/)
{
    return SIP_FALSE;
}

SIP_BOOL FetchTxn_Exists(SIP_VOID* /*pvTxnKey*/, SIP_INT32 /*nOption*/, SIP_VOID** /*ppvOutTxnKey*/,
        SIP_VOID** ppvTxn)
{
    *ppvTxn = pTxn;
    return SIP_TRUE;
}

class SipTransportHandlerTest : public ::testing::Test
{
public:
    SipTransportInfo* pTranspInfo;
    SipMessage* pMessage;
    SipTxnKey* pTxnKey;
    SipTransportHandler* pTranspHandler;
    SipTransportParameter* pTranspParam;

protected:
    virtual void SetUp() override
    {
        SipMsgUtil::Init();

        pTranspInfo = SIP_NULL;
        pMessage = SIP_NULL;
        pTxnKey = SIP_NULL;
        pTranspHandler = SIP_NULL;
        pTranspParam = SIP_NULL;
    }

    virtual void TearDown() override {}

    /* This utility is used only for receive message */
    void FillTransportParameters(const SIP_CHAR* pMsg)
    {
        SIP_UINT16 nError = 0;
        SIP_UINT32 nLength = SipPf_Strlen(pMsg);

        pMessage = new SipMessage();
        ASSERT_TRUE(pMessage != nullptr);
        EXPECT_EQ(SIP_TRUE, pMessage->Decode(pMsg, nLength));

        pTranspHandler = new SipTransportHandler();
        ASSERT_TRUE(pTranspHandler != nullptr);

        pTranspParam = new SipTransportParameter();
        ASSERT_TRUE(pTranspParam != nullptr);
        pTranspParam->SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);
        pTranspParam->SetHostAddress("192.168.1.2");
        pTranspParam->SetPort(5060);
        pTranspParam->SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

        /* Using this call to fill pTranspInfo */
        EXPECT_EQ(SIP_TRUE,
                pTranspHandler->OnSendTransp(
                        pMessage, pTranspParam, pMsg, nLength, &pTranspInfo, &nError));

        pTxnKey = new SipTxnKey(pMessage, &nError);
        ASSERT_TRUE(pTxnKey != nullptr);

        SIP_INT32 eMsgType = pMessage->GetMsgType();
        SIP_INT32 eMethodType = pMessage->GetMethodType();
        SIP_INT32 eTxnType;

        if (eMsgType == SipMessage::REQ_TYPE)
        {
            if ((eMethodType == SipMessage::METHOD_INVITE) ||
                    (eMethodType == SipMessage::METHOD_ACK))
            {
                eTxnType = SipTxn::INVITE_SERVER;
            }
            else
            {
                eTxnType = SipTxn::NON_INVITE_SERVER;
            }
        }
        else
        {
            if (eMethodType == SipMessage::METHOD_INVITE)
            {
                eTxnType = SipTxn::INVITE_CLIENT;
            }
            else
            {
                eTxnType = SipTxn::NON_INVITE_CLIENT;
            }
        }

        pTxn = new SipTxn(eTxnType, pTxnKey, pMessage, nullptr, &nError);
        ASSERT_TRUE(pTxn != nullptr);
        pTxn->UpdateTranspInfo(pTranspInfo);
    }

    void ClearTransportParameters()
    {
        pMessage->SipDelete();
        pTxnKey->SipDelete();
        pTxn->SipDelete();
        delete pTranspHandler;
    }
};

TEST_F(SipTransportHandlerTest, OnSendTransp)
{
    SipTransportHandler objTranspHandler;
    SipTransportInfo* pTranspInfo = SIP_NULL;

    const SIP_CHAR* pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    SIP_UINT32 nLength = SipPf_Strlen(pMsg);

    SIP_UINT16 nError = 0;

    /* SipMessage and pTranspParam null, fail */
    EXPECT_EQ(SIP_FALSE,
            objTranspHandler.OnSendTransp(nullptr, nullptr, pMsg, nLength, &pTranspInfo, &nError));

    SipMessage* pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);

    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pMsg, nLength));

    /* pTranspParam null, fail */
    EXPECT_EQ(SIP_FALSE,
            objTranspHandler.OnSendTransp(pMessage, nullptr, pMsg, nLength, &pTranspInfo, &nError));

    SipTransportParameter objTranspParam;
    objTranspParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);
    objTranspParam.SetHostAddress("192.168.1.2");
    objTranspParam.SetPort(5060);
    objTranspParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    /* Pass valid input, success */
    EXPECT_EQ(SIP_TRUE,
            objTranspHandler.OnSendTransp(
                    pMessage, &objTranspParam, pMsg, nLength, &pTranspInfo, &nError));
}

TEST_F(SipTransportHandlerTest, OnRecvTransp)
{
    /* INVITE Request */
    const SIP_CHAR* pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    FillTransportParameters(pMsg);

    /* Fecth txn wrong, fail */
    // clang-format off
    SipStackCallbacks stCallbacks = {
            &FetchTxn_Fail,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    SIP_UINT16 nError = 0;
    SIP_INT32 nTxnStatus = 0;
    SIP_BOOL bTxnExist = SIP_FALSE;
    SipTxnKey* pNewTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTransp(
                    pMessage, pTranspParam, &nTxnStatus, &bTxnExist, &pNewTxnKey, &nError));

    /* Txn not exists - INVITE Request which is new request, success */
    // clang-format off
    stCallbacks = {
            &FetchTxn_NotExists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTransp(
                    pMessage, pTranspParam, &nTxnStatus, &bTxnExist, &pNewTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_NEW_REQ_RECVD, nTxnStatus);

    /* Txn exists - INVITE Request which is valid, success */
    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    nTxnStatus = 0;
    bTxnExist = SIP_FALSE;
    pNewTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTransp(
                    pMessage, pTranspParam, &nTxnStatus, &bTxnExist, &pNewTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, nTxnStatus);

    ClearTransportParameters();

    /* Non-Invite request message */
    pMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    FillTransportParameters(pMsg);

    /* Txn exists - NON-INVITE, success */
    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    nTxnStatus = 0;
    bTxnExist = SIP_FALSE;
    pNewTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTransp(
                    pMessage, pTranspParam, &nTxnStatus, &bTxnExist, &pNewTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, nTxnStatus);

    ClearTransportParameters();

    /* Non-Invite response message */
    pMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    FillTransportParameters(pMsg);

    /* Txn not exists - NON-INVITE Response, success */
    // clang-format off
    stCallbacks = {
            &FetchTxn_NotExists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    nTxnStatus = 0;
    bTxnExist = SIP_FALSE;
    pNewTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTransp(
                    pMessage, pTranspParam, &nTxnStatus, &bTxnExist, &pNewTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_STRAY_RESP, nTxnStatus);

    ClearTransportParameters();

    /* Invite response message */
    pMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    FillTransportParameters(pMsg);

    /* Txn exists - INVITE Response which is valid, success */
    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    nTxnStatus = 0;
    bTxnExist = SIP_FALSE;
    pNewTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTransp(
                    pMessage, pTranspParam, &nTxnStatus, &bTxnExist, &pNewTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, nTxnStatus);

    ClearTransportParameters();
}

TEST_F(SipTransportHandlerTest, OnRecvTanspError)
{
    SIP_UINT16 nError = 0;
    SIP_INT32 nTxnStatus = 0;

    SipTransportInfo* pNewTranspInfo = SIP_NULL;

    const SIP_CHAR* pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    SIP_UINT32 nLength = SipPf_Strlen(pMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pMsg, nLength));

    pTxnKey = new SipTxnKey(pMessage, &nError);
    ASSERT_TRUE(pTxnKey != nullptr);

    pTranspHandler = new SipTransportHandler();
    ASSERT_TRUE(pTranspHandler != nullptr);

    /* Fecth txn wrong, fail */
    // clang-format off
    SipStackCallbacks stCallbacks = {
            &FetchTxn_Fail,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    /* Txn not exists, fail */
    // clang-format off
    stCallbacks = {
            &FetchTxn_NotExists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    /* Txn exists */
    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pMessage, nullptr, &nError);
    ASSERT_TRUE(pTxn != nullptr);

    /* TXN exists but no transport info, fail */
    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    pTranspInfo = new SipTransportInfo(nullptr, nullptr);
    ASSERT_TRUE(pTranspInfo != nullptr);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* transport info present but transport parameter not present, fail */
    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    pTxn->SipDelete();

    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pMessage, nullptr, &nError);
    ASSERT_TRUE(pTxn != nullptr);

    pTranspParam = new SipTransportParameter();
    ASSERT_TRUE(pTranspParam != nullptr);
    pTranspParam->SetTranspProtocol(SipTransportInfo::PROTOCOL_INVALID);
    pTranspParam->SetHostAddress("192.168.1.2");
    pTranspParam->SetPort(5060);
    pTranspParam->SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    /* Using this call to fill pTranspInfo */
    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnSendTransp(
                    pMessage, pTranspParam, pMsg, nLength, &pTranspInfo, &nError));

    pTxn->UpdateTranspInfo(pTranspInfo);

    /* TXN exists/transport info exists and transport param exists.
       Invalid transp protocol, fail */
    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    /* Change transp protocol to TCP */
    pTranspParam->SetTranspProtocol(SipTransportInfo::PROTOCOL_TCP);

    pTranspParam->SetHostAddress("192.168.1.2");

    /* Using this call to fill pTranspInfo */
    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnSendTransp(
                    pMessage, pTranspParam, pMsg, nLength, &pTranspInfo, &nError));

    pTxn->UpdateTranspInfo(pTranspInfo);

    /* TXN exists/transport info exists and transport param exists.
       Updates transport TCP to UDP for retransmission, success */
    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    EXPECT_EQ(SipTxn::STATUS_RETRANSMISSION, nTxnStatus);

    SipTransportParameter* pMsgSentTransParam = pNewTranspInfo->GetMsgSentTranspParam();
    ASSERT_TRUE(pMsgSentTransParam != nullptr);

    EXPECT_EQ(SipTransportInfo::PROTOCOL_UDP, pMsgSentTransParam->GetTranspProtocol());

    pMessage->SipDelete();
    pTxn->SipDelete();
    delete pTranspHandler;

    /* Transport protocol is not TCP/UDP */
    pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TLS host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pMsg, nLength));

    pTxnKey = new SipTxnKey(pMessage, &nError);
    ASSERT_TRUE(pTxnKey != nullptr);

    pTranspHandler = new SipTransportHandler();
    ASSERT_TRUE(pTranspHandler != nullptr);

    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pMessage, nullptr, &nError);
    ASSERT_TRUE(pTxn != nullptr);

    pTranspParam = new SipTransportParameter();
    ASSERT_TRUE(pTranspParam != nullptr);
    pTranspParam->SetTranspProtocol(SipTransportInfo::PROTOCOL_TLS);
    pTranspParam->SetHostAddress("192.168.1.2");
    pTranspParam->SetPort(5060);
    pTranspParam->SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    /* Using this call to fill pTranspInfo */
    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnSendTransp(
                    pMessage, pTranspParam, pMsg, nLength, &pTranspInfo, &nError));

    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Transport cannot be changed from TLS to UDP, success */
    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    pMessage->SipDelete();
    pTxn->SipDelete();
    delete pTranspHandler;

    /* No Via header to update transport header, fail */
    pMsg = "INVITE sip:user@host SIP/2.0\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pMsg, nLength));

    pTxnKey = new SipTxnKey(pMessage, &nError);
    ASSERT_TRUE(pTxnKey != nullptr);

    pTranspHandler = new SipTransportHandler();
    ASSERT_TRUE(pTranspHandler != nullptr);

    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pMessage, nullptr, &nError);
    ASSERT_TRUE(pTxn != nullptr);

    pTranspParam = new SipTransportParameter();
    ASSERT_TRUE(pTranspParam != nullptr);
    pTranspParam->SetTranspProtocol(SipTransportInfo::PROTOCOL_TCP);
    pTranspParam->SetHostAddress("192.168.1.2");
    pTranspParam->SetPort(5060);
    pTranspParam->SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    /* Using this call to fill pTranspInfo */
    EXPECT_EQ(SIP_TRUE,
            pTranspHandler->OnSendTransp(
                    pMessage, pTranspParam, pMsg, nLength, &pTranspInfo, &nError));

    pTxn->UpdateTranspInfo(pTranspInfo);

    /* No Via header to update transport, fail */
    EXPECT_EQ(SIP_FALSE,
            pTranspHandler->OnRecvTanspError(
                    0, pTxnKey, &nTxnStatus, &pNewTranspInfo, nullptr, &nError));

    pMessage->SipDelete();
    pTxn->SipDelete();
    delete pTranspHandler;
}

TEST_F(SipTransportHandlerTest, IsInviteTxnPresentForAckTxn)
{
    SIP_UINT16 nError = 0;

    const SIP_CHAR* pMsg = "ACK sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=Totag\r\n\
Call-ID: callid\r\n\
CSeq: 3 ACK\r\n\
Content-Length: 0\r\n\
\r\n";

    SIP_UINT32 nLength = SipPf_Strlen(pMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pMsg, nLength));

    pTxnKey = new SipTxnKey(pMessage, &nError);
    ASSERT_TRUE(pTxnKey != nullptr);

    pTranspHandler = new SipTransportHandler();
    ASSERT_TRUE(pTranspHandler != nullptr);

    /* Fecth txn wrong, fail */
    // clang-format off
    SipStackCallbacks stCallbacks = {
            &FetchTxn_Fail,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    EXPECT_EQ(SIP_FALSE, pTranspHandler->IsInviteTxnPresentForAckTxn(pTxnKey));

    /* Txn not exists, fail */
    // clang-format off
    stCallbacks = {
            &FetchTxn_NotExists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    EXPECT_EQ(SIP_FALSE, pTranspHandler->IsInviteTxnPresentForAckTxn(pTxnKey));

    /* Txn exists */
    // clang-format off
    stCallbacks = {
            &FetchTxn_Exists,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL,
            SIP_NULL
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pMessage, nullptr, &nError);
    ASSERT_TRUE(pTxn != nullptr);

    EXPECT_EQ(SIP_TRUE, pTranspHandler->IsInviteTxnPresentForAckTxn(pTxnKey));

    pMessage->SipDelete();
    pTxn->SipDelete();
    delete pTranspHandler;
}

}  // namespace android
