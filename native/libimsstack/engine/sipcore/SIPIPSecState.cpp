/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140304  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "ISipIpSecStateListener.h"
#include "SIPIPSecState.h"

__IMS_TRACE_TAG_SIP__;

PRIVATE
SIPIPSecState::SA::SA() :
        objIP_U(IPAddress::NONE),
        nPort_UC(0),
        nPort_US(0),
        objIP_P(IPAddress::NONE),
        nPort_PC(0),
        nPort_PS(0),
        nState(STATE_INACTIVE)
{
}

PRIVATE
SIPIPSecState::SA::SA(IN CONST IPAddress& objIP_U_, IN IMS_SINT32 nPort_UC_,
        IN IMS_SINT32 nPort_US_, IN CONST IPAddress& objIP_P_, IN IMS_SINT32 nPort_PC_,
        IN IMS_SINT32 nPort_PS_) :
        objIP_U(objIP_U_),
        nPort_UC(nPort_UC_),
        nPort_US(nPort_US_),
        objIP_P(objIP_P_),
        nPort_PC(nPort_PC_),
        nPort_PS(nPort_PS_),
        nState(STATE_INACTIVE)
{
}

PRIVATE
SIPIPSecState::SA::SA(IN CONST SIPIPSecState::SA& objRHS) :
        objIP_U(objRHS.objIP_U),
        nPort_UC(objRHS.nPort_UC),
        nPort_US(objRHS.nPort_US),
        objIP_P(objRHS.objIP_P),
        nPort_PC(objRHS.nPort_PC),
        nPort_PS(objRHS.nPort_PS),
        nState(objRHS.nState),
        objSAStat(objRHS.objSAStat)
{
}

