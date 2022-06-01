/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceSystemTime.h"
#include "ByteArray.h"
#include "SipPrivate.h"
#include "SipDebug.h"
#include "SipRtConfigUtils.h"
#include "ISipTransportListener.h"
#include "ISipLocalDnsQueryListener.h"
#include "SipSocket.h"
#include "SipDatagramSocket.h"
#include "SipStreamSocketNotifier.h"
#include "SipStreamSocket.h"
#include "SipTransport.h"
#include "SipTransportHelper.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPTransportHelper::SIPTransportHelper() :
        EngineActivity(),
        piListener(IMS_NULL),
        objClientInitiatedConnections(IMSMap<IMS_UINTP, IMS_SINT32>()),
        piDnsQueryListener(IMS_NULL)
{
}

PUBLIC VIRTUAL SIPTransportHelper::~SIPTransportHelper()
{
    if (!objSockets.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
        {
            SIPSocket* pSocket = objSockets.GetAt(i);

            if (pSocket != IMS_NULL)
                delete pSocket;
        }
    }

    if (!objBuffers.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objBuffers.GetSize(); ++i)
        {
            TransportBuffer* pBuffer = objBuffers.GetAt(i);

            if (pBuffer != IMS_NULL)
                delete pBuffer;
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPTransportHelper::DispatchMessage(IN IMSMSG& objMSG)
{
    switch (objMSG.GetName())
    {
        case AMSG_PROCESS_MESSAGE:
        {
            while (objBuffers.GetSize() > 0)
            {
                TransportBuffer* pBuffer = objBuffers.GetAt(0);

                if (pBuffer != IMS_NULL)
                {
                    if (piListener != IMS_NULL)
                    {
                        piListener->Transport_PacketReceived(GetSlotId(), pBuffer->objData,
                                pBuffer->objTA_NearEnd, pBuffer->objTA_FarEnd);
                    }

                    objBuffers.RemoveAt(0);
                    delete pBuffer;
                }
                else
                {
                    objBuffers.RemoveAt(0);
                }
            }
        }
            return IMS_TRUE;

        case AMSG_DESTROY_ALL_SOCKETS:
        {
            IPAddress* pLocalIP = reinterpret_cast<IPAddress*>(objMSG.nLparam);

            if (pLocalIP != IMS_NULL)
            {
                DestroyAllSockets(0, *pLocalIP);
                delete pLocalIP;
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

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
PUBLIC
void SIPTransportHelper::Clear()
{
    //---------------------------------------------------------------------------------------------

    while (objSockets.GetSize() > 0)
    {
        SIPSocket* pSocket = objSockets.GetAt(0);

        if (pSocket != IMS_NULL)
            delete pSocket;

        objSockets.RemoveAt(0);
    }

    objClientInitiatedConnections.Clear();
}

/*

Remarks

*/
PUBLIC
SIPSocket* SIPTransportHelper::Create(IN CONST SIPSocketAddress& objSA)
{
    SIPSocket* pSocket = Open(objSA);

    //---------------------------------------------------------------------------------------------

    if (pSocket != IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_ERROR);

        IMS_TRACE_D("TransportHelper :: Socket (%s, %d, %d) will be re-used",
                SipDebug::GetIp(objSA.GetIPAddress()), objSA.GetPort(), objSA.GetType());

        return pSocket;
    }

    if (objSA.GetType() == SIPSocketAddress::SOCKET_UDP)
    {
        SIPDatagramSocket* pDatagramSocket = new SIPDatagramSocket(GetSlotId());

        pDatagramSocket->SetListener(this);
        pSocket = pDatagramSocket;
    }
    else if (objSA.GetType() == SIPSocketAddress::SOCKET_TCP)
    {
        SIPStreamSocketNotifier* pStreamSocketNotifier = new SIPStreamSocketNotifier(GetSlotId());

        pStreamSocketNotifier->SetListener(this);
        pSocket = pStreamSocketNotifier;
    }
    else
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    if (!pSocket->Create(objSA.GetIPAddress(), objSA.GetPort(), objSA.GetSecure()))
    {
        delete pSocket;

        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    if (!AttachSocket(pSocket))
    {
        delete pSocket;

        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    return pSocket;
}

/*

Remarks

*/
PUBLIC
SIPSocket* SIPTransportHelper::CreateStreamSocket(
        IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd)
{
    SIPSocket* pSocket = OpenStreamSocket(objSA, objSA_FarEnd);

    //---------------------------------------------------------------------------------------------

    if (pSocket != IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_ERROR);
        return pSocket;
    }

    if (objSA.GetType() != SIPSocketAddress::SOCKET_TCP_CLIENT)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_NULL;
    }

    SIPStreamSocket* pStreamSocket = new SIPStreamSocket(GetSlotId());

    if (pStreamSocket == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    pStreamSocket->SetFarEnd(objSA_FarEnd.GetIPAddress(), objSA_FarEnd.GetPort());
    pStreamSocket->SetListener(this);

    if (!pStreamSocket->Create(objSA.GetIPAddress(), objSA.GetPort(), objSA.GetSecure()))
    {
        delete pStreamSocket;

        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    if (!AttachSocket(pStreamSocket))
    {
        delete pStreamSocket;

        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_NULL;
    }

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    return pStreamSocket;
}

/*

Remarks

*/
PUBLIC
void SIPTransportHelper::Destroy(IN SIPSocket*& pSocket, IN ISIPSocketListener* piListener)
{
    //---------------------------------------------------------------------------------------------

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

        if ((nType == SIPSocket::TYPE_TCP_CLIENT) || (nType == SIPSocket::TYPE_TCP_CLIENT_OTHER))
        {
            SIPStreamSocket* pStreamSocket = DYNAMIC_CAST(SIPStreamSocket*, pSocket);

            if ((pStreamSocket->IsKeepAliveTimerActive() ||
                        pStreamSocket->IsKeepAlivePermanent()) &&
                    (pStreamSocket->GetState() == SIPSocket::STATE_CONNECTED))
            {
                // The stream socket will be destroyed when the keep-alive timer is expired...
                pSocket = IMS_NULL;
                IMS_TRACE_D("TransportHelper :: Destroy (Keep-Alive) - Sockets (%d)",
                        objSockets.GetSize(), 0, 0);
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

        IMS_TRACE_D("TransportHelper :: Destroy - Sockets (%d)", objSockets.GetSize(), 0, 0);
    }
    else
    {
        IMS_TRACE_D("TransportHelper :: Destroy - Sockets (%d), RefCount (%d, %p)",
                objSockets.GetSize(), nRefCount, pSocket);
    }

    if (objSockets.IsEmpty())
    {
        objClientInitiatedConnections.Clear();
    }
}

/*

Remarks

*/
PUBLIC
void SIPTransportHelper::DestroyStreamSocket(
        IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd)
{
    SIPSocket* pSocket = IMS_NULL;
    IMS_UINT32 nIndexOfSocket = 0xFFFF;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        pSocket = objSockets.GetAt(i);

        if (pSocket->Equals(objSA_FarEnd))
        {
            IPAddress objIP;
            IMS_UINT32 nPort = 0;

            pSocket->GetSockName(objIP, nPort);

            if (!objIP.Equals(objSA.GetIPAddress()) ||
                    (nPort != static_cast<IMS_UINT32>(objSA.GetPort())))
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

    SIPStreamSocket* pStreamSocket = DYNAMIC_CAST(SIPStreamSocket*, pSocket);

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
            if (nIndexOfSocket < objSockets.GetSize())
            {
                objSockets.RemoveAt(nIndexOfSocket);
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

    IMS_TRACE_D("TransportHelper :: DestroyStreamSocket - Sockets (%d), RefCount (%d, 0x%" PFLS_x
                ")",
            objSockets.GetSize(), nRefCount, nSocket);

    if (objSockets.IsEmpty())
    {
        objClientInitiatedConnections.Clear();
    }
}

/*

Remarks

*/
PUBLIC
SIPSocket* SIPTransportHelper::Open(IN CONST SIPSocketAddress& objSA)
{
    //---------------------------------------------------------------------------------------------

    return LookupSocket(objSA);
}

/*

Remarks

*/
PUBLIC
SIPSocket* SIPTransportHelper::OpenStreamSocket(
        IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd)
{
    SIPSocket* pSocket = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    // Look up the socket using the local socket information only
    if (objSA_FarEnd.GetType() == SIPSocketAddress::SOCKET_NONE)
    {
        pSocket = LookupStreamSocket(objSA);
    }
    else
    {
        pSocket = LookupStreamSocket(objSA, objSA_FarEnd);
    }

    if (pSocket != IMS_NULL)
    {
        SIPStreamSocket* pStreamSocket = DYNAMIC_CAST(SIPStreamSocket*, pSocket);

        if (pStreamSocket != IMS_NULL)
        {
            // Start a keep-alive timer to re-use the stream socket
            pStreamSocket->ReuseSocket();
        }
    }

    return pSocket;
}

/*

Remarks

*/
PUBLIC
void SIPTransportHelper::SetListener(IN ISIPTransportListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks
 MULTI_REG_TRANSPORT
*/
PUBLIC
IMS_SINT32 SIPTransportHelper::AttachClientInitiatedConnection(IN SIPSocket* pSocket)
{
    IMS_UINTP nSocketHandle = reinterpret_cast<IMS_UINTP>(pSocket);
    IMS_SLONG nIndex = objClientInitiatedConnections.GetIndexOfKey(nSocketHandle);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        IMS_SINT32 nCount = 1;
        objClientInitiatedConnections.Add(nSocketHandle, nCount);
        IMS_TRACE_D("TransportHelper :: Attach - TCP socket=%" PFLS_x, nSocketHandle, 0, 0);
        return 1;
    }

    IMS_SINT32& nCount = objClientInitiatedConnections.GetValueAt(nIndex);

    nCount++;

    return nCount;
}

/*

Remarks
 MULTI_REG_TRANSPORT
*/
PUBLIC
void SIPTransportHelper::DetachClientInitiatedConnection(IN SIPSocket* pSocket)
{
    IMS_UINTP nSocketHandle = reinterpret_cast<IMS_UINTP>(pSocket);
    IMS_SLONG nIndex = objClientInitiatedConnections.GetIndexOfKey(nSocketHandle);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return;
    }

    IMS_SINT32& nCount = objClientInitiatedConnections.GetValueAt(nIndex);

    nCount--;

    if (nCount <= 0)
    {
        objClientInitiatedConnections.RemoveAt(nIndex);
    }

    IMS_TRACE_D("TransportHelper :: Detach - TCP socket=%" PFLS_x "(%d)", nSocketHandle, nCount, 0);
}

/*

Remarks
 MULTI_REG_TRANSPORT
*/
PUBLIC
IMS_BOOL SIPTransportHelper::IsClientInitiatedConnection(IN SIPSocket* pSocket) const
{
    IMS_SLONG nIndex =
            objClientInitiatedConnections.GetIndexOfKey(reinterpret_cast<IMS_UINTP>(pSocket));

    //---------------------------------------------------------------------------------------------

    return (nIndex >= 0);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPTransportHelper::GetHostByName(
        IN CONST IPAddress& objLocalIP, IN CONST AString& strHostname, OUT IPAddress& objHostIP)
{
    //---------------------------------------------------------------------------------------------

    if (piDnsQueryListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piDnsQueryListener->LocalDnsQuery_GetHostByName(objLocalIP, strHostname, objHostIP);
}

PRIVATE VIRTUAL void SIPTransportHelper::ApplyIpSecForServerSockets()
{
    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); i++)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);
        IMS_SINT32 nSocketType = pSocket->GetType();

        if (nSocketType == SIPSocket::TYPE_TCP || nSocketType == SIPSocket::TYPE_UDP)
        {
            pSocket->ApplyIpSec();
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::DestroyAllSockets(
        IN IMS_SINT32 nMethod /* = 0 */, IN CONST IPAddress& objLocalIP /* = IPAddress::NONE*/)
{
    //---------------------------------------------------------------------------------------------

    if (nMethod == 0)
    {
        IMS_TRACE_D("DestroyAllSockets (S) :: Sockets (%d)", objSockets.GetSize(), 0, 0);

        IMSList<SIPSocket*> objTmpSockets = objSockets;

        for (IMS_UINT32 i = 0; i < objTmpSockets.GetSize(); ++i)
        {
            SIPSocket* pSocket = objTmpSockets.GetAt(i);

            if (pSocket != IMS_NULL)
            {
                if (!objLocalIP.IsNoneAddress())
                {
                    IPAddress objIP(IPAddress::NONE);
                    IMS_UINT32 nPort = 0;

                    pSocket->GetSockName(objIP, nPort);

                    if (objLocalIP.Equals(objIP))
                    {
                        pSocket->NotifyForceClosed();
                    }
                    else if (objIP.IsNoneAddress())
                    {
                        pSocket->NotifyForceClosed();
                    }
                }
                else
                {
                    pSocket->NotifyForceClosed();
                }
            }
        }

        Clear();

        IMS_TRACE_D("DestroyAllSockets (E) :: Sockets (%d)", objSockets.GetSize(), 0, 0);
    }
    else
    {
        IPAddress* pLocalIP = new IPAddress(objLocalIP);
        PostMessage(AMSG_DESTROY_ALL_SOCKETS, 0, reinterpret_cast<IMS_UINTP>(pLocalIP));
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::DestroyTcpSocket(IN CONST IPAddress& objSrcIP,
        IN IMS_UINT32 nSrcPort, IN CONST IPAddress& objDestIP, IN IMS_UINT32 nDestPort,
        IN IMS_BOOL bIsConnectionByPeer /* = IMS_FALSE */)
{
    IMSList<SIPSocket*> objRemovedSockets;
    SIPSocketAddress objSA;

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("DestroyTcpSocket (S) :: Sockets (%d)", objSockets.GetSize(), 0, 0);

    if (!bIsConnectionByPeer)
    {
        objSA.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);
    }
    else
    {
        objSA.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT_BY_PEER);
    }

    objSA.SetPort(nDestPort);
    objSA.SetIPAddress(objDestIP);

    for (IMS_UINT32 i = 0; i < objSockets.GetSize();)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if ((pSocket != IMS_NULL) && pSocket->Equals(objSA))
        {
            IMS_BOOL bFound = IMS_TRUE;

            if (!objSrcIP.IsNoneAddress())
            {
                IPAddress objIP;
                IMS_UINT32 nPort = 0;

                pSocket->GetSockName(objIP, nPort);

                if (objSrcIP.Equals(objIP))
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
                objSockets.RemoveAt(i);
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
        SIPSocket* pSocket = objRemovedSockets.GetAt(i);

        if (pSocket != IMS_NULL)
        {
            delete pSocket;
        }
    }

    IMS_TRACE_D("DestroyTcpSocket (E) :: Sockets (%d)", objSockets.GetSize(), 0, 0);

    if (objSockets.IsEmpty())
    {
        objClientInitiatedConnections.Clear();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::SetIpQos(IN SipRtConfig::IpQos* pIPQoS)
{
    //---------------------------------------------------------------------------------------------

    if (pIPQoS == IMS_NULL)
    {
        return;
    }

    if (pIPQoS->objIpAddr.Equals(IPAddress::NONE) || pIPQoS->objIpAddr.Equals(IPAddress::IPv6NONE))
    {
        for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
        {
            SIPSocket* pSocket = objSockets.GetAt(i);

            if (pSocket != IMS_NULL)
            {
                pSocket->SetOption(ISocket::OPT_IP_QOS, pIPQoS->nValue);
            }
        }
        return;
    }

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if (pSocket == IMS_NULL)
        {
            continue;
        }

        IMS_BOOL bMatched = IMS_FALSE;
        IPAddress objIP;
        IMS_UINT32 nPort = 0;

        pSocket->GetSockName(objIP, nPort);

        if (pIPQoS->objIpAddr.Equals(objIP))
        {
            bMatched = IMS_TRUE;

            if ((pIPQoS->nPort != 0) && (pIPQoS->nPort != static_cast<IMS_SINT32>(nPort)))
            {
                bMatched = IMS_FALSE;
            }
        }

        if (bMatched)
        {
            pSocket->SetOption(ISocket::OPT_IP_QOS, pIPQoS->nValue);
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::SetKeepAlivePolicy(IN CONST IPAddress& objSrcIP,
        IN IMS_UINT32 nSrcPort, IN CONST IPAddress& objDestIP, IN IMS_UINT32 nDestPort,
        IN IMS_SINT32 nPolicy /* = (-1) default */)
{
    SIPSocketAddress objSA;

    //---------------------------------------------------------------------------------------------

    objSA.SetType(SIPSocketAddress::SOCKET_TCP_CLIENT);
    objSA.SetPort(nDestPort);
    objSA.SetIPAddress(objDestIP);

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if ((pSocket != IMS_NULL) && pSocket->Equals(objSA))
        {
            IMS_BOOL bFound = IMS_TRUE;

            if (!objSrcIP.IsNoneAddress())
            {
                IPAddress objIP;
                IMS_UINT32 nPort = 0;

                pSocket->GetSockName(objIP, nPort);

                if (objSrcIP.Equals(objIP))
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
                SIPStreamSocket* pStreamSocket = DYNAMIC_CAST(SIPStreamSocket*, pSocket);

                if (pStreamSocket != IMS_NULL)
                {
                    pStreamSocket->SetKeepAlivePolicy(nPolicy);
                }
                break;
            }
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::SetLocalDnsQueryListener(
        IN ISipLocalDnsQueryListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piDnsQueryListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::DatagramSocket_DataReceived(IN SIPSocket* pSocket,
        IN CONST ByteArray& objBuffer, IN CONST IPAddress& objIPAddress, IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    if (objBuffer.GetLength() == 0)
        return;

    TransportBuffer* pBuffer = new TransportBuffer();

    if (pBuffer == IMS_NULL)
        return;

    pBuffer->objData = objBuffer;

    // Sets a packet source transport information
    pBuffer->objTA_FarEnd.SetProtocol(SIPTransportAddress::PROTOCOL_UDP);
    pBuffer->objTA_FarEnd.SetPort(nPort);
    pBuffer->objTA_FarEnd.SetIPAddress(objIPAddress);

    IPAddress objIPA_NearEnd;
    IMS_UINT32 nPort_NearEnd;

    // Sets a packet destination(my) transport information
    pSocket->GetSockName(objIPA_NearEnd, nPort_NearEnd);
    pBuffer->objTA_NearEnd.SetProtocol(SIPTransportAddress::PROTOCOL_UDP);
    pBuffer->objTA_NearEnd.SetPort(nPort_NearEnd);
    pBuffer->objTA_NearEnd.SetIPAddress(objIPA_NearEnd);

    // DEBUG ...
    pBuffer->DisplayMessage(GetSlotId());

    if (!objBuffers.Append(pBuffer))
    {
        delete pBuffer;
        return;
    }

    // Send event to process the raw SIP message
    PostMessage(AMSG_PROCESS_MESSAGE, 0, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::StreamSocket_ConnectionReceived(IN SIPSocket* pSocket)
{
    SIPSocket* pNewSocket = pSocket->Accept();

    //---------------------------------------------------------------------------------------------

    if (pNewSocket != IMS_NULL)
    {
        if (!AttachSocket(pNewSocket))
        {
            delete pNewSocket;
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::StreamSocket_DataReceived(
        IN SIPSocket* pSocket, IN_OUT ByteArray& objBuffer)
{
    //---------------------------------------------------------------------------------------------

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
        IPAddress objTmpIPA;
        IMS_UINT32 nTmpPort;
        IMS_SINT32 nTransportProtocol = SIPTransportAddress::PROTOCOL_TCP;
        SIPStreamSocket* pStreamSocket = DYNAMIC_CAST(SIPStreamSocket*, pSocket);

        if ((pStreamSocket != IMS_NULL) && pStreamSocket->IsSecureSocket())
        {
            nTransportProtocol = SIPTransportAddress::PROTOCOL_TLS;
        }

        pBuffer->objData = objBuffer;

        // Sets a packet source transport information
        pSocket->GetPeerName(objTmpIPA, nTmpPort);

        pBuffer->objTA_FarEnd.SetProtocol(nTransportProtocol);
        pBuffer->objTA_FarEnd.SetPort(nTmpPort);
        pBuffer->objTA_FarEnd.SetIPAddress(objTmpIPA);

        // Sets a packet destination (my) transport information
        pSocket->GetSockName(objTmpIPA, nTmpPort);

        pBuffer->objTA_NearEnd.SetProtocol(nTransportProtocol);
        pBuffer->objTA_NearEnd.SetPort(nTmpPort);
        pBuffer->objTA_NearEnd.SetIPAddress(objTmpIPA);

        // DEBUG ...
        pBuffer->DisplayMessage(GetSlotId());

        if (!objBuffers.Append(pBuffer))
        {
            delete pBuffer;
            return;
        }

        // Send event to process the message
        PostMessage(AMSG_PROCESS_MESSAGE, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::StreamSocket_KeepAliveExpired(IN SIPSocket* pSocket)
{
    //---------------------------------------------------------------------------------------------

    if (!IsSocketPresent(pSocket))
        return;

    if (pSocket->RemoveListener(IMS_NULL) == 0)
    {
        LookupSocket(*pSocket, IMS_TRUE);

        delete pSocket;
        pSocket = IMS_NULL;

        IMS_TRACE_D("TransportHelper :: Destroy (Keep-Alive Expired) - Sockets (%d)",
                objSockets.GetSize(), 0, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPTransportHelper::StreamSocket_PassiveClosed(IN SIPSocket* pSocket)
{
    //---------------------------------------------------------------------------------------------

    if (!IsSocketPresent(pSocket))
        return;

    if (IsClientInitiatedConnection(pSocket))
    {
        IMS_TRACE_D("SIPTransportHelper :: Managed client initiated connection is required...", 0,
                0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pTmpSocket = objSockets.GetAt(i);

        if (pSocket == pTmpSocket)
        {
            objSockets.RemoveAt(i);

            delete pSocket;
            pSocket = IMS_NULL;

            IMS_TRACE_D("TransportHelper :: Destroy (Passive Closed) - Sockets (%d)",
                    objSockets.GetSize(), 0, 0);
            return;
        }
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPTransportHelper::AttachSocket(IN SIPSocket* pSocket)
{
    //---------------------------------------------------------------------------------------------

    return objSockets.Append(pSocket);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPTransportHelper::IsSocketPresent(IN SIPSocket* pSocket) const
{
    //---------------------------------------------------------------------------------------------

    if (pSocket != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
        {
            SIPSocket* pTmpSocket = objSockets.GetAt(i);

            if (pSocket == pTmpSocket)
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
SIPSocket* SIPTransportHelper::LookupSocket(
        IN CONST SIPSocketAddress& objSA, IN IMS_BOOL bDetach /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if (pSocket->Equals(objSA))
        {
            if (bDetach == IMS_TRUE)
            {
                objSockets.RemoveAt(i);
            }

            return pSocket;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE
SIPSocket* SIPTransportHelper::LookupSocket(
        IN CONST SIPSocket& objSocket, IN IMS_BOOL bDetach /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if (pSocket->Equals(objSocket))
        {
            IMS_SINT32 nType = pSocket->GetType();

            if ((nType == SIPSocketAddress::SOCKET_TCP_CLIENT) ||
                    (nType == SIPSocketAddress::SOCKET_TCP_CLIENT_BY_PEER))
            {
                IPAddress objIP;
                IPAddress objOtherIP;
                IMS_UINT32 nPort = 0;
                IMS_UINT32 nOtherPort = 0;
                SIPSocket* pOtherSocket = const_cast<SIPSocket*>(&objSocket);

                pSocket->GetSockName(objIP, nPort);
                pOtherSocket->GetSockName(objOtherIP, nOtherPort);

                if (!objIP.Equals(objOtherIP) || (nPort != nOtherPort))
                {
                    continue;
                }

                // CASE :: system socket is already closed (no socket handle)
                // To avoid the crash, it just checks the address of socket object.
                if ((pSocket != pOtherSocket) && (nPort == 0) && (nOtherPort == 0))
                {
                    // It's different socket...
                    continue;
                }
            }

            if (bDetach == IMS_TRUE)
            {
                objSockets.RemoveAt(i);
            }

            return pSocket;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE
SIPSocket* SIPTransportHelper::LookupStreamSocket(IN CONST SIPSocketAddress& objSA)
{
    IPAddress objIP;
    IMS_UINT32 nPort = 0;
    SIPSocket* pStreamSocket = IMS_NULL;
    AString strNearEnd;

    //---------------------------------------------------------------------------------------------

    strNearEnd.Sprintf("Near End (%s, %d)", SipDebug::GetIp(objSA.GetIPAddress()), objSA.GetPort());

    // Find a socket using the source address & port again
    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if (pSocket->GetType() != objSA.GetType())
            continue;

        pSocket->GetSockName(objIP, nPort);

        IMS_TRACE_D("Lookup :: TCP client (%s, %d) at (%d)", SipDebug::GetIp(objIP), nPort, i);

        if (objIP.Equals(objSA.GetIPAddress()) &&
                (nPort == static_cast<IMS_UINT32>(objSA.GetPort())))
        {
            IMS_TRACE_D("Lookup :: TCP client (%s, %d), %s", SipDebug::GetIp(objIP), nPort,
                    strNearEnd.GetStr());
            return pSocket;
        }
    }

    if (pStreamSocket == IMS_NULL)
    {
        IMS_TRACE_D("Lookup :: TCP client (not found) (%s, %d), %s",
                SipDebug::GetIp(objSA.GetIPAddress()), objSA.GetPort(), strNearEnd.GetStr());
    }

    return pStreamSocket;
}

/*

Remarks

*/
PRIVATE
SIPSocket* SIPTransportHelper::LookupStreamSocket(
        IN CONST SIPSocketAddress& objSA, IN CONST SIPSocketAddress& objSA_FarEnd)
{
    IMSList<SIPSocket*> objCandidates;
    AString strFarEnd;

    //---------------------------------------------------------------------------------------------

    // LOG_EXCLUDING_SERVER_INFO
    strFarEnd.Sprintf("Far End (%s, %d)",
            SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                    ? "xxx"
                    : SipDebug::GetIp(objSA_FarEnd.GetIPAddress()),
            objSA_FarEnd.GetPort());

    // Find a socket using the destination address & port
    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        SIPSocket* pSocket = objSockets.GetAt(i);

        if (pSocket->Equals(objSA_FarEnd))
        {
            objCandidates.Prepend(pSocket);
        }
    }

    if (objCandidates.IsEmpty())
    {
        IMS_TRACE_D("Lookup :: TCP client does not exist; %s", strFarEnd.GetStr(), 0, 0);
        return IMS_NULL;
    }

    IPAddress objIP;
    IMS_UINT32 nPort = 0;
    SIPSocket* pStreamSocket = IMS_NULL;

    // Find a socket using the source address & port again
    for (IMS_UINT32 i = 0; i < objCandidates.GetSize(); ++i)
    {
        SIPSocket* pSocket = objCandidates.GetAt(i);

        pSocket->GetSockName(objIP, nPort);

        IMS_TRACE_D("Lookup :: TCP client (%s, %d) at (%d)", SipDebug::GetIp(objIP), nPort, i);

        if (objIP.Equals(objSA.GetIPAddress()) &&
                (nPort == static_cast<IMS_UINT32>(objSA.GetPort())))
        {
            IMS_TRACE_D("Lookup :: TCP client (%s, %d), %s", SipDebug::GetIp(objIP), nPort,
                    strFarEnd.GetStr());
            return pSocket;
        }

        // Find a socket using the source address (for TCP client socket w/ random port)
        if ((pStreamSocket == IMS_NULL) && (objSA.GetPort() <= 0))
        {
            if (objIP.Equals(objSA.GetIPAddress()))
            {
                pStreamSocket = pSocket;
                IMS_TRACE_D("Lookup :: TCP client (%s, %d) w/o port, %s", SipDebug::GetIp(objIP),
                        nPort, strFarEnd.GetStr());
            }
        }
    }

    if (pStreamSocket == IMS_NULL)
    {
        IMS_TRACE_D("Lookup :: TCP client (not found) (%s, %d), %s",
                SipDebug::GetIp(objSA.GetIPAddress()), objSA.GetPort(), strFarEnd.GetStr());
    }

    if (objCandidates.GetSize() > 1)
    {
        IMS_TRACE_D(
                "Lookup :: TCP client socket is ambiguous ... (%d)", objCandidates.GetSize(), 0, 0);
    }

    return pStreamSocket;
}

/*

Remarks

*/
PRIVATE
void SIPTransportHelper::TransportBuffer::DisplayMessage(IN IMS_SINT32 nSlotId)
{
    //---------------------------------------------------------------------------------------------

    SIPTransport::PrintMessage(nSlotId, IMS_FALSE, objTA_FarEnd,
            reinterpret_cast<const IMS_CHAR*>(objData.GetData()), objData.GetLength());
}
