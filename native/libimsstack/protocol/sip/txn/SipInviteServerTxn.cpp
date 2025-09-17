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
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"

SIP_BOOL HandleInvalidStateEvent(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "Unhandled state and event.", SIP_ZERO, SIP_ZERO);
    return SIP_FALSE;
}

static SIP_BOOL IdleState_ReceiveInviteRequest(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "IdleState_ReceiveInviteRequest: SipTxnKey memory allocation failed.", SIP_ZERO,
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
                "IdleState_ReceiveInviteRequest: Adding txn to DB failed.", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* TxnObj is added to hash, hence increment ref count */
    pTxn->Increment();

    /* Set Userdata into Txn object */
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
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
    pTxn->SetTxnState(SipTxn::INV_SER_PROCEEDING_ST);

    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_ReceiveInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* This is receive of re-transmitted INVITE request. stack manager to send last response */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Check if TU has already send any 1xx response for INVITE else ignore the INVITE */
    if (pFsmData->m_pTranspInfo != SIP_NULL)
    {
        pFsmData->m_eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    }
    else
    {
        pFsmData->m_eTxnStatus = SipTxn::STATUS_IGNORE_REQ;
    }
    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_SendNon100ProvisionalResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* Stop existing Txn timer*/
    /* Delete the txn key in the util list*/
    pTxn->StopTxnTimer();
    SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());

    // Initialize the retransmission related information
    pTxn->InitRetransmissionInfo();

    /*Check whether RSeq header present or not*/
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    SipMessage* pMsgIn = pFsmData->m_pSipMsgIn;
    SIP_BOOL bRSeqExist = pMsgIn->HasHeader(SipHeaderBase::RSEQ);

    /*Get the status code*/
    SIP_INT16 nStatusCode = SIP_ZERO;
    SipStatusLine* pStatusLine = pMsgIn->GetStatusLine();

    if (pStatusLine != SIP_NULL)
    {
        pStatusLine->GetStatusCode(&nStatusCode);
        pStatusLine->SipDelete();
    }

    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
            "ProceedingState_SendNon100ProvisionalResponse, Status code: %d, hasRSeq: %d.",
            nStatusCode, bRSeqExist);
    // 18x / 199 response
    if (((nStatusCode > 100) && (nStatusCode < 200)) && (bRSeqExist == SIP_TRUE))
    {
        /*Create a duplicate txn key and store in a transaction list to match with PRACK*/
        SipTxnKey* pTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);
        if (pTxnKey != SIP_NULL)
        {
            SIP_UINT32 nRseqNum = SipMessage::GetRSeqNum(pMsgIn, SipHeaderBase::RSEQ);
            pTxnKey->SetRSeq(nRseqNum);
            if (SipTxnUtil::AddTxnKey(pTxnKey) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "ProceedingState_SendNon100ProvisionalResponse: Adding txn key to DB "
                        "failed",
                        SIP_ZERO, SIP_ZERO);
            }
            else
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "ProceedingState_SendNon100ProvisionalResponse: Txnkey added.", SIP_ZERO,
                        SIP_ZERO);
            }
        }

        const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
        SIP_UINT32 nDurationT1 = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);

        /*RFC 3262 mentions to start retransmission timer irrespective of transport being used*/
        /*Start retransmission timer*/
        if (pTxn->StartTxnTimer(SipTxn::TIMER_G, nDurationT1, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "ProceedingState_SendNon100ProvisionalResponse: Starting Timer_G failed.",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->SetMaxDuration(nDurationT1 * 64);
    }

    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* No Change in State, Be in proceeding state*/

    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_SendFailureResponse(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* Stop existing Txn timer*/
    /* Delete the txn key in the util list*/
    pTxn->StopTxnTimer();
    SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());

    // Initialize the retransmission related information
    pTxn->InitRetransmissionInfo();

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationT1 = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);
    SIP_UINT32 nDurationTH = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_H);

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    SipTransportParameter* pTranspParam = pFsmData->m_pTranspParam;
    SIP_INT32 eTranspProtocol = pTranspParam->GetTranspProtocol();

    /* For Unreliable Transport : Start Timer G*/
    if (eTranspProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_G, nDurationT1, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "ProceedingState_SendFailureResponse: Starting Timer_G failed.", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else /*Reliable transport*/
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_H, nDurationTH, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "ProceedingState_SendFailureResponse: Starting Timer_H failed.", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }

        pTxn->IncreaseDurationExpired(nDurationTH);
    }

    pTxn->SetMaxDuration(nDurationTH);

    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_SER_COMPLETED_ST);

    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Send2xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* Stop existing Txn timer*/
    /* Delete the txn key in the util list*/
    pTxn->StopTxnTimer();
    SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());

    // Initialize the retransmission related information
    pTxn->InitRetransmissionInfo();

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationTL = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_L);

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    if (pTxn->StartTxnTimer(SipTxn::TIMER_L, nDurationTL, pnError) != SIP_FALSE)
    {
        pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
        pTxn->SetMaxDuration(nDurationTL);
        pTxn->IncreaseDurationExpired(nDurationTL);
        pTxn->SetTxnState(SipTxn::INV_SER_ACCEPTED_ST);
    }
    else
    {
        pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
        pFsmData->m_bTxnTerminated = SIP_TRUE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();
        pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    }

    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_TransportError(
        SipTxn* /*pTxn*/, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL ProceedingState_Timer_G_H_Timeout(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTimeoutData* pTimeoutData = static_cast<SipTimeoutData*>(pvData);

    if (pTimeoutData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "ProceedingState_Timer_G_H_Timeout: pTimeoutData is NULL", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nDurationExpired = pTxn->GetDurationExpired();
    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nT1Val = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);

    if (nDurationExpired == 0)
    {
        // Timer T1 already fired.
        pTxn->IncreaseDurationExpired(nT1Val);
        nDurationExpired = nT1Val;
    }

    SIP_UINT32 nMaxDuration = pTxn->GetMaxDuration();

    if (pTimeoutData->GetTimerType() == SipTxn::TIMER_G)
    {
        /* Check Max Retransmission Limit or Total expired
           time has crossed the Max duration limit or not */
        if (nDurationExpired >= nMaxDuration)
        {
            pTxn->SetRprTxnTerminated(SIP_TRUE);
            return SIP_TRUE;
        }
        else
        {
            SIP_UINT32 nCurrentDuration = pTxn->GetCurrentDuration();

            /* Double of timeout value for RPR */
            SIP_UINT32 nDuration = nCurrentDuration << 1;

            // Update the timer duration.
            if ((nDurationExpired + nDuration) >= nMaxDuration)
            {
                nDuration = nMaxDuration - nDurationExpired;
            }

            if (nDuration > SIP_ZERO)
            {
                if (pTxn->StartTxnTimer(SipTxn::TIMER_G, nDuration, pnError) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                            "ProceedingState_Timer_G_H_Timeout: Starting Timer_G failed.", SIP_ZERO,
                            SIP_ZERO);
                    return SIP_FALSE;
                }
                pTxn->IncreaseTxnCount();
                pTxn->IncreaseDurationExpired(nDuration);
                pTxn->SetCurrentDuration(nDuration);
            }
        }
    }

    return SIP_TRUE;
}

