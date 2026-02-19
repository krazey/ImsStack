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
#include "SipDState.h"
#include "SipDebug.h"
#include "SipDialog.h"
#include "SipError.h"
#include "SipMethod.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipDialog::~SipDialog()
{
#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipDialog(%s)",
            SipDebug::GetCharA1(m_pDialogEx->GetDialogState()->GetCallId().GetStr(), 8, '@'), 0, 0);
#endif
}

PUBLIC
SipDialog& SipDialog::operator=(IN const SipDialog& other)
{
    if (this != &other)
    {
        m_pDialogEx = other.m_pDialogEx;
    }

    return (*this);
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
PUBLIC
SipClientConnection* SipDialog::CreateClientConnection(IN const AString& strMethod)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_EARLY) && (nState != STATE_CONFIRMED))
    {
        // BYE_REQUEST_ON_DIALOG_TERMINATED
        if (strMethod.Equals(SipMethod::ToName(SipMethod::BYE)))
        {
            IMS_TRACE_D("BYE ignores the dialog state(TERMINATED)", 0, 0, 0);
        }
        else
        {
            SipPrivate::SetLastError(SipError::INVALID_STATE);
            return IMS_NULL;
        }
    }

    if (strMethod.GetLength() == 0)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    SipMethod objMethod(strMethod);

    // Check the method validity for the current dialog
    if (!CheckMethodValidity(objMethod))
    {
        SipPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_NULL;
    }

    // Get or create a proper dialog
    RcPtr<SipDialogEx> pNewDialogEx = GetOptimumDialog(objMethod);

    if (pNewDialogEx.IsNull())
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    SipClientConnection* pScc = new SipClientConnection();

    if (pScc == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    if (pScc->InitDialogRequest(objMethod, pNewDialogEx.Get()) != IMS_SUCCESS)
    {
        delete pScc;

        SipPrivate::SetLastError(SipError::TRANSACTION_UNAVAILABLE);
        return IMS_NULL;
    }

    return pScc;
}

/**
 * Returns the state of the SIP dialog.
 *
 * - STATE_INITIALIZED
 *     Internal state where the dialog has been created.
 *     This state is not visible to the user, since the dialog can be fetched earliest
 *     in the STATE_EARLY state.
 *  - STATE_EARLY
 *     Provisional 101 ~ 199 response received or sent (with to-tag).
 *  - STATE_CONFIRMED
 *     Final 2xx response received (or sent) for the original request.
 *     NOTIFY confirming the subscription received (or sent).
 *  - STATE_TERMINATED
 *     No response or error response (3xx ~ 6xx) received (or sent).
 *     If the dialog is terminated with BYE or un-SUBSCRIBE, GetNewClientConnection(...) method
 *     can't be called in this state.
 */
PUBLIC
IMS_SINT32 SipDialog::GetState() const
{
    // Check if the dialog usage is already in TERMINATED state
    if (m_pDialogEx->IsDialogTerminated())
    {
        return STATE_TERMINATED;
    }

    switch (m_pDialogEx->GetState())
    {
        case SipDState::STATE_TERMINATED:
            return STATE_TERMINATED;

        case SipDState::STATE_EARLY:
            return STATE_EARLY;

        case SipDState::STATE_CONFIRMED:
            return STATE_CONFIRMED;

        default:
            return STATE_INIT;
    }
}

/**
 * Compares if the given SipDialog equals or not.
 */
PUBLIC
IMS_BOOL SipDialog::IsSameDialog(IN const SipDialog* pDialog)
{
    if (pDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipDialogState* pDState = m_pDialogEx->GetDialogState();
    SipDialogState* pOtherDState = pDialog->m_pDialogEx->GetDialogState();

    if (!pDState->Equals(pOtherDState))
    {
        return IMS_FALSE;
    }

    // Only check a dialog usage in this time.
    return m_pDialogEx->Equals(pDialog->m_pDialogEx.Get());
}

PRIVATE
IMS_BOOL SipDialog::CheckMethodValidity(IN const SipMethod& objMethod) const
{
    // Case 1)
    //    Condition - subscribe usage & method except for INVITE/SUBSCRIBE/REFER/NOTIFY
    if (!(m_pDialogEx->IsInviteUsage()) && !objMethod.Equals(SipMethod::INVITE) &&
            !objMethod.Equals(SipMethod::SUBSCRIBE) && !objMethod.Equals(SipMethod::REFER) &&
            !objMethod.Equals(SipMethod::NOTIFY))
    {
        IMS_TRACE_D(
                "%s is not allowed in the subscribe usage", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }
    // Case 2)
    //    Condition - invite usage & NOTIFY method
    else if (m_pDialogEx->IsInviteUsage() && objMethod.Equals(SipMethod::NOTIFY))
    {
        IMS_TRACE_D("%s is not allowed in the invite usage", objMethod.ToString().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
SipDialogEx* SipDialog::GetOptimumDialog(IN const SipMethod& objMethod) const
{
    // Case 1)
    //    Condition - invite usage & method except for REFER/SUBSCRIBE
    //    Action - Use an invite usage as it is
    // Case 2)
    //    Condition - subscribe usage & NOTIFY method
    //    Action - Use an subscribe usage as it is
    if ((m_pDialogEx->IsInviteUsage() && !objMethod.Equals(SipMethod::SUBSCRIBE) &&
                !objMethod.Equals(SipMethod::REFER)) ||
            (!m_pDialogEx->IsInviteUsage() && objMethod.Equals(SipMethod::NOTIFY)))
    {
        return m_pDialogEx.Get();
    }
    // Case 3)
    //    Condition - subscribe usage & INVITE method / invite usage & REFER/SUBSCRIBE method
    //    Action - Create an invite/subscribe usage & use it
    //
    // Other Cases
    //
    else
    {
        // After the request is sent, it will be updated by the client transaction
        return SipDialogEx::CreateDialog(m_pDialogEx->GetDialogState(), objMethod);
    }
}
