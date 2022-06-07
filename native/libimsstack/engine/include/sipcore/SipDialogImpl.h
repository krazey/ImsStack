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
#ifndef SIP_DIALOG_IMPL_H_
#define SIP_DIALOG_IMPL_H_

#include "ISipDialog.h"

class SipDialog;

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
class SipDialogImpl : public ISipDialog
{
public:
    explicit SipDialogImpl(IN SipDialog* pDialog);
    virtual ~SipDialogImpl();

    SipDialogImpl() = delete;
    SipDialogImpl(IN const SipDialogImpl&) = delete;
    SipDialogImpl& operator=(IN const SipDialogImpl&) = delete;

public:
    // ISipObject interface
    void Destroy() override;
    // ISipDialog interface
    ISipDialog* Clone() const override;
    IMS_BOOL Equals(IN const ISipDialog* piDialog) override;
    AString GetDialogId() override;
    ISipClientConnection* GetNewClientConnection(IN const AString& strMethod) override;
    IMS_SINT32 GetState() const override;
    IMS_BOOL IsSameDialog(IN const ISipConnection* piSc) override;
    AString GetComponent(IN IMS_SINT32 nType) const override;

    // BYE_REQUEST_ON_DIALOG_TERMINATED
    AString GetDialogIdEx() override;
    const ISipHeader* GetContactHeader() const override;
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(IN const AString& strParameter,
            IN IMS_SINT32 nOperation = 0 /*(0: ADD, 1: REMOVE)*/) override;
    void TerminateDialogUsage() override;

    inline SipDialog* GetDialog() const { return m_pDialog; }

private:
    SipDialog* m_pDialog;
};

#endif
