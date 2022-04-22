/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/
#include "CarrierConfig.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceUtil.h"
#include "INetworkIpSec.h"
#include "SIPPrivate.h"
#include "SIPFeatures.h"
#include "SIPDebug.h"
#include "SIPRTConfigUtils.h"
#include "ISIPKeepAliveListener.h"
#include "ISIPSocketListener.h"
#include "SIPSocket.h"

__IMS_TRACE_TAG_SIP__;



PUBLIC
SIPSocket::SIPSocket(IN IMS_SINT32 nSlotId,
        IN IMS_SINT32 nType_ /* = SIPSocketAddress::SOCKET_UDP */)
    : IMSSlot(nSlotId)
    , piSocket(IMS_NULL)
    , piKeepAliveListener(IMS_NULL)
    , nState(STATE_CREATED)
    , bForcinglyClosed(IMS_FALSE)
{
    objSA.SetType(nType_);
}

PUBLIC VIRTUAL
SIPSocket::~SIPSocket()
{
    CloseSocket();
#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPSocket", 0, 0, 0);
#endif
}

/*

Remarks

*/
PUBLIC VIRTUAL
SIPSocket* SIPSocket::Accept()
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

PUBLIC VIRTUAL
void SIPSocket::ApplyIpSec(IN INetSocket* piAcceptedSocket /*= IMS_NULL*/)
{
    ApplyIpSecInternal(objSA.GetSocketAddress(), IMS_NULL, piAcceptedSocket);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPSocket::Connect()
{
    //---------------------------------------------------------------------------------------------

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPSocket::Create(IN CONST IPAddress & objIPA, IN IMS_UINT32 nPort /* = 0 */,
        IN IMS_BOOL bSecure /* = IMS_FALSE */)
{
    INetConnection *piNetConnection = NetworkService::GetNetworkService()->FindConnection(objIPA);

    //---------------------------------------------------------------------------------------------

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "No data connection (%s)", SIPDebug::GetIP(objIPA), 0, 0);
        return IMS_FALSE;
    }

    if (GetState() != STATE_CREATED)
    {
        return IMS_FALSE;
    }

    if (bSecure)
    {
        piSocket = NetworkService::GetNetworkService()->CreateSSLSocket(piNetConnection, IMS_NULL);
    }
    else
    {
        piSocket = NetworkService::GetNetworkService()->CreateSocket(piNetConnection);
    }

    if (piSocket == IMS_NULL)
    {
        return IMS_FALSE;
    }

    piSocket->SetListener(this);

    INetSocket::SOCKET_RESULT enResult;
    INetSocket::ADDRESS_FAMILY_ENTYPE enAddressFamily = INetSocket::ADDRESS_FAMILY_INET;

    if (!objIPA.IsIPv4Address())
    {
        enAddressFamily = INetSocket::ADDRESS_FAMILY_INET6;
    }

    if (objSA.GetType() == SIPSocketAddress::SOCKET_UDP)
    {
        enResult = piSocket->Open(INetSocket::TYPE_DGRAM, enAddressFamily);
    }
    else
    {
        enResult = piSocket->Open(INetSocket::TYPE_STREAM, enAddressFamily);
    }

    if (enResult != INetSocket::RESULT_SUCCESS)
    {
        NetworkService::GetNetworkService()->DestroySocket(piSocket);
        piSocket = IMS_NULL;
        return IMS_FALSE;
    }

    // Check the socket option and set it if it is present...
    SetSocketOptions(objIPA, nPort);

    if ((objSA.GetType() != SIPSocketAddress::SOCKET_UDP)
            && SIPFeatures::IsSocketOptionRequiredForTcpMaxSeg(GetSlotId()))
    {
        SetSocketOptionForTcpMaxSeg(piNetConnection, objIPA);
    }

    SetState(STATE_INITIALIZED);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPSocket::Equals(IN CONST SIPSocketAddress &objSA)
{
    //---------------------------------------------------------------------------------------------

    return this->objSA.Equals(objSA);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPSocket::Equals(IN CONST SIPSocket &objSocket)
{
    //---------------------------------------------------------------------------------------------

    return this->objSA.Equals(objSocket.objSA);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPSocket::GetSockName(OUT IPAddress &objIPA, OUT IMS_UINT32 &nPort)
{
    //---------------------------------------------------------------------------------------------

    nPort = objSA.GetPort();
    objIPA = objSA.GetIPAddress();
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SIPSocket::Send(IN CONST IMS_BYTE * /* pBuffer */, IN IMS_SINT32 /* nBuffLen */,
        IN IMS_UINT32 /* nPort = 0 */, IN CONST IPAddress & /* objIPA = IPAddress::NONE */)
{
    //---------------------------------------------------------------------------------------------

    return INetSocket::RESULT_ERROR;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void SIPSocket::NotifyForceClosed()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Socket(%p) is forcingly closed", this, 0, 0);

    SetForcinglyClosed(IMS_TRUE);
    Socket_Closed(piSocket);
}

/*

Remarks

*/
PUBLIC
void SIPSocket::GetPeerName(OUT IPAddress &objIPA, OUT IMS_UINT32 &nPort)
{
    //---------------------------------------------------------------------------------------------

    if (piSocket != IMS_NULL)
    {
        piSocket->GetPeerName(objIPA, nPort);
    }
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 SIPSocket::RemoveListener(IN ISIPSocketListener *piListener_)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISIPSocketListener *piListener = objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            if (piListener == piListener_)
            {
                objListeners.RemoveAt(i);
                break;
            }
        }
    }

    return objListeners.GetSize();
}

/*

Remarks

*/
PUBLIC
void SIPSocket::SetKeepAliveListener(IN ISIPKeepAliveListener *piKeepAliveListener)
{
    //---------------------------------------------------------------------------------------------

    this->piKeepAliveListener = piKeepAliveListener;
}

/*

Remarks

*/
PUBLIC
void SIPSocket::SetListener(IN ISIPSocketListener *piListener_)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISIPSocketListener *piListener = objListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            if (piListener == piListener_)
            {
                return;
            }
        }
    }

    objListeners.Append(piListener_);
}

