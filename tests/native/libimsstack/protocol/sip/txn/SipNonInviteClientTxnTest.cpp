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
extern SIP_VOID MockFsm_ResetTimerCount();

namespace android
{

class SipNonInviteClientTxnTest : public ::testing::Test
{
public:
    SipMessage* pSipMsg = SIP_NULL;

protected:
    virtual void SetUp() override
    {
        SipUtil::GetInstance();

        pSipMsg = new SipMessage();
        pSipMsg->SetMessageType(SipMessage::REQ_TYPE);

        const SIP_CHAR* pMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 REGISTER\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pSipMsg->Decode(pMsg, SipPf_Strlen(pMsg)));

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
        SipUtil::DestroyInstance();
    }
};

TEST_F(SipNonInviteClientTxnTest, IdleState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_IDLE_ST][SipTxn::NON_INV_CLI_INVALID_EVT](
                    SIP_NULL, SIP_NULL, SIP_NULL));

    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, SIP_NULL);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);

    /* Calling with null SipUserData */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_IDLE_ST]
                                    [SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_IDLE_ST]
                                    [SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTxn->SipDelete();
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    pReqLine->SetMethod("CANCEL");
    pReqLine->SipDelete();

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    /* Calling once timer to make startTimer for Timer F return fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_IDLE_ST]
                                    [SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT](
                                            pTxn, pTxnFsmData, &nError));
    /* Calling again timer to make startTimer for Timer F return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_IDLE_ST]
                                    [SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipUserData;
    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
}

TEST_F(SipNonInviteClientTxnTest, TryingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::NON_INVITE_CLIENT, SipTxn::TIMER_E, pTxnKey);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));

    pTxn->SetMaxDuration(4000);
    pTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));
    delete pTimeoutData;

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    /* Calling once timer to make startTimer for Timer K return fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));
    /* Calling to make Timer K return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pTxn->UpdateTranspInfo(pTranspInfo);
    /* Calling with null transport info */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::INV_CLI_CALLING_ST]
                                    [SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT](
                                            pTxn, pTxnFsmData, &nError));

    delete pTxnFsmData;
    delete pSipTranspParam;
    delete pSipUserData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();
}

TEST_F(SipNonInviteClientTxnTest, ProceedingState)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::NON_INVITE_CLIENT, SipTxn::TIMER_E, pTxnKey);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));
    pTxn->SetMaxDuration(4000);
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_PROCEEDING_ST);
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));
    pTxn->SetMaxDuration(4000);
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_PROCEEDING_ST);
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));
    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pSipTranspParam;
    delete pTimeoutData;

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTimeoutData = new SipTimeoutData(SipTxn::NON_INVITE_CLIENT, SipTxn::TIMER_F, pTxnKey);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));
    delete pTimeoutData;
    pTxnKey->SipDelete();
    pTxn->SipDelete();

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pTxn->UpdateTranspInfo(pTranspInfo);
    /* Calling with null transport info */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipTranspParam;

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);
    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Calling timer to make startTimer for Timer K return fail */
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));
    /* Calling timer to make startTimer for Timer K return success */
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));
    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_PROCEEDING_ST]
                                    [SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT](
                                            pTxn, pTxnFsmData, &nError));

    pTxnKey->SipDelete();
    pTxn->SipDelete();
    delete pTxnFsmData;
    delete pSipUserData;
    delete pSipTranspParam;
}

TEST_F(SipNonInviteClientTxnTest, CompletedState)
{
    SIP_UINT16 nError = 0;

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, SIP_NULL, SIP_NULL);
    SipTxn* pTxn = new SipTxn();

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_COMPLETED_ST]
                                    [SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_COMPLETED_ST]
                                    [SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT](
                                            pTxn, pTxnFsmData, &nError));

    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_COMPLETED_ST]
                                    [SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT](
                                            pTxn, pTxnFsmData, &nError));

    SipTimeoutData* pTimeoutData =
            new SipTimeoutData(SipTxn::NON_INVITE_CLIENT, SipTxn::TIMER_K, SIP_NULL);
    EXPECT_EQ(SIP_TRUE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_COMPLETED_ST]
                                    [SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT](
                                            pTxn, pTimeoutData, &nError));
    delete pTimeoutData;
    pTxn->SipDelete();
    delete pTxnFsmData;
}

TEST_F(SipNonInviteClientTxnTest, InvalidState)
{
    EXPECT_EQ(SIP_FALSE,
            gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_INVALID_ST]
                                    [SipTxn::NON_INV_CLI_INVALID_EVT](
                                            SIP_NULL, SIP_NULL, SIP_NULL));
}

}  // namespace android
