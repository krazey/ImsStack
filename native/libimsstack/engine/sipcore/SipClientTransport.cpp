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
#include "NatHelper.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"

#include "private/SipConfig.h"

#include "ISipHeader.h"
#include "SipAddress.h"
#include "SipClientTransport.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipFeatures.h"
#include "SipMessageBuffer.h"
#include "SipPrivate.h"
#include "SipRtConfigUtils.h"
#include "SipStack.h"
#include "SipTransportHelper.h"
#include "SipUtils.h"

#define __IMS_DNS_QUERY_SIP__

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipClientTransport::SipClientTransport(IN IMS_SINT32 nSlotId) :
        SipTransport(nSlotId, TYPE_CLIENT),
        m_pServerSocket(IMS_NULL),
        m_strExtensionTokenForViaBranch(AString::ConstNull())
{
}

PUBLIC
SipClientTransport::~SipClientTransport()
{
    if (m_pServerSocket != IMS_NULL)
    {
        GetTransportHelper()->Destroy(m_pServerSocket, this);
        m_pServerSocket = IMS_NULL;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SipClientTransport", 0, 0, 0);
#endif
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransport::FormViaHeader(
        IN_OUT ::SipMessage*& pSipMsg, IN const SipProfile* pProfile /*= IMS_NULL*/)
{
    // Update transport information with a default values if not provisioned
    CorrectNearEndAddress();

    // Form Via header
    // If topmost Via header already exists, do not insert a Via header.
    if (SipStack::GetHeaderCount(pSipMsg, ISipHeader::VIA) > 0)
    {
        IMS_TRACE_D("The Via header already exist", 0, 0, 0);
        return IMS_TRUE;
    }

    AString strSentBy;
    AString strPort;
    AString strSentProtocol(Sip::STR_SIP_VERSION);
    const SipTransportAddress& objNearEnd = GetAddress(TA_NEAR);
    const SipTransportAddress& objFarEnd = GetAddress(TA_FAR);

    strSentProtocol += TextParser::CHAR_SLASH;

    if (objFarEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TCP)
    {
        strSentProtocol += Sip::STR_TCP_CAPS;
    }
    else if (objFarEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TLS)
    {
        strSentProtocol += Sip::STR_TLS_CAPS;
    }
    else
    {
        // Overwrites the transport protocol by the local configuration...
        IMS_SINT32 nProtocol = Sip::TRANSPORT_EXT_ANY;

        if (!IsExplicitTargetProtocolSelected())
        {
            if (IsTargetProtocolConfigured())
            {
                IMS_SINT32 nTransportExt = GetTransportExt();

                if ((nTransportExt & Sip::TRANSPORT_EXT_TCP) != 0)
                {
                    nProtocol = Sip::TRANSPORT_EXT_TCP;
                }
                else if ((nTransportExt & Sip::TRANSPORT_EXT_TLS) != 0)
                {
                    nProtocol = Sip::TRANSPORT_EXT_TLS;
                }
            }
            else
            {
                IMS_SINT32 nTransportType = SipConfigProxy::GetTransportType(GetSlotId(), pProfile);

                if (nTransportType == SipConfig::TRANSPORT_TYPE_TCP)
                {
                    nProtocol = Sip::TRANSPORT_EXT_TCP;
                }
                else if (nTransportType == SipConfig::TRANSPORT_TYPE_TLS)
                {
                    nProtocol = Sip::TRANSPORT_EXT_TLS;
                }
            }
        }

        if (nProtocol == Sip::TRANSPORT_EXT_TCP)
        {
            strSentProtocol += Sip::STR_TCP_CAPS;
            SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_NEAR);
            SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_FAR);
        }
        else if (nProtocol == Sip::TRANSPORT_EXT_TLS)
        {
            strSentProtocol += Sip::STR_TLS_CAPS;
            SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_NEAR);
            SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_FAR);
        }
        else
        {
            strSentProtocol += Sip::STR_UDP_CAPS;
        }
    }

    strPort.SetNumber(objNearEnd.GetPort());

    IpAddress objLocalAddress(IpAddress::NONE);

    // IMS_IPSEC_UDP_ENC
    if (IsIpSecUdpEncRequired())
    {
        if (NatHelper::IsNatResolverRequired())
        {
            objLocalAddress = NatHelper::GetInstance()->GetPublicAddress(
                    GetSlotId(), objNearEnd.GetIpAddress());
        }

        if (objLocalAddress.Equals(IpAddress::NONE))
        {
            objLocalAddress = objNearEnd.GetIpAddress();
        }
    }
    else
    {
        objLocalAddress = objNearEnd.GetIpAddress();
    }

    // IPv4
    if (objLocalAddress.IsIPv4Address())
    {
        strSentBy = objLocalAddress.ToString();
    }
    // IPv6
    else if (objLocalAddress.IsIPv6Address())
    {
        strSentBy = TextParser::CHAR_LSBRACKET;
        strSentBy += objLocalAddress.ToString();
        strSentBy += TextParser::CHAR_RSBRACKET;
    }
    else
    {
        IMS_TRACE_E(0, "Unsupported IP address (%s)", SipDebug::GetIp(objLocalAddress), 0, 0);
        return IMS_FALSE;
    }

    // Append the port number
    strSentBy += TextParser::CHAR_COLON + strPort;

    // Create a Via branch
    AString strBranch = SipUtils::GenerateViaBranch(m_strExtensionTokenForViaBranch);

    SipHeaderBase* pSipHdr = SipStack::CreateViaHeader(strSentProtocol, strSentBy, strBranch);

    if (pSipHdr == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // "keep" parameter
    if (SipConfigProxy::IsKeepAliveConfigured(GetSlotId(), pProfile))
    {
        if (!SipStack::SetParameter(pSipHdr, Sip::STR_KEEP, AString::ConstNull()))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }
    }

    if (!SipStack::PrependHeader(pSipHdr, pSipMsg))
    {
        SipStack::FreeHeaderEx(pSipHdr);
        return IMS_FALSE;
    }

    // Check a raw SIP message buffer
    if (!IsExplicitTargetProtocolSelected() &&
            (objFarEnd.GetProtocol() == SipTransportAddress::PROTOCOL_UDP))
    {
        RcPtr<SipMessageBuffer> pMessageBuffer = SipMessageBuffer::GetInstance();
        IMS_SINT32 nBuffLen = pMessageBuffer->GetLength();
        IMS_BYTE* pBuffer = pMessageBuffer->GetBuffer(GetSlotId());

        IMS_MEM_Memset(pBuffer, 0x00, nBuffLen);

        // Apply the criteria of SIP message size
        if (!SipStack::EncodeMessage(pSipMsg, SipPrivate::GetEncodingOptions(), pBuffer, nBuffLen))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        IMS_TRACE_D(
                "SipClientTransport :: transport protocol selection - length(%d)", nBuffLen, 0, 0);

        if (nBuffLen > SipConfigProxy::GetTcpCriterionLength(GetSlotId(), pProfile))
        {
            // The sent-protocol in Via header needs to modify with TCP.
            strSentProtocol = Sip::STR_SIP_VERSION;
            strSentProtocol += TextParser::CHAR_SLASH;
            strSentProtocol += Sip::STR_TCP_CAPS;

            if (!SipStack::UpdateSentProtocol(pSipMsg, strSentProtocol))
            {
                SipStack::FreeHeaderEx(pSipHdr);
                return IMS_FALSE;
            }

            // Update the transport protocol of a far end point
            SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_FAR);

            IMS_TRACE_D("SipClientTransport :: sent-protocol is changed to TCP", 0, 0, 0);
        }
    }

    // "rport" parameter : UDP & unprotected port only
    IMS_SINT32 nPortC = GetPortC();

    if (SipConfigProxy::IsRportConfigured(GetSlotId(), pProfile) &&
            strSentProtocol.Contains(Sip::STR_UDP_CAPS) &&
            ((nPortC == 0) || (nPortC == Sip::PORT_UNSPECIFIED) ||
                    (objNearEnd.GetPort() == nPortC)))
    {
        if (!SipStack::SetParameter(pSipHdr, Sip::STR_RPORT, AString::ConstNull()))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }
    }

    SipStack::FreeHeaderEx(pSipHdr);

    // LOG_EXCLUDING_SERVER_INFO
    if (SipRtConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        IMS_TRACE_D("SipClientTransport :: via (%s, %s, %s)", strSentProtocol.GetStr(),
                SipDebug::GetIp(strSentBy), strBranch.GetStr());
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransport::ReserveResource(
        IN const SipProfile* pProfile /*= IMS_NULL*/)
{
    if (!SipTransport::ReserveResource(pProfile))
    {
        return IMS_FALSE;
    }

    // If the keep-alive mechanism is applied and the configured transport protocol
    // is TCP/TLS, all the services will be provided by the TCP client socket
    // to support the keep-alive mechanism.
    IMS_SINT32 nTransportType = SipConfigProxy::GetTransportType(GetSlotId(), pProfile);

    if (SipConfigProxy::IsKeepAliveConfigured(GetSlotId(), pProfile) &&
            (nTransportType == SipConfig::TRANSPORT_TYPE_TCP ||
                    nTransportType == SipConfig::TRANSPORT_TYPE_TLS))
    {
        IMS_TRACE_D("ClientTransport :: Configured transport protocol (%d) is not required "
                    "a server socket",
                nTransportType, 0, 0);
        return IMS_TRUE;
    }

    const SipTransportAddress& objNearEnd = GetAddress(TA_NEAR);
    const SipTransportAddress& objFarEnd = GetAddress(TA_FAR);

    if ((objFarEnd.GetProtocol() != SipTransportAddress::PROTOCOL_TCP) &&
            (objFarEnd.GetProtocol() != SipTransportAddress::PROTOCOL_TLS))
    {
        if (objNearEnd.GetPort() == GetPortC())
        {
            // If the server & client port is same,
            // no need to create socket in case of UDP protocol.
            IMS_TRACE_D("ClientTransport :: UDP server socket (%d) will not be created", GetPortC(),
                    0, 0);
            return IMS_TRUE;
        }
    }

    if (IsTcpConnectionOnlyRequired())
    {
        // Do not create a server socket for the SIP client connection...
        IMS_TRACE_D("ClientTransport :: TCP client connection is only required...", 0, 0, 0);
        return IMS_TRUE;
    }

    // Reserves the listening sockets for incoming SIP message...
    if (m_pServerSocket != IMS_NULL)
    {
        // If the near port number is changed, then destroy & create a TCP server socket.
        IpAddress objIpAddr;
        IMS_UINT32 nPort = 0;

        m_pServerSocket->GetSockName(objIpAddr, nPort);

        if (objNearEnd.GetPort() == static_cast<IMS_SINT32>(nPort))
        {
            return IMS_TRUE;
        }

        GetTransportHelper()->Destroy(m_pServerSocket, this);
        m_pServerSocket = IMS_NULL;
    }

    // RFC5626_FLOW_CONTROL
    if (!IsFlowControlPortConfigured())
    {
        SipSocketAddress objSsa;

        objSsa.SetIpAddress(objNearEnd.GetIpAddress());
        objSsa.SetPort(objNearEnd.GetPort());

        if ((objFarEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TCP) ||
                (objFarEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TLS))
        {
            objSsa.SetType(SipSocketAddress::SOCKET_TCP);

            // If the transport protocol is TLS, SSL socket will be created...
            if (objFarEnd.GetProtocol() == SipTransportAddress::PROTOCOL_TLS)
            {
                objSsa.SetSecure(IMS_TRUE);
            }
        }
        else
        {
            objSsa.SetType(SipSocketAddress::SOCKET_UDP);
        }

        m_pServerSocket = GetTransportHelper()->Create(objSsa);

        if (m_pServerSocket == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating server socket (%s, %d, %d) failed",
                    SipDebug::GetIp(objSsa.GetIpAddress()), objSsa.GetPort(), objSsa.GetType());
            return IMS_FALSE;
        }

        IMS_TRACE_D("ClientTransport :: server (%s, %d, %d) is created",
                SipDebug::GetIp(objSsa.GetIpAddress()), objSsa.GetPort(), objSsa.GetType());

        m_pServerSocket->SetListener(this);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipClientTransport::UpdateDestinationInfo(IN ::SipMessage* pSipMsg,
        IN IMS_BOOL bRoutingLr /*= IMS_TRUE*/, IN SipAddrSpec* pImplicitRoute /*= IMS_NULL*/)
{
    const AString PARAM_TRANSPORT(SipAddress::PARAM_TRANSPORT);

    SipAddrSpec* pAddrSpec;
    IMS_BOOL bImplicitRoutingTransportRequired = IMS_FALSE;
    AString strImplicitRoutingTransport(AString::ConstNull());

    // When determining the destination (next hop), it has been set.
    if (bRoutingLr)
    {
        pAddrSpec = SipStack::GetAddrSpec(pSipMsg, ISipHeader::ROUTE);
    }
    else
    {
        // In case of REGISTER request, it does not require the Route header.
        if (pImplicitRoute != IMS_NULL)
        {
            pAddrSpec = pImplicitRoute;
            SipStack::AddReference(pAddrSpec);

            // Store the transport parameter if present
            SipAddrSpec* pReqUri = SipStack::GetRequestUri(pSipMsg);

            if (pReqUri != IMS_NULL)
            {
                // FIXME: consider "sips" URI scheme
                if (IsSameHostAndPort(pAddrSpec, pReqUri))
                {
                    bImplicitRoutingTransportRequired = IMS_TRUE;
                    strImplicitRoutingTransport = SipStack::GetParameter(pReqUri, PARAM_TRANSPORT);
                }

                SipStack::FreeAddrSpec(pReqUri);
            }

            IMS_TRACE_I("IMPLICIT_ROUTE (%s,%s) will be applied...",
                    strImplicitRoutingTransport.GetStr(),
                    bImplicitRoutingTransportRequired ? "true" : "false", 0);
        }
        else
        {
            // In case of strict routing or no route set.
            pAddrSpec = SipStack::GetRequestUri(pSipMsg);
        }
    }

    if (pAddrSpec == IMS_NULL)
    {
        IMS_TRACE_E(0, "The target URI does not exist...", 0, 0, 0);
        return IMS_FALSE;
    }

    if (SipStack::IsUriSchemeSip(pAddrSpec) || SipStack::IsUriSchemeSips(pAddrSpec))
    {
        AString strTemp;

        if (SipStack::IsUriSchemeSips(pAddrSpec))
        {
            SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_NEAR);
        }

        // If a transport parameter exists, check what the transport type is and update
        // both the destination info. as well as the topmost Via of the request being sent out.
        // IMPLICIT_ROUTING
        if (!bRoutingLr && (pImplicitRoute != IMS_NULL) && bImplicitRoutingTransportRequired)
        {
            // Sets the transport protocol from the original routing information
            strTemp = strImplicitRoutingTransport;
        }
        else
        {
            strTemp = SipStack::GetParameter(pAddrSpec, PARAM_TRANSPORT);
        }

        if (!strTemp.IsNULL() && !strTemp.IsEmpty())
        {
            // Update the sent-protocol field in the topmost Via header
            // to reflect the transport type set.

            if (strTemp.EqualsIgnoreCase(Sip::STR_TCP))
            {
                // SIP/2.0/TCP
                SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_FAR);
            }
            else if (strTemp.EqualsIgnoreCase(Sip::STR_TLS))
            {
                // SIP/2.0/TLS
                SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_FAR);
            }
            else if (strTemp.EqualsIgnoreCase(Sip::STR_UDP))
            {
                if (SipFeatures::IsTransportParameterUdpIgnoredForOutgoingRequest(GetSlotId()))
                {
                    IMS_TRACE_I("SIP transport parameter(udp) is ignored", 0, 0, 0);
                }
                else
                {
                    SetExplicitTargetProtocol(IMS_TRUE);
                }

                // SIP/2.0/UDP
                SetProtocol(SipTransportAddress::PROTOCOL_UDP, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SipTransportAddress::PROTOCOL_UDP, TA_FAR);
            }
            else
            {
                SetTransportProtocolByConfig();
            }
        }
        else
        {
            SetTransportProtocolByConfig();
        }

        // Set the destination address & port number
        AString strFarAddress;
        IMS_UINT32 nFarPort;

        (void)SipStack::GetHostAndPort(pAddrSpec, strFarAddress, nFarPort);

        // If the port number does not exist,
        // set a default port number according to the URI format.
        if (SipStack::IsLastErrorNoExist())
        {
            if (SipStack::IsUriSchemeSips(pAddrSpec))
            {
                nFarPort = Sip::PORT_5061;
            }
            else
            {
                nFarPort = Sip::PORT_5060;
            }
        }
        else
        {
            if (nFarPort == Sip::PORT_UNSPECIFIED)
            {
                nFarPort = Sip::PORT_5060;
            }
        }

        SetPort(nFarPort, TA_FAR);

        IpAddress objFarAddress(strFarAddress);

#if defined(__IMS_DNS_QUERY_SIP__)
        // LOCAL_DNS_QUERY
        const IpAddress& objIpAddr = GetIpAddress();
        IMS_BOOL bDnsQueryRequired = IMS_FALSE;

        if (objFarAddress.IsUnknownAddress())
        {
            bDnsQueryRequired = IMS_TRUE;

            IpAddress objHostIp;

            if (GetTransportHelper()->GetHostByName(objIpAddr, strFarAddress, objHostIp))
            {
                objFarAddress = objHostIp;
                bDnsQueryRequired = IMS_FALSE;
            }
        }

        if (bDnsQueryRequired)
        {
            // Do DNS query ...
            INetworkConnection* piConnection =
                    NetworkService::GetNetworkService()->FindConnection(objIpAddr);

            if (piConnection == IMS_NULL)
            {
                SipStack::FreeAddrSpec(pAddrSpec);

                IMS_TRACE_E(0, "Finding a network connection failed", 0, 0, 0);
                return IMS_FALSE;
            }

            ImsList<IpAddress> objIpAddrs;
            IMS_SINT32 nIpVersion = 0;

            if (objIpAddr.IsIPv4Address())
            {
                nIpVersion = IpAddress::IPV4;
            }
            else if (objIpAddr.IsIPv6Address())
            {
                nIpVersion = IpAddress::IPV6;
            }

            if (piConnection->GetHostByName(strFarAddress, objIpAddrs, nIpVersion) <= 0)
            {
                SipStack::FreeAddrSpec(pAddrSpec);

                IMS_TRACE_E(0, "Getting host address by the name (%s) failed",
                        SipDebug::GetIp(strFarAddress), 0, 0);
                return IMS_FALSE;
            }

            if (objIpAddrs.IsEmpty())
            {
                SipStack::FreeAddrSpec(pAddrSpec);

                IMS_TRACE_E(0, "No entry in the DNS query (%s) result",
                        SipDebug::GetIp(strFarAddress), 0, 0);
                return IMS_FALSE;
            }

            objFarAddress = objIpAddrs.GetAt(0);
        }
#endif

        SetIpAddress(objFarAddress, TA_FAR);

        // Check if an "maddr" parameter exists in the Requsest-URI.
        // If so, this request MUST be sent to the host in the "maddr" parameter.
        strTemp = SipStack::GetParameter(pAddrSpec, AString(SipAddress::PARAM_MADDR));

        if (strTemp.GetLength() > 0)
        {
            // Overwrite the host information using "maddr" field
            SetIpAddress(IpAddress(strTemp), TA_FAR);
        }

        // If a "method" parameter is present, it should be stripped out.
        (void)SipStack::RemoveParameter(AString(SipAddress::PARAM_METHOD), pAddrSpec);

        // TLS feature
        //  If the Request-URI contains SIPS scheme, then irrespective of the Target URI
        //  whether the topmost Route header or Request-Line, the TLS procedures need
        //  to be used. This can be achieved by modifying the dType of addr-spec.
        //
        //  This is to handle the case in which the final destination is identified by a SIPS URI
        //  and the outbound proxy is a loose router having SIP URI.
        //  In this case also, the message SHOULD go over TLS.
        SipAddrSpec* pReqAddrSpec = SipStack::GetRequestUri(pSipMsg);

        if ((pReqAddrSpec != IMS_NULL) && SipStack::IsUriSchemeSips(pReqAddrSpec))
        {
            SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_FAR);
        }

        SipStack::FreeAddrSpec(pReqAddrSpec);

        // LOG_EXCLUDING_SERVER_INFO
        IMS_TRACE_D("UpdateDestinationInfo :: IP (%s), Port (%d)",
                SipRtConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId())
                        ? "xxx"
                        : SipDebug::GetIp(GetIpAddress(TA_FAR)),
                GetPort(TA_FAR), 0);
    }
    else
    {
        // For TEL/IM/PRES URI scheme, we need to resolve the address using DNS query.
        IMS_TRACE_D("UpdateDestinationInfo :: No SIP/SIPS URI scheme (not implemented)", 0, 0, 0);
        SetTransportProtocolByConfig();
    }

    SipStack::FreeAddrSpec(pAddrSpec);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_SINT32 SipClientTransport::ValidateViaHeader(IN ::SipMessage* pSipMsg)
{
    // If the address / port in the topmost Via header does not matches
    // with the current transport information, the message SHOULD be discarded.

    // If more than one Via header field value is present, it SHOULD be discarded by UAC.
    if (SipStack::GetHeaderCount(pSipMsg, ISipHeader::VIA) != 1)
    {
        SipPrivate::SetLastError(SipError::INVALID_MESSAGE);
        return SipPrivate::MESSAGE_DISCARDED;
    }

    IMS_SINT32 nPort = Sip::PORT_UNSPECIFIED;
    AString strHost;

    if (!GetHostNPortFromViaHeader(pSipMsg, strHost, nPort))
    {
        SipPrivate::SetLastError(SipError::INVALID_MESSAGE);
        return SipPrivate::MESSAGE_FAILED;
    }

    IpAddress objViaHost(strHost);

    // If the address or port is not matched, then the message SHOULD be discarded.
    const SipTransportAddress& objNearEnd = GetAddress(TA_NEAR);

    if (!objViaHost.Equals(objNearEnd.GetIpAddress()) || (nPort != objNearEnd.GetPort()))
    {
        SipPrivate::SetLastError(SipError::VIA_ADDRESS_MISMATCH);
        return SipPrivate::MESSAGE_DISCARDED;
    }

    return SipPrivate::MESSAGE_VALID;
}

