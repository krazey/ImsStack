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
#include "SipDebug.h"
#include "SipMessageBuffer.h"
#include "SipStackManager.h"
#include "SipUtil.h"
#include "platform/SipMemory.h"
#include "transport/SipTransportHandler.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnHandler.h"

static SipStackManager* gpStackMngr = SIP_NULL;

SipStackManager::SipStackManager() {}

SipStackManager::~SipStackManager()
{
    SipUtil::DestroyInstance();
}

SipStackManager* SipStackManager::GetInstance()
{
    if (gpStackMngr == SIP_NULL)
    {
        gpStackMngr = new SipStackManager();
    }
    return gpStackMngr;
}

void SipStackManager::Destruct()
{
    if (gpStackMngr != SIP_NULL)
    {
        delete gpStackMngr;
        gpStackMngr = SIP_NULL;
    }
}

SIP_BOOL SipStackManager::SendMsg(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
        IN ISipUserData* pUserData, IN const SIP_CHAR* pSipBuffer, IN SIP_UINT32 nSipBufferLen,
        IN SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError)
{
    /* Input parameter validation */
    if ((pSipMsg == SIP_NULL) || (pTranspParam == SIP_NULL) || (ppTxnKey == SIP_NULL) ||
            (pnError == SIP_NULL))
    {
        return SIP_FALSE;
    }

    *ppTxnKey = SIP_NULL;

    /* Transaction Processing
NOTE: In case of 2XX and successful ACK for INVITE, the Txn module does
not perform any processing, it simply return success. Also pTxnKey will be NULL
     */
    SipTxnInfo objTxnInfo;
    SipTxnHandler objTxnHandler;
    SipTxnKey* pTxnKey = SIP_NULL;

    if (objTxnHandler.OnSendTxn(pSipMsg, pTranspParam, pUserData, &pTxnKey, &objTxnInfo, pnError) ==
            SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnHandler::OnSendTxn:Failed", 0, 0);
        return SIP_FALSE;
    }

    /* Transport layer processing
       pTranspInfo contains the encoded SIP message and remote transport information
     */
    SipTransportHandler objTranspHandler;
    SipTransportInfo* pTranspInfo = SIP_NULL;

    if (objTranspHandler.OnSendTransp(pSipMsg, pTranspParam, pSipBuffer, nSipBufferLen,
                &pTranspInfo, pnError) == SIP_FALSE)
    {
        /* Inform txn about transport error, this terminates Txn */
        objTxnHandler.OnSendTranspError(pTxnKey);

        if (objTxnInfo.m_bTxnCreated == SIP_TRUE)
        {
            // It will be destroyed by the caller.
            objTxnInfo.m_pUserData->SetUserData(SIP_NULL);
        }

        pTxnKey->SipDelete();

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnSendTransp: Failed", 0, 0);
        return SIP_FALSE;
    }

    /* NOTE: All param passed in callback are read only parameters and callback
       implementor shall not deleted any parameters*/
    if (SendToNetwork(pTranspInfo, pUserData) == SIP_FALSE)
    {
        /* BSP_TODO: do we need to inform Txn layer for error
           objTxnHandler.OnSendTranspError */

        if (objTxnInfo.m_bTxnCreated == SIP_TRUE)
        {
            // It will be destroyed by the caller.
            objTxnInfo.m_pUserData->SetUserData(SIP_NULL);
        }

        delete pTranspInfo;
        pTxnKey->SipDelete();

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SendToNetwork: Failed", 0, 0);
        return SIP_FALSE;
    }

    if (objTxnInfo.m_bTxnTerminated == SIP_TRUE)
    {
        pUserData->SetUserData(objTxnInfo.m_pUserData->GetUserData());
        pUserData->SetDeleteFlag(SIP_TRUE);
        objTxnInfo.m_pUserData->SetUserData(SIP_NULL);

        /* Notifies to Transaction User using registered listener
           Delete Txn entry from DB and delete the instance*/
        objTxnHandler.DeleteTxn(pTxnKey);

        pTxnKey->SipDelete();
        delete pTranspInfo;

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SendMsg: TxnTerminated", SIP_ZERO, SIP_ZERO);
    }
    else if (pTxnKey != SIP_NULL)
    {
        if (objTxnHandler.UpdateTxnDetails(pTxnKey, pTranspInfo, pnError) == SIP_FALSE)
        {
            delete pTranspInfo;

            // if ACK no need to maintain transaction
            if (pSipMsg->GetMethodType() != SipMessage::METHOD_ACK)
            {
                if (objTxnInfo.m_bTxnCreated == SIP_TRUE)
                {
                    // It will be destroyed by the caller.
                    objTxnInfo.m_pUserData->SetUserData(SIP_NULL);
                }

                pTxnKey->SipDelete();

                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "UpdateTxnDetails: Failed", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }

        if (objTxnInfo.m_bTxnCreated == SIP_TRUE)
        {
            /* return Key of newly created txn */
            *ppTxnKey = pTxnKey;
        }
        else
        {
            pTxnKey->SipDelete();
        }
    }

    // If it's a new request, then the user data is managed by the transaction object.
    if (objTxnInfo.m_bTxnCreated == SIP_TRUE)
    {
        pUserData->SetUserData(SIP_NULL);
    }

    return SIP_TRUE;
}

SIP_BOOL SipStackManager::OnRecvMessage(IN SipMessage* pSipMsg,
        IN SipTransportParameter* pTranspParam, IN ISipUserData* pUserData,
        OUT SIP_INT32* peTxnStatus, OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError)
{
    /* Input parameter validation */
    if ((pSipMsg == SIP_NULL) || (pTranspParam == SIP_NULL) || (peTxnStatus == SIP_NULL) ||
            (ppTxnKey == SIP_NULL) || (pnError == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage, invalid params", 0, 0);
        return SIP_FALSE;
    }

    *peTxnStatus = SipTxn::STATUS_INVALID;
    *ppTxnKey = SIP_NULL;

    /* Transport layer processing
       1. Validate incoming SIP Message
       2. Match transaction
       3. Return Transport status based on req/resp type and Method and validity of Txn state
NOTE: for valid re-transmitted msg received, pTranspInfo contains the massege that needs to be
send to network
     */
    SipTransportHandler objTranspHandler;
    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;

    if (objTranspHandler.OnRecvTransp(
                pSipMsg, pTranspParam, peTxnStatus, &bTxnExist, &pTxnKey, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage returned failure", 0, 0);
        return SIP_FALSE;
    }

    if (*peTxnStatus == SipTxn::STATUS_STRAY_RESP)
    {
        pTxnKey->SipDelete();
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "OnRecvTransp, Drop Message, TxnStatus[%d]", *peTxnStatus, 0);
        return SIP_TRUE;
    }

    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTransp, TxnStatus[%d]", *peTxnStatus, 0);

    /*
       CASE :: Handling 2xx resp for INVITE (Txn not existing)
       2xx Response for INVITE is received and No Txn exists. This might be re-transmitted 2xx.
       stack user must process this request and can decide whether to ignore or not, in case user
       process this response, user must re-transmit successful ACK
     */
    if ((bTxnExist == SIP_FALSE) && (pSipMsg->GetMsgType() == SipMessage::RESP_TYPE))
    {
        if (pSipMsg->GetMethodType() == SipMessage::METHOD_INVITE)
        {
            SIP_UINT16 nStatusCode = pSipMsg->GetStatusCode();

            if (SipMsgUtil::IsSuccessfulResponse(nStatusCode))
            {
                *peTxnStatus = SipTxn::STATUS_IGNORE_RESP;

                pTxnKey->SipDelete();

                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "OnRecvMessage: INVITE 2xx received (No txn exist)", 0, 0);
                return SIP_TRUE;
            }
        }
    }

    /* Invoke Txn layer handling
       pSipMsg2Send --> contains either 100 Trying when new INVITE request is received or
       failure ACK when failure response for INVITE is received
     */

    SipTxnInfo objTxnInfo;
    SipTxnHandler objTxnHandler;

    if (objTxnHandler.OnRecvTxn(pSipMsg, pTxnKey, pUserData, &objTxnInfo, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage: OnRecvTxn Failed", 0, 0);

        pTxnKey->SipDelete();

        *peTxnStatus = SipTxn::STATUS_INVALID_MESSAGE;

        return SIP_FALSE;
    }

    *peTxnStatus = objTxnInfo.m_eTxnStatus;

    // If it's a new request, then the user data is managed by the transaction object.
    if (objTxnInfo.m_bTxnCreated == SIP_TRUE)
    {
        pUserData->SetUserData(SIP_NULL);
    }
    else
    {
        if (objTxnInfo.m_pUserData != SIP_NULL)
        {
            pUserData->SetUserData(objTxnInfo.m_pUserData->GetUserData());
        }
    }

    /*
       CASE-1 :: Ignore request & ignore response case
       In case of ignore req, ignore resp --> simply return
       stack user can simply ignore these cases
     */
    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTxn :: txnStatus=%d", objTxnInfo.m_eTxnStatus, 0);

    switch (objTxnInfo.m_eTxnStatus)
    {
        case SipTxn::STATUS_IGNORE_REQ:
        case SipTxn::STATUS_IGNORE_RESP:
        case SipTxn::STATUS_STRAY_RESP:
        case SipTxn::STATUS_STRAY_PRACK:
        {
            pTxnKey->SipDelete();
            return SIP_TRUE;
        }
        case SipTxn::STATUS_NEW_REQ_RECVD:
        case SipTxn::STATUS_RETRANSMISSION:
        case SipTxn::STATUS_VALID_MESSAGE:
        {
            break;
        }
        default:
        {
            pTxnKey->SipDelete();
            return SIP_FALSE;
        }
    }

    /*
       CASE-2 :: Handling of Re-Transmitted request/response received in valid txn states
       INV Req--> Re-Transmitted INVITE request recv in proceeding state and completed state,
       re-transmit the last response.
       Non-INV Req --> Re-Transmitted Non-INVITE request recv in proceeding state and completed
       state, re-transmit the last response. INV Failure Resp --> Failure response recv is completed
       state, re-transmit the failure ACK
     */
    if (objTxnInfo.m_eTxnStatus == SipTxn::STATUS_RETRANSMISSION)
    {
        if (SendToNetwork(objTxnInfo.m_pTranspInfo, objTxnInfo.m_pUserData) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SendToNetwork Failed in retransmitted case", 0, 0);
        }

        pTxnKey->SipDelete();

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage: Re-Transmitted Msg received", 0, 0);
        return SIP_TRUE;
    }

    /* Check if any message needs to be send to network.
       Case of Sending of Failure ACK when failure response is received for INVITE
       Case of Sending 100 Trying response, on receive of INVITE
     */
    if ((objTxnInfo.m_pSendSipMsg != SIP_NULL) && (objTxnInfo.m_pUserData != SIP_NULL))
    {
        SipTransportInfo* pTranspInfo = SIP_NULL;
        RcPtr<SipMessageBuffer> pMessageBuffer = SipMessageBuffer::GetInstance();
        SIP_UINT32 nSipBufferLen = pMessageBuffer->GetLength();
        SIP_CHAR* pSipBuffer = reinterpret_cast<SIP_CHAR*>(pMessageBuffer->GetBuffer());

        SipPf_Memset(pSipBuffer, 0x00, nSipBufferLen);

        if (objTxnInfo.m_pSendSipMsg->Encode(
                    &pSipBuffer, &nSipBufferLen, pUserData->GetMsgOptions()) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage:Encode Fail", SIP_ZERO, SIP_ZERO);
            objTxnHandler.OnSendTranspError(pTxnKey);
            pTxnKey->SipDelete();
            return SIP_TRUE;
        }

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage: Invoking OnSendTransp... ", 0, 0);

        if (objTranspHandler.OnSendTransp(objTxnInfo.m_pSendSipMsg, pTranspParam, pSipBuffer,
                    nSipBufferLen, &pTranspInfo, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage: OnSendTransp fail", 0, 0);

            /* Inform txn about transport error, this terminates Txn */
            objTxnHandler.OnSendTranspError(pTxnKey);

            pTxnKey->SipDelete();
            return SIP_TRUE;
        }

        ISipTransactionCallback* piCallback = SipUtil::GetInstance()->GetTransactionCallback();

        if (piCallback != SIP_NULL)
        {
            // Notify SIP message sent by stack to the application for a proper handling
            piCallback->PreProcessMessageSentByStack(
                    objTxnInfo.m_pSendSipMsg, objTxnInfo.m_pUserData);
        }

        /* BSP_TODO:
           In case of 100 Trying, Txn Obj may not have user date, better to use user passed data
           For failure ACK, better to user Txn stored data */
        if (SendToNetwork(pTranspInfo, objTxnInfo.m_pUserData) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage: SendToNetwork Failed", 0, 0);
            pTxnKey->SipDelete();
            delete pTranspInfo;
            return SIP_TRUE;
        }

        SipTransportBuffer* pTransBuffer = pTranspInfo->GetTranspSipBuffer();

        if (piCallback != SIP_NULL)
        {
            piCallback->PostProcessMessageSentByStack(objTxnInfo.m_pSendSipMsg,
                    (pTransBuffer != SIP_NULL) ? pTransBuffer->GetSipBuffer() : SIP_NULL,
                    (pTransBuffer != SIP_NULL) ? pTransBuffer->GetSipBufferLen() : 0,
                    objTxnInfo.m_pUserData);
        }

        /*Update Txn details for last message send to network */
        /* BSP_TODO: it's overwriting the existing transpInfo... is it correct way to do
           why can't only update the necessary details and maintaining old key data for security */
        if (objTxnHandler.UpdateTxnDetails(pTxnKey, pTranspInfo, pnError) == SIP_FALSE)
        {
            /* BSP_TODO: oops what to doooooooooo???*/
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvMessage: UpdateTxnDetails Failed", 0, 0);
        }
    }

    /* INV Client Txn --> For INV 2xx case, txn is terminated
       INV Serv Txn --> For TCP, on ACK recv Txn is terminated */
    if (objTxnInfo.m_bTxnTerminated == SIP_TRUE)
    {
        if (objTxnInfo.m_pUserData != SIP_NULL)
        {
            objTxnInfo.m_pUserData->SetUserData(SIP_NULL);
        }

        pUserData->SetDeleteFlag(SIP_TRUE);

        /* Notifies to Transaction User using registered listener
           Delete Txn entry from DB and delete the instance*/
        objTxnHandler.DeleteTxn(pTxnKey);

        if (pSipMsg->GetMethodType() == SipMessage::METHOD_ACK)
        {
            *ppTxnKey = pTxnKey;
        }
        else
        {
            pTxnKey->SipDelete();
        }

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTxn: Txn Terminated", SIP_ZERO, SIP_ZERO);
    }
    else if ((objTxnInfo.m_bTxnCreated == SIP_TRUE) ||
            (pSipMsg->GetMethodType() == SipMessage::METHOD_ACK))
    {
        /* return Key of newly created txn */
        *ppTxnKey = pTxnKey;
    }
    else if (pSipMsg->GetMethodType() == SipMessage::METHOD_INVITE)
    {
        SIP_UINT16 nStatusCode = pSipMsg->GetStatusCode();

        if (SipMsgUtil::IsSuccessfulResponse(nStatusCode))
        {
            /* return the key to update the to-tag */
            *ppTxnKey = pTxnKey;
        }
    }
    else
    {
        pTxnKey->SipDelete();
        pTxnKey = SIP_NULL;
    }

    return SIP_TRUE;
}

SIP_BOOL SipStackManager::OnRecvTanspError(
        IN SIP_INT32 eTranspError, IN SipTxnKey* pTxnKey, OUT SIP_UINT16* pnError)
{
    SipTransportHandler objTranspHandler;
    SIP_INT32 eTxnStatus = SipTxn::STATUS_INVALID;
    SipTransportInfo* pTranspInfo = SIP_NULL;
    ISipUserData objUserData;

    /* If error is occurred due to Msg Constraint , switch back to UDP and transport */

    if (objTranspHandler.OnRecvTanspError(eTranspError, pTxnKey, &eTxnStatus, &pTranspInfo,
                &objUserData, pnError) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    /* Resend to Network due to message size constraint switch */
    if (eTxnStatus == SipTxn::STATUS_RETRANSMISSION)
    {
        if (SendToNetwork(pTranspInfo, &objUserData) == SIP_TRUE)
        {
            return SIP_TRUE;
        }
    }

    /* Invoke Transaction layer for transport error */
    SipTxnHandler objTxnHandler;
    return objTxnHandler.OnRecvTranspError((SIP_INT32)(*pnError), pTxnKey, pnError);
}

SIP_BOOL SipStackManager::TerminateTxn(SipTxnKey* pTxnKey)
{
    SipTxnHandler objTxnHandler;

    return objTxnHandler.TerminateTxn(pTxnKey);
}

SIP_VOID SipStackManager::RegisterNetwork(ISipNetworkUtil* pNetworkUtil)
{
    SipUtil::GetInstance()->SetNetwork(pNetworkUtil);
}

SIP_VOID SipStackManager::RegisterTransactionCallback(ISipTransactionCallback* pCallback)
{
    SipUtil::GetInstance()->SetTransactionCallback(pCallback);
}

PRIVATE SIP_BOOL SipStackManager::SendToNetwork(
        IN SipTransportInfo* pTranspInfo, IN ISipUserData* pUserData)
{
    if (pTranspInfo == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SendToNetwork Failed transpinfo is NULL", SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    /* Call Sent to Network Callback */
    ISipNetworkUtil* pNetworkUtil = SipUtil::GetInstance()->GetNetwork();
    SipTransportBuffer* pTransSipBuffer = pTranspInfo->GetTranspSipBuffer();
    SipTransportParameter* pActualDestParam = pTranspInfo->GetMsgSentTranspParam();

    if (pNetworkUtil->SendToNetwork(pTransSipBuffer, pActualDestParam, pUserData) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "pNetworkUtil->SendToNetwork utility fxn failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
