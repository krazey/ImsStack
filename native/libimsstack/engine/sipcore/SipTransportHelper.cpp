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
#include "ByteArray.h"
#include "ServiceMemory.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"

#include "ISipLocalDnsQueryListener.h"
#include "ISipTransportMessageListener.h"
#include "SipDatagramSocket.h"
#include "SipDebug.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipSocket.h"
#include "SipStreamSocket.h"
#include "SipStreamSocketNotifier.h"
#include "SipTransport.h"
#include "SipTransportHelper.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipTransportHelper::SipTransportHelper() :
        EngineActivity(),
        m_piMessageListener(IMS_NULL),
        m_objClientInitiatedConnections(ImsMap<IMS_UINTP, IMS_SINT32>()),
        m_piDnsQueryListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SipTransportHelper::~SipTransportHelper()
{
    if (!m_objSockets.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
        {
            SipSocket* pSocket = m_objSockets.GetAt(i);

            if (pSocket != IMS_NULL)
            {
                delete pSocket;
            }
        }
    }

    if (!m_objBuffers.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objBuffers.GetSize(); ++i)
        {
            TransportBuffer* pBuffer = m_objBuffers.GetAt(i);

            if (pBuffer != IMS_NULL)
            {
                delete pBuffer;
            }
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL SipTransportHelper::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_PROCESS_MESSAGE:
        {
            while (m_objBuffers.GetSize() > 0)
            {
                TransportBuffer* pBuffer = m_objBuffers.GetAt(0);

                if (pBuffer != IMS_NULL)
                {
                    if (m_piMessageListener != IMS_NULL)
                    {
                        m_piMessageListener->TransportMessage_PacketReceived(GetSlotId(),
                                pBuffer->m_objData, pBuffer->m_objNearEnd, pBuffer->m_objFarEnd);
                    }

                    m_objBuffers.RemoveAt(0);
                    delete pBuffer;
                }
                else
                {
                    m_objBuffers.RemoveAt(0);
                }
            }
        }
            return IMS_TRUE;

        case AMSG_DESTROY_ALL_SOCKETS:
        {
            IpAddress* pLocalIp = reinterpret_cast<IpAddress*>(objMsg.nLparam);

            if (pLocalIp != IMS_NULL)
            {
                DestroyAllSockets(0, *pLocalIp);
                delete pLocalIp;
            }
            else
            {
                DestroyAllSockets();
            }
            return IMS_TRUE;
        }

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PUBLIC
void SipTransportHelper::Clear()
{
    while (m_objSockets.GetSize() > 0)
    {
        SipSocket* pSocket = m_objSockets.GetAt(0);

        if (pSocket != IMS_NULL)
        {
            delete pSocket;
        }

        m_objSockets.RemoveAt(0);
    }

    m_objClientInitiatedConnections.Clear();
}

PUBLIC
SipSocket* SipTransportHelper::Create(IN const SipSocketAddress& objSockAddr)
{
    SipSocket* pSocket = Open(objSockAddr);

    if (pSocket != IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);

        IMS_TRACE_D("TransportHelper :: Socket (%s, %d, %d) will be re-used",
                SipDebug::GetIp(objSockAddr.GetIpAddress()), objSockAddr.GetPort(),
                objSockAddr.GetType());

        return pSocket;
    }

    if (objSockAddr.GetType() == SipSocketAddress::SOCKET_UDP)
    {
        SipDatagramSocket* pDatagramSocket = new SipDatagramSocket(GetSlotId());

        pDatagramSocket->SetListener(this);
        pSocket = pDatagramSocket;
    }
    else if (objSockAddr.GetType() == SipSocketAddress::SOCKET_TCP)
    {
        SipStreamSocketNotifier* pStreamSocketNotifier = new SipStreamSocketNotifier(GetSlotId());

        pStreamSocketNotifier->SetListener(this);
        pSocket = pStreamSocketNotifier;
    }
    else
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    if (!pSocket->Create(
                objSockAddr.GetIpAddress(), objSockAddr.GetPort(), objSockAddr.GetSecure()))
    {
        delete pSocket;

        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    if (!AttachSocket(pSocket))
    {
        delete pSocket;

        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return pSocket;
}

PUBLIC
SipSocket* SipTransportHelper::CreateStreamSocket(
        IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd)
{
    SipSocket* pSocket = OpenStreamSocket(objSockAddr, objFarEnd);

    if (pSocket != IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return pSocket;
    }

    if (objSockAddr.GetType() != SipSocketAddress::SOCKET_TCP_CLIENT)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    SipStreamSocket* pStreamSocket = new SipStreamSocket(GetSlotId());

    if (pStreamSocket == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    pStreamSocket->SetFarEnd(objFarEnd.GetIpAddress(), objFarEnd.GetPort());
    pStreamSocket->SetListener(this);

    if (!pStreamSocket->Create(
                objSockAddr.GetIpAddress(), objSockAddr.GetPort(), objSockAddr.GetSecure()))
    {
        delete pStreamSocket;

        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    if (!AttachSocket(pStreamSocket))
    {
        delete pStreamSocket;

        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return pStreamSocket;
}

PUBLIC
void SipTransportHelper::Destroy(IN SipSocket*& pSocket, IN ISipSocketListener* piListener)
{
    if (pSocket == IMS_NULL)
    {
        return;
    }

    if (!IsSocketPresent(pSocket))
    {
        IMS_TRACE_D("TransportHelper :: Socket(%p) is already destroyed", pSocket, 0, 0);
        return;
    }

    IMS_SINT32 nRefCount = 0;

    if ((nRefCount = pSocket->RemoveListener(piListener)) == 0)
    {
        IMS_SINT32 nType = pSocket->GetType();

        if ((nType == SipSocket::TYPE_TCP_CLIENT) || (nType == SipSocket::TYPE_TCP_CLIENT_OTHER))
        {
            SipStreamSocket* pStreamSocket = DYNAMIC_CAST(SipStreamSocket*, pSocket);

            if ((pStreamSocket->IsKeepAliveTimerActive() ||
                        pStreamSocket->IsKeepAlivePermanent()) &&
                    (pStreamSocket->GetState() == SipSocket::STATE_CONNECTED))
            {
                // The stream socket will be destroyed when the keep-alive timer is expired...
                pSocket = IMS_NULL;
                IMS_TRACE_D("TransportHelper :: Destroy (Keep-Alive) - Sockets (%d)",
                        m_objSockets.GetSize(), 0, 0);
                return;
            }
        }

        if (LookupSocket(*pSocket, IMS_TRUE) != IMS_NULL)
        {
            delete pSocket;
        }
        else
        {
            if (pSocket->IsSocketForcinlyClosed())
            {
                IMS_TRACE_D("TransportHelper :: Socket(%p) is not found; ignored", pSocket, 0, 0);
            }
            else
            {
                IMS_TRACE_D(
                        "TransportHelper :: Socket(%p) is not found; destroy it", pSocket, 0, 0);

                delete pSocket;
            }
        }

        pSocket = IMS_NULL;

        IMS_TRACE_D("TransportHelper :: Destroy - Sockets (%d)", m_objSockets.GetSize(), 0, 0);
    }
    else
    {
        IMS_TRACE_D("TransportHelper :: Destroy - Sockets (%d), RefCount (%d, %p)",
                m_objSockets.GetSize(), nRefCount, pSocket);
    }

    if (m_objSockets.IsEmpty())
    {
        m_objClientInitiatedConnections.Clear();
    }
}

PUBLIC
void SipTransportHelper::DestroyStreamSocket(
        IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd)
{
    SipSocket* pSocket = IMS_NULL;
    IMS_UINT32 nIndexOfSocket = 0xFFFF;

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        pSocket = m_objSockets.GetAt(i);

        if (pSocket->Equals(objFarEnd))
        {
            IpAddress objIp;
            IMS_UINT32 nPort = 0;

            pSocket->GetSockName(objIp, nPort);

            if (!objIp.Equals(objSockAddr.GetIpAddress()) ||
                    (nPort != static_cast<IMS_UINT32>(objSockAddr.GetPort())))
            {
                pSocket = IMS_NULL;
                continue;
            }

            nIndexOfSocket = i;
            break;
        }

        pSocket = IMS_NULL;
    }

    if (pSocket == IMS_NULL)
    {
        return;
    }

    SipStreamSocket* pStreamSocket = DYNAMIC_CAST(SipStreamSocket*, pSocket);

    if (pStreamSocket == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nRefCount = 0;
    IMS_UINTP nSocket = reinterpret_cast<IMS_UINTP>(pSocket);

    if ((nRefCount = pStreamSocket->RemoveListener(IMS_NULL)) == 0)
    {
        if (pStreamSocket->IsKeepAlivePermanent())
        {
            if (nIndexOfSocket < m_objSockets.GetSize())
            {
                m_objSockets.RemoveAt(nIndexOfSocket);
            }

            delete pStreamSocket;

            IMS_TRACE_D("TransportHelper :: DestroyStreamSocket - Destroyed", 0, 0, 0);
        }
    }
    else
    {
        if (pStreamSocket->IsKeepAlivePermanent())
        {
            pStreamSocket->DisableKeepAlive();
        }
    }

    IMS_TRACE_D("TransportHelper :: DestroyStreamSocket - Sockets(%d), RefCount(%d, 0x%" PFLS_x ")",
            m_objSockets.GetSize(), nRefCount, nSocket);

    if (m_objSockets.IsEmpty())
    {
        m_objClientInitiatedConnections.Clear();
    }
}

PUBLIC
SipSocket* SipTransportHelper::Open(IN const SipSocketAddress& objSockAddr)
{
    return LookupSocket(objSockAddr);
}

PUBLIC
SipSocket* SipTransportHelper::OpenStreamSocket(
        IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd)
{
    SipSocket* pSocket = IMS_NULL;

    // Look up the socket using the local socket information only
    if (objFarEnd.GetType() == SipSocketAddress::SOCKET_NONE)
    {
        pSocket = LookupStreamSocket(objSockAddr);
    }
    else
    {
        pSocket = LookupStreamSocket(objSockAddr, objFarEnd);
    }

    if (pSocket != IMS_NULL)
    {
        SipStreamSocket* pStreamSocket = DYNAMIC_CAST(SipStreamSocket*, pSocket);

        if (pStreamSocket != IMS_NULL)
        {
            // Start a keep-alive timer to re-use the stream socket
            pStreamSocket->ReuseSocket();
        }
    }

    return pSocket;
}

PUBLIC
IMS_SINT32 SipTransportHelper::AttachClientInitiatedConnection(IN SipSocket* pSocket)
{
    IMS_UINTP nSocketHandle = reinterpret_cast<IMS_UINTP>(pSocket);
    IMS_SLONG nIndex = m_objClientInitiatedConnections.GetIndexOfKey(nSocketHandle);

    if (nIndex < 0)
    {
        IMS_SINT32 nCount = 1;
        m_objClientInitiatedConnections.Add(nSocketHandle, nCount);
        IMS_TRACE_D("TransportHelper :: Attach - TCP socket=%" PFLS_x, nSocketHandle, 0, 0);
        return 1;
    }

    IMS_SINT32& nCount = m_objClientInitiatedConnections.GetValueAt(nIndex);

    nCount++;

    return nCount;
}

PUBLIC
void SipTransportHelper::DetachClientInitiatedConnection(IN SipSocket* pSocket)
{
    IMS_UINTP nSocketHandle = reinterpret_cast<IMS_UINTP>(pSocket);
    IMS_SLONG nIndex = m_objClientInitiatedConnections.GetIndexOfKey(nSocketHandle);

    if (nIndex < 0)
    {
        return;
    }

    IMS_SINT32& nCount = m_objClientInitiatedConnections.GetValueAt(nIndex);

    nCount--;

    if (nCount <= 0)
    {
        m_objClientInitiatedConnections.RemoveAt(nIndex);
    }

    IMS_TRACE_D("TransportHelper :: Detach - TCP socket=%" PFLS_x "(%d)", nSocketHandle, nCount, 0);
}

PUBLIC
IMS_BOOL SipTransportHelper::IsClientInitiatedConnection(IN SipSocket* pSocket) const
{
    IMS_SLONG nIndex =
            m_objClientInitiatedConnections.GetIndexOfKey(reinterpret_cast<IMS_UINTP>(pSocket));

    return (nIndex >= 0);
}

PUBLIC
IMS_BOOL SipTransportHelper::GetHostByName(
        IN const IpAddress& objLocalIp, IN const AString& strHostname, OUT IpAddress& objHostIp)
{
    if (m_piDnsQueryListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_piDnsQueryListener->LocalDnsQuery_GetHostByName(objLocalIp, strHostname, objHostIp);
}

PRIVATE VIRTUAL void SipTransportHelper::ApplyIpSecForServerSockets()
{
    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);
        IMS_SINT32 nSocketType = pSocket->GetType();

        if (nSocketType == SipSocket::TYPE_TCP || nSocketType == SipSocket::TYPE_UDP)
        {
            pSocket->ApplyIpSec();
        }
    }
}

PRIVATE VIRTUAL void SipTransportHelper::DestroyAllSockets(
        IN IMS_SINT32 nMethod /*= 0*/, IN const IpAddress& objLocalIp /*= IpAddress::NONE*/)
{
    if (nMethod == 0)
    {
        IMS_TRACE_D("DestroyAllSockets (S) :: Sockets (%d)", m_objSockets.GetSize(), 0, 0);

        ImsList<SipSocket*> objTmpSockets = m_objSockets;

        for (IMS_UINT32 i = 0; i < objTmpSockets.GetSize(); ++i)
        {
            SipSocket* pSocket = objTmpSockets.GetAt(i);

            if (pSocket != IMS_NULL)
            {
                if (!objLocalIp.IsNoneAddress())
                {
                    IpAddress objIp(IpAddress::NONE);
                    IMS_UINT32 nPort = 0;

                    pSocket->GetSockName(objIp, nPort);

                    if (!objLocalIp.Equals(objIp) && !objIp.IsNoneAddress())
                    {
                        continue;
                    }
                }

                pSocket->NotifyForceClosed();
            }
        }

        Clear();

        IMS_TRACE_D("DestroyAllSockets (E) :: Sockets (%d)", m_objSockets.GetSize(), 0, 0);
    }
    else
    {
        IpAddress* pLocalIp = new IpAddress(objLocalIp);
        PostMessage(AMSG_DESTROY_ALL_SOCKETS, 0, reinterpret_cast<IMS_UINTP>(pLocalIp));
    }
}

PRIVATE VIRTUAL void SipTransportHelper::DestroyTcpSocket(IN const IpAddress& objSrcIp,
        IN IMS_UINT32 nSrcPort, IN const IpAddress& objDstIp, IN IMS_UINT32 nDstPort,
        IN IMS_BOOL bIsConnectionByPeer /*= IMS_FALSE*/)
{
    ImsList<SipSocket*> objRemovedSockets;
    SipSocketAddress objSockAddr;

    IMS_TRACE_D("DestroyTcpSocket (S) :: Sockets (%d)", m_objSockets.GetSize(), 0, 0);

    if (!bIsConnectionByPeer)
    {
        objSockAddr.SetType(SipSocketAddress::SOCKET_TCP_CLIENT);
    }
    else
    {
        objSockAddr.SetType(SipSocketAddress::SOCKET_TCP_CLIENT_BY_PEER);
    }

    objSockAddr.SetPort(nDstPort);
    objSockAddr.SetIpAddress(objDstIp);

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize();)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if ((pSocket != IMS_NULL) && pSocket->Equals(objSockAddr))
        {
            IMS_BOOL bFound = IMS_TRUE;

            if (!objSrcIp.IsNoneAddress())
            {
                IpAddress objIp;
                IMS_UINT32 nPort = 0;

                pSocket->GetSockName(objIp, nPort);

                if (objSrcIp.Equals(objIp))
                {
                    if ((nSrcPort != 0) && (nSrcPort != nPort))
                    {
                        bFound = IMS_FALSE;
                    }
                }
                else
                {
                    bFound = IMS_FALSE;
                }
            }

            if (bFound)
            {
                m_objSockets.RemoveAt(i);
                objRemovedSockets.Append(pSocket);

                pSocket->NotifyForceClosed();
            }
            else
            {
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }

    for (IMS_UINT32 i = 0; i < objRemovedSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = objRemovedSockets.GetAt(i);

        if (pSocket != IMS_NULL)
        {
            delete pSocket;
        }
    }

    IMS_TRACE_D("DestroyTcpSocket (E) :: Sockets (%d)", m_objSockets.GetSize(), 0, 0);

    if (m_objSockets.IsEmpty())
    {
        m_objClientInitiatedConnections.Clear();
    }
}

PRIVATE VIRTUAL void SipTransportHelper::SetIpQos(IN SipRtConfig::IpQos* pIpQos)
{
    if (pIpQos == IMS_NULL)
    {
        return;
    }

    if (pIpQos->objIpAddr.Equals(IpAddress::NONE) || pIpQos->objIpAddr.Equals(IpAddress::IPv6NONE))
    {
        for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
        {
            SipSocket* pSocket = m_objSockets.GetAt(i);

            if (pSocket != IMS_NULL)
            {
                pSocket->SetOption(ISocket::OPT_IP_QOS, pIpQos->nValue);
            }
        }
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if (pSocket == IMS_NULL)
        {
            continue;
        }

        IMS_BOOL bMatched = IMS_FALSE;
        IpAddress objIp;
        IMS_UINT32 nPort = 0;

        pSocket->GetSockName(objIp, nPort);

        if (pIpQos->objIpAddr.Equals(objIp))
        {
            bMatched = IMS_TRUE;

            if ((pIpQos->nPort != 0) && (pIpQos->nPort != static_cast<IMS_SINT32>(nPort)))
            {
                bMatched = IMS_FALSE;
            }
        }

        if (bMatched)
        {
            pSocket->SetOption(ISocket::OPT_IP_QOS, pIpQos->nValue);
        }
    }
}

PRIVATE VIRTUAL void SipTransportHelper::SetKeepAlivePolicy(IN const IpAddress& objSrcIp,
        IN IMS_UINT32 nSrcPort, IN const IpAddress& objDstIp, IN IMS_UINT32 nDstPort,
        IN IMS_SINT32 nPolicy /*= (-1) default*/)
{
    SipSocketAddress objSockAddr;

    objSockAddr.SetType(SipSocketAddress::SOCKET_TCP_CLIENT);
    objSockAddr.SetPort(nDstPort);
    objSockAddr.SetIpAddress(objDstIp);

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if ((pSocket != IMS_NULL) && pSocket->Equals(objSockAddr))
        {
            IMS_BOOL bFound = IMS_TRUE;

            if (!objSrcIp.IsNoneAddress())
            {
                IpAddress objIp;
                IMS_UINT32 nPort = 0;

                pSocket->GetSockName(objIp, nPort);

                if (objSrcIp.Equals(objIp))
                {
                    if ((nSrcPort != 0) && (nSrcPort != nPort))
                    {
                        bFound = IMS_FALSE;
                    }
                }
                else
                {
                    bFound = IMS_FALSE;
                }
            }

            if (bFound)
            {
                SipStreamSocket* pStreamSocket = DYNAMIC_CAST(SipStreamSocket*, pSocket);

                if (pStreamSocket != IMS_NULL)
                {
                    pStreamSocket->SetKeepAlivePolicy(nPolicy);
                }
                break;
            }
        }
    }
}

PRIVATE VIRTUAL void SipTransportHelper::SetLocalDnsQueryListener(
        IN ISipLocalDnsQueryListener* piListener)
{
    m_piDnsQueryListener = piListener;
}

PRIVATE VIRTUAL void SipTransportHelper::DatagramSocket_DataReceived(IN SipSocket* pSocket,
        IN const ByteArray& objBuffer, IN const IpAddress& objIp, IN IMS_SINT32 nPort)
{
    if (objBuffer.GetLength() == 0)
    {
        return;
    }

    TransportBuffer* pBuffer = new TransportBuffer();

    if (pBuffer == IMS_NULL)
    {
        return;
    }

    pBuffer->m_objData = objBuffer;

    // Sets a packet source transport information
    pBuffer->m_objFarEnd.SetProtocol(SipTransportAddress::PROTOCOL_UDP);
    pBuffer->m_objFarEnd.SetPort(nPort);
    pBuffer->m_objFarEnd.SetIpAddress(objIp);

    IpAddress objSockIp;
    IMS_UINT32 nSockPort;

    // Sets a packet destination(my) transport information
    pSocket->GetSockName(objSockIp, nSockPort);
    pBuffer->m_objNearEnd.SetProtocol(SipTransportAddress::PROTOCOL_UDP);
    pBuffer->m_objNearEnd.SetPort(nSockPort);
    pBuffer->m_objNearEnd.SetIpAddress(objSockIp);

    // DEBUG ...
    pBuffer->DisplayMessage(GetSlotId());

    if (!m_objBuffers.Append(pBuffer))
    {
        delete pBuffer;
        return;
    }

    // Send event to process the raw SIP message
    PostMessage(AMSG_PROCESS_MESSAGE, 0, 0);
}

PRIVATE VIRTUAL void SipTransportHelper::StreamSocket_ConnectionReceived(IN SipSocket* pSocket)
{
    SipSocket* pNewSocket = pSocket->Accept();

    if (pNewSocket != IMS_NULL)
    {
        if (!AttachSocket(pNewSocket))
        {
            delete pNewSocket;
        }
    }
}

PRIVATE VIRTUAL void SipTransportHelper::StreamSocket_DataReceived(
        IN SipSocket* pSocket, IN_OUT ByteArray& objBuffer)
{
    if (pSocket == IMS_NULL)
    {
        return;
    }

    if (objBuffer.GetLength() == 0)
    {
        return;
    }

    TransportBuffer* pBuffer = new TransportBuffer();

    if (pBuffer != IMS_NULL)
    {
        IpAddress objTmpIp;
        IMS_UINT32 nTmpPort;
        IMS_SINT32 nTransportProtocol = SipTransportAddress::PROTOCOL_TCP;
        SipStreamSocket* pStreamSocket = DYNAMIC_CAST(SipStreamSocket*, pSocket);

        if ((pStreamSocket != IMS_NULL) && pStreamSocket->IsSecureSocket())
        {
            nTransportProtocol = SipTransportAddress::PROTOCOL_TLS;
        }

        pBuffer->m_objData = objBuffer;

        // Sets a packet source transport information
        pSocket->GetPeerName(objTmpIp, nTmpPort);

        pBuffer->m_objFarEnd.SetProtocol(nTransportProtocol);
        pBuffer->m_objFarEnd.SetPort(nTmpPort);
        pBuffer->m_objFarEnd.SetIpAddress(objTmpIp);

        // Sets a packet destination (my) transport information
        pSocket->GetSockName(objTmpIp, nTmpPort);

        pBuffer->m_objNearEnd.SetProtocol(nTransportProtocol);
        pBuffer->m_objNearEnd.SetPort(nTmpPort);
        pBuffer->m_objNearEnd.SetIpAddress(objTmpIp);

        // DEBUG ...
        pBuffer->DisplayMessage(GetSlotId());

        if (!m_objBuffers.Append(pBuffer))
        {
            delete pBuffer;
            return;
        }

        // Send event to process the message
        PostMessage(AMSG_PROCESS_MESSAGE, 0, 0);
    }
}

PRIVATE VIRTUAL void SipTransportHelper::StreamSocket_KeepAliveExpired(IN SipSocket* pSocket)
{
    if (!IsSocketPresent(pSocket))
    {
        return;
    }

    if (pSocket->RemoveListener(IMS_NULL) == 0)
    {
        LookupSocket(*pSocket, IMS_TRUE);
        delete pSocket;
        IMS_TRACE_D("TransportHelper :: Destroy (Keep-Alive Expired) - Sockets (%d)",
                m_objSockets.GetSize(), 0, 0);
    }
}

PRIVATE VIRTUAL void SipTransportHelper::StreamSocket_PassiveClosed(IN SipSocket* pSocket)
{
    if (!IsSocketPresent(pSocket))
    {
        return;
    }

    if (IsClientInitiatedConnection(pSocket))
    {
        IMS_TRACE_D("SipTransportHelper :: Managed client initiated connection is required...", 0,
                0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pTmpSocket = m_objSockets.GetAt(i);

        if (pSocket == pTmpSocket)
        {
            m_objSockets.RemoveAt(i);
            delete pSocket;
            IMS_TRACE_D("TransportHelper :: Destroy (Passive Closed) - Sockets (%d)",
                    m_objSockets.GetSize(), 0, 0);
            return;
        }
    }
}

PRIVATE
IMS_BOOL SipTransportHelper::AttachSocket(IN SipSocket* pSocket)
{
    return m_objSockets.Append(pSocket);
}

PRIVATE
IMS_BOOL SipTransportHelper::IsSocketPresent(IN const SipSocket* pSocket) const
{
    if (pSocket != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
        {
            SipSocket* pTmpSocket = m_objSockets.GetAt(i);

            if (pSocket == pTmpSocket)
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
SipSocket* SipTransportHelper::LookupSocket(
        IN const SipSocketAddress& objSockAddr, IN IMS_BOOL bDetach /*= IMS_FALSE*/)
{
    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if (pSocket->Equals(objSockAddr))
        {
            if (bDetach == IMS_TRUE)
            {
                m_objSockets.RemoveAt(i);
            }

            return pSocket;
        }
    }

    return IMS_NULL;
}

PRIVATE
SipSocket* SipTransportHelper::LookupSocket(
        IN const SipSocket& objSocket, IN IMS_BOOL bDetach /*= IMS_FALSE*/)
{
    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if (pSocket->Equals(objSocket))
        {
            IMS_SINT32 nType = pSocket->GetType();

            if ((nType == SipSocketAddress::SOCKET_TCP_CLIENT) ||
                    (nType == SipSocketAddress::SOCKET_TCP_CLIENT_BY_PEER))
            {
                IpAddress objIp;
                IpAddress objOtherIp;
                IMS_UINT32 nPort = 0;
                IMS_UINT32 nOtherPort = 0;
                SipSocket* pOtherSocket = const_cast<SipSocket*>(&objSocket);

                pSocket->GetSockName(objIp, nPort);
                pOtherSocket->GetSockName(objOtherIp, nOtherPort);

                if (!objIp.Equals(objOtherIp) || (nPort != nOtherPort))
                {
                    continue;
                }

                // CASE :: system socket is already closed (no socket handle)
                // To avoid the crash, it just checks the address of socket object.
                if ((pSocket != pOtherSocket) && (nPort == 0))
                {
                    // It's different socket...
                    continue;
                }
            }

            if (bDetach == IMS_TRUE)
            {
                m_objSockets.RemoveAt(i);
            }

            return pSocket;
        }
    }

    return IMS_NULL;
}

PRIVATE
SipSocket* SipTransportHelper::LookupStreamSocket(IN const SipSocketAddress& objSockAddr)
{
    IpAddress objIp;
    IMS_UINT32 nPort = 0;
    SipSocket* pStreamSocket = IMS_NULL;
    AString strNearEnd;

    strNearEnd.Sprintf("Near End (%s, %d)", SipDebug::GetIp(objSockAddr.GetIpAddress()),
            objSockAddr.GetPort());

    // Find a socket using the source address & port again
    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if (pSocket->GetType() != objSockAddr.GetType())
        {
            continue;
        }

        pSocket->GetSockName(objIp, nPort);

        IMS_TRACE_D("Lookup :: TCP client (%s, %d) at (%d)", SipDebug::GetIp(objIp), nPort, i);

        if (objIp.Equals(objSockAddr.GetIpAddress()) &&
                (nPort == static_cast<IMS_UINT32>(objSockAddr.GetPort())) &&
                !pSocket->IsClosedOrBeingClosed())
        {
            IMS_TRACE_D("Lookup :: TCP client (%s, %d), %s", SipDebug::GetIp(objIp), nPort,
                    strNearEnd.GetStr());
            return pSocket;
        }
    }

    if (pStreamSocket == IMS_NULL)
    {
        IMS_TRACE_D("Lookup :: TCP client (not found) (%s, %d), %s",
                SipDebug::GetIp(objSockAddr.GetIpAddress()), objSockAddr.GetPort(),
                strNearEnd.GetStr());
    }

    return pStreamSocket;
}

PRIVATE
SipSocket* SipTransportHelper::LookupStreamSocket(
        IN const SipSocketAddress& objSockAddr, IN const SipSocketAddress& objFarEnd)
{
    ImsList<SipSocket*> objCandidates;
    AString strFarEnd;

    // LOG_EXCLUDING_SERVER_INFO
    strFarEnd.Sprintf("Far End (%s, %d)",
            SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                    ? "xxx"
                    : SipDebug::GetIp(objFarEnd.GetIpAddress()),
            objFarEnd.GetPort());

    // Find a socket using the destination address & port
    for (IMS_UINT32 i = 0; i < m_objSockets.GetSize(); ++i)
    {
        SipSocket* pSocket = m_objSockets.GetAt(i);

        if (pSocket->Equals(objFarEnd) && !pSocket->IsClosedOrBeingClosed())
        {
            objCandidates.Prepend(pSocket);
        }
    }

    if (objCandidates.IsEmpty())
    {
        IMS_TRACE_D("Lookup :: TCP client does not exist; %s", strFarEnd.GetStr(), 0, 0);
        return IMS_NULL;
    }

    IpAddress objIp;
    IMS_UINT32 nPort = 0;
    SipSocket* pStreamSocket = IMS_NULL;

    // Find a socket using the source address & port again
    for (IMS_UINT32 i = 0; i < objCandidates.GetSize(); ++i)
    {
        SipSocket* pSocket = objCandidates.GetAt(i);

        pSocket->GetSockName(objIp, nPort);

        IMS_TRACE_D("Lookup :: TCP client (%s, %d) at (%d)", SipDebug::GetIp(objIp), nPort, i);

        if (objIp.Equals(objSockAddr.GetIpAddress()) &&
                (nPort == static_cast<IMS_UINT32>(objSockAddr.GetPort())))
        {
            IMS_TRACE_D("Lookup :: TCP client (%s, %d), %s", SipDebug::GetIp(objIp), nPort,
                    strFarEnd.GetStr());
            return pSocket;
        }

        // Find a socket using the source address (for TCP client socket w/ random port)
        if ((pStreamSocket == IMS_NULL) && (objSockAddr.GetPort() <= 0))
        {
            if (objIp.Equals(objSockAddr.GetIpAddress()))
            {
                pStreamSocket = pSocket;
                IMS_TRACE_D("Lookup :: TCP client (%s, %d) w/o port, %s", SipDebug::GetIp(objIp),
                        nPort, strFarEnd.GetStr());
            }
        }
    }

    if (pStreamSocket == IMS_NULL)
    {
        IMS_TRACE_D("Lookup :: TCP client (not found) (%s, %d), %s",
                SipDebug::GetIp(objSockAddr.GetIpAddress()), objSockAddr.GetPort(),
                strFarEnd.GetStr());
    }

    if (objCandidates.GetSize() > 1)
    {
        IMS_TRACE_D(
                "Lookup :: TCP client socket is ambiguous ... (%d)", objCandidates.GetSize(), 0, 0);
    }

    return pStreamSocket;
}

PRIVATE
void SipTransportHelper::TransportBuffer::DisplayMessage(IN IMS_SINT32 nSlotId)
{
    SipTransport::PrintMessage(nSlotId, IMS_FALSE, m_objFarEnd,
            reinterpret_cast<const IMS_CHAR*>(m_objData.GetData()), m_objData.GetLength());
}
