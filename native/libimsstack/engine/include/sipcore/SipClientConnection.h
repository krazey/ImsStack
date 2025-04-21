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
#ifndef SIP_CLIENT_CONNECTION_H_
#define SIP_CLIENT_CONNECTION_H_

#include "Credential.h"
#include "ITimer.h"

#include "ISipClientTransactionStateListener.h"
#include "ISipClientTransmissionListener.h"
#include "Sip.h"
#include "SipClientTransactionState.h"
#include "SipConnection.h"

class IOnSipClientConnectionListener;
class ISipAckPackage;
class ISipGenericChallenge;
class SipAuHelper;
class SipClientTransmissionProxy;
class SipConnectionNotifier;

class SipClientConnection :
        public SipConnection,
        public ISipClientTransactionStateListener,
        public ISipClientTransmissionListener,
        public ITimerListener
{
public:
    SipClientConnection();
    explicit SipClientConnection(IN const AString& strTargetUri);
    explicit SipClientConnection(IN SipClientTransactionState* pCtState);
    virtual ~SipClientConnection();

    SipClientConnection(IN const SipClientConnection&) = delete;
    SipClientConnection& operator=(IN const SipClientConnection&) = delete;

public:
    // IConnection interface
    void Close() override;

    // ISipConnection interface
    IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) override;
    AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex = 0) override;
    ImsList<AString> GetHeaders(IN const AString& strName) override;
    const SipMethod& GetMethod() const override;
    const AString& GetReasonPhrase() const override;
    const AString& GetRequestUri() const override;
    IMS_SINT32 GetStatusCode() const override;
    IMS_RESULT RemoveHeader(IN const AString& strName) override;
    IMS_RESULT Send() override;
    IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strValue) override;
    const ByteArray& GetContent() const override;
    IMS_RESULT SetContent(IN const ByteArray& objContent) override;
    IMS_SINT32 GetHeaderCount(IN const AString& strName) const override;
    void SetSipProfile(IN SipProfile* pProfile) override;

    // ISipClientConnection interface
    IMS_RESULT InitAck();
    SipClientConnection* InitCancel();
    IMS_RESULT InitRequest(IN const AString& strMethod, IN SipConnectionNotifier* pScn);
    IMS_RESULT Receive(IN IMS_SLONG nTimeout = 0);
    IMS_RESULT SetCredentials(IN ImsList<Credential>& objCredentials);
    IMS_RESULT SetCredentials(IN const Credential& objCredential);
    inline void SetListener(IN IOnSipClientConnectionListener* piListener)
    {
        m_piListener = piListener;
    }
    IMS_RESULT SetRequestUri(IN const AString& strUri);
    ISipGenericChallenge* GetAuthenticationChallenge(IN IMS_SINT32 nIndex = 0) const;
    ISipAckPackage* GrabAck();
    IMS_RESULT InitResubmissionRequest();
    void RemoveAllChallenges();
    void RemoveAllCredentials();
    IMS_RESULT SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge);
    void SetExtensionTokenForViaBranch(IN const AString& strToken);
    void SetImplicitRouteHeader(IN const AString& strRouteHeader);
    void SetTransportTuple(IN const IpAddress& objIp, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFc = Sip::PORT_UNSPECIFIED,
            IN IMS_SINT32 nTransportExt = Sip::TRANSPORT_EXT_ANY);

    IMS_RESULT InitDialogRequest(IN const SipMethod& objMethod, IN SipDialogEx* pDialogEx);
    IMS_RESULT SendWithCredentials();

protected:
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

private:
    // ISipClientTransactionStateListener interface
    void ClientTransactionState_ForkedResponseReceived(
            IN SipClientTransactionState* pCtState) override;
    void ClientTransactionState_ResponseReceived(IN ::SipMessage* pSipMsg) override;
    void Transport_NotifyError(IN IMS_SINT32 nCode, IN const AString& strMessage) override;
    // SIP_TRANSPORT_ERROR_REPORT_ON_TXN
    IMS_BOOL IsTransportErrorReportRequired(
            IN IMS_SINT32 nCode, IN const AString& strMessage) const override;

    // ISipClientTransmissionListener class
    void ClientTransmission_NotifyError(IN IMS_SINT32 nCode, IN const AString& strMessage) override;
    void ClientTransmission_TransmissionCompleted() override;

    // ITimerListener interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;
    void StartTcpConnectionMonitoringTimer();
    void StopTcpConnectionMonitoringTimer();
    void SetState(IN IMS_SINT32 nState);

    static IMS_BOOL IsTransportErrorCode104(IN const AString& strMessage);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_INITIALIZED,
        STATE_PROCEEDING,
        STATE_COMPLETED,
        STATE_UNAUTHORIZED,
        STATE_TERMINATED
    };

private:
    enum
    {
        AMSG_SIP_RESPONSE_RECEIVED = AMSG_USER,
        AMSG_SIP_FORKED_RESPONSE_RECEIVED
    };

    static const AString ANONYMOUS_URI;

    IMS_SINT32 m_nState;
    IMS_BOOL m_bAckSent;
    IMS_BOOL m_bResubmissionRequestInitialized;
    // Request-URI should be equal to the INVITE request
    //    : CANCEL & ACK to non-2xx response
    AString m_strTargetUri;
    RcPtr<SipClientTransactionState> m_pCtState;
    ImsList<sipcore::SipMessage*> m_objResponseMessages;
    SipAuHelper* m_pAuHelper;
    IOnSipClientConnectionListener* m_piListener;
    // UDP_FALLBACK
    SipClientTransmissionProxy* m_pTransmissionProxy;
    // A timer to handle an exceptional condition between sending SIP request from UE and
    // closing TCP connection from network.
    ITimer* m_piTcpConnectionMonitoringTimer;
};

#endif
