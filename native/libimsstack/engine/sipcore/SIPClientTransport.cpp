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
#include "ServiceNetwork.h"
#include "NATHelper.h"
#include "ISIPHeader.h"
#include "SIPPrivate.h"
#include "SIPFeatures.h"
#include "SIPDebug.h"
#include "SIPConfigProxy.h"
#include "SIPMessageBuffer.h"
#include "SIPUtil.h"
#include "SIPAddress.h"
#include "SIPRTConfigUtils.h"
#include "SIPTransportHelper.h"
#include "SIPClientTransport.h"
#include "private/SipConfig.h"

#define __IMS_DNS_QUERY_SIP__

__IMS_TRACE_TAG_SIP__;



PUBLIC
SIPClientTransport::SIPClientTransport(IN IMS_SINT32 nSlotId)
    : SIPTransport(nSlotId, TYPE_CLIENT)
    , pServerSocket(IMS_NULL)
    , strExtensionTokenForViaBranch(AString::ConstNull())
{
}

PUBLIC
SIPClientTransport::~SIPClientTransport()
{
    if (pServerSocket != IMS_NULL)
    {
        GetTransportHelper()->Destroy(pServerSocket, this);
        pServerSocket = IMS_NULL;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPClientTransport", 0, 0, 0);
#endif
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC VIRTUAL
IMS_BOOL SIPClientTransport::FormViaHeader(IN_OUT SipMessage *&pstMessage,
        IN CONST SIPProfile *pSIPProfile/* = IMS_NULL*/)
{
    //---------------------------------------------------------------------------------------------

    // Update transport information with a default values if not provisioned
    CorrectNearEndAddress();

    // Form Via header
    // If topmost Via header already exists, do not insert a Via header.
    if (SIPStack::GetHeaderCount(pstMessage, ISIPHeader::VIA) > 0)
    {
        IMS_TRACE_D("The Via header already exist", 0, 0, 0);
        return IMS_TRUE;
    }

    AString strSentBy;
    AString strPort;
    AString strSentProtocol(SIP::STR_SIP_VERSION);
    const SIPTransportAddress &objNearEnd = GetAddress(TA_NEAR);
    const SIPTransportAddress &objFarEnd = GetAddress(TA_FAR);

    strSentProtocol += TextParser::CHAR_SLASH;

    if (objFarEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TCP)
        strSentProtocol += SIP::STR_TCP_CAPS;
    else if (objFarEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TLS)
        strSentProtocol += SIP::STR_TLS_CAPS;
    else
    {
        // Overwrites the transport protocol by the local configuration...
        IMS_SINT32 nProtocol = SIP::TRANSPORT_EXT_ANY;

        if (!IsExplicitTargetProtocolSelected())
        {
            if (IsTargetProtocolConfigured())
            {
                IMS_SINT32 nTransportExt = GetTransportExt();

                if ((nTransportExt & SIP::TRANSPORT_EXT_TCP) != 0)
                {
                    nProtocol = SIP::TRANSPORT_EXT_TCP;
                }
                else if ((nTransportExt & SIP::TRANSPORT_EXT_TLS) != 0)
                {
                    nProtocol = SIP::TRANSPORT_EXT_TLS;
                }
            }
            else
            {
                IMS_SINT32 nTransportType =
                        SIPConfigProxy::GetTransportType(GetSlotId(), pSIPProfile);

                if (nTransportType == SipConfig::TRANSPORT_TYPE_TCP)
                {
                    nProtocol = SIP::TRANSPORT_EXT_TCP;
                }
                else if (nTransportType == SipConfig::TRANSPORT_TYPE_TLS)
                {
                    nProtocol = SIP::TRANSPORT_EXT_TLS;
                }
            }
        }

        if (nProtocol == SIP::TRANSPORT_EXT_TCP)
        {
            strSentProtocol += SIP::STR_TCP_CAPS;
            SetProtocol(SIPTransportAddress::PROTOCOL_TCP, TA_NEAR);
            SetProtocol(SIPTransportAddress::PROTOCOL_TCP, TA_FAR);
        }
        else if (nProtocol == SIP::TRANSPORT_EXT_TLS)
        {
            strSentProtocol += SIP::STR_TLS_CAPS;
            SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_NEAR);
            SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_FAR);
        }
        else
        {
            strSentProtocol += SIP::STR_UDP_CAPS;
        }
    }

    strPort.SetNumber(objNearEnd.GetPort());

    IPAddress objLocalAddress(IPAddress::NONE);

    // IMS_IPSEC_UDP_ENC
    if (IsIPSecUDPEncRequired())
    {
        if (NATHelper::IsNATResolverRequired())
        {
            objLocalAddress = NATHelper::GetInstance()->GetPublicAddress(
                    GetSlotId(), objNearEnd.GetIPAddress());
        }

        if (objLocalAddress.Equals(IPAddress::NONE))
        {
            objLocalAddress = objNearEnd.GetIPAddress();
        }
    }
    else
    {
        objLocalAddress = objNearEnd.GetIPAddress();
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
        IMS_TRACE_E(0, "Unsupported IP address (%s)", SIPDebug::GetIP(objLocalAddress), 0, 0);
        return IMS_FALSE;
    }

    // Append the port number
    strSentBy += TextParser::CHAR_COLON + strPort;

    // Create a Via branch
    AString strBranch = SIPUtil::GenerateViaBranch(strExtensionTokenForViaBranch);

    SipHeader *pstHeader = SIPStack::CreateViaHeader(strSentProtocol, strSentBy, strBranch);

    if (pstHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // "keep" parameter
    if (SIPConfigProxy::IsKeepAliveConfigured(GetSlotId(), pSIPProfile))
    {
        if (!SIPStack::SetParameter(pstHeader, SIP::STR_KEEP, AString::ConstNull()))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }
    }

    if (!SIPStack::PrependHeader(pstHeader, pstMessage))
    {
        SIPStack::FreeHeaderEx(pstHeader);
        return IMS_FALSE;
    }

    // Check a raw SIP message buffer
    if (!IsExplicitTargetProtocolSelected()
            && (objFarEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_UDP))
    {
        RCPtr<SIPMessageBuffer> pMessageBuffer = SIPMessageBuffer::GetInstance();
        IMS_SINT32 nBuffLen = pMessageBuffer->GetLength();
        IMS_BYTE *pBuffer = pMessageBuffer->GetBuffer(GetSlotId());

        IMS_MEM_Memset(pBuffer, 0x00, nBuffLen);

        // Apply the criteria of SIP message size
        if (!SIPStack::EncodeMessage(pstMessage,
                SIPPrivate::GetEncodingOptions(), pBuffer, nBuffLen))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        IMS_TRACE_D("SIPClientTransport :: transport protocol selection - length(%d)",
                nBuffLen, 0, 0);

        if (nBuffLen > SIPConfigProxy::GetTcpCriterionLength(GetSlotId(), pSIPProfile))
        {
            // The sent-protocol in Via header needs to modify with TCP.
            strSentProtocol = SIP::STR_SIP_VERSION;
            strSentProtocol += TextParser::CHAR_SLASH;
            strSentProtocol += SIP::STR_TCP_CAPS;

            if (!SIPStack::UpdateSentProtocol(pstMessage, strSentProtocol))
            {
                SIPStack::FreeHeaderEx(pstHeader);
                return IMS_FALSE;
            }

            // Update the transport protocol of a far end point
            SetProtocol(SIPTransportAddress::PROTOCOL_TCP, TA_FAR);

            IMS_TRACE_D("SIPClientTransport :: sent-protocol is changed to TCP", 0, 0, 0);
        }
    }

    // "rport" parameter : UDP & unprotected port only
    IMS_SINT32 nPortC = GetPortC();

    if (SIPConfigProxy::IsRportConfigured(GetSlotId(), pSIPProfile)
            && strSentProtocol.Contains(SIP::STR_UDP_CAPS)
            && ((nPortC == 0)
                || (nPortC == SIP::PORT_UNSPECIFIED)
                || (objNearEnd.GetPort() == nPortC)))
    {
        if (!SIPStack::SetParameter(pstHeader, SIP::STR_RPORT, AString::ConstNull()))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }
    }

    SIPStack::FreeHeaderEx(pstHeader);

    // LOG_EXCLUDING_SERVER_INFO
    if (SIPRTConfigUtils::IsMessageHiddenInLog(GetSlotId()))
    {
        IMS_TRACE_D("SIPClientTransport :: via (%s, %s, %s)",
                strSentProtocol.GetStr(), SIPDebug::GetIP(strSentBy), strBranch.GetStr());
    }

    return IMS_TRUE;
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC VIRTUAL
IMS_BOOL SIPClientTransport::ReserveResource(IN CONST SIPProfile *pSIPProfile/* = IMS_NULL*/)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPTransport::ReserveResource(pSIPProfile))
    {
        return IMS_FALSE;
    }

    // If the keep-alive mechanism is applied and the configured transport protocol
    // is TCP/TLS, all the services will be provided by the TCP client socket
    // to support the keep-alive mechanism.
    IMS_SINT32 nTransportType = SIPConfigProxy::GetTransportType(GetSlotId(), pSIPProfile);

    if (SIPConfigProxy::IsKeepAliveConfigured(GetSlotId(), pSIPProfile)
            && (nTransportType == SipConfig::TRANSPORT_TYPE_TCP
                || nTransportType == SipConfig::TRANSPORT_TYPE_TLS))
    {
        IMS_TRACE_D("ClientTransport :: Configured transport protocol (%d) is not required " \
                "a server socket", nTransportType, 0, 0);
        return IMS_TRUE;
    }

    const SIPTransportAddress &objNearEnd = GetAddress(TA_NEAR);
    const SIPTransportAddress &objFarEnd = GetAddress(TA_FAR);

    if ((objFarEnd.GetProtocol() != SIPTransportAddress::PROTOCOL_TCP)
            && (objFarEnd.GetProtocol() != SIPTransportAddress::PROTOCOL_TLS))
    {
        if (objNearEnd.GetPort() == GetPortC())
        {
            // If the server & client port is same,
            // no need to create socket in case of UDP protocol.
            IMS_TRACE_D("ClientTransport :: UDP server socket (%d) will not be created",
                    GetPortC(), 0, 0);
            return IMS_TRUE;
        }
    }

    if (IsTCPConnectionOnlyRequired())
    {
        // Do not create a server socket for the SIP client connection...
        IMS_TRACE_D("ClientTransport :: TCP client connection is only required...", 0, 0, 0);
        return IMS_TRUE;
    }

    // Reserves the listening sockets for incoming SIP message...
    if (pServerSocket != IMS_NULL)
    {
        // If the near port number is changed, then destroy & create a TCP server socket.
        IPAddress objIPA;
        IMS_UINT32 nPort = 0;

        pServerSocket->GetSockName(objIPA, nPort);

        if (objNearEnd.GetPort() == static_cast<IMS_SINT32>(nPort))
        {
            return IMS_TRUE;
        }

        GetTransportHelper()->Destroy(pServerSocket, this);
        pServerSocket = IMS_NULL;
    }

    // RFC5626_FLOW_CONTROL
    if (!IsFlowControlPortConfigured())
    {
        SIPSocketAddress objSA;

        objSA.SetIPAddress(objNearEnd.GetIPAddress());
        objSA.SetPort(objNearEnd.GetPort());

        if ((objFarEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TCP)
                || (objFarEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TLS))
        {
            objSA.SetType(SIPSocketAddress::SOCKET_TCP);

            // If the transport protocol is TLS, SSL socket will be created...
            if (objFarEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TLS)
            {
                objSA.SetSecure(IMS_TRUE);
            }
        }
        else
        {
            objSA.SetType(SIPSocketAddress::SOCKET_UDP);
        }

        pServerSocket = GetTransportHelper()->Create(objSA);

        if (pServerSocket == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating server socket (%s, %d, %d) failed",
                    SIPDebug::GetIP(objSA.GetIPAddress()), objSA.GetPort(), objSA.GetType());
            return IMS_FALSE;
        }

        IMS_TRACE_D("ClientTransport :: server (%s, %d, %d) is created",
                SIPDebug::GetIP(objSA.GetIPAddress()), objSA.GetPort(), objSA.GetType());

        pServerSocket->SetListener(this);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPClientTransport::UpdateDestinationInfo(IN SipMessage *pstMessage,
        IN IMS_BOOL bRoutingLR /* = IMS_TRUE */,
        IN SipAddrSpec *pstImplicitRoute /* = IMS_NULL */)
{
    const AString PARAM_TRANSPORT(SIPAddress::PARAM_TRANSPORT);
    SipAddrSpec *pstAddrSpec;
    IMS_BOOL bImplicitRoutingTransportRequired = IMS_FALSE;
    AString strImplicitRoutingTransport(AString::ConstNull());

    //---------------------------------------------------------------------------------------------

    // When determining the destination (next hop), it has been set.
    if (bRoutingLR)
    {
        pstAddrSpec = SIPStack::GetAddrSpec(pstMessage, ISIPHeader::ROUTE);
    }
    else
    {
        // In case of REGISTER request, it does not require the Route header.
        if (pstImplicitRoute != IMS_NULL)
        {
            pstAddrSpec = pstImplicitRoute;
            SIPStack::AddReference(pstAddrSpec);

            // Store the transport parameter if present
            SipAddrSpec *pstReqUri = SIPStack::GetRequestUri(pstMessage);

            if (pstReqUri != IMS_NULL)
            {
                // FIXME: consider "sips" URI scheme
                if (IsSameHostAndPort(pstAddrSpec, pstReqUri))
                {
                    bImplicitRoutingTransportRequired = IMS_TRUE;
                    strImplicitRoutingTransport
                            = SIPStack::GetParameter(pstReqUri, PARAM_TRANSPORT);
                }

                SIPStack::FreeAddrSpec(pstReqUri);
            }

            IMS_TRACE_I("IMPLICIT_ROUTE (%s,%s) will be applied...",
                    strImplicitRoutingTransport.GetStr(),
                    bImplicitRoutingTransportRequired ? "true" : "false", 0);
        }
        else
        {
            // In case of strict routing or no route set.
            pstAddrSpec = SIPStack::GetRequestUri(pstMessage);
        }
    }

    if (pstAddrSpec == IMS_NULL)
    {
        IMS_TRACE_E(0, "The target URI does not exist...", 0, 0, 0);
        return IMS_FALSE;
    }

    if (SIPStack::IsUriSchemeSIP(pstAddrSpec) || SIPStack::IsUriSchemeSIPS(pstAddrSpec))
    {
        AString strTemp;

        if (SIPStack::IsUriSchemeSIPS(pstAddrSpec))
        {
            SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_NEAR);
        }

        // If a transport parameter exists, check what the transport type is and update
        // both the destination info. as well as the topmost Via of the request being sent out.
        // IMPLICIT_ROUTING
        if (!bRoutingLR && (pstImplicitRoute != IMS_NULL) && bImplicitRoutingTransportRequired)
        {
            // Sets the transport protocol from the original routing information
            strTemp = strImplicitRoutingTransport;
        }
        else
        {
            strTemp = SIPStack::GetParameter(pstAddrSpec, PARAM_TRANSPORT);
        }

        if (!strTemp.IsNULL() && !strTemp.IsEmpty())
        {
            // Update the sent-protocol field in the topmost Via header
            // to reflect the transport type set.

            if (strTemp.EqualsIgnoreCase(SIP::STR_TCP))
            {
                // SIP/2.0/TCP
                SetProtocol(SIPTransportAddress::PROTOCOL_TCP, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SIPTransportAddress::PROTOCOL_TCP, TA_FAR);
            }
            else if (strTemp.EqualsIgnoreCase(SIP::STR_TLS))
            {
                // SIP/2.0/TLS
                SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_FAR);
            }
            else if (strTemp.EqualsIgnoreCase(SIP::STR_UDP))
            {
                if (SIPFeatures::IsTransportParameterUdpIgnoredForOutgoingRequest(GetSlotId()))
                {
                    IMS_TRACE_I("SIP transport parameter(udp) is ignored", 0, 0, 0);
                }
                else
                {
                    SetExplicitTargetProtocol(IMS_TRUE);
                }

                // SIP/2.0/UDP
                SetProtocol(SIPTransportAddress::PROTOCOL_UDP, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SIPTransportAddress::PROTOCOL_UDP, TA_FAR);
            }
            else
            {
                // SIP/2.0/UDP
                SetProtocol(SIPTransportAddress::PROTOCOL_UDP, TA_NEAR);
                // Update the destination info. scheme.
                SetProtocol(SIPTransportAddress::PROTOCOL_UDP, TA_FAR);
            }
        }

        // Set the destination address & port number
        AString strFarAddress;
        IMS_UINT32 nFarPort;

        (void) SIPStack::GetHostAndPort(pstAddrSpec, strFarAddress, nFarPort);

        // If the port number does not exist,
        // set a default port number according to the URI format.
        if (SIPStack::IsLastErrorNoExist())
        {
            if (SIPStack::IsUriSchemeSIPS(pstAddrSpec))
            {
                nFarPort = SIP::PORT_5061;
            }
            else
            {
                nFarPort = SIP::PORT_5060;
            }
        }
        else
        {
            if (nFarPort == SIP::PORT_UNSPECIFIED)
            {
                nFarPort = SIP::PORT_5060;
            }
        }

        SetPort(nFarPort, TA_FAR);

        IPAddress objFarAddress(strFarAddress);

