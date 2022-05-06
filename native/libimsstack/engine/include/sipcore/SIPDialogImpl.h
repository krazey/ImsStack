/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description
     This class represents one SIP dialog. The SIPDialog can be retrieved from a SIPConnection
    object, when it is available (at earliest after provisional 101~199 response).
    Three SIP requests can open a dialog: INVITE, SUBSCRIBE/NOTIFY and REFER/NOTIFY.
    An implementation compliant to this specification must support all of the following ways
    of creating dialogs:
    - INVITE-1xx-2xx-ACK will open a dialog. Subsequent SIPClientConnection in the same dialog
    can be obtained by calling GetNewClientConnection(...) method. The dialog is terminated
    when the transaction BYE-200 OK is completed.
    - SUBSCRIBE-200 OK (or matching NOTIFY) will open a dialog. Subsequent SIPClientConnection
    in the same dialog can be obtained by calling GetNewClientConnection(...) method.
    The dialog is terminated when a notifier sends a NOTIFY request with a "Subscription-State"
    of "terminated" and there are no other subscriptions alive in this dialog.
    - REFER-matching NOTIFY will open a dialog. Subsequent SIPClientConnection in the same dialog
    can be obtained by calling GetNewClientConnection(...) method. The dialog is terminated
    when a notifier sends a NOTIFY request with a "Subscription-State" of "terminated"
    and there are no other subscriptions alive in this dialog.
*/

#ifndef _SIP_DIALOG_IMPL_H_
#define _SIP_DIALOG_IMPL_H_

#include "ISipDialog.h"

class SIPDialog;

class SIPDialogImpl : public ISipDialog
{
public:
    explicit SIPDialogImpl(IN SIPDialog* pDialog_);
    virtual ~SIPDialogImpl();

private:
    SIPDialogImpl();
    SIPDialogImpl(IN CONST SIPDialogImpl& objRHS);
    SIPDialogImpl& operator=(IN CONST SIPDialogImpl& objRHS);

public:
    // ISipObject interface
    virtual void Destroy();
    // ISipDialog interface
    virtual ISipDialog* Clone() const;
    virtual IMS_BOOL Equals(IN CONST ISipDialog* piDialog);
    virtual AString GetDialogId();
    virtual ISipClientConnection* GetNewClientConnection(IN CONST AString& strMethod);
    virtual IMS_SINT32 GetState() const;
    virtual IMS_BOOL IsSameDialog(IN CONST ISipConnection* piSC);
    virtual AString GetComponent(IN IMS_SINT32 nType) const;

    //// IMS Extensions
    // BYE_REQUEST_ON_DIALOG_TERMINATED
    virtual AString GetDialogIdEx();
    virtual const ISipHeader* GetContactHeader() const;
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    virtual IMS_RESULT SetContactParameter(
            IN CONST AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    virtual void TerminateDialogUsage();

    inline SIPDialog* GetDialog() const { return pDialog; }

private:
    SIPDialog* pDialog;
};

#endif  // _SIP_DIALOG_IMPL_H_
