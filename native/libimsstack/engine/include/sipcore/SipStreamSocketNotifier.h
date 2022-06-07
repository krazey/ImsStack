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
#ifndef SIP_STREAM_SOCKET_NOTIFIER_H_
#define SIP_STREAM_SOCKET_NOTIFIER_H_

#include "private/SipConfig.h"

#include "SipSocket.h"

class ISipStreamSocketListener;

class SipStreamSocketNotifier : public SipSocket
{
public:
    explicit SipStreamSocketNotifier(IN IMS_SINT32 nSlotId);
    virtual ~SipStreamSocketNotifier();

    SipStreamSocketNotifier(IN const SipStreamSocketNotifier&) = delete;
    SipStreamSocketNotifier& operator=(IN const SipStreamSocketNotifier&) = delete;

public:
    SipSocket* Accept() override;
    IMS_BOOL Create(IN const IPAddress& objIp, IN IMS_UINT32 nPort = 0,
            IN IMS_BOOL bSecure = IMS_FALSE) override;
    inline void SetListener(IN ISipStreamSocketListener* piListener) { m_piListener = piListener; }

public:
    void Socket_OnConnectionReceived(IN ISocket* piSocket) override;
    void Socket_OnClosed(
            IN ISocket* piSocket, IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN) override;

private:
    ISipStreamSocketListener* m_piListener;
};

#endif
