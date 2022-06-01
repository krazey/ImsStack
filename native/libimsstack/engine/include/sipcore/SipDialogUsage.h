/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100309  hwangoo.park@             Created
    </table>

    Description
     This class defines a dialog usage for SIP. Several methods in the SIP can create
    an association between endpoints known as a dialog. Some of these methods can
    also create a different, but related, association within an existing dialog.
    These multiple associations, or dialog usages, require carefully coordinated processing
    as they have independent life-cycles, but share common dialog state.
    There are two dialog usages.
    - A dialog initiated with an INVITE request has an invite usage.
    - A dialog initiated with a SUBSCRIBE request has a subscribe usage.
    - A dialog initiated with a REFER request has a subscribe usage.
*/

#ifndef _SIP_DIALOG_USAGE_H_
#define _SIP_DIALOG_USAGE_H_

#include "SipDialogBase.h"

class SIPDialogUsage
{
public:
    SIPDialogUsage(IN SIPDialogBase* pDialogBase_);
    SIPDialogUsage(IN IMS_SINT32 nType_, IN SIPDialogBase* pDialogBase_);
    SIPDialogUsage(IN CONST SIPDialogUsage& objRHS);
    virtual ~SIPDialogUsage();

private:
    SIPDialogUsage();
    SIPDialogUsage& operator=(IN CONST SIPDialogUsage& objRHS);

public:
    virtual IMS_BOOL InitDialogUsage(IN CONST SIPMessageInfo& objMInfo);
    virtual SIPDialogUsage* Clone() const;
    virtual IMS_BOOL CompareTo(IN CONST SIPMessageInfo& objMInfo) const;
    virtual IMS_BOOL Equals(IN SIPDialogUsage* pDUsage) const;
    virtual AString ToString() const;
    virtual IMS_SINT32 UpdateUsageDetails(IN CONST SIPMessageInfo& objMInfo);

    inline IMS_SINT32 GetType() const { return nType; }

protected:
    virtual IMS_SINT32 GetActionNTrigger(
            IN CONST SIPMessageInfo& objMInfo, OUT IMS_SINT32& nTrigger);
    virtual IMS_BOOL IsUsageTerminated(IN IMS_SINT32 nState, IN IMS_SINT32 nTrigger) const;

    IMS_SINT32 GetActionForResponse(IN CONST SIPMessageInfo& objMInfo);
    IMS_SINT32 GetState() const;

    virtual const IMS_CHAR* TriggerToString(IN IMS_SINT32 nTrigger) const;
    static const IMS_CHAR* ActionToString(IN IMS_SINT32 nAction);

public:
    enum
    {
        TYPE_EPHEMERAL = 0,
        TYPE_INVITE,
        TYPE_SUBSCRIBE
    };

private:
    // Usage type: invite / subscribe / register(pseudo)
    IMS_SINT32 nType;

    SIPDialogBase* pDialogBase;
};

#endif  // _SIP_DIALOG_USAGE_H_
