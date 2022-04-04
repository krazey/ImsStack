/*
    Author
    <table>
    date      author                description
    --------  --------------        ----------
    01/01/2015  vijay.nair@               Created
    </table>

    Description

*/

/*****************************************************************************
  Header Inclusions
*****************************************************************************/
#include "msg/SipHeaderBase.h"
#include "msg/SipRequestLine.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "msg/sip_msgutil.h"

SIP_BOOL gHeaderAttributes[SipHeaderBase::TYPE_END][SipHeaderBase::HEADER_ATTRIBUTE_END] = {
    {SIP_TRUE},//ALLOW
    {SIP_FALSE},//ALLOW_EVENTS
    {SIP_FALSE},//AUTHORIZATION
    {SIP_FALSE},//CALL_ID
    {SIP_FALSE},//CONTACT
    {SIP_FALSE},//CONTACT_WILD
    {SIP_FALSE},//CONTACT_ANY
    {SIP_FALSE},//CONTENT_DISPOSITION
    {SIP_FALSE},//CONTENT_ENCODING
    {SIP_FALSE},//CONTENT_LENGTH
    {SIP_FALSE},//CONTENT_TYPE
    {SIP_FALSE},//CSEQ
    {SIP_FALSE},//EVENT
    {SIP_FALSE},//EXPIRES_DATE
    {SIP_FALSE},//EXPIRES_SEC
    {SIP_FALSE},//EXPIRES_ANY
    {SIP_TRUE},//ACCEPT
    {SIP_FALSE},//MIN_EXPIRES
    {SIP_FALSE},//FROM
    {SIP_FALSE},//MAX_FORWARDS
    {SIP_FALSE},//MIME_VERSION
    {SIP_FALSE},//PRIVACY
    {SIP_FALSE},//P_PREFERRED_IDENTITY
    {SIP_FALSE},//P_ASSERTED_IDENTITY
    {SIP_FALSE},//MIN_SE
    {SIP_FALSE},//PATH
    {SIP_TRUE},//P_ASSOCIATED_URI
    {SIP_FALSE},//P_CALLED_PARTY_ID
    {SIP_FALSE},//P_VISITED_NETWORK_ID
    {SIP_FALSE},//P_CHRG_FUN_ADDR
    {SIP_FALSE},//P_ACCESS_NETWORK_INFO
    {SIP_FALSE},//P_CHARGING_VECTOR
    {SIP_FALSE},//SERVICE_ROUTE
    {SIP_FALSE},//HISTORY_INFO
    {SIP_FALSE},//REQUEST_DISPOSITION
    {SIP_FALSE},//ACCEPT_CONTACT
    {SIP_FALSE},//REJECT_CONTACT
    {SIP_FALSE},//JOIN
    {SIP_FALSE},//SIP_IF_MATCH
    {SIP_FALSE},//SIP_ETAG
    {SIP_FALSE},//PROXY_AUTHENTICATE
    {SIP_FALSE},//PROXY_AUTHORIZATION
    {SIP_FALSE},//RACK
    {SIP_FALSE},//RECORD_ROUTE
    {SIP_FALSE},//REFERRED_BY
    {SIP_FALSE},//REFER_TO
    {SIP_FALSE},//REPLACES
    {SIP_FALSE},//REQUIRE
    {SIP_FALSE},//ROUTE
    {SIP_FALSE},//RSEQ
    {SIP_FALSE},//SECURITY_CLIENT
    {SIP_FALSE},//SECURITY_VERIFY
    {SIP_FALSE},//SECURITY_SERVER
    {SIP_FALSE},//SESSION_EXPIRES
    {SIP_FALSE},//SUBSCRIPTION_STATE
    {SIP_TRUE},//SUPPORTED
    {SIP_FALSE},//TIMESTAMP
    {SIP_FALSE},//TO
    {SIP_FALSE},//UNSUPPORTED
    {SIP_FALSE},//VIA
    {SIP_FALSE},//WARNING
    {SIP_FALSE},//WWW_AUTHENTICATE
    {SIP_TRUE},//UNKNOWN
    {SIP_FALSE},//RETRY_AFTER_DATE
    {SIP_FALSE},//RETRY_AFTER_SEC
    {SIP_FALSE},//RETRY_AFTER_ANY
    {SIP_TRUE},//P_EARLY_MEDIA
    {SIP_FALSE},//RESOURCE_PRIORITY
    {SIP_TRUE},//ACCEPT_RESOURCE_PRIORITY
    {SIP_FALSE},//DATE
    {SIP_TRUE},//ACCEPT_ENCODING
    {SIP_TRUE},//ACCEPT_LANGUAGE
    {SIP_FALSE},//ALERT_INFO
    {SIP_FALSE},//ANSWER_MODE
    {SIP_FALSE},//AUTHENTICATION_INFO
    {SIP_FALSE},//CALL_INFO
    {SIP_FALSE},//CONTENT_LANGUAGE
    {SIP_FALSE},//ERROR_INFO
    {SIP_FALSE},//FLOW_TIMER
    {SIP_FALSE},//IDENTITY
    {SIP_FALSE},//IDENTITY_INFO
    {SIP_FALSE},//IN_REPLY_TO
    {SIP_TRUE},//ORGANIZATION
    {SIP_FALSE},//P_ANSWER_STATE
    {SIP_FALSE},//PERMISSION_MISSING
    {SIP_FALSE},//P_MEDIA_AUTHORIZATION
    {SIP_FALSE},//P_PROFILE_KEY
    {SIP_FALSE},//P_REFUSED_URI_LIST
    {SIP_FALSE},//PRIORITY
    {SIP_FALSE},//PRIV_ANSWER_MODE
    {SIP_FALSE},//PROXY_REQUIRE
    {SIP_FALSE},//P_SERVED_USER
    {SIP_FALSE},//P_USER_DATABASE
    {SIP_FALSE},//REASON
    {SIP_FALSE},//REFER_SUB
    {SIP_FALSE},//REPLY_TO
    {SIP_FALSE},//RESPONSE_KEY
    {SIP_FALSE},//SERVER
    {SIP_TRUE},//SUBJECT
    {SIP_FALSE},//SUPPRESS_IF_MATCH
    {SIP_FALSE},//TARGET_DIALOG
    {SIP_FALSE},//TRIGGER_CONSENT
    {SIP_FALSE},//USER_AGENT
    {SIP_FALSE},//FEATURE_CAPS
    {SIP_FALSE},//GEOLOCATION
    {SIP_FALSE},//GEOLOCATION_ERROR
    {SIP_FALSE},//GEOLOCATION_ROUTING
    {SIP_FALSE},//INFO_PACKAGE
    {SIP_FALSE},//MAX_BREADTH
    {SIP_FALSE},//P_ASSERTED_SERVICE
    {SIP_FALSE},//POLICY_CONTACT
    {SIP_FALSE},//POLICY_ID
    {SIP_FALSE},//P_PREFERRED_SERVICE
    {SIP_TRUE},//RECV_INFO
    {SIP_FALSE},//SESSION_ID
};

