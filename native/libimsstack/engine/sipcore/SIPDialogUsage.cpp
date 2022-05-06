/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100309  hwangoo.park@             Created
    </table>

    Description
     This class defines a dialog usage for SIP. Several methods in the SIP can create
    an association between endpoints known as a dialog. Some of these methods can also create
    a different, but related, association within an existing dialog.
    These multiple associations, or dialog usages, require carefully coordinated processing
    as they have independent life-cycles, but share common dialog state.
    There are two dialog usages.
    - A dialog initiated with an INVITE request has an invite usage.
    - A dialog initiated with a SUBSCRIBE request has a subscribe usage.
    - A dialog initiated with a REFER request has a subscribe usage.
*/

#include "ServiceMemory.h"
#include "SIPPrivate.h"
#include "ISipHeader.h"
#include "SipFeatures.h"
#include "SIPDialogUsage.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPDialogUsage::SIPDialogUsage(IN SIPDialogBase* pDialogBase_) :
        nType(TYPE_EPHEMERAL),
        pDialogBase(pDialogBase_)
{
}

PUBLIC
SIPDialogUsage::SIPDialogUsage(IN IMS_SINT32 nType_, IN SIPDialogBase* pDialogBase_) :
        nType(nType_),
        pDialogBase(pDialogBase_)
{
}

PUBLIC
SIPDialogUsage::SIPDialogUsage(IN CONST SIPDialogUsage& objRHS) :
        nType(objRHS.nType),
        pDialogBase(objRHS.pDialogBase)
{
}

PUBLIC VIRTUAL SIPDialogUsage::~SIPDialogUsage()
{
    IMS_TRACE_D("Destructor :: SIPDialogUsage (%s)",
            (nType == TYPE_EPHEMERAL) ? "EPHEMERAL"
                                      : ((nType == TYPE_INVITE) ? "INVITE" : "SUBSCRIBE"),
            0, 0);
}