PROTECTED VIRTUAL void SipClientTransport::Socket_NotifyError(
        IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode)
{
    if (m_pServerSocket != pSocket)
    {
        IMS_TRACE_D("ClientTransport :: server socket is not matched", 0, 0, 0);

        SipTransport::Socket_NotifyError(pSocket, nErrorCode);
        return;
    }

    IMS_TRACE_D("ClientTransport :: Error (%d)", nErrorCode, 0, 0);

    if (m_pServerSocket != IMS_NULL)
    {
        GetTransportHelper()->Destroy(m_pServerSocket, this);
        m_pServerSocket = IMS_NULL;
    }

    NotifyTransportError(nErrorCode);
}

PRIVATE
void SipClientTransport::SetTransportProtocolByConfig()
{
    IMS_SINT32 nTransportType = SipConfigProxy::GetTransportType(GetSlotId());

    IMS_TRACE_D("SetTransportProtocolByConfig: transportType=%d", nTransportType, 0, 0);

    if (nTransportType == SipConfig::TRANSPORT_TYPE_UDP)
    {
        // SIP/2.0/UDP
        SetProtocol(SipTransportAddress::PROTOCOL_UDP, TA_NEAR);
        SetProtocol(SipTransportAddress::PROTOCOL_UDP, TA_FAR);
        SetExplicitTargetProtocol(IMS_TRUE);
    }
    else if (nTransportType == SipConfig::TRANSPORT_TYPE_TCP)
    {
        // SIP/2.0/TCP
        SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_NEAR);
        SetProtocol(SipTransportAddress::PROTOCOL_TCP, TA_FAR);
    }
    else if (nTransportType == SipConfig::TRANSPORT_TYPE_DYNAMIC_UDP_TCP)
    {
        // Keep the current transport protocol and
        // determine the transport protocol based on the size of SIP message.
    }
    else if (nTransportType == SipConfig::TRANSPORT_TYPE_TLS)
    {
        // SIP/2.0/TLS
        SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_NEAR);
        SetProtocol(SipTransportAddress::PROTOCOL_TLS, TA_FAR);
    }
}

