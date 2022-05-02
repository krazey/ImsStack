/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "StaticSIP.h"
#include "SIPPrivate.h"
#include "SIPUtil.h"
#include "SipDebug.h"
#include "SipConfigProxy.h"
#include "SIPStackHeaders.h"
#include "SIPStackState.h"
#include "SIPAckPackage.h"
#include "SIPFactoryProxy.h"
#include "SIPTransportHelper.h"
#include "SIPMessageHandler.h"
#include "SIPConnectionNotifierImpl.h"
#include "SIPClientConnectionImpl.h"
#include "SIPManager.h"

__IMS_TRACE_TAG_SIP__;



PRIVATE
SIPManager::SIPManager()
    : nState(STATE_INACTIVE)
{
    SIPPrivate::Init();
}

PRIVATE
SIPManager::~SIPManager()
{
    CleanUp();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPManager::AttachDialogState(IN SIPDialogState *pDState)
{
    if (nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pDState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("___ Attach::DialogState (%s)",
            SIPDebug::GetCharA1(pDState->GetCallId().GetStr(), 8, '@'), 0, 0);

    return objDialogStates.Append(pDState);
}

/*

Remarks

*/
PUBLIC
void SIPManager::DetachDialogState(IN SIPDialogState *pDState)
{
    if (nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objDialogStates.GetSize(); ++i)
    {
        SIPDialogState *pTempDState = objDialogStates.GetAt(i);

        if (pTempDState != IMS_NULL)
        {
#if 1
            if (pTempDState == pDState)
            {
                IMS_TRACE_I("___ Detach::DialogState (%s)",
                        SIPDebug::GetCharA1(pDState->GetCallId().GetStr(), 8, '@'),
                        0, 0);

                objDialogStates.RemoveAt(i);
                return;
            }
#else
            IMS_SINT32 nComparisonResult
                    = pExistingDState->CompareTo(pDState, IMS_NULL, IMS_FALSE);

            switch (nComparisonResult)
            {
            case SIPDialogState::MATCHED:
                IMS_TRACE_I("___ Detach::DialogState (%s)",
                        pExistingDState->GetCallId().GetStr(), 0, 0);
                objDialogStates.RemoveAt(n);
                return;
            }
#endif
        }
    }
}

/*

Remarks

*/
PUBLIC
RCPtr<SIPDialogState> SIPManager::LookupDialogState(IN SIPDialogState *pDState,
        IN SipMessage *pstMessage, IN IMS_BOOL bCheckForked /* = IMS_FALSE */,
        OUT IMS_BOOL *pbIsForked /* = IMS_NULL */)
{
    if (nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return IMS_NULL;
    }

    if (pbIsForked != IMS_NULL)
    {
        (*pbIsForked) = IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objDialogStates.GetSize(); ++i)
    {
        SIPDialogState *pTempDState = objDialogStates.GetAt(i);

        if (pTempDState != IMS_NULL)
        {
            IMS_SINT32 nComparisonResult
                    = pTempDState->CompareTo(pDState, pstMessage, bCheckForked);

            switch (nComparisonResult)
            {
            case SIPDialogState::MATCHED:
                return pTempDState;

            case SIPDialogState::NOT_MATCHED:
                break;

            case SIPDialogState::MATCHED_DIFFERENT:
                break;

            case SIPDialogState::MATCHED_FORKED_SUBSCRIBE:
                if (pbIsForked != IMS_NULL)
                {
                    (*pbIsForked) = IMS_TRUE;
                }

                IMS_TRACE_I("___ FORKED NOTIFY RECEIVED ___", 0, 0, 0);
                return pTempDState;

            case SIPDialogState::MATCHED_EARLY_NOTIFY:
                IMS_TRACE_I("___ EARLY NOTIFY RECEIVED ___", 0, 0, 0);
                return pTempDState;

            case SIPDialogState::MATCHED_OVERLAP_DIALING:
                break;

            default:
                break;
            }
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPManager::AttachConnectionNotifier(IN SIPConnectionNotifier *pSCN)
{
    if (nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return IMS_FALSE;
    }

    return objSCNs.Append(pSCN);
}

/*

Remarks

*/
PUBLIC
void SIPManager::DetachConnectionNotifier(IN SIPConnectionNotifier *pSCN)
{
    if (nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objSCNs.GetSize(); ++i)
    {
        SIPConnectionNotifier *pTempSCN = objSCNs.GetAt(i);

        if (pTempSCN != IMS_NULL)
        {
            if (pTempSCN == pSCN)
            {
                objSCNs.RemoveAt(i);
                return;
            }
        }
    }
}

/*

Remarks

*/
PUBLIC
SIPConnectionNotifier* SIPManager::LookupConnectionNotifier(IN CONST SIPTransportAddress &objTA,
        IN CONST AString &strFilter /* = AString::ConstNull() */)
{
    (void) strFilter;

    if (nState != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objSCNs.GetSize(); ++i)
    {
        SIPConnectionNotifier *pTempSCN = objSCNs.GetAt(i);

        if (pTempSCN != IMS_NULL)
        {
            if (pTempSCN->IsSameConnectionNotifier(objTA))
            {
                return pTempSCN;
            }
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC GLOBAL
SIPManager* SIPManager::GetInstance()
{
    static SIPManager *pSIPMngr = IMS_NULL;

    if (pSIPMngr == IMS_NULL)
    {
        pSIPMngr = new SIPManager();
    }

    return pSIPMngr;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPManager::StartUp()
{
    if (nState == STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS ALREADY ACTIVE ...", 0, 0, 0);
        return IMS_TRUE;
    }

    if (nState == STATE_PENDING)
    {
        return IMS_FALSE;
    }

    // Initialize the SIP stack & transaction layer
    SIPStackState::GetInstance()->StartUp();

    // For ACK retransmission for 2xx response to INVITE
    SIPAckPackage::Init();

    SIPDebug::InitLogging();

    nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void SIPManager::CleanUp()
{
    if ((nState == STATE_INACTIVE) || (nState == STATE_PENDING))
    {
        IMS_TRACE_E(0, "... SIP MANAGER IS NOT ACTIVE ...", 0, 0, 0);
        return;
    }

    nState = STATE_PENDING;

    // Delete the dialog state info.
    IMS_TRACE_D("___ Number of DialogState (%d)", objDialogStates.GetSize(), 0, 0);

    if (!objDialogStates.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objDialogStates.GetSize(); ++i)
        {
            SIPDialogState *pDState = objDialogStates.GetAt(i);

            if (pDState != IMS_NULL)
            {
                pDState->RemoveReference();
            }
        }

        objDialogStates.Clear();
    }

    IMS_TRACE_D("___ Number of ConnectionNotifier (%d)", objSCNs.GetSize(), 0, 0);

    if (!objSCNs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSCNs.GetSize(); ++i)
        {
            SIPConnectionNotifier *pSCN = objSCNs.GetAt(i);

            if (pSCN != IMS_NULL)
            {
                delete pSCN;
            }
        }

        objSCNs.Clear();
    }

    SIPStackState::GetInstance()->CleanUp();

    nState = STATE_INACTIVE;
}

/*

Remarks

*/
PUBLIC GLOBAL
IMS_BOOL StaticSIP::Initialize()
{
    if (!SIPManager::GetInstance()->StartUp())
    {
        return IMS_FALSE; // throw exception
    }

    IMS_TRACE_D(">>> SIP ENGINE IS SUCCESSFULLY LOADED <<<", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC GLOBAL
void StaticSIP::Uninitialize()
{
    SIPManager::GetInstance()->CleanUp();

    IMS_TRACE_D(">>> SIP ENGINE IS SUCCESSFULLY UNLOADED <<<", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC GLOBAL
void StaticSIP::InitializeForSlot(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("InitializeForSlot :: slotId=%d", nSlotId, 0, 0);

    IMS_SINT32 nOptions = SIPPrivate::OPTIONS_E;

    if (SIPConfigProxy::IsCompactFormConfigured(nSlotId))
    {
        nOptions |= SIPPrivate::OPT_E_SHORTFORM;
    }
    else
    {
        nOptions |= SIPPrivate::OPT_E_FULLFORM;
    }

    SIPPrivate::Init(nSlotId, nOptions);

    SIPUtil::Init(nSlotId);

    SIPTransportHelper* pTransportHelper
            = SIPFactoryProxy::GetInstance()->GetTransportHelper(nSlotId);

    // Attach the message handler from the network
    pTransportHelper->SetListener(SIPMessageHandler::GetInstance());
}

/*

Remarks

*/
PUBLIC GLOBAL
void StaticSIP::UninitializeForSlot(IN IMS_SINT32 nSlotId)
{
    (void) nSlotId;

    IMS_TRACE_D("UninitializeForSlot :: slotId=%d", nSlotId, 0, 0);
}
