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
#ifndef __SIP_HEADER_BASE_H__
#define __SIP_HEADER_BASE_H__

#include <assert.h>

#include "SipDatatypes.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipParameters.h"

class IParameterComponent;

class SipHeaderBase : public SipRefBase
{
public:
    enum
    {
        TYPE_INVALID = -1,
        ALLOW,
        ALLOW_EVENTS,
        AUTHORIZATION,
        CALL_ID,
        CONTACT,
        CONTACT_WILD,
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
        P_CHRG_FUN_ADDR,
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
        TYPE_END
    };

    enum  // eHeaderAttributes
    {
        HEADER_EMPTY_BODY_ALLOWED = 0,  // Empty header value allowed or not
        HEADER_MULTI_VALUE_ALLOWED,     // Header will have more than one value, e.g., via
        HEADER_ATTRIBUTE_END
    };

private:
    SIP_INT32 m_eHdrType;
    SIP_CHAR* m_pszValue;
    SipParameters* m_pParameters;

public:
    explicit SipHeaderBase(SIP_INT32 eHdrType);
    SipHeaderBase(const SipHeaderBase& objHeader);
    SIP_VOID InitParameters(SipParameters* pParameters);
    SIP_BOOL EncodeHeaderParameters(SIP_CHAR** ppMsgBuffCurrPos, SIP_BOOL bParams = SIP_TRUE);
    SIP_BOOL EncodeParameters(AStringBuffer& objBuffer) const;
    virtual SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const;
    virtual SIP_BOOL EncodeHdr(SIP_CHAR** ppMsgBuffCurrPos, SIP_BOOL bParams = SIP_TRUE);
    virtual SIP_BOOL EncodeHdr(
            SIP_CHAR** ppMsgBuffCurrPos, SIP_BOOL bParams, SIP_UINT32 nMsgOptions)
    {
        (void)nMsgOptions;
        return EncodeHdr(ppMsgBuffCurrPos, bParams);
    }
    SIP_BOOL DecodeHeaderParameters(SIP_CHAR* pStart, SIP_CHAR* pEnd, SIP_CHAR cDelimeter);
    virtual SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
    inline SIP_INT32 GetHdrType() const { return m_eHdrType; }
    inline SIP_VOID SetHdrType(SIP_INT32 eHdrType) { m_eHdrType = eHdrType; }
    SipParameters* GetParameters() const;
    virtual SIP_BOOL IsValidHeader() const;
    virtual SIP_BOOL SetValue(const SIP_CHAR* pszValue);
    virtual const SIP_CHAR* GetValue() const;
    static SIP_BOOL IsHeaderTypeValid(SIP_INT32 eHdrType);
    static SIP_BOOL IsMultiValueHeader(SIP_INT32 eHdrType);
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

protected:
    virtual ~SipHeaderBase();
    static SIP_BOOL FindComment(SIP_CHAR* pszStart, const SIP_CHAR* pszEnd,
            SIP_CHAR*& pszCommentStart, SIP_CHAR*& pszCommentEnd);
};

class SipNameAddrHeader : public SipHeaderBase
{
protected:
    SipNameAddr* m_pNameAddr;

public:
    explicit SipNameAddrHeader(SIP_INT32 eHdrType);
    SipNameAddrHeader(const SipNameAddrHeader& objSipNameAddrHeader);

    SIP_BOOL SetAddrSpec(SipAddrSpec* pAddrSpec);
    SipNameAddr* GetNameAddr();
    SIP_CHAR* GetTag();
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    virtual SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;
    virtual SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    inline SIP_BOOL IsValidHeader() const override
    {
        return (m_pNameAddr == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
    }
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

private:
    virtual ~SipNameAddrHeader();
};
#endif  //__SIP_HEADER_BASE_H__