#if defined(__IMS_DNS_QUERY_SIP__)
        // LOCAL_DNS_QUERY
        const IPAddress &objIPA = GetIPAddress();
        IMS_BOOL bDnsQueryRequired = IMS_FALSE;

        if (objFarAddress.IsUnknownAddress())
        {
            bDnsQueryRequired = IMS_TRUE;

            IPAddress objHostIP;

            if (GetTransportHelper()->GetHostByName(objIPA, strFarAddress, objHostIP))
            {
                objFarAddress = objHostIP;
                bDnsQueryRequired = IMS_FALSE;
            }
        }

        if (bDnsQueryRequired)
        {
            // Do DNS query ...
            INetworkConnection *piConnection =
                    NetworkService::GetNetworkService()->FindConnection(objIPA);

            if (piConnection == IMS_NULL)
            {
                SIPStack::FreeAddrSpec(pstAddrSpec);

                IMS_TRACE_E(0, "Finding a network connection failed", 0, 0, 0);
                return IMS_FALSE;
            }

            IMSList<IPAddress> objIPAddresses;
            IMS_SINT32 nIPVersion = 0;

            if (objIPA.IsIPv4Address())
            {
                nIPVersion = IPAddress::IPV4;
            }
            else if (objIPA.IsIPv6Address())
            {
                nIPVersion = IPAddress::IPV6;
            }

            if (piConnection->GetHostByName(strFarAddress, objIPAddresses, nIPVersion) <= 0)
            {
                SIPStack::FreeAddrSpec(pstAddrSpec);

                IMS_TRACE_E(0, "Getting host address by the name (%s) failed",
                        SIPDebug::GetIP(strFarAddress), 0, 0);
                return IMS_FALSE;
            }

            if (objIPAddresses.IsEmpty())
            {
                SIPStack::FreeAddrSpec(pstAddrSpec);

                IMS_TRACE_E(0, "No entry in the DNS query (%s) result",
                        SIPDebug::GetIP(strFarAddress), 0, 0);
                return IMS_FALSE;
            }

            objFarAddress = objIPAddresses.GetAt(0);
        }
