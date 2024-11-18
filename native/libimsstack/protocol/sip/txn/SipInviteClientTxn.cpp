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
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnTimerValues.h"
#include "txn/SipTxnUtil.h"

extern SIP_VOID* Sip_Cbk_CreateAckRequest(IN SIP_VOID* pvRespMsg, IN ISipUserData* pUserData);

static SIP_BOOL HandleInvalidStateEvent(
        IN SipTxn* pTxn, IN SIP_VOID* pvData, OUT SIP_UINT16* pnError)
{
    (void)pvData;
    (void)pTxn;
    SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "Unhandled state and event.", SIP_ZERO, SIP_ZERO);
    *pnError = ETXN_FSMEVENTERROR;
    return SIP_FALSE;
}

static SIP_BOOL HandleFailureResponse(IN SipTxn* pTxn, IN_OUT SipTxnFsmData* pFsmData,
        OUT SIP_UINT16* pNewTxnState, OUT SIP_UINT16* pnError)
{
    SipMessage* pSipAckMsg = reinterpret_cast<SipMessage*>(
            Sip_Cbk_CreateAckRequest(pFsmData->m_pSipMsgIn, pTxn->GetUserData()));

    if (pSipAckMsg == SIP_NULL)
    {
        pTxn->PrepareACK(pFsmData->m_pSipMsgIn, SIP_FALSE, &pSipAckMsg);
    }

    if (pSipAckMsg == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "HandleFailureResponse: preparing ACK failed.",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SipTxnTimerValues& pSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationTD = pSipTxnTimers.GetTimerValue(SipTxn::TIMER_D);

    /* On basis of Transport Start Timer */
    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();

    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "HandleFailureResponse: Transport %d , TimerD: %d",
            eTranspMsgSentProtocol, nDurationTD);

    /* For Unreliable Transport */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_D, nDurationTD, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "HandleFailureResponse: Starting Timer_D failed.",
                    SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
        /* State Transition */
        *pNewTxnState = SipTxn::INV_CLI_COMPLETED_ST;
    }
    else /* For Reliable Transport */
    {
        /* State Transition for Reliable Transport */
        *pNewTxnState = SipTxn::INV_CLI_TERMINATED_ST;
    }

    /* OUT Paramet Set */
    pFsmData->m_pSendSipMsg = pSipAckMsg;

    return SIP_TRUE;
}

static SIP_BOOL IdleState_SendInviteRequest(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "IdleState_SendInviteRequest: SipTxnKey memory allocation failed.", SIP_ZERO,
                SIP_ZERO);

        if (pNewTxnKey != SIP_NULL)
        {
            pNewTxnKey->SipDelete();
        }
        return SIP_FALSE;
    }

    if (Sip_Cbk_FetchTransaction(reinterpret_cast<SIP_VOID*>(pNewTxnKey), SipTxn::OPT_CREATE,
                SIP_NULL, reinterpret_cast<SIP_VOID**>(&pTxn)) == SIP_FALSE)
    {
        pNewTxnKey->SipDelete();
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "IdleState_SendInviteRequest: Adding txn to DB failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* TxnObj is added to hash, hence increment ref count */
    pTxn->Increment();

    /* Start Timers
       1. Retx Timer : Retransmission Timer for UDP only
       2. Txn Timer  : Req Timeout Timer for any Transport
     */
    const SipTxnTimerValues& pSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationT1 = pSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);
    SIP_UINT32 nDurationTB = pSipTxnTimers.GetTimerValue(SipTxn::TIMER_B);
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    SipTransportParameter* pTranspParam = pFsmData->m_pTranspParam;
    SIP_INT32 eTranspProtocol = pTranspParam->GetTranspProtocol();

    /* For Unreliable Transport : Start Timer A*/
    if (eTranspProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_A, nDurationT1, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "IdleState_SendInviteRequest: Starting Timer_A failed", SIP_ZERO, SIP_ZERO);

            pTxn->RemoveFromTxnPool();
            return SIP_FALSE;
        }
    }
    else /* For Reliable Transport : Start Timer B*/
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_B, nDurationTB, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "IdleState_SendInviteRequest: Starting Timer_B failed.", SIP_ZERO, SIP_ZERO);

            pTxn->RemoveFromTxnPool();
            return SIP_FALSE;
        }
    }
    pTxn->SetMaxDuration(nDurationTB);

    /* Set Userdata into Txn object */
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
    pTxn->SetTxnState(SipTxn::INV_CLI_CALLING_ST);

    return SIP_TRUE;
}

