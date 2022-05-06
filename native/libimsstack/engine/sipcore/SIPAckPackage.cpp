/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140318  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipDebug.h"
#include "SIPAck.h"
#include "SIPAckPackage.h"

__IMS_TRACE_TAG_SIP__;

class SIPAckPackagePrivate
{
public:
    inline SIPAckPackagePrivate() {}

    inline ~SIPAckPackagePrivate()
    {
        for (IMS_UINT32 i = 0; i < objAckPackages.GetSize(); ++i)
        {
            SIPAckPackage* pPackage = objAckPackages.GetAt(i);

            if (pPackage != IMS_NULL)
            {
                delete pPackage;
            }
        }

        objAckPackages.Clear();
    }

public:
    IMSList<SIPAckPackage*> objAckPackages;
};

PRIVATE GLOBAL SIPAckPackagePrivate* SIPAckPackage::pAckPackageP = IMS_NULL;

PRIVATE
SIPAckPackage::SIPAckPackage(IN CONST AString& strCallId_) :
        strCallId(strCallId_)
{
}

PUBLIC VIRTUAL SIPAckPackage::~SIPAckPackage()
{
    for (IMS_UINT32 i = 0; i < objAcks.GetSize(); ++i)
    {
        SIPAck* pAck = objAcks.GetAt(i);

        if (pAck != IMS_NULL)
        {
            delete pAck;
        }
    }

    objAcks.Clear();
}

/*

Remarks

*/
PUBLIC
void SIPAckPackage::AddAck(IN SIPClientTransactionState* pCTState, IN IMS_SINT32 nAliveInterval)
{
    //---------------------------------------------------------------------------------------------

    if (pCTState == IMS_NULL)
    {
        return;
    }

    SipTxnKey* pstTxnKey = pCTState->GetTxnKey();

    if (pstTxnKey == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIPAckPackage :: no txn key", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objAcks.GetSize(); ++i)
    {
        SIPAck* pAck = objAcks.GetAt(i);

        if (pAck == IMS_NULL)
        {
            continue;
        }

        if (pAck->IsSameTransaction(pstTxnKey))
        {
            // ACK already exists in the ACK package
            IMS_TRACE_D("SIPAckPackage :: SIPAck already exists (%s)",
                    SIPStack::TxnKey_GetViaBranch(pstTxnKey), 0, 0);
            return;
        }
    }

    IMS_TRACE_D("SIPAckPackage :: SIPAck (%s, %d)", SIPStack::TxnKey_GetViaBranch(pstTxnKey),
            nAliveInterval, 0);

    objAcks.Append(new SIPAck(pCTState, nAliveInterval));
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPAckPackage::IsSamePackage(IN CONST AString& strCallId) const
{
    //---------------------------------------------------------------------------------------------

    return this->strCallId.Equals(strCallId);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPAckPackage::NotifyStray2xx(IN SipTxnKey* pstTxnKey)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objAcks.GetSize(); ++i)
    {
        SIPAck* pAck = objAcks.GetAt(i);

        if (pAck == IMS_NULL)
        {
            continue;
        }

        if (pAck->IsStrayAck())
        {
            continue;
        }

        if (pAck->IsSameTransaction(pstTxnKey))
        {
            pAck->RetransmitMessage();
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPAckPackage::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (pAckPackageP == IMS_NULL)
    {
        IMS_TRACE_D("SIPAckPackage (%s) is destroyed",
                SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);
        delete this;
        return;
    }

    for (IMS_UINT32 i = 0; i < pAckPackageP->objAckPackages.GetSize(); ++i)
    {
        SIPAckPackage* pPackage = pAckPackageP->objAckPackages.GetAt(i);

        if (pPackage == IMS_NULL)
        {
            continue;
        }

        if (pPackage->IsSamePackage(strCallId))
        {
            IMS_TRACE_D("SIPAckPackage (%s) is destroyed",
                    SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);

            pAckPackageP->objAckPackages.RemoveAt(i);
            delete this;
            break;
        }
    }

    IMS_TRACE_D("SIPAckPackage :: size=%d", pAckPackageP->objAckPackages.GetSize(), 0, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPAckPackage::RemoveStrayAcks()
{
    IMS_UINT32 nTotalCount = objAcks.GetSize();

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objAcks.GetSize();)
    {
        SIPAck* pAck = objAcks.GetAt(i);

        if (pAck == IMS_NULL)
        {
            objAcks.RemoveAt(i);
            continue;
        }

        if (pAck->IsStrayAck())
        {
            objAcks.RemoveAt(i);
            delete pAck;
        }
        else
        {
            ++i;
        }
    }

    if (objAcks.GetSize() < nTotalCount)
    {
        IMS_TRACE_D("SIPAckPackage :: ACK (%d >> %d)", nTotalCount, objAcks.GetSize(), 0);
    }
}

/*

Remarks

*/
PUBLIC GLOBAL SIPAckPackage* SIPAckPackage::CreateAckPackage(IN CONST AString& strCallId)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < pAckPackageP->objAckPackages.GetSize(); ++i)
    {
        SIPAckPackage* pPackage = pAckPackageP->objAckPackages.GetAt(i);

        if (pPackage == IMS_NULL)
        {
            continue;
        }

        if (pPackage->IsSamePackage(strCallId))
        {
            IMS_TRACE_D("SIPAckPackage :: RE-USE (%s)",
                    SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);
            return pPackage;
        }
    }

    SIPAckPackage* pNewPackage = new SIPAckPackage(strCallId);

    pAckPackageP->objAckPackages.Append(pNewPackage);

    IMS_TRACE_D(
            "SIPAckPackage (%s) is created", SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);

    return pNewPackage;
}

/*

Remarks

*/
PUBLIC GLOBAL void SIPAckPackage::Init()
{
    //---------------------------------------------------------------------------------------------

    if (pAckPackageP == IMS_NULL)
    {
        pAckPackageP = new SIPAckPackagePrivate();
    }
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL SIPAckPackage::HandleStray2xx(IN SipMessage* pstMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pAckPackageP == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipTxnKey* pstTxnKey = SIPStack::CreateTxnKey(pstMessage, SIPStack::SIP_TXN_MSG_RECEIVED);

    if (pstTxnKey == IMS_NULL)
    {
        IMS_TRACE_D("SIPAckPackage :: Stray 2xx is discarded", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strCallId(SIPStack::TxnKey_GetCallId(pstTxnKey));
    IMS_BOOL bStray2xxHandled = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pAckPackageP->objAckPackages.GetSize(); ++i)
    {
        SIPAckPackage* pPackage = pAckPackageP->objAckPackages.GetAt(i);

        if (pPackage == IMS_NULL)
        {
            continue;
        }

        if (pPackage->IsSamePackage(strCallId))
        {
            bStray2xxHandled = pPackage->NotifyStray2xx(pstTxnKey);

            if (!bStray2xxHandled)
            {
                IMS_TRACE_D("SIPAckPackage :: ACK (%s) is not handled",
                        SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'), 0, 0);
            }
            break;
        }
    }

    SIPStack::FreeTxnKey(pstTxnKey);

    return bStray2xxHandled;
}
