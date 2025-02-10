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
#include "SipUtil.h"
#include "platform/SipString.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsm.h"
#include "txn/SipTxnFsmData.h"

extern SIP_BOOL MockFsm_FetchTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**);
extern SIP_BOOL MockFsm_StartTimer(SIP_UINT32, SipTimerCallback, SIP_VOID*, SIP_VOID**);
extern SIP_BOOL MockFsm_ReleaseTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**);
extern SIP_VOID* MockFsm_CreateAckRequest(SIP_VOID*, ISipUserData*);
extern SIP_VOID MockFsm_ResetTimerCount();

namespace android
{

class SipInviteServerTxnTest : public ::testing::Test
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

        const SIP_CHAR* pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pSipMsg->Decode(pMsg, SipPf_Strlen(pMsg)));

        pRespSipMsg = new SipMessage();
        pRespSipMsg->SetMessageType(SipMessage::RESP_TYPE);

        SipStatusLine* pStatusLine = new SipStatusLine("SIP/2.0", "180", "Ringing");
        pRespSipMsg->SetStatusLine(pStatusLine);

        SipHeaderBase* pRespRSeqHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RSEQ);
        ASSERT_TRUE(pRespRSeqHdr != nullptr);
        EXPECT_EQ(SIP_TRUE, pRespRSeqHdr->Decode("2", 1));

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
        MockFsm_ResetTimerCount();
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

TEST_F(SipInviteServerTxnTest, IdleState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_IDLE_ST][SipTxn::INV_SER_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));

    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_IDLE_ST][SipTxn::INV_SER_RECV_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    pReqLine->SetMethod("BYE");
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    /* Passing Bye msg to make fetch txn fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_IDLE_ST][SipTxn::INV_SER_RECV_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pSipTranspParam;
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
}

TEST_F(SipInviteServerTxnTest, ProceedingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST][SipTxn::INV_SER_RECV_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);
    /* Passing with Sent msg transport param */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST][SipTxn::INV_SER_RECV_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;

    pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);

    /* Calling once timer to make startTimer for TimerG failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT](
                                      pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for TimerG success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_FAILURE_RESP_EVT](pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for TimerG fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_FAILURE_RESP_EVT](pTxn, pTxnFsmData, &nError));

    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);
    /* Calling once timer to make startTimer for Timer G success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_FAILURE_RESP_EVT](pTxn, pTxnFsmData, &nError));
    /* Calling again timer to make startTimer for Timer G fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_FAILURE_RESP_EVT](pTxn, pTxnFsmData, &nError));
    /* Calling once timer to make startTimer for Timer H success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST][SipTxn::INV_SER_SEND_2XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));
    /* Calling again timer to make startTimer for Timer H fail */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST][SipTxn::INV_SER_SEND_2XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST][SipTxn::INV_SER_TRANSP_ERROR_EVT](
                    pTxn, pTxnFsmData, &nError));

    /* Calling with null SipTimeoutData */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](pTxn, SIP_NULL, &nError));

    pTxn->SetMaxDuration(4000);
    pTxn->SetCurrentDuration(2000);
    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::INVITE_SERVER, SipTxn::TIMER_G, pTxnKey);
    /* Calling once timer to make startTimer for Timer G return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](
                                      pTxn, pTimeoutData, &nError));

    pTxn->SetMaxDuration(8000);
    pTxn->SetCurrentDuration(2000);
    /* Calling again timer to make startTimer for Timer G fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](
                                      pTxn, pTimeoutData, &nError));

    delete pTimeoutData;
    delete pTxnFsmData;
    delete pSipTranspParam;
    delete pSipUserData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();
}

TEST_F(SipInviteServerTxnTest, CompletedState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST][SipTxn::INV_SER_RECV_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST][SipTxn::INV_SER_TRANSP_ERROR_EVT](
                    pTxn, pTxnFsmData, &nError));

    /* Calling once timer to make startTimer for Timer I return fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST][SipTxn::INV_SER_RECV_ACK_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    /* Calling again timer to make startTimer for Timer I return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST][SipTxn::INV_SER_RECV_ACK_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST][SipTxn::INV_SER_RECV_ACK_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](pTxn, pTxnFsmData, &nError));

    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    pTxn->SetMaxDuration(4000);
    pTxn->SetCurrentDuration(2000);
    /* Calling once timer to make startTimer for Timer G return failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](pTxn, pTxnFsmData, &nError));

    pTxn->SetMaxDuration(4000);
    pTxn->SetCurrentDuration(2000);
    /* Calling again timer to make startTimer for Timer G return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](pTxn, pTxnFsmData, &nError));

    pTxn->SetMaxDuration(8000);
    pTxn->SetCurrentDuration(2000);
    /* Calling once timer to make startTimer for Timer G return fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_COMPLETED_ST]
                              [SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT](pTxn, pTxnFsmData, &nError));

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipUserData;
    delete pSipTranspParam;
}

TEST_F(SipInviteServerTxnTest, ConfirmedState)
{
    SIP_UINT16 nError = 0;

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, SIP_NULL, SIP_NULL);
    SipTxn* pTxn = new SipTxn();

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_CONFIRMED_ST][SipTxn::INV_SER_RECV_ACK_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_CONFIRMED_ST][SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT](
                    pTxn, pTxnFsmData, &nError));

    pTxn->SipDelete();
    delete pTxnFsmData;
}

TEST_F(SipInviteServerTxnTest, InvalidState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_INVALID_ST][SipTxn::INV_SER_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));
}
}  // namespace android
