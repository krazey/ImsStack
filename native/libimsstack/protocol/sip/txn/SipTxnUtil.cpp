/*
   Author
   <table>
   date      author                  description
   --------  --------------          ----------
    20100000  syed.malgimani@        Created
   20170110  vijay.nair@             Modified
   </table>

   Description

 */

#include "SipTrace.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"

#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"
#include "msg/sip_msgutil.h"

#include "IMSTypeDef.h"

extern SIP_VOID sip_cbk_displayTxnKey(IN SIP_VOID* pvTxnKey);

SipTxnUtil* SipTxnUtil::m_pSipTxnUtil = SIP_NULL;

/*
 * Description        : This function is constructor for class SipTxnUtil
 */
SipTxnUtil::SipTxnUtil()
        : m_txnKeyList(SipVector<SipTxnKey*>())
{
}

/*
 * Description        : This function gets Class Instance
 */
SipTxnUtil* SipTxnUtil::GetInstance()
{
     if (m_pSipTxnUtil == SIP_NULL)
     {
         m_pSipTxnUtil = new SipTxnUtil();
     }
     return m_pSipTxnUtil;
}

/*
 * Description        : This function searches the list and return the specific TxnKey
 */
SipTxnKey* SipTxnUtil::SearchTxnKey(SipTxnKey* pUserTxnkey, SIP_BOOL bCheckRSeq)
{
    if (pUserTxnkey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnUtil::SearchTxnKey: Txn Object is Null",
                SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }

    SIP_UINT32 nSize = m_txnKeyList.GetSize();

    if (nSize <= SIP_ZERO)
    {
        return SIP_NULL;
    }

    SIP_UINT32 nStoredRules = pUserTxnkey->GetRules();

    pUserTxnkey->SetRules(SipTxnKey::RULE_COMPARE_TO_TAG);

    if (bCheckRSeq == SIP_TRUE)
    {
        pUserTxnkey->AddRule(SipTxnKey::RULE_COMPARE_RSEQ);
    }

    SIP_UINT16 nIndex = SIP_ZERO;
    while (nIndex < nSize)
    {
        SipTxnKey* pStoredTxnKey = m_txnKeyList.GetAt(nIndex++);
        if (pStoredTxnKey == SIP_NULL)
        {
            pUserTxnkey->SetRules(nStoredRules);
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnUtil::SearchTxnKey: Element Retrieval from List Failed",
                SIP_ZERO, SIP_ZERO);
            return SIP_NULL;
        }

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
            "SipTxnUtil::SearchTxnKey, CallID: %s, From Tag: %s.",
            pStoredTxnKey->GetCallId(), pStoredTxnKey->GetFromTag());

        if (pStoredTxnKey->CompareKeysForRPR(pUserTxnkey) != SIP_MATCHES)
        {
            continue;
        }

        pUserTxnkey->SetRules(nStoredRules);

        return pStoredTxnKey;
    }

    pUserTxnkey->SetRules(nStoredRules);

    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnUtil::SearchTxnKey: Element Not Found in the List",
                SIP_ZERO, SIP_ZERO);

    return SIP_NULL;
}

SIP_BOOL SipTxnUtil::IsTxnKeyMatched(SipTxnKey* pUserTxnkey, SipTxnKey* pStoredTxnKey)
{
    if (pStoredTxnKey == SIP_NULL)
    {
        return SIP_FALSE;
    }

    if (pStoredTxnKey->CompareKeysForRPR(pUserTxnkey) != SIP_MATCHES)
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
/*
 * Description        : This function Adds Tnx Key to the list
 */
SIP_BOOL SipTxnUtil::AddTxnKey(SipTxnKey* pTxnKey)
{
    if (m_txnKeyList.Add(pTxnKey) < 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipTxnUtil::AddTxnKey:Adding in list failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipTxnUtil::DeleteTxnKey(SipTxnKey* pUserTxnkey, SIP_BOOL bCheckToTag)
{
    if (pUserTxnkey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnUtil::DeleteTxnKey: Txn Object is Null",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_txnKeyList.GetSize();
    if (nSize <= SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnUtil::DeleteTxnKey: List Size Zero",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nStoredRules = pUserTxnkey->GetRules();

    pUserTxnkey->SetRules(SipTxnKey::RULE_NONE);

    if (bCheckToTag == SIP_TRUE)
    {
        pUserTxnkey->AddRule(SipTxnKey::RULE_COMPARE_TO_TAG);
    }

    SIP_UINT32 nIndex = SIP_ZERO;
    SipTxnKey* pStoredTxnKey = SIP_NULL;
    while (nIndex < nSize)
    {
        pStoredTxnKey = m_txnKeyList.GetAt(nIndex++);
        if (pStoredTxnKey == SIP_NULL)
        {
            pUserTxnkey->SetRules(nStoredRules);
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                "SipTxnUtil::DeleteTxnKey: Element Retrieval from List Failed",
                SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (IsTxnKeyMatched(pUserTxnkey, pStoredTxnKey) == SIP_TRUE)
        {
            sip_cbk_displayTxnKey((SIP_VOID*)pStoredTxnKey);
            pStoredTxnKey->SipDelete();
            m_txnKeyList.RemoveAt(nIndex - SIP_ONE);
            //Check again if further elements matches for the same txn key.
            nIndex--;
            nSize--;
        }
    }

    pUserTxnkey->SetRules(nStoredRules);

    return SIP_TRUE;
}