/****************************************************************************
SipHeaderBase - Class Member Function Implementations
*****************************************************************************/
SipHeaderBase::SipHeaderBase(SIP_INT32 eHdrType)
    : m_eHdrType(eHdrType)
    , m_pszValue(SIP_NULL)
    , m_pParameters(SIP_NULL)
{
}

SipHeaderBase::SipHeaderBase(const SipHeaderBase& objHeader)
    : m_eHdrType(objHeader.m_eHdrType)
    , m_pszValue(SipPf_Strdup(objHeader.m_pszValue))
    , m_pParameters(SIP_NULL)
{
    if (objHeader.m_pParameters != SIP_NULL)
    {
        InitParameters(objHeader.m_pParameters);
    }
}

SipHeaderBase::~SipHeaderBase()
{
    if (m_pParameters != SIP_NULL)
    {
        delete m_pParameters;
    }
    if (m_pszValue != SIP_NULL)
    {
        delete[] m_pszValue;
    }
}

SIP_VOID SipHeaderBase::InitParameters(SipParameters* pParameters)
{
    if (this->m_pParameters != SIP_NULL)
    {
        delete this->m_pParameters;
    }

    if (pParameters != SIP_NULL)
    {
        this->m_pParameters = new SipParameters(*pParameters);
    }
    else
    {
        this->m_pParameters = new SipParameters();
    }
}

SipParameters* SipHeaderBase::GetParameters() const
{
    return m_pParameters;
}

