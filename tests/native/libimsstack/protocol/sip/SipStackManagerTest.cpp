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

#include "SipStackManager.h"
#include "ISipNetworkUtil.h"
#include "ISipTxnListener.h"
#include "ISipTimerUtil.h"
#include "transport/SipTransportHandler.h"
#include "SipStackCallback.h"
#include "SipTxnContext.h"
#include "SipUtil.h"
#include "sip_error.h"

SipTxn* pTxn = SIP_NULL;

class SipTestNetworkUtil : public ISipNetworkUtil
{
public:
    SipTestNetworkUtil() {}
    ~SipTestNetworkUtil() {}

public:
    SIP_BOOL SendToNetwork(SipTransportBuffer*, SipTransportParameter*, ISipUserData*)
    {
        return SIP_TRUE;
    }
};

class SipTestTxnListener : public ISipTxnListener
{
public:
    SipTestTxnListener() {}
    virtual ~SipTestTxnListener() {}

    SIP_BOOL TxnTimeout(ISipUserData*, IMS_SINT32) { return SIP_TRUE; }

    // SIP_BOOL TxnTerminated(ISipUserData* pUserData)
    SIP_BOOL TxnTerminated(ISipUserData*) { return SIP_TRUE; }
};

namespace android
{
SIP_BOOL FetchTransactionStub(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID** ppvTxn)
{
    if (pTxn == SIP_NULL)
    {
        if (*ppvTxn != SIP_NULL)
        {
            pTxn = (SipTxn*)*ppvTxn;
            return SIP_TRUE;
        }
        return SIP_FALSE;
    }

    *ppvTxn = (SIP_VOID*)pTxn;

    return SIP_TRUE;
}

SIP_BOOL StartTimerStub(SIP_UINT32, SipTimerCallback, SIP_VOID*, SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_BOOL ReleaseTransactionStub(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID** ppvTxn)
{
    *ppvTxn = (SIP_VOID*)pTxn;
    pTxn = SIP_NULL;
    return SIP_TRUE;
}

SIP_VOID* CreateAckRequestStub(SIP_VOID*, ISipUserData*)
{
    return SIP_NULL;
}

SIP_VOID PreprocessMessageStub(SIP_VOID*, ISipUserData*) {}

SIP_VOID PostprocessMessageStub(IN SIP_VOID*, IN SIP_CHAR*, IN SIP_UINT32, IN ISipUserData*) {}

SIP_VOID DisplayTxnKeyStub(IN SIP_VOID*) {}

SIP_BOOL StopTimerStub(IN SIP_VOID*, OUT SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_VOID OnTimerExpiredStub(IN ISipUserData*, IN SIP_INT32) {}

class SipStackManagerTest : public ::testing::Test
{
public:
    SipTxnKey* pTxnKey;

protected:
    virtual void SetUp() override { pTxnKey = SIP_NULL; }

    virtual void TearDown() override {}

    SIP_BOOL DeleteTxn(SipTxnKey* pTxnKey)
    {
        if (pTxnKey == SIP_NULL)
        {
            return SIP_FALSE;
        }

        SipTxn* pTransaction = SIP_NULL;
        SipTxnKey* pOutTxnKey = SIP_NULL;
        SIP_BOOL bTxnExist = sip_cbk_releaseTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey),
                TXN_OPT_REMOVE, reinterpret_cast<SIP_VOID**>(&pOutTxnKey),
                reinterpret_cast<SIP_VOID**>(&pTransaction));

        if (bTxnExist == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        if (pTransaction != SIP_NULL)
        {
            pTransaction->SipDelete();
        }

        if (pOutTxnKey != SIP_NULL)
        {
            pOutTxnKey->SipDelete();
        }

        return SIP_TRUE;
    }
};

TEST_F(SipStackManagerTest, SendRecvMessage)
{
    SipStackManager* pSipStackManager = SipStackManager::GetInstance();
    ASSERT_TRUE(pSipStackManager != nullptr);

    pSipStackManager->GetSipUtil()->RegisterNetwork(new SipTestNetworkUtil());
    pSipStackManager->GetSipUtil()->RegisterTxnListener(new SipTestTxnListener());

    // clang-format off
    SipStackCallbacks stCallbacks = {
            &FetchTransactionStub,
            &ReleaseTransactionStub,
            &StartTimerStub,
            &StopTimerStub,
            &OnTimerExpiredStub,
            &CreateAckRequestStub,
            &PreprocessMessageStub,
            &PostprocessMessageStub,
            &DisplayTxnKeyStub,
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    SipTransportParameter objTransportParam;
    objTransportParam.setTranspProtocol(SipTransportInfo::PROTOCOL_UDP);
    objTransportParam.setHostAddress("192.168.1.2");
    objTransportParam.setPort(5060);
    objTransportParam.setTanspIpType(SipTransportInfo::NETWORK_IPV4);

    ISipUserData objUserData(SIP_NULL);

    /* Invite client transaction check - Start */
    char* pReqMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    unsigned int nLength = strlen(pReqMsg);

    SipMessage* pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->DecCompleteMsg(pReqMsg, nLength));

    unsigned short nError = 0;
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

    EXPECT_EQ(SipTxn::INV_CLI_TXN, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    /* Resend message, fail */
    EXPECT_EQ(SIP_FALSE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    EXPECT_EQ(ETXN_ALREADYTRANSACTIONINPROCESSERROR, nError);

    pMessage->SipDelete();

    char* pRespMsg = (char*)"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = strlen(pRespMsg);
    int eTxnStatus = ETXNSTATUS_INVALID;

    SipMessage* pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->DecCompleteMsg(pRespMsg, nLength));

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

    /* In case of retransmitted 2xx for INVITE, return as stray message */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pRespSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_2XX_STRAY_RESP, eTxnStatus);

    pRespSipMessage->SipDelete();
    /* Invite client check for send receive - End */

    char* pAckMsg = (char*)"ACK sip:userAck@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-brAck\r\n\
From: <sip:userAck@host>;tag=Ackabcd\r\n\
To: <sip:user@host>;tag=Ackdcba\r\n\
Call-ID: callidAck\r\n\
CSeq: 1 ACK\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = strlen(pAckMsg);

    SipMessage* pAckSipMessage = new SipMessage();
    ASSERT_TRUE(pAckSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pAckSipMessage->DecCompleteMsg(pAckMsg, nLength));

    /* No matching Invite transaction for ACK, retransmission message */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pAckSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_RETRANSMISSION, eTxnStatus);

    pAckSipMessage->SipDelete();

    /* Invite server check for send receive - Start */
    pReqMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = strlen(pReqMsg);

    pMessage = new SipMessage();
    ASSERT_TRUE(pMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pMessage->DecCompleteMsg(pReqMsg, nLength));

    /* Pass valid arguments, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(
                    pMessage, &objTransportParam, &objUserData, &eTxnStatus, &pTxnKey, &nError));

    EXPECT_EQ(SipTxn::INV_SER_TXN, pTxn->GetTxnType());

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    pMessage->SipDelete();

    pRespMsg = (char*)"SIP/2.0 200 OK\r\n\
Via: SIP/2.0/UDP host;branch=test-br\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:user@host>;tag=dcba\r\n\
Call-ID: callid\r\n\
CSeq: 3 INVITE\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = strlen(pRespMsg);
    eTxnStatus = ETXNSTATUS_INVALID;

    pRespSipMessage = new SipMessage();
    ASSERT_TRUE(pRespSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pRespSipMessage->DecCompleteMsg(pRespMsg, nLength));

    /* Pass valid arguments, success */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->SendMsg(pMessage, &objTransportParam, &objUserData, pReqMsg, nLength,
                    &pTxnKey, &nError));

