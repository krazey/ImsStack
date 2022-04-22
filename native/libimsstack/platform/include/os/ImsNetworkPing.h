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
    : public INetPing
    , public ITimerListener
    , public INetSocketListener
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
    void SetListener(IN INetPingListener* piListener) override;

protected:
    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // INetSocketListener
    void Socket_DataReceived(IN INetSocket* piSocket) override;
    void Socket_SendEnabled(IN INetSocket* piSocket) override;
    void Socket_ConnectionReceived(IN INetSocket* piSocket) override;
    void Socket_Connected(IN INetSocket* piSocket) override;
    void Socket_Closed(IN INetSocket* piSocket,
            IN IMS_SINT32 nReason = INetSocket::CLOSE_REASON_UNKNOWN) override;

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

    INetPingListener* m_piListener;

    IMS_SINT32 m_nState;
    ITimer* m_piTimer;
    INetSocket* m_piSocket;
};

#endif
