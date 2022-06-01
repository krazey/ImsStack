/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100309  hwangoo.park@             Created
    </table>

    Description
     This class defines an INVITE dialog usage.
        - Created by:
        1) Non-100 provisional responses to INVITE
        2) 200 response to INVITE
        - Destroyed by:
        1) 200 responses to BYE
        2) Certain failure responses to INVITE, UPDATE, PRACK, INFO or BYE
        3) Anything that destroys a dialog and all its usages
*/

#include "ServiceMemory.h"
#include "SipPrivate.h"
#include "SipDialogInviteUsage.h"

__IMS_TRACE_TAG_SIP__;

// clang-format off
PRIVATE GLOBAL const IMS_SINT32
SIPDialogInviteUsage::STATE_TABLE[SIPDState::STATE_MAX][SIPDialogInviteUsage::TRIGGER_MAX] =
{
    // STATE_INIT
    {
        SIPDState::STATE_INIT,                 // TRIGGER_INIT
        SIPDState::STATE_EARLY,                // TRIGGER_1XX
        SIPDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SIPDState::STATE_INIT,                 // TRIGGER_NON_2XX
        SIPDState::STATE_INIT,                 // TRIGGER_2XX_BYE
        SIPDState::STATE_INIT                  // TRIGGER_BYE
    },
    // STATE_TERMINATED
    {
        SIPDState::STATE_TERMINATED,           // TRIGGER_INIT
        SIPDState::STATE_TERMINATED,           // TRIGGER_1XX
        SIPDState::STATE_TERMINATED,           // TRIGGER_2XX
        SIPDState::STATE_TERMINATED,           // TRIGGER_NON_2XX
        SIPDState::STATE_TERMINATED,           // TRIGGER_2XX_BYE
        SIPDState::STATE_TERMINATED            // TRIGGER_BYE
    },
    // STATE_EARLY
    {
        SIPDState::STATE_EARLY,                // TRIGGER_INIT
        SIPDState::STATE_EARLY,                // TRIGGER_1XX
        SIPDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SIPDState::STATE_TERMINATED,           // TRIGGER_NON_2XX
        SIPDState::STATE_TERMINATED,           // TRIGGER_2XX_BYE
        SIPDState::STATE_EARLY                 // TRIGGER_BYE
    },
    // STATE_CONFIRMED
    {
        SIPDState::STATE_CONFIRMED,            // TRIGGER_INIT
        SIPDState::STATE_CONFIRMED,            // TRIGGER_1XX
        SIPDState::STATE_CONFIRMED,            // TRIGGER_2XX
        SIPDState::STATE_CONFIRMED,            // TRIGGER_NON_2XX
        SIPDState::STATE_TERMINATED,           // TRIGGER_2XX_BYE
        SIPDState::STATE_CONFIRMED             // TRIGGER_BYE
    }
};
// clang-format on

PUBLIC
SIPDialogInviteUsage::SIPDialogInviteUsage(IN SIPDialogBase* pDialog_) :
        SIPDialogUsage(SIPDialogUsage::TYPE_INVITE, pDialog_)
{
}

PUBLIC
SIPDialogInviteUsage::SIPDialogInviteUsage(IN CONST SIPDialogInviteUsage& objRHS) :
        SIPDialogUsage(objRHS)
{
}

PUBLIC VIRTUAL SIPDialogInviteUsage::~SIPDialogInviteUsage() {}

PUBLIC VIRTUAL SIPDialogUsage* SIPDialogInviteUsage::Clone() const
{
    //---------------------------------------------------------------------------------------------

    return new SIPDialogInviteUsage(*this);
}

