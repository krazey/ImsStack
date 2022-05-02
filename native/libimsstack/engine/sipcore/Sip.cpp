/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description
     This class is the class for a global enumerations, definitions and constant variables.
*/

#include "ServiceMemory.h"
#include "SIPPrivate.h"



PUBLIC GLOBAL
const IMS_CHAR SIP::CONNECTION_SCHEME_SIP[] = "sip";
PUBLIC GLOBAL
const IMS_CHAR SIP::CONNECTION_SCHEME_SIPS[] = "sips";

PUBLIC GLOBAL
const IMS_CHAR SIP::STR_SIP_VERSION[] = "SIP/2.0";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_SIP_VERSION_ONLY[] = "2.0";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_SIP[] = "sip";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_SIPS[] = "sips";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TEL[] = "tel";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_IM[] = "im";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_PRES[] = "pres";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_UDP[] = "udp";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TCP[] = "tcp";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TLS[] = "tls";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_UDP_CAPS[] = "UDP";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TCP_CAPS[] = "TCP";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TLS_CAPS[] = "TLS";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_BRANCH_MAGIC_COOKIE[] = "z9hG4bK";
#if 0
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TAG_MAGIC_COOKIE[] = "gims";
#endif

PUBLIC GLOBAL
const IMS_CHAR SIP::STR_100REL[] = "100rel";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_ACTIVE[] = "active";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_APPLICATION_SDP[] = "application/sdp";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_EARLY_SESSION[] = "early-session";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_FROM_CHANGE[] = "from-change";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_GRUU[] = "gruu";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_MULTIPART[] = "multipart";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_MULTIPART_MIXED[] = "multipart/mixed";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_REFER[] = "refer";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_SEC_AGREE[] = "sec-agree";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_PENDING[] = "pending";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TERMINATED[] = "terminated";

PUBLIC GLOBAL
const IMS_CHAR SIP::STR_BOUNDARY[] = "boundary";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_BRANCH[] = "branch";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_EXPIRES[] = "expires";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_KEEP[] = "keep";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_LR[] = "lr";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_METHOD[] = "method";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_OB[] = "ob";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_RPORT[] = "rport";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_RECEIVED[] = "received";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_REG_ID[] = "reg-id";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_SIP_INSTANCE[] = "+sip.instance";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TYPE[] = "type";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TRANSPORT_EXT[] = "transport-ext";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_FROM_TAG[] = "from-tag";
PUBLIC GLOBAL
const IMS_CHAR SIP::STR_TO_TAG[] = "to-tag";
PUBLIC GLOBAL
const AString SIP::STR_TAG("tag");
