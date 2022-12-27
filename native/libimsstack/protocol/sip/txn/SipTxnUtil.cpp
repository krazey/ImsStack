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

#include "SipDebug.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnUtil.h"

extern SIP_VOID Sip_Cbk_DisplayTxnKey(IN SIP_VOID* pvTxnKey);

static SipVector<SipTxnKey*> s_txnKeyList;

/*
 * Description        : This function searches the list and return the specific TxnKey
 */
SipTxnKey* SipTxnUtil::SearchTxnKey(SipTxnKey* pUserTxnkey, SIP_BOOL bCheckRSeq)
{
    if (pUserTxnkey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnUtil::SearchTxnKey: Txn Object is Null",
                SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }

    SIP_UINT32 nSize = s_txnKeyList.GetSize();

    if (nSize == SIP_ZERO)
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
        SipTxnKey* pStoredTxnKey = s_txnKeyList.GetAt(nIndex++);
        if (pStoredTxnKey == SIP_NULL)
        {
            pUserTxnkey->SetRules(nStoredRules);
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "SipTxnUtil::SearchTxnKey: Element Retrieval from List Failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_NULL;
        }

        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnUtil::SearchTxnKey, CallID: %s, From Tag: %s.",
                pStoredTxnKey->GetCallId(), pStoredTxnKey->GetFromTag());

        if (pStoredTxnKey->CompareKeysForRPR(pUserTxnkey) != SIP_MATCHES)
        {
            continue;
        }

        pUserTxnkey->SetRules(nStoredRules);

        return pStoredTxnKey;
    }

    pUserTxnkey->SetRules(nStoredRules);

    SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnUtil::SearchTxnKey: Element Not Found in the List",
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
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipTxnUtil::CompareKeysForRPR Not matched",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
/*
 * Description        : This function Adds Tnx Key to the list
 */
SIP_BOOL SipTxnUtil::AddTxnKey(SipTxnKey* pTxnKey)
{
    if (s_txnKeyList.Add(pTxnKey) < 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipTxnUtil::AddTxnKey:Adding in list failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipTxnUtil::DeleteTxnKey(SipTxnKey* pUserTxnkey, SIP_BOOL bCheckToTag)
{
    if (pUserTxnkey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SipTxnUtil::DeleteTxnKey: Txn Object is Null",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = s_txnKeyList.GetSize();
    if (nSize == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTXN, "SipTxnUtil::DeleteTxnKey: List Size Zero", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nStoredRules = pUserTxnkey->GetRules();

    pUserTxnkey->SetRules(SipTxnKey::RULE_NONE);

    if (bCheckToTag == SIP_TRUE)
    {
        pUserTxnkey->AddRule(SipTxnKey::RULE_COMPARE_TO_TAG);
    }

    SIP_UINT32 nIndex = SIP_ZERO;
    while (nIndex < nSize)
    {
        SipTxnKey* pStoredTxnKey = s_txnKeyList.GetAt(nIndex++);
        if (pStoredTxnKey == SIP_NULL)
        {
            pUserTxnkey->SetRules(nStoredRules);
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN,
                    "SipTxnUtil::DeleteTxnKey: Element Retrieval from List Failed", SIP_ZERO,
                    SIP_ZERO);
            return SIP_FALSE;
        }

        if (IsTxnKeyMatched(pUserTxnkey, pStoredTxnKey) == SIP_TRUE)
        {
            Sip_Cbk_DisplayTxnKey(static_cast<SIP_VOID*>(pStoredTxnKey));
            pStoredTxnKey->SipDelete();
            s_txnKeyList.RemoveAt(nIndex - SIP_ONE);
            // Check again if further elements matches for the same txn key.
            nIndex--;
            nSize--;
        }
    }

    pUserTxnkey->SetRules(nStoredRules);

    return SIP_TRUE;
}
