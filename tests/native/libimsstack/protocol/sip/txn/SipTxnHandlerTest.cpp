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
#include "SipTxnContext.h"
#include "SipUtil.h"
#include "platform/SipString.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTimeoutData.h"
#include "txn/SipTxnHandler.h"

namespace android
{

SIP_BOOL bFromRecvTxn;
SipVector<SipTxn*> objTxnList;
SIP_BOOL Mock_FetchTransaction(
        SIP_VOID* pvTxnKey, SIP_INT32 nOption, SIP_VOID** /*ppvOutTxnKey*/, SIP_VOID** ppvTxn)
{
    if ((pvTxnKey == SIP_NULL) || (ppvTxn == SIP_NULL))
    {
        return SIP_FALSE;
    }

    if (nOption == SipTxn::OPT_CREATE)
    {
        if (SipPf_Strcmp((static_cast<SipTxnKey*>(pvTxnKey))->GetMethod(), "BYE") == 0)
        {
            return SIP_FALSE;
        }
        if ((ppvTxn != SIP_NULL) && (*ppvTxn != SIP_NULL))
        {
            objTxnList.Add((SipTxn*)*ppvTxn);
        }
        return SIP_TRUE;
    }
    else
    {
        SIP_UINT16 nError;

        SIP_INT32 eMsgType = (static_cast<SipTxnKey*>(pvTxnKey))->GetMsgType();
        switch (eMsgType)
        {
            case SipMessage::REQ_TYPE:
            {
                if (SipPf_Strcmp(((static_cast<SipTxnKey*>(pvTxnKey)))->GetMethod(), "CANCEL") == 0)
                {
                    return SIP_TRUE;
                }

                if (SipPf_Strcmp(((static_cast<SipTxnKey*>(pvTxnKey)))->GetMethod(), "UPDATE") == 0)
                {
                    SipMessage* pTempSipMsg = new SipMessage();
                    *ppvTxn = new SipTxn(SipTxn::INV_SER_TXN, static_cast<SipTxnKey*>(pvTxnKey),
                            pTempSipMsg, SIP_NULL, &nError);
                    pTempSipMsg->SipDelete();
                    return SIP_TRUE;
                }
                SIP_UINT32 nSize = objTxnList.GetSize();

                for (SIP_UINT32 i = 0; i < nSize; i++)
                {
                    SipTxn* pTxn = objTxnList.GetAt(i);

                    if (pTxn != SIP_NULL)
                    {
                        if ((static_cast<SipTxnKey*>(pvTxnKey))->CompareKeys(pTxn->GetTxnKey()) ==
                                SIP_MATCHES)
                        {
                            if (ppvTxn != SIP_NULL)
                            {
                                *ppvTxn = pTxn;
                                return SIP_TRUE;
                            }
                        }
                    }
                }
                return SIP_FALSE;
            }
            case SipMessage::TYPE_INVALID:
                (static_cast<SipTxnKey*>(pvTxnKey))->CompareKeys(SIP_NULL);
                (static_cast<SipTxnKey*>(pvTxnKey))->CompareKeysForRPR(SIP_NULL);
                return SIP_FALSE;
            case SipMessage::RESP_TYPE:
            {
                if ((((static_cast<SipTxnKey*>(pvTxnKey)))->GetRespCode() == 202) ||
                        ((static_cast<SipTxnKey*>(pvTxnKey)))->GetRespCode() == 603)
                {
                    return SIP_FALSE;
                }
                else if (((static_cast<SipTxnKey*>(pvTxnKey)))->GetRespCode() == 420)
                {
                    return SIP_TRUE;
                }
                SipMessage* pTempSipMsg;
                if (bFromRecvTxn == SIP_TRUE)
                {
                    pTempSipMsg = new SipMessage();
                    *ppvTxn = new SipTxn(SipTxn::INV_CLI_TXN, static_cast<SipTxnKey*>(pvTxnKey),
                            pTempSipMsg, SIP_NULL, &nError);
                    SipTxn* pTempTxn = (SipTxn*)*ppvTxn;
                    pTempTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);
                    pTempSipMsg->SipDelete();
                    objTxnList.Add((SipTxn*)*ppvTxn);
                    return SIP_TRUE;
                }

                SIP_UINT32 nSize = objTxnList.GetSize();

                for (SIP_UINT32 i = 0; i < nSize; i++)
                {
                    SipTxn* pTxn = objTxnList.GetAt(i);

                    if (pTxn != SIP_NULL)
                    {
                        if ((static_cast<SipTxnKey*>(pvTxnKey))->CompareKeys(pTxn->GetTxnKey()) ==
                                SIP_MATCHES)
                        {
                            if (ppvTxn != SIP_NULL)
                            {
                                if (((static_cast<SipTxnKey*>(pvTxnKey)))->GetRespCode() == 480)
                                {
                                    pTxn->SetTxnState(SipTxn::INV_SER_IDLE_ST);
                                }
                                *ppvTxn = pTxn;
                                return SIP_TRUE;
                            }
                        }
                    }
                }
                pTempSipMsg = new SipMessage();
                *ppvTxn = new SipTxn(SipTxn::INV_SER_TXN, static_cast<SipTxnKey*>(pvTxnKey),
                        pTempSipMsg, SIP_NULL, &nError);
                SipTxn* pTempTxn = (SipTxn*)*ppvTxn;
                pTempTxn->SetTxnState(SipTxn::INV_SER_PROCEEDING_ST);
                pTempSipMsg->SipDelete();
                objTxnList.Add((SipTxn*)*ppvTxn);
                return SIP_TRUE;
            }
            default:
                return SIP_TRUE;
        }
    }
}

