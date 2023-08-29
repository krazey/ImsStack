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
#ifndef SIP_HEADER_NAME_H_
#define SIP_HEADER_NAME_H_

#include "ImsTypeDef.h"

/**
 * @brief This class defines SIP header names.
 */
class SipHeaderName
{
public:
    SipHeaderName() = delete;

public:
    static const IMS_CHAR ALLOW[];
    static const IMS_CHAR ALLOW_EVENTS[];
    static const IMS_CHAR AUTHORIZATION[];
    static const IMS_CHAR CALL_ID[];
    static const IMS_CHAR CONTACT[];
    static const IMS_CHAR CONTENT_DISPOSITION[];
    static const IMS_CHAR CONTENT_ENCODING[];
    static const IMS_CHAR CONTENT_LENGTH[];
    static const IMS_CHAR CONTENT_TYPE[];
    static const IMS_CHAR CSEQ[];
    static const IMS_CHAR EVENT[];
    static const IMS_CHAR EXPIRES[];
    static const IMS_CHAR ACCEPT[];
    static const IMS_CHAR MIN_EXPIRES[];
    static const IMS_CHAR FROM[];
    static const IMS_CHAR MAX_FORWARDS[];
    static const IMS_CHAR MIME_VERSION[];
    static const IMS_CHAR PRIVACY[];
    static const IMS_CHAR P_PREFERRED_IDENTITY[];
    static const IMS_CHAR P_ASSERTED_IDENTITY[];
    static const IMS_CHAR MIN_SE[];
    static const IMS_CHAR PATH[];
    static const IMS_CHAR P_ASSOCIATED_URI[];
    static const IMS_CHAR P_CALLED_PARTY_ID[];
    static const IMS_CHAR P_VISITED_NETWORK_ID[];
    static const IMS_CHAR P_CHARGING_FUNCTION_ADDRESSES[];
    static const IMS_CHAR P_ACCESS_NETWORK_INFO[];
    static const IMS_CHAR P_CHARGING_VECTOR[];
    static const IMS_CHAR SERVICE_ROUTE[];
    static const IMS_CHAR HISTORY_INFO[];
    static const IMS_CHAR REQUEST_DISPOSITION[];
    static const IMS_CHAR ACCEPT_CONTACT[];
    static const IMS_CHAR REJECT_CONTACT[];
    static const IMS_CHAR JOIN[];
    static const IMS_CHAR SIP_IF_MATCH[];
    static const IMS_CHAR SIP_ETAG[];
    static const IMS_CHAR PROXY_AUTHENTICATE[];
    static const IMS_CHAR PROXY_AUTHORIZATION[];
    static const IMS_CHAR RACK[];
    static const IMS_CHAR RECORD_ROUTE[];
    static const IMS_CHAR REFERRED_BY[];
    static const IMS_CHAR REFER_TO[];
    static const IMS_CHAR REPLACES[];
    static const IMS_CHAR REQUIRE[];
    static const IMS_CHAR ROUTE[];
    static const IMS_CHAR RSEQ[];
    static const IMS_CHAR SECURITY_CLIENT[];
    static const IMS_CHAR SECURITY_VERIFY[];
    static const IMS_CHAR SECURITY_SERVER[];
    static const IMS_CHAR SESSION_EXPIRES[];
    static const IMS_CHAR SUBSCRIPTION_STATE[];
    static const IMS_CHAR SUPPORTED[];
    static const IMS_CHAR TIMESTAMP[];
    static const IMS_CHAR TO[];
    static const IMS_CHAR UNSUPPORTED[];
    static const IMS_CHAR VIA[];
    static const IMS_CHAR WARNING[];
    static const IMS_CHAR WWW_AUTHENTICATE[];
    static const IMS_CHAR RETRY_AFTER[];
    static const IMS_CHAR P_EARLY_MEDIA[];
    static const IMS_CHAR RESOURCE_PRIORITY[];
    static const IMS_CHAR ACCEPT_RESOURCE_PRIORITY[];
    static const IMS_CHAR DATE[];

