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
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"

SIP_BOOL InvSerFsm_NullFxn(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "InvSerFsm_NullFxn: Invalid Handling", SIP_ZERO, SIP_ZERO);
    return SIP_FALSE;
}

static SIP_BOOL InvSerFsm_IdleStRecvInvReqEvt(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "InvSerFsm_IdleStRecvInvReqEvt:pNewTxnKey memory fail",
                SIP_ZERO, SIP_ZERO);

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
                "InvSerFsm_IdleStRecvInvReqEvt:Adding Txn into DB Fails \n", SIP_ZERO, SIP_ZERO);
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
    pFsmData->bTxnCreated = SIP_TRUE;
    pFsmData->eTxnStatus = SipTxn::STATUS_NEW_REQ_RECVD;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_SER_PROCEEDING_ST);

    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ProceedingStRecvInvReqEvt(
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
        pFsmData->eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    }
    else
    {
        pFsmData->eTxnStatus = SipTxn::STATUS_IGNORE_REQ;
    }
    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ProceedingStSendNon100ProvRespEvt(
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
            "InvSerFsm_ProceedingStSendNon100ProvRespEvt, Status Code: %d, RSeq: %d.", nStatusCode,
            bRSeqExist);
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
                        "InvSerFsm_ProceedingStSendNon100ProvRespEvt: TxnKey insertion failed",
                        SIP_ZERO, SIP_ZERO);
            }
            else
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "InvSerFsm_ProceedingStSendNon100ProvRespEvt, Txnkey added to the utility "
                        "list.",
                        SIP_ZERO, SIP_ZERO);
            }
        }

        const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
        SIP_UINT32 nDurationT1 = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_T1);
        SIP_UINT32 nDurationTH = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_H);

        /*RFC 3262 mentions to start retransmission timer irrespective of transport being used*/
        /*Start retransmission timer*/
        if (pTxn->StartTxnTimer(SipTxn::TIMER_G, nDurationT1, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "InvSerFsm_ProceedingStSendNon100ProvRespEvt: Start Timer Failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
        pTxn->SetMaxDuration(nDurationTH);
    }

    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* No Change in State, Be in proceeding state*/

    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ProceedingStSend3xx6xxFailureRespEvt(
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
                    "InvSerFsm_ProceedingStSend3xx6xxFailureRespEvt: Start Timer Failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else /*Reliable transport*/
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_H, nDurationTH, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "InvSerFsm_ProceedingStSend3xx6xxFailureRespEvt:Start Timer Failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }

        pTxn->IncreaseDurationExpired(nDurationTH);
    }

    pTxn->SetMaxDuration(nDurationTH);

    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* State Transition */
    pTxn->SetTxnState(SipTxn::INV_SER_COMPLETED_ST);

    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ProceedingStSend2xxSuccessRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    /* Stop existing Txn timer*/
    /* Delete the txn key in the util list*/
    pTxn->StopTxnTimer();
    SipTxnUtil::DeleteTxnKey(pTxn->GetTxnKey());

    // Initialize the retransmission related information
    pTxn->InitRetransmissionInfo();

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationTH = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_H);

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    if (pTxn->StartTxnTimer(SipTxn::TIMER_H, nDurationTH, pnError) != SIP_FALSE)
    {
        pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
        pTxn->SetMaxDuration(nDurationTH);
        pTxn->IncreaseDurationExpired(nDurationTH);
        pTxn->SetTxnState(SipTxn::INV_SER_COMPLETED_ST);
    }
    else
    {
        pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
        pFsmData->bTxnTerminated = SIP_TRUE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();
        pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    }

    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ProceedingStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pvData;
    (void)pnError;
    pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ProceedingStTimerG_H_TimeoutEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTimeoutData* pTimeoutData = static_cast<SipTimeoutData*>(pvData);

    if (pTimeoutData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "InvSerFsm_ProceedingStTranspErrorEvt : pTimeoutData is NULL", SIP_ZERO, SIP_ZERO);
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
            pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
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
                            "InvSerFsm_ProceedingStTimerG_H_TimeoutEvt: Start Timer Failed",
                            SIP_ZERO, SIP_ZERO);
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

