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
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
// SOL_UDP
#include <linux/udp.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "ImsMessageDef.h"
#include "ImsNetworkConnectionState.h"
#include "INetworkIpSec.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "network/OsSocket.h"
#include "network/OsSocketDef.h"
#include "network/OsSocketMsg.h"
#include "network/OsSocketService.h"

__IMS_TRACE_TAG_IPL__;

#if defined(_DEBUG)
#define IMSSOCKET_DEBUG(ec) OsSocket::OutputDebugString(ec, __IMS_FUNC__, __IMS_LINE__)
#else
#define IMSSOCKET_DEBUG(ec)
#endif

#define IMS_MAX_RETRY_COUNT_ON_BIND_FAILED 3

LOCAL
void osSocket_GetAddressNPort(IN const struct sockaddr* pstSockAddr, IN IMS_UINT32 nAddrLen,
        OUT IpAddress& objIpAddr, OUT IMS_UINT32& nPort)
{
    if (pstSockAddr == IMS_NULL)
    {
        return;
    }

    // IPv4
    if (pstSockAddr->sa_family == AF_INET)
    {
        const struct sockaddr_in* pstAddr4 =
                reinterpret_cast<const struct sockaddr_in*>(pstSockAddr);
        IMS_CHAR acIpv4[32] = {
                0,
        };
        const IMS_CHAR* pszIpv4 = inet_ntop(AF_INET,
                static_cast<const void*>(&(pstAddr4->sin_addr.s_addr)), acIpv4, sizeof(acIpv4));

        nPort = ntohs(pstAddr4->sin_port);
        objIpAddr.Parse(pszIpv4);

        IMS_TRACE_D("getnameinfo() - %s : %d",
                OsUtil::GetInstance()->IsDebugMode() ? acIpv4 : "xxx", nPort, 0);
    }
    // IPv6
    else if (pstSockAddr->sa_family == AF_INET6)
    {
        const struct sockaddr_in6* pstAddr6 =
                reinterpret_cast<const struct sockaddr_in6*>(pstSockAddr);
        IMS_CHAR acIpv6[64] = {
                0,
        };
        const IMS_CHAR* pszIpv6 = inet_ntop(AF_INET6,
                static_cast<const void*>(&(pstAddr6->sin6_addr.s6_addr)), acIpv6, sizeof(acIpv6));

        nPort = ntohs(pstAddr6->sin6_port);
        objIpAddr.Parse(pszIpv6);

        IMS_TRACE_D("getnameinfo() - [%s] : %d",
                OsUtil::GetInstance()->IsDebugMode() ? acIpv6 : "xxx", nPort, 0);
    }
    else
    {
        // Invalid IP address size
        IMS_TRACE_E(0, "Invalid IP address size (%d)", nAddrLen, 0, 0);
        return;
    }
}

// NETWORK_INTERFACE_FOR_SOCKET
LOCAL
void osSocket_SetNetworkForSocket(
        IN IMS_SINT32 nSlotId, IN IMS_SOCKET hSocket, IN IMS_CONNECTION hConnection)
{
    if ((hSocket == INVALID_SOCKET) || (hConnection == 0))
    {
        return;
    }

    ImsNetworkConnectionState* pState = ImsNetworkConnectionState::GetInstance();
    const ImsNetworkConnection* pNc = pState->LookupHandle(hConnection);

    if (pNc == IMS_NULL)
    {
        return;
    }

    if (PlatformContext::GetInstance()->GetSystem()->BindSocket(
                pNc->GetApnType(), hSocket, nSlotId) != 1)
    {
        IMS_TRACE_D("BindSocket is failed - slotId=%d, apnType=%d, sockFd=%d.", nSlotId,
                pNc->GetApnType(), hSocket);
    }
}

PRIVATE GLOBAL IMS_UINT16 OsSocket::s_nInternalSocketId = 1;

PUBLIC
OsSocket::OsSocket() :
        OsSocketBase(),
        m_hSocket(INVALID_SOCKET),
        m_eAddressFamily(ADDRESS_FAMILY_INET),
        m_nSocketEvent(0),
        m_piListener(IMS_NULL),
        m_nCloseReason(CLOSE_REASON_UNKNOWN),
        m_nOptionForShutdown(-1),
        m_objSocketAddress(IpAddress::NONE),
        m_nSocketPort(0),
        m_nInternalSocketId(0)
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

    // FIX_TIMING_ISSUE
    m_nInternalSocketId = s_nInternalSocketId++;

    if (s_nInternalSocketId == 0xFFFE)
    {
        s_nInternalSocketId = 1;
    }
}

