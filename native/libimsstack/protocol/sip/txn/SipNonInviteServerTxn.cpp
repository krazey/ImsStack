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
#include "SipDatatypes.h"
#include "SipDebug.h"
#include "SipStackError.h"
#include "SipUtil.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"

static SIP_BOOL HandleInvalidStateEvent(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "Unhandled state and event.", SIP_ZERO, SIP_ZERO);
    return SIP_FALSE;
}

static SIP_BOOL SendFinalResponse(
        SipTxn* pTxn, void* pvData, SIP_UINT16* pnNewTxnState, SIP_UINT16* pnError)
{
    const SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    const SipTransportParameter* pTransparam = pFsmData->m_pTranspParam;

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationTJ = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_J);

    SIP_INT32 eTranspMsgSentProtocol = pTransparam->GetTranspProtocol();

    /* For Unreliable Transport : Start Timer J*/
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_J, nDurationTJ, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SendFinalResponse: Starting Timer_J failed.",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->SetCurrentDuration(nDurationTJ);
        *pnNewTxnState = SipTxn::NON_INV_SER_COMPLETED_ST;
    }
    /* For Reliable Transport : Terminate Txn */
    else
    {
        *pnNewTxnState = SipTxn::NON_INV_SER_TERMINATED_ST;
        return SIP_TRUE;
    }
    return SIP_TRUE;
}

static SIP_BOOL IdleState_ReceiveNonInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "IdleState_ReceiveNonInviteRequest: SipTxnKey memory allocation failed.", SIP_ZERO,
                SIP_ZERO);

        if (pNewTxnKey != SIP_NULL)
        {
            pNewTxnKey->SipDelete();
        }
        return SIP_FALSE;
    }

    SIP_BOOL bStatus = SIP_FALSE;
    ISipTransactionCallback* pCallback = SipUtil::GetInstance()->GetTransactionCallback();

    if (pCallback != SIP_NULL)
    {
        bStatus = pCallback->FetchTransaction(pNewTxnKey, SipTxn::OPT_CREATE, pTxn);
    }

    if (bStatus == SIP_FALSE)
    {
        pNewTxnKey->SipDelete();
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "IdleState_ReceiveNonInviteRequest: Adding Txn to DB failed.", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* TxnObj is added to hash, hence increment ref count */
    pTxn->Increment();

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Set user data in Txn Obj */
    if (pFsmData->m_pUserData != SIP_NULL)
    {
        SIP_VOID* pvTUdata = pFsmData->m_pUserData->GetUserData();
        ISipUserData* pUserData = new ISipUserData(pvTUdata);
        if (pUserData == SIP_NULL)
        {
            pTxn->RemoveFromTxnPool();
            return SIP_FALSE;
        }
        pTxn->SetUserData(pUserData);
    }

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_bTxnCreated = SIP_TRUE;
    pFsmData->m_eTxnStatus = SipTxn::STATUS_NEW_REQ_RECVD;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TRYING_ST);

    SipMessage* pMsgIn = pFsmData->m_pSipMsgIn;

    /*If PRACK message received, stop 18x retransmission timer if any*/
    if (pMsgIn->GetMethodType() == SipMessage::METHOD_PRACK)
    {
        SipTxn* pInvSerTxn = SIP_NULL;  // Corresponding INV Txn.

        SIP_UINT32 nRseqNum = SipMessage::GetRSeqNum(pMsgIn, SipHeaderBase::RACK);
        pNewTxnKey->SetRSeq(nRseqNum);
        SipTxnKey* pINVTxnKey = SipTxnUtil::SearchTxnKey(pNewTxnKey);

        if (pINVTxnKey != SIP_NULL)
        {
            SIP_BOOL bStatus = SIP_FALSE;
            ISipTransactionCallback* pCallback = SipUtil::GetInstance()->GetTransactionCallback();

            if (pCallback != SIP_NULL)
            {
                bStatus = pCallback->FetchTransaction(pINVTxnKey, SipTxn::OPT_FETCH, pInvSerTxn);
            }

            if (bStatus == SIP_TRUE)
            {
                if (pInvSerTxn != SIP_NULL)
                {
                    pInvSerTxn->StopTxnTimer();
                }
                if (SipTxnUtil::DeleteTxnKey(pNewTxnKey, SIP_TRUE) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                            "IdleState_ReceiveNonInviteRequest: Deleting txn key from list failed.",
                            SIP_ZERO, SIP_ZERO);
                }
            }
            else
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "IdleState_ReceiveNonInviteRequest: Fetching txn failed, can not stop "
                        "timer.",
                        SIP_ZERO, SIP_ZERO);
            }
        }
        else
        {
            pINVTxnKey = SipTxnUtil::SearchTxnKey(pNewTxnKey, SIP_FALSE);

            if (pINVTxnKey != SIP_NULL)
            {
                // INVITE transaction exists,
                // but the PRACK request which is not matched with RSeq is received.
                // It needs to send 481 response.
                pFsmData->m_eTxnStatus = SipTxn::STATUS_STRAY_PRACK;
            }
            else
            {
                // INVITE server transaction is terminated by sending 200 OK,
                // but UA MUST be prepared to process PRACK request
                // if the processing of UA core is possible.
            }
        }
    }
    return SIP_TRUE;
}

