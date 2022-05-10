#include "txn/SipTxn.h"
#include "txn/sip_txn_fsm.h"

#include "SipConfiguration.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "SipUtil.h"
#include "platform/sip_pf_string.h"
#include "txn/SipTimeoutData.h"

#ifdef SIP_TRACE_ENABLE
static SIP_CHAR szInvClientTxnFsmSt[SipTxn::INV_CLI_INVALID_ST + 1][SIP_15] = {
        "IdleSt", "CallingSt", "ProceedingSt", "CompletedSt", "TerminatedSt", "InvalidSt"};

static SIP_CHAR szInvClientTxnFsmEvt[SipTxn::INV_CLI_INVALID_EVT + 1][SIP_20] = {"SendInvReqEvt",
        "TimerA_B_TimeoutEvt", "TimerD_TimeoutEvt", "Recv1xxRespEvt", "Recv2xxRespEvt",
        "Recv3xx6xxRespEvt", "TranspErrorEvt", "InvalidEvt"};

static SIP_CHAR szInvSerTxnFsmSt[SipTxn::INV_SER_INVALID_ST + 1][SIP_15] = {
        "IdleSt", "ProceedingSt", "CompletedSt", "ConfirmedSt", "TerminatedSt", "InvalidSt"};

static SIP_CHAR szInvSerTxnFsmEvt[SipTxn::INV_SER_INVALID_EVT + 1][SIP_25] = {"RecvInvReqEvt",
        "SendNon100ProvRespEvt", "Send3xx6xxFailureRespEvt", "Send2xxSuccessRespEvt",
        "TranspErrorEvt", "RecvAckReqEvt", "TimerG_H_TimeoutEvt", "TimerI_TimeoutEvt",
        "InvalidEvt"};

static SIP_CHAR szNonInvClientTxnFsmSt[SipTxn::NON_INV_CLI_INVALID_ST + 1][SIP_15] = {
        "IdleSt", "TryingSt", "ProceedingSt", "CompletedSt", "TerminatedSt", "InvalidSt"};

static SIP_CHAR szNonInvClientTxnFsmEvt[SipTxn::NON_INV_CLI_INVALID_EVT + 1][SIP_22] = {
        "SendNonInvReq_Evt", "Timer_E_F_TimeoutEvt", "Recv1xxRespEvt", "Recv2xx6xxRespEvt",
        "TranspErrorEvt", "Timer_K_TimeoutEvt", "InvalidEvt"};

static SIP_CHAR szSipNonInvSerTxnFsmSt[SipTxn::NON_INV_SER_INVALID_ST + 1][SIP_15] = {
        "IdleSt", "TryingSt", "ProceedingSt", "CompletedSt", "TerminatedSt", "InvalidSt"};

static SIP_CHAR szSipNonInvSerTxnFsmEvt[SipTxn::NON_INV_SER_INVALID_EVT + 1][SIP_20] = {
        "RecvNonInvReqEvt", "Send1xxRespEvt", "Send2xx6xxRespEvt", "TranspErrorEvt",
        "Timer_J_TimeoutEvt", "InvalidEvt"};

static SIP_CHAR szSipTxnTimer[SipTxn::TIMER_TYPE_INVALID + 1][SIP_14] = {"Timer1", "Timer2",
        "Timer4", "TimerA", "TimerB", "TimerC", "TimerD", "TimerE", "TimerF", "TimerG", "TimerH",
        "TimerI", "TimerJ", "TimerK", "TimerOther", "TimerInvalid"};
#endif  // #ifdef SIP_TRACE_ENABLE