PUBLIC VIRTUAL OsSocket::~OsSocket()
{
    IMS_TRACE_D("Destructor :: OsSocket (%d)", m_nInternalSocketId, 0, 0);

    if (m_hSocket != INVALID_SOCKET)
    {
        IMS_TRACE_I("Socket :: Close(%d-%d)", m_hSocket, GetInternalSocketId(), 0);

        if (GetSocketType() == TYPE_STREAM)
        {
            // Disable send()
            OsSocket::ShutDown((m_nOptionForShutdown < 0) ? SHUTDOWN_BOTH : m_nOptionForShutdown);
        }

        OsSocketService::GetInstance()->KillSocket(m_hSocket);
        close(m_hSocket);
        UnbindSocketFromIpSecTransform(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
}

PUBLIC VIRTUAL IMS_SINT32 OsSocket::GetLastError() const
{
    if (m_hSocket == INVALID_SOCKET)
    {
        return 0;
    }

    IMS_SINT32 nErrorCode = 0;
    socklen_t nSize = sizeof(nErrorCode);

    if (getsockopt(m_hSocket, SOL_SOCKET, SO_ERROR, static_cast<void*>(&nErrorCode), &nSize) == 0)
    {
        if (nErrorCode != 0)
        {
            IMS_TRACE_D("SocketLastError - socket=%d, error=%d(%s)", m_hSocket, nErrorCode,
                    strerror(nErrorCode));
        }
    }

    return nErrorCode;
}

PUBLIC VIRTUAL IMS_SINT32 OsSocket::GetSocketState() const
{
#define MAX_PEEK_BUF_SIZE 5

    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return SOCKET_STATE_CLOSED;
    }

    if (GetSocketType() == TYPE_DGRAM)
    {
        return SOCKET_STATE_DATA_AVAILABLE;
    }

    IMS_CHAR acBuffer[MAX_PEEK_BUF_SIZE] = {
            0,
    };
    IMS_SINT32 nReadSize = recv(m_hSocket, acBuffer, MAX_PEEK_BUF_SIZE, MSG_PEEK);

    if (nReadSize == 0)
    {
        // Socket is closed by the peer
        IMS_TRACE_D("TCP client socket (%d-%d) is closed by the peer", m_hSocket,
                m_nInternalSocketId, 0);
        return SOCKET_STATE_CLOSED;
    }
    else if (nReadSize < 0)
    {
        switch (errno)
        {
            case EINTR:
            case EWOULDBLOCK:
            case EINPROGRESS:
                IMS_TRACE_D("TCP client socket (%d-%d) is connecting ... (%d)", m_hSocket,
                        m_nInternalSocketId, errno);
                return SOCKET_STATE_NOT_READY;

            case ENOTCONN:
                IMS_TRACE_D("TCP client socket (%d-%d) is not connected ... (%d)", m_hSocket,
                        m_nInternalSocketId, errno);
                return SOCKET_STATE_NOT_READY;

            default:
                IMS_TRACE_D("TCP client socket (%d-%d) is abnormally closed ... (%d)", m_hSocket,
                        m_nInternalSocketId, errno);
                return SOCKET_STATE_CLOSED;
        }
    }

    return SOCKET_STATE_DATA_AVAILABLE;
}

PUBLIC GLOBAL IMS_BOOL OsSocket::StartUp()
{
    return OsSocketService::GetInstance()->StartUp();
}

PUBLIC GLOBAL void OsSocket::CleanUp()
{
    OsSocketService::GetInstance()->CleanUp();
}

PUBLIC GLOBAL IMS_BOOL OsSocket::CheckIpAndPortAvailability(
        IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort, IN SOCKET_ENTYPE enType)
{
    IMS_SINT32 nSockType;
    IMS_SINT32 nAf = PF_INET;

    switch (enType)
    {
        case TYPE_STREAM:
            nSockType = SOCK_STREAM | SOCK_CLOEXEC;
            break;
        case TYPE_DGRAM:
            nSockType = SOCK_DGRAM | SOCK_CLOEXEC;
            break;
        default:
            return IMS_FALSE;
    }

    if (objIpAddr.IsIPv4Address())
    {
        nAf = PF_INET;
    }
    else if (objIpAddr.IsIPv6Address())
    {
        nAf = PF_INET6;
    }
    else
    {
        return IMS_FALSE;
    }

    IMS_SOCKET hSocket = socket(nAf, nSockType, IPPROTO_IP);

    if (hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Creating a socket handle failed - %d(%s)", errno, strerror(errno), 0);
        return IMS_FALSE;
    }

    // IPv4
    if (nAf == PF_INET)
    {
        struct sockaddr_in stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin_family = AF_INET;

        if (objIpAddr.Equals(IpAddress::ANY) || objIpAddr.Equals(IpAddress::NONE))
        {
            stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            stSockAddr.sin_addr.s_addr = htonl(objIpAddr.ToIPv4Address());
        }

        stSockAddr.sin_port = htons(static_cast<u_short>(nPort));

        if (bind(hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                    sizeof(stSockAddr)) < 0)
        {
            IMS_TRACE_D("CheckIpAndPortAvailability :: bind failed - %d(%s)", errno,
                    strerror(errno), 0);
            close(hSocket);
            return IMS_FALSE;
        }
    }
    // IPv6
    else
    {
        struct sockaddr_in6 stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin6_family = AF_INET6;
        stSockAddr.sin6_port = htons(static_cast<u_short>(nPort));

        if (inet_pton(AF_INET6, objIpAddr.ToString().GetStr(),
                    static_cast<void*>(stSockAddr.sin6_addr.s6_addr)) != 1)
        {
            IMS_TRACE_E(0, "inet_pton(%s, %d) failed", objIpAddr.ToCharString(), nPort, 0);
            close(hSocket);
            return IMS_FALSE;
        }

        if (bind(hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                    sizeof(stSockAddr)) < 0)
        {
            IMS_TRACE_D("CheckIpAndPortAvailability :: bind failed - %d(%s)", errno,
                    strerror(errno), 0);
            close(hSocket);
            return IMS_FALSE;
        }
    }

    close(hSocket);

    return IMS_TRUE;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Open(IN SOCKET_ENTYPE eType,
        IN ISocketListener* piListener,
        IN ADDRESS_FAMILY_ENTYPE eAddressFamily /*= ADDRESS_FAMILY_INET*/)
{
    SetListener(piListener);

    return Open(eType, eAddressFamily);
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Open(
        IN SOCKET_ENTYPE eType, IN ADDRESS_FAMILY_ENTYPE eAddressFamily /*= ADDRESS_FAMILY_INET*/)
{
    enum
    {
        PROTOCOL_TYPE = IPPROTO_IP
    };

    IMS_SINT32 nAf;
    IMS_SINT32 nSockType = SOCK_CLOEXEC | SOCK_NONBLOCK;
    IMS_SINT32 nSelectEvent = (FD_READ | FD_CLOSE);

    if (m_hSocket != INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket (%d) is already opened", m_hSocket, 0, 0);
        return RESULT_ERROR;
    }

    if (eType == TYPE_DGRAM)
    {
        nSelectEvent |= FD_WRITE;
    }
    // __LINUX_SOCKET_EVENT__ {
    else
    {
        // Set the event (FD_READ) when the TCP socket is connected
        // (after invoking Connect() or Listen())
        nSelectEvent = FD_CLOSE | FD_TCP;
    }
    // }

    switch (eType)
    {
        case TYPE_STREAM:
            nSockType |= SOCK_STREAM;
            break;
        case TYPE_DGRAM:
            nSockType |= SOCK_DGRAM;
            break;
        default:
            return RESULT_ERROR;
    }

    switch (eAddressFamily)
    {
        case ADDRESS_FAMILY_INET:
            nAf = PF_INET;
            break;
        case ADDRESS_FAMILY_INET6:
            nAf = PF_INET6;
            break;
        default:
            return RESULT_ERROR;
    }

    SetSocketType(eType);
    m_eAddressFamily = eAddressFamily;

    m_hSocket = socket(nAf, nSockType, PROTOCOL_TYPE);

    IMS_TRACE_I("Socket :: Open(%d-%d)", m_hSocket, GetInternalSocketId(), 0);

    if (m_hSocket != INVALID_SOCKET)
    {
        OsSocketService::GetInstance()->AttachHandle(m_hSocket, this);

        if (SelectEvent(nSelectEvent))
        {
            return RESULT_SUCCESS;
        }
    }

    /*
     * ENETDOWN
     * EAFNOSUPPORT
     * EINPROGRESS
     * EMFILE
     * ENOBUFS
     * EPROTONOSUPPORT
     * EPROTOTYPE
     * ESOCKTNOSUPPORT
     */
    IMSSOCKET_DEBUG(errno);

    return RESULT_ERROR;
}

PROTECTED VIRTUAL void OsSocket::SetListener(IN ISocketListener* piListener)
{
    m_piListener = piListener;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Close()
{
    SetCloseReason(CLOSE_REASON_USER_ACTION);
    SetSocketConnected(IMS_FALSE);

    if (m_hSocket != INVALID_SOCKET)
    {
        IMS_TRACE_I("Socket :: Close(%d-%d)", m_hSocket, GetInternalSocketId(), 0);

        if (GetSocketType() == TYPE_STREAM)
        {
            // Disable send()
            ShutDown(((m_nOptionForShutdown < 0) ? SHUTDOWN_BOTH : m_nOptionForShutdown));
        }

        IMS_SOCKET hClosedSocket = m_hSocket;

        // Release the socket event
        if (SelectEvent(0))
        {
            OsSocketService::GetInstance()->KillSocket(m_hSocket);
            m_hSocket = INVALID_SOCKET;
        }

        if (close(hClosedSocket) == SOCKET_ERROR)
        {
            UnbindSocketFromIpSecTransform(hClosedSocket);

            // do something
            IMS_SINT32 nError = errno;

            switch (nError)
            {
                case EINPROGRESS:
                case EWOULDBLOCK:
                    IMSSOCKET_DEBUG(nError);
                    return RESULT_WOULDBLOCK;

                case ENETDOWN:
                case ENOTSOCK:
                case EINTR:
                default:
                    IMSSOCKET_DEBUG(nError);
                    break;
            }

            // What to do : returns SUCCESS or ERROR ?????
            return RESULT_ERROR;
        }

        UnbindSocketFromIpSecTransform(hClosedSocket);

        // 100 ms : wait for socket closing ...
        usleep(100000);
    }

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL ISocket* OsSocket::Accept()
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return IMS_NULL;
    }

    OsSocketService* pSocketService = OsSocketService::GetInstance();

    if (pSocketService == IMS_NULL)
    {
        return IMS_NULL;
    }

    OsSocket* pNewSocket = CreateSocket();

    if (pNewSocket == IMS_NULL)
    {
        return IMS_NULL;
    }

    SOCKET hAcceptSocket = accept4(m_hSocket, IMS_NULL, IMS_NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);

    SelectEventEx(FD_ACCEPT);

    if (hAcceptSocket == INVALID_SOCKET)
    {
        pNewSocket->m_hSocket = INVALID_SOCKET;

        /*
         * ENETDOWN
         * EFAULT
         * EINTR
         * EINPROGRESS
         * EINVAL
         * EMFILE
         * ENOBUFS
         * ENOTSOCK
         * EOPNOTSUPP
         * EWOULDBLOCK
         */
        IMSSOCKET_DEBUG(errno);

        delete pNewSocket;
        return IMS_NULL;
    }
    else
    {
        pNewSocket->BindNetworkConnection(GetNetworkConnection());

        pNewSocket->m_hSocket = hAcceptSocket;
        pNewSocket->SetSocketType(GetSocketType());
        pNewSocket->m_eAddressFamily = m_eAddressFamily;

        pNewSocket->SetSocketConnected(IMS_TRUE);

        // Inherits the same property from TCP server socket
        pSocketService->AttachHandle(hAcceptSocket, pNewSocket);

        // Accepted socket has the same event as the listening socket has.
        // if (!pNewSocket->SelectEvent(m_nSocketEvent & (~FD_ACCEPT)))
        // __LINUX_SOCKET_EVENT__ {
        if (!pNewSocket->SelectEvent((m_nSocketEvent | FD_READ | FD_TCP_C) & (~FD_ACCEPT)))
        // }
        {
            IMSSOCKET_DEBUG(errno);

            pNewSocket->Close();

            delete pNewSocket;
            return IMS_NULL;
        }

        // NETWORK_INTERFACE_FOR_SOCKET
        // For an incoming TCP connection, the network interface will be inherited
        // from the TCP server socket, so we don't need to bind the newly accepted socket
        // with the network interface.
        // osSocket_SetNetworkForSocket(m_piOwnerThread->GetSlotId(),
        //        pNewSocket->GetSocket(), pNewSocket->GetNetworkConnection());
    }

    return pNewSocket;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Bind(
        IN const IpAddress& objSocketAddress, IN IMS_UINT32 nSocketPort)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    IMS_SINT32 nBindResult = SOCKET_ERROR;

    // IPv4 address format
    if (m_eAddressFamily == ADDRESS_FAMILY_INET)
    {
        if (!objSocketAddress.IsIPv4Address())
        {
            return RESULT_ERROR;
        }

        struct sockaddr_in stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin_family = AF_INET;

        if (objSocketAddress.Equals(IpAddress::ANY) || objSocketAddress.Equals(IpAddress::NONE))
        {
            stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            stSockAddr.sin_addr.s_addr = htonl(objSocketAddress.ToIPv4Address());
        }

        stSockAddr.sin_port = htons(static_cast<u_short>(nSocketPort));

        // Linux auto configuration takes the several seconds when the local IP address is changed.
        // So, waiting for the auto configuration completion during 3 seconds.
        {
            IMS_SINT32 nRetryCount = 0;

            do
            {
                nBindResult = bind(m_hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                        sizeof(stSockAddr));

                if (nRetryCount >= IMS_MAX_RETRY_COUNT_ON_BIND_FAILED)
                {
                    break;
                }

                // "Cannot assign requested address"
                if ((nBindResult == SOCKET_ERROR) && (errno == EADDRNOTAVAIL))
                {
                    ++nRetryCount;
                    IMS_TRACE_D("Waiting for linux network interface (%d) ...", nRetryCount, 0, 0);
                    usleep(1000000);  // 1 seconds
                }
                else
                {
                    break;
                }
            } while (1);
        }
    }
    // IPv6 address format
    else
    {
        if (objSocketAddress.IsIPv4Address())
        {
            return RESULT_ERROR;
        }

        struct sockaddr_in6 stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin6_family = AF_INET6;
        stSockAddr.sin6_port = htons(static_cast<u_short>(nSocketPort));

        // 4 Why the specified IPv6 address can't bind the socket ??
        if (inet_pton(AF_INET6, objSocketAddress.ToString().GetStr(),
                    static_cast<void*>(stSockAddr.sin6_addr.s6_addr)) != 1)
        {
            IMS_TRACE_E(
                    0, "inet_pton(%s, %d) failed", objSocketAddress.ToCharString(), nSocketPort, 0);
            return RESULT_ERROR;
        }

        // Linux auto configuration takes the several seconds
        // when the local IP address is changed.
        // So, waiting for the auto configuration completion during 3 seconds.
        {
            IMS_SINT32 nRetryCount = 0;

            do
            {
                nBindResult = bind(m_hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                        sizeof(stSockAddr));

                if (nRetryCount >= IMS_MAX_RETRY_COUNT_ON_BIND_FAILED)
                {
                    break;
                }

                // "Cannot assign requested address"
                if ((nBindResult == SOCKET_ERROR) && (errno == EADDRNOTAVAIL))
                {
                    ++nRetryCount;
                    IMS_TRACE_D("Waiting for linux network interface (%d) ...", nRetryCount, 0, 0);
                    usleep(1000000);  // 1 seconds
                }
                else
                {
                    break;
                }
            } while (1);
        }
    }

    if (nBindResult == SOCKET_ERROR)
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            // Socket is already bound to an address
            case EINVAL:
                return RESULT_SUCCESS;

            case EINPROGRESS:
                // NETWORK_INTERFACE_FOR_SOCKET
                osSocket_SetNetworkForSocket(
                        m_piOwnerThread->GetSlotId(), m_hSocket, GetNetworkConnection());
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EACCES:
            case EADDRINUSE:
            case EADDRNOTAVAIL:
            case EFAULT:
            case ENOBUFS:
            case ENOTSOCK:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        return RESULT_ERROR;
    }

    // PATCH_FOR_NON_SOCKET
    m_objSocketAddress = objSocketAddress;
    m_nSocketPort = nSocketPort;

    // NETWORK_INTERFACE_FOR_SOCKET
    osSocket_SetNetworkForSocket(m_piOwnerThread->GetSlotId(), m_hSocket, GetNetworkConnection());

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Connect(
        IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    IMS_SINT32 nConnectResult = SOCKET_ERROR;

    // IPv4 address format
    if (m_eAddressFamily == ADDRESS_FAMILY_INET)
    {
        if (!objHostAddress.IsIPv4Address())
        {
            return RESULT_ERROR;
        }

        if (objHostAddress.Equals(IpAddress::ANY) || objHostAddress.Equals(IpAddress::NONE))
        {
            return RESULT_ERROR;
        }

        struct sockaddr_in stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin_family = AF_INET;
        stSockAddr.sin_addr.s_addr = inet_addr(objHostAddress.ToString().GetStr());

        if (stSockAddr.sin_addr.s_addr == INADDR_NONE)
        {
            hostent* pHost = gethostbyname(objHostAddress.ToString().GetStr());

            if (pHost != NULL)
            {
                stSockAddr.sin_addr.s_addr =
                        reinterpret_cast<struct in_addr*>(pHost->h_addr)->s_addr;
            }
            else
            {
                IMSSOCKET_DEBUG(EINVAL);
                return RESULT_ERROR;
            }
        }

        stSockAddr.sin_port = htons(static_cast<u_short>(nHostPort));

        nConnectResult = connect(m_hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                sizeof(stSockAddr));

        if (nConnectResult == SOCKET_ERROR)
        {
            IMS_SINT32 nError = errno;

            // Retry TCP connection in the below error cases
            if ((nError == ECONNREFUSED) || (nError == ENETUNREACH) || (nError == ETIMEDOUT))
            {
                if (SOCKET_ERROR !=
                        connect(m_hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                                sizeof(stSockAddr)))
                {
                    SetSocketConnected(IMS_TRUE);
                    // __LINUX_SOCKET_EVENT__ {
                    SelectEventEx(FD_READ | FD_TCP_C);
                    // }
                    return RESULT_SUCCESS;
                }
            }
            else if ((nError == EINPROGRESS) || (nError == EWOULDBLOCK))
            {
                // 4 Set Event Writable
            }

            SetSocketConnected(IMS_FALSE);
        }
        else
        {
            SetSocketConnected(IMS_TRUE);
        }
    }
    // IPv6 address format
    else
    {
        if (objHostAddress.IsIPv4Address())
        {
            return RESULT_ERROR;
        }

        if (objHostAddress.Equals(IpAddress::IPv6ANY) || objHostAddress.Equals(IpAddress::IPv6NONE))
        {
            return RESULT_ERROR;
        }

        struct sockaddr_in6 stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin6_family = AF_INET6;
        stSockAddr.sin6_port = htons(static_cast<u_short>(nHostPort));

        if (inet_pton(AF_INET6, objHostAddress.ToString().GetStr(),
                    static_cast<void*>(stSockAddr.sin6_addr.s6_addr)) != 1)
        {
            IMS_TRACE_E(0, "inet_pton(%s, %d) failed", objHostAddress.ToCharString(), nHostPort, 0);
            return RESULT_ERROR;
        }

        nConnectResult = connect(m_hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                sizeof(stSockAddr));

        if (nConnectResult == SOCKET_ERROR)
        {
            int nError = errno;

            // Retry TCP connection in the below error cases
            if ((nError == ECONNREFUSED) || (nError == ENETUNREACH) || (nError == ETIMEDOUT))
            {
                if (SOCKET_ERROR !=
                        connect(m_hSocket, reinterpret_cast<const struct sockaddr*>(&stSockAddr),
                                sizeof(stSockAddr)))
                {
                    SetSocketConnected(IMS_TRUE);
                    // __LINUX_SOCKET_EVENT__ {
                    SelectEventEx(FD_READ | FD_TCP_C);
                    // }
                    return RESULT_SUCCESS;
                }
            }

            SetSocketConnected(IMS_FALSE);
        }
        else
        {
            SetSocketConnected(IMS_TRUE);
        }
    }

    if (nConnectResult == SOCKET_ERROR)
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            case EISCONN:
                return RESULT_SUCCESS;

            case ETIMEDOUT:
            case EALREADY:
            case EINPROGRESS:
            case EWOULDBLOCK:
                // SelectEventEx(FD_CONNECT);
                // __LINUX_SOCKET_EVENT__ {
                SelectEventEx(FD_CONNECT | FD_READ | FD_TCP_C);
                // }
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EADDRINUSE:
            case EINTR:
            case EADDRNOTAVAIL:
            case EAFNOSUPPORT:
            case ECONNREFUSED:
            case EFAULT:
            case EINVAL:
            case ENETUNREACH:
            case ENOBUFS:
            case ENOTSOCK:
            case EACCES:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        return RESULT_ERROR;
    }

    // __LINUX_SOCKET_EVENT__ {
    SelectEventEx(FD_READ | FD_TCP_C);
    // }

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Listen(IN IMS_SINT32 nBackLog /*= MAX_BACKLOG*/)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    SetServerSocket(IMS_TRUE);

    if (listen(m_hSocket, nBackLog) == SOCKET_ERROR)
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            case EINPROGRESS:
                SelectEventEx(FD_ACCEPT);
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EADDRINUSE:
            case EINVAL:
            case EISCONN:
            case EMFILE:
            case ENOBUFS:
            case ENOTSOCK:
            case EOPNOTSUPP:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        return RESULT_ERROR;
    }

    if (!SelectEventEx(FD_ACCEPT))
    {
        IMSSOCKET_DEBUG(errno);
        return RESULT_ERROR;
    }

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL IMS_SINT32 OsSocket::Receive(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    IMS_SINT32 nReadBytes;

    if ((nReadBytes = recv(m_hSocket, static_cast<void*>(pBuffer), nBuffLen, 0)) == SOCKET_ERROR)
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            case EMSGSIZE:
                //????
                IMSSOCKET_DEBUG(nError);
                break;

            case EINTR:
            case EINPROGRESS:
            case EWOULDBLOCK:
                SelectEventEx(FD_READ);
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EFAULT:
            case ENOTCONN:
            case ENETRESET:
            case ENOTSOCK:
            case ESHUTDOWN:
            case EINVAL:
            case ECONNABORTED:
            case ETIMEDOUT:
            case ECONNRESET:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        // PATCH_FOR_NON_SOCKET
        if (nError == ENOTSOCK)
        {
            SetCloseReason(CLOSE_REASON_INTERNAL_ERROR);
            NotifyClosed(0);
        }

        return RESULT_ERROR;
    }

    if (nReadBytes == 0)
    {
        // The connection is gracefully closed by the remote connection.
        // CLOSED_BY_NETWORK
        // Close();
        OsSocket::NotifyMessage(IMS_SOCKET_CLOSED);
        return 0;
    }

    if (nReadBytes < nBuffLen)
    {
        SelectEventEx(FD_READ);
    }

    return nReadBytes;
}

PROTECTED VIRTUAL IMS_SINT32 OsSocket::Send(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    IMS_SINT32 nWrittenBytes;

    if ((nWrittenBytes = send(m_hSocket, static_cast<const void*>(pBuffer), nBuffLen, 0)) ==
            SOCKET_ERROR)
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            case EMSGSIZE:
                SelectEventEx(FD_WRITE);
                IMSSOCKET_DEBUG(nError);
                break;

            case EINTR:
            case EHOSTUNREACH:
            case EINPROGRESS:
            case EWOULDBLOCK:
                SelectEventEx(FD_WRITE);
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EACCES:
            case EFAULT:
            case ENETRESET:
            case ENOBUFS:
            case ENOTCONN:
            case ENOTSOCK:
            case EOPNOTSUPP:
            case ESHUTDOWN:
            case EINVAL:
            case ECONNABORTED:
            case ECONNRESET:
            case ETIMEDOUT:
            case EPROTONOSUPPORT:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        // PATCH_FOR_NON_SOCKET
        if (nError == ENOTSOCK)
        {
            SetCloseReason(CLOSE_REASON_INTERNAL_ERROR);
            NotifyClosed(0);
        }

        return RESULT_ERROR;
    }

    return nWrittenBytes;
}

