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
#ifndef SIP_CLIENT_CONNECTION_IMPL_H_
#define SIP_CLIENT_CONNECTION_IMPL_H_

#include "IOnSipClientConnectionListener.h"
#include "IOnSipErrorListener.h"
#include "ISipClientConnection.h"

class SipClientConnection;
class SipDialogImpl;

class SipClientConnectionImpl :
        public ISipClientConnection,
        public IOnSipErrorListener,
        public IOnSipClientConnectionListener
{
public:
    explicit SipClientConnectionImpl(IN SipClientConnection* pScc);
    virtual ~SipClientConnectionImpl();

    SipClientConnectionImpl() = delete;
    SipClientConnectionImpl(IN const SipClientConnectionImpl&) = delete;
    SipClientConnectionImpl& operator=(IN const SipClientConnectionImpl&) = delete;

public:
    IMS_RESULT InitDialogRequest();

private:
    // IConnection interface implementation
    void Close() override;

    // ISipConnection interface implementation
    IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) override;
    ISipDialog* GetDialog() const override;
    AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex = 0) override;
    ImsList<AString> GetHeaders(IN const AString& strName) override;
    const SipMethod& GetMethod() const override;
    const AString& GetReasonPhrase() const override;
    const AString& GetRequestUri() const override;
    IMS_SINT32 GetStatusCode() const override;
    IMS_RESULT RemoveHeader(IN const AString& strName) override;
    IMS_RESULT Send() override;
    inline void SetErrorListener(IN ISipErrorListener* piListener) override
    {
        m_piErrorListener = piListener;
    }
    IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) override;
    const ByteArray& GetContent() const override;
    IMS_RESULT SetContent(IN const ByteArray& objContent) override;
    IMS_SINT32 GetHeaderCount(IN const AString& strName) const override;
    ISipMessage* GetMessage() const override;
    IMS_SINT32 GetSlotId() const override;
    void SetSipProfile(IN SipProfile* pProfile) override;
    void SetTransactionTimerValues(IN const SipTimerValues& objTimerValues) override;

    // ISipClientConnection interface implementation
    IMS_RESULT InitAck() override;
    ISipClientConnection* InitCancel() override;
    IMS_RESULT InitRequest(IN const AString& strMethod, IN ISipConnectionNotifier* piScn) override;
    IMS_RESULT Receive(IN IMS_SLONG nTimeout = 0) override;
    IMS_RESULT SetCredentials(IN ImsList<Credential>& objCredentials) override;
    IMS_RESULT SetCredentials(IN const Credential& objCredential) override;
    inline void SetListener(IN ISipClientConnectionListener* piListener) override
    {
        m_piListener = piListener;
    }
    IMS_RESULT SetRequestUri(IN const AString& strUri) override;
    ISipGenericChallenge* GetAuthenticationChallenge(IN IMS_SINT32 nIndex = 0) const override;
    ISipAckPackage* GrabAck() override;
    IMS_RESULT InitResubmissionRequest() override;
    void RemoveAllChallenges() override;
    void RemoveAllCredentials() override;
    IMS_RESULT SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge) override;
    void SetExtensionTokenForViaBranch(IN const AString& strToken) override;
    void SetImplicitRouteHeader(IN const AString& strRouteHeader) override;
    void SetTransportTuple(IN const IpAddress& objIp, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFc = Sip::PORT_UNSPECIFIED,
            IN IMS_SINT32 nTransportExt = Sip::TRANSPORT_EXT_ANY) override;

    // IOnSipErrorListener interface
    void OnError_NotifyError(
            IN SipConnection* pSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // IOnSipClientConnectionListener interface
    void OnClientConnection_NotifyResponse(IN SipClientConnection* pScc) override;
    void OnClientConnection_NotifyForkedResponse(
            IN SipClientConnection* pScc, IN SipClientConnection* pForkedScc) override;

private:
    SipClientConnection* m_pScc;
    SipDialogImpl* m_pDialogImpl;
    ISipErrorListener* m_piErrorListener;
    ISipClientConnectionListener* m_piListener;
};

#endif
