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
#include "txn/SipTxnUtil.h"

static SIP_BOOL NonInvSerFsm_NullFxn(SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    SIP_DEBUG_STACKBUG(
            ESIPTRACE_MODTXN, "NonInvSerFsm_NullFxn: Invalid Handling", SIP_ZERO, SIP_ZERO);
    return SIP_FALSE;
}

static SIP_BOOL NonInvSer_Send2xx6xxResp(
        SipTxn* pTxn, void* pvData, SIP_UINT16* pnNewTxnState, SIP_UINT16* pnError)
{
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);
    SipTransportParameter* pTransparam = pFsmData->m_pTranspParam;

    const SipTxnTimerValues& objSipTxnTimers = pTxn->GetSipTxnTimers();
    SIP_UINT32 nDurationTJ = objSipTxnTimers.GetTimerValue(SipTxn::TIMER_J);

    SIP_INT32 eTranspMsgSentProtocol = pTransparam->GetTranspProtocol();

    /* For Unreliable Transport : Start Timer J*/
    if (eTranspMsgSentProtocol == SipTransportInfo::PROTOCOL_UDP)
    {
        if (pTxn->StartTxnTimer(SipTxn::TIMER_J, nDurationTJ, pnError) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "NonInvSer_Send2xx6xxResp: StartTxnTimer:Failed \n",
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

static SIP_BOOL NonInvSerFsm_IdleStRecvNonInvReqEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SipTxnKey* pNewTxnKey = new SipTxnKey(pTxn->GetTxnKey(), pnError);

    if ((pNewTxnKey == SIP_NULL) || (*pnError == E_ERR_PF_MALLOCFAILED))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvSerFsm_IdleStRecvNonInvReqEvt:pNewTxnKey memory fail", SIP_ZERO, SIP_ZERO);

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
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvSerFsm_IdleStRecvNonInvReqEvt:Adding Txn into DB Fails \n", SIP_ZERO,
                SIP_ZERO);
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
    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "NonInvSerFsm_IdleStRecvNonInvReqEvt: Method : %d",
            pMsgIn->GetMethodType(), SIP_ZERO);

    /*If PRACK message received, stop 18x retransmission timer if any*/
    if (pMsgIn->GetMethodType() == SipMessage::METHOD_PRACK)
    {
        SipTxn* pInvSerTxn = SIP_NULL;  // Corresponding INV Txn.

        SIP_UINT32 nRseqNum = SipMessage::GetRSeqNum(pMsgIn, SipHeaderBase::RACK);
        pNewTxnKey->SetRSeq(nRseqNum);
        SipTxnKey* pINVTxnKey = SipTxnUtil::SearchTxnKey(pNewTxnKey);

        if (pINVTxnKey != SIP_NULL)
        {
            if (Sip_Cbk_FetchTransaction(reinterpret_cast<SIP_VOID*>(pINVTxnKey), SipTxn::OPT_FETCH,
                        SIP_NULL, reinterpret_cast<SIP_VOID**>(&pInvSerTxn)) == SIP_TRUE)
            {
                if (pInvSerTxn != SIP_NULL)
                {
                    pInvSerTxn->StopTxnTimer();
                }
                if (SipTxnUtil::DeleteTxnKey(pNewTxnKey, SIP_TRUE) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                            "NonInvSerFsm_IdleStRecvNonInvReqEvt:Deleting Txn Key from list Failed "
                            "\n",
                            SIP_ZERO, SIP_ZERO);
                }
            }
            else
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                        "NonInvSerFsm_IdleStRecvNonInvReqEvt, FETCH failure, can not stop Timer.",
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

static SIP_BOOL NonInvSerFsm_TryingStRecvNonInvReqEvt(
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

static SIP_BOOL NonInvSerFsm_TryingStSend1xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    /* Fill FSM data for stack manager */
    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    pTxn->SetTxnState(SipTxn::NON_INV_SER_PROCEEDING_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvSerFsm_TryingStSend2xx6xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState = SIP_ZERO;

    /* handling of response and state transition occur inside the fxn */
    if (NonInvSer_Send2xx6xxResp(pTxn, pvData, &nNewTxnState, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvSerFsm_TryingStSend2xx6xxRespEvt: response processing fail", SIP_ZERO,
                SIP_ZERO);
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

static SIP_BOOL NonInvSerFsm_TryingStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvSerFsm_ProceedingStRecvNonInvReqEvt(
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

static SIP_BOOL NonInvSerFsm_ProceedingStSend1xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pTxn;
    (void)pnError;
    SipTxnFsmData* pFsmData = static_cast<SipTxnFsmData*>(pvData);

    pFsmData->m_eTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* No Change in State */
    return SIP_TRUE;
}

static SIP_BOOL NonInvSerFsm_ProceedingStSend2xx6xxRespEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_UINT16 nNewTxnState;

    /* handling of response and state transition occur inside the fxn */
    if (NonInvSer_Send2xx6xxResp(pTxn, pvData, &nNewTxnState, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "NonInvSerFsm_ProceedingStSend2xx6xxRespEvt: response processing fail", SIP_ZERO,
                SIP_ZERO);
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

static SIP_BOOL NonInvSerFsm_ProceedingStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData;
    (void)pTxn;
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvSerFsm_CompletedStRecvNonInvReqEvt(
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

static SIP_BOOL NonInvSerFsm_CompletedStTranspErrorEvt(
        SipTxn* pTxn, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    (void)pnError;
    (void)pvData, (void)pTxn;
    pTxn->StopTxnTimer();
    pTxn->SetTxnState(SipTxn::NON_INV_SER_TERMINATED_ST);
    return SIP_TRUE;
}

static SIP_BOOL NonInvSerFsm_CompletedStTimer_J_TimeoutEvt(
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
        NonInvSerFsm_IdleStRecvNonInvReqEvt, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        NonInvSerFsm_NullFxn
    },
    /* TRYING State:: S1*/
    {
        NonInvSerFsm_TryingStRecvNonInvReqEvt, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        NonInvSerFsm_TryingStSend1xxRespEvt, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        NonInvSerFsm_TryingStSend2xx6xxRespEvt, /* Send2xx6xxRespEvt */
        NonInvSerFsm_TryingStTranspErrorEvt, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        NonInvSerFsm_NullFxn
    },
    /* PROCEEDING State:: S2*/
    {
        NonInvSerFsm_ProceedingStRecvNonInvReqEvt, /* RecvNonInvReqEvt */
        NonInvSerFsm_ProceedingStSend1xxRespEvt, /* Send1xxRespEvt */
        NonInvSerFsm_ProceedingStSend2xx6xxRespEvt, /* Send2xx6xxRespEvt */
        NonInvSerFsm_ProceedingStTranspErrorEvt, /* ranspnErrorEvt */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        NonInvSerFsm_NullFxn
    },
    /* COMPLETED State:: S3*/
    {
        NonInvSerFsm_CompletedStRecvNonInvReqEvt, /* RecvNonInvReqEvt */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT */
        NonInvSerFsm_CompletedStTranspErrorEvt, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        NonInvSerFsm_CompletedStTimer_J_TimeoutEvt, /* Timer_J_TimeoutEvt */
        NonInvSerFsm_NullFxn
    },
    /* TERMINATED State:: S4*/
    {
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        NonInvSerFsm_NullFxn
    },
    /* Invalid State:: S5*/
    {
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_RECV_NON_INV_REQ_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_1XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_SEND_2XX_6XX_RESP_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TRANSP_ERROR_EVT */
        NonInvSerFsm_NullFxn, /* SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT */
        NonInvSerFsm_NullFxn
    }
};
// clang-format on
