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
#ifndef INTERFACE_SIP_HEADER_H_
#define INTERFACE_SIP_HEADER_H_

#include "AString.h"
#include "ISipObject.h"

class SipAddress;
class SipParameter;

/**
 * @brief This class provides an interface to handle SIP headers.
 */
class ISipHeader : public ISipObject
{
protected:
    virtual ~ISipHeader() = default;

public:
    /**
     * @brief Clones a SIP header.
     *
     * @return Pointer to new SipHeader.
     */
    virtual ISipHeader* Clone() const = 0;

    /**
     * @brief Checks if the given SipHeader is the same.
     *
     * @param piHeader Pointer to ISipHeader which is compared
     * @return If both SIP headers matched, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL Equals(IN const ISipHeader* piHeader) const = 0;

    /**
     * @brief Returns the pointer to SipAddress if this header is a format of SIP address.
     *
     * @return Pointer to SipAddress if the header is a format of SIP address, or null
     */
    virtual const SipAddress* GetSipAddress() const = 0;

    /**
     * @brief Returns the full header value including the header parameters.
     *
     * For example,\n
     *     User1 <sip:user1@ims.com>;tag=1928301774
     *
     * @return If the full header value including the header parameters, returns non-null string.\n
     *         If the header has only a header name, returns empty or null string.
     */
    virtual AString GetHeaderValue() const = 0;

    /**
     * @brief Returns the header name of this SipHeader.
     *
     * @return The header name of this SipHeader.
     */
    virtual const AString& GetName() const = 0;

    /**
     * @brief Returns the pointer to SipParameter of one header parameter.
     *
     * @param strName The parameter name to be returned
     * @return Pointer to SipParameter; Parameter to the specified parameter name.
     */
    virtual const SipParameter* GetParameter(IN const AString& strName) const = 0;

    /**
     * @brief Returns the names of all the header parameters.
     *
     * If there are no header parameters, the method returns an empty list.
     *
     * @param objParamNames Names of the header parameters
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT GetParameterNames(OUT ImsList<AString>& objParamNames) const = 0;

    /**
     * @brief Returns the list of all the header parameters in this SipHeader.
     *
     * @return The list of SipParameter for this SipHeader.
     */
    virtual const ImsList<SipParameter*>& GetParameters() const = 0;

    /**
     * @brief Returns the enumeration type of this SipHeader which corresponds to the header name.
     *
     * @return Enumeration type of this SipHeader.
     */
    virtual IMS_SINT32 GetType() const = 0;

    /**
     * @brief Returns the header value without header parameters.
     *
     * For example,\n
     * for header <sip:user@192.168.200.201>;expires=3600,
     * this method returns <sip:user@192.168.200.201>.\n
     * In the case of an authorization or authentication header,
     * it returns only the authentication scheme e.g. "Digest".
     *
     * @return If the header value without header parameters, returns the header body field.\n
     *         If the value was set to be null or empty string, returns an empty string.
     */
    virtual const AString& GetValue() const = 0;

    /**
     * @brief Returns the header value without header parameters.
     *
     * It is the same method with GetValue()
     * except for returning an integer value for the header value.
     *
     * @return If the header value without header parameters, returns the header body field.\n
     *         If the value can't have an integer value, returns {@code INVALID_INT}.
     */
    virtual IMS_SINT32 GetValueInt() const = 0;

    /**
     * @brief Removes the header parameter if it is found in this SipHeader.
     *
     * @param strName Name of the header parameter
     */
    virtual void RemoveParameter(IN const AString& strName) = 0;

    /**
     * @brief Sets the header name, for example "Contact".
     *
     * If the argument has a leading and trailing white spaces, those are ignored.
     *
     * @param strName Name of the header
     */
    virtual void SetName(IN const AString& strName) = 0;

    /**
     * @brief Sets the value of header parameter.
     *
     * If the parameter does not exist, it will be added.\n
     * For example, for header value "<sip:user@192.168.200.201>" calling
     * SetParameter("expires", "3600") will construct header value
     * "<sip:user@192.168.200.201>;expires=3600".\n
     * If the value is null or empty string, the parameter is interpreted as a parameter
     * without value.
     *
     * @param strName Name of the header parameter
     * @param strValue Value of the header parameter (null or empty string is allowed)
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetParameter(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Sets the header value as string with any header parameters.
     *
     * If the header value is an empty string or null, it means a header with no value.
     *
     * @param strHeaderValue The full header value
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetHeaderValue(IN const AString& strHeaderValue) = 0;

    /**
     * @brief Sets the header value as string without parameters.
     *
     * For example,
     *  for the header "<sip:user@192.168.200.201>", the existing (if any) header parameter values
     * are not modified.\n
     * If the header value is an empty string or NULL, it means a header with no value.
     *
     * For an authorization and authentication header, this method sets
     * the authentication scheme e.g. "Digest".
     *
     * @param strValue The value of the header
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetValue(IN const AString& strValue) = 0;

    /**
     * @brief Sets the header value as string without parameters.
     *
     * It is the same method with SetValue(...)
     * except for setting an integer value for the header value.\n
     * If the header value cannot have an integer value, it fails.
     *
     * @param nValue The value of the header
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetValueInt(IN IMS_SINT32 nValue) = 0;

    /**
     * @brief Returns the string representation of the header according to header type.
     *
     * For example,
     *     - From: User1 <sip:user1@ims.com>;tag=192168200201
     *     - WWW-Authenticate: Digest realm="ims.com", domain="sip:ims.com", auth="auth",
     *       nonce="f84f1cec41e6cbe5aea9c8e88d359", opaque ="", stale=FALSE, algorithm=MD5
     *
     * The value part of the header may be missing if the header was created with empty string
     * or null as strValue and has not been set using SetValue().
     *
     * @return String representation of this SipHeader.
     */
    virtual AString ToString() const = 0;

