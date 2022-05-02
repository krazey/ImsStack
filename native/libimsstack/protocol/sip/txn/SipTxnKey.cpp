/*
   Author
   <table>
   date      author                 description
   --------  --------------         ----------
   20100000  syed.malgimani@        Created
   20170110  vijay.nair@            Modified
   </table>

   Description

 */

#include "txn/SipTxn.h"
#include "txn/SipTxnKey.h"
#include "SipTrace.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"

SipTxnKey::SipTxnKey()
    :m_nRSeqNum (SIP_ZERO), m_eMsgType(SipMessage::TYPE_INVALID),
    m_pszViaBranchParam(SIP_NULL), m_pszViaHost (SIP_NULL), m_nViaHostPort (SIP_ZERO),
    m_pszMethod (SIP_NULL), m_nCseqNum (SIP_ZERO), m_pRequestUri(SIP_NULL),
    m_pszToTag(SIP_NULL), m_pszFromTag(SIP_NULL), m_pszCallId (SIP_NULL),
    m_nRespCode (SIP_ZERO), m_eTxnType(SipTxn::INVALID_TXN),
    m_nRules(RULE_COMPARE_TO_TAG | RULE_COMPARE_VIA_BRANCH)
{
}

SipTxnKey::SipTxnKey(SipTxnKey* pTxnKey, SIP_UINT16* pnError)
    : m_nRSeqNum (SIP_ZERO), m_eMsgType(SipMessage::TYPE_INVALID),
    m_pszViaBranchParam(SIP_NULL), m_pszViaHost (SIP_NULL), m_nViaHostPort (SIP_ZERO),
    m_pszMethod (SIP_NULL), m_nCseqNum (SIP_ZERO), m_pRequestUri(SIP_NULL),
    m_pszToTag(SIP_NULL), m_pszFromTag(SIP_NULL), m_pszCallId (SIP_NULL),
    m_nRespCode (SIP_ZERO), m_eTxnType(SipTxn::INVALID_TXN),
    m_nRules(RULE_COMPARE_TO_TAG | RULE_COMPARE_VIA_BRANCH)
{
    Init(pTxnKey, pnError);
}

