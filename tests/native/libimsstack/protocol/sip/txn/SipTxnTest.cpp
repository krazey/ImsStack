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
#include "SipVector.h"
#include "include/MockISipTransactionCallback.h"
#include "include/MockSipTransaction.h"
#include "platform/SipString.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsm.h"
#include "txn/SipTxnFsmData.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

SipVector<MockSipTransaction*> objTxnList;

class SipTxnTest : public ::testing::Test
{
public:
    SipMessage* pSipMsg = SIP_NULL;
    MockISipTransactionCallback* pMockISipTransactionCallback;
    static constexpr SIP_INT32 TIMER_ID = 1;

protected:
    virtual void SetUp() override
    {
        pMockISipTransactionCallback = new MockISipTransactionCallback();
        SipUtil::GetInstance()->SetTransactionCallback(pMockISipTransactionCallback);

        ON_CALL(*pMockISipTransactionCallback, StartTimer(_, _, _))
                .WillByDefault(Return(static_cast<void*>(const_cast<SIP_INT32*>(&TIMER_ID))));

        ON_CALL(*pMockISipTransactionCallback, FetchTransaction(_, _, _))
                .WillByDefault(Invoke(
                        [](IN SipTxnKey* pTxnKey, IN SIP_INT32 nOption, OUT SipTxn*& pOutTxn)
                        {
                            if (nOption == SipTxn::OPT_CREATE)
                            {
                                MockSipTransaction* pMockTxn =
                                        new MockSipTransaction(pTxnKey, pOutTxn);
                                objTxnList.Add(pMockTxn);
                                return SIP_TRUE;
                            }
                            else
                            {
                                SIP_UINT32 nSize = objTxnList.GetSize();

                                for (SIP_UINT32 i = 0; i < nSize; i++)
                                {
                                    MockSipTransaction* pMockTxn = objTxnList.GetAt(i);
                                    SipTxn* pTxn = pMockTxn->GetTxn();

                                    if (pTxn != SIP_NULL)
                                    {
                                        if (pTxnKey->CompareKeys(pTxn->GetTxnKey()) == SIP_MATCHES)
                                        {
                                            pOutTxn = pTxn;
                                            return SIP_TRUE;
                                        }
                                    }
                                }
                                return SIP_FALSE;
                            }
                        }));

        ON_CALL(*pMockISipTransactionCallback, ReleaseTransaction(_, _, _, _))
                .WillByDefault(Invoke(
                        [](SipTxnKey* pTxnKey, SIP_INT32, SipTxnKey*& pOutTxnKey, SipTxn*& pOutTxn)
                        {
                            for (SIP_UINT32 i = 0; i < objTxnList.GetSize(); i++)
                            {
                                MockSipTransaction* pMockTxn = objTxnList.GetAt(i);
                                SipTxn* pTxn = pMockTxn->GetTxn();

                                if (pTxn != SIP_NULL)
                                {
                                    if (pTxnKey->CompareKeys(pTxn->GetTxnKey()) == SIP_MATCHES)
                                    {
                                        pOutTxnKey = pMockTxn->GetKey();
                                        pOutTxn = pTxn;

                                        delete pMockTxn;
                                        objTxnList.RemoveAt(i);
                                        return SIP_TRUE;
                                    }
                                }
                            }
                            return SIP_TRUE;
                        }));

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
    }

    virtual void TearDown() override
    {
        if (pSipMsg != SIP_NULL)
        {
            pSipMsg->SipDelete();
        }
        if (pMockISipTransactionCallback != SIP_NULL)
        {
            delete pMockISipTransactionCallback;
            pMockISipTransactionCallback = SIP_NULL;
        }
        SipUtil::DestroyInstance();
    }
};

TEST_F(SipTxnTest, InvokeFsm_NonInviteClient)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTimerContext* pSipTxnTimerContext = new SipTimerContext();

    /* Calling NonInvCli Fsm in idle state with send NonInvReq event
       timer E will be started and txn will move to trying state */
    SipTxn* pTxn =
            new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::NON_INV_CLI_TRYING_ST, pTxn->GetTxnState());
    pTxn->SetMaxDuration(4000);

    /* Invoking timeout callback for Timer E */
    SipTimeoutData* pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_E);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::NON_INV_CLI_TRYING_ST, pTxn->GetTxnState());

    /* Calling NonInvCli Fsm in trying state with 2xx_6xx recv event
       timer K will be started and txn will move to completed state */
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::NON_INV_CLI_COMPLETED_ST, pTxn->GetTxnState());

    /* Invoking timeout callback for Timer K */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_K);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::NON_INV_CLI_TERMINATED_ST, pTxn->GetTxnState());

    /* start invalid timer */
    EXPECT_EQ(SIP_TRUE, pTxn->StartTxnTimer(SipTxn::TIMER_TYPE_INVALID, 1000, &nError));
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxn->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);

    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::NON_INV_CLI_TRYING_ST, pTxn->GetTxnState());

    /* Invoking timeout callback for Timer F */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_F);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::NON_INV_CLI_TERMINATED_ST, pTxn->GetTxnState());

    delete pSipTranspParam;
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
}