static SIP_BOOL CallingState_Timer_A_B_Timeout(
        SipTxn* pTxn, SIP_VOID* /*pvData*/, SIP_UINT16* pnError)
{
    SIP_UINT32 nDurationExpired = pTxn->GetDurationExpired();

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nT1Val = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);

    if (nDurationExpired == 0)
    {
        // Timer T1 already fired.
        pTxn->IncreaseDurationExpired(nT1Val);
        nDurationExpired = nT1Val;
    }

    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();
    SIP_UINT16 nReTxCount = pTxn->GetRetransmissionCount();
    SIP_UINT32 nDuration = SIP_ZERO;
    SIP_UINT32 nMaxDuration = pTxn->GetMaxDuration();

    /* For Unreliable Transport : Restart Timer */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        /* Check Max Retransmission Limit or Total expired
           time has crossed the Max duration limit or not */
        if (nDurationExpired >= nMaxDuration)
        {
            /* Stop Retransmissions : May notify StackUSer on Termination of Txn */
            pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);
            nDuration = SIP_ZERO;  // Transaction to be timedout immediately.
        }
        else
        {
            /* Start Timer A with updated Duration */
            /* RFC 3261:17.1.1.2    Request is retransmitted with
               intervals that double after each transmission.
            */
            nReTxCount = nReTxCount + SIP_ONE;
            SIP_UINT32 nPow2 = SIP_ONE << nReTxCount;
            nDuration = nPow2 * nT1Val;

            // Update the timer duration.
            if ((nDurationExpired + nDuration) >= nMaxDuration)
            {
                nDuration = nMaxDuration - nDurationExpired;
            }
        }
    }
    else /* For Reliable Transport */
    {
        /* Terminate Transaction */
        pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);
        nDuration = SIP_ZERO;  // Transaction to be timedout immediately.
    }

    /* Start Timer A with updated Duration */
    if (nDuration > SIP_ZERO)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_A, nDuration, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "CallingState_Timer_A_B_Timeout: Starting Timer_A failed.", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->IncreaseTxnCount();
        pTxn->IncreaseDurationExpired(nDuration);
        pTxn->SetCurrentDuration(nDuration);
    }

    return SIP_TRUE;
}

static SIP_BOOL CallingState_Receive1xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    /* Stop existing Txn timer */
    pTxn->StopTxnTimer();

    /* Get user data */
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    pFsmData->m_pOutUserData = pTxn->GetUserData();

    /* Fill FSM data for stack manager */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    // RPR handling for retransmission
    SipMessage* pMsgIn = pFsmData->m_pSipMsgIn;
    SIP_INT16 nStatusCode = SIP_ZERO;
    SipStatusLine* pStatusLine = pMsgIn->GetStatusLine();

    if (pStatusLine != SIP_NULL)
    {
        pStatusLine->GetStatusCode(&nStatusCode);
        pStatusLine->SipDelete();
    }

    if (nStatusCode != 100)
    {
        SIP_BOOL bRSeqExist = pMsgIn->HasHeader(SipHeaderBase::RSEQ);

        if (bRSeqExist == SIP_TRUE)
        {
            SipTxnKey* pRprTxnKey = new SipTxnKey(pMsgIn, pnError);

            if (SipTxnUtil::AddTxnKey(pRprTxnKey) == SIP_FALSE)
            {
                pRprTxnKey->SipDelete();

                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "CallingState_Receive1xxResponse: Adding RprTxnKey failed.", SIP_ZERO,
                        SIP_ZERO);
            }
        }
    }

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_CLI_PROCEEDING_ST);

    return SIP_TRUE;
}

static SIP_BOOL CallingState_Receive2xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    /* Stop existing Txn timer */
    pTxn->StopTxnTimer();

    /* Get user data */
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    pFsmData->m_pOutUserData = pTxn->GetUserData();

    /* Fill FSM data for stack manager */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pFsmData->m_bTxnTerminated = SIP_TRUE;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);

    return SIP_TRUE;
}

static SIP_BOOL CallingState_ReceiveFailureResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* Stop Timer A/B */
    pTxn->StopTxnTimer();

    /* Prepare failure ACK message and fill into pTxnFsmData.
       state txn is done in this fxn */
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    if (HandleFailureResponse(pTxn, pFsmData, &nNewTxnState, pnError) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    /* NOTE:
       pTxnFsmData is updated with failure ACK message that needs to be send to network.
     */

    if (nNewTxnState == SipTxn::INV_CLI_TERMINATED_ST)
    {
        pFsmData->m_bTxnTerminated = SIP_TRUE;
    }

    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* state transition*/
    pTxn->SetTxnState(nNewTxnState);

    return SIP_TRUE;
}

