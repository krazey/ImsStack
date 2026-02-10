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
#include "ISipNetworkUtil.h"
#include "SipConfiguration.h"
#include "SipDebug.h"
#include "SipStackError.h"
#include "SipUtil.h"
#include "platform/SipString.h"
#include "txn/SipTimeoutData.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsm.h"
#include "txn/SipTxnUtil.h"

SipTxn::SipTxn() :
        m_eTxnType(SipTxn::INVALID),
        m_pTxnKey(SIP_NULL),
        m_pSipMsg(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        m_pUserData(SIP_NULL),
        m_nTxnState(SIP_ZERO),
        m_nRetransmissionCount(SIP_ZERO),
        m_eTimerType(SipTxn::TIMER_TYPE_INVALID),
        m_pvTimerId(SIP_NULL),
        m_nMaxDuration(SIP_ZERO),
        m_nDurationExpired(SIP_ZERO),
        m_nCurrentDuration(SIP_ZERO),
        m_bRprTxnTerminated(SIP_FALSE),
        m_bAckReceived(SIP_FALSE)
{
}

SipTxn::SipTxn(IN SIP_INT32 eTxnType, IN const SipTxnKey* pTxnKey, IN const SipMessage* pSipMsg,
        IN const SipTimerContext* pSipTxnTimerContext, OUT SIP_UINT16* pnError) :
        m_eTxnType(SipTxn::INVALID),
        m_pTxnKey(SIP_NULL),
        m_pSipMsg(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        m_pUserData(SIP_NULL),
        m_nTxnState(SIP_ZERO),
        m_nRetransmissionCount(SIP_ZERO),
        m_eTimerType(SipTxn::TIMER_TYPE_INVALID),
        m_pvTimerId(SIP_NULL),
        m_nMaxDuration(SIP_ZERO),
        m_nDurationExpired(SIP_ZERO),
        m_nCurrentDuration(SIP_ZERO),
        m_bRprTxnTerminated(SIP_FALSE),
        m_bAckReceived(SIP_FALSE)
{
    m_eTxnType = eTxnType;
    m_pTxnKey = new SipTxnKey(pTxnKey, pnError);

    if ((pSipTxnTimerContext != SIP_NULL) && (pSipTxnTimerContext->m_pTxnSipTxnTimers != SIP_NULL))
    {
        m_objTxnTimerValues.UpdateSipTimers(
                pSipTxnTimerContext->m_nTimerOptions, pSipTxnTimerContext->m_pTxnSipTxnTimers);
    }

    if (m_pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxn SipTxnKey NULL \n", SIP_ZERO, SIP_ZERO);
        return;
    }

    if (*pnError == E_ERR_PF_MALLOCFAILED)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxn Malloc Failed \n", SIP_ZERO, SIP_ZERO);
        m_pTxnKey->SipDelete();
        m_pTxnKey = SIP_NULL;
    }

    m_pSipMsg = new SipMessage(*pSipMsg);

    /* IDLE State */
    m_nTxnState = SIP_ZERO;
    m_nRetransmissionCount = SIP_ZERO;
}

SipTxn::~SipTxn()
{
    if (m_pTranspInfo != SIP_NULL)
    {
        delete m_pTranspInfo;
        m_pTranspInfo = SIP_NULL;
    }

    if (m_pUserData != SIP_NULL)
    {
        delete m_pUserData;
        m_pUserData = SIP_NULL;
    }

    if (m_pTxnKey != SIP_NULL)
    {
        m_pTxnKey->SipDelete();
        m_pTxnKey = SIP_NULL;
    }

    if (m_pSipMsg != SIP_NULL)
    {
        m_pSipMsg->SipDelete();
        m_pSipMsg = SIP_NULL;
    }
}

SIP_BOOL SipTxn::InvokeFsm(SIP_UINT16 nEvent, SIP_VOID* pvData, SIP_UINT16* pnError)
{
    SIP_INT32 eTxnType = m_eTxnType;
    SIP_UINT32 nTxnState = m_nTxnState;

    switch (eTxnType)
    {
        case SipTxn::INVITE_CLIENT:
        {
            if (gpfSipInvClientTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "gpfSipInvClientTxnFsm Fails", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::INVITE_SERVER:
        {
            if (gpfSipInvSerTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "gpfSipInvSerTxnFsm Fails", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::NON_INVITE_CLIENT:
        {
            if (gpfSipNonInvClientTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "gpfSipNonInvClientTxnFsm Fail", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::NON_INVITE_SERVER:
        {
            if (gpfSipNonInvSerTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "gpfSipNonInvSerTxnFsm Fail", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        default:
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "SipTxn::Invalid Txn Type eTxnType[%d] nTxnState[%d]", eTxnType, nTxnState);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

/*
   This fxn stop the running txn timer and inform to user to abort the existing NW transaction
*/
SIP_BOOL SipTxn::AbortTxn()
{
    ISipTransactionCallback* pCallback = SipUtil::GetInstance()->GetTransactionCallback();
    if (m_pvTimerId != SIP_NULL && pCallback != SIP_NULL)
    {
        SipTimeoutData* pTimeoutData = SIP_NULL;
        pCallback->StopTimer(m_pvTimerId, pTimeoutData);
        m_pvTimerId = SIP_NULL;

        if (pTimeoutData != SIP_NULL)
        {
            delete pTimeoutData;
        }
    }

    SipTxnUtil::DeleteTxnKey(m_pTxnKey, SIP_TRUE);

    return SIP_TRUE;
}

SIP_BOOL SipTxn::StartTxnTimer(SIP_UINT32 eTimerType, SIP_UINT32 nDuration, SIP_UINT16* pnError)
{
    SipTimeoutData* pTimeoutData = new SipTimeoutData(m_eTxnType, (SIP_INT32)eTimerType, m_pTxnKey);

    if (pTimeoutData == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer: Memory Failed \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (nDuration > SIP_ZERO)
    {
        ISipTransactionCallback* pCallback = SipUtil::GetInstance()->GetTransactionCallback();
        if (pCallback == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer:pTimer is NULL ", SIP_ZERO, SIP_ZERO);
            delete pTimeoutData;
            return SIP_FALSE;
        }

        m_pvTimerId = pCallback->StartTimer(nDuration, CbkTxnTimeout, pTimeoutData);
        if (m_pvTimerId == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "StartTxnTimer: StartTimer Failed", SIP_ZERO, SIP_ZERO);

            delete pTimeoutData;
            return SIP_FALSE;
        }

        m_eTimerType = (SIP_INT32)eTimerType;
        m_nCurrentDuration = nDuration;
    }
    else
    {
        m_eTimerType = (SIP_INT32)eTimerType;
        m_nCurrentDuration = nDuration;
        /*make the timer id null & pass null as timer id in callback
          function to avoid Timer ID Mismatch in callback function*/
        SetTimerId(SIP_NULL);
        CbkTxnTimeout(pTimeoutData, SIP_NULL);
    }
    return SIP_TRUE;
}

SIP_BOOL SipTxn::StopTxnTimer()
{
    if (m_pvTimerId == SIP_NULL)
    {
        return SIP_TRUE;
    }

    ISipTransactionCallback* pCallback = SipUtil::GetInstance()->GetTransactionCallback();
    if (pCallback == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxn::StopTxnTimer:pTimer is NULL ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTimeoutData* pTimeoutData = SIP_NULL;
    pCallback->StopTimer(m_pvTimerId, pTimeoutData);
    m_pvTimerId = SIP_NULL;
    delete pTimeoutData;

    return SIP_TRUE;
}

SIP_BOOL SipTxn::PrepareACK(SipMessage* pSipRespMsg, /* IN */
        SIP_BOOL bSetMsgBody, /* IN */ SipMessage** ppSipAckMsg /* OUT */)
{
    /* Ref:RFC 3261 Sec.17.1.1.3 Construction of the ACK Request*/
    SipMessage* pSipAckMsg = new SipMessage(SipMessage::REQ_TYPE);
    SIP_BOOL bStatus = SIP_TRUE;

    (void)bSetMsgBody;

    /* Set 'Call-ID' from INVITE Request     */
    SipHeaderBase* pCallIdHdr = m_pSipMsg->GetHdrObj(SipHeaderBase::CALL_ID);
    if (pCallIdHdr != SIP_NULL)
    {
        SipHeaderBase* pCallId = new SipHeaderBase(*pCallIdHdr);
        pCallIdHdr->SipDelete();

        bStatus = pSipAckMsg->SetHeader(pCallId);
        pCallId->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "PrepareACK: Set CallID Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }

    /* Set 'From'     from INVITE Request */
    SipNameAddrHeader* pFromHdr =
            static_cast<SipNameAddrHeader*>(pSipRespMsg->GetHdrObj(SipHeaderBase::FROM));
    if (SIP_NULL != pFromHdr)
    {
        SipNameAddrHeader* pFrom = new SipNameAddrHeader(*pFromHdr);
        pFromHdr->SipDelete();

        bStatus = pSipAckMsg->SetHeader(pFrom);
        pFrom->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "PrepareACK: Set From Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }
    /* Set Request-Uri from INVITE Request */
    SipRequestLine* pReqLine = SIP_NULL;
    pReqLine = m_pSipMsg->GetReqLine();

    if (pReqLine != SIP_NULL)
    {
        SipAddrSpec* pReqUri = pReqLine->GetReqUri();
        pReqLine->SipDelete();

        if (pReqUri != SIP_NULL)
        {
            SipAddrSpec* pNewObjReqUri = new SipAddrSpec(*pReqUri);
            pReqUri->SipDelete();
            pReqLine = new SipRequestLine(
                    SipMsgUtil::METHOD_ACK, pNewObjReqUri, SipMsgUtil::SIP_VERSION);
            if (pReqLine == SIP_NULL)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "PrepareACK: SipRequestLine NULL \n", SIP_ZERO, SIP_ZERO);
                pSipAckMsg->SipDelete();
                return SIP_FALSE;
            }
            pSipAckMsg->SetRequestline(pReqLine);
        }
    }

    /* Set 'To' from INVITE Response */
    SipNameAddrHeader* pToHdr =
            static_cast<SipNameAddrHeader*>(pSipRespMsg->GetHdrObj(SipHeaderBase::TO));
    if (pToHdr != SIP_NULL)
    {
        SipNameAddrHeader* pTo = new SipNameAddrHeader(*pToHdr);
        pToHdr->SipDelete();

        bStatus = pSipAckMsg->SetHeader(pTo);
        pTo->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "PrepareACK: Set To Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }

    /* Set 'Via' from INVITE Request    */
    SipViaHeader* pViaHdr = static_cast<SipViaHeader*>(m_pSipMsg->GetHdrObj(SipHeaderBase::VIA));
    if (pViaHdr != SIP_NULL)
    {
        SipViaHeader* pVia = new SipViaHeader(*pViaHdr);
        pViaHdr->SipDelete();

        bStatus = pSipAckMsg->SetHeader(pVia);
        pVia->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "PrepareACK: Set Via Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }

    /* Set CSeq    -Num    from INVITE Request */
    SipCSeqHeader* pCSeqHdr =
            static_cast<SipCSeqHeader*>(m_pSipMsg->GetHdrObj(SipHeaderBase::CSEQ));
    if (pCSeqHdr != SIP_NULL)
    {
        SipCSeqHeader* pCseq = new SipCSeqHeader(*pCSeqHdr);
        pCSeqHdr->SipDelete();

        /* Set CSeq-Method as ACk */
        pCseq->SetMethod("ACK");

        bStatus = pSipAckMsg->SetHeader(pCseq);
        pCseq->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "PrepareACK: Set CSeq Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }

    /* Set MaxForward Header */
    SipIntegerHeader* pMaxForward = new SipIntegerHeader(SipHeaderBase::MAX_FORWARDS);
    SIP_CHAR szMaxFwdValue[SipMsgUtil::MAX_INT_VALUE_LEN] = {
            0,
    };
    SipPf_Sprintf(szMaxFwdValue, "%d", SIP_MAX_HOP);
    pMaxForward->SetValue(szMaxFwdValue);
    bStatus = pSipAckMsg->SetHeader(pMaxForward);
    pMaxForward->SipDelete();
    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "PrepareACK: Set MaxForward Failed \n", SIP_ZERO, SIP_ZERO);
        pSipAckMsg->SipDelete();
        return SIP_FALSE;
    }

    /* Set UserAgent from INVITE Request */
    SipUserAgentHeader* pUserAgentHdr =
            static_cast<SipUserAgentHeader*>(m_pSipMsg->GetHdrObj(SipHeaderBase::USER_AGENT));
    if (pUserAgentHdr != SIP_NULL)
    {
        SipUserAgentHeader* pUserAgent = new SipUserAgentHeader(*pUserAgentHdr);
        pUserAgentHdr->SipDelete();

        bStatus = pSipAckMsg->SetHeader(pUserAgent);
        pUserAgent->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "PrepareACK: Set UserAgent Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }
    if (SipConfiguration::GetInstance()->IsPANIHeaderReqdForACK())
    {
        /* Set PANI header from INVITE Request*/
        SipHeaderBase* pPANIHdr = m_pSipMsg->GetHdrObj(SipHeaderBase::P_ACCESS_NETWORK_INFO);
        if (pPANIHdr != SIP_NULL)
        {
            SipHeaderBase* pPANI = new SipHeaderBase(*pPANIHdr);
            pPANIHdr->SipDelete();
            bStatus = pSipAckMsg->SetHeader(pPANI);
            pPANI->SipDelete();
            if (bStatus == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "PrepareACK: Set PANI Failed \n", SIP_ZERO, SIP_ZERO);
                pSipAckMsg->SipDelete();
                return SIP_FALSE;
            }
        }
    }

    /* If Route is present in INVITE Request ,Copy Route Header*/
    /* Need to Clone */
    SipHeaderList* pRouteList = m_pSipMsg->GetHdrList(SipHeaderBase::ROUTE);
    if (pRouteList != SIP_NULL)
    {
        bStatus = pSipAckMsg->SetHdrList(pRouteList);
        pRouteList->SipDelete();
        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "PrepareACK: Set Route Failed \n", SIP_ZERO, SIP_ZERO);
            pSipAckMsg->SipDelete();
            return SIP_FALSE;
        }
    }

    *ppSipAckMsg = pSipAckMsg;
    return SIP_TRUE;
}

SIP_VOID* SipTxn::GetTimerId()
{
    return m_pvTimerId;
}

SIP_BOOL SipTxn::UpdateTranspInfo(SipTransportInfo* pTranspInfo)
{
    if (m_pTranspInfo != SIP_NULL)
    {
        delete m_pTranspInfo;
    }

    m_pTranspInfo = pTranspInfo;
    return SIP_TRUE;
}

/* Remove Txn entry from DB. NOTE-> it does not delete Txn obj */
SIP_VOID SipTxn::RemoveFromTxnPool()
{
    SipTxn_RemoveFromTxnPool(m_pTxnKey);
}

SipTransportInfo* SipTxn::GetTranspInfo()
{
    return m_pTranspInfo;
}

ISipUserData* SipTxn::GetUserData()
{
    return m_pUserData;
}

SIP_INT32 SipTxn::GetMsgSentProto()
{
    if (m_pTranspInfo == SIP_NULL)
    {
        /*  stack error */
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTRANSP,
                "SipTransp_OnReceiveTranspLayer: sipFetchElement Error \n", SIP_ZERO, SIP_ZERO);
        return SipTransportInfo::PROTOCOL_INVALID;
    }

    const SipTransportParameter* pMsgSentTransParam = m_pTranspInfo->GetMsgSentTranspParam();
    if (pMsgSentTransParam == SIP_NULL)
    {
        /*  stack error */
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTRANSP,
                "SipTransp_OnReceiveTranspLayer: sipFetchElement Error \n", SIP_ZERO, SIP_ZERO);
        return SipTransportInfo::PROTOCOL_INVALID;
    }

    SIP_INT32 eTranspMsgSentProtocol = pMsgSentTransParam->GetTranspProtocol();

    return eTranspMsgSentProtocol;
}

SIP_VOID SipTxn::SetUserData(ISipUserData* pUserData)
{
    if (m_pUserData != SIP_NULL)
    {
        delete m_pUserData;
    }

    m_pUserData = pUserData;
}

SIP_BOOL SipTxn::IsTxnTerminated()
{
    switch (m_eTxnType)
    {
        case SipTxn::INVITE_CLIENT:
        {
            if (m_nTxnState == SipTxn::INV_CLI_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;

        case SipTxn::NON_INVITE_CLIENT:
        {
            if (m_nTxnState == SipTxn::NON_INV_CLI_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;

        case SipTxn::INVITE_SERVER:
        {
            if (m_nTxnState == SipTxn::INV_SER_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;
        case SipTxn::NON_INVITE_SERVER:
        {
            if (m_nTxnState == SipTxn::NON_INV_SER_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;

        default:
        {
            return SIP_FALSE;
        }
    }

    return SIP_FALSE;
}

SIP_VOID SipTxn::InitRetransmissionInfo()
{
    m_nRetransmissionCount = SIP_ZERO;
    m_nMaxDuration = SIP_ZERO;
    m_nDurationExpired = SIP_ZERO;
    m_nCurrentDuration = SIP_ZERO;
}

SIP_VOID SipTxn::SetResponseCode(SIP_UINT16 nRespCode)
{
    if (m_pTxnKey != SIP_NULL)
    {
        m_pTxnKey->SetResponseCode(nRespCode);
    }
}

SIP_VOID CbkTxnTimeout(SIP_VOID* pvobjTimeoutData, const SIP_VOID* pvTimerId)
{
    SipTimeoutData* pTimeoutData = static_cast<SipTimeoutData*>(pvobjTimeoutData);
    if (pTimeoutData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "CbkTxnTimeout: pTimeoutData is NULL", SIP_ZERO, SIP_ZERO);
        return;
    }

    SipTxnKey* pTxnKey = pTimeoutData->GetTxnKey();
    SipTxn* pTxn = SIP_NULL;
    SipUtil* pUtil = SipUtil::GetInstance();
    ISipTransactionCallback* pCallback = pUtil->GetTransactionCallback();
    SIP_BOOL bTxnExist = SIP_FALSE;

    if (pCallback != SIP_NULL)
    {
        bTxnExist = pCallback->FetchTransaction(pTxnKey, SipTxn::OPT_FETCH, pTxn);
    }

    if (bTxnExist == SIP_YES)
    {
        if (pTxn == SIP_NULL)
        {
            /*  stack error */
            SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "CbkTxnTimeout: Stack Error,Db Status Error",
                    SIP_ZERO, SIP_ZERO);
            delete pTimeoutData;
            return;
        }
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "CbkTxnTimeout: TXN Not Exists\n", SIP_ZERO, SIP_ZERO);
        delete pTimeoutData;
        return;
    }

    pTxn->Increment();

    /* Timer Id is no longer valid. So remove from Transaction Object */

    if (pvTimerId != pTxn->GetTimerId())
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "CbkTxnTimeout --> Timer ID Mismatch", SIP_ZERO, SIP_ZERO);

        delete pTimeoutData;
        pTxn->SipDelete();
        return;
    }

    pTxn->SetTimerId(SIP_NULL);

    ISipUserData* pUserData = pTxn->GetUserData();
    /*No need to notify txntimeout to listener if the txn is already terminated*/
    /* Notify user is txn is terminated */
    SIP_INT32 eTimerType = SipTxn::TIMER_TYPE_INVALID;
    if (pTxn->IsTxnTerminated() == SIP_TRUE)
    {
        if (pCallback != SIP_NULL)
        {
            pCallback->NotifyTimerExpired(pUserData, (SIP_INT32)eTimerType);
        }

        SipTxn_RemoveFromTxnPool(pTxnKey);

        delete pTimeoutData;
        pTxn->SipDelete();

        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "***CbkTxnTimeout --> TxnTerminated***\n", SIP_ZERO, SIP_ZERO);
        return;
    }

    SIP_UINT16 nEvent = SIP_INVALID;
    SIP_INT32 eTxnType = pTxn->GetTxnType();
    eTimerType = pTimeoutData->GetTimerType();
    SIP_BOOL bStatus = SIP_TRUE;

    switch (eTxnType)
    {
        case SipTxn::INVITE_CLIENT:
        {
            if ((eTimerType == SipTxn::TIMER_A) || (eTimerType == SipTxn::TIMER_B))
            {
                nEvent = SipTxn::INV_CLI_TIMER_A_B_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMER_D)
            {
                nEvent = SipTxn::INV_CLI_TIMER_D_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMER_M)
            {
                nEvent = SipTxn::INV_CLI_TIMER_M_TIME_OUT_EVT;
            }
            else
            {
                nEvent = SipTxn::INV_CLI_INVALID_EVT;
                SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN,
                        "CbkTxnTimeout[InvCliTxn]: Invalid Timer Evt[%d]", eTimerType, SIP_ZERO);

                bStatus = SIP_FALSE;
            }
        }
        break;

        case SipTxn::NON_INVITE_CLIENT:
        {
            if ((eTimerType == SipTxn::TIMER_E) || (eTimerType == SipTxn::TIMER_F))
            {
                nEvent = SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMER_K)
            {
                nEvent = SipTxn::NON_INV_CLI_TIMER_K_TIME_OUT_EVT;
            }
            else
            {
                nEvent = SipTxn::NON_INV_CLI_INVALID_EVT;
                SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN,
                        "CbkTxnTimeout[NonInvCliTxn]: Invalid Timer Evt[%d]", eTimerType, SIP_ZERO);

                bStatus = SIP_FALSE;
            }
        }
        break;

        case SipTxn::INVITE_SERVER:
        {
            if ((eTimerType == SipTxn::TIMER_G) || (eTimerType == SipTxn::TIMER_H))
            {
                nEvent = SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMER_I)
            {
                nEvent = SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMER_L)
            {
                nEvent = SipTxn::INV_SER_TIMER_L_TIME_OUT_EVT;
            }
            else
            {
                nEvent = SipTxn::INV_SER_INVALID_EVT;
                SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN,
                        "CbkTxnTimeout[InvSerTxn]: Invalid Timer Evt[%d]", eTimerType, SIP_ZERO);

                bStatus = SIP_FALSE;
            }
        }
        break;
        case SipTxn::NON_INVITE_SERVER:
        {
            if (eTimerType == SipTxn::TIMER_J)
            {
                nEvent = SipTxn::NON_INV_SER_TIMER_J_TIME_OUT_EVT;
            }
            else
            {
                nEvent = SipTxn::NON_INV_SER_INVALID_EVT;
                SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN,
                        "CbkTxnTimeout[NonInvSerTxn]: Invalid Timer Evt[%d]", eTimerType, SIP_ZERO);

                bStatus = SIP_FALSE;
            }
        }
        break;

        default:
        {
            SIP_DEBUG_STACKBUG(
                    ESIPTRACE_MODTXN, "CbkTxnTimeout: Invalid Txn type", SIP_ZERO, SIP_ZERO);

            bStatus = SIP_FALSE;
        }
        break;
    }

    if (bStatus == SIP_FALSE)
    {
        delete pTimeoutData;
        pTxn->SipDelete();
        return;
    }

    SIP_UINT16 nError = SIP_ZERO;
    bStatus = pTxn->InvokeFsm(nEvent, pTimeoutData, &nError);
    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "CbkTxnTimeout: Invoke FSM Fail", SIP_ZERO, SIP_ZERO);

        delete pTimeoutData;
        pTxn->SipDelete();
        return;
    }

    /*No need to notify txntimeout to listener if the txn is already terminated*/
    SIP_BOOL bTxnTerminated = pTxn->IsTxnTerminated();

    if (bTxnTerminated == SIP_TRUE || pTxn->IsRprTxnTerminated() == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "***CbkTxnTimeout: %s***",
                (bTxnTerminated == SIP_TRUE) ? "TxnTerminated" : "RprTxnTerminated", SIP_ZERO);

        if (pCallback != SIP_NULL)
        {
            pCallback->NotifyTimerExpired(pUserData, (SIP_INT32)eTimerType);
        }

        if (bTxnTerminated == SIP_TRUE)
        {
            SipTxn_RemoveFromTxnPool(pTxnKey);
            pTxn->SipDelete();
        }

        delete pTimeoutData;
        return;
    }

    delete pTimeoutData;

    /* On transaction timer expiry, if txn is not terminated, it mean there are message
    to re-transmit */
    ISipNetworkUtil* pNetworkUtil = pUtil->GetNetwork();
    SipTransportInfo* pTranspInfo = pTxn->GetTranspInfo();

    if (pTranspInfo != SIP_NULL)
    {
        SipTransportBuffer* pTransSipBuffer = pTranspInfo->GetTranspSipBuffer();
        SipTransportParameter* pActualDestParam = pTranspInfo->GetMsgSentTranspParam();

        bStatus = pNetworkUtil->SendToNetwork(pTransSipBuffer, pActualDestParam, pUserData);

        pTxn->SipDelete();

        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "CbkTxnTimeout: SendToNetwork Fail", SIP_ZERO, SIP_ZERO);
            return;
        }
    }
    else
    {
        pTxn->SipDelete();
    }
}

SIP_VOID SipTxn_RemoveFromTxnPool(SipTxnKey* pTxnKey)
{
    SipTxn* pTempTxn = SIP_NULL;
    SipTxnKey* pTempTxnKey = SIP_NULL;
    ISipTransactionCallback* pCallback = SipUtil::GetInstance()->GetTransactionCallback();
    SIP_BOOL bStatus = SIP_FALSE;

    if (pCallback != SIP_NULL)
    {
        bStatus = pCallback->ReleaseTransaction(pTxnKey, SipTxn::OPT_REMOVE, pTempTxnKey, pTempTxn);
    }

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "SipTxn_RemoveFromTxnPool:\n", SIP_ZERO, SIP_ZERO);
        return;
    }

    if (pTempTxnKey != SIP_NULL)
    {
        pTempTxnKey->SipDelete();
    }

    if (pTempTxn != SIP_NULL)
    {
        pTempTxn->SipDelete();
    }
}
