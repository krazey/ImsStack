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
#ifndef INTERFACE_SOCKET_H_
#define INTERFACE_SOCKET_H_

#include "IpAddress.h"

class ISocketListener;

class ISocket
{
protected:
    virtual ~ISocket() = default;

public:
    /// Return types of socket operation
    enum SOCKET_RESULT
    {
        RESULT_ERROR = (-1),
        RESULT_WOULDBLOCK = 0,
        RESULT_SUCCESS = 1
    };

    /// Type of socket
    enum SOCKET_ENTYPE
    {
        TYPE_STREAM = 1,
        TYPE_DGRAM = 2,
    };

    /// Type of address family
    enum ADDRESS_FAMILY_ENTYPE
    {
        ADDRESS_FAMILY_INET = 1,
        ADDRESS_FAMILY_INET6 = 2
    };

    enum
    {
        MAX_BACKLOG = 3
    };

    /// Type of socket option
    enum
    {
        OPT_BASE = 0,

        /// GET/SET - QoS of IP level : IP_TOS (IPv4), IP_TCLASS (IPv6)
        OPT_IP_QOS,
        /// GET/SET - KeepAlive configuration of TCP level
        /// Number of unacknowledged probes (count)
        OPT_TCP_KEEPCNT,
        /// Interval between the last data packet sent & the first keepalive probe (seconds)
        OPT_TCP_KEEPIDLE,
        /// Interval between subsequential keepalive probes (seconds)
        OPT_TCP_KEEPINTVL,
        /// The maximum segment size for outgoing TCP packets
        OPT_TCP_MAXSEG,
        /// GET/SET - set buffer size for sending/receiving the data
        OPT_RCVBUF,
        OPT_SNDBUF,
        /// SET
        OPT_REUSEADDR,
        /// SET
        OPT_LINGER,
        /// SET
        OPT_KEEPALIVE,
        /// SET
        OPT_UDP_ENCAP,

        /// IMS extensions
        /// SET - set the shutdown method for TCP connection
        OPT_SHUTDOWN,

        OPT_MAX
    };

    /// Parameter for specific socket option
    enum
    {
        /// OPT_UDP_ENCAP
        OPT_UDP_ENCAP_ESPINUDP_NON_IKE = 1,
        OPT_UDP_ENCAP_ESPINUDP = 2,
        OPT_UDP_ENCAP_L2TPINUDP = 3
    };

    /// Type of close reason
    enum
    {
        CLOSE_REASON_UNKNOWN = 0,
        CLOSE_REASON_USER_ACTION,
        CLOSE_REASON_REMOTE_ACTION,
        CLOSE_REASON_DATA_CONNECTION_LOST,
        CLOSE_REASON_INTERNAL_ERROR
    };

public:
    /**
     * @brief Returns a socket identifier of the lower layer.
     *        If the platform is the Linux, it returns a file descriptor of this socket.
     *        If the socket is not opened or already closed, it returns INVALID_SOCKET.
     *
     * @return A socket identifier or INVALID_SOCKET
     */
    virtual IMS_SINT32 GetSocketId() const = 0;

    /**
     * @brief Returns the socket type of this socket.
     *
     * @return A socket type (SOCKET_ENTYPE)
     */
    virtual SOCKET_ENTYPE GetSocketType() const = 0;

    virtual SOCKET_RESULT Open(IN SOCKET_ENTYPE eType, IN ISocketListener* piListener,
            IN ADDRESS_FAMILY_ENTYPE eAddressFamily = ADDRESS_FAMILY_INET) = 0;

    virtual SOCKET_RESULT Open(IN SOCKET_ENTYPE eType,
            IN ADDRESS_FAMILY_ENTYPE eAddressFamily = ADDRESS_FAMILY_INET) = 0;

    virtual void SetListener(IN ISocketListener* piListener) = 0;

    virtual SOCKET_RESULT Close() = 0;

    virtual ISocket* Accept() = 0;

    virtual SOCKET_RESULT Bind(IN const IpAddress& objSocketAddress, IN IMS_UINT32 nSocketPort) = 0;

    virtual SOCKET_RESULT Connect(IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort) = 0;

    virtual SOCKET_RESULT Listen(IN IMS_SINT32 nBackLog = MAX_BACKLOG) = 0;

    virtual IMS_SINT32 Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen) = 0;

    virtual IMS_SINT32 Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen) = 0;

    virtual IMS_SINT32 ReceiveFrom(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            OUT IpAddress& objHostAddress, OUT IMS_UINT32& nHostPort) = 0;

    virtual IMS_SINT32 SendTo(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
            IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort) = 0;

    virtual SOCKET_RESULT GetPeerName(OUT IpAddress& objPeerAddress, OUT IMS_UINT32& nPeerPort) = 0;

    virtual SOCKET_RESULT GetSockName(
            OUT IpAddress& objSocketAddress, OUT IMS_UINT32& nSocketPort) = 0;

    virtual IMS_BOOL Equals(IN const ISocket* piSocket) = 0;

    virtual IMS_SINT32 GetOption(IN IMS_SINT32 nOption) = 0;

    virtual IMS_BOOL SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue) = 0;

    virtual IMS_BOOL IsClosedOrBeingClosed() const = 0;
};

class ISocketListener
{
protected:
    virtual ~ISocketListener() = default;

public:
    virtual void Socket_OnDataReceived(IN ISocket* piSocket) = 0;

    virtual void Socket_OnSendEnabled(IN ISocket* piSocket) = 0;

    virtual void Socket_OnConnectionReceived(IN ISocket* piSocket) = 0;

    virtual void Socket_OnConnected(IN ISocket* piSocket) = 0;

    virtual void Socket_OnClosed(
            IN ISocket* piSocket, IN IMS_SINT32 nReason = ISocket::CLOSE_REASON_UNKNOWN) = 0;
};

#endif