PROTECTED VIRTUAL IMS_SINT32 OsSocket::ReceiveFrom(OUT IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        OUT IpAddress& objHostAddress, OUT IMS_UINT32& nHostPort)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    struct sockaddr_in stAddr4;
    struct sockaddr_in6 stAddr6;
    struct sockaddr* pstSockAddr = IMS_NULL;
    socklen_t nSockAddrLen = 0;

    if (m_eAddressFamily == ADDRESS_FAMILY_INET)
    {
        nSockAddrLen = sizeof(stAddr4);
        memset(&stAddr4, 0x00, nSockAddrLen);
        pstSockAddr = reinterpret_cast<struct sockaddr*>(&stAddr4);
    }
    else
    {
        nSockAddrLen = sizeof(stAddr6);
        memset(&stAddr6, 0x00, nSockAddrLen);
        pstSockAddr = reinterpret_cast<struct sockaddr*>(&stAddr6);
    }

    IMS_SINT32 nReadBytes = recvfrom(
            m_hSocket, static_cast<void*>(pBuffer), nBuffLen, 0, pstSockAddr, &nSockAddrLen);

    if (nReadBytes != SOCKET_ERROR)
    {
        osSocket_GetAddressNPort(pstSockAddr, nSockAddrLen, objHostAddress, nHostPort);

        SelectEventEx(FD_READ);
        return nReadBytes;
    }
    else
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            case EMSGSIZE:
                //????
                IMSSOCKET_DEBUG(nError);
                break;

            case EINTR:
            case EISCONN:
            case EINPROGRESS:
            case EWOULDBLOCK:
                SelectEventEx(FD_READ);
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EFAULT:
            case EINVAL:
            case ENETRESET:
            case ENOTSOCK:
            case ESHUTDOWN:
            case ETIMEDOUT:
            case ECONNRESET:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        // PATCH_FOR_NON_SOCKET
        if (nError == ENOTSOCK)
        {
            SetCloseReason(CLOSE_REASON_INTERNAL_ERROR);
            NotifyClosed(0);
        }
    }

    return RESULT_ERROR;
}