    pRespSipMessage->SipDelete();
    /* Invite server check for send receive - End */

    pAckMsg = (char*)"ACK sip:userAck@host SIP/2.0\r\n\
Via: SIP/2.0/UDP host;branch=test-brAck\r\n\
From: <sip:userAck@host>;tag=Ackabcd\r\n\
To: <sip:user@host>;tag=Ackdcba\r\n\
Call-ID: callidAck\r\n\
CSeq: 1 ACK\r\n\
Content-Length: 0\r\n\
\r\n";

    nLength = strlen(pAckMsg);

    pAckSipMessage = new SipMessage();
    ASSERT_TRUE(pAckSipMessage != nullptr);
    EXPECT_EQ(SIP_TRUE, pAckSipMessage->DecCompleteMsg(pAckMsg, nLength));

    EXPECT_TRUE(pTxn != nullptr);

    /* ACK for Invite transaction, valid message */
    EXPECT_EQ(SIP_TRUE,
            pSipStackManager->OnRecvMessage(pAckSipMessage, &objTransportParam, &objUserData,
                    &eTxnStatus, &pTxnKey, &nError));
    EXPECT_EQ(SipTxn::STATUS_VALID_MESSAGE, eTxnStatus);

    pAckSipMessage->SipDelete();

    DeleteTxn(pTxnKey);

    pTxnKey->SipDelete();
    pTxnKey = SIP_NULL;

    delete pSipStackManager;
}

}  // namespace android