TEST_F(SipTxnTest, InvokeFsm_InviteClient)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTimerContext* pSipTxnTimerContext = new SipTimerContext();
    SipTxn* pTxn =
            new SipTxn(SipTxn::INVITE_CLIENT, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);

    /* Calling Inv cli fsm with send invite event in UDP timer A will start */
    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_CLI_SEND_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_CLI_CALLING_ST, pTxn->GetTxnState());

    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Invoking timeout callback for Timer A */
    SipTimeoutData* pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_A);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::INV_CLI_CALLING_ST, pTxn->GetTxnState());

    /* start invalid timer */
    EXPECT_EQ(SIP_TRUE, pTxn->StartTxnTimer(SipTxn::TIMER_TYPE_INVALID, 1000, &nError));
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_TYPE_INVALID);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    /* Calling Inv cli fsm with 3xx_6xx event timer D will start */
    EXPECT_EQ(
            SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_CLI_COMPLETED_ST, pTxn->GetTxnState());

    /* Invoking timeout callback for Timer D */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_D);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::INV_CLI_TERMINATED_ST, pTxn->GetTxnState());

    delete pSipTranspParam;
    delete pTxnFsmData;
    pTxn->SipDelete();

    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_TCP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxn = new SipTxn(SipTxn::INVITE_CLIENT, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);

    /* Calling Inv cli fsm with send invite event in UDP timer B will start */
    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_CLI_SEND_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_CLI_CALLING_ST, pTxn->GetTxnState());

    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_B);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    EXPECT_EQ(SIP_FALSE, pTxn->InvokeFsm(SipTxn::INV_CLI_INVALID_EVT, pTxnFsmData, &nError));

    delete pSipTranspParam;
    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    delete pSipTxnTimerContext;
}

TEST_F(SipTxnTest, InvokeFsm_InviteServer)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTimerContext* pSipTxnTimerContext = new SipTimerContext();

    SipTxn* pTxn =
            new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);
    /* Calling Inv ser fsm with recv invite event state will be moved to proceeding state*/
    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_SER_RECV_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_PROCEEDING_ST, pTxn->GetTxnState());

    /* Calling Inv ser fsm with send RPR resp to start timer G */
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_PROCEEDING_ST, pTxn->GetTxnState());
    pTxn->SetMaxDuration(4000);

    /* Invoking timeout callback for Timer G */
    SipTimeoutData* pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_G);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::INV_SER_PROCEEDING_ST, pTxn->GetTxnState());

    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Calling Inv ser fsm send 2xx resp event and timer L will be started
        state will be moved to completed state */
    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_SER_SEND_2XX_RESP_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_ACCEPTED_ST, pTxn->GetTxnState());
    pTxn->SetMaxDuration(140000);

    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_SER_RECV_ACK_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_ACCEPTED_ST, pTxn->GetTxnState());

    /* Invoking timeout callback for Timer L */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_L);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::INV_SER_TERMINATED_ST, pTxn->GetTxnState());

    /* Calling timeout with not matching TxnKey */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey());
    pTimeoutData->SetTimerType(SipTxn::TIMER_L);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::INV_SER_TERMINATED_ST, pTxn->GetTxnState());

    /* start invalid timer in Inv Ser txn and  invoking timeout with invalid timer type*/
    EXPECT_EQ(SIP_TRUE, pTxn->StartTxnTimer(SipTxn::TIMER_TYPE_INVALID, 1000, &nError));
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_TYPE_INVALID);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    delete pSipTranspParam;
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    delete pSipTxnTimerContext;

    pSipUserData = new ISipUserData(SIP_NULL);
    pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pSipTxnTimerContext = new SipTimerContext();

    pTxn = new SipTxn(SipTxn::INVITE_SERVER, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);
    /* Calling Inv ser fsm with recv invite event state will be moved to proceeding state*/
    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_SER_RECV_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_PROCEEDING_ST, pTxn->GetTxnState());

    EXPECT_EQ(
            SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_SER_SEND_FAILURE_RESP_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_COMPLETED_ST, pTxn->GetTxnState());

    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);
    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);

    /* Calling Inv ser fsm with recv ACK in completed state
       event in UDP timer I will start */
    EXPECT_EQ(SIP_TRUE, pTxn->InvokeFsm(SipTxn::INV_SER_RECV_ACK_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::INV_SER_CONFIRMED_ST, pTxn->GetTxnState());

    /* Invoking timeout callback for Timer I */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_I);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::INV_SER_TERMINATED_ST, pTxn->GetTxnState());

    delete pSipTranspParam;
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    delete pSipTxnTimerContext;
}

