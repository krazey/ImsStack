/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  hwangoo.park@             Created
    </table>

    Description
     This class defines an extended SIP dialog.
    It has a dialog usage to support a multiple dialog usage in SIP.
*/

#include "ServiceMemory.h"
#include "SipPrivate.h"
#include "SipDebug.h"
#include "SipDialogInviteUsage.h"
#include "SipDialogSubscribeUsage.h"
#include "SipDialogEx.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPDialogEx::SIPDialogEx(IN SIPDialogState* pDState_) :
        SIPDialogBase(pDState_),
        bFlag_PermanentDialog(IMS_FALSE),
        bFlag_InitWithDelay(IMS_FALSE),
        bFlag_IsDialogTerminated(IMS_FALSE),
        pDialogUsage(IMS_NULL)
{
}

PUBLIC
SIPDialogEx::SIPDialogEx(IN CONST SIPDialogEx& objRHS) :
        SIPDialogBase(objRHS),
        bFlag_PermanentDialog(objRHS.bFlag_PermanentDialog),
        bFlag_InitWithDelay(objRHS.bFlag_InitWithDelay),
        bFlag_IsDialogTerminated(objRHS.bFlag_IsDialogTerminated),
        pDialogUsage(IMS_NULL)
{
    // NOTE: If reference count is not used, you MUST implement this copy constructor.
}

PUBLIC VIRTUAL SIPDialogEx::~SIPDialogEx()
{
    if (bFlag_PermanentDialog && !bFlag_IsDialogTerminated)
    {
        SIPDialogState* pDState = GetDialogState();

        if (pDState != IMS_NULL)
        {
            pDState->RemoveDialogUsage(this);
        }
    }

    if (pDialogUsage != IMS_NULL)
    {
        delete pDialogUsage;
        pDialogUsage = IMS_NULL;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPDialogEx (%s)",
            SipDebug::GetCharA1(GetDialogState()->GetCallId().GetStr(), 8, '@'), 0, 0);
#endif
}

PUBLIC
SIPDialogEx& SIPDialogEx::operator=(IN CONST SIPDialogEx& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        SIPDialogBase::operator=(objRHS);
        bFlag_PermanentDialog = objRHS.bFlag_PermanentDialog;
        bFlag_InitWithDelay = objRHS.bFlag_InitWithDelay;
        bFlag_IsDialogTerminated = objRHS.bFlag_IsDialogTerminated;
        // NOTE: If reference count is not used, this MUST be newly created.
        if (objRHS.pDialogUsage == IMS_NULL)
        {
            pDialogUsage = IMS_NULL;
        }
        else
        {
            pDialogUsage = objRHS.pDialogUsage->Clone();
        }
    }

    return (*this);
}