PROTECTED VIRTUAL IMS_SINT32 OsSocket::SendTo(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        IN const IpAddress& objHostAddress, IN IMS_UINT32 nHostPort)
{
    // PATCH_FOR_NON_SOCKET
    IMS_SINT32 nSentCount = 0;

    // PATCH_FOR_NON_SOCKET
RETRY_SENDTO:

    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    IMS_SINT32 nWrittenBytes = 0;

    // IPv4 address format
    if (m_eAddressFamily == ADDRESS_FAMILY_INET)
    {
        if (!objHostAddress.IsIPv4Address())
        {
            return RESULT_ERROR;
        }

        struct sockaddr_in stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin_family = AF_INET;

        if (objHostAddress.Equals(IpAddress::ANY) || objHostAddress.Equals(IpAddress::NONE))
        {
            stSockAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        }
        else
        {
            stSockAddr.sin_addr.s_addr = inet_addr(objHostAddress.ToString().GetStr());
            if (stSockAddr.sin_addr.s_addr == INADDR_NONE)
            {
                hostent* pHost = gethostbyname(objHostAddress.ToString().GetStr());

                if (pHost != IMS_NULL)
                {
                    stSockAddr.sin_addr.s_addr =
                            reinterpret_cast<struct in_addr*>(pHost->h_addr)->s_addr;
                }
                else
                {
                    IMSSOCKET_DEBUG(EINVAL);
                    return RESULT_ERROR;
                }
            }
        }

        stSockAddr.sin_port = htons(static_cast<u_short>(nHostPort));

        nWrittenBytes = sendto(m_hSocket, static_cast<const void*>(pBuffer), nBuffLen, 0,
                reinterpret_cast<const struct sockaddr*>(&stSockAddr), sizeof(stSockAddr));
    }
    // IPv6 address format
    else
    {
        if (objHostAddress.IsIPv4Address())
        {
            return RESULT_ERROR;
        }

        struct sockaddr_in6 stSockAddr;

        memset(&stSockAddr, 0x00, sizeof(stSockAddr));

        stSockAddr.sin6_family = AF_INET6;
        stSockAddr.sin6_port = htons(static_cast<u_short>(nHostPort));

        if (inet_pton(AF_INET6, objHostAddress.ToString().GetStr(),
                    static_cast<void*>(stSockAddr.sin6_addr.s6_addr)) != 1)
        {
            IMS_TRACE_E(0, "inet_pton(%s, %d) failed", objHostAddress.ToCharString(), nHostPort, 0);
            return RESULT_ERROR;
        }

        nWrittenBytes = sendto(m_hSocket, static_cast<const void*>(pBuffer), nBuffLen, 0,
                reinterpret_cast<const struct sockaddr*>(&stSockAddr), sizeof(stSockAddr));
    }

    if (nWrittenBytes == SOCKET_ERROR)
    {
        IMS_SINT32 nError = errno;

        switch (nError)
        {
            case EMSGSIZE:
                //????
                SelectEventEx(FD_WRITE);
                IMSSOCKET_DEBUG(nError);
                break;

                // case EHOSTUNREACH:
                // case ENETUNREACH:
            case EINTR:
            case EINPROGRESS:
            case EWOULDBLOCK:
                SelectEventEx(FD_WRITE);
                IMSSOCKET_DEBUG(nError);
                return RESULT_WOULDBLOCK;

            case ENETDOWN:
            case EACCES:
            case EINVAL:
            case EFAULT:
            case ENETRESET:
            case ENOBUFS:
            case ENOTCONN:
            case ENOTSOCK:
            case EOPNOTSUPP:
            case ESHUTDOWN:
            case ECONNABORTED:
            case ECONNRESET:
            case EADDRNOTAVAIL:
            case EAFNOSUPPORT:
            case EDESTADDRREQ:
            case ETIMEDOUT:
            case EPROTONOSUPPORT:
            default:
                IMSSOCKET_DEBUG(nError);
                break;
        }

        // PATCH_FOR_NON_SOCKET
        if (nError == ENOTSOCK)
        {
            IMS_TRACE_D("Socket operation on non-socket (%d); try to recover the socket", m_hSocket,
                    0, 0);

            if (DoSocketRecovery() == RESULT_SUCCESS)
            {
                if (nSentCount == 0)
                {
                    nSentCount++;
                    goto RETRY_SENDTO;
                }
            }
            else
            {
                SetCloseReason(CLOSE_REASON_INTERNAL_ERROR);
                NotifyClosed(0);
            }
        }

        return RESULT_ERROR;
    }

    return nWrittenBytes;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::GetPeerName(
        OUT IpAddress& objPeerAddress, OUT IMS_UINT32& nPeerPort)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        if (m_eAddressFamily == ADDRESS_FAMILY_INET)
        {
            objPeerAddress = IpAddress::NONE;
        }
        else
        {
            objPeerAddress = IpAddress::IPv6NONE;
        }

        nPeerPort = 0;

        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    struct sockaddr_in stAddr4;
    struct sockaddr_in6 stAddr6;
    struct sockaddr* pstSockAddr = IMS_NULL;
    socklen_t nSockAddrLen = 0;

    if (m_eAddressFamily == ADDRESS_FAMILY_INET)
    {
        nSockAddrLen = sizeof(stAddr4);
        memset(&stAddr4, 0x00, nSockAddrLen);
        pstSockAddr = reinterpret_cast<struct sockaddr*>(&stAddr4);
    }
    else
    {
        nSockAddrLen = sizeof(stAddr6);
        memset(&stAddr6, 0x00, nSockAddrLen);
        pstSockAddr = reinterpret_cast<struct sockaddr*>(&stAddr6);
    }

    if (getpeername(m_hSocket, pstSockAddr, &nSockAddrLen) == SOCKET_ERROR)
    {
        IMSSOCKET_DEBUG(errno);
        return RESULT_ERROR;
    }

    osSocket_GetAddressNPort(pstSockAddr, nSockAddrLen, objPeerAddress, nPeerPort);

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::GetSockName(
        OUT IpAddress& objSocketAddress, OUT IMS_UINT32& nSocketPort)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        if (m_eAddressFamily == ADDRESS_FAMILY_INET)
        {
            objSocketAddress = IpAddress::NONE;
        }
        else
        {
            objSocketAddress = IpAddress::IPv6NONE;
        }

        nSocketPort = 0;

        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return RESULT_ERROR;
    }

    struct sockaddr_in stAddr4;
    struct sockaddr_in6 stAddr6;
    struct sockaddr* pstSockAddr = IMS_NULL;
    socklen_t nSockAddrLen = 0;

    if (m_eAddressFamily == ADDRESS_FAMILY_INET)
    {
        nSockAddrLen = sizeof(stAddr4);
        memset(&stAddr4, 0x00, nSockAddrLen);
        pstSockAddr = reinterpret_cast<struct sockaddr*>(&stAddr4);
    }
    else
    {
        nSockAddrLen = sizeof(stAddr6);
        memset(&stAddr6, 0x00, nSockAddrLen);
        pstSockAddr = reinterpret_cast<struct sockaddr*>(&stAddr6);
    }

    if (getsockname(m_hSocket, pstSockAddr, &nSockAddrLen) == SOCKET_ERROR)
    {
        IMSSOCKET_DEBUG(errno);
        return RESULT_ERROR;
    }

    osSocket_GetAddressNPort(pstSockAddr, nSockAddrLen, objSocketAddress, nSocketPort);

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL IMS_BOOL OsSocket::Equals(IN const ISocket* piSocket)
{
    const OsSocket* pSocket = static_cast<const OsSocket*>(piSocket);

    if (pSocket == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_hSocket == INVALID_SOCKET)
    {
        return (this == pSocket);
    }

    return (m_hSocket == pSocket->m_hSocket);
}