SIP_BOOL Mock_StartTimer(SIP_UINT32, SipTimerCallback, SIP_VOID* pvData, SIP_VOID**)
{
    SipTimeoutData* pTimeoutData = reinterpret_cast<SipTimeoutData*>(pvData);
    delete pTimeoutData;
    return SIP_TRUE;
}

SIP_BOOL Mock_ReleaseTransaction(
        SIP_VOID* pvTxnKey, SIP_INT32, SIP_VOID** ppvOutTxnKey, SIP_VOID** ppvTxn)
{
    if (((static_cast<SipTxnKey*>(pvTxnKey))->GetMsgType() == SipMessage::TYPE_INVALID))
    {
        return SIP_FALSE;
    }
    SIP_UINT32 nSize = objTxnList.GetSize();

    for (SIP_UINT32 i = 0; i < nSize; i++)
    {
        SipTxn* pTxn = objTxnList.GetAt(i);

        if (pTxn != SIP_NULL)
        {
            if ((static_cast<SipTxnKey*>(pvTxnKey))->CompareKeys(pTxn->GetTxnKey()) == SIP_MATCHES)
            {
                objTxnList.RemoveAt(i);
                if (ppvTxn != SIP_NULL)
                {
                    *ppvTxn = pTxn;
                    *ppvOutTxnKey = pvTxnKey;
                    return SIP_TRUE;
                }
            }
        }
    }
    return SIP_TRUE;
}