SipTxnKey::SipTxnKey(SipMessage* pSipMsg, SIP_UINT16* pnError)
    : m_nRSeqNum(SIP_ZERO), m_eMsgType(SipMessage::TYPE_INVALID),
    m_pszViaBranchParam(SIP_NULL), m_pszViaHost (SIP_NULL), m_nViaHostPort (SIP_ZERO),
    m_pszMethod (SIP_NULL), m_nCseqNum (SIP_ZERO), m_pRequestUri(SIP_NULL),
    m_pszToTag(SIP_NULL), m_pszFromTag(SIP_NULL), m_pszCallId (SIP_NULL),
    m_nRespCode (SIP_ZERO), m_eTxnType(SipTxn::INVALID_TXN),
    m_nRules(RULE_COMPARE_TO_TAG | RULE_COMPARE_VIA_BRANCH)
{
    m_eMsgType = pSipMsg->GetMsgType();

    if (m_eMsgType == SipMessage::TYPE_INVALID)
    {
        *pnError = EMSGERR_INVALIDMSGTYPE;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey Constructor: Invalid Txn\n", SIP_ZERO, SIP_ZERO);
        return;
    }

    /* Fetch Branch : Branch can be null */
    SipViaHeader* pViaHdr = (SipViaHeader*)pSipMsg->GetHdrObj(SipHeaderBase::VIA);
    if (pViaHdr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey Constructor: ViaHdr is NULL Txn\n", SIP_ZERO, SIP_ZERO);
        return;
    }
    m_pszViaBranchParam = const_cast<SIP_CHAR*>(pViaHdr->GetBranch());

    /* Fetch Host : Host cannot be null */
    m_pszViaHost = SipPf_Strdup(pViaHdr->GetHost());

    if (m_pszViaHost == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey Constructor: No m_pszViaHost \n", SIP_ZERO, SIP_ZERO);
        return;
    }

    /* Fetch Port : Port cannot be 0 */
    m_nViaHostPort = pViaHdr->GetPort();
    pViaHdr->SipDelete();

    /* Fetch based on Message Type */
    if (m_eMsgType == SipMessage::REQ_TYPE)
    {
        SipAddrSpec* pAddrSpec = SIP_NULL;
        /* Fetch Request URI */
        SipRequestLine* pReqLine = pSipMsg->GetReqLine();
        if (pReqLine == SIP_NULL)
        {
            *pnError = EMSGERR_REQLINEMISSING;
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "SipTxnKey::SipTxnKey(pTxnKey): EMSGERR_REQLINEMISSING",
                    SIP_ZERO, SIP_ZERO);
            return;
        }
        pAddrSpec = pReqLine->GetReqUri();
        pReqLine->SipDelete();

        m_pRequestUri = new SipAddrSpec(*pAddrSpec);
        pAddrSpec->SipDelete();

        if (m_pRequestUri == SIP_NULL)
        {
            *pnError = E_ERR_PF_MALLOCFAILED;
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "SipTxnKey::SipTxnKey(pTxnKey): Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return;
        }

        /* Fetch Method */
        m_pszMethod = SipPf_Strdup(pReqLine->GetMethod());

        /* Fetch CSeq Num */
        SipCSeqHeader* pCseqHdr = (SipCSeqHeader *)pSipMsg->GetHdrObj(SipHeaderBase::CSEQ);
        if (pCseqHdr != SIP_NULL)
        {
            m_nCseqNum = pCseqHdr->GetCSeq();
            pCseqHdr->SipDelete();
        }
    }
    else
    {
        /* Fetch Method */
        SipCSeqHeader* pCseqHdr = (SipCSeqHeader *)pSipMsg->GetHdrObj(SipHeaderBase::CSEQ);
        if (pCseqHdr != SIP_NULL)
        {
            m_pszMethod = SipPf_Strdup(pCseqHdr->GetMethod());

            /* Fetch CSeq Num */
            m_nCseqNum = pCseqHdr->GetCSeq();
            pCseqHdr->SipDelete();
        }
        m_nRespCode = pSipMsg->GetStatusCode();
    }

    /* Fetch To-Tag */
    SipToHeader* pToHdr = (SipToHeader *)pSipMsg->GetHdrObj(SipHeaderBase::TO);
    if (pToHdr == SIP_NULL)
    {
        *pnError = EMSGERR_TOMISSING;
        return;
    }
    m_pszToTag = pToHdr->GetTag();

    pToHdr->SipDelete();

    /* Fetch From-Tag */
    SipFromHeader* pFromHdr = (SipFromHeader*)pSipMsg->GetHdrObj(SipHeaderBase::FROM);
    if (pFromHdr == SIP_NULL)
    {
        *pnError = EMSGERR_FROMMISSING;
        return;
    }
    m_pszFromTag = pFromHdr->GetTag();

    pFromHdr->SipDelete();

    /* Fetch Call-ID */
    SipHeaderBase* pCallHdr = pSipMsg->GetHdrObj(SipHeaderBase::CALL_ID);
    if (pCallHdr != SIP_NULL)
    {
        m_pszCallId = SipPf_Strdup(pCallHdr->GetValue());
        pCallHdr->SipDelete();
    }

    SipIntegerHeader* pRSeqhdr = (SipIntegerHeader*)pSipMsg->GetHdrObj(SipHeaderBase::RSEQ);
    if (pRSeqhdr != SIP_NULL)
    {
        SetRSeq(pRSeqhdr->GetValueInt());
        pRSeqhdr->SipDelete();
    }
}

SipTxnKey::~SipTxnKey()
{
    Clear();
}

