#ifndef _SIP_H_
#define _SIP_H_

#include "TextParser.h"
#include "SipError.h"

/**
 * @brief This class defines a constant values for SIP handling.
 */
class SIP
{
public:
    /// Types of SIP URI scheme supported by SIP stack
    enum
    {
        URI_SCHEME_NONE = (-1),
        URI_SCHEME_SIP = 0,
        URI_SCHEME_SIPS,
        URI_SCHEME_TEL,
        URI_SCHEME_IM,
        URI_SCHEME_PRES,
        URI_SCHEME_MAX
    };

    /// Types of the transport supported by SIP stack
    enum
    {
        TRANSPORT_ANY = 0,
        TRANSPORT_UDP,
        TRANSPORT_TCP,
        TRANSPORT_TLS,
        TRANSPORT_MAX,
    };

    /// MULTI_REG_TRANSPORT :: Extension for transport protocol selection\n
    /// It will be provided by the additional parameters when creating an SCN.
    /// Parameter - name: "transport_ext"
    enum
    {
        TRANSPORT_EXT_ANY = 0x0000,
        TRANSPORT_EXT_UDP = 0x0001,
        TRANSPORT_EXT_TCP = 0x0002,
        TRANSPORT_EXT_TLS = 0x0004,
        TRANSPORT_EXT_IPSEC = 0x0010,
        TRANSPORT_EXT_IPSEC_UDP_ENC = 0x0020,
        TRANSPORT_EXT_TCP_ONLY = 0x0080
    };

    /// SIP default port numbers
    enum
    {
        /// SIP default port number, UDP / TCP
        PORT_5060 = 5060,
        /// SIPS default port number, TLS
        PORT_5061 = 5061,

        /// Port number for IPSec ("UDP-enc-tun" mode)
        PORT_U_ENC = 4500,

        /// Implicit port number according to URI scheme
        PORT_UNSPECIFIED = 0xFFFF
    };

    /// Various size for interworking with socket interface on SIP message handling
    enum
    {
        /// Default MTU for IPV6
        MTU_IPV6 = 1280,
        /// Default MTU for IPV4
        MTU_IPV4 = 1500
    };

    /// Overheads for SIP packet
    enum
    {
        /// 200 bytes for SIP response
        PACKET_OVERHEAD_SIP = 200,
        /// ESP headers for IMS IPSec
        PACKET_OVERHEAD_ESP = 60,
        /// TCP headers
        PACKET_OVERHEAD_TCP = 20,
        /// IP headers (v6)
        PACKET_OVERHEAD_IPV6 = 40,
        /// IP headers (v4)
        PACKET_OVERHEAD_IPV4 = 20,
        /// ePDG IP/TCP/IPSec headers
        PACKET_OVERHEAD_EPDG = 60
    };

public:
    inline static IMS_BOOL IsPortSpecified(IN IMS_SINT32 nPort)
    { return ((nPort > 0) && (nPort < PORT_UNSPECIFIED)); }

public:
    //// Connection related constants
    /// "sip"
    static const IMS_CHAR CONNECTION_SCHEME_SIP[];
    /// "sips"
    static const IMS_CHAR CONNECTION_SCHEME_SIPS[];

    // Constant values - General tokens for parsing
    static const IMS_CHAR STR_SIP_VERSION[];

    static const IMS_CHAR STR_SIP_VERSION_ONLY[];

    static const IMS_CHAR STR_SIP[];
    static const IMS_CHAR STR_SIPS[];
    static const IMS_CHAR STR_TEL[];
    static const IMS_CHAR STR_IM[];
    static const IMS_CHAR STR_PRES[];

    static const IMS_CHAR STR_UDP[];
    static const IMS_CHAR STR_TCP[];
    static const IMS_CHAR STR_TLS[];

    static const IMS_CHAR STR_UDP_CAPS[];
    static const IMS_CHAR STR_TCP_CAPS[];
    static const IMS_CHAR STR_TLS_CAPS[];

    static const IMS_CHAR STR_BRANCH_MAGIC_COOKIE[];
#if 0
    static const IMS_CHAR STR_TAG_MAGIC_COOKIE[];
#endif

    // Constant values - Header tokens for parsing
    static const IMS_CHAR STR_100REL[];
    static const IMS_CHAR STR_ACTIVE[];
    static const IMS_CHAR STR_APPLICATION_SDP[];
    static const IMS_CHAR STR_EARLY_SESSION[];
    static const IMS_CHAR STR_FROM_CHANGE[];
    static const IMS_CHAR STR_GRUU[];
    static const IMS_CHAR STR_MULTIPART[];
    static const IMS_CHAR STR_MULTIPART_MIXED[];
    static const IMS_CHAR STR_REFER[];
    static const IMS_CHAR STR_PENDING[];
    static const IMS_CHAR STR_SEC_AGREE[];
    static const IMS_CHAR STR_TERMINATED[];

    // Constant values - Parameter tokens for parsing
    static const IMS_CHAR STR_BOUNDARY[];
    static const IMS_CHAR STR_BRANCH[];
    static const IMS_CHAR STR_EXPIRES[];
    static const IMS_CHAR STR_KEEP[];
    static const IMS_CHAR STR_LR[];
    static const IMS_CHAR STR_METHOD[];
    static const IMS_CHAR STR_OB[];
    static const IMS_CHAR STR_RPORT[];
    static const IMS_CHAR STR_RECEIVED[];
    static const IMS_CHAR STR_REG_ID[];
    static const IMS_CHAR STR_SIP_INSTANCE[];
    static const IMS_CHAR STR_TYPE[];
    // Non-standard parameters
    // MULTI_REG_TRANSPORT
    static const IMS_CHAR STR_TRANSPORT_EXT[];

    static const IMS_CHAR STR_FROM_TAG[];
    static const IMS_CHAR STR_TO_TAG[];

    static const AString STR_TAG;
};

#endif // _SIP_H_
