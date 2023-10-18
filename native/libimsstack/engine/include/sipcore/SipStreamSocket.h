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
#ifndef SIP_STREAM_SOCKET_H_
#define SIP_STREAM_SOCKET_H_

#include "ITimer.h"

#include "private/SipConfig.h"

#include "SipMessageFraming.h"
#include "SipSocket.h"

class ISipStreamSocketListener;

class SipStreamSocket : public SipSocket, public ITimerListener
{
public:
    explicit SipStreamSocket(IN IMS_SINT32 nSlotId);
    SipStreamSocket(IN IMS_SINT32 nSlotId, IN ISocket* piSocket);
    virtual ~SipStreamSocket();

    SipStreamSocket(IN const SipStreamSocket&) = delete;
    SipStreamSocket& operator=(IN const SipStreamSocket&) = delete;

public:
    void ApplyIpSec(IN ISocket* piAcceptedSocket = IMS_NULL) override;
    IMS_BOOL Connect() override;
    IMS_BOOL Create(IN const IpAddress& objIp, IN IMS_UINT32 nPort = 0,
            IN IMS_BOOL bSecure = IMS_FALSE) override;
    void GetSockName(OUT IpAddress& objIp, OUT IMS_UINT32& nPort) override;
    IMS_SINT32 Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen, IN IMS_UINT32 nPort = 0,
            IN const IpAddress& objIp = IpAddress::NONE) override;
    void NotifyForceClosed() override;

    void DisableKeepAlive();
    inline IMS_BOOL IsKeepAliveTimerActive() const { return (m_piKeepAliveTimer != IMS_NULL); }
    IMS_BOOL IsKeepAlivePermanent() const;
    inline IMS_BOOL IsSecureSocket() const { return m_bSecure; }
    void ReuseSocket();
    void SetConfigForSipKeepAlive(IN IMS_BOOL bSipKeepAlive);
    void SetFarEnd(IN const IpAddress& objIp, IN IMS_UINT32 nPort);
    void SetKeepAlivePolicy(IN IMS_SINT32 nPolicy);
    inline void SetListener(IN ISipStreamSocketListener* piListener) { m_piListener = piListener; }

protected:
    // ITimerListener interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    void Socket_OnDataReceived(IN ISocket* piSocket) override;
    void Socket_OnSendEnabled(IN ISocket* piSocket) override;
    void Socket_OnConnected(IN ISocket* piSocket) override;
    void Socket_OnClosed(
            IN ISocket* piSocket, IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN) override;

private:
    IMS_RESULT StartTxTimer(IN IMS_SINT32 nDuration);
    void StopTxTimer();
    IMS_RESULT StartKeepAliveTimer(IN IMS_SINT32 nInactiveInterval);
    void StopKeepAliveTimer();
    void UpdateLastAliveTime();

private:
    enum
    {
        CONNECTION_TIMER_VALUE = 10000
    };
    enum
    {
        WOULDBLOCK_TIMER_VALUE = 33000
    };
    enum
    {
        KEEPALIVE_TIMER_VALUE = 60000
    };

    SipConfig::TcpTimerValues m_objTcpTimerValues;
    SipMessageFraming m_objMsgFraming;

    IMS_BOOL m_bSecure;
    IMS_BOOL m_bSipKeepAliveConfigured;

    ITimer* m_piTxTimer;
    IMS_UINT32 m_nLastAliveTime;
    ITimer* m_piKeepAliveTimer;
    ISipStreamSocketListener* m_piListener;
};

#endif
