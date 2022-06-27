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
#include "txn/SipTxnHandler.h"
#include "transport/SipTransportInfo.h"
#include "SipStackCallback.h"
#include "txn/SipTimeoutData.h"

namespace android
{

SIP_BOOL bFromRecvTxn;

SIP_BOOL Mock_FetchTransaction(
        SIP_VOID* pvTxnKey, SIP_INT32 nOption, SIP_VOID** /*ppvOutTxnKey*/, SIP_VOID** ppvTxn)
{
    if (nOption == TXN_OPT_CREATE)
    {
        if (strcmp((((SipTxnKey*)pvTxnKey))->GetMethod(), "BYE") == 0)
        {
            return SIP_FALSE;
        }
        if (*ppvTxn != SIP_NULL)
        {
            ((SipTxn*)*ppvTxn)->decrement();
        }

        ((SipTxnKey*)pvTxnKey)->SipDelete();
        return SIP_TRUE;
    }
    else
    {
        SIP_UINT16 nError;
        SIP_INT32 eMsgType = ((SipTxnKey*)pvTxnKey)->GetMsgType();

        switch (eMsgType)
        {
            case SipMessage::REQ_TYPE:
            {
                if (strcmp((((SipTxnKey*)pvTxnKey))->GetMethod(), "CANCEL") == 0)
                {
                    return SIP_TRUE;
                }

                if (strcmp((((SipTxnKey*)pvTxnKey))->GetMethod(), "UPDATE") == 0)
                {
                    *ppvTxn = new SipTxn();
                    return SIP_TRUE;
                }
                return SIP_FALSE;
            }
            case SipMessage::TYPE_INVALID:
                return SIP_FALSE;
            case SipMessage::RESP_TYPE:
            {
                if (((((SipTxnKey*)pvTxnKey))->GetRespCode() == 202) ||
                        (((SipTxnKey*)pvTxnKey))->GetRespCode() == 603)
                {
                    return SIP_FALSE;
                }
                SipMessage* pTempSipMsg = new SipMessage();
                if ((((SipTxnKey*)pvTxnKey))->GetRespCode() == 480)
                {
                    *ppvTxn = new SipTxn(
                            SipTxn::INV_SER_TXN, SIP_NULL, pTempSipMsg, SIP_NULL, &nError);
                    pTempSipMsg->SipDelete();
                    return SIP_TRUE;
                }
                else if (bFromRecvTxn == SIP_TRUE)
                {
                    *ppvTxn = new SipTxn(
                            SipTxn::INV_CLI_TXN, SIP_NULL, pTempSipMsg, SIP_NULL, &nError);
                    SipTxn* pTempTxn = (SipTxn*)*ppvTxn;
                    pTempTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);
                    pTempSipMsg->SipDelete();
                    return SIP_TRUE;
                }
                else
                {
                    *ppvTxn = new SipTxn(
                            SipTxn::INV_SER_TXN, SIP_NULL, pTempSipMsg, SIP_NULL, &nError);
                    SipTxn* pTempTxn = (SipTxn*)*ppvTxn;
                    pTempTxn->SetTxnState(SipTxn::INV_SER_PROCEEDING_ST);
                    pTempSipMsg->SipDelete();
                    return SIP_TRUE;
                }
            }
            default:
                return SIP_FALSE;
        }
    }
}

SIP_BOOL Mock_StartTimer(SIP_UINT32, SipTimerCallback, SIP_VOID* pvData, SIP_VOID**)
{
    SipTimeoutData* pTimeoutData = reinterpret_cast<SipTimeoutData*>(pvData);
    delete pTimeoutData;
    return SIP_TRUE;
}