SIP_VOID SipTxnKey::Init(SipTxnKey* pTxnKey, SIP_UINT16* pnError)
{
    if (pTxnKey == SIP_NULL)
    {
        return;
    }

    Clear();

    m_nRSeqNum = pTxnKey->m_nRSeqNum;
    m_eMsgType = pTxnKey->m_eMsgType;
    m_nRespCode = pTxnKey->m_nRespCode;
    m_eTxnType = pTxnKey->m_eTxnType;
    m_nRules = pTxnKey->m_nRules;

    m_pszViaBranchParam = (SIP_CHAR *)SipPf_Strdup((SIP_CHAR *)pTxnKey->m_pszViaBranchParam);
    if (m_pszViaBranchParam == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey::Init: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return;
    }
    m_pszViaHost = (SIP_CHAR *)SipPf_Strdup((SIP_CHAR *)pTxnKey->m_pszViaHost);
    if (m_pszViaHost == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey::Init: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        delete[] m_pszViaBranchParam;
        m_pszViaBranchParam = SIP_NULL;
        return;
    }
    m_nViaHostPort = pTxnKey->m_nViaHostPort;
    m_pszMethod = (SIP_CHAR *)SipPf_Strdup((SIP_CHAR *)pTxnKey->m_pszMethod);
    if (m_pszMethod == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey::Init: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        delete[] m_pszViaHost;
        m_pszViaHost = SIP_NULL;
        delete[] m_pszViaBranchParam;
        m_pszViaBranchParam = SIP_NULL;
        return;
    }

    m_nCseqNum = pTxnKey->m_nCseqNum;
    m_pRequestUri = new SipAddrSpec(*(pTxnKey->m_pRequestUri));
    if (m_pRequestUri == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey::Init: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        delete[] m_pszMethod;
        m_pszMethod = SIP_NULL;
        delete[] m_pszViaHost;
        m_pszViaHost = SIP_NULL;
        delete[] m_pszViaBranchParam;
        m_pszViaBranchParam = SIP_NULL;
        return;
    }

    if (pTxnKey->m_pszToTag != SIP_NULL)
    {
        m_pszToTag = (SIP_CHAR *)SipPf_Strdup((SIP_CHAR *)pTxnKey->m_pszToTag);
        if (m_pszToTag == SIP_NULL)
        {
            *pnError = E_ERR_PF_MALLOCFAILED;
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "SipTxnKey::Init: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            delete[] m_pszMethod;
            m_pszMethod = SIP_NULL;
            delete[] m_pszViaHost;
            m_pszViaHost = SIP_NULL;
            delete[] m_pszViaBranchParam;
            m_pszViaBranchParam = SIP_NULL;
            delete m_pRequestUri;
            m_pRequestUri = SIP_NULL;
            return;
        }
    }

    m_pszFromTag = (SIP_CHAR *)SipPf_Strdup((SIP_CHAR *)pTxnKey->m_pszFromTag);
    if (m_pszFromTag == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey::Init: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        delete[] m_pszToTag;
        m_pszToTag = SIP_NULL;
        delete[] m_pszMethod;
        m_pszMethod = SIP_NULL;
        delete[] m_pszViaHost;
        m_pszViaHost = SIP_NULL;
        delete[] m_pszViaBranchParam;
        m_pszViaBranchParam = SIP_NULL;
        delete m_pRequestUri;
        m_pRequestUri = SIP_NULL;
        return;
    }
    m_pszCallId = (SIP_CHAR *)SipPf_Strdup(
            (SIP_CHAR *)pTxnKey->m_pszCallId);
    if (m_pszCallId == SIP_NULL)
    {
        *pnError = E_ERR_PF_MALLOCFAILED;
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnKey::Init: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        delete[] m_pszFromTag;
        m_pszFromTag = SIP_NULL;
        delete[] m_pszToTag;
        m_pszToTag = SIP_NULL;
        delete[] m_pszMethod;
        m_pszMethod = SIP_NULL;
        delete[] m_pszViaHost;
        m_pszViaHost = SIP_NULL;
        delete[] m_pszViaBranchParam;
        m_pszViaBranchParam = SIP_NULL;
        delete m_pRequestUri;
        m_pRequestUri = SIP_NULL;
        return;
    }
}

SIP_VOID SipTxnKey::SetCallId(const SIP_CHAR* pszCallId)
{
    if (m_pszCallId != SIP_NULL)
    {
        delete[] m_pszCallId;
    }

    m_pszCallId = (SIP_CHAR *)SipPf_Strdup(pszCallId);
}

SIP_VOID SipTxnKey::SetFromTag(const SIP_CHAR* pszFromTag)
{
    if (m_pszFromTag != SIP_NULL)
    {
        delete[] m_pszFromTag;
    }

    m_pszFromTag = (SIP_CHAR*)SipPf_Strdup(pszFromTag);
}

SIP_VOID SipTxnKey::SetMethod(const SIP_CHAR* pszMethod)
{
    if (m_pszMethod != SIP_NULL)
    {
        delete[] m_pszMethod;
    }

    m_pszMethod = (SIP_CHAR*)SipPf_Strdup(pszMethod);
}

