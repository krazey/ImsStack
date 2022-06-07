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
#ifndef SIP_DIALOG_H_
#define SIP_DIALOG_H_

#include "SipDialogEx.h"

class ISipHeader;
class SipClientConnection;

/**
 * @brief This class represents one SIP dialog.
 *
 * The SipDialog can be retrieved from a SipConnection object, when it is available
 * (at earliest after provisional 101~199 response). Three SIP requests can open a dialog:
 * INVITE, SUBSCRIBE/NOTIFY and REFER/NOTIFY.
 * An implementation compliant to this specification must support all of the following ways of
 * creating dialogs:
 *   - INVITE-1xx-2xx-ACK will open a dialog. Subsequent SipClientConnection in the same dialog
 *     can be obtained by calling GetNewClientConnection(...) method.
 *     The dialog is terminated when the transaction BYE-200 OK is completed.
 *   - SUBSCRIBE-200 OK (or matching NOTIFY) will open a dialog. Subsequent SipClientConnection
 *     in the same dialog can be obtained by calling GetNewClientConnection(...) method.
 *     The dialog is terminated when a notifier sends a NOTIFY request with a "Subscription-State"
 *     of "terminated" and there are no other subscriptions alive in this dialog.
 *   - REFER-matching NOTIFY will open a dialog. Subsequent SipClientConnection in the same dialog
 *     can be obtained by calling GetNewClientConnection(...) method. The dialog is terminated
 *     when a notifier sends a NOTIFY request with a "Subscription-State" of "terminated"
 *     and there are no other subscriptions alive in this dialog.
 */
class SipDialog
{
public:
    SipDialog() = delete;
    inline explicit SipDialog(IN SipDialogEx* pDialogEx) :
            m_pDialogEx(pDialogEx)
    {
    }
    inline SipDialog(IN const SipDialog& other) :
            m_pDialogEx(other.m_pDialogEx)
    {
    }
    virtual ~SipDialog();

public:
    SipDialog& operator=(IN const SipDialog& other);

public:
    SipClientConnection* CreateClientConnection(IN const AString& strMethod);
    /**
     * @brief Returns the Call-ID component of this dialog.
     */
    inline const AString& GetCallId() const { return m_pDialogEx->GetDialogState()->GetCallId(); }
    /**
     * @brief Returns the local contact address of this dialog.
     */
    inline const ISipHeader* GetContactHeader() const
    {
        return m_pDialogEx->GetDialogState()->GetContactHeader();
    }
    /**
     * @brief Returns the local-tag component of this dialog.
     */
    inline AString GetLocalTag() const { return m_pDialogEx->GetDialogState()->GetLocalTag(); }
    /**
     * @brief Returns the remote-tag component of this dialog.
     */
    inline AString GetRemoteTag() const { return m_pDialogEx->GetDialogState()->GetRemoteTag(); }
    IMS_SINT32 GetState() const;
    IMS_BOOL IsSameDialog(IN const SipDialog* pDialog);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    inline IMS_RESULT SetContactParameter(
            IN const AString& strParameter, IN IMS_SINT32 nOperation = 0 /*(0: ADD, 1: REMOVE)*/)
    {
        return m_pDialogEx->GetDialogState()->SetContactParameter(strParameter, nOperation);
    }
    inline void TerminateDialogUsage() { m_pDialogEx->TerminateDialogUsage(); }
    inline void UpdateDialog(IN SipDialogEx* pDialogEx) { m_pDialogEx = pDialogEx; }

private:
    IMS_BOOL CheckMethodValidity(IN const SipMethod& objMethod) const;
    SipDialogEx* GetOptimumDialog(IN const SipMethod& objMethod) const;

public:
    /// State of SIP dialog
    enum
    {
        STATE_INIT = (-1),
        STATE_TERMINATED = 0,
        STATE_EARLY = 1,
        STATE_CONFIRMED = 2
    };

private:
    RCPtr<SipDialogEx> m_pDialogEx;
};

#endif
