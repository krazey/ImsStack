/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100309  hwangoo.park@             Created
    </table>

    Description
     This class defines a SUBSCRIBE dialog usage.
        - Created by:
        1) 200 class responses to SUBSCRIBE
        2) 200 class responses to REFER
        3) NOTIFY requests
        - Destroyed by:
        1) 200 class responses to NOTIFY-terminated
        2) NOTIFY or refresh-SUBSCRIBE request timeout
        3) Certain failure responses to NOTIFY or SUBSCRIBE
        4) Expiration without refresh if network issues prevent the terminal NOTIFY from arriving
        5) Anything that destroys a dialog and all its usages.
*/

#ifndef _SIP_DIALOG_SUBSCRIBE_USAGE_H_
#define _SIP_DIALOG_SUBSCRIBE_USAGE_H_

#include "SipDialogUsage.h"

////
// ONLY OUTGOING SUBSCRIPTION WILL BE HANDLED IN THIS MOMENT... 2010/03/17
////

class SIPDialogSubscribeUsage : public SIPDialogUsage
{
public:
    SIPDialogSubscribeUsage(IN SIPDialogBase* pDialog_);
    SIPDialogSubscribeUsage(IN CONST SIPDialogSubscribeUsage& objRHS);
    virtual ~SIPDialogSubscribeUsage();

private:
    SIPDialogSubscribeUsage();
    SIPDialogSubscribeUsage& operator=(IN CONST SIPDialogSubscribeUsage& objRHS);

public:
    // SIPDialogUsage class
    virtual IMS_BOOL InitDialogUsage(IN CONST SIPMessageInfo& objMInfo);
    virtual SIPDialogUsage* Clone() const;
    virtual IMS_BOOL CompareTo(IN CONST SIPMessageInfo& objMInfo) const;
    virtual IMS_BOOL Equals(IN SIPDialogUsage* pDUsage) const;
    virtual AString ToString() const;
    virtual IMS_SINT32 UpdateUsageDetails(IN CONST SIPMessageInfo& objMInfo);

    IMS_BOOL InitDialogUsage(IN CONST SipMethod& objMethod);

    static IMS_SINT32 GetNextState(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger);

protected:
    virtual IMS_SINT32 GetActionNTrigger(
            IN CONST SIPMessageInfo& objMInfo, OUT IMS_SINT32& nTrigger);
    virtual IMS_BOOL IsUsageTerminated(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger) const;

    virtual const IMS_CHAR* TriggerToString(IN IMS_SINT32 nTrigger) const;

public:
    // State of Subscription-State
    enum
    {
        SUB_STATE_INIT = 0,

        SUB_STATE_PENDING,
        SUB_STATE_ACTIVE,
        SUB_STATE_TERMINATED
    };

    // Method of initial subscription (explicit / implicit <-> SUBSCRIBE / REFER)
    enum
    {
        METHOD_SUBSCRIBE,
        METHOD_REFER
    };

private:
    // SUBSCRIBE usage: TRIGGER events for dialog state transition
    enum
    {
        TRIGGER_INIT = SIPDState::TRIGGER_INIT,

        TRIGGER_1XX,
        TRIGGER_2XX,
        TRIGGER_NON_2XX,
        TRIGGER_XXX_NOTIFY_TERMINATED,
        TRIGGER_NOTIFY,

        TRIGGER_MAX
    };

    static const IMS_SINT32 STATE_TABLE[SIPDState::STATE_MAX][TRIGGER_MAX];

    // 4 Shall we need to check Expires header in SUBSCRIBE request or expires parameter
    // 4 in NOTIFY request
    //  For Subscription-State header info. of NOTIFY request
    IMS_SINT32 nSubState;

    // Method type: SUBSCRIBE or REFER
    IMS_SINT32 nMethod;

    // Event package name & id which is a parameter of Event header
    AString strEvent;
    AString strEventId;

    // For CSeq number of the NOTIFY request with "terminated"
    // when the subsequent NOTIFY request is received
    // before sending the response of the previous NOTIFY request...
    IMS_UINT32 nCSeqForNOTIFYWithTerminated;
};

#endif  // _SIP_DIALOG_SUBSCRIBE_USAGE_H_