SipTxn::SipTxn() :
        m_eTxnType(SipTxn::INVALID_TXN),
        m_pTxnKey(SIP_NULL),
        m_pSipMsg(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        m_pUserData(SIP_NULL),
        m_nTxnState(SIP_ZERO),
        m_nReTxCount(SIP_ZERO),
        m_eTimerType(SipTxn::TIMER_TYPE_INVALID),
        m_pvTimerId(SIP_NULL),
        m_nMaxDuration(SIP_ZERO),
        m_nDurationExpired(SIP_ZERO),
        m_nCurrentDuration(SIP_ZERO)
{
}

SipTxn::SipTxn(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey, IN SipMessage* pSipMsg,
        IN SipTimerContext* pSipTxnTimerContext, OUT SIP_UINT16* pnError) :
        m_eTxnType(SipTxn::INVALID_TXN),
        m_pTxnKey(SIP_NULL),
        m_pSipMsg(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        m_pUserData(SIP_NULL),
        m_nTxnState(SIP_ZERO),
        m_nReTxCount(SIP_ZERO),
        m_eTimerType(SipTxn::TIMER_TYPE_INVALID),
        m_pvTimerId(SIP_NULL),
        m_nMaxDuration(SIP_ZERO),
        m_nDurationExpired(SIP_ZERO),
        m_nCurrentDuration(SIP_ZERO)
{
    m_eTxnType = eTxnType;
    SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "+SipTxn Newly Add m_eTxnType[%d] eTxnType[%d]", m_eTxnType,
            eTxnType);
    m_pTxnKey = new SipTxnKey(pTxnKey, pnError);

    if ((pSipTxnTimerContext != SIP_NULL) && (pSipTxnTimerContext->pTxnSipTxnTimers != SIP_NULL))
    {
        objTxnTimerValues.UpdateSipTimers(
                pSipTxnTimerContext->nTimerOptions, pSipTxnTimerContext->pTxnSipTxnTimers);
    }

    if (m_pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxn SipTxnKey NULL \n", SIP_ZERO, SIP_ZERO);
        return;
    }

    if (*pnError == E_ERR_PF_MALLOCFAILED)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxn Malloc Failed \n", SIP_ZERO, SIP_ZERO);
        delete m_pTxnKey;
        m_pTxnKey = SIP_NULL;
    }

    m_pSipMsg = new SipMessage(*pSipMsg);

    /* IDLE State */
    m_nTxnState = SIP_ZERO;
    m_nReTxCount = SIP_ZERO;
}

SipTxn::~SipTxn()
{
    SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn Destructor[%x]", this, 0);

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
        case SipTxn::INV_CLI_TXN:
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::InvokeFsm:InvCliTxn [%s][%s]",
                    szInvClientTxnFsmSt[m_nTxnState], szInvClientTxnFsmEvt[nEvent]);

            if (gpfSipInvClientTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "gpfSipInvClientTxnFsm Fails", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::INV_SER_TXN:
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::InvokeFsm:InvSerTxn [%s][%s]",
                    szInvSerTxnFsmSt[m_nTxnState], szInvSerTxnFsmEvt[nEvent]);

            if (gpfSipInvSerTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "gpfSipInvSerTxnFsm Fails", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::NON_INV_CLI_TXN:
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::InvokeFsm:NonInvCliTxn [%s][%s]",
                    szNonInvClientTxnFsmSt[m_nTxnState], szNonInvClientTxnFsmEvt[nEvent]);

            if (gpfSipNonInvClientTxnFsm[nTxnState][nEvent](this, pvData, pnError) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODTXN, "gpfSipNonInvClientTxnFsm Fail", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
        break;

        case SipTxn::NON_INV_SER_TXN:
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::InvokeFsm:NonInvSerTxn [%s][%s]",
                    szSipNonInvSerTxnFsmSt[m_nTxnState], szSipNonInvSerTxnFsmEvt[nEvent]);

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
    SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::AbortTxn Invoked", 0, 0);

    SipUtil* pUtil = SipUtil_GetInstance();
    if (pUtil == SIP_NULL)
    {
        return SIP_FALSE;
    }

    ISipTimerUtil* pTimer = pUtil->GetTimer();
    if (m_pvTimerId != SIP_NULL)
    {
        SipTimeoutData* pTimeoutData = (SipTimeoutData*)pTimer->StopTimerEx(m_pvTimerId);
        m_pvTimerId = SIP_NULL;

        if (pTimeoutData != SIP_NULL)
        {
            delete pTimeoutData;
        }
    }

    return SIP_TRUE;
}

