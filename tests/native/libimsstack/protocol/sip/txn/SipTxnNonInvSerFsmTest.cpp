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
#include "txn/SipTxnUtil.h"

extern SIP_BOOL MockFsm_FetchTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**);
extern SIP_BOOL MockFsm_StartTimer(SIP_UINT32, SipTimerCallback, SIP_VOID*, SIP_VOID**);
extern SIP_BOOL MockFsm_ReleaseTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**);
extern SIP_VOID MockFsm_ResetTimerCount();

namespace android
{

class SipTxnNonInvSerFsmTest : public ::testing::Test
{
public:
    SipMessage* pSipMsg = SIP_NULL;

protected:
    virtual void SetUp() override
    {
        SipUtil_Construct();

        pSipMsg = new SipMessage();
        pSipMsg->SetMessageType(SipMessage::REQ_TYPE);

        const SIP_CHAR* pMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 REGISTER\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

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
        MockFsm_ResetTimerCount();
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

TEST_F(SipTxnNonInvSerFsmTest, NonInvSer_IdleState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST][SipTxn::NON_INV_SER_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));

    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    /* Calling without filling transport info so considered as reliable */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    pReqLine->SetMethod("BYE");
    pReqLine->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, SIP_NULL);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling with Bye msg to make fetch txn return false */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));
    pTxn->SipDelete();
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    SipMessage* pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::RESP_TYPE);

    const SIP_CHAR* pMsg = "SIP/2.0 183 Ringing\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=too\r\n\
Call-ID: 13459809802\r\n\
CSeq: 1 INVITE\r\n\
RSeq: 2\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);

    /*Calling Invite Server with 183 msg in proceeding state to add RPR txn key in SipTxnUtil */
    pTxnKey = new SipTxnKey(pTempSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::INV_SER_TXN, pTxnKey, pTempSipMsg, SIP_NULL, &nError);
    EXPECT_EQ(SIP_FALSE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT](
                                      pTxn, pTxnFsmData, &nError));
    EXPECT_EQ(SIP_TRUE,
            gpfSipInvSerTxnFsm[SipTxn::INV_SER_PROCEEDING_ST]
                              [SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT](
                                      pTxn, pTxnFsmData, &nError));
    pTxn->SipDelete();
    pTempSipMsg->SipDelete();
    pTxnKey->SipDelete();
    delete pTxnFsmData;

    pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::RESP_TYPE);

    pMsg = "PRACK sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=too\r\n\
Call-ID: 13459809802\r\n\
CSeq: 2 PRACK\r\n\
RAck: 562 1 INVITE\r\n\
\r\n";

    EXPECT_EQ(SIP_TRUE, pTempSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

    nError = 0;
    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pTempSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pTempSipMsg, SIP_NULL, &nError);
    /* Calling fsm with PRACK msg on idle state to not match with above 183 message */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    pTempSipMsg->SipDelete();
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::RESP_TYPE);

    pMsg = "PRACK sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=too\r\n\
Call-ID: 13459809802\r\n\
CSeq: 2 PRACK\r\n\
RAck: 2 1 INVITE\r\n\
\r\n";

    EXPECT_EQ(SIP_TRUE, pTempSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

    nError = 0;
    pTxnFsmData = new SipTxnFsmData(pTempSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pTempSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pTempSipMsg, SIP_NULL, &nError);

    /* Calling fsm with PRACK msg on idle state to match with above 183 message */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_IDLE_ST]
                                 [SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT](
                                         pTxn, pTxnFsmData, &nError));

    /* Calling Search txn key with null */
    SipTxnUtil::SearchTxnKey(SIP_NULL, SIP_FALSE);
    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipUserData;
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    pTempSipMsg->SipDelete();
}

TEST_F(SipTxnNonInvSerFsmTest, NonInvSer_TryingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

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

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipTranspParam;

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);
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
    pTxnKey->SipDelete();
    pTxn->SipDelete();
}

TEST_F(SipTxnNonInvSerFsmTest, NonInvSer_ProceedingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INV_SER_TXN, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

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

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipTranspParam;

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);
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

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipUserData;
    delete pSipTranspParam;
}

TEST_F(SipTxnNonInvSerFsmTest, NonInvSer_CompletedState)
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

    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::NON_INV_SER_TXN, SipTxn::TIMER_J, SIP_NULL);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_COMPLETED_ST]
                                 [SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT](
                                         pTxn, pTimeoutData, &nError));
    delete pTimeoutData;
    pTxn->SipDelete();
    delete pTxnFsmData;
}

TEST_F(SipTxnNonInvSerFsmTest, NonInvSer_InvalidState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_INVALID_ST][SipTxn::NON_INV_SER_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));
}

}  // namespace android
