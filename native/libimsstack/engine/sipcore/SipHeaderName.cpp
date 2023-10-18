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
#include "SipHeaderName.h"

PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ALLOW[] = "Allow";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ALLOW_EVENTS[] = "Allow-Events";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::AUTHORIZATION[] = "Authorization";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CALL_ID[] = "Call-ID";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTACT[] = "Contact";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_DISPOSITION[] = "Content-Disposition";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_ENCODING[] = "Content-Encoding";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_LENGTH[] = "Content-Length";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_TYPE[] = "Content-Type";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CSEQ[] = "CSeq";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::EVENT[] = "Event";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::EXPIRES[] = "Expires";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ACCEPT[] = "Accept";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::MIN_EXPIRES[] = "Min-Expires";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::FROM[] = "From";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::MAX_FORWARDS[] = "Max-Forwards";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::MIME_VERSION[] = "Mime-Version";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PRIVACY[] = "Privacy";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_PREFERRED_IDENTITY[] = "P-Preferred-Identity";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_ASSERTED_IDENTITY[] = "P-Asserted-Identity";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::MIN_SE[] = "Min-SE";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PATH[] = "Path";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_ASSOCIATED_URI[] = "P-Associated-URI";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_CALLED_PARTY_ID[] = "P-Called-Party-ID";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_VISITED_NETWORK_ID[] = "P-Visited-Network-ID";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_CHARGING_FUNCTION_ADDRESSES[] =
        "P-Charging-Function-Addresses";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_ACCESS_NETWORK_INFO[] = "P-Access-Network-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_CHARGING_VECTOR[] = "P-Charging-Vector";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SERVICE_ROUTE[] = "Service-Route";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::HISTORY_INFO[] = "History-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REQUEST_DISPOSITION[] = "Request-Disposition";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ACCEPT_CONTACT[] = "Accept-Contact";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REJECT_CONTACT[] = "Reject-Contact";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::JOIN[] = "Join";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SIP_IF_MATCH[] = "SIP-If-Match";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SIP_ETAG[] = "SIP-ETag";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PROXY_AUTHENTICATE[] = "Proxy-Authenticate";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PROXY_AUTHORIZATION[] = "Proxy-Authorization";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RACK[] = "RAck";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RECORD_ROUTE[] = "Record-Route";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REFERRED_BY[] = "Referred-By";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REFER_TO[] = "Refer-To";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REPLACES[] = "Replaces";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REQUIRE[] = "Require";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ROUTE[] = "Route";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RSEQ[] = "RSeq";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SECURITY_CLIENT[] = "Security-Client";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SECURITY_VERIFY[] = "Security-Verify";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SECURITY_SERVER[] = "Security-Server";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SESSION_EXPIRES[] = "Session-Expires";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SUBSCRIPTION_STATE[] = "Subscription-State";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SUPPORTED[] = "Supported";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::TIMESTAMP[] = "Timestamp";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::TO[] = "To";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::UNSUPPORTED[] = "Unsupported";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::VIA[] = "Via";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::WARNING[] = "Warning";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::WWW_AUTHENTICATE[] = "WWW-Authenticate";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RETRY_AFTER[] = "Retry-After";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_EARLY_MEDIA[] = "P-Early-Media";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RESOURCE_PRIORITY[] = "Resource-Priority";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ACCEPT_RESOURCE_PRIORITY[] = "Accept-Resource-Priority";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::DATE[] = "Date";

PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ACCEPT_ENCODING[] = "Accept-Encoding";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ACCEPT_LANGUAGE[] = "Accept-Language";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ALERT_INFO[] = "Alert-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ANSWER_MODE[] = "Answer-Mode";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::AUTHENTICATION_INFO[] = "Authentication-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CALL_INFO[] = "Call-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_LANGUAGE[] = "Content-Language";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ERROR_INFO[] = "Error-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::FEATURE_CAPS[] = "Feature-Caps";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::FLOW_TIMER[] = "Flow-Timer";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::GEOLOCATION[] = "Geolocation";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::GEOLOCATION_ERROR[] = "Geolocation-Error";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::GEOLOCATION_ROUTING[] = "Geolocation-Routing";
// Compact Form : y
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::IDENTITY[] = "Identity";
// Compact Form : n
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::IDENTITY_INFO[] = "Identity-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::IN_REPLY_TO[] = "In-Reply-To";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::INFO_PACKAGE[] = "Info-Package";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::MAX_BREADTH[] = "Max-Breadth";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::ORGANIZATION[] = "Organization";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_ANSWER_STATE[] = "P-Answer-State";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_ASSERTED_SERVICE[] = "P-Asserted-Service";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_MEDIA_AUTHORIZATION[] = "P-Media-Authorization";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_PREFERRED_SERVICE[] = "P-Preferred-Service";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_PROFILE_KEY[] = "P-Profile-Key";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_REFUSED_URI_LIST[] = "P-Refused-URI-List";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_SERVED_USER[] = "P-Served-User";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::P_USER_DATABASE[] = "P-User-Database";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PERMISSION_MISSING[] = "Permission-Missing";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::POLICY_CONTACT[] = "Policy-Contact";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::POLICY_ID[] = "Policy-ID";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PRIV_ANSWER_MODE[] = "Priv-Answer-Mode";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PRIORITY[] = "Priority";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::PROXY_REQUIRE[] = "Proxy-Require";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REASON[] = "Reason";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RECV_INFO[] = "Recv-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REFER_SUB[] = "Refer-Sub";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::REPLY_TO[] = "Reply-To";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RESPONSE_KEY[] = "Response-Key";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SERVER[] = "Server";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SESSION_ID[] = "Session-ID";
// Compact Form : s
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SUBJECT[] = "Subject";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::SUPPRESS_IF_MATCH[] = "Suppress-If-Match";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::TARGET_DIALOG[] = "Target-Dialog";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::TRIGGER_CONSENT[] = "Trigger-Consent";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::USER_AGENT[] = "User-Agent";

// Unknown header types in the current stack release
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CELLULAR_NETWORK_INFO[] = "Cellular-Network-Info";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_DESCRIPTION[] = "Content-Description";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_ID[] = "Content-ID";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CONTENT_TRANSFER_ENCODING[] =
        "Content-Transfer-Encoding";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::DIVERSION[] = "Diversion";
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::RESPONSE_SOURCE[] = "Response-Source";

// Compact form: u, i, m, e, l, c, o, f, d, a, j, b, r, x, k, t, v, y, n, s
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_ALLOW_EVENTS = 'u';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_CALL_ID = 'i';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_CONTACT = 'm';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_CONTENT_ENCODING = 'e';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_CONTENT_LENGTH = 'l';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_CONTENT_TYPE = 'c';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_EVENT = 'o';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_FROM = 'f';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_REQUEST_DISPOSITION = 'd';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_ACCEPT_CONTACT = 'a';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_REJECT_CONTACT = 'j';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_REFERRED_BY = 'b';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_REFER_TO = 'r';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_SESSION_EXPIRES = 'x';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_SUPPORTED = 'k';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_TO = 't';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_VIA = 'v';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_IDENTITY = 'y';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_IDENTITY_INFO = 'n';
PUBLIC GLOBAL const IMS_CHAR SipHeaderName::CF_SUBJECT = 's';
