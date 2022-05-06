/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100526  hwangoo.park@             Created
    </table>

    Description
     This class represents one SIP dialog. The SIPDialog can be retrieved from a SIPConnection
    object, when it is available (at earliest after provisional 101~199 response).
    Three SIP requests can open a dialog: INVITE, SUBSCRIBE/NOTIFY and REFER/NOTIFY.
    An implementation compliant to this specification must support all of the following ways of
    creating dialogs:
    - INVITE-1xx-2xx-ACK will open a dialog. Subsequent SIPClientConnection in the same dialog
    can be obtained by calling GetNewClientConnection(...) method.
    The dialog is terminated when the transaction BYE-200 OK is completed.
    - SUBSCRIBE-200 OK (or matching NOTIFY) will open a dialog. Subsequent SIPClientConnection
    in the same dialog can be obtained by calling GetNewClientConnection(...) method.
    The dialog is terminated when a notifier sends a NOTIFY request with a "Subscription-State"
    of "terminated" and there are no other subscriptions alive in this dialog.
    - REFER-matching NOTIFY will open a dialog. Subsequent SIPClientConnection in the same dialog
    can be obtained by calling GetNewClientConnection(...) method. The dialog is terminated
    when a notifier sends a NOTIFY request with a "Subscription-State" of "terminated"
    and there are no other subscriptions alive in this dialog.
*/

#ifndef _SIP_DIALOG_H_
#define _SIP_DIALOG_H_

#include "SIPDialogEx.h"

class ISipHeader;
class SIPClientConnection;

class SIPDialog
{
public:
    explicit SIPDialog(IN SIPDialogEx* pDialogEx_);
    SIPDialog(IN CONST SIPDialog& objRHS);
    virtual ~SIPDialog();

private:
    SIPDialog();

public:
    SIPDialog& operator=(IN CONST SIPDialog& objRHS);

public:
    SIPClientConnection* CreateClientConnection(IN CONST AString& strMethod);
    const AString& GetCallId() const;
    const ISipHeader* GetContactHeader() const;
    AString GetLocalTag() const;
    AString GetRemoteTag() const;
    IMS_SINT32 GetState() const;
    IMS_BOOL IsSameDialog(IN CONST SIPDialog* pDialog);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(
            IN CONST AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    void TerminateDialogUsage();
    void UpdateDialog(IN SIPDialogEx* pDialogEx);

private:
    IMS_BOOL CheckMethodValidity(IN CONST SipMethod& objMethod) const;
    SIPDialogEx* GetOptimumDialog(IN CONST SipMethod& objMethod) const;

public:
    // State of SIP dialog
    enum
    {
        STATE_INIT = (-1),
        STATE_TERMINATED = 0,
        STATE_EARLY = 1,
        STATE_CONFIRMED = 2
    };

private:
    RCPtr<SIPDialogEx> pDialogEx;
};

#endif  // _SIP_DIALOG_H_