    static const IMS_CHAR ACCEPT_ENCODING[];
    static const IMS_CHAR ACCEPT_LANGUAGE[];
    static const IMS_CHAR ALERT_INFO[];
    static const IMS_CHAR ANSWER_MODE[];  // RFC 5373
    static const IMS_CHAR AUTHENTICATION_INFO[];
    static const IMS_CHAR CALL_INFO[];
    static const IMS_CHAR CONTENT_LANGUAGE[];
    static const IMS_CHAR ERROR_INFO[];
    static const IMS_CHAR FEATURE_CAPS[];         // RFC 6809 (for Proxy, Registrar, B2BUA)
    static const IMS_CHAR FLOW_TIMER[];           // RFC 5626
    static const IMS_CHAR GEOLOCATION[];          // RFC 6442
    static const IMS_CHAR GEOLOCATION_ERROR[];    // RFC 6442
    static const IMS_CHAR GEOLOCATION_ROUTING[];  // RFC 6442
    static const IMS_CHAR IDENTITY[];             // RFC 4474, Compact Form : y
    static const IMS_CHAR IDENTITY_INFO[];        // RFC 4474, Compact Form : n
    static const IMS_CHAR IN_REPLY_TO[];
    static const IMS_CHAR INFO_PACKAGE[];
    static const IMS_CHAR MAX_BREADTH[];  // RFC 5393
    static const IMS_CHAR ORGANIZATION[];
    static const IMS_CHAR P_ANSWER_STATE[];         // RFC 4964
    static const IMS_CHAR P_ASSERTED_SERVICE[];     // RFC 6050
    static const IMS_CHAR P_MEDIA_AUTHORIZATION[];  // RFC 3313
    static const IMS_CHAR P_PREFERRED_SERVICE[];    // RFC 6050
    static const IMS_CHAR P_PROFILE_KEY[];          // RFC 5002
    static const IMS_CHAR P_REFUSED_URI_LIST[];     // RFC 5318
    static const IMS_CHAR P_USER_DATABASE[];
    static const IMS_CHAR P_SERVED_USER[];       // RFC 5502 (for S-CSCF/AS)
    static const IMS_CHAR PERMISSION_MISSING[];  // RFC 5360
    static const IMS_CHAR POLICY_CONTACT[];      // RFC 6794 (Session Policy Framework)
    static const IMS_CHAR POLICY_ID[];           // RFC 6794 (Session Policy Framework)
    static const IMS_CHAR PRIORITY[];            // RFC 4412
    static const IMS_CHAR PRIV_ANSWER_MODE[];    // RFC 5373
    static const IMS_CHAR PROXY_REQUIRE[];
    static const IMS_CHAR REASON[];
    static const IMS_CHAR RECV_INFO[];
    static const IMS_CHAR REFER_SUB[];  // RFC 4488
    static const IMS_CHAR REPLY_TO[];
    static const IMS_CHAR RESPONSE_KEY[];
    static const IMS_CHAR SERVER[];
    static const IMS_CHAR SESSION_ID[];  // draft-kaplan-insipid-session-id-04 (expires: 140910)
    static const IMS_CHAR SUBJECT[];     // Compact Form : s
    static const IMS_CHAR SUPPRESS_IF_MATCH[];  // RFC 5839
    static const IMS_CHAR TARGET_DIALOG[];      // RFC 4538, "tdialog" option-tag
    static const IMS_CHAR TRIGGER_CONSENT[];    // RFC 5360
    static const IMS_CHAR USER_AGENT[];

    // Unknown header types in the current stack release
    static const IMS_CHAR CELLULAR_NETWORK_INFO[];
    static const IMS_CHAR CONTENT_DESCRIPTION[];
    static const IMS_CHAR CONTENT_ID[];
    static const IMS_CHAR CONTENT_TRANSFER_ENCODING[];
    static const IMS_CHAR DIVERSION[];  // RFC 7544 (Mapping of Diversion and History-Info)
    static const IMS_CHAR RESPONSE_SOURCE[];

    // Compact form: u, i, m, e, l, c, e, f, d, a, j, b, r, x, k, t, v, y, n, s
    static const IMS_CHAR CF_ALLOW_EVENTS;
    static const IMS_CHAR CF_CALL_ID;
    static const IMS_CHAR CF_CONTACT;
    static const IMS_CHAR CF_CONTENT_ENCODING;
    static const IMS_CHAR CF_CONTENT_LENGTH;
    static const IMS_CHAR CF_CONTENT_TYPE;
    static const IMS_CHAR CF_EVENT;
    static const IMS_CHAR CF_FROM;
    static const IMS_CHAR CF_REQUEST_DISPOSITION;
    static const IMS_CHAR CF_ACCEPT_CONTACT;
    static const IMS_CHAR CF_REJECT_CONTACT;
    static const IMS_CHAR CF_REFERRED_BY;
    static const IMS_CHAR CF_REFER_TO;
    static const IMS_CHAR CF_SESSION_EXPIRES;
    static const IMS_CHAR CF_SUPPORTED;
    static const IMS_CHAR CF_TO;
    static const IMS_CHAR CF_VIA;
    static const IMS_CHAR CF_IDENTITY;
    static const IMS_CHAR CF_IDENTITY_INFO;
    static const IMS_CHAR CF_SUBJECT;
};

#endif
