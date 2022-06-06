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
extern SIP_VOID* MockFsm_CreateAckRequest(SIP_VOID*, ISipUserData*);

namespace android
{

class Sip_txn_InvCliFsmTest : public ::testing::Test
{
public:
    SipMessage* pSipMsg = SIP_NULL;
    SipMessage* pRespSipMsg = SIP_NULL;

protected:
    virtual void SetUp() override
    {
        SipUtil_Construct();

        pSipMsg = new SipMessage();
        pSipMsg->SetMessageType(SipMessage::REQ_TYPE);
        char* pMsg = (char*)"INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pSipMsg->DecCompleteMsg(pMsg, strlen(pMsg)));

        pRespSipMsg = new SipMessage();
        pRespSipMsg->SetMessageType(SipMessage::RESP_TYPE);

        SipStatusLine* pStatusLine = new SipStatusLine("SIP/2.0", "180", "Ringing");
        pRespSipMsg->SetStatusLine(pStatusLine);

        SipHeaderBase* pRespRSeqHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RSEQ);
        ASSERT_TRUE(pRespRSeqHdr != nullptr);
        EXPECT_EQ(SIP_TRUE, pRespRSeqHdr->DecodeHdr((SIP_CHAR*)"2", 1));

        EXPECT_EQ(SIP_TRUE, pRespSipMsg->SetHeader(pRespRSeqHdr));
        pRespRSeqHdr->SipDelete();

        static const SipStackCallbacks stTestCallbacks = {
                &MockFsm_FetchTransaction,
                &MockFsm_ReleaseTransaction,
                &MockFsm_StartTimer,
                SIP_NULL,
                SIP_NULL,
                &MockFsm_CreateAckRequest,
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
        if (pRespSipMsg != SIP_NULL)
        {
            pRespSipMsg->SipDelete();
        }
        SipUtil_Destruct();
    }
};

TEST_F(Sip_txn_InvCliFsmTest, InvCli_IdleState)
{
    SIP_UINT16 nError = 0;
    /* Calling with all null values */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_INVALID_EVT](
                    SIP_NULL, SIP_NULL, &nError));

    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    /* Calling once to make startTimer for timer A fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_SEND_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for timer A success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_SEND_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pSipTranspParam;
    delete pTxnFsmData;
    delete pTxnKey;

    pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling once to make startTimer for timer B fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_SEND_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pTxnKey;

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("BYE"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling with BYE msg so that fetch msg returns fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_SEND_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pSipUserData;
    delete pSipTranspParam;
    delete pTxnFsmData;
    delete pTxnKey;
}

TEST_F(Sip_txn_InvCliFsmTest, InvCli_CallingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTxnFsmData, &nError));

    pTxn->SetMaxDuration(4000);
    pTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);
    /* Increase max duration to continue retransmission timer A
       first time calling starttimer to make timer A return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTxnFsmData, &nError));

    pTxn->SetMaxDuration(8000);
    pTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);
    /* Increase max duration to continue retransmission timer A
       second time calling starttimer to make timer A return failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pSipTranspParam;
    delete pTxnFsmData;
    delete pTxnKey;
    delete pTxn;

    pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Calling with TCP transport info */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;

    pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST][SipTxn::INV_CLI_RECV_2XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Calling once to make startTimer for timer D success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for timer D failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST][SipTxn::INV_CLI_TRANSP_ERROR_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;
    delete pSipTranspParam;
    delete pSipUserData;
    delete pTxnKey;
    delete pTxn;
}

TEST_F(Sip_txn_InvCliFsmTest, InvCli_ProceedingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam = new SipTransportParameter(
            (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_2XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));
    /* Calling once to make startTimer for timer D success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));
    /* Calling again to make startTimer for timer D failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    delete pTxnKey;
    delete pTxn;
    delete pTxnFsmData;

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling without passing transport info will be considered as reliable */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_TRANSP_ERROR_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxnKey;
    delete pTxn;
    delete pTxnFsmData;
    delete pSipUserData;
    delete pSipTranspParam;
}

TEST_F(Sip_txn_InvCliFsmTest, InvCli_CompletedState)
{
    SIP_UINT16 nError = 0;

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, SIP_NULL, SIP_NULL);
    SipTxn* pTxn = new SipTxn();

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_COMPLETED_ST]
                                 [SipTxn::INV_CLI_TIMERD_TIME_OUT_EVT](pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_COMPLETED_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_COMPLETED_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_COMPLETED_ST][SipTxn::INV_CLI_TRANSP_ERROR_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxn;
    delete pTxnFsmData;
}

TEST_F(Sip_txn_InvCliFsmTest, InvCli_InvalidState)
{
    SIP_UINT16 nError = 0;
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_INVALID_ST][SipTxn::INV_CLI_INVALID_EVT](
                    SIP_NULL, SIP_NULL, &nError));
}
}  // namespace android
