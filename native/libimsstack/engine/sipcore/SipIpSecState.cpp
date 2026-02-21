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

#include "ISipIpSecStateListener.h"
#include "SipIpSecState.h"
#include "SipStack.h"
#include "SipTransportAddress.h"

__IMS_TRACE_TAG_SIP_CORE__;

PRIVATE
SipIpSecState::SecurityAssociation::SecurityAssociation() :
        objIpU(IpAddress::NONE),
        nPortUc(0),
        nPortUs(0),
        objIpP(IpAddress::NONE),
        nPortPc(0),
        nPortPs(0),
        nState(STATE_INACTIVE)
{
}

PRIVATE
SipIpSecState::SecurityAssociation::SecurityAssociation(IN const IpAddress& objIpU_,
        IN IMS_SINT32 nPortUc_, IN IMS_SINT32 nPortUs_, IN const IpAddress& objIpP_,
        IN IMS_SINT32 nPortPc_, IN IMS_SINT32 nPortPs_) :
        objIpU(objIpU_),
        nPortUc(nPortUc_),
        nPortUs(nPortUs_),
        objIpP(objIpP_),
        nPortPc(nPortPc_),
        nPortPs(nPortPs_),
        nState(STATE_INACTIVE)
{
}

PRIVATE
SipIpSecState::SecurityAssociation::SecurityAssociation(
        IN const SipIpSecState::SecurityAssociation& other) :
        objIpU(other.objIpU),
        nPortUc(other.nPortUc),
        nPortUs(other.nPortUs),
        objIpP(other.objIpP),
        nPortPc(other.nPortPc),
        nPortPs(other.nPortPs),
        nState(other.nState),
        objSipTxnKeys(other.objSipTxnKeys)
{
}

PRIVATE
SipIpSecState::SecurityAssociation::~SecurityAssociation()
{
    IMS_TRACE_D("SA: Destructor - txnkeys=%d", objSipTxnKeys.GetSize(), 0, 0);
}

