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
#ifndef OS_SSL_SOCKET_H_
#define OS_SSL_SOCKET_H_

#include "ITimer.h"
#include "OsSocket.h"

class OsSsl;

class OsSslSocket : public OsSocket, public ITimerListener
{
public:
    explicit OsSslSocket(IN SslCertificate* pCertificate);
    virtual ~OsSslSocket();

    OsSslSocket(IN const OsSslSocket&) = delete;
    OsSslSocket& operator=(IN const OsSslSocket&) = delete;

public:
    static void LoadLibrary();

protected:
    // ISocket class
    SOCKET_RESULT Open(IN SOCKET_ENTYPE eType, IN ISocketListener* piListener,
            IN ADDRESS_FAMILY_ENTYPE eAddrFamily = ADDRESS_FAMILY_INET) override;
    SOCKET_RESULT Open(IN SOCKET_ENTYPE eType,
            IN ADDRESS_FAMILY_ENTYPE eAddrFamily = ADDRESS_FAMILY_INET) override;
    IMS_SINT32 Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen) override;
    IMS_SINT32 Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen) override;

    // ImsSocket class
    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

    // OsSocketBase class
    IMS_SINT32 GetSocketState() const override;
    void NotifyConnected(IN IMS_SINT32 nErrorCode) override;
    void NotifyDataReceived(IN IMS_SINT32 nErrorCode) override;

    // OsSocket class
    IMS_BOOL ShutDown(IN IMS_SINT32 nHow = SHUTDOWN_BOTH) override final;

    // ITimerListener class
    void Timer_TimerExpired(IN ITimer* piTimer) override;

private:
    void DoHandshake();
    IMS_BOOL IsSslConnected() const;
    void SetSslState(IN IMS_SINT32 nState);

    static const IMS_CHAR* SslStateToString(IN IMS_SINT32 nState);

private:
    // SSL connection retry counts
    static const IMS_SINT32 SSL_CONNECT_RETRY_COUNT = 20;
    // SSL connection retry interval (ms)
    static const IMS_SINT32 SSL_CONNECT_RETRY_INTERVAL = 500;

    // SSL handshake states
    enum
    {
        SSL_STATE_IDLE = 0,
        SSL_STATE_CONNECTING,
        SSL_STATE_CONNECTED
    };

    IMS_SINT32 m_nSslState;

    IMS_SINT32 m_nSslConnectRetryCount;
    ITimer* m_piSslConnectRetryTimer;

    OsSsl* m_pSsl;
};

#endif