static SIP_BOOL CallingState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    pTxn->StopTxnTimer();

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Receive1xxResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Get user data */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    SipMessage* pMsgIn = pFsmData->m_pSipMsgIn;

    SIP_INT16 nStatusCode = SIP_ZERO;
    SipStatusLine* pStatusLine = pMsgIn->GetStatusLine();

    if (pStatusLine != SIP_NULL)
    {
        pStatusLine->GetStatusCode(&nStatusCode);
        pStatusLine->SipDelete();
    }

    if (nStatusCode != 100)
    {
        SIP_BOOL bRSeqExist = pMsgIn->HasHeader(SipHeaderBase::RSEQ);
        if (bRSeqExist == SIP_TRUE)
        {
            SipTxnKey* pTempTxnKey = new SipTxnKey(pMsgIn, pnError);
            SipTxnKey* pINVTxnKey = SipTxnUtil::SearchTxnKey(pTempTxnKey);
            if (pINVTxnKey != SIP_NULL)
            {
                pFsmData->m_eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
                if (pTempTxnKey != SIP_NULL)
                {
                    pTempTxnKey->SipDelete();
                }
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "ProceedingState_Receive1xxResponse: Retransmitted message.", SIP_ZERO,
                        SIP_ZERO);
            }
            else
            {
                pINVTxnKey = SipTxnUtil::SearchTxnKey(pTempTxnKey, SIP_FALSE);
                if (pINVTxnKey != SIP_NULL)
                {
                    SIP_UINT32 nRseq = pTempTxnKey->GetRSeq();
                    pTempTxnKey->SipDelete();
                    if (pINVTxnKey->GetRSeq() + 1 != nRseq)
                    {
                        pFsmData->m_eTxnStatus = SipTxn::STATUS_STRAY_RESP;
                        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                                "ProceedingState_Receive1xxResponse: Stray message.", SIP_ZERO,
                                SIP_ZERO);
                    }
                    else
                    {
                        pINVTxnKey->SetRSeq(nRseq);
                        pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
                    }
                }
                else
                {
                    if (SipTxnUtil::AddTxnKey(pTempTxnKey) == SIP_FALSE)
                    {
                        pTempTxnKey->SipDelete();
                        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                                "ProceedingState_Receive1xxResponse: Adding txn key failed.",
                                SIP_ZERO, SIP_ZERO);
                    }
                    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
                }
            }
        }
    }
    /* Do Noting Update the Response Handle free Previous one */
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Receive2xxResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());

    /* Fill FSM data for stack manager */
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pFsmData->m_bTxnTerminated = SIP_TRUE;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);

    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_ReceiveFailureResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    SIP_UINT16 nNewTxnState = SIP_ZERO;
    /* Prepare failure ACK message and fill into pTxnFsmData.
       state txn is done in this fxn */
    if (HandleFailureResponse(pTxn, pFsmData, &nNewTxnState, pnError) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    /* NOTE:
       pTxnFsmData is updated with failure ACK message that needs to be send to network.
     */

    if (nNewTxnState == SipTxn::INV_CLI_TERMINATED_ST)
    {
        pFsmData->m_bTxnTerminated = SIP_TRUE;
    }

    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* state transition*/
    pTxn->SetTxnState(nNewTxnState);

    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    /* Transition to Terminated State */
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);

    return SIP_TRUE;
}

static SIP_BOOL CompletedState_Timer_D_Timeout(
        SipTxn* pTxn, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    /* Transition to Terminated State */
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);

    return SIP_TRUE;
}

static SIP_BOOL CompletedState_Receive1xxResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Old response received , simply obsorved the messages*/

    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_IGNORE_RESP;

    /* old response received. remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_ReceiveFailureResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is receive of re-transmitted failure response. Failure ACK shall be sent */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_TransportError(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;

    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::INV_CLI_TERMINATED_ST);

    return SIP_TRUE;
}

// clang-format off
SIP_BOOL (*gpfSipInvClientTxnFsm[SipTxn::INV_CLI_INVALID_ST + 1][SipTxn::INV_CLI_INVALID_EVT + 1])
(SipTxn* pTxn, SIP_VOID* pvData, /* Event specific data */ SIP_UINT16* pnError) = {
    /* IDLE State:: S0*/
    {
        IdleState_SendInviteRequest, /* SipTxn::INV_CLI_SEND_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent
    },
    /* CALLING State:: S1*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_SEND_INV_REQ_EVT */
        CallingState_Timer_A_B_Timeout, /* SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT */
        CallingState_Receive1xxResponse, /* SipTxn::INV_CLI_RECV_1XX_RESP_EVT */
        CallingState_Receive2xxResponse, /* SipTxn::INV_CLI_RECV_2XX_RESP_EVT */
        CallingState_ReceiveFailureResponse, /* SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT */
        CallingState_TransportError, /* SipTxn::INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent
    },
    /* PROCEEDING State:: S2*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_SEND_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT */
        ProceedingState_Receive1xxResponse, /*SipTxn::INV_CLI_RECV_1XX_RESP_EVT */
        ProceedingState_Receive2xxResponse, /* SipTxn::INV_CLI_RECV_2XX_RESP_EVT */
        ProceedingState_ReceiveFailureResponse, /* SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT */
        ProceedingState_TransportError, /* SipTxn::INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent
    },
    /* COMPLETED State:: S3*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_SEND_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT */
        CompletedState_Timer_D_Timeout, /* SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT */
        CompletedState_Receive1xxResponse, /* SipTxn::INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_2XX_RESP_EVT */
        CompletedState_ReceiveFailureResponse, /* SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT */
        CompletedState_TransportError, /* SipTxn::INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent
    },
    /* TERMINATED State:: S4*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_SEND_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent
    },
    /* Invalid State:: S5*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_SEND_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_1XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_RECV_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_CLI_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent
    }
};
// clang-format on