class SipTxnHandlerTest : public ::testing::Test
{
public:
    ISipUserData* pSipUserData = SIP_NULL;
    SipTransportParameter* pSipTranspParam = SIP_NULL;
    SipMessage* pSipMsg = SIP_NULL;
    SipMessage* pRespSipMsg = SIP_NULL;
    SipTxnHandler* pTxnHandler = SIP_NULL;
    SipTxnContext* pSipTxnContext = SIP_NULL;

protected:
    virtual void SetUp() override
    {
        bFromRecvTxn = SIP_FALSE;
        SipUtil_Construct();

        pTxnHandler = new SipTxnHandler();
        pSipMsg = new SipMessage();
        pSipMsg->SetMessageType(SipMessage::REQ_TYPE);

        const SIP_CHAR* pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=11df\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

        // Response Msg
        pRespSipMsg = new SipMessage();
        pRespSipMsg->SetMessageType(SipMessage::RESP_TYPE);

        pMsg = "SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pRespSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

        pSipTxnContext = new SipTxnContext();
        pSipUserData = new ISipUserData(pSipTxnContext);

        pSipTranspParam =
                new SipTransportParameter("192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

        static const SipStackCallbacks stTestCallbacks = {
                &Mock_FetchTransaction,
                &Mock_ReleaseTransaction,
                &Mock_StartTimer,
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
        if (pRespSipMsg != SIP_NULL)
        {
            pRespSipMsg->SipDelete();
        }
        if (pSipTxnContext != SIP_NULL)
        {
            delete pSipTxnContext;
        }
        if (pSipUserData != SIP_NULL)
        {
            delete pSipUserData;
        }
        if (pSipTranspParam != SIP_NULL)
        {
            delete pSipTranspParam;
        }
        if (pTxnHandler != SIP_NULL)
        {
            delete pTxnHandler;
        }

        SipUtil_Destruct();
    }
};

TEST_F(SipTxnHandlerTest, OnSendTxn_Invite)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError = 0;

    // Send INVITE msg
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnSendTxn_NonInvite)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError = 0;