static SIP_BOOL TryingState_ReceiveNonInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is receive of re-transmitted non-INVITE request.
       since previous req is already passed to TU and response is not been send, ignore the request
     */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_IGNORE_REQ;

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL TryingState_Send1xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    pTxn->SetTxnState(SipTxn::NON_INV_SER_PROCEEDING_ST);
    return SIP_TRUE;
}

static SIP_BOOL TryingState_SendFinalResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    /* handling of response and state transition occur inside the fxn */
    if (SendFinalResponse(pTxn, pvData, &nNewTxnState, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "TryingState_SendFinalResponse failed.", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Fill FSM data for stack manager */
    if (nNewTxnState == SipTxn::NON_INV_SER_TERMINATED_ST)
    {
        pFsmData->m_bTxnTerminated = SIP_TRUE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();
    }
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    pTxn->SetTxnState(nNewTxnState);

    return SIP_TRUE;
}

static SIP_BOOL TryingState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_ReceiveNonInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is receive of re-transmitted non-INVITE request.
       stack manager to send last response */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Send1xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pTxn;
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* No Change in State */
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_SendFinalResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState;

    /* handling of response and state transition occur inside the fxn */
    if (SendFinalResponse(pTxn, pvData, &nNewTxnState, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "ProceedingState_SendFinalResponse failed.", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Fill FSM data for stack manager */
    if (nNewTxnState == SipTxn::NON_INV_SER_TERMINATED_ST)
    {
        pFsmData->m_bTxnTerminated = SIP_TRUE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();
    }
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    pTxn->SetTxnState(nNewTxnState);
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_ReceiveNonInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is receive of re-transmitted non-INVITE request.
       stack manager to send last response */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData, (void)pTxn;
    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_Timer_J_Timeout(
        SipTxn* pTxn, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    /* State Transition */
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

// clang-format off
SIP_BOOL (*gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_INVALID_ST + 1]
                                [SipTxn::NON_INV_SER_INVALID_EVT + 1])
(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError) = {
    /* Idle State:: S0*/
    {
        IdleState_ReceiveNonInviteRequest, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* TRYING State:: S1*/
    {
        TryingState_ReceiveNonInviteRequest, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        TryingState_Send1xxResponse, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        TryingState_SendFinalResponse, /* SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT */
        TryingState_TransportError, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* PROCEEDING State:: S2*/
    {
        ProceedingState_ReceiveNonInviteRequest, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        ProceedingState_Send1xxResponse, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        ProceedingState_SendFinalResponse, /* SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT */
        ProceedingState_TransportError, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* COMPLETED State:: S3*/
    {
        CompletedState_ReceiveNonInviteRequest, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT */
        CompletedState_TransportError, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        CompletedState_Timer_J_Timeout, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* TERMINATED State:: S4*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* Invalid State:: S5*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_SEND_FINAL_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        HandleInvalidStateEvent
    }
};
// clang-format on
