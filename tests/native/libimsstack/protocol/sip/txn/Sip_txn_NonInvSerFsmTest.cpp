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

#include "SipUtil.h"
#include "txn/SipTxn.h"
#include "txn/sip_txn_fsm.h"
#include "txn/SipTxnFsmData.h"
#include "transport/SipTransportInfo.h"
#include "SipStackCallback.h"
#include "txn/SipTimeoutData.h"

extern SIP_BOOL MockFsm_FetchTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**);
extern SIP_BOOL MockFsm_StartTimer(SIP_UINT32, SipTimerCallback, SIP_VOID*, SIP_VOID**);
extern SIP_BOOL MockFsm_ReleaseTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**);

namespace android
{

class Sip_txn_NonInvSerFsmTest : public ::testing::Test
{
public:
    SipMessage* pSipMsg = SIP_NULL;

protected:
    virtual void SetUp() override
    {
        SipUtil_Construct();

        pSipMsg = new SipMessage();
        pSipMsg->SetMessageType(SipMessage::REQ_TYPE);

        char* pMsg = (char*)"REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 REGISTER\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pSipMsg->DecCompleteMsg(pMsg, strlen(pMsg)));

        static const SipStackCallbacks stTestCallbacks = {
                &MockFsm_FetchTransaction,
                &MockFsm_ReleaseTransaction,
                &MockFsm_StartTimer,
                SIP_NULL,
                SIP_NULL,
                SIP_NULL,
                SIP_NULL,
                SIP_NULL,
                SIP_NULL,
        };

        SipStackCallback_SetCallbacks(stTestCallbacks);
    }

    virtual void TearDown() override
    {
        if (pSipMsg != SIP_NULL)
        {
            pSipMsg->SipDelete();
        }
        SipUtil_Destruct();
    }
};

TEST_F(Sip_txn_NonInvSerFsmTest, NonInvSer_IdleState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST][SipTxn::NON_INV_SER_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));

    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pTxnFsmData;
    delete pTxnKey;

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    /* Calling without filling transport info so considered as reliable */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pSipTranspParam;
    delete pTxnFsmData;
    delete pTxnKey;

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("BYE"));
    pReqLine->SipDelete();

    pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, SIP_NULL);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling with Bye msg to make fetch txn return false */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pSipTranspParam;
    delete pTxnFsmData;
    delete pTxnKey;

    pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("PRACK"));
    pReqLine->SipDelete();

    SipHeaderBase* pRAckHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RACK);
    ASSERT_TRUE(pRAckHdr != nullptr);
    EXPECT_EQ(SIP_TRUE, pRAckHdr->DecodeHdr((SIP_CHAR*)"2 1 INVITE", strlen("2 1 INVITE")));

    EXPECT_EQ(SIP_TRUE, pSipMsg->SetHeader(pRAckHdr));
    pRAckHdr->SipDelete();

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pSipUserData;
    delete pSipTranspParam;
    delete pTxnFsmData;
    delete pTxnKey;
}

TEST_F(Sip_txn_NonInvSerFsmTest, NonInvSer_TryingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_TRYING_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_TRYING_ST]
                                 [SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    /* Calling once to make startTimer for timer J return failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_TRYING_ST]
                                 [SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for timer J return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_TRYING_ST]
                                 [SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxnKey;
    delete pTxn;
    delete pTxnFsmData;
    delete pSipTranspParam;

    pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_TRYING_ST]
                                 [SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_TRYING_ST]
                                 [SipTxn::NON_INV_SER_TRANSP_ERROR_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;
    delete pSipTranspParam;
    delete pSipUserData;
    delete pTxnKey;
    delete pTxn;
}

TEST_F(Sip_txn_NonInvSerFsmTest, NonInvSer_ProceedingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_PROCEEDING_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_PROCEEDING_ST]
                                 [SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));
    /* Calling once to make startTimer for timer J return failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_PROCEEDING_ST]
                                 [SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));
    /* Calling again to make startTimer for timer J return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_PROCEEDING_ST]
                                 [SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxnKey;
    delete pTxn;
    delete pTxnFsmData;
    delete pSipTranspParam;

    pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    /* Calling to with TCP transport info */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_PROCEEDING_ST]
                                 [SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_PROCEEDING_ST]
                                 [SipTxn::NON_INV_SER_TRANSP_ERROR_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxnKey;
    delete pTxn;
    delete pTxnFsmData;
    delete pSipUserData;
    delete pSipTranspParam;
}

TEST_F(Sip_txn_NonInvSerFsmTest, NonInvSer_CompletedState)
{
    SIP_UINT16 nError = 0;

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, SIP_NULL, SIP_NULL);
    SipTxn* pTxn = new SipTxn();

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_COMPLETED_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_COMPLETED_ST]
                                 [SipTxn::NON_INV_SER_TRANSP_ERROR_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_COMPLETED_ST]
                                 [SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pTxnFsmData;
}

TEST_F(Sip_txn_NonInvSerFsmTest, NonInvSer_InvalidState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_INVALID_ST][SipTxn::NON_INV_SER_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));
}

}  // namespace android