static SIP_BOOL InvSerFsm_CompletedStRecvInvReqEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* This is receive of re-transmitted INVITE request. stack manager to send last response */
    pFsmData->eTxnStatus = SipTxn::STATUS_RETRANSMISSION;
    pFsmData->m_pOutUserData = pTxn->GetUserData();
    pFsmData->m_pTranspInfo = pTxn->GetTranspInfo();

    /* Remain in same state */

    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_CompletedStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pvData;
    (void)pnError;

    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_CompletedStRecvAckReqEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    pTxn->StopTxnTimer();

    SIP_INT32 eTranspMsgSentProtocol = pTxn->GetMsgSentProto();

    SIP_UINT16 nNextState;

    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    /* For Unreliable Transport */
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        /* Start Timer I */
        const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
        SIP_UINT32 nDurationTI = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_I);

        nNextState = SipTxn::INV_SER_CONFIRMED_ST;

        pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();

        if (pTxn->StartTxnTimer(SipTxn::TIMER_I, nDurationTI, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "InvSerFsm_CompletedStRecvAckReqEvt:Start Timer Failed \n", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else /* For Reliable Transport */
    {
        // TimerI = 0, for TCP.
        pFsmData->bTxnTerminated = SIP_TRUE;
        pFsmData->m_pOutUserData = pTxn->GetUserData();
        nNextState = SipTxn::INV_SER_TERMINATED_ST;
    }

    pTxn->SetTxnState(nNextState);
    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_CompletedStTimerG_H_TimeoutEvt(
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
        SIP_BOOL bStatus = pTxn->StartTxnTimer(SipTxn::TIMER_G, nDuration, pnError);
        if (bStatus == SIP_FALSE)
        {
            return SIP_FALSE;
        }
        pTxn->IncreaseTxnCount();
        pTxn->IncreaseDurationExpired(nDuration);
        pTxn->SetCurrentDuration(nDuration);
    }

    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ConfirmedStRecvAckReqEvt(
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
    pFsmData->eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* Remain in same state */
    return SIP_TRUE;
}

static SIP_BOOL InvSerFsm_ConfirmedStTimerI_TimeoutEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
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
        InvSerFsm_IdleStRecvInvReqEvt, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_2XX_SUCCESS_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TRANSP_ERROR_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT*/
        InvSerFsm_NullFxn
    },
    /* PROCEEDING State:: S1*/
    {
        InvSerFsm_ProceedingStRecvInvReqEvt, /* InvSerFsm_ProceedingStRecvInvReqEvt,
                                                SipTxn::INV_SER_RECV_INV_REQ_EVT */
        InvSerFsm_ProceedingStSendNon100ProvRespEvt, /* SendNon100ProvRespEvt*/
        InvSerFsm_ProceedingStSend3xx6xxFailureRespEvt, /* Send3xx6xxFailureRespEvt*/
        InvSerFsm_ProceedingStSend2xxSuccessRespEvt, /*Send2xxSuccessRespEvt*/
        InvSerFsm_ProceedingStTranspErrorEvt, /* SipTxn::INV_SER_TRANSP_ERROR_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT*/
        InvSerFsm_ProceedingStTimerG_H_TimeoutEvt, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT*/
        InvSerFsm_NullFxn
    },
    /* COMPLETE State:: S2*/
    {
        InvSerFsm_CompletedStRecvInvReqEvt, /*InvSerFsm_CompletedStRecvInvReqEvt,
                                              SipTxn::INV_SER_RECV_INV_REQ_EVT */
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_2XX_SUCCESS_RESP_EVT*/
        InvSerFsm_CompletedStTranspErrorEvt, /* SipTxn::INV_SER_TRANSP_ERROR_EVT*/
        InvSerFsm_CompletedStRecvAckReqEvt, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT*/
        InvSerFsm_CompletedStTimerG_H_TimeoutEvt, /* TimerG_H_TimeoutEvt*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT*/
        InvSerFsm_NullFxn
    },
    /* CONFIRMED State:: S3*/
    {
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_2XX_SUCCESS_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TRANSP_ERROR_EVT*/
        InvSerFsm_ConfirmedStRecvAckReqEvt, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT*/
        InvSerFsm_NullFxn, /* TimerG_H_TimeoutEvt*/
        InvSerFsm_ConfirmedStTimerI_TimeoutEvt, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT*/
        InvSerFsm_NullFxn
    },
    /* TERMINATED State:: S4*/
    {
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_2XX_SUCCESS_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TRANSP_ERROR_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT*/
        InvSerFsm_NullFxn
    },
    /* Invalid State:: S4*/
    {
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_INV_REQ_EVT */
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_NON_100_PROV_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_SEND_2XX_SUCCESS_RESP_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TRANSP_ERROR_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_RECV_ACK_REQ_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT*/
        InvSerFsm_NullFxn, /* SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT*/
        InvSerFsm_NullFxn
    }
};
// clang-format on