    /**
     * @brief Returns the string representation of the header according to header type
     *        without header name.
     *
     * For example,
     *     - From: User1 <sip:user1@ims.com>;tag=192168200201
     *       => User1 <sip:user1@ims.com>;tag=192168200201
     *     - WWW-Authenticate: Digest realm="ims.com", domain="sip:ims.com", auth="auth",
     *       nonce="f84f1cec41e6cbe5aea9c8e88d359", opaque ="", stale=FALSE, algorithm=MD5
     *       => Digest realm="ims.com", domain="sip:ims.com", auth="auth",
     *       nonce="f84f1cec41e6cbe5aea9c8e88d359", opaque ="", stale=FALSE, algorithm=MD5
     *
     * The value part of the header may be missing if the header was created with empty string
     * or null as strValue and has not been set using SetValue().
     *
     * @return String representation of this SipHeader without header name.
     */
    virtual AString ToStringWithoutName() const = 0;

public:
    enum
    {
        INVALID_INT = -1
    };

    /// Type of SIP header (defined headers)
    enum
    {
        INVALID = (-1),
        ALLOW = 0,
        ALLOW_EVENTS,
        AUTHORIZATION,
        CALL_ID,
        CONTACT_NORMAL,
        CONTACT_WILDCARD,
        CONTACT_ANY,
        CONTENT_DISPOSITION,
        CONTENT_ENCODING,
        CONTENT_LENGTH,
        CONTENT_TYPE,
        CSEQ,
        EVENT,
        EXPIRES_DATE,
        EXPIRES_SEC,
        EXPIRES_ANY,
        ACCEPT,
        MIN_EXPIRES,
        FROM,
        MAX_FORWARDS,
        MIME_VERSION,
        PRIVACY,
        P_PREFERRED_IDENTITY,
        P_ASSERTED_IDENTITY,
        MIN_SE,
        PATH,
        P_ASSOCIATED_URI,
        P_CALLED_PARTY_ID,
        P_VISITED_NETWORK_ID,
        P_CHARGING_FUNCTION_ADDRESSES,
        P_ACCESS_NETWORK_INFO,
        P_CHARGING_VECTOR,
        SERVICE_ROUTE,
        HISTORY_INFO,
        REQUEST_DISPOSITION,
        ACCEPT_CONTACT,
        REJECT_CONTACT,
        JOIN,
        SIP_IF_MATCH,
        SIP_ETAG,
        PROXY_AUTHENTICATE,
        PROXY_AUTHORIZATION,
        RACK,
        RECORD_ROUTE,
        REFERRED_BY,
        REFER_TO,
        REPLACES,
        REQUIRE,
        ROUTE,
        RSEQ,
        SECURITY_CLIENT,
        SECURITY_VERIFY,
        SECURITY_SERVER,
        SESSION_EXPIRES,
        SUBSCRIPTION_STATE,
        SUPPORTED,
        TIMESTAMP,
        TO,
        UNSUPPORTED,
        VIA,
        WARNING,
        WWW_AUTHENTICATE,
        UNKNOWN,
        RETRY_AFTER_DATE,
        RETRY_AFTER_SEC,
        RETRY_AFTER_ANY,
        P_EARLY_MEDIA,
        RESOURCE_PRIORITY,
        ACCEPT_RESOURCE_PRIORITY,
        DATE,
        ACCEPT_ENCODING,
        ACCEPT_LANGUAGE,
        ALERT_INFO,
        ANSWER_MODE,
        AUTHENTICATION_INFO,
        CALL_INFO,
        CONTENT_LANGUAGE,
        ERROR_INFO,
        FLOW_TIMER,
        IDENTITY,
        IDENTITY_INFO,
        IN_REPLY_TO,
        ORGANIZATION,
        P_ANSWER_STATE,
        PERMISSION_MISSING,
        P_MEDIA_AUTHORIZATION,
        P_PROFILE_KEY,
        P_REFUSED_URI_LIST,
        PRIORITY,
        PRIV_ANSWER_MODE,
        PROXY_REQUIRE,
        P_SERVED_USER,
        P_USER_DATABASE,
        REASON,
        REFER_SUB,
        REPLY_TO,
        RESPONSE_KEY,
        SERVER,
        SUBJECT,
        SUPPRESS_IF_MATCH,
        TARGET_DIALOG,
        TRIGGER_CONSENT,
        USER_AGENT,
        FEATURE_CAPS,
        GEOLOCATION,
        GEOLOCATION_ERROR,
        GEOLOCATION_ROUTING,
        INFO_PACKAGE,
        MAX_BREADTH,
        P_ASSERTED_SERVICE,
        POLICY_CONTACT,
        POLICY_ID,
        P_PREFERRED_SERVICE,
        RECV_INFO,
        SESSION_ID,
        ANY
    };
};

#endif
