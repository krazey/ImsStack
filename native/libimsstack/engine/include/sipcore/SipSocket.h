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
#ifndef SIP_SOCKET_H_
#define SIP_SOCKET_H_

#include "ImsSlot.h"

#include "ISocket.h"
#include "SipSocketAddress.h"

class INetworkConnection;
class ISipKeepAliveListener;
class ISipSocketListener;

class SipSocket : public ImsSlot, public ISocketListener
{
public:
    explicit SipSocket(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType = SipSocketAddress::SOCKET_UDP);
    virtual ~SipSocket();

    SipSocket(IN const SipSocket&) = delete;
    SipSocket& operator=(IN const SipSocket&) = delete;

public:
    inline virtual SipSocket* Accept() { return IMS_NULL; }
    inline virtual void ApplyIpSec(IN ISocket* piAcceptedSocket = IMS_NULL)
    {
        ApplyIpSecInternal(m_objSockAddr.GetSocketAddress(), IMS_NULL, piAcceptedSocket);
    }
    inline virtual IMS_BOOL Connect() { return IMS_TRUE; }
    virtual IMS_BOOL Create(
            IN const IpAddress& objIp, IN IMS_UINT32 nPort = 0, IN IMS_BOOL bSecure = IMS_FALSE);
    inline virtual IMS_BOOL Equals(IN const SipSocketAddress& objSockAddr)
    {
        return m_objSockAddr.Equals(objSockAddr);
    }
    inline virtual IMS_BOOL Equals(IN const SipSocket& objSocket)
    {
        return m_objSockAddr.Equals(objSocket.m_objSockAddr);
    }
    virtual void GetSockName(OUT IpAddress& objIp, OUT IMS_UINT32& nPort);
    inline virtual IMS_SINT32 Send(IN const IMS_BYTE* /*pBuffer*/, IN IMS_SINT32 /*nBuffLen*/,
            IN IMS_UINT32 nPort = 0, IN const IpAddress& objIp = IpAddress::NONE)
    {
        (void)nPort;
        (void)objIp;
        return ISocket::RESULT_ERROR;
    }
    virtual void NotifyForceClosed();

    void GetPeerName(OUT IpAddress& objIp, OUT IMS_UINT32& nPort);
    IMS_SINT32 RemoveListener(IN const ISipSocketListener* piListener);
    inline void SetKeepAliveListener(IN ISipKeepAliveListener* piKeepAliveListener)
    {
        m_piKeepAliveListener = piKeepAliveListener;
    }
    void SetListener(IN ISipSocketListener* piListener);
    /** Same as ISocket::SetOption(...) */
    void SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue);

    inline IMS_SINT32 GetState() const { return m_nState; }
    inline IMS_SINT32 GetType() const { return m_objSockAddr.GetType(); }
    inline IMS_BOOL IsSocketForcinlyClosed() const { return m_bForcinglyClosed; }
    IMS_BOOL IsClosedOrBeingClosed() const
    {
        return m_piSocket != IMS_NULL ? m_piSocket->IsClosedOrBeingClosed() : IMS_TRUE;
    }

protected:
    // ISocketListener interface
    inline void Socket_OnDataReceived(IN ISocket* /*piSocket*/) override {}
    void Socket_OnSendEnabled(IN ISocket* piSocket) override;
    inline void Socket_OnConnectionReceived(IN ISocket* /*piSocket*/) override {}
    void Socket_OnConnected(IN ISocket* piSocket) override;
    void Socket_OnClosed(
            IN ISocket* piSocket, IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN) override;

    void ApplyIpSecInternal(IN const SocketAddress& objLocal,
            IN const SocketAddress* pRemote = IMS_NULL, IN ISocket* piAcceptedSocket = IMS_NULL);
    void CloseSocket();
    void NotifyPongReceived();
    inline void SetForcinglyClosed(IN IMS_BOOL bClosed) { m_bForcinglyClosed = bClosed; }
    void SetSocketOptionForTcpMaxSeg(
            IN INetworkConnection* piConnection, IN const IpAddress& objLocalIp);
    void SetSocketOptions(IN const IpAddress& objLocalIp, IN IMS_UINT32 nLocalPort);
    void SetState(IN IMS_SINT32 nState);

    static void SetSocketOption(IN IMS_SINT32 nSlotId, IN ISocket* piSocket,
            IN const IpAddress& objLocalIp, IN IMS_UINT32 nLocalPort, IN IMS_SINT32 nConfigItem,
            IN IMS_SINT32 nSocketOption, IN const IMS_CHAR* pszOptionName);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        TYPE_NONE = 0,
        TYPE_UDP,
        TYPE_TCP,
        TYPE_TCP_CLIENT,
        TYPE_TCP_CLIENT_OTHER
    };

    enum
    {
        STATE_CREATED = 0,
        STATE_INITIALIZED,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_CLOSING,
        STATE_TERMINATED
    };

    enum
    {
        ERROR_NO_ERROR = 0,
        ERROR_WOULDBLOCK_TIMEDOUT,
        ERROR_CONNECTION_TIMEDOUT,
        ERROR_SEND_FAILED,
        ERROR_CLOSED,
        ERROR_CONNECT_FAILED,
        ERROR_DATA_CONNECTION_LOST
    };

protected:
    // Index to the destination transport tuple
    // In case of UDP/TCP server socket, this information is set to the local socket information.
    // In case of TCP client socket, this information is set to the peer socket information.
    //    1) Connected by UA : Set at the calling time of Connect(...).
    //    2) Accepted by UA : Set at the calling time of Accept(...) using GetPeerName(...).
    SipSocketAddress m_objSockAddr;

    ISocket* m_piSocket;
    ImsList<ISipSocketListener*> m_objListeners;
    ISipKeepAliveListener* m_piKeepAliveListener;

private:
    IMS_SINT32 m_nState;
    IMS_BOOL m_bForcinglyClosed;
};

#endif
