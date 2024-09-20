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
#include "ServiceTrace.h"

#include "SipAck.h"
#include "SipAckPackage.h"
#include "SipDebug.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

class SipAckPackagePrivate
{
public:
    inline SipAckPackagePrivate() {}

    inline ~SipAckPackagePrivate()
    {
        for (IMS_UINT32 i = 0; i < objAckPackages.GetSize(); ++i)
        {
            SipAckPackage* pPackage = objAckPackages.GetAt(i);

            if (pPackage != IMS_NULL)
            {
                delete pPackage;
            }
        }

        objAckPackages.Clear();
    }

public:
    ImsList<SipAckPackage*> objAckPackages;
};

PRIVATE GLOBAL SipAckPackagePrivate* SipAckPackage::s_pAckPackagePrivate = IMS_NULL;

PRIVATE
SipAckPackage::SipAckPackage(IN const AString& strCallId) :
        m_strCallId(strCallId)
{
}

PUBLIC VIRTUAL SipAckPackage::~SipAckPackage()
{
    for (IMS_UINT32 i = 0; i < m_objAcks.GetSize(); ++i)
    {
        SipAck* pAck = m_objAcks.GetAt(i);

        if (pAck != IMS_NULL)
        {
            delete pAck;
        }
    }

    m_objAcks.Clear();
}

PUBLIC
void SipAckPackage::AddAck(IN SipClientTransactionState* pCtState, IN IMS_SINT32 nAliveInterval)
{
    if (pCtState == IMS_NULL)
    {
        return;
    }

    ::SipTxnKey* pTxnKey = pCtState->GetTxnKey();

    if (pTxnKey == IMS_NULL)
    {
        IMS_TRACE_E(0, "SipAckPackage :: no txn key", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAcks.GetSize(); ++i)
    {
        SipAck* pAck = m_objAcks.GetAt(i);

        if (pAck == IMS_NULL)
        {
            continue;
        }

        if (pAck->IsSameTransaction(pTxnKey))
        {
            // ACK already exists in the ACK package
            IMS_TRACE_D("SipAckPackage :: SipAck already exists (%s)",
                    SipStack::TxnKey_GetViaBranch(pTxnKey), 0, 0);
            return;
        }
    }

    IMS_TRACE_D("SipAckPackage :: SipAck (%s, %d)", SipStack::TxnKey_GetViaBranch(pTxnKey),
            nAliveInterval, 0);

    m_objAcks.Append(new SipAck(pCtState, nAliveInterval));
}

PUBLIC
IMS_BOOL SipAckPackage::NotifyStray2xx(IN ::SipTxnKey* pTxnKey)
{
    for (IMS_UINT32 i = 0; i < m_objAcks.GetSize(); ++i)
    {
        SipAck* pAck = m_objAcks.GetAt(i);

        if (pAck == IMS_NULL)
        {
            continue;
        }

        if (pAck->IsStrayAck())
        {
            continue;
        }

        if (pAck->IsSameTransaction(pTxnKey))
        {
            pAck->RetransmitMessage();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL void SipAckPackage::Destroy()
{
    if (s_pAckPackagePrivate == IMS_NULL)
    {
        IMS_TRACE_D("SipAckPackage (%s) is destroyed",
                SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'), 0, 0);
        delete this;
        return;
    }

    for (IMS_UINT32 i = 0; i < s_pAckPackagePrivate->objAckPackages.GetSize(); ++i)
    {
        SipAckPackage* pPackage = s_pAckPackagePrivate->objAckPackages.GetAt(i);

        if (pPackage == IMS_NULL)
        {
            continue;
        }

        if (pPackage->IsSamePackage(m_strCallId))
        {
            IMS_TRACE_D("SipAckPackage (%s) is destroyed",
                    SipDebug::GetCharA1(m_strCallId.GetStr(), 8, '@'), 0, 0);

            s_pAckPackagePrivate->objAckPackages.RemoveAt(i);
            delete this;
            break;
        }
    }

    IMS_TRACE_D("SipAckPackage :: size=%d", s_pAckPackagePrivate->objAckPackages.GetSize(), 0, 0);
}

PRIVATE VIRTUAL void SipAckPackage::RemoveStrayAcks()
{
    IMS_UINT32 nTotalCount = m_objAcks.GetSize();

    for (IMS_UINT32 i = 0; i < m_objAcks.GetSize();)
    {
        SipAck* pAck = m_objAcks.GetAt(i);

        if (pAck == IMS_NULL)
        {
            m_objAcks.RemoveAt(i);
            continue;
        }

        if (pAck->IsStrayAck())
        {
            m_objAcks.RemoveAt(i);
            delete pAck;
        }
        else
        {
            ++i;
        }
    }

    if (m_objAcks.GetSize() < nTotalCount)
    {
        IMS_TRACE_D("SipAckPackage :: ACK (%d >> %d)", nTotalCount, m_objAcks.GetSize(), 0);
    }
}

PUBLIC GLOBAL SipAckPackage* SipAckPackage::CreateAckPackage(IN const AString& strCallId)
{
    for (IMS_UINT32 i = 0; i < s_pAckPackagePrivate->objAckPackages.GetSize(); ++i)
    {
        SipAckPackage* pPackage = s_pAckPackagePrivate->objAckPackages.GetAt(i);

        if (pPackage == IMS_NULL)
        {
            continue;
        }

        if (pPackage->IsSamePackage(strCallId))
        {
            IMS_TRACE_D("SipAckPackage :: RE-USE (%s)",
                    SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);
            return pPackage;
        }
    }

    SipAckPackage* pNewPackage = new SipAckPackage(strCallId);

    s_pAckPackagePrivate->objAckPackages.Append(pNewPackage);

    IMS_TRACE_D(
            "SipAckPackage (%s) is created", SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);

    return pNewPackage;
}

PUBLIC GLOBAL void SipAckPackage::Init()
{
    if (s_pAckPackagePrivate == IMS_NULL)
    {
        s_pAckPackagePrivate = new SipAckPackagePrivate();
    }
}

PUBLIC GLOBAL IMS_BOOL SipAckPackage::HandleStray2xx(IN ::SipMessage* pSipMsg)
{
    if (s_pAckPackagePrivate == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ::SipTxnKey* pTxnKey = SipStack::CreateTxnKey(pSipMsg, SipStack::SIP_TXN_MSG_RECEIVED);

    if (pTxnKey == IMS_NULL)
    {
        IMS_TRACE_D("SipAckPackage :: Stray 2xx is discarded", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strCallId(SipStack::TxnKey_GetCallId(pTxnKey));
    IMS_BOOL bStray2xxHandled = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < s_pAckPackagePrivate->objAckPackages.GetSize(); ++i)
    {
        SipAckPackage* pPackage = s_pAckPackagePrivate->objAckPackages.GetAt(i);

        if (pPackage == IMS_NULL)
        {
            continue;
        }

        if (pPackage->IsSamePackage(strCallId))
        {
            bStray2xxHandled = pPackage->NotifyStray2xx(pTxnKey);

            if (!bStray2xxHandled)
            {
                IMS_TRACE_D("SipAckPackage :: ACK (%s) is not handled",
                        SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);
            }
            break;
        }
    }

    SipStack::FreeTxnKey(pTxnKey);

    return bStray2xxHandled;
}