SIP_BOOL Mock_ReleaseTransaction(SIP_VOID* pvTxnKey, SIP_INT32, SIP_VOID**, SIP_VOID**)
{
    if ((((SipTxnKey*)pvTxnKey)->GetMsgType() == SipMessage::TYPE_INVALID))
    {
        return SIP_FALSE;
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

protected:
    virtual void SetUp() override
    {
        bFromRecvTxn = SIP_FALSE;
        SipUtil_Construct();

        pTxnHandler = new SipTxnHandler();
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

        // Response Msg
        pRespSipMsg = new SipMessage();
        pRespSipMsg->SetMessageType(SipMessage::RESP_TYPE);

        pMsg = (char*)"SIP/2.0 406 Not Acceptable\r\n\
Via: SIP/2.0/UDP pc33.atlanta.com;branch=z9hG4bs8\r\n\
From: <sip:user@host>;tag=abcd\r\n\
To: <sip:userA@host>\r\n\
Call-ID: 1332a-3c0d31@2409:192.168.35.156\r\n\
CSeq: 1 INVITE\r\n\
\r\n";
        EXPECT_EQ(SIP_TRUE, pRespSipMsg->DecCompleteMsg(pMsg, strlen(pMsg)));

        pSipUserData = new ISipUserData(SIP_NULL);

        pSipTranspParam = new SipTransportParameter(
                (SIP_CHAR*)"192.168.35.156", 5060, SipTransportInfo::PROTOCOL_UDP);

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
    if (pTxnKey != SIP_NULL)
    {
        delete pTxnKey;
    }
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
    if (pTxnKey != SIP_NULL)
    {
        delete pTxnKey;
    }

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("PRACK"));
    pReqLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    if (pTxnKey != SIP_NULL)
    {
        delete pTxnKey;
    }

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
    delete pTempMsg;
}

TEST_F(SipTxnHandlerTest, OnSendTxn_ResponseMsg)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError = 0;

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    if (pTxnKey != SIP_NULL)
    {
        delete pTxnKey;
    }

    // Send 2xx Response msg
    SipStatusLine* pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("200");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    if (pTxnKey != SIP_NULL)
    {
        delete pTxnKey;
    }

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("202");
    pStatusLine->SipDelete();

    EXPECT_EQ(SIP_TRUE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));
    if (pTxnKey != SIP_NULL)
    {
        delete pTxnKey;
    }

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

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnSendTxn(
                    pRespSipMsg, pSipTranspParam, pSipUserData, &pTxnKey, pTxnInfo, &nError));

    SipCSeqHeader* pCSeq = (SipCSeqHeader*)pRespSipMsg->GetHdrObj(ESIPHDR_CSEQ);

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
    delete pTempMsg;
    delete pTxnKey;
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, OnRecvTxn_Request)
{
    SipTxnInfo* pTxnInfo = new SipTxnInfo();
    SIP_UINT16 nError;
    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, &nError);

    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    delete pTxnKey;

    SipRequestLine* pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("PRACK"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    delete pTxnKey;

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("CANCEL"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    delete pTxnKey;

    pReqLine = pSipMsg->GetReqLine();
    ASSERT_TRUE(pReqLine != nullptr);
    EXPECT_EQ(SIP_TRUE, pReqLine->SetMethod("BYE"));
    pReqLine->SipDelete();

    pTxnKey = new SipTxnKey(pSipMsg, &nError);
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTxn(pSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    delete pTxnKey;
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
    delete pTxnKey;

    SipStatusLine* pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("603");
    pStatusLine->SipDelete();
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    delete pTxnKey;

    pStatusLine = pRespSipMsg->GetStatusLine();
    ASSERT_TRUE(pStatusLine != nullptr);
    pStatusLine->SetStatusCode("480");
    pStatusLine->SipDelete();
    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);

    EXPECT_EQ(SIP_FALSE,
            pTxnHandler->OnRecvTxn(pRespSipMsg, pTxnKey, pSipUserData, pTxnInfo, &nError));
    delete pTxnKey;
    delete pTxnInfo;
}

TEST_F(SipTxnHandlerTest, UpdateTxnDetails)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SipTransportInfo* pTranspInfo = new SipTransportInfo(pSipTranspParam, SIP_NULL);
    SIP_UINT16 nError;

    EXPECT_EQ(SIP_FALSE, pTxnHandler->UpdateTxnDetails(SIP_NULL, pTranspInfo, &nError));

    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->UpdateTxnDetails(pTxnKey, pTranspInfo, &nError));
    delete pTxnKey;
    pTxnKey = SIP_NULL;

    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->UpdateTxnDetails(pTxnKey, pTranspInfo, &nError));
    delete pTxnKey;
    delete pTranspInfo;
}

TEST_F(SipTxnHandlerTest, OnRecvTranspError)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError = SipTransportInfo::PROTOCOL_UDP;

    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTranspError(nError, SIP_NULL, &nError));

    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));
    delete pTxnKey;

    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnRecvTranspError(nError, pTxnKey, &nError));
    delete pTxnKey;
}

TEST_F(SipTxnHandlerTest, OnSendTranspError)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError;

    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnSendTranspError(SIP_NULL));

    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->OnSendTranspError(pTxnKey));
    delete pTxnKey;
    pTxnKey = SIP_NULL;

    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->OnSendTranspError(pTxnKey));
    delete pTxnKey;
}

TEST_F(SipTxnHandlerTest, TerminateTxn)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError;

    EXPECT_EQ(SIP_FALSE, pTxnHandler->TerminateTxn(SIP_NULL));

    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->TerminateTxn(pTxnKey));
    delete pTxnKey;
    pTxnKey = SIP_NULL;

    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->TerminateTxn(pTxnKey));
    delete pTxnKey;
}

TEST_F(SipTxnHandlerTest, DeleteTxn)
{
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_UINT16 nError;

    EXPECT_EQ(SIP_FALSE, pTxnHandler->DeleteTxn(SIP_NULL));

    pTxnKey = new SipTxnKey();
    EXPECT_EQ(SIP_FALSE, pTxnHandler->DeleteTxn(pTxnKey));
    delete pTxnKey;
    pTxnKey = SIP_NULL;

    pTxnKey = new SipTxnKey(pRespSipMsg, &nError);
    EXPECT_EQ(SIP_TRUE, pTxnHandler->DeleteTxn(pTxnKey));
    delete pTxnKey;
}

}  // namespace android