static SIP_BOOL AcceptedState_ReceiveInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is the reception of re-transmitted INVITE request.
       Stack manager to send last response. */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_IGNORE_REQ;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Remain in same state */

    return SIP_TRUE;
}

static SIP_BOOL AcceptedState_Send2xxResponse(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pFsmData->m_pOutUserData = pTxn->GetUserData();

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL AcceptedState_TransportError(
        SipTxn* /*pTxn*/, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    /* Remain in same state*/
    return SIP_TRUE;
}

static SIP_BOOL AcceptedState_ReceiveAckRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* /*pnError*/)
{
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    pFsmData->m_pOutUserData = pTxn->GetUserData();

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL AcceptedState_Timer_L_Timeout(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;

    pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_ReceiveInviteRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is receive of re-transmitted INVITE request. stack manager to send last response */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Remain in same state */

    return SIP_TRUE;
}

static SIP_BOOL CompletedState_TransportError(
        SipTxn* /*pTxn*/, SIP_VOID* /*pvData*/, SIP_UINT16* /*pnError*/)
{
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_ReceiveAckRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    pTxn->StopTxnTimer();

    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();

    SIP_UINT16 nNextState;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
    /* For Unreliable Transport */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        /* Start Timer I */
        const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
        SIP_UINT32 nDurationTI = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_I);

        nNextState = SipTxn::INV_SER_CONFIRMED_ST;

        pFsmData->m_pOutUserData = pTxn->GetUserData();

        if (pTxn->StartTxnTimer(SipTxn::TIMER_I, nDurationTI, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "CompletedState_ReceiveAckRequest: Starting Timer_I failed.", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else /* For Reliable Transport */
    {
        // TimerI = 0, for TCP.
        pFsmData->m_bTxnTerminated = SIP_TRUE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();
        nNextState = SipTxn::INV_SER_TERMINATED_ST;
    }

    pTxn->SetTxnState(nNextState);
    return SIP_TRUE;
}

static SIP_BOOL CompletedState_Timer_G_H_Timeout(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pvData;

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
            pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
            nDuration = SIP_ZERO;  // Transaction to be timedout immediately.
        }
        else
        {
            SIP_UINT32 nCurrentDuration = pTxn->GetCurrentDuration();
            SIP_UINT32 nNextDuration = nCurrentDuration << 1;
            SIP_UINT32 nDurationT2 = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T2);

            /* MIN(2*T1, T2) seconds*/
            nDuration = SIP_MIN(nNextDuration, nDurationT2);

            // Update the timer duration.
            if ((nDurationExpired + nDuration) >= nMaxDuration)
            {
                nDuration = nMaxDuration - nDurationExpired;
            }
        }
    }
    else /* For Reliable Transport */
    {
        pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
        nDuration = SIP_ZERO;  // Transaction to be timedout immediately.
    }

    if (nDuration > SIP_ZERO)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_G, nDuration, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "CompletedState_Timer_G_H_Timeout: Starting Timer_G failed.", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->IncreaseTxnCount();
        pTxn->IncreaseDurationExpired(nDuration);
        pTxn->SetCurrentDuration(nDuration);
    }

    return SIP_TRUE;
}