#endif

        SetIPAddress(objFarAddress, TA_FAR);

        // Check if an "maddr" parameter exists in the Requsest-URI.
        // If so, this request MUST be sent to the host in the "maddr" parameter.
        strTemp = SIPStack::GetParameter(pstAddrSpec, AString(SIPAddress::PARAM_MADDR));

        if (strTemp.GetLength() > 0)
        {
            // Overwrite the host information using "maddr" field
            SetIPAddress(IPAddress(strTemp), TA_FAR);
        }

        // If a "method" parameter is present, it should be stripped out.
        (void) SIPStack::RemoveParameter(AString(SIPAddress::PARAM_METHOD), pstAddrSpec);

        // TLS feature
        //  If the Request-URI contains SIPS scheme, then irrespective of the Target URI
        //  whether the topmost Route header or Request-Line, the TLS procedures need
        //  to be used. This can be achieved by modifying the dType of addr-spec.
        //
        //  This is to handle the case in which the final destination is identified by a SIPS URI
        //  and the outbound proxy is a loose router having SIP URI.
        //  In this case also, the message SHOULD go over TLS.
        SipAddrSpec *pstReqAddrSpec = SIPStack::GetRequestUri(pstMessage);

        if ((pstReqAddrSpec != IMS_NULL) && SIPStack::IsUriSchemeSIPS(pstReqAddrSpec))
        {
            SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_FAR);
        }

        SIPStack::FreeAddrSpec(pstReqAddrSpec);

        // LOG_EXCLUDING_SERVER_INFO
        IMS_TRACE_D("UpdateDestinationInfo :: IP (%s), Port (%d)",
                SIPRTConfigUtils::IsRoutingInfoHiddenInLog(GetSlotId()) ? \
                        "xxx" : SIPDebug::GetIP(GetIPAddress(TA_FAR)),
                GetPort(TA_FAR), 0);
    }
    else
    {
        // TODO:: For TEL/IM/PRES URI scheme, we need to resolve the address using DNS query.
        IMS_TRACE_D("UpdateDestinationInfo :: No SIP/SIPS URI scheme (not implemented)", 0, 0, 0);
    }

    SIPStack::FreeAddrSpec(pstAddrSpec);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SIPClientTransport::ValidateViaHeader(IN SipMessage *pstMessage)
{
    // If the address / port in the topmost Via header does not matches
    // with the current transport information, the message SHOULD be discarded.

    //---------------------------------------------------------------------------------------------

    // If more than one Via header field value is present, it SHOULD be discarded by UAC.
    if (SIPStack::GetHeaderCount(pstMessage, ISIPHeader::VIA) != 1)
    {
        SIPPrivate::SetLastError(SIPError::INVALID_MESSAGE);
        return SIPPrivate::MESSAGE_DISCARDED;
    }

    IMS_SINT32 nPort = SIP::PORT_UNSPECIFIED;
    AString strHost;

    if (!GetHostNPortFromViaHeader(pstMessage, strHost, nPort))
    {
        SIPPrivate::SetLastError(SIPError::INVALID_MESSAGE);
        return SIPPrivate::MESSAGE_FAILED;
    }

    IPAddress objViaHost(strHost);

    // If the address or port is not matched, then the message SHOULD be discarded.
    const SIPTransportAddress &objNearEnd = GetAddress(TA_NEAR);

    if (!objViaHost.Equals(objNearEnd.GetIPAddress()) || (nPort != objNearEnd.GetPort()))
    {
        SIPPrivate::SetLastError(SIPError::VIA_ADDRESS_MISMATCH);
        return SIPPrivate::MESSAGE_DISCARDED;
    }

    return SIPPrivate::MESSAGE_VALID;
}