SIP_BOOL SipTxn::StartTxnTimer(SIP_UINT32 eTimerType, SIP_UINT32 nDuration, SIP_UINT16* pnError)
{
    SipTimeoutData* pTimeoutData = new SipTimeoutData(m_eTxnType, (SIP_INT32)eTimerType, m_pTxnKey);

    if (pTimeoutData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer: Memory Failed \n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (*pnError == EERR_MALLOCFAILED)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer: Memory Failed \n", SIP_ZERO, SIP_ZERO);
        delete pTimeoutData;
        return SIP_FALSE;
    }

    if (nDuration > SIP_ZERO)
    {
        SipUtil* pUtil = SipUtil_GetInstance();

        if (pUtil == SIP_NULL)
        {
            SIP_TRACE_NORMAL(
                    ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer:pUtil is NULL ", SIP_ZERO, SIP_ZERO);
            delete pTimeoutData;
            return SIP_FALSE;
        }

        ISipTimerUtil* pTimer = pUtil->GetTimer();

        if (pTimer == SIP_NULL)
        {
            SIP_TRACE_NORMAL(
                    ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer:pTimer is NULL ", SIP_ZERO, SIP_ZERO);
            delete pTimeoutData;
            return SIP_FALSE;
        }

        if (pTimer->StartTimer(&m_pvTimerId, nDuration, SIP_FALSE, CbkTxnTimeout, pTimeoutData) ==
                SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODTXN, "StartTxnTimer: StartTimer Failed", SIP_ZERO, SIP_ZERO);

            delete pTimeoutData;
            return SIP_FALSE;
        }

        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::StartTxnTimer[%s,%d]",
                szSipTxnTimer[eTimerType], nDuration);

        m_eTimerType = (SIP_INT32)eTimerType;
        m_nCurrentDuration = nDuration;
    }
    else
    {
        m_eTimerType = (SIP_INT32)eTimerType;
        m_nCurrentDuration = nDuration;
        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                "SipTxn::StartTxnTimer, nDuration is zero, invoke FSM directly.", SIP_ZERO,
                SIP_ZERO);
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

    SipUtil* pUtil = SipUtil_GetInstance();
    if (pUtil == SIP_NULL)
    {
        SIP_TRACE_NORMAL(
                ESIPTRACE_MODTXN, "SipTxn::StopTxnTimer:pUtil is NULL ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    ISipTimerUtil* pTimer = pUtil->GetTimer();
    if (pTimer == SIP_NULL)
    {
        SIP_TRACE_NORMAL(
                ESIPTRACE_MODTXN, "SipTxn::StopTxnTimer:pTimer is NULL ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipTimeoutData* pTimeoutData = SIP_NULL;
    pTimeoutData = (SipTimeoutData*)pTimer->StopTimerEx(m_pvTimerId);
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
    SipFromHeader* pFromHdr = (SipFromHeader*)pSipRespMsg->GetHdrObj(SipHeaderBase::FROM);
    if (SIP_NULL != pFromHdr)
    {
        SipFromHeader* pFrom = new SipFromHeader(*pFromHdr);
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
            SipRequestLine* pReqLine = new SipRequestLine(
                    (SIP_CHAR*)ACK_METHOD, pNewObjReqUri, (SIP_CHAR*)SIP_SIPVERSION);
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
    SipToHeader* pToHdr = (SipToHeader*)pSipRespMsg->GetHdrObj(SipHeaderBase::TO);
    if (pToHdr != SIP_NULL)
    {
        SipToHeader* pTo = new SipToHeader(*pToHdr);
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
    SipViaHeader* pViaHdr = (SipViaHeader*)m_pSipMsg->GetHdrObj(SipHeaderBase::VIA);
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
    SipCSeqHeader* pCSeqHdr = (SipCSeqHeader*)m_pSipMsg->GetHdrObj(SipHeaderBase::CSEQ);
    if (pCSeqHdr != SIP_NULL)
    {
        SipCSeqHeader* pCseq = new SipCSeqHeader(*pCSeqHdr);
        pCSeqHdr->SipDelete();

        /* Set CSeq-Method     as ACk */
        pCseq->SetMethod((SIP_CHAR*)"ACK");

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
            (SipUserAgentHeader*)m_pSipMsg->GetHdrObj(SipHeaderBase::USER_AGENT);
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

    /* ###TODO Setting of Msg Body in Response currently not supported */
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
    SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::GetUserData Called", SIP_ZERO, SIP_ZERO);
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

    SipTransportParameter* pMsgSentTransParam = m_pTranspInfo->GetMsgSentTranspParam();
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

SIP_BOOL SipTxn::SetUserData(ISipUserData* pUserData)
{
    if (pUserData == SIP_NULL)
    {
        return SIP_TRUE;
    }

    SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "SipTxn::SetUserData Called", SIP_ZERO, SIP_ZERO);

    if (m_pUserData != SIP_NULL)
    {
        delete m_pUserData;
    }

    m_pUserData = pUserData;

    return SIP_TRUE;
}

SIP_BOOL SipTxn::IsTxnTerminated()
{
    switch (m_eTxnType)
    {
        case SipTxn::INV_CLI_TXN:
        {
            if (m_nTxnState == SipTxn::INV_CLI_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;

        case SipTxn::NON_INV_CLI_TXN:
        {
            if (m_nTxnState == SipTxn::NON_INV_CLI_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;

        case SipTxn::INV_SER_TXN:
        {
            if (m_nTxnState == SipTxn::INV_SER_TERMINATED_ST)
            {
                return SIP_TRUE;
            }
        }
        break;
        case SipTxn::NON_INV_SER_TXN:
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
        break;
    }

    return SIP_FALSE;
}

SIP_VOID SipTxn::InitRetransmissionInfo()
{
    m_nReTxCount = SIP_ZERO;
    m_nMaxDuration = SIP_ZERO;
    m_nDurationExpired = SIP_ZERO;
    m_nCurrentDuration = SIP_ZERO;
}

SIP_VOID SipTxn::SetRespCode(SIP_UINT16 nRespCode)
{
    if (m_pTxnKey != SIP_NULL)
    {
        m_pTxnKey->SetRespCode(nRespCode);
    }
}

SIP_VOID CbkTxnTimeout(SIP_VOID* pvobjTimeoutData, SIP_VOID* pvTimerId)
{
    SIP_TRACE_NORMAL(
            ESIPTRACE_MODTXN, "\n***CbkTxnTimeout --> Processing Started***", SIP_ZERO, SIP_ZERO);

    SipTimeoutData* pTimeoutData = (SipTimeoutData*)pvobjTimeoutData;
    if (pTimeoutData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "CbkTxnTimeout: pTimeoutData is NULL", SIP_ZERO, SIP_ZERO);
        return;
    }

    SipTxnKey* pTxnKey = pTimeoutData->GetTxnKey();
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = sip_cbk_fetchTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey),
            TXN_OPT_FETCH, SIP_NULL, reinterpret_cast<SIP_VOID**>(&pTxn));

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

    pTxn->increment();

    /* Timer Id is no longer valid. So remove from Transaction Object */

    if (pvTimerId != pTxn->GetTimerId())
    {
        SIP_TRACE_NORMAL(
                ESIPTRACE_MODTXN, "CbkTxnTimeout --> Timer ID Mismatch", SIP_ZERO, SIP_ZERO);

        delete pTimeoutData;
        pTxn->SipDelete();
        return;
    }

    pTxn->SetTimerId(SIP_NULL);

    SipUtil* pUtil = SipUtil_GetInstance();
    if (pUtil == SIP_NULL)
    {
        delete pTimeoutData;
        pTxn->SipDelete();
        return;
    }

    ISipUserData* pUserData = pTxn->GetUserData();

    /*No need to notify txntimeout to listener if the txn is already terminated*/
    /* Notify user is txn is terminated */
    ISipTxnListener* pTxnListener = pUtil->GetTxnListener();

    SIP_INT32 eTimerType = SipTxn::TIMER_TYPE_INVALID;
    if (pTxn->IsTxnTerminated() == SIP_TRUE)
    {
        if (pTxnListener != SIP_NULL)
        {
            pTxnListener->TxnTimeout(pUserData, (SIP_INT32)eTimerType);
        }

        SipTxn_RemoveFromTxnPool(pTxnKey);

        delete pTimeoutData;
        pTxn->SipDelete();

        SIP_TRACE_NORMAL(
                ESIPTRACE_MODTXN, "***CbkTxnTimeout --> TxnTerminated***\n", SIP_ZERO, SIP_ZERO);
        return;
    }

    SIP_UINT16 nEvent = SIP_INVALID;
    SIP_INT32 eTxnType = pTxn->GetTxnType();
    eTimerType = pTimeoutData->GetTimerType();
    SIP_BOOL bStatus = SIP_TRUE;

    switch (eTxnType)
    {
        case SipTxn::INV_CLI_TXN:
        {
            if ((eTimerType == SipTxn::TIMERA) || (eTimerType == SipTxn::TIMERB))
            {
                nEvent = SipTxn::INV_CLI_TIMERA_B_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMERD)
            {
                nEvent = SipTxn::INV_CLI_TIMERD_TIME_OUT_EVT;
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

        case SipTxn::NON_INV_CLI_TXN:
        {
            if ((eTimerType == SipTxn::TIMERE) || (eTimerType == SipTxn::TIMERF))
            {
                nEvent = SipTxn::NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMERK)
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

        case SipTxn::INV_SER_TXN:
        {
            if ((eTimerType == SipTxn::TIMERG) || (eTimerType == SipTxn::TIMERH))
            {
                nEvent = SipTxn::INV_SER_TIMER_G_H_TIME_OUT_EVT;
            }
            else if (eTimerType == SipTxn::TIMERI)
            {
                nEvent = SipTxn::INV_SER_TIMER_I_TIME_OUT_EVT;
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
        case SipTxn::NON_INV_SER_TXN:
        {
            if (eTimerType == SipTxn::TIMERJ)
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
    if (pTxn->IsTxnTerminated() == SIP_TRUE)
    {
        if (pTxnListener != SIP_NULL)
        {
            pTxnListener->TxnTimeout(pUserData, (SIP_INT32)eTimerType);
        }

        SipTxn_RemoveFromTxnPool(pTxnKey);

        delete pTimeoutData;
        pTxn->SipDelete();

        SIP_TRACE_NORMAL(
                ESIPTRACE_MODTXN, "***CbkTxnTimeout --> TxnTerminated***\n", SIP_ZERO, SIP_ZERO);
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

    SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "CbkTxnTimeout Processing Done", SIP_ZERO, SIP_ZERO);
}

SIP_VOID SipTxn_RemoveFromTxnPool(SipTxnKey* pTxnKey)
{
    SipTxn* pTempTxn = SIP_NULL;
    SipTxnKey* pTempTxnKey = SIP_NULL;

    if (sip_cbk_releaseTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey), TXN_OPT_REMOVE,
                reinterpret_cast<SIP_VOID**>(&pTempTxnKey),
                reinterpret_cast<SIP_VOID**>(&pTempTxn)) == SIP_FALSE)
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