    // Send Request Msg
    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("ACK"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->DeleteTxn(pTxnKey));

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("PRACK"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->DeleteTxn(pTxnKey));

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("CANCEL"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("BYE"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("UPDATE"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnSendTxn_Invalid)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTxnInfo* pTxnInfo = SIP_NULL;
    SIP_UINT16 nError = 0;

    // Send invalid msg
    SipMessage* pTempMsg = new SipMessage();
    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pTempMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    pTempMsg->SipDelete();
}

TEST_F(SipTxnHandlerTest, OnSendTxn_ResponseMsg)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError = 0;

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    // Send 2xx Response msg
    SipStatusLine* pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("200");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("202");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("183");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("603");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("480");
    pStatusLine->SipDelete();

    SipTxnKey* pInvTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(
            SIP_TRUE, pTxnHandler->OnRecvTxn(pSipMsg, pInvTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pInvTxnKey));

    SipCSeqHeader* pCSeq = static_cast<SipCSeqHeader*>(pRespSipMsg->GetHdrObj(SipHeaderBase::CSEQ));

    if (pCSeq != IMS_NULL)
    {
        pCSeq->SetMethod("UPDATE");
        pCSeq->SipDelete();
    }

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("406");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnRecvTxn_Invalid)
{
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError;
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);

    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(SIP_NULL, pTxnKey, pSipUserData, pTxnInfo, &nError));

    SipMessage* pTempMsg = new SipMessage();
    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(pTempMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    const SIP_CHAR* pMsg = "SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    pTempMsg->SetMessageType(SipMessage::RESP_TYPE);
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    /* Calling with invalid SipMessage : without To header */
    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(pTempMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    pTempMsg->SipDelete();

    pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::RESP_TYPE);
    pMsg = "SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
To: <sip:user@host>;tag=abcd\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    /* Calling with invalid SipMessage : without From header */
    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(pTempMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    pTempMsg->SipDelete();

    pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::RESP_TYPE);

    pMsg = "SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
To: <sip:user@host>;tag=abcd\r\n\
From: <sip:user@host>;\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    /* Calling with invalid SipMessage : without Callid header */
    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(pTempMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    pTempMsg->SipDelete();

    pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::RESP_TYPE);

    pMsg = "SIP/2.0 406 Not Acceptable\r\n\
To: <sip:user@host>;tag=abcd\r\n\
From: <sip:user@host>;\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    /* Calling with invalid SipMessage : without Via header */
    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(pTempMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    pTempMsg->SipDelete();

    pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::RESP_TYPE);
    pMsg = "SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
To: <sip:user@host>;tag=abcd\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
From: <sip:user@host>;tag=abcd\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    /* Calling with invalid SipMessage : without CSeq header */
    EXPECT_EQ(
            SIP_FALSE, pTxnHandler->OnRecvTxn(pTempMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    pTempMsg->SipDelete();
    pTxnKey->SipDelete();
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnRecvTxn_Request)
{
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError;
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);

    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    pTxnKey->SetCseqNum(22);
    /* Calling with different Key by changing CSeq for key comparison to mismatch */
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    pTxnKey->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    pTxnKey->SetCseqNum(1);
    pTxnKey->SetMethod("REGISTER");

    /* Calling with different method in the sipmsg for key comparison to mismatch */
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    SipMessage* pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::REQ_TYPE);
    const SIP_CHAR* pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: callid\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    pTxnKey = new SipTxnKey(pTempMsg, &nError);
    /* Calling with different callid for key comparison to mismatch */
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    pTempMsg->SipDelete();
    pTxnKey->SipDelete();

    pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::REQ_TYPE);
    pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=tag\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    pTxnKey = new SipTxnKey(pTempMsg, &nError);
    /* Calling with different from tag for key comparison to mismatch */
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    pTempMsg->SipDelete();
    pTxnKey->SipDelete();

    pTempMsg = new SipMessage();
    pTempMsg->SetMessageType(SipMessage::REQ_TYPE);
    pMsg = "INVITE sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>;tag=990\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pTempMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));
    pTxnKey = new SipTxnKey(pTempMsg, &nError);
    EXPECT_STREQ("990", pTxnKey->GetToTag());
    EXPECT_STREQ("z9hG4bs8", pTxnKey->GetViaBranchParam());
    EXPECT_STREQ("pc33.atlanta.com", pTxnKey->GetViaHost());
    EXPECT_EQ(1, pTxnKey->GetCSeqNum());

    /* Calling with different to tag for key comparison to mismatch */
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    pTempMsg->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    /* Calling with same key for key comparison to match and terminate txn */
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("ACK"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("PRACK"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("CANCEL"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    pTxnKey->SipDelete();

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("BYE"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    pTxnKey->SipDelete();
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnRecvTxn_Response)
{
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError;
    bFromRecvTxn = SIP_TRUE;

    SipTxnKey* pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    delete pTxnInfo;

    pTxnInfo = new SipTxnInfo();
    SipMessage* pNonInvSipMsg = new SipMessage();
    pNonInvSipMsg->SetMessageType(SipMessage::RESP_TYPE);

    const SIP_CHAR* pMsg = "SIP/2.0 200 Ok\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 MESSAGE\r\n\
\r\n";

    EXPECT_EQ(SIP_TRUE, pNonInvSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

    pTxnKey = new SipTxnKey(pNonInvSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnRecvTxn(pNonInvSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    SipStatusLine* pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("603");
    pStatusLine->SipDelete();

    /* Calling with 603 response message so that in Mock_FetchTransaction return
    false. So OnRecvTxn when recv resp msg without matching txn will fail */
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("183");
    pStatusLine->SipDelete();

    SipHeaderBase* pRSeqHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RSEQ);
    ASSERT_TRUE(pRSeqHdr != nullptr);
    EXPECT_EQ(SIP_TRUE, pRSeqHdr->DecodeHdr("10", 2));
    EXPECT_EQ(SIP_TRUE, pRespSipMsg->SetHeader(pRSeqHdr));

    /* Calling with valid 183 response message */
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("420");
    pStatusLine->SipDelete();

    /* Calling with valid 420 response message so that in Mock_FetchTransaction return
    true but txn is not passed. So OnRecvTxn when recv resp msg without matching txn will fail*/
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    pTxnKey->SipDelete();

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("200");
    pStatusLine->SipDelete();
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);

    /* Calling with valid 200 resp message */
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnRecvTranspError)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError = SipTransportInfo::PROTOCOL_UDP;

    /* Calling with null SipTxnKey.
    To test if txnkey is null OnRecvTranspError will be failed */
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTranspError(nError, SIP_NULL, &nError));

    /* Calling with invalid SipTxnKey i.e.
    To test if txnkey is not valid fetch txn will be failed.
    Then OnRecvTranspError will be failed */
    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));
    pTxnKey->SipDelete();

    /* Calling with valid SipTxnKey by creating with resp msg*/
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));

    SipStatusLine* pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("420");
    pStatusLine->SipDelete();

    /* Creating txnKey with 420 response message so that in Mock_FetchTransaction return
    true but txn is not passed. Inorder to test if txn is null OnRecvTranspError will be failed */
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));
    pTxnKey->SipDelete();

    /* Calling with valid SipTxnKey by creating with INVITE req message
    first calling send txn to add txn to list.
    then called OnRecvTranspError so fetch will return valid txn */
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));

    SipMessage* pNonInvSipMsg = new SipMessage();
    pNonInvSipMsg->SetMessageType(SipMessage::REQ_TYPE);

    const SIP_CHAR* pMsg = "REGISTER sip:user@host SIP/2.0\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 REGISTER\r\n\
\r\n";
    EXPECT_EQ(SIP_TRUE, pNonInvSipMsg->DecCompleteMsg(pMsg, SipPf_Strlen(pMsg)));

    /* Calling with valid SipTxnKey by creating with NON INVITE req message
    first calling send txn to add txn to list.
    then called OnRecvTranspError so fetch will return valid txn */
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pNonInvSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));

    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));

    /* Calling with valid SipTxnKey by creating with NON INVITE req message
    first calling recv txn to add txn to list.
    then called OnRecvTranspError so fetch will return valid txn */
    pTxnKey = new SipTxnKey(pNonInvSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnRecvTxn(pNonInvSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));

    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));
    delete pTxnInfo;
    pNonInvSipMsg->SipDelete();
}

