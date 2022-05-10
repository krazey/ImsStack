#ifndef __SIP_HEADER_BASE_H__
#define __SIP_HEADER_BASE_H__

#include <assert.h>

#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "msg/SipParameterList.h"
#include "msg/SipAddrSpec.h"
#include "SipPercentEncoding.h"
#include "msg/IParameterComponent.h"

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
        HEADER_ATTRIBUTE_END
    };

private:
    int m_eHdrType;
    SIP_CHAR* m_pszValue;
    SipParameters* m_pParameters;

public:
    SipHeaderBase(SIP_INT32 eHdrType);
    SipHeaderBase(const SipHeaderBase& objHeader);
    virtual ~SipHeaderBase();
    SIP_VOID InitParameters(SipParameters* pParameters);
    SIP_BOOL EncodeHeaderParameters(SIP_CHAR** ppMsgBuffCurrPos, SIP_BOOL bParams = SIP_TRUE);
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
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

protected:
    static SIP_BOOL FindComment(SIP_CHAR* pszStart, SIP_CHAR* pszEnd, SIP_CHAR*& pszCommentStart,
            SIP_CHAR*& pszCommentEnd);
};

class SipNameAddrHeader : public SipHeaderBase, public IParameterComponent
{
protected:
    SipNameAddr* m_pNameAddr;

public:
    SipNameAddrHeader(SIP_INT32 eHdrType);
    SipNameAddrHeader(const SipNameAddrHeader& objSipNameAddrHeader);
    virtual ~SipNameAddrHeader();
    virtual SIP_BOOL IsValidComponent(const SIP_CHAR* pszComponent) const;
    SIP_BOOL SetNameAddr(SipNameAddr* pSipNameAddr);
    SIP_BOOL SetAddrSpec(SipAddrSpec* pAddrSpec);
    SipNameAddr* GetNameAddr();
    virtual SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);
    virtual SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
    SIP_BOOL IsPercentEncHdr() const;
    inline SIP_BOOL IsValidHeader() const
    {
        return (m_pNameAddr == SIP_NULL) ? SIP_FALSE : SIP_TRUE;
    }
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);
};
#endif  //__SIP_HEADER_BASE_H__