PRIVATE
SIPIPSecState::SA::~SA()
{
    IMS_TRACE_D("SA :: Destructor - txnkeys=%d", objSAStat.GetSize(), 0, 0);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPIPSecState::SA::AddTransaction(IN CONST SIPTxnKey* pTxnKey)
{
    for (IMS_UINT32 i = 0; i < objSAStat.GetSize(); ++i)
    {
        const SIPTxnKey& objTxnKey = objSAStat.GetAt(i);

        if (objTxnKey.Equals(pTxnKey))
        {
            IMS_TRACE_D("SA :: Txn (%d:%s) already exists", objTxnKey.GetCSeq(),
                    objTxnKey.GetViaBranch().GetStr(), 0);
            return IMS_FALSE;
        }
    }

    objSAStat.Append(*pTxnKey);

    IMS_TRACE_D("SA :: Txn (%d:%s) is added - size=%d", pTxnKey->GetCSeq(),
            pTxnKey->GetViaBranch().GetStr(), objSAStat.GetSize());

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPIPSecState::SA::CheckIPAddress(
        IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd) const
{
    if (!objIP_P.Equals(objFarEnd.GetIPAddress()) || !objIP_U.Equals(objNearEnd.GetIPAddress()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPIPSecState::SA::GetSA(IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd, IN IMS_SINT32 nDirection) const
{
    if (nDirection == SA_IN)
    {
        // SA_PPC_PUS_U_IN
        // SA_PPC_PUS_T_IN
        // SA_PPS_PUC_T_IN
        if (objNearEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_UDP)
        {
            if ((nPort_PC == objFarEnd.GetPort()) && (nPort_US == objNearEnd.GetPort()))
            {
                return SA_PPC_PUS_U_IN;
            }
        }
        else
        {
            if ((nPort_PC == objFarEnd.GetPort()) && (nPort_US == objNearEnd.GetPort()))
            {
                return SA_PPC_PUS_T_IN;
            }
            else if ((nPort_PS == objFarEnd.GetPort()) && (nPort_UC == objNearEnd.GetPort()))
            {
                return SA_PPS_PUC_T_IN;
            }
        }
    }
    else if (nDirection == SA_OUT)
    {
        // SA_PUC_PPS_U_OUT
        // SA_PUC_PPS_T_OUT
        // SA_PUS_PPC_T_OUT
        if (objNearEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_UDP)
        {
            if ((nPort_UC == objNearEnd.GetPort()) && (nPort_PS == objFarEnd.GetPort()))
            {
                return SA_PUC_PPS_U_OUT;
            }
        }
        else
        {
            if ((nPort_UC == objNearEnd.GetPort()) && (nPort_PS == objFarEnd.GetPort()))
            {
                return SA_PUC_PPS_T_OUT;
            }
            else if ((nPort_US == objNearEnd.GetPort()) && (nPort_PC == objFarEnd.GetPort()))
            {
                return SA_PUS_PPC_T_OUT;
            }
        }
    }

    return SA_END;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPIPSecState::SA::GetState() const
{
    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPIPSecState::SA::HasPendingTransaction() const
{
    return !objSAStat.IsEmpty();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPIPSecState::SA::RemoveTransaction(IN CONST SIPTxnKey* pTxnKey)
{
    IMS_BOOL bRemoved = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objSAStat.GetSize(); ++i)
    {
        const SIPTxnKey& objTxnKey = objSAStat.GetAt(i);

        if (objTxnKey.Equals(pTxnKey))
        {
            objSAStat.RemoveAt(i);
            bRemoved = IMS_TRUE;
            break;
        }
    }

    if (bRemoved)
    {
        IMS_TRACE_D("SA :: Txn (%d:%s) is removed - size=%d", pTxnKey->GetCSeq(),
                pTxnKey->GetViaBranch().GetStr(), objSAStat.GetSize());
    }

    return bRemoved;
}

/*

Remarks

*/
PUBLIC
void SIPIPSecState::SA::SetState(IN IMS_SINT32 nState)
{
    if (this->nState != nState)
    {
        IMS_TRACE_I("SA :: %s >> %s", StateToString(this->nState), StateToString(nState), 0);

        this->nState = nState;
    }
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* SIPIPSecState::SA::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case SIPIPSecState::STATE_INACTIVE:
            return "STATE_INACTIVE";
        case SIPIPSecState::STATE_CREATED:
            return "STATE_CREATED";
        case SIPIPSecState::STATE_PENDING:
            return "STATE_PENDING";
        case SIPIPSecState::STATE_ACTIVE:
            return "STATE_ACTIVE";
        case SIPIPSecState::STATE_TERMINATED:
            return "STATE_TERMINATED";
        case SIPIPSecState::STATE_TERMINATED_PENDING:
            return "STATE_TERMINATED_PENDING";
        default:
            return "__INVALID_STATE__";
    }
}

PUBLIC
SIPIPSecState::SIPIPSecState() :
        EngineActivity(),
        pNewSA(IMS_NULL),
        pOldSA(IMS_NULL),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SIPIPSecState::~SIPIPSecState() {}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPIPSecState::IsIPSecEnabled() const
{
    return (pNewSA != IMS_NULL) || (pOldSA != IMS_NULL);
}

/*

Remarks

*/
PUBLIC
void SIPIPSecState::NotifyMessageReceived(IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd, IN SipMessage* pstMessage)
{
    SIPTxnKey* pTxnKey = SIPStack::CreateTxnKey(pstMessage);

    if (pTxnKey != IMS_NULL)
    {
        NotifyMessageReceivedInternal(objNearEnd, objFarEnd, pTxnKey);

        delete pTxnKey;
    }
}

/*

Remarks

*/
PUBLIC
void SIPIPSecState::NotifyMessageSent(IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd, IN SipMessage* pstMessage)
{
    SIPTxnKey* pTxnKey = SIPStack::CreateTxnKey(pstMessage);

    if (pTxnKey != IMS_NULL)
    {
        NotifyMessageSentInternal(objNearEnd, objFarEnd, pTxnKey);

        delete pTxnKey;
    }
}

/*

Remarks

*/
PUBLIC
void SIPIPSecState::NotifyMessageSentFailed(IN SipMessage* pstMessage)
{
    SIPTxnKey* pTxnKey = SIPStack::CreateTxnKey(pstMessage);

    if (pTxnKey != IMS_NULL)
    {
        NotifyTransactionAbortedInternal(pTxnKey);

        delete pTxnKey;
    }
}

/*

Remarks

*/
PUBLIC
void SIPIPSecState::NotifyTransactionAborted(IN SipTxnKey* pstTxnKey)
{
    SIPTxnKey* pTxnKey = SIPStack::CreateTxnKeyFromKey(pstTxnKey);

    if (pTxnKey != IMS_NULL)
    {
        NotifyTransactionAbortedInternal(pTxnKey);

        delete pTxnKey;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL SIPIPSecState::DispatchMessage(IN IMSMSG& objMSG)
{
    switch (objMSG.GetName())
    {
        case AMSG_NOTIFY_STATE_CHANGED:
            if (piListener == IMS_NULL)
            {
                IMS_TRACE_D("Listener is null", 0, 0, 0);
                return IMS_FALSE;
            }

            piListener->IpSecState_StateChanged(LONG_TO_SINT(objMSG.nWparam));
            return IMS_TRUE;

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPIPSecState::ClearIpSecSa(IN IMS_SINT32 nSAType)
{
    if ((nSAType == SA_NEW) && (pNewSA != IMS_NULL))
    {
        delete pNewSA;
        pNewSA = IMS_NULL;
    }
    else if ((nSAType == SA_OLD) && (pOldSA != IMS_NULL))
    {
        delete pOldSA;
        pOldSA = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPIPSecState::GetState(IN IMS_SINT32 nSAType) const
{
    if ((nSAType == SA_NEW) && (pNewSA != IMS_NULL))
    {
        return pNewSA->nState;
    }
    else if ((nSAType == SA_OLD) && (pOldSA != IMS_NULL))
    {
        return pOldSA->nState;
    }

    return STATE_INACTIVE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL SIPIPSecState::HasPendingTransaction(IN IMS_SINT32 nSAType) const
{
    if ((nSAType == SA_NEW) && (pNewSA != IMS_NULL))
    {
        return pNewSA->HasPendingTransaction();
    }
    else if ((nSAType == SA_OLD) && (pOldSA != IMS_NULL))
    {
        return pOldSA->HasPendingTransaction();
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPIPSecState::SetIpSecSa(IN IMS_SINT32 nSAType, IN CONST IPAddress& objIP_U,
        IN IMS_SINT32 nPort_UC, IN IMS_SINT32 nPort_US, IN CONST IPAddress& objIP_P,
        IN IMS_SINT32 nPort_PC, IN IMS_SINT32 nPort_PS)
{
    if (nSAType == SA_NEW)
    {
        if (pNewSA != IMS_NULL)
        {
            if (pOldSA != IMS_NULL)
            {
                delete pOldSA;
            }

            pOldSA = new SA(*pNewSA);

            if (!pOldSA->HasPendingTransaction())
            {
                pOldSA->SetState(STATE_TERMINATED);
            }

            delete pNewSA;
        }

        pNewSA = new SA(objIP_U, nPort_UC, nPort_US, objIP_P, nPort_PC, nPort_PS);

        pNewSA->SetState(STATE_CREATED);
    }
    else if (nSAType == SA_OLD)
    {
        if (pOldSA != IMS_NULL)
        {
            delete pOldSA;
        }

        pOldSA = new SA(objIP_U, nPort_UC, nPort_US, objIP_P, nPort_PC, nPort_PS);

        pOldSA->SetState(STATE_TERMINATED);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPIPSecState::SetListener(IN ISipIpSecStateListener* piListener)
{
    this->piListener = piListener;
}

/*

Remarks

*/
PRIVATE
void SIPIPSecState::NotifyMessageReceivedInternal(IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd, IN SIPTxnKey* pTxnKey)
{
    IMS_BOOL bStrayResponseOnNewSA = IMS_FALSE;
    IMS_BOOL bStrayResponseOnOldSA = IMS_FALSE;

    //// Check a new SA...
    if ((pNewSA != IMS_NULL) && pNewSA->CheckIPAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSAPair = pNewSA->GetSA(objNearEnd, objFarEnd, SA::SA_IN);

        if (nSAPair != SA::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSAPair);

                // Incoming SIP request
                pNewSA->AddTransaction(pTxnKey);

                if (pNewSA->GetState() != STATE_ACTIVE)
                {
                    pNewSA->SetState(STATE_ACTIVE);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Incoming SIP response
                if (!pNewSA->RemoveTransaction(pTxnKey))
                {
                    bStrayResponseOnNewSA = IMS_TRUE;
                }

                if (pNewSA->GetState() != STATE_ACTIVE)
                {
                    pNewSA->SetState(STATE_ACTIVE);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
                }
            }
        }
    }

    // Checks an old SA...
    if ((pOldSA != IMS_NULL) && pOldSA->CheckIPAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSAPair = pOldSA->GetSA(objNearEnd, objFarEnd, SA::SA_IN);

        if (nSAPair != SA::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSAPair);

                // Incoming SIP request
                pOldSA->AddTransaction(pTxnKey);

                if (pOldSA->GetState() == STATE_TERMINATED)
                {
                    pOldSA->SetState(STATE_TERMINATED_PENDING);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Incoming SIP response
                if (pOldSA->RemoveTransaction(pTxnKey))
                {
                    if (!pOldSA->HasPendingTransaction() &&
                            (pOldSA->GetState() == STATE_TERMINATED_PENDING))
                    {
                        pOldSA->SetState(STATE_TERMINATED);
                        PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                    }
                }
                else
                {
                    bStrayResponseOnOldSA = IMS_TRUE;
                }
            }
        }

        if ((pNewSA != IMS_NULL) && (pNewSA->GetState() == STATE_ACTIVE) &&
                (pOldSA->GetState() != STATE_TERMINATED) && !pOldSA->HasPendingTransaction())
        {
            pOldSA->SetState(STATE_TERMINATED);
            PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
        }
    }

    if (bStrayResponseOnNewSA && (pOldSA != IMS_NULL))
    {
        IMS_TRACE_D("IPSecState :: Stray response is detected on new SA", 0, 0, 0);

        if (pOldSA->RemoveTransaction(pTxnKey))
        {
            if (!pOldSA->HasPendingTransaction())
            {
                if (pOldSA->GetState() == STATE_TERMINATED_PENDING)
                {
                    pOldSA->SetState(STATE_TERMINATED);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
                else if ((pNewSA != IMS_NULL) && (pNewSA->GetState() == STATE_ACTIVE) &&
                        (pOldSA->GetState() != STATE_TERMINATED))
                {
                    pOldSA->SetState(STATE_TERMINATED);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
        }
    }

    if (bStrayResponseOnOldSA && (pNewSA != IMS_NULL))
    {
        IMS_TRACE_D("IPSecState :: Stray response is detected on old SA", 0, 0, 0);

        pNewSA->RemoveTransaction(pTxnKey);
    }
}

/*

Remarks

*/
PRIVATE
void SIPIPSecState::NotifyMessageSentInternal(IN CONST SIPTransportAddress& objNearEnd,
        IN CONST SIPTransportAddress& objFarEnd, IN SIPTxnKey* pTxnKey)
{
    IMS_BOOL bStrayResponseOnNewSA = IMS_FALSE;
    IMS_BOOL bStrayResponseOnOldSA = IMS_FALSE;

    //// Check a new SA...
    if ((pNewSA != IMS_NULL) && pNewSA->CheckIPAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSAPair = pNewSA->GetSA(objNearEnd, objFarEnd, SA::SA_IN);

        if (nSAPair != SA::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSAPair);

                // Outgoing SIP request
                pNewSA->AddTransaction(pTxnKey);

                if (pNewSA->GetState() == STATE_CREATED)
                {
                    pNewSA->SetState(STATE_PENDING);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Outgoing SIP response
                if (!pNewSA->RemoveTransaction(pTxnKey))
                {
                    bStrayResponseOnNewSA = IMS_TRUE;
                }
            }
        }
    }

    // Checks an old SA...
    if ((pOldSA != IMS_NULL) && pOldSA->CheckIPAddress(objNearEnd, objFarEnd))
    {
        IMS_SINT32 nSAPair = pOldSA->GetSA(objNearEnd, objFarEnd, SA::SA_IN);

        if (nSAPair != SA::SA_END)
        {
            if (pTxnKey->GetStatusCode() == 0)
            {
                pTxnKey->SetExtraInt(nSAPair);

                // Outgoing SIP request
                pOldSA->AddTransaction(pTxnKey);

                if (pOldSA->GetState() == STATE_TERMINATED)
                {
                    pOldSA->SetState(STATE_TERMINATED_PENDING);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
            else if (SipStatusCode::IsFinal(pTxnKey->GetStatusCode()))
            {
                // Outgoing SIP response
                if (pOldSA->RemoveTransaction(pTxnKey))
                {
                    if (!pOldSA->HasPendingTransaction() &&
                            (pOldSA->GetState() == STATE_TERMINATED_PENDING))
                    {
                        pOldSA->SetState(STATE_TERMINATED);
                        PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                    }
                }
                else
                {
                    bStrayResponseOnOldSA = IMS_TRUE;
                }
            }
        }

        if ((pNewSA != IMS_NULL) && (pNewSA->GetState() == STATE_ACTIVE) &&
                (pOldSA->GetState() != STATE_TERMINATED) && !pOldSA->HasPendingTransaction())
        {
            pOldSA->SetState(STATE_TERMINATED);
            PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
        }
    }

    if (bStrayResponseOnNewSA && (pOldSA != IMS_NULL))
    {
        IMS_TRACE_D("IPSecState :: Stray response is detected on new SA", 0, 0, 0);

        if (pOldSA->RemoveTransaction(pTxnKey))
        {
            if (!pOldSA->HasPendingTransaction())
            {
                if (pOldSA->GetState() == STATE_TERMINATED_PENDING)
                {
                    pOldSA->SetState(STATE_TERMINATED);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
                else if ((pNewSA != IMS_NULL) && (pNewSA->GetState() == STATE_ACTIVE) &&
                        (pOldSA->GetState() != STATE_TERMINATED))
                {
                    pOldSA->SetState(STATE_TERMINATED);
                    PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
                }
            }
        }
    }

    if (bStrayResponseOnOldSA && (pNewSA != IMS_NULL))
    {
        IMS_TRACE_D("IPSecState :: Stray response is detected on old SA", 0, 0, 0);

        pNewSA->RemoveTransaction(pTxnKey);
    }
}

/*

Remarks

*/
PRIVATE
void SIPIPSecState::NotifyTransactionAbortedInternal(IN SIPTxnKey* pTxnKey)
{
    if (pNewSA != IMS_NULL)
    {
        if (pNewSA->RemoveTransaction(pTxnKey))
        {
            if (!pNewSA->HasPendingTransaction() && (pNewSA->GetState() == STATE_PENDING))
            {
                pNewSA->SetState(STATE_CREATED);
                PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_NEW, 0);
            }

            return;
        }
    }

    if (pOldSA != IMS_NULL)
    {
        if (pOldSA->RemoveTransaction(pTxnKey))
        {
            if (!pOldSA->HasPendingTransaction() &&
                    (pOldSA->GetState() == STATE_TERMINATED_PENDING))
            {
                pOldSA->SetState(STATE_TERMINATED);
                PostMessage(AMSG_NOTIFY_STATE_CHANGED, SA_OLD, 0);
            }

            return;
        }
    }
}
