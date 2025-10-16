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
#include "platform/SipString.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"

static SIP_BOOL HandleInvalidStateEvent(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pTxn;
    (void)pvData;

    SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "Unhandled state and event.", SIP_ZERO, SIP_ZERO);
    return SIP_FALSE;
}

static SIP_BOOL HandleTimeout(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pvData;

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nT1Val = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);

    SIP_UINT32 nDurationExpired = pTxn->GetDurationExpired();
    if (nDurationExpired == 0)
    {
        // Timer T1 already fired.
        pTxn->IncreaseDurationExpired(nT1Val);
        nDurationExpired = nT1Val;
    }

    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();
    SIP_UINT32 nMaxDuration = pTxn->GetMaxDuration();

    SIP_UINT32 nDuration = SIP_ZERO;
    /* For Unreliable Transport */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        /* Check Max Retransmission Limit or Total expired
           time has crossed the Max duration limit or not */
        if (nDurationExpired >= nMaxDuration)
        {
            /* Stop Retransmissions : May notify StackUSer on Termination of Txn */
            pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
            nDuration = SIP_ZERO;  // Transaction to be timedout immediately.
        }
        else
        {
            /* Restarting of Timer is based on the current state
               RFC 3261: Sec17.1.2.1
               If timer E fires while Trying state, then MIN(2*T1, T2).
               If Timer E fires while in the "Proceeding" state,then reset with a value of T2
               seconds.
            */
            SIP_UINT16 nCurTxnState = pTxn->GetTxnState();
            SIP_UINT32 nDurationT2 = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T2);
            if (nCurTxnState == SipTxn::NON_INV_CLI_TRYING_ST)
            {
                SIP_UINT32 nCurrentDuration = pTxn->GetCurrentDuration();
                SIP_UINT32 nNextDuration = nCurrentDuration << 1;

                /* MIN(2*T1, T2) seconds*/
                nDuration = SIP_MIN(nNextDuration, nDurationT2);
            }
            else /* SipTxn::NON_INV_CLI_PROCEEDING_ST */
            {
                nDuration = nDurationT2;
            }

            // Update the timer duration.
            if ((nDurationExpired + nDuration) >= nMaxDuration)
            {
                nDuration = nMaxDuration - nDurationExpired;
            }
        }
    }
    else /* For Reliable Transport */
    {
        pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
        nDuration = SIP_ZERO;  // Transaction to be timedout immediately.
    }

    /* Start Timer E with updated Duration */
    if (nDuration > SIP_ZERO)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_E, nDuration, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "HandleTimeout: Starting Timer_E failed.", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->IncreaseTxnCount();
        pTxn->IncreaseDurationExpired(nDuration);
        pTxn->SetCurrentDuration(nDuration);
    }

    return SIP_TRUE;
}

static SIP_BOOL HandleFinalResponse(SipTxn* pTxn, SIP_UINT16* pNewTxnState, SIP_UINT16* pnError)
{
    pTxn->StopTxnTimer();

    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();

    /* For Unreliable Transport */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        /* Start Timer K */
        const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
        SIP_UINT32 nDurationTK = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_K);

        if (pTxn->StartTxnTimer(SipTxn::TIMER_K, nDurationTK, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "HandleFinalResponse: Starting Timer_K failed.",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->SetCurrentDuration(nDurationTK);
        *pNewTxnState = SipTxn::NON_INV_CLI_COMPLETED_ST;
    }
    else /* For Reliable Transport */
    {
        *pNewTxnState = SipTxn::NON_INV_CLI_TERMINATED_ST;
        return SIP_TRUE;
    }

    return SIP_TRUE;
}