/*

Remarks

*/
PUBLIC
void SIPSocket::SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue)
{
    //---------------------------------------------------------------------------------------------

    if (piSocket == IMS_NULL)
    {
        return;
    }

    if (GetState() == STATE_TERMINATED)
    {
        return;
    }

    if (!piSocket->SetOption(nOption, nOptionValue))
    {
        IMS_TRACE_E(0, "Setting socket option(%d=%d) failed", nOption, nOptionValue, 0);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPSocket::Socket_DataReceived(IN INetSocket * /* piSocket */)
{
    //---------------------------------------------------------------------------------------------

}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPSocket::Socket_SendEnabled(IN INetSocket * /* piSocket */)
{
    IMSList<ISIPSocketListener*> objTmpListeners = objListeners;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISIPSocketListener *piListener = objTmpListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Socket_SendEnabled(this);
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPSocket::Socket_ConnectionReceived(IN INetSocket * /* piSocket */)
{
    //---------------------------------------------------------------------------------------------

}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPSocket::Socket_Connected(IN INetSocket * /* piSocket */)
{
    IMSList<ISIPSocketListener*> objTmpListeners = objListeners;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISIPSocketListener *piListener = objTmpListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Socket_SendEnabled(this);
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPSocket::Socket_Closed(IN INetSocket * /* piSocket */,
        IN IMS_SINT32 nReason /* = INetSocket::CLOSE_REASON_UNKNOWN */)
{
    IMSList<ISIPSocketListener*> objTmpListeners = objListeners;
    IMS_SINT32 nErrorCode = ERROR_CLOSED;
    IMS_SINT32 nPrevState = GetState();

    //---------------------------------------------------------------------------------------------

    SetState(STATE_TERMINATED);

    CloseSocket();

    if (nReason == INetSocket::CLOSE_REASON_DATA_CONNECTION_LOST)
    {
        nErrorCode = ERROR_DATA_CONNECTION_LOST;
    }
    else
    {
        if (nPrevState == STATE_CONNECTING)
        {
            nErrorCode = ERROR_CONNECT_FAILED;
        }
    }

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISIPSocketListener *piListener = objTmpListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Socket_NotifyError(this, nErrorCode);
        }
    }
}

PROTECTED
void SIPSocket::ApplyIpSecInternal(IN const SocketAddress& objLocal,
        IN const SocketAddress* pRemote /*= IMS_NULL*/,
        IN INetSocket* piAcceptedSocket /*= IMS_NULL*/)
{
    INetIPSec* piIpSec = NetworkService::GetNetworkService()->GetIPSec();

    if (piIpSec != IMS_NULL)
    {
        if (piAcceptedSocket == IMS_NULL)
        {
            IMS_TRACE_D("ApplyIpSecInternal: socket=%d, local-addr=(%s), remote-addr=(%s)",
                    piSocket->GetSocketId(), objLocal.ToString().GetStr(),
                    (pRemote == IMS_NULL) ? "__NULL__" : pRemote->ToString().GetStr());

            piIpSec->ApplyIpSecTransform(piSocket, objLocal, pRemote);
        }
        else
        {
            IMS_TRACE_D("ApplyIpSecInternal: socket=%d, serverSocket=%d",
                    piAcceptedSocket->GetSocketId(), piSocket->GetSocketId(), 0);

            piIpSec->ApplyIpSecTransform(piAcceptedSocket, piSocket);
        }
    }
}

PROTECTED
void SIPSocket::CloseSocket()
{
    if (piSocket != IMS_NULL)
    {
        piSocket->SetListener(IMS_NULL);
        piSocket->Close();
        NetworkService::GetNetworkService()->DestroySocket(piSocket);
        piSocket = IMS_NULL;
    }
}

/*

Remarks

*/
PROTECTED
void SIPSocket::NotifyPongReceived()
{
    //---------------------------------------------------------------------------------------------

    if (piKeepAliveListener != IMS_NULL)
    {
        piKeepAliveListener->KeepAlive_PongReceived();
    }
}

/*

Remarks

*/
PROTECTED
void SIPSocket::SetForcinglyClosed(IN IMS_BOOL bClosed)
{
    //---------------------------------------------------------------------------------------------

    bForcinglyClosed = bClosed;
}

/*

Remarks

*/
PROTECTED
void SIPSocket::SetSocketOptionForTcpMaxSeg(
        IN INetConnection* piConnection, IN CONST IPAddress &objLocalIP)
{
    // MSS(Max Segment Size) for TCP
    IMS_SINT32 nMss = piConnection->GetMtu();
    IMS_BOOL bIPV6 = !objLocalIP.IsIPv4Address();
    IMS_BOOL bWfcSupported = CarrierConfig::IsWfcEnabled(GetSlotId());
    IMS_SINT32 nOverhead = SIP::PACKET_OVERHEAD_ESP + SIP::PACKET_OVERHEAD_TCP;

    //---------------------------------------------------------------------------------------------

    // Total overhead : esp + tcp + ip + edpg
    nOverhead += (bIPV6 ? SIP::PACKET_OVERHEAD_IPV6 : SIP::PACKET_OVERHEAD_IPV4);
    nOverhead += (bWfcSupported ? SIP::PACKET_OVERHEAD_EPDG : 0);

    if (bIPV6 && bWfcSupported && (nMss > SIP::MTU_IPV6))
    {
        IMS_TRACE_I("MTU (IPv6) :: %d >> %d", nMss, SIP::MTU_IPV6, 0);

        nMss = SIP::MTU_IPV6;
    }

    if (nMss <= 0)
    {
        nMss = bIPV6 ? SIP::MTU_IPV6 : SIP::MTU_IPV4;
    }

    // MSS will be set to (MTU - lower layer's overhead)
    nMss -= nOverhead;

    if (nMss > 0)
    {
        SetOption(INetSocket::OPT_TCP_MAXSEG, nMss);
    }
}

/*

Remarks

*/
PROTECTED
void SIPSocket::SetSocketOptions(IN CONST IPAddress &objLocalIP, IN IMS_UINT32 nLocalPort)
{
    IMS_SINT32 nSocketType = objSA.GetType();

    //---------------------------------------------------------------------------------------------

    // REUSEADDR / LINGER / SHUTDOWN option for StreamSocket
    if (nSocketType != SIPSocketAddress::SOCKET_UDP)
    {
        SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                SIPRTConfig::CONFIG_I_REUSEADDR, INetSocket::OPT_REUSEADDR,
                "OPT_REUSEADDR");
        SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                SIPRTConfig::CONFIG_I_LINGER, INetSocket::OPT_LINGER,
                "OPT_LINGER");
        SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                SIPRTConfig::CONFIG_I_SHUTDOWN, INetSocket::OPT_SHUTDOWN,
                "OPT_SHUTDOWN");

        // TCP_KEEPCNT / TCP_KEEPIDLE / TCP_KEEPINTVL option for StreamSocket (client only)
        if ((nSocketType == SIPSocketAddress::SOCKET_TCP_CLIENT)
                || (nSocketType == SIPSocketAddress::SOCKET_TCP_CLIENT_BY_PEER))
        {
            SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                    SIPRTConfig::CONFIG_I_KEEPALIVE, INetSocket::OPT_KEEPALIVE,
                    "OPT_KEEPALIVE");

            SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                    SIPRTConfig::CONFIG_I_TCP_KEEP_COUNT, INetSocket::OPT_TCP_KEEPCNT,
                    "OPT_TCP_KEEPCNT");
            SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                    SIPRTConfig::CONFIG_I_TCP_KEEP_IDLE, INetSocket::OPT_TCP_KEEPIDLE,
                    "OPT_TCP_KEEPIDLE");
            SetSocketOption(GetSlotId(), piSocket, objLocalIP, nLocalPort,
                    SIPRTConfig::CONFIG_I_TCP_KEEP_INTERVAL, INetSocket::OPT_TCP_KEEPINTVL,
                    "OPT_TCP_KEEPINTVL");
        }
    }

    SIPRTConfigHelper *pConfigHelper = SIPRTConfigUtils::GetConfigHelper(GetSlotId());

    // IP-level QoS option
    if (pConfigHelper->IsItemConfigured(SIPRTConfig::CONFIG_I_IP_QOS))
    {
        const SIPRTConfig::IPQoS *pIPQoS = pConfigHelper->GetIPQoS(objLocalIP, nLocalPort);

        if (pIPQoS != IMS_NULL)
        {
            if (!piSocket->SetOption(INetSocket::OPT_IP_QOS, pIPQoS->nValue))
            {
                IMS_TRACE_E(0, "Setting IP-level QoS failed", 0, 0, 0);
            }

            if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
            {
                IMS_TRACE_D("SIPSocket :: OPT_IP_QOS (option=0x%02X)", pIPQoS->nValue, 0, 0);
            }
        }
    }
    else if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_D("SIPSocket :: OPT_IP_QOS is not configured", 0, 0, 0);
    }
}

