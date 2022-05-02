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
#include "ISipHeader.h"
#include "SIPPrivate.h"
#include "SipDebug.h"
#include "SipAddress.h"
#include "SIPServerTransport.h"

__IMS_TRACE_TAG_SIP__;


PUBLIC
SIPServerTransport::SIPServerTransport(IN IMS_SINT32 nSlotId,
        IN CONST SIPTransportAddress &objTA_NearEnd_,
        IN CONST SIPTransportAddress &objTA_FarEnd_)
    : SIPTransport(nSlotId, SIPTransport::TYPE_SERVER)
{
    SetAddress(objTA_NearEnd_, TA_NEAR);
    SetAddress(objTA_FarEnd_, TA_FAR);

    // Only for TCP client socket
    // The destination information (address / port) will be updated before sending the response,
    // so if the request is received from TCP connection, it needs to be stored
    // in the transport layer.
    // If the socket is already reserved, the transport layer will use the reserved socket.
    StoreResource();
}

PUBLIC VIRTUAL
SIPServerTransport::~SIPServerTransport()
{
#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPServerTransport", 0, 0, 0);
#endif
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC VIRTUAL
IMS_BOOL SIPServerTransport::FormViaHeader(IN_OUT SipMessage *&pstMessage,
        IN CONST SIPProfile */*pSIPProfile = IMS_NULL*/)
{
    // Get the topmost Via header from the message
    SipHeaderBase *pstViaHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::VIA);
    AString strRPort(SIP::STR_RPORT);
    const SIPTransportAddress &objFarEnd = GetAddress(TA_FAR);

    //---------------------------------------------------------------------------------------------

    // Check if the Via header includes a "rport" parameter or not.
    if (SIPStack::HasParameter(pstViaHeader, strRPort))
    {
        // Set the actual received port number from the current transport information
        // "rport" parameter : UDP & unprotected port only
        IMS_SINT32 nPortC = GetPortC();
        AString strSentProtocol = SIPStack::GetSentProtocolFromVia(pstViaHeader);

        if (strSentProtocol.Contains(SIP::STR_UDP_CAPS)
                && ((nPortC == 0)
                    || (nPortC == SIP::PORT_UNSPECIFIED)
                    || (GetPort(TA_NEAR) == nPortC)))
        {
            AString strPort;

            strPort.SetNumber(objFarEnd.GetPort());

            if (!SIPStack::SetParameter(pstViaHeader, strRPort, strPort))
            {
                SIPStack::FreeHeaderEx(pstViaHeader);
                return IMS_FALSE;
            }
        }
    }

    // Examine the value of the "sent-by" parameter in the top Via header field value.
    // If the host portion of the "sent-by" parameter contains a domain name, or
    // if it contains an IP address that differs from the packet source address,
    // the server MUST add a "received" parameter to that Via header field value.
    AString strHost;
    AString strSentBy = SIPStack::GetSentByFromVia(pstViaHeader);
    IMS_SINT32 nPort = SIP::PORT_UNSPECIFIED;

    // Parse the host & port information from "sent-by" field
    SIPTransport::ParseHostNPort(strSentBy, strHost, nPort);

    IPAddress objHost;
    IMS_BOOL bNumericIPFormat = objHost.Parse(strHost);

    // Check if the actual received IP & the host in "sent-by" field are same
    if (!bNumericIPFormat || !objHost.Equals(objFarEnd.GetIPAddress()))
    {
        // The address returned by recvfrom is not same as address put in the Via header.
        // Add the "received" parameter in Via header.
        if (!SIPStack::SetParameter(pstViaHeader,
                AString(SIP::STR_RECEIVED), objFarEnd.GetIPAddress().ToString()))
        {
            SIPStack::FreeHeaderEx(pstViaHeader);
            return IMS_FALSE;
        }
    }

    SIPStack::FreeHeaderEx(pstViaHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL SIPServerTransport::UpdateDestinationInfo(IN SipMessage *pstMessage,
        IN IMS_BOOL /* bRoutingLR = IMS_TRUE */,
        IN SipAddrSpec * /* pstImplicitRoute = IMS_NULL */)
{
    // For a response which is being sent out, updating the destination info. is done as follows:
    //
    // 1. If the Via header contains a "maddr" parameter, send the response to the IP address
    //   it contains and to the port specified in the "sent-by" field of the topmost Via header.
    // 2. If "maddr" is absent, but the "received" parameter is present,
    //   then send the response to the port indicated in the "sent-by" field of the topmost
    //   Via header (if none given, then a default port is 5060).
    // 3. If neither "maddr" nor "received" parameter are present, the destination info. is set
    //   to the address / port of the topmost Via header.
    SipHeaderBase *pstViaHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::VIA);
    AString strSentBy = SIPStack::GetSentByFromVia(pstViaHeader);
    AString strViaHost;
    IMS_SINT32 nViaPort;

    //---------------------------------------------------------------------------------------------

    // Parse the host & port information from "sent-by" field
    SIPTransport::ParseHostNPort(strSentBy, strViaHost, nViaPort);

    SetIPAddress(IPAddress(strViaHost), TA_FAR);
    SetPort(nViaPort, TA_FAR);

    // Now, check if "maddr" & "received" parameter are present
    AString strTemp = SIPStack::GetParameter(pstViaHeader, AString(SIPAddress::PARAM_MADDR));

    if (strTemp.GetLength() > 0)
    {
        // If "maddr" parameter is found, it is the highest precedence.
        // So, update the destination address as this value.
        SetIPAddress(IPAddress(strTemp), TA_FAR);
    }
    else
    {
        // Find the "received" parameter
        strTemp = SIPStack::GetParameter(pstViaHeader, AString(SIP::STR_RECEIVED));

        if (strTemp.GetLength() > 0)
        {
            SetIPAddress(IPAddress(strTemp), TA_FAR);
        }
    }

    // Check if "rport" parameter is present or not
    strTemp = SIPStack::GetParameter(pstViaHeader, AString(SIP::STR_RPORT));

    if (strTemp.GetLength() > 0)
    {
        IMS_SINT32 nRPort = strTemp.ToInt32();

        if (nRPort != 0)
            SetPort(nRPort, TA_FAR);
    }

    // Update the destination transport protocol from "sent-protocol" field in the top Via header
    strTemp = SIPStack::GetSentProtocolFromVia(pstViaHeader);

    // "SIP/2.0/" : TCP / UDP / TLS / SCTP
    if (strTemp.EndsWith(SIP::STR_TLS_CAPS))
    {
        SetProtocol(SIPTransportAddress::PROTOCOL_TLS, TA_FAR);

        // If the port number does not set explicitly,
        // then check the transport protocol & set the port.
        if (GetPort(TA_FAR) == SIP::PORT_UNSPECIFIED)
            SetPort(SIP::PORT_5061, TA_FAR);
    }
    else if (strTemp.EndsWith(SIP::STR_TCP_CAPS))
    {
        SetProtocol(SIPTransportAddress::PROTOCOL_TCP, TA_FAR);

        // If the port number does not set explicitly,
        // then check the transport protocol & set the port.
        if (GetPort(TA_FAR) == SIP::PORT_UNSPECIFIED)
            SetPort(SIP::PORT_5060, TA_FAR);
    }
    else
    {
        SetProtocol(SIPTransportAddress::PROTOCOL_UDP, TA_FAR);

        // If the port number does not set explicitly,
        // then check the transport protocol & set the port.
        if (GetPort(TA_FAR) == SIP::PORT_UNSPECIFIED)
            SetPort(SIP::PORT_5060, TA_FAR);
    }

    SIPStack::FreeHeaderEx(pstViaHeader);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 SIPServerTransport::ValidateViaHeader(IN SipMessage *pstMessage)
{
    // Get the topmost Via header from the message
    SipHeaderBase *pstViaHeader = SIPStack::GetHeader(pstMessage, ISIPHeader::VIA);

    //---------------------------------------------------------------------------------------------

    // Check if there is a mismatch between the protocol in the topmost Via
    // and the actual protocol on which it was received.
    //
    // If the topmost Via header has scheme SIP/2.0/TCP, but actually came on UDP,
    // (or vice versa) flag off error. Application SHOULD respond to this with 400 "Bad Request".
    const SIPTransportAddress &objNearEnd = GetAddress(TA_NEAR);
    AString strSentProtocol = SIPStack::GetSentProtocolFromVia(pstViaHeader);

    // "SIP/2.0/" : 8 characters
    strSentProtocol = strSentProtocol.Mid(8);

    if ((strSentProtocol.EqualsIgnoreCase(SIP::STR_TCP_CAPS)
            && (objNearEnd.GetProtocol() != SIPTransportAddress::PROTOCOL_TCP))
            || (strSentProtocol.EqualsIgnoreCase(SIP::STR_TLS_CAPS)
                && (objNearEnd.GetProtocol() != SIPTransportAddress::PROTOCOL_TLS))
            || (strSentProtocol.EqualsIgnoreCase(SIP::STR_UDP_CAPS)
                && (objNearEnd.GetProtocol() != SIPTransportAddress::PROTOCOL_UDP)))
    {
        // Correct the sent-protocol in Via header
        strSentProtocol = SIPStack::GetSentProtocolFromVia(pstViaHeader);

        // "SIP/2.0/" : 8 characters
        strSentProtocol = strSentProtocol.Left(8);

        if (objNearEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TCP)
            strSentProtocol += SIP::STR_TCP_CAPS;
        else if (objNearEnd.GetProtocol() == SIPTransportAddress::PROTOCOL_TLS)
            strSentProtocol += SIP::STR_TLS_CAPS;
        else
            strSentProtocol += SIP::STR_UDP_CAPS;

        SIPStack::UpdateSentProtocol(pstMessage, strSentProtocol);
        SIPStack::FreeHeaderEx(pstViaHeader);

        IMS_TRACE_E(0, "Transport protocol is mismatched in Via header", 0, 0, 0);

        SIPPrivate::SetLastError(SIPError::VIA_PROTOCOL_MISMATCH);

        return SIPPrivate::MESSAGE_INVALID_400;
    }

    // Free the local reference
    SIPStack::FreeHeaderEx(pstViaHeader);

    return SIPPrivate::MESSAGE_VALID;
}