static SIP_BOOL ConfirmedState_ReceiveAckRequest(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Absorb ACK retranmsissions.*/
    /* RFC 3261: 17.2.1 INVITE Server Transaction
       The purpose of the "Confirmed" state is to absorb any additional ACK
       messages that arrive, triggered from retransmissions of the final
       response */

    /* Re-transmitted failure ACK is received. Simply absorved the message */

    /* Fill FSM data for stack manager */
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL ConfirmedState_Timer_I_Timeout(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;

    pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

// clang-format off
SIP_BOOL(*gpfSipInvSerTxnFsm[SipTxn::INV_SER_INVALID_ST + 1][SipTxn::INV_SER_INVALID_EVT + 1])
(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError) = {
    /* IDLE State:: S0*/
    {
        IdleState_ReceiveInviteRequest, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* PROCEEDING State:: S1*/
    {
        ProceedingState_ReceiveInviteRequest, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        ProceedingState_SendNon100ProvisionalResponse, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        ProceedingState_SendFailureResponse, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT */
        ProceedingState_Send2xxResponse, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        ProceedingState_TransportError, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        ProceedingState_Timer_G_H_Timeout, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* ACCEPTED State:: S2*/
    {
        AcceptedState_ReceiveInviteRequest, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT*/
        AcceptedState_Send2xxResponse, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        AcceptedState_TransportError, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        AcceptedState_ReceiveAckRequest, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        AcceptedState_Timer_L_Timeout, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* COMPLETE State:: S3*/
    {
        CompletedState_ReceiveInviteRequest, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        CompletedState_TransportError, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        CompletedState_ReceiveAckRequest, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        CompletedState_Timer_G_H_Timeout, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* CONFIRMED State:: S4*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        ConfirmedState_ReceiveAckRequest, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        ConfirmedState_Timer_I_Timeout, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* TERMINATED State:: S5*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    },
    /* Invalid State:: S6*/
    {
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_FAILURE_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_SEND_2XX_RESP_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TRANSP_ERROR_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT */
        HandleInvalidStateEvent, /* SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT */
        HandleInvalidStateEvent
    }
};
// clang-format on