TEST_F(SipTxnTest, InvokeFsm_NonInviteServer)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTimerContext* pSipTxnTimerContext = new SipTimerContext();

    SipTxn* pTxn =
            new SipTxn(SipTxn::NON_INVITE_SERVER, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);
    /* Calling Non Inv ser fsm with recv non invite event state will be moved to trying state*/
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::NON_INV_SER_TRYING_ST, pTxn->GetTxnState());

    /* Start invalid timer */
    EXPECT_EQ(SIP_TRUE, pTxn->StartTxnTimer(SipTxn::TIMER_TYPE_INVALID, 1000, &nError));
    SipTimeoutData* pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_TYPE_INVALID);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    /* Calling Non Inv ser fsm with recv 2xx resp event in trying state
       in UDP timer J will start */
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT, pTxnFsmData, &nError));
    EXPECT_EQ(SipTxn::NON_INV_SER_COMPLETED_ST, pTxn->GetTxnState());

    /* Invoking timeout callback for Timer J */
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    pTimeoutData->SetTimerType(SipTxn::TIMER_J);
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());
    EXPECT_EQ(SipTxn::NON_INV_SER_TERMINATED_ST, pTxn->GetTxnState());

    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    /* Invoking Timeout with invalid timerID */
    SIP_INT32 nTimerId = 0;
    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    CbkTxnTimeout(pTimeoutData, &nTimerId);

    delete pSipTranspParam;
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    delete pSipTxnTimerContext;
}

TEST_F(SipTxnTest, StartTxnTimer)
{
    SIP_UINT16 nError = 0;
    SipTxn* pTxn = new SipTxn();

    EXPECT_EQ(SIP_TRUE, pTxn->StartTxnTimer(SipTxn::TIMER_G, 500, &nError));
    ASSERT_TRUE(pTxn->GetTimerId() != nullptr);
    EXPECT_EQ(SIP_TRUE, pTxn->StopTxnTimer());

    pTxn->SipDelete();
}

TEST_F(SipTxnTest, PrepareACK)
{
    SIP_UINT16 nError = 0;
    SipMessage* pOutMsg = SIP_NULL;

    SipMessage* pRespSipMsg = new SipMessage();
    pRespSipMsg->SetMessageType(SipMessage::RESP_TYPE);

    SipMessage* pInSipMsg = new SipMessage();
    pInSipMsg->SetMessageType(SipMessage::REQ_TYPE);

    const SIP_CHAR* pMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 REGISTER\r\n\
Route: <pcscf>\r\n\
P-Access-Network-Info: 3GPP-UTRAN;utran-cell-id-3gpp=B20E\r\n\
User-Agent: pixel\r\n\
\r\n";

    EXPECT_EQ(SIP_TRUE, pInSipMsg->Decode(pMsg, SipPf_Strlen(pMsg)));
    SipTxn* pTxn = new SipTxn(SipTxn::NON_INVITE_SERVER, SIP_NULL, pInSipMsg, SIP_NULL, &nError);

    pMsg = "SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";

    EXPECT_EQ(SIP_TRUE, pRespSipMsg->Decode(pMsg, SipPf_Strlen(pMsg)));
    EXPECT_EQ(SIP_TRUE, pTxn->PrepareACK(pRespSipMsg, SIP_TRUE, &pOutMsg));

    pInSipMsg->SipDelete();
    pTxn->SipDelete();
    pRespSipMsg->SipDelete();
}

TEST_F(SipTxnTest, SetUserData)
{
    SipTxn* pTxn = new SipTxn();

    pTxn->SetUserData(new ISipUserData());
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    EXPECT_EQ(SipConfiguration::MSG_OPT_ENCODE_NONE, pSipUserData->GetMsgOptions());
    pSipUserData->SetMsgOptions(SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE);
    EXPECT_EQ(SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE, pSipUserData->GetMsgOptions());
    EXPECT_EQ(SIP_FALSE, pSipUserData->GetDeleteFlag());
    pTxn->SetUserData(pSipUserData);

    pTxn->SipDelete();
}

