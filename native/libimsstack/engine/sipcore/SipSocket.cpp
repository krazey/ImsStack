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
#include "CarrierConfig.h"
#include "INetworkConnection.h"
#include "INetworkIpSec.h"
#include "ISystemProperty.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceTrace.h"

#include "ISipKeepAliveListener.h"
#include "ISipSocketListener.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipFeatures.h"
#include "SipPrivate.h"
#include "SipRtConfigHelper.h"
#include "SipRtConfigUtils.h"
#include "SipSocket.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipSocket::SipSocket(
        IN IMS_SINT32 nSlotId, IN IMS_SINT32 nType /*= SipSocketAddress::SOCKET_UDP*/) :
        ImsSlot(nSlotId),
        m_piSocket(IMS_NULL),
        m_piKeepAliveListener(IMS_NULL),
        m_nState(STATE_CREATED),
        m_bForcinglyClosed(IMS_FALSE)
{
    m_objSockAddr.SetType(nType);
}

PUBLIC VIRTUAL SipSocket::~SipSocket()
{
    CloseSocket();
#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipSocket", 0, 0, 0);
#endif
}

PUBLIC VIRTUAL IMS_BOOL SipSocket::Create(
        IN const IpAddress& objIp, IN IMS_UINT32 nPort /*= 0*/, IN IMS_BOOL bSecure /*= IMS_FALSE*/)
{
    INetworkConnection* piNetConnection =
            NetworkService::GetNetworkService()->FindConnection(objIp);

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "No data connection (%s)", SipDebug::GetIp(objIp), 0, 0);
        return IMS_FALSE;
    }

    if (GetState() != STATE_CREATED)
    {
        return IMS_FALSE;
    }

    if (bSecure)
    {
        m_piSocket =
                NetworkService::GetNetworkService()->CreateSslSocket(piNetConnection, IMS_NULL);
    }
    else
    {
        m_piSocket = NetworkService::GetNetworkService()->CreateSocket(piNetConnection);
    }

    if (m_piSocket == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_piSocket->SetListener(this);

    ISocket::SOCKET_RESULT eResult;
    ISocket::ADDRESS_FAMILY_ENTYPE eAddressFamily = ISocket::ADDRESS_FAMILY_INET;

    if (!objIp.IsIPv4Address())
    {
        eAddressFamily = ISocket::ADDRESS_FAMILY_INET6;
    }

    if (m_objSockAddr.GetType() == SipSocketAddress::SOCKET_UDP)
    {
        eResult = m_piSocket->Open(ISocket::TYPE_DGRAM, eAddressFamily);
    }
    else
    {
        eResult = m_piSocket->Open(ISocket::TYPE_STREAM, eAddressFamily);
    }

    if (eResult != ISocket::RESULT_SUCCESS)
    {
        NetworkService::GetNetworkService()->DestroySocket(m_piSocket);
        m_piSocket = IMS_NULL;
        return IMS_FALSE;
    }

    // Check the socket option and set it if it is present.
    SetSocketOptions(objIp, nPort);

    if ((m_objSockAddr.GetType() != SipSocketAddress::SOCKET_UDP) &&
            SipFeatures::IsSocketOptionRequiredForTcpMaxSeg(GetSlotId()))
    {
        SetSocketOptionForTcpMaxSeg(piNetConnection, objIp);
    }

    SetState(STATE_INITIALIZED);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void SipSocket::GetSockName(OUT IpAddress& objIp, OUT IMS_UINT32& nPort)
{
    nPort = m_objSockAddr.GetPort();
    objIp = m_objSockAddr.GetIpAddress();
}

PUBLIC VIRTUAL void SipSocket::NotifyForceClosed()
{
    IMS_TRACE_D("Socket(%p) is forcingly closed", this, 0, 0);

    SetForcinglyClosed(IMS_TRUE);
    Socket_OnClosed(m_piSocket);
}

PUBLIC
void SipSocket::GetPeerName(OUT IpAddress& objIp, OUT IMS_UINT32& nPort)
{
    if (m_piSocket != IMS_NULL)
    {
        m_piSocket->GetPeerName(objIp, nPort);
    }
}

PUBLIC
IMS_SINT32 SipSocket::RemoveListener(IN const ISipSocketListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const ISipSocketListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener != IMS_NULL)
        {
            if (piTmpListener == piListener)
            {
                m_objListeners.RemoveAt(i);
                break;
            }
        }
    }

    return m_objListeners.GetSize();
}