// For an initial requests
PUBLIC
IMS_BOOL SIPDialogEx::InitDialog(IN CONST SipMethod& objMethod)
{
    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SipMethod::INVITE))
    {
        pDialogUsage = new SIPDialogInviteUsage(this);
    }
    else if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER))
    {
        SIPDialogSubscribeUsage* pSubscribeUsage = new SIPDialogSubscribeUsage(this);

        if (pSubscribeUsage == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pSubscribeUsage->InitDialogUsage(objMethod))
        {
            IMS_TRACE_E(0, "Initializing a dialog usage (%s) failed", objMethod.ToString().GetStr(),
                    0, 0);

            delete pSubscribeUsage;
            return IMS_FALSE;
        }

        pDialogUsage = pSubscribeUsage;
    }
    else
    {
        pDialogUsage = new SIPDialogUsage(this);
    }

    if (pDialogUsage == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Instantiating a dialog usage (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // Before sending a request message to the network,
    // if the method, "InitDialogWithDelay(...)" is invoked, this flag will be used to identify
    // whether the operation will be performed or not.
    bFlag_InitWithDelay = IMS_TRUE;

    return IMS_TRUE;
}

// For a dialog request or incoming requests
PUBLIC
IMS_BOOL SIPDialogEx::InitDialog(IN CONST SIPMessageInfo& objMInfo)
{
    const SipMethod& objMethod = objMInfo.GetMethod();

    //---------------------------------------------------------------------------------------------

    if (objMethod.Equals(SipMethod::INVITE))
    {
        pDialogUsage = new SIPDialogInviteUsage(this);
    }
    // For a forked NOTIFY request, adds NOTIFY method checking
    else if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER) ||
            objMethod.Equals(SipMethod::NOTIFY))
    {
        pDialogUsage = new SIPDialogSubscribeUsage(this);
    }
    else
    {
        pDialogUsage = new SIPDialogUsage(this);
    }

    if (pDialogUsage == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Instantiating a dialog usage (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (!pDialogUsage->InitDialogUsage(objMInfo))
    {
        IMS_TRACE_E(
                0, "Initializing a dialog usage (%s) failed", objMethod.ToString().GetStr(), 0, 0);

        delete pDialogUsage;
        pDialogUsage = IMS_NULL;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SIPDialogEx::InitDialogWithDelay(IN CONST SIPMessageInfo& objMInfo)
{
    //---------------------------------------------------------------------------------------------

    // If we don't need to initialize a dialog, it will be ignored in here.
    if (!bFlag_InitWithDelay)
    {
        return IMS_TRUE;
    }

    if (pDialogUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pDialogUsage->InitDialogUsage(objMInfo);
}

PUBLIC
IMS_BOOL SIPDialogEx::CompareTo(IN CONST SIPMessageInfo& objMInfo) const
{
    //---------------------------------------------------------------------------------------------

    if (pDialogUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pDialogUsage->CompareTo(objMInfo);
}

PUBLIC
IMS_BOOL SIPDialogEx::Equals(IN SIPDialogEx* pDialogEx) const
{
    //---------------------------------------------------------------------------------------------

    if (pDialogEx == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((pDialogUsage == IMS_NULL) || (pDialogEx->pDialogUsage == IMS_NULL))
    {
        return IMS_FALSE;
    }

    return pDialogUsage->Equals(pDialogEx->pDialogUsage);
}

PUBLIC
IMS_BOOL SIPDialogEx::IsInviteUsage() const
{
    //---------------------------------------------------------------------------------------------

    if (pDialogUsage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (pDialogUsage->GetType() == SIPDialogUsage::TYPE_INVITE);
}

PUBLIC
IMS_BOOL SIPDialogEx::IsDialogTerminated() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_IsDialogTerminated;
}

PUBLIC
void SIPDialogEx::TerminateDialogUsage()
{
    IMS_TRACE_I("TerminateDialogUsage :: %s",
            ((pDialogUsage != IMS_NULL) ? pDialogUsage->ToString().GetStr() : "__NULL__"), 0, 0);

    OnTerminated();
}

PUBLIC
IMS_SINT32 SIPDialogEx::UpdateDialogDetails(IN CONST SIPMessageInfo& objMInfo)
{
    //---------------------------------------------------------------------------------------------

    if (pDialogUsage == IMS_NULL)
    {
        return SIPPrivate::MESSAGE_FAILED;
    }

    return pDialogUsage->UpdateUsageDetails(objMInfo);
}

PUBLIC GLOBAL SIPDialogEx* SIPDialogEx::CreateDialog(IN CONST SipMethod& objMethod)
{
    SIPDialogState* pDState = new SIPDialogState();

    //---------------------------------------------------------------------------------------------

    if (pDState == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Instantiating a dialog state (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    SIPDialogEx* pDialogEx = CreateDialog(pDState, objMethod);

    if (pDialogEx == IMS_NULL)
    {
        delete pDState;

        IMS_TRACE_E(0, "Instantiating a dialog (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    return pDialogEx;
}

PUBLIC GLOBAL SIPDialogEx* SIPDialogEx::CreateDialog(
        IN SIPDialogState* pDState, IN CONST SipMethod& objMethod)
{
    SIPDialogEx* pDialogEx = new SIPDialogEx(pDState);

    //---------------------------------------------------------------------------------------------

    if (pDialogEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a dialog (%s) failed", objMethod.ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!pDialogEx->InitDialog(objMethod))
    {
        delete pDialogEx;
        return IMS_NULL;
    }

    return pDialogEx;
}

PUBLIC GLOBAL SIPDialogEx* SIPDialogEx::CreateDialog(
        IN SIPDialogState* pDState, IN CONST SIPMessageInfo& objMInfo)
{
    SIPDialogEx* pDialogEx = new SIPDialogEx(pDState);

    //---------------------------------------------------------------------------------------------

    if (pDialogEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a dialog (%s) failed",
                objMInfo.GetMethod().ToString().GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (!pDialogEx->InitDialog(objMInfo))
    {
        delete pDialogEx;
        return IMS_NULL;
    }

    return pDialogEx;
}

PROTECTED VIRTUAL IMS_BOOL SIPDialogEx::OnInit()
{
    //---------------------------------------------------------------------------------------------

    if (bFlag_PermanentDialog == IMS_FALSE)
    {
        SIPDialogState* pDState = GetDialogState();

        if (pDState == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pDState->AddDialogUsage(this))
        {
            return IMS_FALSE;
        }

        bFlag_PermanentDialog = IMS_TRUE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void SIPDialogEx::OnTerminated()
{
    //---------------------------------------------------------------------------------------------

    if (bFlag_PermanentDialog == IMS_TRUE)
    {
        SIPDialogState* pDState = GetDialogState();

        if (pDState != IMS_NULL)
        {
            pDState->RemoveDialogUsage(this);
        }

        bFlag_PermanentDialog = IMS_FALSE;
    }

    bFlag_IsDialogTerminated = IMS_TRUE;
}

PROTECTED VIRTUAL IMS_SINT32 SIPDialogEx::OnUpdateDialogDetails(IN CONST SIPMessageInfo& objMInfo,
        IN IMS_SINT32 nUsage, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger)
{
    IMS_SINT32 nUsageState = SIPDState::STATE_MAX;

    //---------------------------------------------------------------------------------------------

    switch (nUsage)
    {
        case SIPDialogUsage::TYPE_INVITE:
            nUsageState = SIPDialogInviteUsage::GetNextState(GetState(), nTrigger);
            break;

        case SIPDialogUsage::TYPE_SUBSCRIBE:
            nUsageState = SIPDialogSubscribeUsage::GetNextState(GetState(), nTrigger);
            break;

        default:
            break;
    }

    SIPDialogState* pDState = GetDialogState();

    if (pDState == IMS_NULL)
    {
        return SIPPrivate::MESSAGE_FAILED;
    }

    return pDState->UpdateDialogDetails(objMInfo, nUsageState, nAction, nTrigger);
}