PUBLIC VIRTUAL IMS_BOOL SIPDialogInviteUsage::CompareTo(IN CONST SIPMessageInfo& objMInfo) const
{
    const SipMethod& objMethod = objMInfo.GetMethod();

    //---------------------------------------------------------------------------------------------

    // CASE : subscribe usage & register usage
    if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER) ||
            objMethod.Equals(SipMethod::NOTIFY) || objMethod.Equals(SipMethod::REGISTER))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_SINT32 SIPDialogInviteUsage::UpdateUsageDetails(
        IN CONST SIPMessageInfo& objMInfo)
{
    //---------------------------------------------------------------------------------------------

    // Update the shared dialog states
    return SIPDialogUsage::UpdateUsageDetails(objMInfo);
}

PUBLIC GLOBAL IMS_SINT32 SIPDialogInviteUsage::GetNextState(
        IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger)
{
    //---------------------------------------------------------------------------------------------

    if ((nTrigger < TRIGGER_INIT) || (nTrigger >= TRIGGER_MAX))
        return SIPDState::STATE_MAX;

    return STATE_TABLE[nState][nTrigger];
}

PROTECTED VIRTUAL IMS_SINT32 SIPDialogInviteUsage::GetActionNTrigger(
        IN CONST SIPMessageInfo& objMInfo, OUT IMS_SINT32& nTrigger)
{
    SipMessage* pstMessage = objMInfo.GetMessage();

    IMS_SINT32 nAction = SIPDState::ACTION_TRANSIT_STATE;

    //---------------------------------------------------------------------------------------------

    nTrigger = TRIGGER_INIT;

    if (SIPStack::IsRequestMessage(pstMessage))
    {
        if (objMInfo.GetMethod().Equals(SipMethod::BYE))
            nTrigger = TRIGGER_BYE;
    }
    else
    {
        const SipMethod& objMethod = objMInfo.GetMethod();

        nAction = GetActionForResponse(objMInfo);

        IMS_SINT32 nStatusCode = SIPStack::GetStatusCode(pstMessage);

        if (SipStatusCode::IsProvisional(nStatusCode))
        {
            nTrigger = TRIGGER_1XX;
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (objMethod.Equals(SipMethod::BYE))
            {
                nAction = SIPDState::ACTION_TRANSIT_STATE;
                nTrigger = TRIGGER_2XX_BYE;
            }
            else
            {
                if (!objMethod.Equals(SipMethod::INVITE))
                {
                    nAction = SIPDState::ACTION_IGNORE;
                }

                nTrigger = TRIGGER_2XX;
            }
        }
        else
        {
            if (GetState() == SIPDState::STATE_EARLY)
            {
                if (objMethod.Equals(SipMethod::INVITE))
                {
                    nTrigger = TRIGGER_NON_2XX;
                }
            }
            else
            {
                // 4 How to handle in case of BYE ???
                if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::BYE))
                {
                    nAction = SIPDState::ACTION_IGNORE;
                }

                nTrigger = TRIGGER_NON_2XX;
            }
        }
    }

    return nAction;
}

PROTECTED VIRTUAL IMS_BOOL SIPDialogInviteUsage::IsUsageTerminated(
        IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger) const
{
    //---------------------------------------------------------------------------------------------

    if (GetNextState(nState, nTrigger) == SIPDState::STATE_TERMINATED)
        return IMS_TRUE;

    return IMS_FALSE;
}

PROTECTED VIRTUAL const IMS_CHAR* SIPDialogInviteUsage::TriggerToString(
        IN IMS_SINT32 nTrigger) const
{
    //---------------------------------------------------------------------------------------------

    switch (nTrigger)
    {
        case TRIGGER_1XX:
            return "TRIGGER_1XX";
        case TRIGGER_2XX:
            return "TRIGGER_2XX";
        case TRIGGER_NON_2XX:
            return "TRIGGER_NON_2XX";
        case TRIGGER_2XX_BYE:
            return "TRIGGER_2XX_BYE";
        case TRIGGER_BYE:
            return "TRIGGER_BYE";
        default:
            return SIPDialogUsage::TriggerToString(nTrigger);
    }
}