SIP_BOOL SipHeaderBase::FindComment(SIP_CHAR* pszStart, SIP_CHAR* pszEnd,
        SIP_CHAR*& pszCommentStart, SIP_CHAR*& pszCommentEnd)
{
    SIP_CHAR* pszCurrent = pszStart;
    SIP_INT32 nCount = 0;

    while (pszCurrent <= pszEnd)
    {
        if (*pszCurrent == LPARAN)
        {
            if (nCount == 0)
            {
                pszCommentStart = pszCurrent;
            }
            nCount++;
        }

        if (*pszCurrent == RPARAN)
        {
            nCount--;

            if (nCount == 0)
            {
                pszCommentEnd = pszCurrent;
                return SIP_TRUE;
            }
        }
        pszCurrent++;
    }

    return SIP_FALSE;
}

SIP_BOOL SipHeaderBase::IsValidHeader() const
{
    if (m_pszValue != SIP_NULL)
    {
        return SIP_TRUE;
    }
    return gHeaderAttributes[m_eHdrType][HEADER_EMPTY_BODY_ALLOWED];
}

SIP_BOOL SipHeaderBase::SetValue(const SIP_CHAR* pszValue)
{
    return SetCharVar(pszValue, m_pszValue);
}

const SIP_CHAR* SipHeaderBase::GetValue() const
{
    return m_pszValue;
}

SIP_BOOL SipHeaderBase::EncodeHeaderParameters(SIP_CHAR** ppCurrPos,
        SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    SipParameterList* pParameterList =
            (m_pParameters != SIP_NULL) ? m_pParameters->GetParameterList() : SIP_NULL;

    if ((bParams == SIP_TRUE) && (pParameterList != SIP_NULL))
    {
        return pParameterList->EncodeList(ppCurrPos,SIP_SEMI);
    }
    return SIP_TRUE;
}

SIP_BOOL SipHeaderBase::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No header Value to encode", SIP_ZERO, SIP_ZERO);
        return gHeaderAttributes[m_eHdrType][HEADER_EMPTY_BODY_ALLOWED];
    }

    SipPf_Strcpy(*ppCurrPos, m_pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipHeaderBase::DecodeHeaderParameters(SIP_CHAR* pStart,
        SIP_CHAR* pEnd, SIP_CHAR cDelimeter)
{
    if (m_pParameters == SIP_NULL)
    {
        InitParameters(SIP_NULL);
    }

    if (m_pParameters == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipParameterList* pParameterList = m_pParameters->GetParameterList();

    if (pParameterList->DecHdrSipParameterList(pStart, pEnd, cDelimeter) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaderBase::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "zero length buffer", SIP_ZERO, SIP_ZERO);
        return gHeaderAttributes[m_eHdrType][HEADER_EMPTY_BODY_ALLOWED];
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;
    /*First Check the presence of params i.e. ";" and decode if present*/
    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        pEndPt = pTempPre;
    }
    /*Now Decode the Value*/
    SIP_CHAR* pszValue = sipCreateString(pStartPt, pEndPt);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;
    return SIP_TRUE;
}

SipHeaderBase* SipHeaderBase::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipHeaderBase(*pHeader);
    }
    return new SipHeaderBase(eHeaderType);
}

/****************************************************************************
  SipNameAddrHeader - Class Member Function Implementations
*****************************************************************************/
SipNameAddrHeader::SipNameAddrHeader(SIP_INT32 eHdrType)
    : SipHeaderBase(eHdrType)
    , m_pNameAddr(SIP_NULL)
{
    m_pNameAddr = new SipNameAddr();
}

SipNameAddrHeader::SipNameAddrHeader(const SipNameAddrHeader& objSipNameAddrHeader)
    : SipHeaderBase(objSipNameAddrHeader)
    , m_pNameAddr(SIP_NULL)
{
    if (objSipNameAddrHeader.m_pNameAddr != SIP_NULL)
    {
        m_pNameAddr = new SipNameAddr(*(objSipNameAddrHeader.m_pNameAddr));
        m_pNameAddr->SetParameterComponent(this);
    }
}

SipNameAddrHeader::~SipNameAddrHeader()
{
    if (m_pNameAddr != SIP_NULL)
    {
        m_pNameAddr->SetParameterComponent(SIP_NULL);
        m_pNameAddr->SipDelete();
    }
}

