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
#ifndef OS_SOCKET_BASE_H_
#define OS_SOCKET_BASE_H_

#include "ImsSocket.h"

class OsSocketBase
    : public ImsSocket
{
public:
    inline OsSocketBase()
        : ImsSocket()
        , m_eType(TYPE_DGRAM)
        , m_bServerSocket(IMS_FALSE)
        , m_bSocketConnected(IMS_FALSE)
    {}
    inline virtual ~OsSocketBase()
    {}

public:
    // ISocket class
    inline SOCKET_ENTYPE GetSocketType() const override
    { return m_eType; }

    virtual void NotifyDataReceived(IN IMS_SINT32 nErrorCode) = 0;
    virtual void NotifySendEnabled(IN IMS_SINT32 nErrorCode) = 0;
    virtual void NotifyConnectionReceived(IN IMS_SINT32 nErrorCode) = 0;
    virtual void NotifyConnected(IN IMS_SINT32 nErrorCode) = 0;
    virtual void NotifyClosed(IN IMS_SINT32 nErrorCode) = 0;
    virtual void NotifyAcceptCompleted(IN IMS_SOCKET hSocket) = 0;

    inline virtual IMS_SINT32 GetLastError() const
    { return 0; }
    inline virtual IMS_SINT32 GetSocketState() const
    { return OsSocketBase::SOCKET_STATE_CLOSED; }

    inline IMS_BOOL IsServerSocket() const
    { return m_bServerSocket; }
    inline IMS_BOOL IsSocketConnected() const
    { return m_bSocketConnected; }

protected:
    inline void SetServerSocket(IN IMS_BOOL bServerSocket)
    { m_bServerSocket = bServerSocket; }
    inline void SetSocketType(IN SOCKET_ENTYPE eType)
    { m_eType = eType; }
    inline void SetSocketConnected(IN IMS_BOOL bConnected)
    { m_bSocketConnected = bConnected; }

public:
    // Socket state when the data received; only for TCP socket
    enum
    {
        SOCKET_STATE_CLOSED = 0,
        SOCKET_STATE_NOT_READY,
        SOCKET_STATE_DATA_AVAILABLE
    };

private:
    SOCKET_ENTYPE m_eType;
    IMS_BOOL m_bServerSocket;
    IMS_BOOL m_bSocketConnected;
};

#endif
