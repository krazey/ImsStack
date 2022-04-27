/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100707  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceTrace.h"
#include "ISIPHeader.h"
#include "SIP.h"
#include "SIPStatusCode.h"
#include "SIPDebug.h"
#include "SIPClientTransactionState.h"
#include "SIPForkedTransactionManager.h"

__IMS_TRACE_TAG_SIP__;



PUBLIC
SIPForkedTransactionManager::SIPForkedTransactionManager()
    : RCObject()
    , nStatusCode(SIPStatusCode::SC_INVALID)
{
}

PUBLIC
SIPForkedTransactionManager::SIPForkedTransactionManager(
        IN const SIPForkedTransactionManager& objRHS)
    : RCObject(objRHS)
    , nStatusCode(objRHS.nStatusCode)
{
}

PUBLIC VIRTUAL
SIPForkedTransactionManager::~SIPForkedTransactionManager()
{
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPForkedTransactionManager::Add(IN SIPClientTransactionState *pCTState)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTxnStates.GetSize(); ++i)
    {
        const RCPtr<SIPClientTransactionState> &pTmpCTState = objTxnStates.GetAt(i);

        if (pTmpCTState.Get() == pCTState)
        {
            // Already exists
            return IMS_TRUE;
        }
    }

    if (!objTxnStates.Append(pCTState))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPForkedTransactionManager::IsEmpty() const
{
    //---------------------------------------------------------------------------------------------

    return objTxnStates.IsEmpty();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPForkedTransactionManager::IsTransactionCompleted() const
{
    //---------------------------------------------------------------------------------------------

    return SIPStatusCode::IsFinal(nStatusCode);
}

/*

Remarks

*/
PUBLIC
SIPClientTransactionState* SIPForkedTransactionManager::Lookup(IN SipMessage *pstMessage) const
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (objTxnStates.IsEmpty())
    {
        return IMS_NULL;
    }

    // Not forked case
    if (objTxnStates.GetSize() == 1)
    {
        const RCPtr<SIPClientTransactionState> &pCTState = objTxnStates.GetAt(0);

        return pCTState.Get();
    }

    SipHeaderBase *pstHeader;
    AString strNewLocalTag;
    AString strNewRemoteTag;
    AString strCallId;

    // Call Id
    pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::CALL_ID);
    SIPStack::EncodeHeaderBody(pstHeader, IMS_FALSE, strCallId);
    SIPStack::FreeHeaderEx(pstHeader);

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        // Get local tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

        strNewLocalTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);

        // Get remote tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);

        strNewRemoteTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);
    }
    else
    {
        // Get local tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::FROM);

        strNewLocalTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);

        // Get remote tag
        pstHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::TO);

        strNewRemoteTag = SIPStack::GetParameter(pstHeader, SIP::STR_TAG);
        SIPStack::FreeHeaderEx(pstHeader);
    }

    for (IMS_UINT32 i = 0; i < objTxnStates.GetSize(); ++i)
    {
        const RCPtr<SIPClientTransactionState> &pCTState = objTxnStates.GetAt(i);
        SIPDialogState *pDState = pCTState->GetDialog()->GetDialogState();

        AString strLocalTag = pDState->GetLocalTag();
        AString strRemoteTag = pDState->GetRemoteTag();

        if (strCallId.Equals(pDState->GetCallId())
                && strLocalTag.Equals(strNewLocalTag)
                && strRemoteTag.Equals(strNewRemoteTag))
        {
            IMS_TRACE_D("ForkedTransactionManager :: Dialog " \
                    "(call-id=%s, local-tag=%s, remote-tag=%s)",
                    SIPDebug::GetCharA1(strCallId.GetStr(), 8, '@'),
                    strLocalTag.GetStr(), strRemoteTag.GetStr());
            return pCTState.Get();
        }
    }

    const RCPtr<SIPClientTransactionState> &pCTState
            = objTxnStates.GetAt(objTxnStates.GetSize() - 1);

    return pCTState.Get();
}

/*

Remarks

*/
PUBLIC
void SIPForkedTransactionManager::Remove(IN SIPClientTransactionState *pCTState)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTxnStates.GetSize(); ++i)
    {
        const RCPtr<SIPClientTransactionState> &pTmpCTState = objTxnStates.GetAt(i);

        if (pTmpCTState.Get() == pCTState)
        {
            // Found
            objTxnStates.RemoveAt(i);
            break;
        }
    }
}

/*

Remarks

*/
PUBLIC
void SIPForkedTransactionManager::SetTransactionCompleted(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    this->nStatusCode = nStatusCode;
}
