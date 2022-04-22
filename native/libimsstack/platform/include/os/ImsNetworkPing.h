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
#ifndef IMS_NET_PING_H_
#define IMS_NET_PING_H_

#include "INetworkPing.h"
#include "ISocket.h"
#include "ITimer.h"

class ImsNetworkPing
    : public INetworkPing
    , public ITimerListener
    , public ISocketListener
{
public:
    ImsNetworkPing();
    virtual ~ImsNetworkPing();

    ImsNetworkPing(IN const ImsNetworkPing&) = delete;
    ImsNetworkPing& operator=(IN const ImsNetworkPing&) = delete;

public:
    void Destroy() override;
    IMS_SINT32 Ping(IN const IPAddress& objSrcIp,
            IN const IPAddress& objDstIp, IN IMS_SINT32 nDstPort,
            IN IMS_SINT32 nWaitTime) override;
    void SetListener(IN INetworkPingListener* piListener) override;

protected:
    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // ISocketListener
    void Socket_OnDataReceived(IN ISocket* piSocket) override;
    void Socket_OnSendEnabled(IN ISocket* piSocket) override;
    void Socket_OnConnectionReceived(IN ISocket* piSocket) override;
    void Socket_OnConnected(IN ISocket* piSocket) override;
    void Socket_OnClosed(IN ISocket* piSocket,
            IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN) override;

    void ClearResources();
    IMS_BOOL PrepareResources(IN const IPAddress& objIp, IN IMS_BOOL bTimerRequired);
    void ClearAndNotifyResult(IN IMS_SINT32 nResult);
    void NotifyResult(IN IMS_SINT32 nResult);

    IMS_SINT32 GetState() const;
    void SetState(IN IMS_SINT32 nState);

    static IMS_SINT32 GetRandomPort(IN const IPAddress& objAddr);

private:
    enum
    {
        STATE_IDLE = 0,
        STATE_PENDING = 1
    };

    INetworkPingListener* m_piListener;

    IMS_SINT32 m_nState;
    ITimer* m_piTimer;
    ISocket* m_piSocket;
};

#endif