TEST_F(SipTxnHandlerTest, OnSendTranspError)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError;

    /* Calling with null SipTxnKey.
    To test if txnkey is null OnSendTranspError will be failed */
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnSendTranspError(SIP_NULL));

    /* Calling with invalid SipTxnKey i.e.
    To test if txnkey is not valid fetch txn will be failed.
    Then OnSendTranspError will be failed */
    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnSendTranspError(pTxnKey));
    pTxnKey->SipDelete();

    SipMessage* pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::REQ_TYPE);

    const SIP_CHAR* pReqUri = "sip:2222@ims.mnc861.mcc405.3gppnetwork.org";
    SipAddrSpec* pAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pAddrSpec->DecodeAddrSpec(pReqUri, SipPf_Strlen(pReqUri)));
    SipRequestLine* pobjReqLine = new SipRequestLine("INVITE", pAddrSpec, "SIP/2.0");
    ASSERT_TRUE(pobjReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetRequestline(pobjReqLine));

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    SipHeaderBase* pToHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TO);
    ASSERT_TRUE(pViaHdr != nullptr);
    ASSERT_TRUE(pToHdr != nullptr);
    const SIP_CHAR* pViaValue = "SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8";
    const SIP_CHAR* pToValue = "<sip:1111@ims.mnc861.mcc405.3gppnetwork.org>";
    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, SipPf_Strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pToHdr->DecodeHdr(pToValue, SipPf_Strlen(pToValue)));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pViaHdr));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pToHdr));

    /* Calling with valid SipTxnKey by creating with req msg
    but with only request uri, via & to header */
    pTxnKey = new SipTxnKey(pTempSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnSendTranspError(pTxnKey));
    pTxnKey->SipDelete();
    pTempSipMsg->SipDelete();
}

