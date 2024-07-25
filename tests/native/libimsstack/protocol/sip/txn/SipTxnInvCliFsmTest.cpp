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

class SipTxnInvCliFsmTest : public ::testing::Test
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
        EXPECT_EQ(SIP_TRUE, pSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

        pRespSipMsg = new SipMessage();
        pRespSipMsg->SetMessageType(SipMessage::RESP_TYPE);

        pMsg = "SIP/2.0 183 Ringing\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=too\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pRespSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

        SipHeaderBase* pRespRSeqHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RSEQ);
        ASSERT_TRUE(pRespRSeqHdr != nullptr);
        EXPECT_EQ(SIP_TRUE, pRespRSeqHdr->DecodeHdr("2", 1));

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

TEST_F(SipTxnInvCliFsmTest, InvCli_IdleState)
{
    SIP_UINT16 nError = 0;
    /* Calling with all null values */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_INVALID_EVT](
                    SIP_NULL, SIP_NULL, &nError));

    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
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

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling once to make startTimer for timer B fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_IDLE_ST][SipTxn::INV_CLI_SEND_INV_REQ_EVT](
                    pTxn, pTxnFsmData, &nError));
    pTxnKey->SipDelete();
    pTxn->SipDelete();

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

    pTxn->SipDelete();
    delete pSipUserData;
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
}

TEST_F(SipTxnInvCliFsmTest, InvCli_CallingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::INV_CLI_TXN, SipTxn::TIMER_A, pTxnKey);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTimeoutData, &nError));

    pTxn->SetMaxDuration(8000);
    pTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);
    /* Increase max duration to continue retransmission timer A
       calling starttimer once to make timer A return failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTimeoutData, &nError));

    pTxn->SetMaxDuration(4000);
    pTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);
    /* Increase max duration to continue retransmission timer A
       calling starttimer again to make timer A return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTimeoutData, &nError));

    delete pTimeoutData;
    delete pSipTranspParam;
    pTxnKey->SipDelete();
    pTxn->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    pTimeoutData = new SipTimeoutData(SipTxn::INV_CLI_TXN, SipTxn::TIMER_B, pTxnKey);
    /* Calling with TCP transport info */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT](
                                         pTxn, pTimeoutData, &nError));
    delete pTimeoutData;

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST][SipTxn::INV_CLI_RECV_2XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Calling once to make startTimer for timer D failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for timer D success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST][SipTxn::INV_CLI_TRANSP_ERROR_EVT](
                    pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;
    delete pSipTranspParam;
    delete pSipUserData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();
}

TEST_F(SipTxnInvCliFsmTest, InvCli_ProceedingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pRespSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::INV_CLI_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    /*Calling the retransmission 183 with different callID */
    SipMessage* pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::RESP_TYPE);

    const SIP_CHAR* pMsg = "SIP/2.0 183 Ringing\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=too\r\n\
Call-ID: 1332\r\n\
CSeq: 1 INVITE\r\n\
RSeq: 2\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    delete pTxnFsmData;
    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);

    /*Calling the 183 msg in proceeding state*/
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    /*Calling the 183 retransmission with different from tag */
    SipHeaderBase* pFromHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::FROM);
    ASSERT_TRUE(pFromHdr != nullptr);
    SipHeaderBase* pCallIDHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CALL_ID);
    ASSERT_TRUE(pCallIDHdr != nullptr);

    const SIP_CHAR* pCallIdValue = "1332a-3c0d31@2409:192.168.35.156";
    const SIP_CHAR* pFromValue = "<sip:user@host>;tag=a89";

    EXPECT_EQ(SIP_TRUE, pFromHdr->DecodeHdr(pFromValue, SipPf_Strlen(pFromValue)));
    EXPECT_EQ(SIP_TRUE, pCallIDHdr->DecodeHdr(pCallIdValue, SipPf_Strlen(pCallIdValue)));

    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pFromHdr));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pCallIDHdr));

    pFromHdr->SipDelete();
    pCallIDHdr->SipDelete();

    delete pTxnFsmData;
    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    /*Calling the 183 retransmission msg with different to tag */
    pFromHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::FROM);
    ASSERT_TRUE(pFromHdr != nullptr);
    SipHeaderBase* pToHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TO);
    ASSERT_TRUE(pToHdr != nullptr);

    const SIP_CHAR* pToValue = "<sip:userA@host>;tag=one";
    pFromValue = "<sip:user@host>;tag=abcd";

    EXPECT_EQ(SIP_TRUE, pFromHdr->DecodeHdr(pFromValue, SipPf_Strlen(pFromValue)));
    EXPECT_EQ(SIP_TRUE, pToHdr->DecodeHdr(pToValue, SipPf_Strlen(pToValue)));

    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pFromHdr));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pToHdr));

    pFromHdr->SipDelete();
    pToHdr->SipDelete();

    delete pTxnFsmData;
    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    /*Calling the retransmission 183 with different Rseq num */
    pToHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TO);
    ASSERT_TRUE(pToHdr != nullptr);
    SipHeaderBase* pRSeqHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RSEQ);
    ASSERT_TRUE(pRSeqHdr != nullptr);

    pToValue = "<sip:userA@host>;tag=too";
    EXPECT_EQ(SIP_TRUE, pToHdr->DecodeHdr(pToValue, SipPf_Strlen(pToValue)));
    EXPECT_EQ(SIP_TRUE, pRSeqHdr->DecodeHdr("90", 1));

    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pToHdr));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pRSeqHdr));

    pRSeqHdr->SipDelete();
    pToHdr->SipDelete();

    delete pTxnFsmData;
    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_1XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST][SipTxn::INV_CLI_RECV_2XX_RESP_EVT](
                    pTxn, pTxnFsmData, &nError));

    /* Calling once to make startTimer for timer D failure */
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));

    /* Calling again to make startTimer for timer D success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_PROCEEDING_ST]
                                 [SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT](
                                         pTxn, pTxnFsmData, &nError));
    pTxnKey->SipDelete();
    pTxn->SipDelete();
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

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipUserData;
    delete pSipTranspParam;
    pTempSipMsg->SipDelete();
}

TEST_F(SipTxnInvCliFsmTest, InvCli_CompletedState)
{
    SIP_UINT16 nError = 0;
    SipTxn* pTxn = new SipTxn();
    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::INV_CLI_TXN, SipTxn::TIMER_D, SIP_NULL);

    EXPECT_EQ(SIP_TRUE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_COMPLETED_ST]
                                 [SipTxn::INV_CLI_TIMERD_TIME_OUT_EVT](
                                         pTxn, pTimeoutData, &nError));
    delete pTimeoutData;

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, SIP_NULL, SIP_NULL);
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

    pTxn->SipDelete();
    delete pTxnFsmData;
}

TEST_F(SipTxnInvCliFsmTest, InvCli_InvalidState)
{
    SIP_UINT16 nError = 0;
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvClientTxnFsm[SipTxn::INV_CLI_INVALID_ST][SipTxn::INV_CLI_INVALID_EVT](
                    SIP_NULL, SIP_NULL, &nError));
}
}  // namespace android