PUBLIC
IMS_BOOL SipIpSecState::SecurityAssociation::AddTransaction(IN const sipcore::SipTxnKey* pTxnKey)
{
    for (IMS_UINT32 i = 0; i < objSipTxnKeys.GetSize(); ++i)
    {
        const sipcore::SipTxnKey& objTxnKey = objSipTxnKeys.GetAt(i);

        if (objTxnKey.Equals(pTxnKey))
        {
            IMS_TRACE_D("SA: Txn(%d:%s) already exists", objTxnKey.GetCSeq(),
                    objTxnKey.GetViaBranch().GetStr(), 0);
            return IMS_FALSE;
        }
    }

    objSipTxnKeys.Append(*pTxnKey);

    IMS_TRACE_D("SA: Txn(%d:%s) is added - size=%d", pTxnKey->GetCSeq(),
            pTxnKey->GetViaBranch().GetStr(), objSipTxnKeys.GetSize());

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipIpSecState::SecurityAssociation::CheckIpAddress(
        IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd) const
{
    if (!objIpP.Equals(objFarEnd.GetIpAddress()) || !objIpU.Equals(objNearEnd.GetIpAddress()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 SipIpSecState::SecurityAssociation::GetSa(IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd, IN IMS_SINT32 nDirection) const
{
    if (nDirection == DIRECTION_IN)
    {
        // SA_PPC_PUS_U_IN
        // SA_PPC_PUS_T_IN
        // SA_PPS_PUC_T_IN
        if (objNearEnd.GetProtocol() == SipTransportAddress::PROTOCOL_UDP)
        {
            if ((nPortPc == objFarEnd.GetPort()) && (nPortUs == objNearEnd.GetPort()))
            {
                return SA_PPC_PUS_U_IN;
            }
        }
        else
        {
            if ((nPortPc == objFarEnd.GetPort()) && (nPortUs == objNearEnd.GetPort()))
            {
                return SA_PPC_PUS_T_IN;
            }
            else if ((nPortPs == objFarEnd.GetPort()) && (nPortUc == objNearEnd.GetPort()))
            {
                return SA_PPS_PUC_T_IN;
            }
        }
    }
    else if (nDirection == DIRECTION_OUT)
    {
        // SA_PUC_PPS_U_OUT
        // SA_PUC_PPS_T_OUT
        // SA_PUS_PPC_T_OUT
        if (objNearEnd.GetProtocol() == SipTransportAddress::PROTOCOL_UDP)
        {
            if ((nPortUc == objNearEnd.GetPort()) && (nPortPs == objFarEnd.GetPort()))
            {
                return SA_PUC_PPS_U_OUT;
            }
        }
        else
        {
            if ((nPortUc == objNearEnd.GetPort()) && (nPortPs == objFarEnd.GetPort()))
            {
                return SA_PUC_PPS_T_OUT;
            }
            else if ((nPortUs == objNearEnd.GetPort()) && (nPortPc == objFarEnd.GetPort()))
            {
                return SA_PUS_PPC_T_OUT;
            }
        }
    }

    return SA_END;
}

PUBLIC
IMS_BOOL SipIpSecState::SecurityAssociation::RemoveTransaction(IN const sipcore::SipTxnKey* pTxnKey)
{
    IMS_BOOL bRemoved = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objSipTxnKeys.GetSize(); ++i)
    {
        const sipcore::SipTxnKey& objTxnKey = objSipTxnKeys.GetAt(i);

        if (objTxnKey.Equals(pTxnKey))
        {
            objSipTxnKeys.RemoveAt(i);
            bRemoved = IMS_TRUE;
            break;
        }
    }

    if (bRemoved)
    {
        IMS_TRACE_D("SA: Txn(%d:%s) is removed - size=%d", pTxnKey->GetCSeq(),
                pTxnKey->GetViaBranch().GetStr(), objSipTxnKeys.GetSize());
    }

    return bRemoved;
}

PUBLIC
void SipIpSecState::SecurityAssociation::SetState(IN IMS_SINT32 nState)
{
    if (this->nState != nState)
    {
        IMS_TRACE_I("SA: %s >> %s", StateToString(this->nState), StateToString(nState), 0);

        this->nState = nState;
    }
}

PRIVATE GLOBAL const IMS_CHAR* SipIpSecState::SecurityAssociation::StateToString(
        IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case SipIpSecState::STATE_INACTIVE:
            return "STATE_INACTIVE";
        case SipIpSecState::STATE_CREATED:
            return "STATE_CREATED";
        case SipIpSecState::STATE_PENDING:
            return "STATE_PENDING";
        case SipIpSecState::STATE_ACTIVE:
            return "STATE_ACTIVE";
        case SipIpSecState::STATE_TERMINATED:
            return "STATE_TERMINATED";
        case SipIpSecState::STATE_TERMINATED_PENDING:
            return "STATE_TERMINATED_PENDING";
        default:
            return "__INVALID_STATE__";
    }
}

PUBLIC
SipIpSecState::SipIpSecState() :
        EngineActivity(),
        m_pNewSa(IMS_NULL),
        m_pOldSa(IMS_NULL),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SipIpSecState::~SipIpSecState()
{
    if (m_pOldSa != IMS_NULL)
    {
        delete m_pOldSa;
        m_pOldSa = IMS_NULL;
    }

    if (m_pNewSa != IMS_NULL)
    {
        delete m_pNewSa;
        m_pNewSa = IMS_NULL;
    }
}

PUBLIC
void SipIpSecState::NotifyMessageReceived(IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd, IN ::SipMessage* pSipMsg)
{
    sipcore::SipTxnKey* pTxnKey = SipStack::CreateTxnKey(pSipMsg);

    if (pTxnKey != IMS_NULL)
    {
        NotifyMessageReceivedInternal(objNearEnd, objFarEnd, pTxnKey);

        delete pTxnKey;
    }
}

PUBLIC
void SipIpSecState::NotifyMessageSent(IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd, IN ::SipMessage* pSipMsg)
{
    sipcore::SipTxnKey* pTxnKey = SipStack::CreateTxnKey(pSipMsg);

    if (pTxnKey != IMS_NULL)
    {
        NotifyMessageSentInternal(objNearEnd, objFarEnd, pTxnKey);

        delete pTxnKey;
    }
}

PUBLIC
void SipIpSecState::NotifyMessageSentFailed(IN ::SipMessage* pSipMsg)
{
    sipcore::SipTxnKey* pTxnKey = SipStack::CreateTxnKey(pSipMsg);

    if (pTxnKey != IMS_NULL)
    {
        NotifyTransactionAbortedInternal(pTxnKey);

        delete pTxnKey;
    }
}

PUBLIC
void SipIpSecState::NotifyTransactionAborted(IN const ::SipTxnKey* pSipTxnKey)
{
    sipcore::SipTxnKey* pTxnKey = SipStack::CreateTxnKeyFromKey(pSipTxnKey);

    if (pTxnKey != IMS_NULL)
    {
        NotifyTransactionAbortedInternal(pTxnKey);

        delete pTxnKey;
    }
}

PRIVATE VIRTUAL IMS_BOOL SipIpSecState::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_NOTIFY_STATE_CHANGED:
            if (m_piListener == IMS_NULL)
            {
                IMS_TRACE_D("Listener is null", 0, 0, 0);
                return IMS_FALSE;
            }

            m_piListener->IpSecState_StateChanged(LONG_TO_SINT(objMsg.nWparam));
            return IMS_TRUE;

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL void SipIpSecState::ClearIpSecSa(IN IMS_SINT32 nSaType)
{
    if ((nSaType == SA_NEW) && (m_pNewSa != IMS_NULL))
    {
        delete m_pNewSa;
        m_pNewSa = IMS_NULL;
    }
    else if ((nSaType == SA_OLD) && (m_pOldSa != IMS_NULL))
    {
        delete m_pOldSa;
        m_pOldSa = IMS_NULL;
    }
}

PRIVATE VIRTUAL IMS_SINT32 SipIpSecState::GetState(IN IMS_SINT32 nSaType) const
{
    if ((nSaType == SA_NEW) && (m_pNewSa != IMS_NULL))
    {
        return m_pNewSa->nState;
    }
    else if ((nSaType == SA_OLD) && (m_pOldSa != IMS_NULL))
    {
        return m_pOldSa->nState;
    }

    return STATE_INACTIVE;
}

PRIVATE VIRTUAL IMS_BOOL SipIpSecState::HasPendingTransaction(IN IMS_SINT32 nSaType) const
{
    if ((nSaType == SA_NEW) && (m_pNewSa != IMS_NULL))
    {
        return m_pNewSa->HasPendingTransaction();
    }
    else if ((nSaType == SA_OLD) && (m_pOldSa != IMS_NULL))
    {
        return m_pOldSa->HasPendingTransaction();
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL void SipIpSecState::SetIpSecSa(IN IMS_SINT32 nSaType, IN const IpAddress& objIpU,
        IN IMS_SINT32 nPortUc, IN IMS_SINT32 nPortUs, IN const IpAddress& objIpP,
        IN IMS_SINT32 nPortPc, IN IMS_SINT32 nPortPs)
{
    if (nSaType == SA_NEW)
    {
        if (m_pNewSa != IMS_NULL)
        {
            if (m_pOldSa != IMS_NULL)
            {
                delete m_pOldSa;
            }

            m_pOldSa = new SecurityAssociation(*m_pNewSa);

            if (!m_pOldSa->HasPendingTransaction())
            {
                m_pOldSa->SetState(STATE_TERMINATED);
            }

            delete m_pNewSa;
        }

        m_pNewSa = new SecurityAssociation(objIpU, nPortUc, nPortUs, objIpP, nPortPc, nPortPs);

        m_pNewSa->SetState(STATE_CREATED);
    }
    else if (nSaType == SA_OLD)
    {
        if (m_pOldSa != IMS_NULL)
        {
            delete m_pOldSa;
        }

        m_pOldSa = new SecurityAssociation(objIpU, nPortUc, nPortUs, objIpP, nPortPc, nPortPs);

        m_pOldSa->SetState(STATE_TERMINATED);
    }
}

PRIVATE
void SipIpSecState::NotifyMessageReceivedInternal(IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd, IN sipcore::SipTxnKey* pTxnKey)
{
    IMS_BOOL bStrayResponseOnNewSa = IMS_FALSE;
    IMS_BOOL bStrayResponseOnOldSa = IMS_FALSE;

    // Check a new SA.
    if ((m_pNewSa != IMS_NULL) && m_pNewSa->CheckIpAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSaPair =
                m_pNewSa->GetSa(objNearEnd, objFarEnd, SecurityAssociation::DIRECTION_IN);

        if (nSaPair != SecurityAssociation::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSaPair);

                // Incoming SIP request
                m_pNewSa->AddTransaction(pTxnKey);

                if (m_pNewSa->GetState() != STATE_ACTIVE)
                {
                    m_pNewSa->SetState(STATE_ACTIVE);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Incoming SIP response
                if (!m_pNewSa->RemoveTransaction(pTxnKey))
                {
                    bStrayResponseOnNewSa = IMS_TRUE;
                }

                if (m_pNewSa->GetState() != STATE_ACTIVE)
                {
                    m_pNewSa->SetState(STATE_ACTIVE);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
                }
            }
        }
    }

    // Checks an old SA.
    if ((m_pOldSa != IMS_NULL) && m_pOldSa->CheckIpAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSaPair =
                m_pOldSa->GetSa(objNearEnd, objFarEnd, SecurityAssociation::DIRECTION_IN);

        if (nSaPair != SecurityAssociation::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSaPair);

                // Incoming SIP request
                m_pOldSa->AddTransaction(pTxnKey);

                if (m_pOldSa->GetState() == STATE_TERMINATED)
                {
                    m_pOldSa->SetState(STATE_TERMINATED_PENDING);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Incoming SIP response
                if (m_pOldSa->RemoveTransaction(pTxnKey))
                {
                    if (!m_pOldSa->HasPendingTransaction() &&
                            (m_pOldSa->GetState() == STATE_TERMINATED_PENDING))
                    {
                        m_pOldSa->SetState(STATE_TERMINATED);
                        PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                    }
                }
                else
                {
                    bStrayResponseOnOldSa = IMS_TRUE;
                }
            }
        }

        if ((m_pNewSa != IMS_NULL) && (m_pNewSa->GetState() == STATE_ACTIVE) &&
                (m_pOldSa->GetState() != STATE_TERMINATED) && !m_pOldSa->HasPendingTransaction())
        {
            m_pOldSa->SetState(STATE_TERMINATED);
            PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
        }
    }

    if (bStrayResponseOnNewSa && (m_pOldSa != IMS_NULL))
    {
        IMS_TRACE_D("IpSecState: Stray response is detected on new SA", 0, 0, 0);

        if (m_pOldSa->RemoveTransaction(pTxnKey))
        {
            if (!m_pOldSa->HasPendingTransaction())
            {
                if ((m_pOldSa->GetState() == STATE_TERMINATED_PENDING) ||
                        ((m_pNewSa != IMS_NULL) && (m_pNewSa->GetState() == STATE_ACTIVE) &&
                                (m_pOldSa->GetState() != STATE_TERMINATED)))
                {
                    m_pOldSa->SetState(STATE_TERMINATED);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
        }
    }

    if (bStrayResponseOnOldSa && (m_pNewSa != IMS_NULL))
    {
        IMS_TRACE_D("IpSecState: Stray response is detected on old SA", 0, 0, 0);

        m_pNewSa->RemoveTransaction(pTxnKey);
    }
}

PRIVATE
void SipIpSecState::NotifyMessageSentInternal(IN const SipTransportAddress& objNearEnd,
        IN const SipTransportAddress& objFarEnd, IN sipcore::SipTxnKey* pTxnKey)
{
    IMS_BOOL bStrayResponseOnNewSa = IMS_FALSE;
    IMS_BOOL bStrayResponseOnOldSa = IMS_FALSE;

    // Check a new SA.
    if ((m_pNewSa != IMS_NULL) && m_pNewSa->CheckIpAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSaPair =
                m_pNewSa->GetSa(objNearEnd, objFarEnd, SecurityAssociation::DIRECTION_IN);

        if (nSaPair != SecurityAssociation::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSaPair);

                // Outgoing SIP request
                m_pNewSa->AddTransaction(pTxnKey);

                if (m_pNewSa->GetState() == STATE_CREATED)
                {
                    m_pNewSa->SetState(STATE_PENDING);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Outgoing SIP response
                if (!m_pNewSa->RemoveTransaction(pTxnKey))
                {
                    bStrayResponseOnNewSa = IMS_TRUE;
                }
            }
        }
    }

    // Checks an old SA.
    if ((m_pOldSa != IMS_NULL) && m_pOldSa->CheckIpAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSaPair =
                m_pOldSa->GetSa(objNearEnd, objFarEnd, SecurityAssociation::DIRECTION_IN);

        if (nSaPair != SecurityAssociation::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSaPair);

                // Outgoing SIP request
                m_pOldSa->AddTransaction(pTxnKey);

                if (m_pOldSa->GetState() == STATE_TERMINATED)
                {
                    m_pOldSa->SetState(STATE_TERMINATED_PENDING);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Outgoing SIP response
                if (m_pOldSa->RemoveTransaction(pTxnKey))
                {
                    if (!m_pOldSa->HasPendingTransaction() &&
                            (m_pOldSa->GetState() == STATE_TERMINATED_PENDING))
                    {
                        m_pOldSa->SetState(STATE_TERMINATED);
                        PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                    }
                }
                else
                {
                    bStrayResponseOnOldSa = IMS_TRUE;
                }
            }
        }

        if ((m_pNewSa != IMS_NULL) && (m_pNewSa->GetState() == STATE_ACTIVE) &&
                (m_pOldSa->GetState() != STATE_TERMINATED) && !m_pOldSa->HasPendingTransaction())
        {
            m_pOldSa->SetState(STATE_TERMINATED);
            PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
        }
    }

    if (bStrayResponseOnNewSa && (m_pOldSa != IMS_NULL))
    {
        IMS_TRACE_D("IpSecState: Stray response is detected on new SA", 0, 0, 0);

        if (m_pOldSa->RemoveTransaction(pTxnKey))
        {
            if (!m_pOldSa->HasPendingTransaction())
            {
                if ((m_pOldSa->GetState() == STATE_TERMINATED_PENDING) ||
                        ((m_pNewSa != IMS_NULL) && (m_pNewSa->GetState() == STATE_ACTIVE) &&
                                (m_pOldSa->GetState() != STATE_TERMINATED)))
                {
                    m_pOldSa->SetState(STATE_TERMINATED);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
        }
    }

    if (bStrayResponseOnOldSa && (m_pNewSa != IMS_NULL))
    {
        IMS_TRACE_D("IpSecState: Stray response is detected on old SA", 0, 0, 0);

        m_pNewSa->RemoveTransaction(pTxnKey);
    }
}

PRIVATE
void SipIpSecState::NotifyTransactionAbortedInternal(IN const sipcore::SipTxnKey* pTxnKey)
{
    if (m_pNewSa != IMS_NULL)
    {
        if (m_pNewSa->RemoveTransaction(pTxnKey))
        {
            if (!m_pNewSa->HasPendingTransaction() && (m_pNewSa->GetState() == STATE_PENDING))
            {
                m_pNewSa->SetState(STATE_CREATED);
                PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
            }

            return;
        }
    }

    if (m_pOldSa != IMS_NULL)
    {
        if (m_pOldSa->RemoveTransaction(pTxnKey))
        {
            if (!m_pOldSa->HasPendingTransaction() &&
                    (m_pOldSa->GetState() == STATE_TERMINATED_PENDING))
            {
                m_pOldSa->SetState(STATE_TERMINATED);
                PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
            }

            return;
        }
    }
}