PUBLIC VIRTUAL IMS_BOOL SIPDialogUsage::InitDialogUsage(IN CONST SIPMessageInfo& /* objMInfo */)
{
    //---------------------------------------------------------------------------------------------

    if (nType != TYPE_EPHEMERAL)
    {
        if (!pDialogBase->OnInit())
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL SIPDialogUsage* SIPDialogUsage::Clone() const
{
    //---------------------------------------------------------------------------------------------

    return new SIPDialogUsage(*this);
}

PUBLIC VIRTUAL IMS_BOOL SIPDialogUsage::CompareTo(IN CONST SIPMessageInfo& /* objMInfo */) const
{
    //---------------------------------------------------------------------------------------------

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SIPDialogUsage::Equals(IN SIPDialogUsage* pDUsage) const
{
    //---------------------------------------------------------------------------------------------

    if (pDUsage == IMS_NULL)
        return IMS_FALSE;

    if (nType != pDUsage->nType)
        return IMS_FALSE;

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SIPDialogUsage::ToString() const
{
    AString strValue;

    strValue.Sprintf("DialogUsage: [%s]",
            (nType == TYPE_INVITE) ? "Invite"
                                   : ((nType == TYPE_SUBSCRIBE) ? "Subscribe" : "Ephemeral"));

    return strValue;
}

PUBLIC VIRTUAL IMS_SINT32 SIPDialogUsage::UpdateUsageDetails(IN CONST SIPMessageInfo& objMInfo)
{
    IMS_SINT32 nTrigger = SIPDState::TRIGGER_INIT;
    IMS_SINT32 nAction = GetActionNTrigger(objMInfo, nTrigger);

    //---------------------------------------------------------------------------------------------

    // If a dialog usage needs to be destroyed, then remove it and update the dialog details.
    if (nType != TYPE_EPHEMERAL)
    {
        if (IsUsageTerminated(pDialogBase->GetState(), nTrigger) ||
                (nAction == SIPDState::ACTION_DESTROY_USAGE) ||
                (nAction == SIPDState::ACTION_DESTROY_DIALOG))
        {
            pDialogBase->OnTerminated();
        }
    }

    IMS_TRACE_D("UpdateDialogDetails() - %s : %s", ActionToString(nAction),
            TriggerToString(nTrigger), 0);

    return pDialogBase->OnUpdateDialogDetails(objMInfo, nType, nAction, nTrigger);
}

PROTECTED VIRTUAL IMS_SINT32 SIPDialogUsage::GetActionNTrigger(
        IN CONST SIPMessageInfo& /* objMInfo */, OUT IMS_SINT32& nTrigger)
{
    //---------------------------------------------------------------------------------------------

    nTrigger = SIPDState::TRIGGER_INIT;

    return SIPDState::ACTION_IGNORE;
}

PROTECTED VIRTUAL IMS_BOOL SIPDialogUsage::IsUsageTerminated(
        IN IMS_SINT32 /* nState */, IN IMS_SINT32 /* nTrigger */) const
{
    //---------------------------------------------------------------------------------------------

    return IMS_TRUE;
}

PROTECTED
IMS_SINT32 SIPDialogUsage::GetActionForResponse(IN CONST SIPMessageInfo& objMInfo)
{
    // Base class's method takes an action by the response message only.
    SipMessage* pstMessage = objMInfo.GetMessage();

    //---------------------------------------------------------------------------------------------

    if (!SIPStack::IsRequestMessage(pstMessage))
    {
        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);
        SIPDialogState* pDialogState = pDialogBase->GetDialogState();

        switch (nStatusCode)
        {
            // Impacts on a dialog usage (-> destroys a dialog usage)
            case SipStatusCode::SC_481:
                if (objMInfo.IsOutgoingMessage() && objMInfo.GetMethod().Equals(SipMethod::PRACK))
                {
                    IMS_TRACE_D("481-PRACK is ignored on dialog transition", 0, 0, 0);
                    return SIPDState::ACTION_IGNORE;
                }

                return SIPDState::ACTION_DESTROY_USAGE;

            case SipStatusCode::SC_405:
            case SipStatusCode::SC_501:
                if (pDialogBase->GetState() == SIPDState::STATE_CONFIRMED)
                {
                    const SipMethod& objMethod = objMInfo.GetMethod();

                    if (nType == TYPE_INVITE)
                    {
                        if (!objMethod.Equals(SipMethod::INVITE) &&
                                !objMethod.Equals(SipMethod::UPDATE) &&
                                !objMethod.Equals(SipMethod::PRACK) &&
                                !objMethod.Equals(SipMethod::BYE))
                        {
                            return SIPDState::ACTION_IGNORE;
                        }
                    }
                    else if (nType == TYPE_SUBSCRIBE)
                    {
                        if (!objMethod.Equals(SipMethod::SUBSCRIBE) &&
                                !objMethod.Equals(SipMethod::NOTIFY))
                        {
                            return SIPDState::ACTION_IGNORE;
                        }
                    }
                }

                if (SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(
                            objMInfo.GetSlotId()) ||
                        ((pDialogState != IMS_NULL) && pDialogState->HasMultipleDialogUsages()))
                {
                    return SIPDState::ACTION_DESTROY_USAGE;
                }

                if (pDialogBase->GetState() != SIPDState::STATE_CONFIRMED)
                {
                    return SIPDState::ACTION_TRANSIT_STATE;
                }
                break;

            case SipStatusCode::SC_489:
                if ((nType != TYPE_SUBSCRIBE) &&
                        (pDialogBase->GetState() == SIPDState::STATE_CONFIRMED))
                {
                    return SIPDState::ACTION_IGNORE;
                }

                return SIPDState::ACTION_DESTROY_USAGE;

            case SipStatusCode::SC_480:
                if (pDialogBase->GetState() == SIPDState::STATE_CONFIRMED)
                {
                    if (SIPStack::IsHeaderPresent(pstMessage, ISipHeader::RETRY_AFTER_ANY))
                    {
                        return SIPDState::ACTION_IGNORE;
                    }
                }

                if (SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(
                            objMInfo.GetSlotId()) ||
                        ((pDialogState != IMS_NULL) && pDialogState->HasMultipleDialogUsages()))
                {
                    return SIPDState::ACTION_DESTROY_USAGE;
                }

                if (pDialogBase->GetState() != SIPDState::STATE_CONFIRMED)
                {
                    return SIPDState::ACTION_TRANSIT_STATE;
                }
                break;

                // Impacts on a dialog (-> destroys a dialog)
            case SipStatusCode::SC_404:
            case SipStatusCode::SC_410:
            case SipStatusCode::SC_416:
            case SipStatusCode::SC_482:
            case SipStatusCode::SC_483:
            case SipStatusCode::SC_484:
            case SipStatusCode::SC_485:
            case SipStatusCode::SC_502:
            case SipStatusCode::SC_604:
                // FIXME: Should 502 be excluded if the dialog usage is for subscription? (RFC 6665)
                if (SipFeatures::IsMultipleDialogUsagesRequiredForNonSharedDialog(
                            objMInfo.GetSlotId()) ||
                        ((pDialogState != IMS_NULL) && pDialogState->HasMultipleDialogUsages()))
                {
                    return SIPDState::ACTION_DESTROY_DIALOG;
                }

                if (pDialogBase->GetState() != SIPDState::STATE_CONFIRMED)
                {
                    return SIPDState::ACTION_TRANSIT_STATE;
                }
                break;

                // Impacts on transaction only
            default:
                // In this case, the state will not be changed ...
                if (pDialogBase->GetState() == SIPDState::STATE_CONFIRMED)
                {
                    if (nStatusCode == SipStatusCode::SC_408)
                    {
                        if (nType == TYPE_INVITE)
                        {
                            // It should be verified in the commercial networks...
                            // return SIPDState::ACTION_DESTROY_USAGE;
                        }
                        else if (nType == TYPE_SUBSCRIBE)
                        {
                            // If re-SUBSCRIBE is timed out (Timer F),
                            // it does not terminate the dialog usage.
                            // if (objMInfo.GetMethod().Equals(SipMethod::NOTIFY))
                            //{
                            //    return SIPDState::ACTION_DESTROY_USAGE;
                            //}
                        }
                    }
                    break;
                }

                return SIPDState::ACTION_TRANSIT_STATE;
        }
    }

    return SIPDState::ACTION_IGNORE;
}

PROTECTED
IMS_SINT32 SIPDialogUsage::GetState() const
{
    //---------------------------------------------------------------------------------------------

    if (pDialogBase == IMS_NULL)
    {
        return SIPDState::STATE_INIT;
    }

    return pDialogBase->GetState();
}

PROTECTED VIRTUAL const IMS_CHAR* SIPDialogUsage::TriggerToString(IN IMS_SINT32 nTrigger) const
{
    //---------------------------------------------------------------------------------------------

    switch (nTrigger)
    {
        case SIPDState::TRIGGER_INIT:
            return "TRIGGER_INIT";
        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* SIPDialogUsage::ActionToString(IN IMS_SINT32 nAction)
{
    //---------------------------------------------------------------------------------------------

    switch (nAction)
    {
        case SIPDState::ACTION_IGNORE:
            return "ACTION_IGNORE";
        case SIPDState::ACTION_TRANSIT_STATE:
            return "ACTION_TRANSIT_STATE";
        case SIPDState::ACTION_DESTROY_USAGE:
            return "ACTION_DESTROY_USAGE";
        case SIPDState::ACTION_DESTROY_DIALOG:
            return "ACTION_DESTROY_DIALOG";
        default:
            return "__INVALID__";
    }
}
