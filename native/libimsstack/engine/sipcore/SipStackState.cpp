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
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

#include "private/SipConfigV.h"

#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipPrivate.h"
#include "SipStack.h"
#include "SipStackState.h"
#include "SipStackTransaction.h"
#include "SipTxnContextData.h"

__IMS_TRACE_TAG_SIP_CORE__;

PRIVATE
SipStackState::SipStackState() :
        m_piLock(IMS_NULL),
        m_objTxnAggregate(ImsMap<IMS_UINT32, ImsList<SipStackTransaction*>>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC
SipStackState::~SipStackState()
{
    CleanUp();
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
void SipStackState::CleanUp()
{
    LockGuard objLock(m_piLock);

    IMS_TRACE_D("StackState: Number Of Transaction(%d)", m_objTxnAggregate.GetSize(), 0, 0);

    if (!m_objTxnAggregate.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objTxnAggregate.GetSize(); ++i)
        {
            ImsList<SipStackTransaction*>& objTransactions = m_objTxnAggregate.GetValueAt(i);

            for (IMS_UINT32 j = 0; j < objTransactions.GetSize(); ++j)
            {
                SipStackTransaction* pTransaction = objTransactions.GetAt(j);

                if (pTransaction == IMS_NULL)
                {
                    continue;
                }

                ::SipTxnKey* pKey = pTransaction->GetKey();

                SipStack::TerminateTransaction(pKey);

                IMS_TRACE_D("StackState: call-id=%s, via-branch=%s",
                        SipDebug::GetCharA1(SipStack::TxnKey_GetCallId(pKey), 8, '@'),
                        SipStack::TxnKey_GetViaBranch(pKey), 0);

                delete pTransaction;
            }
        }

        m_objTxnAggregate.Clear();
    }
}

PUBLIC
void SipStackState::StartUp()
{
    // For transaction layer handling
    // Initialize the retransmission initial timer values, timeout timer values,
    // and wait timer values.
    SipStack::Initialize();
}

PUBLIC
IMS_BOOL SipStackState::AbortTransaction(IN ::SipTxnKey* pKey, IN SipTransactionState* pTxnState)
{
    if (pKey == IMS_NULL || pTxnState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipTxnContext* pTxnContext = SipStack::CreateTxnContext();

    if (pTxnContext != IMS_NULL)
    {
        SipTxnContextData* pTxnContextData = new SipTxnContextData();

        if (pTxnContextData != IMS_NULL)
        {
            pTxnContextData->SetTxnState(pTxnState);
        }

        pTxnContext->m_pTxnContextData = reinterpret_cast<SIP_VOID*>(pTxnContextData);
    }

    // Release the stack transaction structure & stop retransmissions.
    if (!SipStack::AbortTransaction(pKey, pTxnContext))
    {
        SipStack::DestroyTxnContext(pTxnContext);
        return IMS_FALSE;
    }

    SipStack::DestroyTxnContext(pTxnContext);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipStackState::FetchTransaction(
        IN ::SipTxnKey* pKey, IN IMS_SINT32 nOption, OUT ::SipTxnKey*& pOutKey, OUT SipTxn*& pTxn)
{
    LockGuard objLock(m_piLock);
    const SipStackTransaction* pTransaction = FindTransaction(pKey);

    if (pTransaction == IMS_NULL)
    {
        if (nOption == TXN_CREATE)
        {
            return AddTransaction(pKey, pTxn);
        }
        else
        {
            pOutKey = IMS_NULL;
            pTxn = IMS_NULL;
            return IMS_FALSE;
        }
    }

    pOutKey = pTransaction->GetKey();
    pTxn = pTransaction->GetTxn();

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipStackState::ReleaseTransaction(
        IN ::SipTxnKey* pKey, IN IMS_SINT32 nOption, OUT ::SipTxnKey*& pOutKey, OUT SipTxn*& pTxn)
{
    LockGuard objLock(m_piLock);
    SipStackTransaction* pTransaction = RemoveTransaction(pKey, nOption);

    if (pTransaction == IMS_NULL)
    {
        pOutKey = IMS_NULL;
        pTxn = IMS_NULL;

        return IMS_FALSE;
    }

    if (nOption == TXN_REMOVE)
    {
        pOutKey = pTransaction->GetKey();
        pTxn = pTransaction->GetTxn();

        delete pTransaction;
    }

    return IMS_TRUE;
}

PUBLIC
void SipStackState::SetTransactionTimerValues(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile)
{
    const SipConfigV* pSipConfigV =
            DYNAMIC_CAST(const SipConfigV*, SipConfigProxy::GetSipConfigV(nSlotId));

    if ((pProfile == IMS_NULL) && (pSipConfigV != IMS_NULL) &&
            !pSipConfigV->IsTimerValueConfiguredOnRuntime())
    {
        // Do not update the transaction timer values in runtime.
        IMS_TRACE_D("SIP timer values are not configured on runtime", 0, 0, 0);
        return;
    }

    // For transaction layer handling
    // Initialize the retransmission initial timer values, timeout timer values,
    // and wait timer values.
    SipStack::SetTransactionTimerValues(nSlotId, pProfile, pSipConfigV);
}

PUBLIC GLOBAL SipStackState* SipStackState::GetInstance()
{
    static SipStackState* s_pStackState = IMS_NULL;

    if (s_pStackState == IMS_NULL)
    {
        s_pStackState = new SipStackState();
    }

    return s_pStackState;
}

PRIVATE
IMS_BOOL SipStackState::AddTransaction(IN ::SipTxnKey* pKey, IN SipTxn* pTxn)
{
    SipStackTransaction* pTransaction = new SipStackTransaction(pKey, pTxn);

    if (pTransaction == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nKey = AString::GetHashCode(SipStack::TxnKey_GetCallId(pKey));
    IMS_SLONG nKeyIndex = m_objTxnAggregate.GetIndexOfKey(nKey);

    if (nKeyIndex >= 0)
    {
        ImsList<SipStackTransaction*>& objTransactions = m_objTxnAggregate.GetValueAt(nKeyIndex);

        if (!objTransactions.Append(pTransaction))
        {
            delete pTransaction;
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    ImsList<SipStackTransaction*> objTransactions;

    if (!objTransactions.Append(pTransaction))
    {
        delete pTransaction;
        return IMS_FALSE;
    }

    m_objTxnAggregate.Add(nKey, objTransactions);
    return IMS_TRUE;
}

PRIVATE
SipStackTransaction* SipStackState::FindTransaction(IN ::SipTxnKey* pKey)
{
    IMS_UINT32 nKey = AString::GetHashCode(SipStack::TxnKey_GetCallId(pKey));
    IMS_SLONG nKeyIndex = m_objTxnAggregate.GetIndexOfKey(nKey);

    if (nKeyIndex < 0)
    {
        // Transaction not found
        return IMS_NULL;
    }

    ImsList<SipStackTransaction*>& objTransactions = m_objTxnAggregate.GetValueAt(nKeyIndex);

    for (IMS_UINT32 i = 0; i < objTransactions.GetSize(); ++i)
    {
        SipStackTransaction* pTransaction = objTransactions.GetAt(i);

        // Check if the transaction is matched or not
        if (pTransaction->CompareKey(pKey))
        {
            // Matched transaction found.
            return pTransaction;
        }
    }

    return IMS_NULL;
}

PRIVATE
SipStackTransaction* SipStackState::RemoveTransaction(IN ::SipTxnKey* pKey, IN IMS_SINT32 nOption)
{
    IMS_UINT32 nKey = AString::GetHashCode(SipStack::TxnKey_GetCallId(pKey));
    IMS_SLONG nKeyIndex = m_objTxnAggregate.GetIndexOfKey(nKey);

    if (nKeyIndex < 0)
    {
        // Transaction not found
        IMS_TRACE_D("RemoveTransaction(%s): not found", SipStack::TxnKey_GetViaBranch(pKey), 0, 0);
        return IMS_NULL;
    }

    ImsList<SipStackTransaction*>& objTransactions = m_objTxnAggregate.GetValueAt(nKeyIndex);

    for (IMS_UINT32 i = 0; i < objTransactions.GetSize(); ++i)
    {
        SipStackTransaction* pTransaction = objTransactions.GetAt(i);

        // Check if the transaction is matched or not
        if (pTransaction->CompareKey(pKey))
        {
            // Matched transaction found.
            if (nOption == TXN_REMOVE)
            {
                IMS_UINT32 nOldTxnCount = GetTransactionCount();

                objTransactions.RemoveAt(i);

                if (objTransactions.IsEmpty())
                {
                    // No element
                    m_objTxnAggregate.RemoveAt(nKeyIndex);
                }

                SipStack::DisplayTxnKey(pTransaction->GetKey());

                IMS_TRACE_D("RemoveTransaction (%d >> %d)", nOldTxnCount, GetTransactionCount(), 0);
            }

            return pTransaction;
        }
    }

    return IMS_NULL;
}

PRIVATE
IMS_UINT32 SipStackState::GetTransactionCount() const
{
    IMS_UINT32 nCount = 0;

    for (IMS_UINT32 i = 0; i < m_objTxnAggregate.GetSize(); ++i)
    {
        const ImsList<SipStackTransaction*>& objTransactions = m_objTxnAggregate.GetValueAt(i);

        nCount += objTransactions.GetSize();
    }

    return nCount;
}
