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

#include "ISipNetworkUtil.h"
#include "SipStackError.h"
#include "SipStackManager.h"
#include "SipTxnContext.h"
#include "SipUtil.h"
#include "platform/SipString.h"
#include "transport/SipTransportHandler.h"
#include "txn/include/MockISipTransactionCallback.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

SipTxn* pTxn = SIP_NULL;

class SipTestNetworkUtil : public ISipNetworkUtil
{
public:
    static SIP_BOOL bSendStatus;
    SipTestNetworkUtil() {}
    ~SipTestNetworkUtil() {}

public:
    SIP_BOOL SendToNetwork(SipTransportBuffer*, SipTransportParameter*, ISipUserData*) override
    {
        return (bSendStatus == SIP_TRUE) ? SIP_TRUE : SIP_FALSE;
    }
};

SIP_BOOL SipTestNetworkUtil::bSendStatus = SIP_TRUE;

namespace android
{

class SipStackManagerTest : public ::testing::Test
{
public:
    SipTxnKey* pTxnKey;
    MockISipTransactionCallback* pMockISipTransactionCallback;
    static constexpr SIP_INT32 TIMER_ID = 1;

protected:
    virtual void SetUp() override
    {
        pMockISipTransactionCallback = new MockISipTransactionCallback();

        ON_CALL(*pMockISipTransactionCallback, StartTimer(_, _, _))
                .WillByDefault(Return(static_cast<void*>(const_cast<SIP_INT32*>(&TIMER_ID))));

        ON_CALL(*pMockISipTransactionCallback, FetchTransaction(_, _, _))
                .WillByDefault(Invoke(
                        [](IN const SipTxnKey* pTxnKey, Unused, OUT SipTxn*& pOutTxn)
                        {
                            if (pTxn == SIP_NULL)
                            {
                                if (pOutTxn != SIP_NULL)
                                {
                                    pTxn = pOutTxn;
                                    return SIP_TRUE;
                                }
                                return SIP_FALSE;
                            }

                            pOutTxn = pTxn;
                            return SIP_TRUE;
                        }));

        ON_CALL(*pMockISipTransactionCallback, FetchTransaction(_, _, _, _))
                .WillByDefault(Invoke(
                        [](Unused, Unused, Unused, OUT SipTxn*& pOutTxn)
                        {
                            if (pTxn == SIP_NULL)
                            {
                                if (pOutTxn != SIP_NULL)
                                {
                                    pTxn = pOutTxn;
                                    return SIP_TRUE;
                                }
                                return SIP_FALSE;
                            }

                            pOutTxn = pTxn;
                            return SIP_TRUE;
                        }));

        ON_CALL(*pMockISipTransactionCallback, ReleaseTransaction(_, _, _, _))
                .WillByDefault(Invoke(
                        [](Unused, Unused, Unused, SipTxn*& pOutTxn)
                        {
                            pOutTxn = pTxn;
                            pTxn = SIP_NULL;
                            return SIP_TRUE;
                        }));

        pTxn = SIP_NULL;
        pTxnKey = SIP_NULL;
    }

    virtual void TearDown() override
    {
        if (pMockISipTransactionCallback != SIP_NULL)
        {
            delete pMockISipTransactionCallback;
            pMockISipTransactionCallback = SIP_NULL;
        }
    }
};

TEST_F(SipStackManagerTest, SendRecvMessage)
{
    SipStackManager* pSipStackManager = SipStackManager::GetInstance();
    ASSERT_TRUE(pSipStackManager != nullptr);

    pSipStackManager->RegisterNetwork(new SipTestNetworkUtil());
    pSipStackManager->RegisterTransactionCallback(pMockISipTransactionCallback);

    SipTransportParameter objTransportParam;
    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);
    objTransportParam.SetHostAddress("192.168.1.2");
    objTransportParam.SetPort(5060);
    objTransportParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    ISipUserData objUserData(SIP_NULL);

    /* Invite client transaction check - Start */
    const SIP_CHAR* pReqMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    SIP_UINT32 nLength = SipPf_Strlen(pReqMsg);

    SipMessage* pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    SIP_UINT16 nError = 0;
    SipTxnKey* pTxnKey = SIP_NULL;