PUBLIC
void SipSocket::SetListener(IN ISipSocketListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const ISipSocketListener* piTmpListener = m_objListeners.GetAt(i);

        if (piTmpListener != IMS_NULL)
        {
            if (piTmpListener == piListener)
            {
                return;
            }
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void SipSocket::SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue)
{
    if (m_piSocket == IMS_NULL)
    {
        return;
    }

    if (GetState() == STATE_TERMINATED)
    {
        return;
    }

    if (!m_piSocket->SetOption(nOption, nOptionValue))
    {
        IMS_TRACE_E(0, "Setting socket option(%d=%d) failed", nOption, nOptionValue, 0);
    }
}

PROTECTED VIRTUAL void SipSocket::Socket_OnSendEnabled(IN ISocket* /*piSocket*/)
{
    ImsList<ISipSocketListener*> objTmpListeners = m_objListeners;

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISipSocketListener* piListener = objTmpListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Socket_SendEnabled(this);
        }
    }
}

PROTECTED VIRTUAL void SipSocket::Socket_OnConnected(IN ISocket* /*piSocket*/)
{
    ImsList<ISipSocketListener*> objTmpListeners = m_objListeners;

    for (IMS_UINT32 i = 0; i < objTmpListeners.GetSize(); ++i)
    {
        ISipSocketListener* piListener = objTmpListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Socket_SendEnabled(this);
        }
    }
}

PROTECTED VIRTUAL void SipSocket::Socket_OnClosed(
        IN ISocket* /*piSocket*/, IN IMS_SINT32 nReason /*= ISocket::CLOSE_REASON_UNKNOWN*/)
{
    ImsList<ISipSocketListener*> objTmpListeners = m_objListeners;
    IMS_SINT32 nErrorCode = ERROR_CLOSED;
    IMS_SINT32 nPrevState = GetState();

    SetState(STATE_TERMINATED);

    CloseSocket();

    if (nReason == ISocket::CLOSE_REASON_DATA_CONNECTION_LOST)
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
        ISipSocketListener* piListener = objTmpListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Socket_NotifyError(this, nErrorCode);
        }
    }
}

PROTECTED
void SipSocket::ApplyIpSecInternal(IN const SocketAddress& objLocal,
        IN const SocketAddress* pRemote /*= IMS_NULL*/,
        IN SipSocket* pAcceptedSocket /*= IMS_NULL*/)
{
    INetworkIpSec* piIpSec = NetworkService::GetNetworkService()->GetIpSec(GetSlotId());

    if (piIpSec != IMS_NULL)
    {
        if (pAcceptedSocket == IMS_NULL)
        {
            IMS_TRACE_D("ApplyIpSecInternal: socket=%d, local-addr=(%s), remote-addr=(%s)",
                    m_piSocket->GetSocketId(), objLocal.ToString().GetStr(),
                    (pRemote == IMS_NULL) ? "__NULL__" : pRemote->ToString().GetStr());

            piIpSec->ApplyIpSecTransform(m_piSocket, objLocal, pRemote);
        }
        else
        {
            IMS_TRACE_D("ApplyIpSecInternal: socket=%d, serverSocket=%d",
                    pAcceptedSocket->m_piSocket->GetSocketId(), m_piSocket->GetSocketId(), 0);

            piIpSec->ApplyIpSecTransform(pAcceptedSocket->m_piSocket, m_piSocket,
                    pAcceptedSocket->m_objSockAddr.GetSocketAddress());
        }
    }
}

PROTECTED
void SipSocket::CloseSocket()
{
    if (m_piSocket != IMS_NULL)
    {
        m_piSocket->SetListener(IMS_NULL);
        m_piSocket->Close();
        NetworkService::GetNetworkService()->DestroySocket(m_piSocket);
        m_piSocket = IMS_NULL;
    }
}

PROTECTED
void SipSocket::NotifyPongReceived()
{
    if (m_piKeepAliveListener != IMS_NULL)
    {
        m_piKeepAliveListener->KeepAlive_PongReceived();
    }
}

PROTECTED
void SipSocket::SetSocketOptionForTcpMaxSeg(
        IN const INetworkConnection* piConnection, IN const IpAddress& objLocalIp)
{
    // MSS(Max Segment Size) for TCP
    IMS_SINT32 nMss = piConnection->GetMtu();
    IMS_BOOL bIpV6 = !objLocalIp.IsIPv4Address();
    IMS_BOOL bWfcSupported = CarrierConfig::IsWfcEnabled(GetSlotId());
    IMS_SINT32 nOverhead = Sip::PACKET_OVERHEAD_ESP + Sip::PACKET_OVERHEAD_TCP;

    // Total overhead : esp + tcp + ip + edpg
    nOverhead += (bIpV6 ? Sip::PACKET_OVERHEAD_IPV6 : Sip::PACKET_OVERHEAD_IPV4);
    nOverhead += (bWfcSupported ? Sip::PACKET_OVERHEAD_EPDG : 0);

    // The MTU size will be adjusted to the MTU size of IPv6 regardless of the PDN's IP version
    // because ePDG can use the IPv6 for tunneling.
    if (bWfcSupported && (nMss > Sip::MTU_IPV6))
    {
        IMS_TRACE_I("MTU(%s): %d >> %d", (bIpV6 ? "IPv6" : "IPv4"), nMss, Sip::MTU_IPV6);
        nMss = Sip::MTU_IPV6;
    }

    if (nMss <= 0)
    {
        nMss = bIpV6 ? Sip::MTU_IPV6 : (bWfcSupported ? Sip::MTU_IPV6 : Sip::MTU_IPV4);
    }

    // MSS will be set to (MTU - lower layer's overhead)
    nMss -= nOverhead;

    if (nMss > 0)
    {
        SetOption(ISocket::OPT_TCP_MAXSEG, nMss);
    }
}