SIP_VOID SipTxnKey::SetRequestUri(SipAddrSpec* pRequestUri)
{
    if (m_pRequestUri != SIP_NULL)
    {
        m_pRequestUri->SipDelete();
    }

    m_pRequestUri = pRequestUri;
}

SIP_VOID SipTxnKey::SetToTag(const SIP_CHAR* pszToTag)
{
    if (m_pszToTag != SIP_NULL)
    {
        delete[] m_pszToTag;
    }

    m_pszToTag = (SIP_CHAR*)SipPf_Strdup(pszToTag);
}

SIP_VOID SipTxnKey::SetViaBranchParam(const SIP_CHAR* pszViaBranchParam)
{
    if (m_pszViaBranchParam != SIP_NULL)
    {
        delete[] m_pszViaBranchParam;
    }

    m_pszViaBranchParam = (SIP_CHAR*)SipPf_Strdup(pszViaBranchParam);
}

SIP_VOID SipTxnKey::SetViaHost(const SIP_CHAR* pszViaHost)
{
    if (m_pszViaHost != SIP_NULL)
    {
        delete[] m_pszViaHost;
    }

    m_pszViaHost = (SIP_CHAR *)SipPf_Strdup(pszViaHost);
}

SIP_INT32 SipTxnKey::CompareKeys(SipTxnKey* pGeneratedKey)
{
    if (pGeneratedKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "txn-comparison: GeneratedKey is null", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    if (pGeneratedKey->HasRule(RULE_COMPARE_VIA_BRANCH) == SIP_TRUE)
    {
        if (SipPf_Stricmp(m_pszViaBranchParam, pGeneratedKey->m_pszViaBranchParam) != SIP_EQUALS)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                    "txn-comparison: not-match >> Via branch", SIP_ZERO, SIP_ZERO);
            return SIP_NOT_MATCH;
        }
    }

    if (m_nCseqNum != pGeneratedKey->m_nCseqNum)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                "txn-comparison: not-match >> CSeq (s:%d, g:%d)",
                m_nCseqNum, pGeneratedKey->m_nCseqNum);
        return SIP_NOT_MATCH;
    }

    SIP_BOOL bACKFor2xxSent = SIP_FALSE;

    if (SipPf_Strcmp(ACK_METHOD, pGeneratedKey->m_pszMethod) != SIP_EQUALS)
    {
        if (SipPf_Strcmp(m_pszMethod, pGeneratedKey->m_pszMethod) != SIP_EQUALS)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                    "txn-comparison: not-match >> method", SIP_ZERO, SIP_ZERO);
            return SIP_NOT_MATCH;
        }
    }
    else
    {
        // 487 retransmission case
        if (SipPf_Strcmp(m_pszMethod, CANCEL_METHOD) == SIP_EQUALS)
        {
            return SIP_NOT_MATCH;
        }

        // Successful response & received ACK request : always new one
        // Test equipment issue: same transaction key - cseq / via-branch / from-tag / to-tag
        if (pGeneratedKey->m_eTxnType == SipTxn::INV_SER_TXN
                && m_eTxnType == SipTxn::INV_SER_TXN)
        {
            if (SIP_SUCCESSFUL_RESP(m_nRespCode))
            {
                bACKFor2xxSent = SIP_TRUE;
            }
        }
    }

    if (SipPf_Strcmp(m_pszCallId, pGeneratedKey->m_pszCallId) != SIP_EQUALS)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                "txn-comparison: not-match >> call-id", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    if (SipPf_Stricmp(m_pszFromTag, pGeneratedKey->m_pszFromTag) != SIP_EQUALS)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                "txn-comparison: not-match >> from-tag", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    if (HasRule(RULE_COMPARE_TO_TAG) == SIP_TRUE
            && pGeneratedKey->HasRule(RULE_COMPARE_TO_TAG) == SIP_TRUE)
    {
        if (m_pszToTag != SIP_NULL && pGeneratedKey->m_pszToTag != SIP_NULL)
        {
            if (SipPf_Stricmp(m_pszToTag, pGeneratedKey->m_pszToTag) != SIP_EQUALS)
            {
                SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                        "txn-comparison: not-match >> to-tag", SIP_ZERO, SIP_ZERO);
                return SIP_NOT_MATCH;
            }
        }
        else
        {
        }
    }

    if (pGeneratedKey->HasRule(RULE_COMPARE_RSEQ) == SIP_TRUE)
    {
        if (m_nRSeqNum != SIP_ZERO && pGeneratedKey->m_nRSeqNum != SIP_ZERO)
        {
            if (m_nRSeqNum != pGeneratedKey->m_nRSeqNum)
            {
                SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                        "txn-comparison: not-match >> RSeq (s:%d, g:%d)",
                        m_nRSeqNum, pGeneratedKey->m_nRSeqNum);
                return SIP_NOT_MATCH;
            }
        }
    }

    if (bACKFor2xxSent == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "txn-comparison: ACK for 2xx sent has same transaction identifiers",
                SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    return SIP_MATCHES;
}