PRIVATE GLOBAL IMS_BOOL SipClientTransport::IsSameHostAndPort(
        IN SipAddrSpec* pAddrSpec1, IN SipAddrSpec* pAddrSpec2)
{
    if ((pAddrSpec1 == IMS_NULL) || (pAddrSpec2 == IMS_NULL))
    {
        return IMS_FALSE;
    }

    AString strHost1;
    IMS_UINT32 nPort1;

    (void)SipStack::GetHostAndPort(pAddrSpec1, strHost1, nPort1);

    // Adjust the port number
    if (SipStack::IsLastErrorNoExist())
    {
        if (SipStack::IsUriSchemeSips(pAddrSpec1))
        {
            nPort1 = Sip::PORT_5061;
        }
        else
        {
            nPort1 = Sip::PORT_5060;
        }
    }
    else
    {
        if (nPort1 == Sip::PORT_UNSPECIFIED)
        {
            nPort1 = Sip::PORT_5060;
        }
    }

    AString strHost2;
    IMS_UINT32 nPort2;

    (void)SipStack::GetHostAndPort(pAddrSpec2, strHost2, nPort2);

    // Adjust the port number
    if (SipStack::IsLastErrorNoExist())
    {
        if (SipStack::IsUriSchemeSips(pAddrSpec2))
        {
            nPort2 = Sip::PORT_5061;
        }
        else
        {
            nPort2 = Sip::PORT_5060;
        }
    }
    else
    {
        if (nPort2 == Sip::PORT_UNSPECIFIED)
        {
            nPort2 = Sip::PORT_5060;
        }
    }

    if (nPort1 != nPort2)
    {
        return IMS_FALSE;
    }

    IpAddress objIpAddr1(strHost1);
    IpAddress objIpAddr2(strHost2);

    return objIpAddr1.Equals(objIpAddr2);
}
