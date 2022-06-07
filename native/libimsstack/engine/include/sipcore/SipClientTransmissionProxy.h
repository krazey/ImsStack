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
#ifndef SIP_CLIENT_TRANSMISSION_PROXY_H_
#define SIP_CLIENT_TRANSMISSION_PROXY_H_

// TCP active connection MUST be created at the start time of raw SIP message transmission.
#include "EngineActivity.h"
#include "ISipSocketListener.h"

class ISipClientTransmissionListener;
class SipClientTransactionState;
class SipTimerValues;

class SipClientTransmissionProxy : public EngineActivity, public ISipSocketListener
{
public:
    SipClientTransmissionProxy();
    virtual ~SipClientTransmissionProxy();

    SipClientTransmissionProxy(IN const SipClientTransmissionProxy&) = delete;
    SipClientTransmissionProxy& operator=(IN const SipClientTransmissionProxy&) = delete;

public:
    // EngineActivity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    void Abort();
    IMS_RESULT Send();
    IMS_RESULT SendWithCredentials();
    inline void SetListener(IN ISipClientTransmissionListener* piListener)
    {
        m_piListener = piListener;
    }
    inline void SetTimerValues(IN SipTimerValues* pTimerValues) { m_pTimerValues = pTimerValues; }
    inline void SetTransactionState(IN SipClientTransactionState* pCtState)
    {
        m_pCtState = pCtState;
    }

private:
    // ISipSocketListener class
    void Socket_NotifyError(IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode) override;
    void Socket_SendEnabled(IN SipSocket* pSocket) override;

    void DestroyStreamSocket();
    IMS_BOOL IsUdpFallbackRequired() const;
    IMS_BOOL IsUdpFallbackSupported() const;
    void NotifyTransportError(IN IMS_SINT32 nErrorCode);
    IMS_RESULT PrepareStreamSocket();
    IMS_RESULT SendMessage();
    void SendPendingMessage();

public:
    enum
    {
        RESULT_NOK = (-1),
        RESULT_OK = 0,
        RESULT_PENDING = 1
    };

private:
    enum
    {
        AMSG_SEND_MESSAGE = AMSG_USER,
        AMSG_NOTIFY_TRANSPORT_ERROR
    };

    SipClientTransactionState* m_pCtState;
    ISipClientTransmissionListener* m_piListener;
    SipTimerValues* m_pTimerValues;
    IMS_BOOL m_bIsResubmittedRequest;
    // TCP socket
    SipSocket* m_pSocket;
};

#endif
