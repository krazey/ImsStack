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
#ifndef SIP_DATAGRAM_SOCKET_H_
#define SIP_DATAGRAM_SOCKET_H_

#include "SipSocket.h"

class ISipDatagramSocketListener;

class SipDatagramSocket : public SipSocket
{
public:
    explicit SipDatagramSocket(IN IMS_SINT32 nSlotId);
    ~SipDatagramSocket() override;

public:
    IMS_BOOL Create(IN const IpAddress& objIp, IN IMS_UINT32 nPort = 0,
            IN IMS_BOOL bSecure = IMS_FALSE) override;
    IMS_SINT32 Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen, IN IMS_UINT32 nPort = 0,
            IN const IpAddress& objIp = IpAddress::NONE) override;
    inline void SetListener(IN ISipDatagramSocketListener* piListener)
    {
        m_piListener = piListener;
    }

protected:
    // ISocketListener interface
    void Socket_OnDataReceived(IN ISocket* piSocket) override;
    void Socket_OnSendEnabled(IN ISocket* piSocket) override;

private:
    ISipDatagramSocketListener* m_piListener;
};

#endif
