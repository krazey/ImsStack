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
#ifndef SIP_CONNECTION_NOTIFIER_IMPL_H_
#define SIP_CONNECTION_NOTIFIER_IMPL_H_

#include "IOnSipConnectionNotifierErrorListener.h"
#include "IOnSipServerConnectionListener.h"
#include "ISipConnectionNotifier.h"

class ISipServerConnectionListener;
class SipConnectionNotifier;

class SipConnectionNotifierImpl :
        public ISipConnectionNotifier,
        public IOnSipServerConnectionListener,
        public IOnSipConnectionNotifierErrorListener
{
public:
    explicit SipConnectionNotifierImpl(IN SipConnectionNotifier* pScn);
    virtual ~SipConnectionNotifierImpl();

    SipConnectionNotifierImpl(IN const SipConnectionNotifierImpl&) = delete;
    SipConnectionNotifierImpl& operator=(IN const SipConnectionNotifierImpl&) = delete;

public:
    inline SipConnectionNotifier* GetConnectionNotifier() const { return m_pScn; };

private:
    // IConnection interface
    void Close() override;

    // ISipConnectionNotifier interface
    ISipServerConnection* AcceptAndOpen() override;
    const IpAddress& GetLocalAddress() const override;
    IMS_SINT32 GetLocalPort() const override;
    inline void SetListener(IN ISipServerConnectionListener* piListener) override
    {
        m_piListener = piListener;
    }
    ISipServerConnection* AcceptAndOpen(OUT ISipDialog*& piOrigDialog) override;
    AString GetContactAddress() const override;
    SipProfile* GetSipProfile() const override;
    IMS_SINT32 GetSlotId() const override;
    IMS_BOOL IsTransportResourceReserved(IN IMS_SINT32 nType = TRANSPORT_ALL) const override;
    IMS_RESULT ReserveTransportResource(IN const IpAddress& objIp, IN IMS_SINT32 nPortS,
            IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl) override;
    IMS_RESULT RestoreTransportResource(
            IN IMS_SINT32 nType, IN const IpAddress& objPeerIp, IN IMS_SINT32 nPeerPort) override;
    void SetFromAndContact(IN const AString& strFrom, IN const AString& strDisplayName,
            IN const AString& strUserInfo) override;
    // MULTI_REG_SIP_PROFILE
    void SetSipProfile(IN SipProfile* pProfile) override;
    void UpdatePortFlowControl(IN IMS_SINT32 nPort) override;
    void UpdatePortUc(IN IMS_SINT32 nPort) override;
    void AddErrorListener(IN ISipConnectionNotifierErrorListener* piListener) override;
    void RemoveErrorListener(IN ISipConnectionNotifierErrorListener* piListener) override;

    // IOnSipServerConnectionListener interface
    void OnServerConnection_NotifyRequest(IN SipConnectionNotifier* pScn) override;
    void OnServerConnection_NotifyForkedRequest(IN SipConnectionNotifier* pScn) override;

    // IOnSipConnectionNotifierErrorListener interface
    void OnConnectionNotifierError_NotifyError(IN SipConnectionNotifier* pScn, IN IMS_SINT32 nCode,
            IN const AString& strMessage) override;

private:
    SipConnectionNotifier* m_pScn;
    ISipServerConnectionListener* m_piListener;
    ImsList<ISipConnectionNotifierErrorListener*> m_objErrorListeners;
};

#endif