SIP_BOOL SipNameAddrHeader::SetAddrSpec(SipAddrSpec* pAddrSpec)
{
    if (m_pNameAddr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    if (m_pNameAddr->m_pAddrSpec != SIP_NULL)
    {
        (m_pNameAddr->m_pAddrSpec)->SipDelete();
    }

    m_pNameAddr->m_pAddrSpec  = pAddrSpec;
    if (pAddrSpec)
    {
        pAddrSpec->increment();
    }

    return SIP_TRUE;
}

SipNameAddr* SipNameAddrHeader::GetNameAddr()
{
    if (m_pNameAddr != SIP_NULL)
    {
        m_pNameAddr->increment();
        return m_pNameAddr;
    }
    return SIP_NULL;
}

SIP_BOOL  SipNameAddrHeader::IsValidComponent(const SIP_CHAR* pszComponent) const
{
    SIP_INT32 eHdrType = GetHdrType();
    if ((eHdrType == SipHeaderBase::TO) || (eHdrType == SipHeaderBase::FROM) ||
        (eHdrType == SipHeaderBase::CONTACT) || (eHdrType == SipHeaderBase::RECORD_ROUTE) ||
        (eHdrType == SipHeaderBase::ROUTE))
    {
        if (SipPf_Stricmp(pszComponent, SIP_USER) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_PASSWORD) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_HOST) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_PORT) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_USER_PRM) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_METHOD) == 0)
        {
            return SIP_FALSE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_MADDR_PRM) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_TTL_PRM) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_TRNSPORT_PRM) == 0)
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_LR_PRM) == 0)
        {
            return SIP_TRUE;
        }
        else if ((eHdrType == SipHeaderBase::CONTACT) &&
            (SipPf_Stricmp(pszComponent, SIP_OTHER_PRM) == 0))
        {
            return SIP_TRUE;
        }
        else if (SipPf_Stricmp(pszComponent, SIP_HEADERS) == 0)
        {
            return SIP_TRUE;
        }
    }
    return SIP_FALSE;
}


SIP_BOOL SipNameAddrHeader::IsPercentEncHdr() const
{
    SIP_INT32 eHdrType = GetHdrType();
    if ((eHdrType == SipHeaderBase::TO) || (eHdrType == SipHeaderBase::FROM) ||
        (eHdrType == SipHeaderBase::ROUTE) || (eHdrType == SipHeaderBase::RECORD_ROUTE) ||
        (eHdrType == SipHeaderBase::CONTACT) || (eHdrType == SipHeaderBase::HISTORY_INFO) ||
        (eHdrType == SipHeaderBase::TRIGGER_CONSENT))
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipNameAddrHeader::SetNameAddr(SipNameAddr* pSipNameAddr)
{
    if (pSipNameAddr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    if (m_pNameAddr != SIP_NULL)
    {
        m_pNameAddr->SipDelete();
    }

    pSipNameAddr->increment();
    m_pNameAddr = pSipNameAddr;
    return SIP_TRUE;
}

SIP_BOOL SipNameAddrHeader::EncodeHdr(SIP_CHAR** ppCurrPos,
        SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pNameAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "name address not present", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pNameAddr->SetParameterComponent(this);
    if (m_pNameAddr->EncodeNameAddr(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode name address fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipNameAddrHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, RIGHT_ANGLE) == SIP_TRUE)
    {
        if (m_pNameAddr->DecodeNameAddr(pStartPt, pTempPre) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        pStartPt = pTempNext;
        pTempNext = SIP_NULL;
        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempNext, SIP_SEMI) ==  SIP_TRUE)
        {
            pTempNext = pTempNext + SIP_TWO;
        }
    }
    else
    {
        SIP_INT32 nLen = nDecLen;
        if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
        {
            nLen = pTempPre - pStartPt + SIP_ONE;
        }

#ifdef SIP_STRICT_PARSING
        if ((GetHdrType() == SipHeaderBase::CONTACT) &&
            (IsValidAddress(pStartPt, nLen) == SIP_FALSE))
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Address Spec", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
#endif
        SipAddrSpec* pAddrSpec = new SipAddrSpec();
        if (pAddrSpec == SIP_NULL)
        {
            return SIP_FALSE;
        }

        if (IsPercentEncHdr() == SIP_TRUE)
        {
            pAddrSpec->SetParameterComponent(static_cast<IParameterComponent*>(this));
        }

        if (pAddrSpec->DecodeAddrSpec(pStartPt, nLen) == SIP_FALSE)
        {
            pAddrSpec->SipDelete();
            return SIP_FALSE;
        }
        m_pNameAddr->m_pAddrSpec = pAddrSpec;
    }

    return (pTempNext != SIP_NULL) ?
            DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) : SIP_TRUE;
}

SipHeaderBase* SipNameAddrHeader::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipNameAddrHeader(*reinterpret_cast<SipNameAddrHeader*>(pHeader));
    }
    return new SipNameAddrHeader(eHdr);
}
