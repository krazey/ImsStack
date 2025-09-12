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
#ifndef SIP_CONNECTION_FACTORY_H_
#define SIP_CONNECTION_FACTORY_H_

#include "EngineActivity.h"
#include "ISipConnectionFactory.h"
#include "util/ICancellableMethod.h"
#include "util/IDialogMethod.h"

class Service;

class SipConnectionFactory :
        public EngineActivity,
        public IDialogMethod,
        public ICancellableMethod,
        public ISipConnectionFactory
{
public:
    explicit SipConnectionFactory(IN Service* pService);
    SipConnectionFactory(IN Service* pService, IN ISipServerConnection* piSsc);
    ~SipConnectionFactory() override;

    SipConnectionFactory(IN const SipConnectionFactory&) = delete;
    SipConnectionFactory& operator=(IN const SipConnectionFactory&) = delete;

public:
    // EngineActivity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // IDialogMethod class
    IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSsc) const override;
    IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // ICancellableMethod class
    IMS_BOOL Cancellable_Compare(IN ISipServerConnection* piSscCancel) const override;
    IMS_BOOL Cancellable_NotifyRequest(IN ISipServerConnection* piSscCancel) override;

    // IMethod class
    void Destroy() override;
    inline void SetMessageMediator(IN IMessageMediator* /*piMediator*/) override {}
    // ISipConnectionFactory class
    ISipClientConnection* CreateClientConnection(IN const SipMethod& objMethod,
            IN const SipAddress* pFrom, IN const SipAddress* pTo) override;
    ISipClientConnection* CreateClientConnection(
            IN ISipDialog* piDialog, IN const SipMethod& objMethod) override;
    IMS_BOOL CreateResponse(IN_OUT ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
            IN const AString& strPhrase = AString::ConstNull()) override;
    inline ISipServerConnection* GetNewServerConnection() override { return m_piInitialSsc; }
    void SetDialog(IN ISipDialog* piDialog) override;
    inline void SetListener(IN ISipConnectionFactoryListener* piListener) override
    {
        m_piListener = piListener;
    }
    inline void SetSscForCancel(IN ISipServerConnection* piSsc) override { m_piInviteSsc = piSsc; }

private:
    enum
    {
        AMSG_SSC_FOR_MID_DIALOG_RECEIVED = AMSG_USER
    };

    Service* m_pService;
    ISipDialog* m_piDialog;
    ISipConnectionFactoryListener* m_piListener;
    // It is only maintained for a new incoming request
    ISipServerConnection* m_piInitialSsc;
    ISipServerConnection* m_piInviteSsc;
};

#endif