/*

Remarks

*/
PROTECTED
void SIPSocket::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("SIPSocket :: %s to %s", StateToString(this->nState), StateToString(nState), 0);

    this->nState = nState;
}

/*

Remarks

*/
PROTECTED GLOBAL
void SIPSocket::SetSocketOption(IN IMS_SINT32 nSlotId, IN INetSocket *piSocket,
        IN CONST IPAddress &objLocalIP, IN IMS_UINT32 nLocalPort,
        IN IMS_SINT32 nConfigItem, IN IMS_SINT32 nSocketOption,
        IN CONST IMS_CHAR *pszOptionName)
{
    SIPRTConfigHelper *pConfigHelper = SIPRTConfigUtils::GetConfigHelper(nSlotId);

    //---------------------------------------------------------------------------------------------

    if (pConfigHelper->IsItemConfigured(nConfigItem))
    {
        const SIPRTConfig::SocketOption *pSO
                = pConfigHelper->GetSocketOption(nConfigItem);
        const SIPRTConfig::SocketOption *pDedicatedSO
                = pConfigHelper->GetSocketOption(nConfigItem, objLocalIP, nLocalPort);

        if (pDedicatedSO != IMS_NULL)
        {
            pSO = pDedicatedSO;
        }

        if (pSO != IMS_NULL)
        {
            if (!piSocket->SetOption(nSocketOption, pSO->nValue))
            {
                IMS_TRACE_E(0, "Setting %s failed", pszOptionName, 0, 0);
            }

            if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
            {
                IMS_TRACE_D("SIPSocket :: %s (option=%d)", pszOptionName, pSO->nValue, 0);
            }
        }
    }
    else if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_D("SIPSocket :: %s is not configured", pszOptionName, 0, 0);
    }
}

/*

Remarks

*/
PROTECTED GLOBAL
const IMS_CHAR* SIPSocket::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
    case STATE_CREATED:
        return "STATE_CREATED";
    case STATE_INITIALIZED:
        return "STATE_INITIALIZED";
    case STATE_CONNECTING:
        return "STATE_CONNECTING";
    case STATE_CONNECTED:
        return "STATE_CONNECTED";
    case STATE_CLOSING:
        return "STATE_CLOSING";
    case STATE_TERMINATED:
        return "STATE_TERMINATED";
    default:
        return "__INVALID__";
    }
}
