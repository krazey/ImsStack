/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  hwangoo.park@             Created
    </table>

    Description
     This class defines a base class for SIP dialog class.
*/

#ifndef _SIP_DIALOG_BASE_H_
#define _SIP_DIALOG_BASE_H_

#include "SipDialogState.h"

class SIPDialogBase : public RCObject
{
public:
    explicit SIPDialogBase(IN SIPDialogState* pDState_);
    SIPDialogBase(IN CONST SIPDialogBase& objRHS);
    virtual ~SIPDialogBase();

private:
    SIPDialogBase();

public:
    SIPDialogBase& operator=(IN CONST SIPDialogBase& objRHS);

public:
    virtual IMS_BOOL OnInit() = 0;
    virtual void OnTerminated() = 0;
    virtual IMS_SINT32 OnUpdateDialogDetails(IN CONST SIPMessageInfo& objMInfo,
            IN IMS_SINT32 nUsage, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger) = 0;

    SIPDialogState* GetDialogState() const;
    IMS_SINT32 GetState() const;

    static IMS_BOOL IsDialogCreatable(IN CONST SipMethod& objMethod);

private:
    // Shared dialog information among all dialog usages
    RCPtr<SIPDialogState> pDState;
};

#endif  // _SIP_DIALOG_BASE_H_
