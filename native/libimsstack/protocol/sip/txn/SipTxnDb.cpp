#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"

#include "SipTrace.h"
#include "sip_debug.h"
#include "sip_error.h"
#include "txn/SipTxnDb.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxn.h"

#include "IpAddress.h"

#define SIP_MAX_HASH_BUCKETS 100
#define SIP_MAX_HASH_ENTRIES 4000
#define SIP_CALC_HASH_VAL_A  07777777777
#define SIP_CALC_HASH_VAL_B  3
#define SIP_CALC_HASH_VAL_C  28
#define SIP_CALC_HASH_VAL_D  40
#define SIP_CALC_HASH_VAL_E  0140

extern SIP_VOID sip_cbk_displayTxnKey(IN SIP_VOID* pvTxnKey);

/* Global Variable */
static SipTxnDb* gpTxnDb = SIP_NULL;

SIP_UINT32 sipTxnCalculateHash(SIP_VOID* pvStrKey)
{
    if (pvStrKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(EERR_INVALIDPARAM, "sipCalculateHash: Key is Null", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Prepare Key String = CallId */
    SipTxnKey* pTxnKey = (SipTxnKey*)(pvStrKey);
    SIP_CHAR* pszCallId = pTxnKey->GetCallId();
    SIP_UINT16 nKeyLen = SipPf_Strlen(pszCallId);

    SIP_CHAR* pszStr = pszCallId;
    SIP_CHAR* pszStrEnd = pszStr + nKeyLen;
    SIP_CHAR cVal = SIP_ZERO;
    SIP_UINT32 nHash = SIP_ZERO;

    while (pszStr != pszStrEnd)
    {
        cVal = *pszStr;
        pszStr = pszStr + 1;
        if (cVal >= SIP_CALC_HASH_VAL_E)
        {
            cVal = cVal - SIP_CALC_HASH_VAL_D;
        }
        nHash = ((nHash << SIP_CALC_HASH_VAL_B) + (nHash >> SIP_CALC_HASH_VAL_C) + cVal);
    }

    return nHash & SIP_CALC_HASH_VAL_A;
}

SIP_CHAR sipTxnCompareHashKey(SIP_VOID* pvStoredKey, SIP_VOID* pvUserKey)
{
    if ((pvStoredKey == SIP_NULL) || (pvUserKey == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "sipCompareHashKey: pvStoredKey or pvUserKey is Null",
                SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    SipTxnKey* pStoredKey = (SipTxnKey*)pvStoredKey;
    SipTxnKey* pUserKey = (SipTxnKey*)pvUserKey;
    if (pUserKey->HasRule(SipTxnKey::RULE_COMPARE_VIA_BRANCH) == SIP_TRUE)
    {
        if (SipPf_Strcmp(pStoredKey->GetViaBranchParam(), pUserKey->GetViaBranchParam()) !=
                SIP_EQUALS)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "sipCompareHashKey: Branch Parameter doesn't match",
                    SIP_ZERO, SIP_ZERO);
            return SIP_NOT_MATCH;
        }
    }

    /* Compare Method for Non-ACK only */
    SIP_CHAR* pszUserMethod = pUserKey->GetMethod();
    SIP_BOOL bACKFor2xxSent = SIP_FALSE;

    if (SipPf_Strcmp(ACK_METHOD, pszUserMethod) != SIP_EQUALS)
    {
        if (SipPf_Strcmp(pStoredKey->GetMethod(), pszUserMethod) != SIP_EQUALS)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "sipCompareHashKey: CseqMethod doesn't match",
                    SIP_ZERO, SIP_ZERO);
            return SIP_NOT_MATCH;
        }
    }
    else
    {  // 487 retransmission issue.
        if (SipPf_Strcmp(pStoredKey->GetMethod(), CANCEL_METHOD) == SIP_ZERO)
        {
            return SIP_NOT_MATCH;
        }

        // Successful response & received ACK request : always new one
        // Test equipment issue: same transaction key - cseq / via-branch / from-tag / to-tag
        if (pUserKey->GetTxnType() == SipTxn::INV_SER_TXN &&
                pStoredKey->GetTxnType() == SipTxn::INV_SER_TXN)
        {
            SIP_UINT16 nStoredStatusCode = pStoredKey->GetRespCode();

            if (SIP_SUCCESSFUL_RESP(nStoredStatusCode))
            {
                bACKFor2xxSent = SIP_TRUE;
            }
        }
    }

    /*Host validation*/
    if (pUserKey->HasRule(SipTxnKey::RULE_COMPARE_VIA_BRANCH) == SIP_TRUE)
    {
        IPAddress objStoredHost(AString((CONST IMS_CHAR*)pStoredKey->GetViaHost()));
        IPAddress objRecevdHost(AString((CONST IMS_CHAR*)pUserKey->GetViaHost()));
        if (objStoredHost.Equals(objRecevdHost) == SIP_FALSE)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "sipCompareHashKey: VIA Host doesn't match",
                    SIP_ZERO, SIP_ZERO);
            return SIP_NOT_MATCH;
        }
    }

    if (SipPf_Strcmp(pStoredKey->GetFromTag(), pUserKey->GetFromTag()) != SIP_EQUALS)
    {
        SIP_TRACE_NORMAL(
                ESIPTRACE_MODTXN, "sipCompareHashKey: SentByVal doesn't match", SIP_ZERO, SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    SIP_CHAR* pStoredToTag = pStoredKey->GetToTag();
    SIP_CHAR* pUserToTag = pUserKey->GetToTag();
    if ((pStoredToTag != SIP_NULL) && (pUserToTag != SIP_NULL))
    {
        if (SipPf_Strcmp(pStoredToTag, pUserToTag) != SIP_EQUALS)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODTXN, "sipCompareHashKey: SentByVal doesn't match",
                    SIP_ZERO, SIP_ZERO);
            return SIP_NOT_MATCH;
        }
    }

    if (bACKFor2xxSent == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "txn-comparison: ACK for 2xx sent has same transaction identifiers", SIP_ZERO,
                SIP_ZERO);
        return SIP_NOT_MATCH;
    }

    return SIP_MATCHES;
}