TEST_F(SipTxnHandlerTest, TerminateTxn)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError;

    /* Calling with null SipTxnKey.
    To test if txnkey is null TerminateTxn will be failed */
    EXPECT_EQ(SIP_FALSE, pTxnHandler->TerminateTxn(SIP_NULL));

    /* Calling with invalid SipTxnKey i.e.
    To test if txnkey is not valid fetch txn will be failed.
    Then TerminateTxn will be failed */
    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->TerminateTxn(pTxnKey));
    pTxnKey->SipDelete();

    SipMessage* pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::REQ_TYPE);

    const SIP_CHAR* pReqUri = "sip:2222@ims.mnc861.mcc405.3gppnetwork.org";
    SipAddrSpec* pAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pAddrSpec->DecodeAddrSpec(pReqUri, SipPf_Strlen(pReqUri)));
    SipRequestLine* pobjReqLine = new SipRequestLine("INVITE", pAddrSpec, "SIP/2.0");
    ASSERT_TRUE(pobjReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetRequestline(pobjReqLine));

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    const SIP_CHAR* pViaValue = "SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8";
    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, SipPf_Strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pViaHdr));

    /* Calling with valid SipTxnKey by creating with req msg
    but with only request uri & via header */
    pTxnKey = new SipTxnKey(pTempSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    pTxnKey->SipDelete();
    pTempSipMsg->SipDelete();

    /* Calling with valid SipTxnKey by creating with resp msg */
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    pTxnKey->SipDelete();
}

TEST_F(SipTxnHandlerTest, DeleteTxn)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError;

    /* Calling with null SipTxnKey.
    To test if txnkey is null DeleteTxn will be failed */
    EXPECT_EQ(SIP_FALSE, pTxnHandler->DeleteTxn(SIP_NULL));

    /* Calling with invalid SipTxnKey i.e.
    To test if txnkey is not valid fetch txn will be failed.
    Then DeleteTxn will be failed */
    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->DeleteTxn(pTxnKey));
    pTxnKey->SipDelete();

    SipMessage* pTempSipMsg = new SipMessage();
    pTempSipMsg->SetMessageType(SipMessage::REQ_TYPE);

    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    const SIP_CHAR* pViaValue = "SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bKnashds8";
    EXPECT_EQ(SIP_TRUE, pViaHdr->DecodeHdr(pViaValue, SipPf_Strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pTempSipMsg->SetHeader(pViaHdr));

    /* Calling with valid SipTxnKey by creating with req msg
    but without request uri */
    pTxnKey = new SipTxnKey(pTempSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->DeleteTxn(pTxnKey));
    pTxnKey->SipDelete();

    /* Calling with valid SipTxnKey by creating with resp msg */
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->DeleteTxn(pTxnKey));
    pTxnKey->SipDelete();
    pTempSipMsg->SipDelete();
}

TEST_F(SipTxnHandlerTest, UpdateTxnDetails)
{
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SIP_UINT16 nError;

    /* Calling with null SipTxnKey.
    To test if txnkey is null UpdateTxnDetails will be failed */
    EXPECT_EQ(SIP_FALSE, pTxnHandler->UpdateTxnDetails(SIP_NULL, pTranspInfo, &nError));

    /* Calling with invalid SipTxnKey i.e.
    To test if txnkey is not valid fetch txn will be failed.
    Then UpdateTxnDetails will be failed */
    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->UpdateTxnDetails(pTxnKey, pTranspInfo, &nError));
    pTxnKey->SipDelete();

    SipStatusLine* pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("200");
    pStatusLine->SipDelete();
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);

    /* Calling with valid SipTxnKey by creating with 200 response message
    first calling recv txn to add txn to list.
    then called UpdateTxnDetails so fetch will return valid txn */
    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->UpdateTxnDetails(pTxnKey, pTranspInfo, &nError));
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));

    pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("420");
    pStatusLine->SipDelete();

    /* Calling with 420 response message so that in Mock_FetchTransaction return
    true but txn is not passed. Inorder to test if txn is null Update will be failed */
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE, pTxnHandler->UpdateTxnDetails(pTxnKey, pTranspInfo, &nError));

    pTxnKey->SipDelete();
    delete pTranspInfo;
    delete pTxnInfo;
}

}  // namespace android
