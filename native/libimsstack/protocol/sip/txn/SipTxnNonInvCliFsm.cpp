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
#include "platform/SipString.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"

static SIP_BOOL NonInvCliFsm_NullFxn(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pTxn;
    (void)pvData;

    SIP_DEBUG_STACKBUG(
            ESIPTRACE_MODTXN, "NonInvCliFsm_NullFxn: Invalid Handling", SIP_ZERO, SIP_ZERO);
    return SIP_FALSE;
}

static SIP_BOOL NonInvClient_TimeoutHandling(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
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
        SIP_BOOL bTimerStatus = pTxn->StartTxnTimer(SipTxn::TIMER_E, nDuration, pnError);
        if (bTimerStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "NonInvCli_Recv2xx6xxResp:StartTxnTimer E, Failed\n", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->IncreaseTxnCount();
        pTxn->IncreaseDurationExpired(nDuration);
        pTxn->SetCurrentDuration(nDuration);
    }

    return SIP_TRUE;
}

static SIP_BOOL NonInvCli_Recv2xx6xxResp(
        SipTxn* pTxn, SIP_UINT16* pNewTxnState, SIP_UINT16* pnError)
{
    pTxn->StopTxnTimer();

    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();

    /* For Unreliable Transport */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        /* Start Timer K */
        const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
        SIP_UINT32 nDurationTK = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_K);
        SIP_BOOL bStatus = pTxn->StartTxnTimer(SipTxn::TIMER_K, nDurationTK, pnError);
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "NonInvCli_Recv2xx6xxResp:StartTxnTimer K, Failed\n", SIP_ZERO, SIP_ZERO);
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