SIP_INT32 SipTxnKey::CompareKeysForRPR(SipTxnKey* pGeneratedKey)
{
    if (pGeneratedKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "rpr-txn-comparison: GeneratedKey is null", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    if (SipPf_Strcmp(m_pszMethod, INVITE_METHOD) != SIP_EQUALS)
    {
        // Check if this condition is required or not...
        return SIP_NOT_MATCH;
    }

    if (SipPf_Strcmp(m_pszCallId, pGeneratedKey->m_pszCallId) != SIP_EQUALS)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                "rpr-txn-comparison: not-match >> call-id", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    if (SipPf_Stricmp(m_pszFromTag, pGeneratedKey->m_pszFromTag) != SIP_EQUALS)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                "rpr-txn-comparison: not-match >> from-tag", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    if (pGeneratedKey->HasRule(RULE_COMPARE_TO_TAG) == SIP_TRUE)
    {
        if (m_pszToTag != SIP_NULL && pGeneratedKey->m_pszToTag != SIP_NULL)
        {
            if (SipPf_Stricmp(m_pszToTag, pGeneratedKey->m_pszToTag) != SIP_EQUALS)
            {
                SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                        "rpr-txn-comparison: not-match >> to-tag", SIP_ZERO, SIP_ZERO);
                return SIP_NOT_MATCH;
            }
        }
    }

    if (pGeneratedKey->HasRule(RULE_COMPARE_RSEQ) == SIP_TRUE)
    {
        if (m_nRSeqNum != SIP_ZERO && pGeneratedKey->m_nRSeqNum != SIP_ZERO)
        {
            if (m_nRSeqNum != pGeneratedKey->m_nRSeqNum)
            {
                SIP_TRACE_NORMAL(ESIPTRACE_MODTXN,
                        "rpr-txn-comparison: not-match >> RSeq (s:%d, g:%d)",
                        m_nRSeqNum, pGeneratedKey->m_nRSeqNum);
                return SIP_NOT_MATCH;
            }
        }
    }

    return SIP_MATCHES;
}

SIP_VOID SipTxnKey::Clear()
{
    if (m_pszViaBranchParam != SIP_NULL)
    {
        delete[] m_pszViaBranchParam;
        m_pszViaBranchParam = SIP_NULL;
    }
    if (m_pszViaHost != SIP_NULL)
    {
        delete[] m_pszViaHost;
        m_pszViaHost = SIP_NULL;
    }
    if (m_pszMethod != SIP_NULL)
    {
        delete[] m_pszMethod;
        m_pszMethod = SIP_NULL;
    }
    if (m_pRequestUri != SIP_NULL)
    {
        m_pRequestUri->SipDelete();
        m_pRequestUri = SIP_NULL;
    }
    if (m_pszToTag != SIP_NULL)
    {
        delete[] m_pszToTag;
        m_pszToTag = SIP_NULL;
    }
    if (m_pszFromTag != SIP_NULL)
    {
        delete[] m_pszFromTag;
        m_pszFromTag = SIP_NULL;
    }
    if (m_pszCallId != SIP_NULL)
    {
        delete[] m_pszCallId;
        m_pszCallId = SIP_NULL;
    }

    m_nRSeqNum = SIP_ZERO;
    m_eMsgType = SipMessage::TYPE_INVALID;
    m_nViaHostPort = SIP_ZERO;
    m_nCseqNum = SIP_ZERO;
    m_nRespCode = SIP_ZERO;
    m_eTxnType = SipTxn::INVALID_TXN;
    m_nRules = RULE_COMPARE_TO_TAG | RULE_COMPARE_VIA_BRANCH;
}
