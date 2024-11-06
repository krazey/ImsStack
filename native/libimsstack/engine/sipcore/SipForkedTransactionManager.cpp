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
#include "ServiceTrace.h"

#include "ISipHeader.h"
#include "Sip.h"
#include "SipClientTransactionState.h"
#include "SipDebug.h"
#include "SipForkedTransactionManager.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
IMS_BOOL SipForkedTransactionManager::Add(IN SipClientTransactionState* pCtState)
{
    for (IMS_UINT32 i = 0; i < m_objTxnStates.GetSize(); ++i)
    {
        const RcPtr<SipClientTransactionState>& pTmpCtState = m_objTxnStates.GetAt(i);

        if (pTmpCtState.Get() == pCtState)
        {
            // Already exists
            return IMS_TRUE;
        }
    }

    if (!m_objTxnStates.Append(pCtState))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
SipClientTransactionState* SipForkedTransactionManager::Lookup(IN ::SipMessage* pSipMsg) const
{
    if (pSipMsg == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (m_objTxnStates.IsEmpty())
    {
        return IMS_NULL;
    }

    // Not forked case
    if (m_objTxnStates.GetSize() == 1)
    {
        const RcPtr<SipClientTransactionState>& pCtState = m_objTxnStates.GetAt(0);

        return pCtState.Get();
    }

    SipHeaderBase* pSipHdr;
    AString strNewLocalTag;
    AString strNewRemoteTag;
    AString strCallId;

    // Call Id
    pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::CALL_ID);
    SipStack::EncodeHeaderBody(pSipHdr, IMS_FALSE, strCallId);
    SipStack::FreeHeaderEx(pSipHdr);

    if (SipStack::IsRequestMessage(pSipMsg))
    {
        // Get local tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

        strNewLocalTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);

        // Get remote tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);

        strNewRemoteTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);
    }
    else
    {
        // Get local tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::FROM);

        strNewLocalTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);

        // Get remote tag
        pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::TO);

        strNewRemoteTag = SipStack::GetParameter(pSipHdr, Sip::STR_TAG);
        SipStack::FreeHeaderEx(pSipHdr);
    }

    for (IMS_UINT32 i = 0; i < m_objTxnStates.GetSize(); ++i)
    {
        const RcPtr<SipClientTransactionState>& pCtState = m_objTxnStates.GetAt(i);
        SipDialogState* pDState = pCtState->GetDialog()->GetDialogState();

        AString strLocalTag = pDState->GetLocalTag();
        AString strRemoteTag = pDState->GetRemoteTag();

        if (strCallId.Equals(pDState->GetCallId()) && strLocalTag.Equals(strNewLocalTag) &&
                strRemoteTag.Equals(strNewRemoteTag))
        {
            IMS_TRACE_D("ForkedTransactionManager :: Dialog "
                        "(call-id=%s, local-tag=%s, remote-tag=%s)",
                    SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), strLocalTag.GetStr(),
                    strRemoteTag.GetStr());
            return pCtState.Get();
        }
    }

    const RcPtr<SipClientTransactionState>& pCtState =
            m_objTxnStates.GetAt(m_objTxnStates.GetSize() - 1);

    return pCtState.Get();
}

PUBLIC
void SipForkedTransactionManager::Remove(IN const SipClientTransactionState* pCtState)
{
    for (IMS_UINT32 i = 0; i < m_objTxnStates.GetSize(); ++i)
    {
        const RcPtr<SipClientTransactionState>& pTmpCtState = m_objTxnStates.GetAt(i);

        if (pTmpCtState.Get() == pCtState)
        {
            // Found
            m_objTxnStates.RemoveAt(i);
            break;
        }
    }
}