    /* Passing invalig arguments, fail */
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(nullptr, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(
                    pMessage, nullptr, &objUserData, pReqMsg, nLength, &pTxnKey, &nError));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    nullptr, &nError));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, nullptr));

    /* Pass valid arguments, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::INVITE_CLIENT, pTxn->GetTxnType());

    ASSERT_TRUE(pTxnKey != nullptr);

    ASSERT_TRUE(pTxnKey->GetToTag() == nullptr);

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    /* Resend message, fail */
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(ETXN_ALREADYTRANSACTIONINPROCESSERROR, nError);

    pMessage->SipDelete();

    const SIP_CHAR* pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    SIP_INT32 eTxnStatus = SipTxn::STATUS_INVALID;

    SipMessage* pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    /* Passing invalig arguments, fail */
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(
                    nullptr, nullptr, &objUserData, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(
                    pRespSipMessage, nullptr, &objUserData, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(
                    pRespSipMessage, &objTransportParam, &objUserData, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, nullptr));

    /* Pass valid arguments, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    ASSERT_TRUE(pTxnKey != nullptr);

    EXPECT_STREQ(pTxnKey->GetToTag(), "dcba");

    pSipStackManager->TerminateTxn(pTxnKey);
    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    /* In case of 2xx response received when no matching txn, return as ignored message */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_IGNORE_RESP, eTxnStatus);

    pRespSipMessage->SipDelete();
    /* Invite client check for send receive - End */

    const SIP_CHAR* pAckMsg = "ACK sip:userAck@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-brAck\r\n\
From: <sip:userAck@host>;tag=Ackabcd\r\n\
To: <sip:user@host>;tag=Ackdcba\r\n\
Call-ID: callidAck\r\n\
CSeq: 1 ACK\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pAckMsg);

    SipMessage* pAckSipMessage = new SipMessage();
    ASSERT_TRUE(pAckSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pAckSipMessage->Decode(pAckMsg, nLength));

    /* No matching Invite transaction for ACK, retransmission message */
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(pAckSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_INVALID_MESSAGE, eTxnStatus);

    pAckSipMessage->SipDelete();

    /* Invite client with fail response receive, response with failure ACK - Start */
    pReqMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pReqMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_TCP);

    nError = 0;
    pTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::INVITE_CLIENT, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    pMessage->SipDelete();

    pRespMsg = "SIP/2.0 486 Busy Here\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pRespSipMessage->SipDelete();

    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);

    // Clear INVITE client transaction.
    pTxn->RemoveFromTxnPool();
    /* Invite client with fail response receive, response with failure ACK - End */

    /* Invite server check for send receive - Start */
    pReqMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pReqMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    /* Pass valid arguments, success */
    ASSERT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(
                    pMessage, &objTransportParam, &objUserData, &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::INVITE_SERVER, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    /* Pass valid arguments, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pRespSipMessage, &objTransportParam, &objUserData, pRespMsg,
                    nLength, &pTxnKey, &nError));

    pRespSipMessage->SipDelete();

    /* Received retransmitted INVITE in accepted state - ignore message, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(
                    pMessage, &objTransportParam, &objUserData, &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_IGNORE_REQ, eTxnStatus);
    pMessage->SipDelete();
    /* Invite server check for send receive - End */

    pAckMsg = "ACK sip:userAck@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-brAck\r\n\
From: <sip:userAck@host>;tag=Ackabcd\r\n\
To: <sip:user@host>;tag=Ackdcba\r\n\
Call-ID: callidAck\r\n\
CSeq: 1 ACK\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pAckMsg);

    pAckSipMessage = new SipMessage();
    ASSERT_TRUE(pAckSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pAckSipMessage->Decode(pAckMsg, nLength));

    EXPECT_TRUE(pTxn != nullptr);

    /* ACK for Invite transaction, valid message */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pAckSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pAckSipMessage->SipDelete();

    pSipStackManager->TerminateTxn(pTxnKey);

    pTxnKey->SipDelete();

    /* Send to network fail */
    SipTestNetworkUtil::bSendStatus = SIP_FALSE;

    pReqMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pReqMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    nError = 0;
    pTxnKey = SIP_NULL;

    /* Pass valid arguments - Send to network fail, fail */
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));
    pMessage->SipDelete();

    pTxn->SipDelete();
    pTxn = SIP_NULL;

    SipTestNetworkUtil::bSendStatus = SIP_TRUE;

    /* Non-Invite server with TCP, check for send receive - Start */
    pReqMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pReqMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_TCP);

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(
                    pMessage, &objTransportParam, &objUserData, &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::NON_INVITE_SERVER, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    pMessage->SipDelete();

    pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    /* Pass valid arguments, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pRespSipMessage, &objTransportParam, &objUserData, pRespMsg,
                    nLength, &pTxnKey, &nError));

    pRespSipMessage->SipDelete();

    /* Non-Invite server with TCP, check for send receive - End */

    /* Non-Invite response message received where no transaction present, stray message */
    pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::STATUS_STRAY_RESP, eTxnStatus);

    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);

    pRespSipMessage->SipDelete();

    /* Invalid message type - OnReceiveTransp fail while checking mandatory params */
    SipMessage* pInvalidMsg = new SipMessage(SipMessage::TYPE_INVALID);
    ASSERT_TRUE(pInvalidMsg != nullptr);

    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->OnRecvMessage(
                    pInvalidMsg, &objTransportParam, &objUserData, &eTxnStatus, &pTxnKey, &nError));

    pInvalidMsg->SipDelete();

    /* Non-Invite Client with receive 1xx in completed state, ignore response, success - Start*/
    pReqMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pReqMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    nError = 0;
    pTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::NON_INVITE_CLIENT, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    pMessage->SipDelete();

    pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 100 Trying\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 REGISTER\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::STATUS_IGNORE_RESP, eTxnStatus);

    pSipStackManager->TerminateTxn(pTxnKey);

    if (pTxn != SIP_NULL)
    {
        pTxn->SipDelete();
        pTxn = SIP_NULL;
    }
    /* Non-Invite Client with receive 1xx in completed state, ignore response, success - End*/

    pSipStackManager->Destruct();
}