static SIP_BOOL IdleState_SendNonInviteRequest(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "IdleState_SendNonInviteRequest:  SipTxnKey memory allocation failed.", SIP_ZERO,
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
                "IdleState_SendNonInviteRequest: Adding Txn to DB failed.", SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    /* TxnObj is added to hash, hence increment ref count */
    pTxn->Increment();

    /* Start Timer
       1. Retx Timer : for UDP only
       2. Transaction Timer : for TCP
     */
    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationT1 = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);
    SIP_UINT32 nDurationTF = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_F);
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    const SipTransportParameter* pTranspParam = pFsmData->m_pTranspParam;
    SIP_INT32 eTranspProtocol = pTranspParam->GetTranspProtocol();

    /* For Unreliable Transport : Start Timer E*/
    if (eTranspProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_E, nDurationT1, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "IdleState_SendNonInviteRequest:  Starting Timer_E failed.", SIP_ZERO,
                    SIP_ZERO);
            pTxn->RemoveFromTxnPool();
            return SIP_FALSE;
        }
    }
    else /* For Reliable Transport : Start Timer F*/
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_F, nDurationTF, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "IdleState_SendNonInviteRequest:  Starting Timer_F failed.", SIP_ZERO,
                    SIP_ZERO);
            pTxn->RemoveFromTxnPool();
            return SIP_FALSE;
        }
    }

    pTxn->SetMaxDuration(nDurationTF);

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

    /* State Transition */
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TRYING_ST);

    if (SipPf_Strcmp(pNewTxnKey->GetMethod(), SipMsgUtil::METHOD_CANCEL) == SIP_EQUALS)
    {
        SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());
    }
    return SIP_TRUE;
}

static SIP_BOOL TryingState_Timer_E_F_Timeout(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* handling of timeout and state transition occure inside the function */
    if (HandleTimeout(pTxn, pvData, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "TryingState_Timer_E_F_Timeout: Timeout handling failed.", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

static SIP_BOOL TryingState_Receive1xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_PROCEEDING_ST);
    return SIP_TRUE;
}

static SIP_BOOL TryingState_ReceiveFinalResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    /* state transition occur inside the function */
    if (HandleFinalResponse(pTxn, &nNewTxnState, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "TryingState_ReceiveFinalResponse failed.", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Fill FSM data for stack manager */
    if (nNewTxnState == SipTxn::NON_INV_CLI_TERMINATED_ST)
    {
        pFsmData->m_bTxnTerminated = SIP_TRUE;
    }

    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    pTxn->SetTxnState(nNewTxnState);

    return SIP_TRUE;
}

static SIP_BOOL TryingState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;

    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Timer_E_F_Timeout(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* handling of timeout and state transition occure inside the function */
    if (HandleTimeout(pTxn, pvData, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "ProceedingState_Timer_E_F_Timeout: Timeout handling failed.", SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Receive1xxResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_ReceiveFinalResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    /* state transition occur inside the function */
    if (HandleFinalResponse(pTxn, &nNewTxnState, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "ProceedingState_ReceiveFinalResponse failed.",
                SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Fill FSM data for stack manager */
    if (nNewTxnState == SipTxn::NON_INV_CLI_TERMINATED_ST)
    {
        pFsmData->m_bTxnTerminated = SIP_TRUE;
    }

    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pTxn->SetTxnState(nNewTxnState);
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_Timer_K_Timeout(
        SipTxn* pTxn, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    /* State Transition */
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_Receive1xxResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_IGNORE_RESP;

    /* remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_ReceiveFinalResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_IGNORE_RESP;

    /* remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);

    return SIP_TRUE;
}

// clang-format off
SIP_BOOL (*gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_INVALID_ST + 1]
                                   [SipTxn::NON_INV_CLI_INVALID_EVT + 1])
(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError) =
{
    /* Idle State:: S0*/
    {
        IdleState_SendNonInviteRequest, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        HandleInvalidStateEvent

    },
    /* TRYING State:: S1*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        TryingState_Timer_E_F_Timeout, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        TryingState_Receive1xxResponse, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        TryingState_ReceiveFinalResponse, /* SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT */
        TryingState_TransportError, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        HandleInvalidStateEvent

    },
    /* PROCEEDING State:: S2*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        ProceedingState_Timer_E_F_Timeout, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        ProceedingState_Receive1xxResponse, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        ProceedingState_ReceiveFinalResponse, /* SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT */
        ProceedingState_TransportError, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        HandleInvalidStateEvent

    },
    /* COMPLETED State:: S3*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        CompletedState_Receive1xxResponse, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        CompletedState_ReceiveFinalResponse, /* SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT */
        CompletedState_TransportError, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        CompletedState_Timer_K_Timeout, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* TERMINATED State:: S4*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* Invalid State:: S5*/
    {
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_RECV_FINAL_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        HandleInvalidStateEvent
    }
};
// clang-format on
