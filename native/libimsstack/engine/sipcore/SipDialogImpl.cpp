/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipClientConnection.h"
#include "SipClientConnectionImpl.h"
#include "SipDialog.h"
#include "SipDialogImpl.h"
#include "SipError.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipDialogImpl::SipDialogImpl(IN SipDialog* pDialog) :
        m_pDialog(pDialog)
{
}

PUBLIC VIRTUAL SipDialogImpl::~SipDialogImpl()
{
    if (m_pDialog != IMS_NULL)
    {
        delete m_pDialog;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipDialogImpl", 0, 0, 0);
#endif
}

/**
 * Destroys its own resource.
 */
PUBLIC VIRTUAL void SipDialogImpl::Destroy()
{
    delete this;
}

/**
 * Clones the SIP dialog object.
 */
PUBLIC VIRTUAL ISipDialog* SipDialogImpl::Clone() const
{
    if (m_pDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISipDialog is null", 0, 0, 0);
        return IMS_NULL;
    }

    SipDialog* pNewDialog = new SipDialog(*m_pDialog);

    if (pNewDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "Allocating Dialog failed", 0, 0, 0);
        return IMS_NULL;
    }

    SipDialogImpl* pDialogImpl = new SipDialogImpl(pNewDialog);

    if (pDialogImpl == IMS_NULL)
    {
        delete pNewDialog;

        IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        return IMS_NULL;
    }

    return pDialogImpl;
}

/**
 * Compares if the specified ISipDialog equals or not.
 */
PUBLIC VIRTUAL IMS_BOOL SipDialogImpl::Equals(IN const ISipDialog* piDialog)
{
    const SipDialogImpl* pDialogImpl = DYNAMIC_CAST(const SipDialogImpl*, piDialog);

    if (pDialogImpl == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pDialog->IsSameDialog(pDialogImpl->m_pDialog);
}

/**
 * Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.
 */
PUBLIC VIRTUAL AString SipDialogImpl::GetDialogId()
{
    if (m_pDialog->GetState() == SipDialog::STATE_TERMINATED)
    {
        return AString::ConstNull();
    }

    // Call-ID + Local-Tag + Remote-Tag
    return (m_pDialog->GetCallId() + m_pDialog->GetLocalTag() + m_pDialog->GetRemoteTag());
}

/**
 * Returns a new SipClientConnection in this dialog. The returned SipClientConnection will be
 * in STATE_INITIALIZED state. The object is initialized with the given method and default headers.
 *
 * The following headers will be set by the method:
 *     To
 *     From
 *     CSeq
 *     Call-ID
 *     Via
 *     Route
 *     Contact
 *     Max-Forwards
 */
PUBLIC VIRTUAL ISipClientConnection* SipDialogImpl::GetNewClientConnection(
        IN const AString& strMethod)
{
    SipClientConnection* pScc = m_pDialog->CreateClientConnection(strMethod);

    if (pScc == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "Creating a new SIP client connection (%s) failed", strMethod.GetStr(), 0, 0);
        return IMS_NULL;
    }

    SipClientConnectionImpl* pSccImpl = new SipClientConnectionImpl(pScc);

    if (pSccImpl == IMS_NULL)
    {
        delete pScc;
        SipPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Allocating SCCImpl (%s) failed", strMethod.GetStr(), 0, 0);
        return IMS_NULL;
    }

    if (pSccImpl->InitDialogRequest() != IMS_SUCCESS)
    {
        delete pSccImpl;
        SipPrivate::SetLastError(SipError::NO_MEMORY);

        IMS_TRACE_E(0, "Initializing Dialog info. (%s) failed", strMethod.GetStr(), 0, 0);
        return IMS_NULL;
    }

    return pSccImpl;
}

/**
 * Returns the state of the SIP dialog.
 *
 * - STATE_INITIALIZED
 *      Internal state where the dialog has been created.
 *      This state is not visible to the user, since the dialog can be fetched earliest
 *      in the STATE_EARLY state.
 * - STATE_EARLY
 *      Provisional 101 ~ 199 response received or sent (with to-tag).
 * - STATE_CONFIRMED
 *      Final 2xx response received (or sent) for the original request.
 *      NOTIFY confirming the subscription received (or sent).
 * - STATE_TERMINATED
 *      No response or error response (3xx ~ 6xx) received (or sent).
 *      If the dialog is terminated with BYE or un-SUBSCRIBE, GetNewClientConnection(...) method
 *      can't be called in this state.
 */
PUBLIC VIRTUAL IMS_SINT32 SipDialogImpl::GetState() const
{
    switch (m_pDialog->GetState())
    {
        case SipDialog::STATE_TERMINATED:
            return ISipDialog::STATE_TERMINATED;

        case SipDialog::STATE_EARLY:
            return ISipDialog::STATE_EARLY;

        case SipDialog::STATE_CONFIRMED:
            return ISipDialog::STATE_CONFIRMED;

        default:
            return ISipDialog::STATE_INIT;
    }
}

/**
 * Compares if the given ISipConnection belongs to this dialog or not.
 */
PUBLIC VIRTUAL IMS_BOOL SipDialogImpl::IsSameDialog(IN const ISipConnection* piSc)
{
    if (piSc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipDialogImpl* pDialogImpl = DYNAMIC_CAST(SipDialogImpl*, piSc->GetDialog());

    if (pDialogImpl == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pDialog->IsSameDialog(pDialogImpl->m_pDialog);
}

/**
 * Returns the component (call-id, local-tag, remote-tag) of this dialog.
 */
PUBLIC VIRTUAL AString SipDialogImpl::GetComponent(IN IMS_SINT32 nType) const
{
    switch (nType)
    {
        case COMPONENT_CALL_ID:
            return m_pDialog->GetCallId();

        case COMPONENT_LOCAL_TAG:
            return m_pDialog->GetLocalTag();

        case COMPONENT_REMOTE_TAG:
            return m_pDialog->GetRemoteTag();

        default:
            return AString::ConstNull();
    }
}

/**
 * Returns the ID (Call-ID + Local Tag + Remote Tag) of the SIP dialog.
 *
 * BYE_REQUEST_ON_DIALOG_TERMINATED
 */
PUBLIC VIRTUAL AString SipDialogImpl::GetDialogIdEx()
{
    // Call-ID + Local-Tag + Remote-Tag
    return (m_pDialog->GetCallId() + m_pDialog->GetLocalTag() + m_pDialog->GetRemoteTag());
}

/**
 * Returns the local contact address of this dialog.
 */
PUBLIC VIRTUAL const ISipHeader* SipDialogImpl::GetContactHeader() const
{
    return m_pDialog->GetContactHeader();
}

/**
 * Sets the contact header parameter on the early or confirmed state.
 *
 * CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
 */
PUBLIC VIRTUAL IMS_RESULT SipDialogImpl::SetContactParameter(
        IN const AString& strParameter, IN IMS_SINT32 nOperation /*= 0 (0: ADD, 1: REMOVE)*/)
{
    return m_pDialog->SetContactParameter(strParameter, nOperation);
}

/**
 * Terminates the SIP dialog usage explicitly.
 *
 * Use case) remove the dialog usage for "refer" event package subscription.
 */
PUBLIC VIRTUAL void SipDialogImpl::TerminateDialogUsage()
{
    m_pDialog->TerminateDialogUsage();
}
