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

#include "ServiceMemory.h"
#include "SipPrivate.h"
#include "SipDialog.h"
#include "SipClientConnection.h"
#include "SipClientConnectionImpl.h"
#include "SipDialogImpl.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPDialogImpl::SIPDialogImpl(IN SIPDialog* pDialog_) :
        pDialog(pDialog_)
{
}

PUBLIC VIRTUAL SIPDialogImpl::~SIPDialogImpl()
{
    if (pDialog != IMS_NULL)
    {
        delete pDialog;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPDialogImpl", 0, 0, 0);
#endif
}

/*
 Destroys its own resource.

Remarks

*/
PUBLIC VIRTUAL void SIPDialogImpl::Destroy()
{
    //---------------------------------------------------------------------------------------------

    delete this;
}

/*
 Clones the SIP dialog object.

Remarks

*/
PUBLIC VIRTUAL ISipDialog* SIPDialogImpl::Clone() const
{
    //---------------------------------------------------------------------------------------------

    if (pDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "NULL - Dialog", 0, 0, 0);
        return IMS_NULL;
    }

    SIPDialog* pNewDialog = new SIPDialog(*pDialog);

    if (pNewDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating Dialog failed", 0, 0, 0);
        return IMS_NULL;
    }

    SIPDialogImpl* pDialogImpl = new SIPDialogImpl(pNewDialog);

    if (pDialogImpl == IMS_NULL)
    {
        delete pNewDialog;

        IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pDialogImpl;
}

/*
 Compares if the specified ISipDialog equals or not.

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPDialogImpl::Equals(IN CONST ISipDialog* piDialog)
{
    const SIPDialogImpl* pDialogImpl = DYNAMIC_CAST(const SIPDialogImpl*, piDialog);

    //---------------------------------------------------------------------------------------------

    if (pDialogImpl == IMS_NULL)
        return IMS_FALSE;

    return pDialog->IsSameDialog(pDialogImpl->pDialog);
}

/*
 Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.

Remarks

*/
PUBLIC VIRTUAL AString SIPDialogImpl::GetDialogId()
{
    //---------------------------------------------------------------------------------------------

    if (pDialog->GetState() == SIPDialog::STATE_TERMINATED)
        return AString::ConstNull();

    // Call-ID + Local-Tag + Remote-Tag
    return (pDialog->GetCallId() + pDialog->GetLocalTag() + pDialog->GetRemoteTag());
}

/*
 Returns a new SIPClientConnection in this dialog. The returned SIPClientConnection will be
in STATE_INITIALIZED state. The object is initialized with the given method and default headers.

Remarks
 The following headers will be set by the method:
    To
    From
    CSeq
    Call-ID
    Via
    Route
    Contact
    Max-Forwards
*/
PUBLIC VIRTUAL ISipClientConnection* SIPDialogImpl::GetNewClientConnection(
        IN CONST AString& strMethod)
{
    SIPClientConnection* pSCC = pDialog->CreateClientConnection(strMethod);

    //---------------------------------------------------------------------------------------------

    if (pSCC == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Creating a new SIP client connection (%s) failed", strMethod.GetStr(), 0, 0);
        return IMS_NULL;
    }

    SIPClientConnectionImpl* pSCCImpl = new SIPClientConnectionImpl(pSCC);

    if (pSCCImpl == IMS_NULL)
    {
        delete pSCC;
        SIPPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Allocating SCCImpl (%s) failed", strMethod.GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (pSCCImpl->InitDialogRequest() != IMS_SUCCESS)
    {
        delete pSCCImpl;
        SIPPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Initializing Dialog info. (%s) failed", strMethod.GetStr(), 0, 0);
        return IMS_NULL;
    }

    return pSCCImpl;
}

/*
 Returns the state of the SIP dialog.

Remarks
    - STATE_INITIALIZED
        Internal state where the dialog has been created.
        This state is not visible to the user, since the dialog can be fetched earliest
        in the STATE_EARLY state.
    - STATE_EARLY
        Provisional 101 ~ 199 response received or sent (with to-tag).
    - STATE_CONFIRMED
        Final 2xx response received (or sent) for the original request.
        NOTIFY confirming the subscription received (or sent).
    - STATE_TERMINATED
        No response or error response (3xx ~ 6xx) received (or sent).
        If the dialog is terminated with BYE or un-SUBSCRIBE, GetNewClientConnection(...) method
        can't be called in this state.

*/
PUBLIC VIRTUAL IMS_SINT32 SIPDialogImpl::GetState() const
{
    //---------------------------------------------------------------------------------------------

    switch (pDialog->GetState())
    {
        case SIPDialog::STATE_TERMINATED:
            return ISipDialog::STATE_TERMINATED;

        case SIPDialog::STATE_EARLY:
            return ISipDialog::STATE_EARLY;

        case SIPDialog::STATE_CONFIRMED:
            return ISipDialog::STATE_CONFIRMED;

        default:
            return ISipDialog::STATE_INIT;
    }
}

/*
 Compares if the given ISipConnection belongs to this dialog or not.

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPDialogImpl::IsSameDialog(IN CONST ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    if (piSC == IMS_NULL)
        return IMS_FALSE;

    SIPDialogImpl* pDialogImpl = DYNAMIC_CAST(SIPDialogImpl*, piSC->GetDialog());

    if (pDialogImpl == IMS_NULL)
        return IMS_FALSE;

    return pDialog->IsSameDialog(pDialogImpl->pDialog);
}

/*
 Returns the component (call-id, local-tag, remote-tag) of this dialog.

Remarks

*/
PUBLIC VIRTUAL AString SIPDialogImpl::GetComponent(IN IMS_SINT32 nType) const
{
    //---------------------------------------------------------------------------------------------

    switch (nType)
    {
        case COMPONENT_CALL_ID:
            return pDialog->GetCallId();

        case COMPONENT_LOCAL_TAG:
            return pDialog->GetLocalTag();

        case COMPONENT_REMOTE_TAG:
            return pDialog->GetRemoteTag();

        default:
            return AString::ConstNull();
    }
}

/*
 Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.

Remarks
 BYE_REQUEST_ON_DIALOG_TERMINATED
*/
PUBLIC VIRTUAL AString SIPDialogImpl::GetDialogIdEx()
{
    //---------------------------------------------------------------------------------------------

    // Call-ID + Local-Tag + Remote-Tag
    return (pDialog->GetCallId() + pDialog->GetLocalTag() + pDialog->GetRemoteTag());
}

/*
 Returns the local contact address of this dialog.

Remarks

*/
PUBLIC VIRTUAL const ISipHeader* SIPDialogImpl::GetContactHeader() const
{
    //---------------------------------------------------------------------------------------------

    return pDialog->GetContactHeader();
}

/*
 Sets the contact header parameter on the early or confirmed state.

Remarks
 CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST

*/
PUBLIC VIRTUAL IMS_RESULT SIPDialogImpl::SetContactParameter(
        IN CONST AString& strParameter, IN IMS_SINT32 nOperation /* = 0 (0: ADD, 1: REMOVE) */)
{
    //---------------------------------------------------------------------------------------------

    return pDialog->SetContactParameter(strParameter, nOperation);
}

/*
 Terminates the SIP dialog usage explicitly.

Remarks
 Use case) remove the dialog usage for "refer" event package subscription.

*/
PUBLIC VIRTUAL void SIPDialogImpl::TerminateDialogUsage()
{
    pDialog->TerminateDialogUsage();
}