/*

Remarks

*/
PUBLIC
void SIPClientTransport::SetExtensionTokenForViaBranch(IN CONST AString &strToken)
{
    //---------------------------------------------------------------------------------------------

    strExtensionTokenForViaBranch = strToken;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void SIPClientTransport::Socket_NotifyError(IN SIPSocket *pSocket, IN IMS_SINT32 nErrorCode)
{
    //---------------------------------------------------------------------------------------------

    if (pServerSocket != pSocket)
    {
        IMS_TRACE_D("ClientTransport :: server socket is not matched", 0, 0, 0);

        SIPTransport::Socket_NotifyError(pSocket, nErrorCode);
        return;
    }

    IMS_TRACE_D("ClientTransport :: Error (%d)", nErrorCode, 0, 0);

    NotifyTransportError(nErrorCode);

    if (pServerSocket != IMS_NULL)
    {
        GetTransportHelper()->Destroy(pServerSocket, this);
        pServerSocket = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL SIPClientTransport::IsSameHostAndPort(IN SipAddrSpec *pstAddrSpec1,
        IN SipAddrSpec *pstAddrSpec2)
{
    AString strHost1;
    AString strHost2;
    IMS_UINT32 nPort1;
    IMS_UINT32 nPort2;

    //---------------------------------------------------------------------------------------------

    if ((pstAddrSpec1 == IMS_NULL) || (pstAddrSpec2 == IMS_NULL))
    {
        return IMS_FALSE;
    }

    (void) SIPStack::GetHostAndPort(pstAddrSpec1, strHost1, nPort1);

    // Adjust the port number
    if (SIPStack::IsLastErrorNoExist())
    {
        if (SIPStack::IsUriSchemeSIPS(pstAddrSpec1))
        {
            nPort1 = SIP::PORT_5061;
        }
        else
        {
            nPort1 = SIP::PORT_5060;
        }
    }
    else
    {
        if (nPort1 == SIP::PORT_UNSPECIFIED)
        {
            nPort1 = SIP::PORT_5060;
        }
    }

    (void) SIPStack::GetHostAndPort(pstAddrSpec2, strHost2, nPort2);

    // Adjust the port number
    if (SIPStack::IsLastErrorNoExist())
    {
        if (SIPStack::IsUriSchemeSIPS(pstAddrSpec2))
        {
            nPort2 = SIP::PORT_5061;
        }
        else
        {
            nPort2 = SIP::PORT_5060;
        }
    }
    else
    {
        if (nPort2 == SIP::PORT_UNSPECIFIED)
        {
            nPort2 = SIP::PORT_5060;
        }
    }

    if (nPort1 != nPort2)
    {
        return IMS_FALSE;
    }

    IPAddress objIP1(strHost1);
    IPAddress objIP2(strHost2);

    return objIP1.Equals(objIP2);
}
