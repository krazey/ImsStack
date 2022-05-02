/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090614  toastops@                 Created
    </table>

    Description

*/

#include "SipHeaderName.h"



PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ALLOW[] = "Allow";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ALLOW_EVENTS[] = "Allow-Events";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::AUTHORIZATION[] = "Authorization";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CALL_ID[] = "Call-ID";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTACT[] = "Contact";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_DISPOSITION[] = "Content-Disposition";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_ENCODING[] = "Content-Encoding";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_LENGTH[] = "Content-Length";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_TYPE[] = "Content-Type";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CSEQ[] = "CSeq";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::EVENT[] = "Event";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::EXPIRES[] = "Expires";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ACCEPT[] = "Accept";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::MIN_EXPIRES[] = "Min-Expires";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::FROM[] = "From";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::MAX_FORWARDS[] = "Max-Forwards";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::MIME_VERSION[] = "Mime-Version";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PRIVACY[] = "Privacy";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_PREFERRED_IDENTITY[] = "P-Preferred-Identity";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_ASSERTED_IDENTITY[] = "P-Asserted-Identity";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::MIN_SE[] = "Min-SE";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PATH[] = "Path";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_ASSOCIATED_URI[] = "P-Associated-URI";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_CALLED_PARTY_ID[] = "P-Called-Party-ID";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_VISITED_NETWORK_ID[] = "P-Visited-Network-ID";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_CHARGING_FUNCTION_ADDRESSES[] = "P-Charging-Function-Addresses";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_ACCESS_NETWORK_INFO[] = "P-Access-Network-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_CHARGING_VECTOR[] = "P-Charging-Vector";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SERVICE_ROUTE[] = "Service-Route";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::HISTORY_INFO[] = "History-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REQUEST_DISPOSITION[] = "Request-Disposition";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ACCEPT_CONTACT[] = "Accept-Contact";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REJECT_CONTACT[] = "Reject-Contact";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::JOIN[] = "Join";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SIP_IF_MATCH[] = "SIP-If-Match";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SIP_ETAG[] = "SIP-ETag";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PROXY_AUTHENTICATE[] = "Proxy-Authenticate";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PROXY_AUTHORIZATION[] = "Proxy-Authorization";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RACK[] = "RAck";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RECORD_ROUTE[] = "Record-Route";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REFERRED_BY[] = "Referred-By";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REFER_TO[] = "Refer-To";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REPLACES[] = "Replaces";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REQUIRE[] = "Require";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ROUTE[] = "Route";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RSEQ[] = "RSeq";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SECURITY_CLIENT[] = "Security-Client";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SECURITY_VERIFY[] = "Security-Verify";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SECURITY_SERVER[] = "Security-Server";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SESSION_EXPIRES[] = "Session-Expires";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SUBSCRIPTION_STATE[] = "Subscription-State";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SUPPORTED[] = "Supported";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::TIMESTAMP[] = "Timestamp";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::TO[] = "To";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::UNSUPPORTED[] = "Unsupported";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::VIA[] = "Via";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::WARNING[] = "Warning";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::WWW_AUTHENTICATE[] = "WWW-Authenticate";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RETRY_AFTER[] = "Retry-After";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_EARLY_MEDIA[] = "P-Early-Media";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RESOURCE_PRIORITY[] = "Resource-Priority";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ACCEPT_RESOURCE_PRIORITY[] = "Accept-Resource-Priority";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::DATE[] = "Date";

// Unknown header types in the current stack release
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ACCEPT_ENCODING[] = "Accept-Encoding";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ACCEPT_LANGUAGE[] = "Accept-Language";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ALERT_INFO[] = "Alert-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ANSWER_MODE[] = "Answer-Mode";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::AUTHENTICATION_INFO[] = "Authentication-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CALL_INFO[] = "Call-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_DESCRIPTION[] = "Content-Description";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_ID[] = "Content-ID";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_LANGUAGE[] = "Content-Language";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CONTENT_TRANSFER_ENCODING[] = "Content-Transfer-Encoding";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::DIVERSION[] = "Diversion";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ERROR_INFO[] = "Error-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::FEATURE_CAPS[] = "Feature-Caps";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::FLOW_TIMER[] = "Flow-Timer";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::GEOLOCATION[] = "Geolocation";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::GEOLOCATION_ROUTING[] = "Geolocation-Routing";
PUBLIC GLOBAL
// Compact Form : y
const IMS_CHAR SIPHeaderName::IDENTITY[] = "Identity";
PUBLIC GLOBAL
// Compact Form : n
const IMS_CHAR SIPHeaderName::IDENTITY_INFO[] = "Identity-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::IN_REPLY_TO[] = "In-Reply-To";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::INFO_PACKAGE[] = "Info-Package";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::ORGANIZATION[] = "Organization";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_ANSWER_STATE[] = "P-Answer-State";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_MEDIA_AUTHORIZATION[] = "P-Media-Authorization";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_PREFERRED_SERVICE[] = "P-Preferred-Service";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_PROFILE_KEY[] = "P-Profile-Key";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_REFUSED_URI_LIST[] = "P-Refused-URI-List";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_SERVED_USER[] = "P-Served-User";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::P_USER_DATABASE[] = "P-User-Database";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PERMISSION_MISSING[] = "Permission-Missing";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::POLICY_CONTACT[] = "Policy-Contact";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::POLICY_ID[] = "Policy-ID";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PRIV_ANSWER_MODE[] = "Priv-Answer-Mode";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PRIORITY[] = "Priority";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::PROXY_REQUIRE[] = "Proxy-Require";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REASON[] = "Reason";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RECV_INFO[] = "Recv-Info";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REFER_SUB[] = "Refer-Sub";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::REPLY_TO[] = "Reply-To";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::RESPONSE_KEY[] = "Response-Key";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SERVER[] = "Server";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SESSION_ID[] = "Session-ID";
PUBLIC GLOBAL
// Compact Form : s
const IMS_CHAR SIPHeaderName::SUBJECT[] = "Subject";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::SUPPRESS_IF_MATCH[] = "Suppress-If-Match";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::TARGET_DIALOG[] = "Target-Dialog";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::TRIGGER_CONSENT[] = "Trigger-Consent";
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::USER_AGENT[] = "User-Agent";

// Compact form: u, i, m, e, l, c, o, f, d, a, j, b, r, x, k, t, v, y, n, s
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_ALLOW_EVENTS = 'u';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_CALL_ID = 'i';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_CONTACT = 'm';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_CONTENT_ENCODING = 'e';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_CONTENT_LENGTH = 'l';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_CONTENT_TYPE = 'c';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_EVENT = 'o';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_FROM = 'f';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_REQUEST_DISPOSITION = 'd';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_ACCEPT_CONTACT = 'a';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_REJECT_CONTACT = 'j';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_REFERRED_BY = 'b';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_REFER_TO = 'r';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_SESSION_EXPIRES = 'x';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_SUPPORTED = 'k';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_TO = 't';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_VIA = 'v';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_IDENTITY = 'y';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_IDENTITY_INFO = 'n';
PUBLIC GLOBAL
const IMS_CHAR SIPHeaderName::CF_SUBJECT = 's';