static SIP_BOOL NonInvCliFsm_IdleStSendNonInvReqEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvCliFsm_IdleStSendNonInvReqEvt:pNewTxnKey memory fail", SIP_ZERO, SIP_ZERO);

        if (pNewTxnKey != SIP_NULL)
        {
            pNewTxnKey->SipDelete();
        }
        return SIP_FALSE;
    }

    SIP_BOOL bStatus = Sip_Cbk_FetchTransaction(reinterpret_cast<SIP_VOID*>(pNewTxnKey),
            SipTxn::OPT_CREATE, SIP_NULL, reinterpret_cast<SIP_VOID**>(&pTxn));

    if (bStatus == SIP_FALSE)
    {
        pNewTxnKey->SipDelete();
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvCliFsm_IdleStSendNonInvReqEvt:Adding Txn into DB Fails \n", SIP_ZERO,
                SIP_ZERO);

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
    SipTransportParameter* pTranspParam = pFsmData->m_pTranspParam;
    SIP_INT32 eTranspProtocol = pTranspParam->GetTranspProtocol();

    /* For Unreliable Transport : Start Timer E*/
    if (eTranspProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        bStatus = pTxn->StartTxnTimer(SipTxn::TIMER_E, nDurationT1, pnError);
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "NonInvCliFsm_IdleStSendNonInvReqEvt:StartTxnTimer:Failed \n", SIP_ZERO,
                    SIP_ZERO);
            pTxn->RemoveFromTxnPool();
            return SIP_FALSE;
        }
    }
    else /* For Reliable Transport : Start Timer F*/
    {
        bStatus = pTxn->StartTxnTimer(SipTxn::TIMER_F, nDurationTF, pnError);
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "NonInvCliFsm_IdleStSendNonInvReqEvt:StartTxnTimer:Failed \n", SIP_ZERO,
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
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "pFsmData->m_pUserData is NULL", SIP_ZERO, SIP_ZERO);
    }

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->bTxnCreated = SIP_TRUE;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TRYING_ST);

    if (SipPf_Strcmp(pNewTxnKey->GetMethod(), CANCEL_METHOD) == SIP_EQUALS)
    {
        SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());
    }
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_TryingStTimer_E_F_TimeoutEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* handling of timeout and state transition occure inside the function */
    SIP_BOOL bStatus = NonInvClient_TimeoutHandling(pTxn, pvData, pnError);

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "NonInvCli:Timeout handling failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_TryingStRecv1xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_PROCEEDING_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_TryingStRecv2xx6xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    /* state transition occur inside the function */
    SIP_BOOL bStatus = NonInvCli_Recv2xx6xxResp(pTxn, &nNewTxnState, pnError);
    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "NonInvCli:2xx-6xx handling failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Fill FSM data for stack manager */
    if (nNewTxnState == SipTxn::NON_INV_CLI_TERMINATED_ST)
    {
        pFsmData->bTxnTerminated = SIP_TRUE;
    }

    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    pTxn->SetTxnState(nNewTxnState);

    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_TryingStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;

    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_ProceedingStTimer_E_F_TimeoutEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* handling of timeout and state transition occure inside the function */
    SIP_BOOL bStatus = NonInvClient_TimeoutHandling(pTxn, pvData, pnError);

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvCliFsm_ProceedingStTimer_E_F_TimeoutEvt:Timeout handling failed \n",
                SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_ProceedingStRecv1xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_ProceedingStRecv2xx6xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    /* state transition occur inside the function */
    SIP_BOOL bStatus = NonInvCli_Recv2xx6xxResp(pTxn, &nNewTxnState, pnError);
    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvCliFsm_ProceedingStRecv2xx6xxRespEvt:Resp handling failed \n", SIP_ZERO,
                SIP_ZERO);

        return SIP_FALSE;
    }

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* Fill FSM data for stack manager */
    if (nNewTxnState == SipTxn::NON_INV_CLI_TERMINATED_ST)
    {
        pFsmData->bTxnTerminated = SIP_TRUE;
    }

    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pTxn->SetTxnState(nNewTxnState);
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_ProceedingStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_CompletedStTimer_K_TimeoutEvt(
        SipTxn* pTxn, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    /* State Transition */
    pTxn->SetTxnState(SipTxn::NON_INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_CompletedStRecv1xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->eTxnStatus = SipTxn::STATUS_IGNORE_RESP;

    /* remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_CompletedStRecv2xx6xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->eTxnStatus = SipTxn::STATUS_IGNORE_RESP;

    /* remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL NonInvCliFsm_CompletedStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
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
        NonInvCliFsm_IdleStSendNonInvReqEvt, /* SendNonInvReq_Evt */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_RECV_2XX_6XX_RESP_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn

    },
    /* TRYING State:: S1*/
    {
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        NonInvCliFsm_TryingStTimer_E_F_TimeoutEvt, /* Timer_E_F_TimeoutEvt */
        NonInvCliFsm_TryingStRecv1xxRespEvt, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        NonInvCliFsm_TryingStRecv2xx6xxRespEvt, /* Recv2xx6xxRespEvt */
        NonInvCliFsm_TryingStTranspErrorEvt, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn

    },
    /* PROCEEDING State:: S2*/
    {
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        NonInvCliFsm_ProceedingStTimer_E_F_TimeoutEvt, /* Timer_E_F_TimeoutEvt */
        NonInvCliFsm_ProceedingStRecv1xxRespEvt, /* Recv1xxRespEvt */
        NonInvCliFsm_ProceedingStRecv2xx6xxRespEvt, /* Recv2xx6xxRespEvt */
        NonInvCliFsm_ProceedingStTranspErrorEvt, /* TranspErrorEvt */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn

    },
    /* COMPLETED State:: S3*/
    {
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        NonInvCliFsm_CompletedStRecv1xxRespEvt, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        NonInvCliFsm_CompletedStRecv2xx6xxRespEvt, /* SipTxn::NON_INV_CLI_RECV_2XX_6XX_RESP_EVT */
        NonInvCliFsm_CompletedStTranspErrorEvt, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT*/
        NonInvCliFsm_CompletedStTimer_K_TimeoutEvt, /* Timer_K_TimeoutEvt*/
        NonInvCliFsm_NullFxn
    },
    /* TERMINATED State:: S4*/
    {
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_RECV_2XX_6XX_RESP_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT*/
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn
    },
    /* Invalid State:: S5*/
    {
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_SEND_NON_INV_REQ_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_RECV_1XX_RESP_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_RECV_2XX_6XX_RESP_EVT */
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TRANSP_ERROR_EVT*/
        NonInvCliFsm_NullFxn, /* SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT */
        NonInvCliFsm_NullFxn
    }
};
// clang-format on