TEST_F(SipStackManagerTest, RecvResponseMessage)
{
    SipStackManager* pSipStackManager = SipStackManager::GetInstance();
    ASSERT_TRUE(pSipStackManager != nullptr);

    pSipStackManager->RegisterNetwork(new SipTestNetworkUtil());
    pSipStackManager->RegisterTransactionCallback(pMockISipTransactionCallback);

    SipTransportParameter objTransportParam;
    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);
    objTransportParam.SetHostAddress("192.168.1.2");
    objTransportParam.SetPort(5060);
    objTransportParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    ISipUserData objUserData(SIP_NULL);

    /* Invite client transaction check - Start */
    const SIP_CHAR* pReqMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    SIP_UINT32 nLength = SipPf_Strlen(pReqMsg);

    SipMessage* pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    SIP_UINT16 nError = 0;
    SipTxnKey* pTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::INVITE_CLIENT, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;
    pMessage->SipDelete();

    const SIP_CHAR* pRespMsg = "SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked3\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
RSeq: 20\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    SIP_INT32 eTxnStatus = SipTxn::STATUS_INVALID;

    SipMessage* pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked1\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    ASSERT_TRUE(pTxnKey != nullptr);
    EXPECT_STREQ(pTxnKey->GetToTag(), "forked1");

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    /* In case of retransmitted 2xx for INVITE, return as valid message */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked2\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::STATUS_IGNORE_RESP, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 486 Busy Here\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked2\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::STATUS_IGNORE_RESP, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked3\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
RSeq: 20\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_RETRANSMISSION, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 180 Ringing\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked3\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
RSeq: 20\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_RETRANSMISSION, eTxnStatus);

    pRespSipMessage->SipDelete();

    pRespMsg = "SIP/2.0 200 OK\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=forked3\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
RSeq: 20\r\n\
\r\n";

    nLength = SipPf_Strlen(pRespMsg);
    eTxnStatus = SipTxn::STATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->Decode(pRespMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pTxn->RemoveFromTxnPool();
    pSipStackManager->Destruct();
}

TEST_F(SipStackManagerTest, OnRecvTanspError)
{
    SipStackManager* pSipStackManager = SipStackManager::GetInstance();
    ASSERT_TRUE(pSipStackManager != nullptr);

    SIP_UINT16 nError = 0;
    SipTxnKey* pTxnKey = SIP_NULL;

    EXPECT_EQ(SIP_FALSE, pSipStackManager->OnRecvTanspError(0, pTxnKey, &nError));

    pSipStackManager->RegisterNetwork(new SipTestNetworkUtil());
    pSipStackManager->RegisterTransactionCallback(pMockISipTransactionCallback);

    SipTransportParameter objTransportParam;
    objTransportParam.SetTranspProtocol(SipTransportInfo::PROTOCOL_TCP);
    objTransportParam.SetHostAddress("192.168.1.2");
    objTransportParam.SetPort(5060);
    objTransportParam.SetTanspIpType(SipTransportInfo::NETWORK_IPV4);

    ISipUserData objUserData(SIP_NULL);

    /* Invite client transaction check - Start */
    const SIP_CHAR* pReqMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/TCP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    SIP_UINT32 nLength = SipPf_Strlen(pReqMsg);

    SipMessage* pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->Decode(pReqMsg, nLength));

    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::INVITE_CLIENT, pTxn->GetTxnType());

    EXPECT_EQ(SIP_TRUE, pSipStackManager->OnRecvTanspError(0, pTxnKey, &nError));

    pMessage->SipDelete();

    pSipStackManager->TerminateTxn(pTxnKey);

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    pSipStackManager->Destruct();
}

}  // namespace android