PROTECTED VIRTUAL IMS_SINT32 OsSocket::GetOption(IN IMS_SINT32 nOption)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return (-1);
    }

    IMS_SINT32 nOptVal = 0;
    socklen_t nOptSize = sizeof(nOptVal);

    switch (nOption)
    {
        case OPT_IP_QOS:
            if (m_eAddressFamily == ADDRESS_FAMILY_INET)
            {
                if (getsockopt(m_hSocket, SOL_IP, IP_TOS, static_cast<void*>(&nOptVal), &nOptSize) <
                        0)
                {
                    return (-1);
                }
            }
            else
            {
                if (getsockopt(m_hSocket, SOL_IPV6, IPV6_TCLASS, static_cast<void*>(&nOptVal),
                            &nOptSize) < 0)
                {
                    return (-1);
                }
            }
            break;

            // tcp_keepalive_probes (count)
            //    The number of unacknowledged probes to send before considering
            //    the connection dead and notifying the application layer
        case OPT_TCP_KEEPCNT:
            if (getsockopt(m_hSocket, SOL_TCP, TCP_KEEPCNT, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

            // tcp_keepalive_time (seconds)
            //    The interval between the last data packet sent (simple ACKs are not
            //    considered data) and the first keepalive probe; after the connection
            //    is marked to need keepalive, this counter is not used any further
        case OPT_TCP_KEEPIDLE:
            if (getsockopt(m_hSocket, SOL_TCP, TCP_KEEPIDLE, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

            // tcp_keepalive_intvl (seconds)
            //    The interval between subsequential keepalive probes, regardless of
            //    what the connection has exchanged in the meantime
        case OPT_TCP_KEEPINTVL:
            if (getsockopt(m_hSocket, SOL_TCP, TCP_KEEPINTVL, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

        case OPT_TCP_MAXSEG:
            if (getsockopt(m_hSocket, SOL_TCP, TCP_MAXSEG, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

        case OPT_KEEPALIVE:
            if (getsockopt(m_hSocket, SOL_SOCKET, SO_KEEPALIVE, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

        case OPT_RCVBUF:
            if (getsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

        case OPT_SNDBUF:
            if (getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, static_cast<void*>(&nOptVal),
                        &nOptSize) < 0)
            {
                return (-1);
            }
            break;

        default:
            IMS_TRACE_E(0, "Unsupported socket option (%d)", nOption, 0, 0);
            return (-1);
    }

    return nOptVal;
}

PROTECTED VIRTUAL IMS_BOOL OsSocket::SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        IMS_TRACE_E(0, "Socket handle is null", 0, 0, 0);
        return IMS_FALSE;
    }

    switch (nOption)
    {
        case OPT_IP_QOS:
            if (m_eAddressFamily == ADDRESS_FAMILY_INET)
            {
                if (setsockopt(m_hSocket, SOL_IP, IP_TOS, static_cast<const void*>(&nOptionValue),
                            sizeof(nOptionValue)) < 0)
                {
                    return IMS_FALSE;
                }
            }
            else
            {
                if (setsockopt(m_hSocket, SOL_IPV6, IPV6_TCLASS,
                            static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
                {
                    return IMS_FALSE;
                }
            }

            IMS_TRACE_D("SetOption (%d) - IP_QOS (TOS or TCLASS), %d", m_hSocket, nOptionValue, 0);
            break;

            // tcp_keepalive_probes (count)
            //    The number of unacknowledged probes to send before considering
            //    the connection dead and notifying the application layer
        case OPT_TCP_KEEPCNT:
            if (setsockopt(m_hSocket, SOL_TCP, TCP_KEEPCNT, static_cast<const void*>(&nOptionValue),
                        sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - TCP_KEEPCNT, %d", m_hSocket, nOptionValue, 0);
            break;

            // tcp_keepalive_time (seconds)
            //    The interval between the last data packet sent (simple ACKs are not
            //    considered data) and the first keepalive probe; after the connection
            //    is marked to need keepalive, this counter is not used any further
        case OPT_TCP_KEEPIDLE:
            if (setsockopt(m_hSocket, SOL_TCP, TCP_KEEPIDLE,
                        static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - TCP_KEEPIDLE, %d", m_hSocket, nOptionValue, 0);
            break;

            // tcp_keepalive_intvl (seconds)
            //    The interval between subsequential keepalive probes, regardless of
            //    what the connection has exchanged in the meantime
        case OPT_TCP_KEEPINTVL:
            if (setsockopt(m_hSocket, SOL_TCP, TCP_KEEPINTVL,
                        static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - TCP_KEEPINTVL, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_TCP_MAXSEG:
            if (setsockopt(m_hSocket, SOL_TCP, TCP_MAXSEG, static_cast<const void*>(&nOptionValue),
                        sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - TCP_MAXSEG, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_RCVBUF:
            if (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF,
                        static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - SO_RCVBUF, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_SNDBUF:
            if (setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF,
                        static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - SO_SNDBUF, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_REUSEADDR:
            if (setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR,
                        static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - SO_REUSEADDR, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_LINGER:
        {
            linger stLinger;

            stLinger.l_onoff = (nOptionValue >= 0) ? 1 : 0;
            stLinger.l_linger = (nOptionValue < 0) ? 0 : nOptionValue;  // seconds

            if (setsockopt(m_hSocket, SOL_SOCKET, SO_LINGER, static_cast<const void*>(&stLinger),
                        sizeof(stLinger)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - SO_LINGER, %d, %d", m_hSocket, stLinger.l_onoff,
                    nOptionValue);
        }
        break;

        case OPT_KEEPALIVE:
            if (setsockopt(m_hSocket, SOL_SOCKET, SO_KEEPALIVE,
                        static_cast<const void*>(&nOptionValue), sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - OPT_KEEPALIVE, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_UDP_ENCAP:
            if (setsockopt(m_hSocket, SOL_UDP, UDP_ENCAP, static_cast<const void*>(&nOptionValue),
                        sizeof(nOptionValue)) < 0)
            {
                return IMS_FALSE;
            }

            IMS_TRACE_D("SetOption (%d) - OPT_UDP_ENCAP, %d", m_hSocket, nOptionValue, 0);
            break;

        case OPT_SHUTDOWN:
            if (nOptionValue >= SHUTDOWN_RX)
            {
                m_nOptionForShutdown = nOptionValue;
            }

            IMS_TRACE_D("SetOption (%d) - OPT_SHUTDOWN, %d", m_hSocket, nOptionValue, 0);
            break;

        default:
            IMS_TRACE_D("Unsupported socket option (%d)", nOption, 0, 0);
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL ISocket::SOCKET_RESULT OsSocket::Abort()
{
    SetCloseReason(CLOSE_REASON_USER_ACTION);

    if (m_hSocket != INVALID_SOCKET)
    {
        if (GetSocketType() == TYPE_STREAM)
        {
            // Disable send()
            ShutDown((((m_nOptionForShutdown < 0) ? SHUTDOWN_BOTH : m_nOptionForShutdown)));
        }

        if (close(m_hSocket) == SOCKET_ERROR)
        {
            // do something
            IMS_SINT32 nError = errno;

            switch (nError)
            {
                case EINPROGRESS:
                case EWOULDBLOCK:
                    IMSSOCKET_DEBUG(nError);
                    break;

                case ENETDOWN:
                case ENOTSOCK:
                case EINTR:
                default:
                    IMSSOCKET_DEBUG(nError);
                    break;
            }
        }

        UnbindSocketFromIpSecTransform(m_hSocket);

        // Release the socket event
        if (SelectEvent(0))
        {
            m_hSocket = INVALID_SOCKET;
        }
    }

    return RESULT_SUCCESS;
}

PROTECTED VIRTUAL void OsSocket::ClosedByDataConnection()
{
    SetCloseReason(CLOSE_REASON_DATA_CONNECTION_LOST);

    // Notify the application that this socket is closed
    NotifyClosed(0);
}

PROTECTED VIRTUAL void OsSocket::Destroy()
{
    OsSocketService* pSocketService = OsSocketService::GetInstance();

    if (pSocketService == IMS_NULL)
    {
        delete this;
        return;
    }

    if (m_hSocket != INVALID_SOCKET)
    {
        Close();
    }

    m_piListener = IMS_NULL;

    pSocketService->AddDeadSocket(this);
    pSocketService->SendControlEvent();
}

PROTECTED VIRTUAL void OsSocket::DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    (void)nLparam;

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "Listener is null", 0, 0, 0);
        return;
    }

    // FIX_TIMING_ISSUE
    if (GetInternalSocketId() != IMS_SOCKET_HIWORD(nLparam))
    {
        IMS_TRACE_D("Socket(%d) is matched, but internal socket id(%d) is not matched;"
                    " ignored",
                m_hSocket, GetInternalSocketId(), 0);
        return;
    }

    switch (nWparam)
    {
        case IMS_SOCKET_DATA_RECEIVED:
            m_piListener->Socket_OnDataReceived(this);
            break;

        case IMS_SOCKET_CLOSED:
        {
            // Checks if the owner thread is same or not...

            // FIX_SOCKET_SYNC_ISSUE
            const IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

            if (piThread != m_piOwnerThread)
            {
                IMS_TRACE_D("Socket :: socket(%d) owner is different; ignored...", m_hSocket, 0, 0);
                break;
            }

            m_piListener->Socket_OnClosed(this, GetCloseReason());
            break;
        }
        case IMS_SOCKET_SEND_ENABLED:
            m_piListener->Socket_OnSendEnabled(this);
            break;

        case IMS_SOCKET_CONNECTION_RECEIVED:
            m_piListener->Socket_OnConnectionReceived(this);
            break;

        case IMS_SOCKET_CONNECTED:
            m_piListener->Socket_OnConnected(this);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void OsSocket::NotifyDataReceived(IN IMS_SINT32 nErrorCode)
{
    if (nErrorCode == 0)
    {
        DeselectEventEx(FD_READ);
        OsSocket::NotifyMessage(IMS_SOCKET_DATA_RECEIVED);
    }
    else
    {
        IMS_TRACE_D("NotifyDataReceived - socket=%d, error=%d", m_hSocket, nErrorCode, 0);
    }
}

PROTECTED VIRTUAL void OsSocket::NotifySendEnabled(IN IMS_SINT32 nErrorCode)
{
    if (nErrorCode == 0)
    {
        DeselectEventEx(FD_WRITE);
        OsSocket::NotifyMessage(IMS_SOCKET_SEND_ENABLED);
    }
    else
    {
        IMS_TRACE_D("NotifySendEnabled - socket=%d, error=%d", m_hSocket, nErrorCode, 0);
    }
}

PROTECTED VIRTUAL void OsSocket::NotifyConnectionReceived(IN IMS_SINT32 nErrorCode)
{
    if (nErrorCode == 0)
    {
        DeselectEventEx(FD_ACCEPT);
        OsSocket::NotifyMessage(IMS_SOCKET_CONNECTION_RECEIVED);
    }
    else
    {
        IMS_TRACE_D("NotifyConnectionReceived - socket=%d, error=%d", m_hSocket, nErrorCode, 0);

        // Send a CLOSED event when socket is aborted by the local system.
        if ((nErrorCode == ENETDOWN) || (nErrorCode == ENETRESET) || (nErrorCode == ECONNABORTED))
        {
            // Aborted by local system
            NotifyClosed(nErrorCode);
        }
    }
}

PROTECTED VIRTUAL void OsSocket::NotifyConnected(IN IMS_SINT32 nErrorCode)
{
    switch (nErrorCode)
    {
        case 0:
        {
            if (GetCloseReason() != CLOSE_REASON_UNKNOWN)
            {
                DeselectEventEx(FD_CONNECT);
                IMS_TRACE_E(0, "Socket (%d) is already aborted or closed on NotifyConnected",
                        m_hSocket, 0, 0);
                break;
            }

            SetSocketConnected(IMS_TRUE);
            DeselectEventEx(FD_CONNECT);

            OsSocket::NotifyMessage(IMS_SOCKET_CONNECTED);
            break;
        }
        case EAFNOSUPPORT:  // FALL-THROUGH
        case ECONNREFUSED:  // FALL-THROUGH
        case ENETUNREACH:   // FALL-THROUGH
        case ENOBUFS:       // FALL-THROUGH
        case ETIMEDOUT:
        {
            SetSocketConnected(IMS_FALSE);
            // 3 SOCKET_CONNECTION_FAILED ??? : new event ???
            DeselectEventEx(FD_READ);
            OsSocket::NotifyMessage(IMS_SOCKET_CLOSED);
            break;
        }
        default:
        {
            SetSocketConnected(IMS_FALSE);
            DeselectEventEx(FD_READ);
            IMS_TRACE_E(0, "Unknown socket (%d) error (%d) on Connected", m_hSocket, nErrorCode, 0);
            OsSocket::NotifyMessage(IMS_SOCKET_CLOSED);
            break;
        }
    }
}

PROTECTED VIRTUAL void OsSocket::NotifyClosed(IN IMS_SINT32 nErrorCode)
{
    SetCloseReason(CLOSE_REASON_REMOTE_ACTION);
    SetSocketConnected(IMS_FALSE);

    // Remove all events: READ/WRITE/CLOSE (common), TCP/TCP_C (PollFdSet)
    DeselectEventEx(FD_READ | FD_WRITE | FD_CLOSE | FD_TCP | FD_TCP_C);

    IMS_TRACE_D("NotifyClosed - socket=%d, error=%d", m_hSocket, nErrorCode, 0);

    // FIX_SOCKET_SYNC_ISSUE
    if (m_nCloseReason == CLOSE_REASON_USER_ACTION)
    {
        IMS_TRACE_D("Socket(%d) is already closed by the user; ignored...", m_hSocket, 0, 0);
        return;
    }

    // Closed by network: ENETDOWN, ECONNRESET, ECONNABORTED
    OsSocket::NotifyMessage(IMS_SOCKET_CLOSED);
}

PROTECTED VIRTUAL void OsSocket::NotifyAcceptCompleted(IN IMS_SOCKET hSocket)
{
    m_hSocket = hSocket;
    DeselectEventEx(FD_ACCEPT);
}

PROTECTED VIRTUAL OsSocket* OsSocket::CreateSocket()
{
    return new OsSocket();
}

PROTECTED VIRTUAL IMS_BOOL OsSocket::ShutDown(IN IMS_SINT32 nHow /*= SHUTDOWN_BOTH*/)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        return IMS_FALSE;
    }

    if ((nHow != SHUTDOWN_RX) && (nHow != SHUTDOWN_TX) && (nHow != SHUTDOWN_BOTH))
    {
        return IMS_FALSE;
    }

    return (shutdown(m_hSocket, nHow) != SOCKET_ERROR);
}

/**
 * Previous events are removed
 * New event is set.
 */
PROTECTED
IMS_BOOL OsSocket::SelectEvent(IN IMS_SLONG nEvent /*= EVENT_FD_ALL*/)
{
    if (m_hSocket == INVALID_SOCKET)
    {
        return IMS_FALSE;
    }

    OsSocketService* pSocketService = OsSocketService::GetInstance();

    if (pSocketService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pSocketService->RemoveEvent(m_hSocket, m_nSocketEvent);
    m_nSocketEvent = nEvent;

    if (m_nSocketEvent != 0)
    {
        pSocketService->SetEvent(m_hSocket, nEvent);
    }

    pSocketService->SendControlEvent();

    return IMS_TRUE;
}

/**
 * Use this function, only when event is added.
 */
PROTECTED
IMS_BOOL OsSocket::SelectEventEx(IN IMS_SLONG nEvent)
{
    if ((m_nSocketEvent & nEvent) == nEvent)
    {
        return IMS_TRUE;
    }

    return SelectEvent(m_nSocketEvent | nEvent);
}

PROTECTED
IMS_BOOL OsSocket::DeselectEventEx(IN IMS_SLONG nEvent)
{
    if ((m_nSocketEvent & nEvent) == 0)
    {
        return IMS_TRUE;
    }

    return SelectEvent(m_nSocketEvent & (~nEvent));
}

PROTECTED
void OsSocket::NotifyMessage(IN IMS_SINT32 nMsgParam)
{
    if (m_piOwnerThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "Owner thread is not set; Ignore message (%d-%d : %d)", m_hSocket,
                GetInternalSocketId(), nMsgParam);
        return;
    }

    IMS_UINT32 nLparam = IMS_SOCKET_MAKEPARAM(GetInternalSocketId(), m_hSocket);

    m_piOwnerThread->PostMessageI(IMS_MSG_SOCKET, nMsgParam, nLparam);
}

PROTECTED
IMS_SINT32 OsSocket::GetCloseReason() const
{
    return m_nCloseReason;
}

PROTECTED
void OsSocket::SetCloseReason(IN IMS_SINT32 nReason)
{
    if (m_nCloseReason != CLOSE_REASON_UNKNOWN)
    {
        return;
    }

    m_nCloseReason = nReason;
}

PROTECTED
IMS_SOCKET OsSocket::GetSocket() const
{
    return m_hSocket;
}

/**
 * PATCH_FOR_NON_SOCKET
 */
PROTECTED
ISocket::SOCKET_RESULT OsSocket::DoSocketRecovery()
{
    IMS_SOCKET hClosedSocket = m_hSocket;

    // Remove the existing socket (non-socket)
    if (SelectEvent(0))
    {
        OsSocketService::GetInstance()->KillSocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }

    UnbindSocketFromIpSecTransform(hClosedSocket);

    if (Open(GetSocketType(), m_eAddressFamily) == RESULT_ERROR)
    {
        IMS_TRACE_E(0, "Creating a new socket failed", 0, 0, 0);
        return RESULT_ERROR;
    }

    if (Bind(m_objSocketAddress, m_nSocketPort) == RESULT_ERROR)
    {
        IMS_TRACE_E(0, "Binding a socket (%d) failed", m_hSocket, 0, 0);
        return RESULT_ERROR;
    }

    BindSocketToIpSecTransform();

    return RESULT_SUCCESS;
}

PROTECTED
void OsSocket::BindSocketToIpSecTransform()
{
    if (GetSocketType() != TYPE_DGRAM)
    {
        return;
    }

    INetworkIpSec* piIpSec =
            NetworkService::GetNetworkService()->GetIpSec(m_piOwnerThread->GetSlotId());

    if (piIpSec != IMS_NULL)
    {
        IMS_TRACE_I("BindSocketToIpSecTransform: socket=%d", m_hSocket, 0, 0);

        SocketAddress objSockAddr(m_objSocketAddress, m_nSocketPort);

        piIpSec->ApplyIpSecTransform(this, objSockAddr, IMS_NULL);
    }
}

PROTECTED
void OsSocket::UnbindSocketFromIpSecTransform(IN IMS_SOCKET hSocket)
{
    INetworkIpSec* piIpSec =
            NetworkService::GetNetworkService()->GetIpSec(m_piOwnerThread->GetSlotId());

    if (piIpSec != IMS_NULL)
    {
        IMS_TRACE_I("UnbindSocketFromIpSecTransform: socket=%d", hSocket, 0, 0);

        piIpSec->RemoveIpSecTransforms(hSocket);
    }
}

#ifdef _DEBUG
PROTECTED
void OsSocket::OutputDebugString(
        IN IMS_SINT32 nErrorCode, IN const IMS_CHAR* pszModule, IN IMS_SINT32 nLine)
{
    AString strLog;

    strLog.Sprintf("socket=%d, error=%d(%s), code-line=%s:%d", m_hSocket, nErrorCode,
            strerror(nErrorCode), pszModule, nLine);

    IMS_TRACE_E(nErrorCode, "SocketError - %s", strLog.GetStr(), 0, 0);
}
#endif  // _DEBUG
