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

#ifndef _SIP_DIALOG_INVITE_USAGE_H_
#define _SIP_DIALOG_INVITE_USAGE_H_

#include "SIPDialogUsage.h"

class SIPDialogInviteUsage : public SIPDialogUsage
{
public:
    SIPDialogInviteUsage(IN SIPDialogBase* pDialog_);
    SIPDialogInviteUsage(IN CONST SIPDialogInviteUsage& objRHS);
    virtual ~SIPDialogInviteUsage();

private:
    SIPDialogInviteUsage();
    SIPDialogInviteUsage& operator=(IN CONST SIPDialogInviteUsage& objRHS);

public:
    // SIPDialogUsage class
    virtual SIPDialogUsage* Clone() const;
    virtual IMS_BOOL CompareTo(IN CONST SIPMessageInfo& objMInfo) const;
    virtual IMS_SINT32 UpdateUsageDetails(IN CONST SIPMessageInfo& objMInfo);

    static IMS_SINT32 GetNextState(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger);

protected:
    virtual IMS_SINT32 GetActionNTrigger(
            IN CONST SIPMessageInfo& objMInfo, OUT IMS_SINT32& nTrigger);
    virtual IMS_BOOL IsUsageTerminated(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger) const;

    virtual const IMS_CHAR* TriggerToString(IN IMS_SINT32 nTrigger) const;

private:
    // INVITE usage: TRIGGER events for dialog state transition
    enum
    {
        TRIGGER_INIT = SIPDState::TRIGGER_INIT,

        TRIGGER_1XX,
        TRIGGER_2XX,
        TRIGGER_NON_2XX,
        TRIGGER_2XX_BYE,
        TRIGGER_BYE,

        TRIGGER_MAX
    };

private:
    static const IMS_SINT32 STATE_TABLE[SIPDState::STATE_MAX][TRIGGER_MAX];
};

#endif  // _SIP_DIALOG_INVITE_USAGE_H_