TEST_F(SipTxnTest, IsTxnTerminated)
{
    SIP_UINT16 nError = 0;

    SipTxn* pTxn = new SipTxn(SipTxn::NON_INVITE_CLIENT, SIP_NULL, pSipMsg, SIP_NULL, &nError);
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    EXPECT_EQ(SIP_TRUE, pTxn->IsTxnTerminated());

    pTxn->SipDelete();
    pTxn = new SipTxn(SipTxn::INVITE_CLIENT, SIP_NULL, pSipMsg, SIP_NULL, &nError);
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);
    EXPECT_EQ(SIP_TRUE, pTxn->IsTxnTerminated());

    pTxn->SipDelete();
    pTxn = new SipTxn(SipTxn::INVITE_SERVER, SIP_NULL, pSipMsg, SIP_NULL, &nError);
    pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    EXPECT_EQ(SIP_TRUE, pTxn->IsTxnTerminated());

    pTxn->SipDelete();
    pTxn = new SipTxn(SipTxn::NON_INVITE_SERVER, SIP_NULL, pSipMsg, SIP_NULL, &nError);
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    EXPECT_EQ(SIP_TRUE, pTxn->IsTxnTerminated());

    pTxn->SipDelete();
    pTxn = new SipTxn(SipTxn::INVALID, SIP_NULL, pSipMsg, SIP_NULL, &nError);
    EXPECT_EQ(SIP_FALSE, pTxn->IsTxnTerminated());

    pTxn->SipDelete();
}

TEST_F(SipTxnTest, InvalidTxn)
{
    SIP_UINT16 nError = 0;
    SipMessage* pInSipMsg = new SipMessage();
    pInSipMsg->SetMessageType(SipMessage::TYPE_INVALID);
    SipTxnKey* pTxnKey = new SipTxnKey(pInSipMsg, &nError);
    EXPECT_EQ(SipTxn::INVALID, pTxnKey->GetTxnType());

    SipTxn* pTxn = new SipTxn(SipTxn::INVALID, pTxnKey, pSipMsg, SIP_NULL, &nError);

    EXPECT_EQ(SIP_FALSE, pTxn->InvokeFsm(SipTxn::NON_INV_CLI_INVALID_EVT, SIP_NULL, &nError));

    SipTimeoutData* pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    CbkTxnTimeout(pTimeoutData, pTxn->GetTimerId());

    pTimeoutData = new SipTimeoutData();
    pTimeoutData->SetTxnKey(new SipTxnKey(pTxn->GetTxnKey(), &nError));
    CbkTxnTimeout(pTimeoutData, SIP_NULL);

    CbkTxnTimeout(SIP_NULL, SIP_NULL);

    SipTxn_RemoveFromTxnPool(SIP_NULL);
    pTxn->SipDelete();
    pTxnKey->SipDelete();
    pInSipMsg->SipDelete();
}

TEST_F(SipTxnTest, AbortTxn)
{
    SIP_UINT16 nError = 0;
    ISipUserData* pSipUserData = new ISipUserData(SIP_NULL);
    SipTransportParameter* pSipTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    SipTxnFsmData* pTxnFsmData = new SipTxnFsmData(pSipMsg, pSipTranspParam, pSipUserData);
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);
    SipTimerContext* pSipTxnTimerContext = new SipTimerContext();

    /* Calling NonInvCli Fsm in idle state with send NonInvReq event
       timer E will be started and txn will move to trying state */
    SipTxn* pTxn =
            new SipTxn(SipTxn::NON_INVITE_CLIENT, pTxnKey, pSipMsg, pSipTxnTimerContext, &nError);
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SipTransportParameter* pSipSendTranspParam =
            new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

    pTranspInfo->SetMsgSentTranspParam(pSipSendTranspParam);
    pTxn->UpdateTranspInfo(pTranspInfo);
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT, pTxnFsmData, &nError));

    /* Aborting txn before timeout */
    EXPECT_EQ(SIP_TRUE, pTxn->AbortTxn());

    /* Aborting txn without SipUtil */
    EXPECT_EQ(SIP_TRUE,
            pTxn->InvokeFsm(SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT, pTxnFsmData, &nError));

    delete pSipTranspParam;
    pTxn->RemoveFromTxnPool();
    pTxn->SipDelete();
    delete pSipUserData;
    delete pTxnFsmData;
    pTxnKey->SipDelete();
    delete pSipTxnTimerContext;
}
}  // namespace android