PROTECTED
void SipSocket::SetSocketOptions(IN const IpAddress& objLocalIp, IN IMS_UINT32 nLocalPort)
{
    IMS_SINT32 nSocketType = m_objSockAddr.GetType();

    // REUSEADDR / LINGER / SHUTDOWN option for StreamSocket
    if (nSocketType != SipSocketAddress::SOCKET_UDP)
    {
        SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                SipRtConfig::CONFIG_I_REUSEADDR, ISocket::OPT_REUSEADDR, "OPT_REUSEADDR");
        SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                SipRtConfig::CONFIG_I_LINGER, ISocket::OPT_LINGER, "OPT_LINGER");
        SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                SipRtConfig::CONFIG_I_SHUTDOWN, ISocket::OPT_SHUTDOWN, "OPT_SHUTDOWN");

        // TCP_KEEPCNT / TCP_KEEPIDLE / TCP_KEEPINTVL option for StreamSocket (client only)
        if ((nSocketType == SipSocketAddress::SOCKET_TCP_CLIENT) ||
                (nSocketType == SipSocketAddress::SOCKET_TCP_CLIENT_BY_PEER))
        {
            SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                    SipRtConfig::CONFIG_I_KEEPALIVE, ISocket::OPT_KEEPALIVE, "OPT_KEEPALIVE");

            SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                    SipRtConfig::CONFIG_I_TCP_KEEP_COUNT, ISocket::OPT_TCP_KEEPCNT,
                    "OPT_TCP_KEEPCNT");
            SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                    SipRtConfig::CONFIG_I_TCP_KEEP_IDLE, ISocket::OPT_TCP_KEEPIDLE,
                    "OPT_TCP_KEEPIDLE");
            SetSocketOption(GetSlotId(), m_piSocket, objLocalIp, nLocalPort,
                    SipRtConfig::CONFIG_I_TCP_KEEP_INTERVAL, ISocket::OPT_TCP_KEEPINTVL,
                    "OPT_TCP_KEEPINTVL");
        }
    }

    const SipRtConfigHelper* pConfigHelper = SipRtConfigUtils::GetConfigHelper(GetSlotId());

    // IP-level QoS option
    if (pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_IP_QOS))
    {
        const SipRtConfig::IpQos* pIpQos = pConfigHelper->GetIpQos(objLocalIp, nLocalPort);

        if (pIpQos != IMS_NULL)
        {
            if (!m_piSocket->SetOption(ISocket::OPT_IP_QOS, pIpQos->nValue))
            {
                IMS_TRACE_E(0, "Setting IP-level QoS failed", 0, 0, 0);
            }

#ifdef __IMS_SIP_DEBUG__
            IMS_TRACE_D("SipSocket: OPT_IP_QOS (option=0x%02X)", pIpQos->nValue, 0, 0);
#endif
        }
    }
}

PROTECTED
void SipSocket::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("SipSocket: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PROTECTED GLOBAL void SipSocket::SetSocketOption(IN IMS_SINT32 nSlotId, IN ISocket* piSocket,
        IN const IpAddress& objLocalIp, IN IMS_UINT32 nLocalPort, IN IMS_SINT32 nConfigItem,
        IN IMS_SINT32 nSocketOption, IN const IMS_CHAR* pszOptionName)
{
    const SipRtConfigHelper* pConfigHelper = SipRtConfigUtils::GetConfigHelper(nSlotId);

    if (pConfigHelper->IsItemConfigured(nConfigItem))
    {
        const SipRtConfig::SocketOption* pSockOpt = pConfigHelper->GetSocketOption(nConfigItem);
        const SipRtConfig::SocketOption* pDedicatedSockOpt =
                pConfigHelper->GetSocketOption(nConfigItem, objLocalIp, nLocalPort);

        if (pDedicatedSockOpt != IMS_NULL)
        {
            pSockOpt = pDedicatedSockOpt;
        }

        if (pSockOpt != IMS_NULL)
        {
            if (!piSocket->SetOption(nSocketOption, pSockOpt->nValue))
            {
                IMS_TRACE_E(0, "Setting %s failed", pszOptionName, 0, 0);
            }

#ifdef __IMS_SIP_DEBUG__
            IMS_TRACE_D("SipSocket: %s (option=%d)", pszOptionName, pSockOpt->nValue, 0);
#endif
        }
    }
}

PROTECTED GLOBAL const IMS_CHAR* SipSocket::StateToString(IN IMS_SINT32 nState)
{
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
