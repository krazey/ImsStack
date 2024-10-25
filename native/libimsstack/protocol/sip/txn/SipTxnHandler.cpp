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
#include "SipStackError.h"
#include "SipTxnContext.h"
#include "SipUtil.h"
#include "msg/SipMessage.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsm.h"
#include "txn/SipTxnHandler.h"

static SIP_INT32 GetNonInvCliFsmEvt(SIP_UINT16 nStatusCode);
static SIP_INT32 GetNonInvSerFsmEvt(SIP_UINT16 nStatusCode);
static SIP_INT32 GetInvCliFsmEvt(SIP_UINT16 nStatusCode);
static SIP_INT32 GetInvSerFsmEvt(SIP_UINT16 nStatusCode);

SIP_BOOL SipTxnHandler::OnSendTxn(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
        IN ISipUserData* pUserData, IN SipTxnKey** ppTxnKey, OUT SipTxnInfo* pTxnInfo,
        OUT SIP_UINT16* pnError)
{
    SipTxnFsmData objTxnFsmData(pSipMsg, pTranspParam, pUserData);

    SipTxnKey* pTxnKey = SIP_NULL;
    SIP_INT32 eTxnType = SipTxn::INVALID_TXN;
    /* Validate txn params from sip message and returns txn key and txn type */
    if (ValidateSendTxn(pSipMsg, &eTxnType, &pTxnKey, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnSendTxn:ValidateSendTxn Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // SetTxnType, InvCli, NonInvCli, InvSer and NonInvSer
    pTxnKey->SetTxnType(eTxnType);

    /* Get txn object from txn DB */
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTXN, "OnSendTxn:GetTxnObjFromDb Fails \n", SIP_ZERO, SIP_ZERO);
        pTxnKey->SipDelete();
        return SIP_FALSE;
    }

    /* Check if Txn already exists for Req */
    if (bTxnExist == SIP_YES)
    {
        if (pTxnKey->GetMsgType() == SipMessage::REQ_TYPE)
        {
            *pnError = ETXN_ALREADYTRANSACTIONINPROCESSERROR;

            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "OnSendTxn: Txn Already in Progress\n", SIP_ZERO, SIP_ZERO);

            pTxnKey->SipDelete();
            return SIP_FALSE;
        }
    }
    else /* Txn not Found */
    {
        /* handle Response case */
        if (pTxnKey->GetMsgType() == SipMessage::RESP_TYPE)
        {
            /* 2xx retransmission for invite message should be sent directly to transport */
            if (pSipMsg->GetMethodType() == SipMessage::METHOD_INVITE)
            {
                SIP_UINT16 nStatusCode = pSipMsg->GetStatusCode();
                if (SipMsgUtil::IsSuccessfulResponse(nStatusCode))
                {
                    *ppTxnKey = pTxnKey;
                    return SIP_TRUE;
                }
            }

            pTxnKey->SipDelete();
            return SIP_FALSE;
        }
        else
        {
            /* For ACK send directly to Transport : request check is for better understanding */
            if ((pTxnKey->GetMsgType() == SipMessage::REQ_TYPE) &&
                    (pSipMsg->GetMethodType() == SipMessage::METHOD_ACK))
            {
                /* Send ACK Directly to N/w */
                *ppTxnKey = pTxnKey;
                return SIP_TRUE;
            }
        }
    } /* end of txn not found */

    /* Invoke FSM for Transaction state handling */

    switch (eTxnType)
    {
        case SipTxn::INV_CLI_TXN:
        case SipTxn::NON_INV_CLI_TXN:
        {
            /* Invoking client FSM to process and send request. It create new txn object */
            if (HandleClientTxnSend(eTxnType, pTxnKey, &objTxnFsmData, pnError) == SIP_FALSE)
            {
                pTxnKey->SipDelete();

                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnSendTxn: HandleClientTxnSend fail", SIP_ZERO,
                        SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;
        case SipTxn::INV_SER_TXN:
        case SipTxn::NON_INV_SER_TXN:
        {
            /* Invokes server FSM to process and send response */
            if (HandleServerTxnSend(eTxnType, pTxnKey, &objTxnFsmData, pnError) == SIP_FALSE)
            {
                pTxnKey->SipDelete();

                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnSendTxn: HandleServerTxnSend fail", SIP_ZERO,
                        SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;
        default:
        {
            pTxnKey->SipDelete();

            *pnError = SipTxn::INVALID_TXN;
            SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "OnSendTxn: INVALID Txn Type", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        break;
    }

    pTxnInfo->bTxnTerminated = objTxnFsmData.bTxnTerminated;
    pTxnInfo->bTxnCreated = objTxnFsmData.bTxnCreated;

    if ((objTxnFsmData.bTxnCreated == SIP_TRUE) || (objTxnFsmData.bTxnTerminated == SIP_TRUE))
    {
        pTxnInfo->m_pUserData = objTxnFsmData.m_pOutUserData;
    }

    *ppTxnKey = pTxnKey;
    return SIP_TRUE;
}

SIP_BOOL SipTxnHandler::OnRecvTxn(IN SipMessage* pSipMsg, IN SipTxnKey* pTxnKey,
        IN ISipUserData* pUserData, OUT SipTxnInfo* pTxnInfo, OUT SIP_UINT16* pnError)
{
    SipTxnFsmData objTxnFsmData(pSipMsg,
            SIP_NULL,  // Transport param
            pUserData);

    if ((pSipMsg == SIP_NULL) || (pTxnKey == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTxn: NULL param", 0, 0);
        return SIP_FALSE;
    }

    SIP_INT32 eTxnType = SipTxn::INVALID_TXN;
    /* Validate Txn params from sip message and returns txn type */
    if (ValidateRecvTxn(pSipMsg, &eTxnType) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTxn: ValidateRecvTxn failure \n", 0, 0);
        return SIP_FALSE;
    }

    // SetTxnType, InvCli, NonInvCli, InvSer and NonInvSer
    pTxnKey->SetTxnType(eTxnType);

    switch (eTxnType)
    {
        case SipTxn::INV_CLI_TXN:
        case SipTxn::NON_INV_CLI_TXN:
        {
            /* Invoking client FSM to process received response */
            if (HandleClientTxnRecv(eTxnType, pTxnKey, &objTxnFsmData, IN_OUT pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "OnRecvTxn:HandleClientTxnRecv fail", SIP_ZERO, SIP_ZERO);

                return SIP_FALSE;
            }

            /* INV Client Txn --> For INV 2xx case, txn is terminated */
            if (objTxnFsmData.bTxnTerminated == SIP_TRUE)
            {
                /* stack manager to Notifies to Transaction User using registered listener
                   and Delete Txn entry from DB and delete the instance*/
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "OnRecvTxn: Client Txn Terminated", SIP_ZERO, SIP_ZERO);
            }
        }
        break;

        case SipTxn::INV_SER_TXN:
        case SipTxn::NON_INV_SER_TXN:
        {
            /* For new request(no txn exist, pTxn is NULL), create new txn and process
            request. For existing txn, it should be Failure ACK message, process the message */
            if (HandleServerTxnRecv(eTxnType, pTxnKey, &objTxnFsmData, IN_OUT pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "OnRecvTxn:HandleServerTxnRecv fail", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }

            /* INV Serv Txn --> For TCP, on ACK recv Txn is terminated */
            if (objTxnFsmData.bTxnTerminated == SIP_TRUE)
            {
                /* stack manager to Notifies to Transaction User using registered listener
                   and Delete Txn entry from DB and delete the instance*/
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "OnRecvTxn: Server Txn Terminated", SIP_ZERO, SIP_ZERO);
            }
        }
        break;

        default:
        {
            *pnError = SipTxn::INVALID_TXN;
            SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "OnRecvTxn: INVALID Txn Type", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        break;
    }

    if ((eTxnType == SipTxn::INV_CLI_TXN) || (eTxnType == SipTxn::INV_SER_TXN))
    {
        /* INV Client Txn : when failure response is received, stack send failure ACK
           inform to stack manager for sending of failure ACK request.
           NOTE--> For all other cases, it contains NULL

           INV Serv Txn: when new INV request is received, stack send 100 Trying
           response. inform to stack manager for sending of 100 Trying resp message.
           NOTE--> For all other cases, it contains NULL
         */
        pTxnInfo->m_pSendSipMsg = objTxnFsmData.m_pSendSipMsg;
    }

    if (objTxnFsmData.eTxnStatus == SipTxn::STATUS_RETRANSMISSION)
    {
        pTxnInfo->m_pTranspInfo = objTxnFsmData.m_pTranspInfo;
    }

    pTxnInfo->m_pUserData = objTxnFsmData.m_pOutUserData;
    pTxnInfo->bTxnTerminated = objTxnFsmData.bTxnTerminated;
    pTxnInfo->bTxnCreated = objTxnFsmData.bTxnCreated;
    pTxnInfo->eTxnStatus = objTxnFsmData.eTxnStatus;

    return SIP_TRUE;
}

SIP_BOOL SipTxnHandler::OnRecvTranspError(
        SIP_INT32 eTransErrro, SipTxnKey* pTxnKey, SIP_UINT16* pnError)
{
    if (pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "OnRecvTranspError: pTxnKey  Null \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Get Transaction Object from Db, based on Key */
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;

    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "OnRecvTranspError:GetTxnObjFromDb Fails \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTxn == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTranspError: No SipTxn \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_INT32 eTxnType = pTxn->GetTxnType();

    switch (eTxnType)
    {
        case SipTxn::INV_CLI_TXN:
        {
            if (pTxn->InvokeFsm(SipTxn::INV_CLI_TRANSP_ERROR_EVT, &eTransErrro, pnError) ==
                    SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTranspError: INV-CLI-FSM fails\n",
                        SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::NON_INV_CLI_TXN:
        {
            if (pTxn->InvokeFsm(SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT, &eTransErrro, pnError) ==
                    SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTranspError: Non-INV-CLI-FSM fails\n",
                        SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::INV_SER_TXN:
        {
            if (pTxn->InvokeFsm(SipTxn::INV_SER_TRANSP_ERROR_EVT, &eTransErrro, pnError) ==
                    SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTranspError: INV-SERV-FSM fails\n",
                        SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;
        case SipTxn::NON_INV_SER_TXN:
        {
            if (pTxn->InvokeFsm(SipTxn::NON_INV_SER_TRANSP_ERROR_EVT, &eTransErrro, pnError) ==
                    SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "OnRecvTranspError: Non-INV-SER-FSM fails\n",
                        SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;
        default:
        {
            *pnError = SipTxn::INVALID_TXN;
            SIP_DEBUG_STACKBUG(
                    ESIPTRACE_MODTXN, "OnRecvTranspError: INVALID Txn Type\n", SIP_ZERO, SIP_ZERO);
        }
        break;
    }

    /* Notify Txn Termination, need not send retransmission */
    if (pTxn->IsTxnTerminated() == SIP_TRUE)
    {
        /* Notifies to Transaction User using registered listener
           Delete Txn entry from DB and delete the instance*/

        DeleteTxn(pTxnKey);

        return SIP_TRUE;
    }
    return SIP_TRUE;
}

SIP_BOOL SipTxnHandler::OnSendTranspError(SipTxnKey* pTxnKey)
{
    if (pTxnKey == SIP_NULL)
    {
        return SIP_FALSE;
    }

    if (TerminateTxn(pTxnKey) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "OnSendTranspError: TerminateTxn Error", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipTxnHandler::TerminateTxn(SipTxnKey* pTxnKey)
{
    if (pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "TerminateTxn: pTxnKey NULL", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxn* pTxn = SIP_NULL;
    SipTxnKey* pOutTxnKey = SIP_NULL;
    SIP_BOOL bTxnExist =
            Sip_Cbk_ReleaseTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey), SipTxn::OPT_REMOVE,
                    reinterpret_cast<SIP_VOID**>(&pOutTxnKey), reinterpret_cast<SIP_VOID**>(&pTxn));

    if (bTxnExist == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "TerminateTxn: Txn Not existing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTxn != SIP_NULL)
    {
        if (pTxn->AbortTxn() == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "TerminateTxn: AbortTxn Fail", SIP_ZERO, SIP_ZERO);
        }

        ISipUserData* pUserData = pTxn->GetUserData();

        if (pUserData != SIP_NULL)
        {
            pUserData->SetDeleteFlag(SIP_TRUE);
        }

        /* Notifies to Transaction User using registered listener */
        NotifyTxnTermination(pTxn);

        pTxn->SipDelete();
    }

    if (pOutTxnKey != SIP_NULL)
    {
        pOutTxnKey->SipDelete();
    }

    return SIP_TRUE;
}

SIP_BOOL SipTxnHandler::DeleteTxn(SipTxnKey* pTxnKey)
{
    if (pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "DeleteTxn: pTxnKey NULL", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxn* pTxn = SIP_NULL;
    SipTxnKey* pOutTxnKey = SIP_NULL;
    SIP_BOOL bTxnExist =
            Sip_Cbk_ReleaseTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey), SipTxn::OPT_REMOVE,
                    reinterpret_cast<SIP_VOID**>(&pOutTxnKey), reinterpret_cast<SIP_VOID**>(&pTxn));

    if (bTxnExist == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "TerminateTxn: Txn Not existing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTxn != SIP_NULL)
    {
        /* Notifies to Transaction User using registered listener */
        NotifyTxnTermination(pTxn);

        pTxn->SipDelete();
    }

    if (pOutTxnKey != SIP_NULL)
    {
        pOutTxnKey->SipDelete();
    }

    return SIP_TRUE;
}

SIP_BOOL SipTxnHandler::UpdateTxnDetails(
        SipTxnKey* pTxnKey, SipTransportInfo* pTranspInfo, SIP_UINT16* pnError)
{
    if (pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "UpdateTxnDetails: Key is Null \n", SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_TRUE;
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "UpdateTxnDetails: GetTxnObjFromDb failure \n",
                SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    if (bTxnExist == SIP_NO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "UpdateTxnDetails: GetTxnObjFromDb no object \n",
                SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    pTxn->Increment();
    pTxn->UpdateTranspInfo(pTranspInfo);
    pTxn->SipDelete();
    return SIP_TRUE;
}

PRIVATE SIP_INT32 SipTxnHandler::GetTxnType(
        SIP_INT32 eMsgDir, SIP_INT32 eMethodType, SIP_INT32 eMsgType)
{
    if ((eMsgDir == SipTxn::INVALID_DIR) || (eMethodType == SipMessage::METHOD_INVALID) ||
            (eMsgType == SipMessage::TYPE_INVALID))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "GetTxnType: ETXN_INVALID", SIP_ZERO, SIP_ZERO);
        return SipTxn::INVALID_TXN;
    }

    if (eMsgDir == SipTxn::SEND)
    {
        if (eMsgType == SipMessage::REQ_TYPE)
        {
            if ((eMethodType == SipMessage::METHOD_INVITE) ||
                    (eMethodType == SipMessage::METHOD_ACK))
            {
                return SipTxn::INV_CLI_TXN;
            }
            else
            {
                return SipTxn::NON_INV_CLI_TXN;
            }
        }
        else
        {
            if (eMethodType == SipMessage::METHOD_INVITE)
            {
                return SipTxn::INV_SER_TXN;
            }
            else
            {
                return SipTxn::NON_INV_SER_TXN;
            }
        }
    }
    else /* eMsgDir is SipTxn::RECV */
    {
        if (eMsgType == SipMessage::REQ_TYPE)
        {
            if ((eMethodType == SipMessage::METHOD_INVITE) ||
                    (eMethodType == SipMessage::METHOD_ACK))
            {
                return SipTxn::INV_SER_TXN;
            }
            else
            {
                return SipTxn::NON_INV_SER_TXN;
            }
        }
        else
        {
            if (eMethodType == SipMessage::METHOD_INVITE)
            {
                return SipTxn::INV_CLI_TXN;
            }
            else
            {
                return SipTxn::NON_INV_CLI_TXN;
            }
        }
    }

    return SipTxn::INVALID_TXN;
}

PRIVATE SIP_BOOL SipTxnHandler::GetTxnObjFromDb(
        SipTxnKey* pTxnKey, SipTxn** ppTxn, SIP_BOOL* pbTxnExist, SIP_UINT16* pnError)
{
    SIP_BOOL bTxnExist = Sip_Cbk_FetchTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey),
            SipTxn::OPT_FETCH, SIP_NULL, reinterpret_cast<SIP_VOID**>(ppTxn));

    if ((bTxnExist == SIP_YES) && (*ppTxn == SIP_NULL))
    {
        /*  stack error */
        *pnError = ETXN_STACKERROR;
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "GetTxnObjFromDb: Stack Error,Db Status Error\n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    *pbTxnExist = bTxnExist;
    return SIP_TRUE;
}

PRIVATE SIP_BOOL SipTxnHandler::GetTxnObjFromDb(SipTxnKey* pTxnKey, SipTxn** ppTxn,
        SipTxnKey** ppOutTxnKey, SIP_BOOL* pbTxnExist, SIP_UINT16* pnError)
{
    SIP_BOOL bTxnExist =
            Sip_Cbk_FetchTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey), SipTxn::OPT_FETCH,
                    reinterpret_cast<SIP_VOID**>(ppOutTxnKey), reinterpret_cast<SIP_VOID**>(ppTxn));

    if ((bTxnExist == SIP_YES) && (*ppTxn == SIP_NULL))
    {
        /*  stack error */
        *pnError = ETXN_STACKERROR;
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "GetTxnObjFromDb: Stack Error,Db Status Error\n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    *pbTxnExist = bTxnExist;
    return SIP_TRUE;
}

PRIVATE SIP_BOOL SipTxnHandler::HandleClientTxnSend(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
        IN_OUT SipTxnFsmData* pTxnFsmData, OUT SIP_UINT16* pnError)
{
    SipTimerContext* pSipTimerContext = SIP_NULL;

    if (pTxnFsmData->m_pUserData != SIP_NULL)
    {
        SIP_VOID* pUserData = pTxnFsmData->m_pUserData->GetUserData();
        if (pUserData != SIP_NULL)
        {
            pSipTimerContext = (static_cast<SipTxnContext*>(pUserData))->pSipTimerContext;
        }
    }

    SipTxn* pTxn =
            new SipTxn(eTxnType, pTxnKey, pTxnFsmData->m_pSipMsgIn, pSipTimerContext, pnError);

    /* checking for txn object null or error parameter value from txn constructor */
    if ((pTxn == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "HandleClientTxnSend --> memory fail", SIP_ZERO, SIP_ZERO);
        if (pTxn != SIP_NULL)
        {
            pTxn->SipDelete();
        }
        return SIP_FALSE;
    }

    SIP_UINT16 nEvent;

    if (eTxnType == SipTxn::INV_CLI_TXN)
    {
        nEvent = SipTxn::INV_CLI_SEND_INV_REQ_EVT;
    }
    else
    {
        nEvent = SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT;
    }

    if (pTxn->InvokeFsm(nEvent, pTxnFsmData, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "HandleClientTxnSend -> FSM handling fail eTxnType[%d] nEvent[%d]", eTxnType,
                nEvent);
        pTxn->SipDelete();
        return SIP_FALSE;
    }

    pTxn->SipDelete();
    return SIP_TRUE;
}

PRIVATE SIP_BOOL SipTxnHandler::HandleServerTxnSend(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
        IN_OUT SipTxnFsmData* pTxnFsmData, IN SIP_UINT16* pnError)
{
    SipTxn* pTxn = SIP_NULL;
    SipTxnKey* pOutTxnKey = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;

    /* Get Txn object from txn DB if exists */
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &pOutTxnKey, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "HandleClientTxnRecv:GetTxnObjFromDb Fails \n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if ((pTxn == SIP_NULL) || (bTxnExist == SIP_FALSE))
    {
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTXN, "HandleClientTxnRecv:No Txn Exists", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pTxn->Increment();

    SIP_UINT16 nStatusCode = pTxnFsmData->m_pSipMsgIn->GetStatusCode();
    SIP_UINT16 nEvent;
    if (eTxnType == SipTxn::INV_SER_TXN)
    {
        nEvent = GetInvSerFsmEvt(nStatusCode);

        pTxn->SetResponseCode(nStatusCode);

        if (pOutTxnKey != SIP_NULL)
        {
            pOutTxnKey->SetResponseCode(nStatusCode);
        }
    }
    else
    {
        nEvent = GetNonInvSerFsmEvt(nStatusCode);
    }

    if ((nEvent == SipTxn::INV_SER_INVALID_EVT) || (nEvent == SipTxn::NON_INV_SER_INVALID_EVT))
    {
        pTxn->SipDelete();
        *pnError = EMSGERR_INVALIDSTATUSCODE;
        return SIP_FALSE;
    }

    // Update SIP server transaction's timers
    if (pTxnFsmData->m_pUserData != SIP_NULL)
    {
        SIP_VOID* pUserData = pTxnFsmData->m_pUserData->GetUserData();

        if (pUserData != SIP_NULL)
        {
            SipTimerContext* pTimerContext =
                    (static_cast<SipTxnContext*>(pUserData))->pSipTimerContext;

            if (pTimerContext != SIP_NULL)
            {
                SipTxnTimerValues& objTimerValues =
                        const_cast<SipTxnTimerValues&>(pTxn->GetSipTxnTimers());
                objTimerValues.UpdateSipTimers(
                        pTimerContext->nTimerOptions, pTimerContext->pTxnSipTxnTimers);
            }
        }
    }

    SIP_BOOL bStatus = pTxn->InvokeFsm((SIP_UINT16)nEvent, pTxnFsmData, pnError);

    pTxn->SipDelete();
    return bStatus;
}

PRIVATE SIP_BOOL SipTxnHandler::HandleClientTxnRecv(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
        IN_OUT SipTxnFsmData* pTxnFsmData, OUT SIP_UINT16* pnError)
{
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;

    /* Get Txn object from txn DB if exists */
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "HandleClientTxnRecv:GetTxnObjFromDb Fails \n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if ((pTxn == SIP_NULL) || (bTxnExist == SIP_FALSE))
    {
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTXN, "HandleClientTxnRecv:No Txn Exists", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pTxn->Increment();

    /* Received Response : Generate Event based on response code */
    SIP_UINT16 nStatusCode = pTxnFsmData->m_pSipMsgIn->GetStatusCode();
    SIP_UINT16 nEvent;
    if (eTxnType == SipTxn::INV_CLI_TXN)
    {
        nEvent = GetInvCliFsmEvt(nStatusCode);
    }
    else
    {
        nEvent = GetNonInvCliFsmEvt(nStatusCode);
    }

    if ((nEvent == SipTxn::INV_CLI_INVALID_EVT) || (nEvent == SipTxn::NON_INV_CLI_INVALID_EVT))
    {
        pTxn->SipDelete();

        *pnError = EMSGERR_INVALIDSTATUSCODE;
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "HandleClientTxnRecv, Invalid Status Code", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    SIP_BOOL bStatus = pTxn->InvokeFsm(nEvent, pTxnFsmData, pnError);

    pTxn->SipDelete();

    return bStatus;
}

PRIVATE SIP_BOOL SipTxnHandler::HandleServerTxnRecv(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
        IN_OUT SipTxnFsmData* pTxnFsmData, OUT SIP_UINT16* pnError)
{
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;

    /* Get Txn object from txn DB if exists */
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "HandleClientTxnRecv:GetTxnObjFromDb Fails \n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTxn != SIP_NULL)
    {
        pTxn->Increment();
    }

    SIP_UINT16 nEvent = SIP_ZERO;
    SIP_INT32 eMethod = pTxnFsmData->m_pSipMsgIn->GetMethodType();
    if (eTxnType == SipTxn::INV_SER_TXN)
    {
        if (eMethod == SipMessage::METHOD_INVITE)
        {
            nEvent = SipTxn::INV_SER_RECV_INV_REQ_EVT;
        }
        else
        {
            nEvent = SipTxn::INV_SER_RECV_ACK_REQ_EVT;

            SipTxnKey* pInviteTxnKey = (pTxn != SIP_NULL) ? pTxn->GetTxnKey() : SIP_NULL;

            if (pInviteTxnKey != SIP_NULL)
            {
                pTxnKey->SetResponseCode(pInviteTxnKey->GetResponseCode());
            }
        }
    }
    else
    {
        nEvent = SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT;
    }

    if (pTxn == SIP_NULL)
    {
        SipTimerContext* pSipTimerContext = SIP_NULL;
        if (pTxnFsmData->m_pUserData != SIP_NULL)
        {
            SIP_VOID* objUserData = pTxnFsmData->m_pUserData->GetUserData();
            if (objUserData != SIP_NULL)
            {
                pSipTimerContext = (static_cast<SipTxnContext*>(objUserData))->pSipTimerContext;
            }
        }

        /* This is new request(INV or non-INV) case and hence create the transaction obj */
        pTxn = new SipTxn(eTxnType, pTxnKey, pTxnFsmData->m_pSipMsgIn, pSipTimerContext, pnError);

        if (pTxn == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxn: malloc fails", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    /* Invoking Server FSM */
    if (pTxn->InvokeFsm(nEvent, pTxnFsmData, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "HandleServerTxnRecv -> FSM handling fail eTxnType[%d] nEvent[%d]", eTxnType,
                nEvent);

        pTxn->SipDelete();

        return SIP_FALSE;
    }

    pTxn->SipDelete();
    return SIP_TRUE;
}

PRIVATE SIP_VOID SipTxnHandler::NotifyTxnTermination(IN SipTxn* pTxn)
{
    SipUtil* pUtil = SipUtil_GetInstance();
    if (pUtil == SIP_NULL)
    {
        return;
    }

    ISipUserData* pUserData = pTxn->GetUserData();
    ISipTxnListener* pTxnListener = pUtil->GetTxnListener();

    if (pTxnListener != SIP_NULL)
    {
        pTxnListener->TxnTerminated(pUserData);
    }
}

PRIVATE SIP_BOOL SipTxnHandler::ValidateRecvTxn(SipMessage* pSipMsg, IN SIP_INT32* peTxnType OUT)
{
    SIP_INT32 eMsgType = SipMessage::TYPE_INVALID;
    SIP_INT32 eMethodType = SipMessage::METHOD_INVALID;

    /* Check if it's proper to start Transaction Ref: RFC 3261 8.1.1.*/
    if (SipMessage::CheckTxnMandatoryParams(pSipMsg, &eMsgType, &eMethodType) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "ValidateRecvTxn: CheckTxnMandatoryParams fails\n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    *peTxnType = GetTxnType(SipTxn::RECV, eMethodType, eMsgType);

    if (*peTxnType == SipTxn::INVALID_TXN)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "ValidateRecvTxn: Invalid Txn", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

PRIVATE SIP_BOOL SipTxnHandler::ValidateSendTxn(IN SipMessage* pSipMsg, OUT SIP_INT32* peTxnType,
        OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError)
{
    SIP_INT32 eMsgType = SipMessage::TYPE_INVALID;
    SIP_INT32 eMethodType = SipMessage::METHOD_INVALID;

    /* Check if it's proper to start Transaction Ref: RFC 3261 8.1.1.*/
    if (SipMessage::CheckTxnMandatoryParams(pSipMsg, &eMsgType, &eMethodType) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnHandler: CheckTxnMandatoryParams fails\n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_INT32 eTxnType = GetTxnType(SipTxn::SEND, eMethodType, eMsgType);

    if (eTxnType == SipTxn::INVALID_TXN)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnHandler: Invalid Txn\n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, pnError);
    if (pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxnHandler:key Creation Fails \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    else if (*pnError == EERR_MALLOCFAILED)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxnHandler:key Creation Fails \n", SIP_ZERO, SIP_ZERO);
        pTxnKey->SipDelete();
        return SIP_FALSE;
    }

    *peTxnType = eTxnType;
    *ppTxnKey = pTxnKey;

    return SIP_TRUE;
}

static SIP_INT32 GetNonInvCliFsmEvt(SIP_UINT16 nStatusCode)
{
    if (SipMsgUtil::IsProvisionalResponse(nStatusCode))
    {
        return SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT;
    }
    else if (SipMsgUtil::IsNonProvisionalResponse(nStatusCode))
    {
        return SipTxn::NON_INV_CLI_RECV_2XX_6XX_RESP_EVT;
    }
    else
    {
        return SipTxn::NON_INV_CLI_INVALID_EVT;
    }
}

static SIP_INT32 GetNonInvSerFsmEvt(SIP_UINT16 nStatusCode)
{
    if (SipMsgUtil::IsProvisionalResponse(nStatusCode))
    {
        return SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT;
    }
    else if (SipMsgUtil::IsNonProvisionalResponse(nStatusCode))
    {
        return SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT;
    }
    else
    {
        return SipTxn::NON_INV_SER_INVALID_EVT;
    }
}

static SIP_INT32 GetInvCliFsmEvt(SIP_UINT16 nStatusCode)
{
    if (SipMsgUtil::IsProvisionalResponse(nStatusCode))
    {
        return SipTxn::INV_CLI_RECV_1XX_RESP_EVT;
    }
    else if (SipMsgUtil::IsSuccessfulResponse(nStatusCode))
    {
        return SipTxn::INV_CLI_RECV_2XX_RESP_EVT;
    }
    else if (SipMsgUtil::IsFailureResponse(nStatusCode))
    {
        return SipTxn::INV_CLI_RECV_3XX_6XX_RESP_EVT;
    }
    else
    {
        return SipTxn::INV_CLI_INVALID_EVT;
    }
}

static SIP_INT32 GetInvSerFsmEvt(SIP_UINT16 nStatusCode)
{
    if (SipMsgUtil::IsProvisionalResponse(nStatusCode))
    {
        return SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT;
    }
    else if (SipMsgUtil::IsSuccessfulResponse(nStatusCode))
    {
        return SipTxn::INV_SER_SEND_2XX_SUCCESS_RESP_EVT;
    }
    else if (SipMsgUtil::IsFailureResponse(nStatusCode))
    {
        return SipTxn::INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT;
    }
    else
    {
        return SipTxn::INV_SER_INVALID_EVT;
    }
}