SIP_VOID sipTxnFreeElement(SIP_VOID* pvHashElem)
{
    SipTxn* pTxn = (SipTxn*)pvHashElem;

    if (pTxn != SIP_NULL)
    {
        pTxn->SipDelete();
    }

    SIP_TRACE_NORMAL(
            ESIPTRACE_MODTXN, "sipTxnFreeElement: TxnObject is freed ", SIP_ZERO, SIP_ZERO);
}

/* Default free function to free the key in the hash table */
SIP_VOID sipTxnFreeKey(SIP_VOID* pvHashKey)
{
    SipTxnKey* pTxnKey = (SipTxnKey*)pvHashKey;

    if (pTxnKey != SIP_NULL)
    {
        sip_cbk_displayTxnKey(pTxnKey);
        pTxnKey->SipDelete();
    }
}

SipTxnDb::SipTxnDb()
{
    SIP_UINT16 nError = SIP_ZERO;

    m_pTxnHash = new SipHash(sipTxnCalculateHash, sipTxnCompareHashKey, sipTxnFreeElement,
            sipTxnFreeKey, SIP_MAX_HASH_BUCKETS, SIP_MAX_HASH_ENTRIES, &nError);
}

SipTxnDb::~SipTxnDb()
{
    delete m_pTxnHash;
    m_pTxnHash = SIP_NULL;
}

SIP_BOOL SipTxnDb::AddElement(SIP_VOID* pvElement, SIP_VOID* pvKey, SIP_UINT16* pnError)
{
    return m_pTxnHash->Hash_Add(pvElement, pvKey, pnError);
}

SIP_BOOL SipTxnDb::FetchElement(SIP_VOID* pvKey, SIP_VOID** ppElement, SIP_UINT16* pnError)
{
    /* Fetch Element increments Reference count */
    *ppElement = m_pTxnHash->Hash_Fetch(pvKey, pnError);
    if (*ppElement == SIP_NULL)
    {
        return SIP_FALSE;
    }
    /* Reduce Reference Count */
    m_pTxnHash->Hash_Release(pvKey);
    return SIP_TRUE;
}

SIP_BOOL SipTxnDb::FetchElement(
        SIP_VOID* pvKey, SIP_VOID** ppElement, SIP_VOID** ppKey, SIP_UINT16* pnError)
{
    /* Fetch Element increments Reference count */
    *ppElement = m_pTxnHash->Hash_Fetch(pvKey, ppKey, pnError);
    if (*ppElement == SIP_NULL)
    {
        return SIP_FALSE;
    }
    /* Reduce Reference Count */
    m_pTxnHash->Hash_Release(pvKey);
    return SIP_TRUE;
}

SIP_BOOL SipTxnDb::RemoveElement(SIP_VOID* pvKey, SIP_UINT16* pnError)
{
    return m_pTxnHash->Hash_Remove(pvKey, pnError);
}

void SipTxnDb_Construct()
{
    SipTxnDb* pTxnDb = gpTxnDb;

    if (pTxnDb)
    {
        return;
    }

    pTxnDb = new SipTxnDb();
    gpTxnDb = pTxnDb;
}

void SipTxnDb_Destruct()
{
    SipTxnDb* pTxnDb = gpTxnDb;

    if (pTxnDb == SIP_NULL)
    {
        return;
    }

    delete pTxnDb;
    gpTxnDb = SIP_NULL;
}

SipTxnDb* SipTxnDb_GetInstance()
{
    SipTxnDb* pTxnDb = gpTxnDb;
    return pTxnDb;
}
