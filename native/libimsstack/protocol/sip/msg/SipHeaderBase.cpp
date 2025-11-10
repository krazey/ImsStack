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
#include "SipDebug.h"
#include "msg/SipHeaderBase.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SIP_BOOL gHeaderAttributes[SipHeaderBase::TYPE_END][SipHeaderBase::HEADER_ATTRIBUTE_END] = {
        {SIP_TRUE,  SIP_TRUE }, // ALLOW
        {SIP_FALSE, SIP_TRUE }, // ALLOW_EVENTS
        {SIP_FALSE, SIP_FALSE}, // AUTHORIZATION
        {SIP_FALSE, SIP_FALSE}, // CALL_ID
        {SIP_FALSE, SIP_TRUE }, // CONTACT
        {SIP_FALSE, SIP_TRUE }, // CONTACT_WILD
        {SIP_FALSE, SIP_TRUE }, // CONTACT_ANY
        {SIP_FALSE, SIP_FALSE}, // CONTENT_DISPOSITION
        {SIP_FALSE, SIP_TRUE }, // CONTENT_ENCODING
        {SIP_FALSE, SIP_FALSE}, // CONTENT_LENGTH
        {SIP_FALSE, SIP_FALSE}, // CONTENT_TYPE
        {SIP_FALSE, SIP_FALSE}, // CSEQ
        {SIP_FALSE, SIP_FALSE}, // EVENT
        {SIP_FALSE, SIP_FALSE}, // EXPIRES_DATE
        {SIP_FALSE, SIP_FALSE}, // EXPIRES_SEC
        {SIP_FALSE, SIP_FALSE}, // EXPIRES_ANY
        {SIP_TRUE,  SIP_TRUE }, // ACCEPT
        {SIP_FALSE, SIP_FALSE}, // MIN_EXPIRES
        {SIP_FALSE, SIP_FALSE}, // FROM
        {SIP_FALSE, SIP_FALSE}, // MAX_FORWARDS
        {SIP_FALSE, SIP_FALSE}, // MIME_VERSION
        {SIP_FALSE, SIP_FALSE}, // PRIVACY
        {SIP_FALSE, SIP_TRUE }, // P_PREFERRED_IDENTITY
        {SIP_FALSE, SIP_TRUE }, // P_ASSERTED_IDENTITY
        {SIP_FALSE, SIP_FALSE}, // MIN_SE
        {SIP_FALSE, SIP_TRUE }, // PATH
        {SIP_TRUE,  SIP_TRUE }, // P_ASSOCIATED_URI
        {SIP_FALSE, SIP_FALSE}, // P_CALLED_PARTY_ID
        {SIP_FALSE, SIP_TRUE }, // P_VISITED_NETWORK_ID
        {SIP_FALSE, SIP_TRUE }, // P_CHRG_FUN_ADDR
        {SIP_FALSE, SIP_TRUE }, // P_ACCESS_NETWORK_INFO
        {SIP_FALSE, SIP_FALSE}, // P_CHARGING_VECTOR
        {SIP_FALSE, SIP_TRUE }, // SERVICE_ROUTE
        {SIP_FALSE, SIP_TRUE }, // HISTORY_INFO
        {SIP_FALSE, SIP_TRUE }, // REQUEST_DISPOSITION
        {SIP_FALSE, SIP_TRUE }, // ACCEPT_CONTACT
        {SIP_FALSE, SIP_TRUE }, // REJECT_CONTACT
        {SIP_FALSE, SIP_FALSE}, // JOIN
        {SIP_FALSE, SIP_FALSE}, // SIP_IF_MATCH
        {SIP_FALSE, SIP_FALSE}, // SIP_ETAG
        {SIP_FALSE, SIP_FALSE}, // PROXY_AUTHENTICATE
        {SIP_FALSE, SIP_FALSE}, // PROXY_AUTHORIZATION
        {SIP_FALSE, SIP_FALSE}, // RACK
        {SIP_FALSE, SIP_TRUE }, // RECORD_ROUTE
        {SIP_FALSE, SIP_FALSE}, // REFERRED_BY
        {SIP_FALSE, SIP_FALSE}, // REFER_TO
        {SIP_FALSE, SIP_FALSE}, // REPLACES
        {SIP_FALSE, SIP_TRUE }, // REQUIRE
        {SIP_FALSE, SIP_TRUE }, // ROUTE
        {SIP_FALSE, SIP_FALSE}, // RSEQ
        {SIP_FALSE, SIP_TRUE }, // SECURITY_CLIENT
        {SIP_FALSE, SIP_TRUE }, // SECURITY_VERIFY
        {SIP_FALSE, SIP_TRUE }, // SECURITY_SERVER
        {SIP_FALSE, SIP_FALSE}, // SESSION_EXPIRES
        {SIP_FALSE, SIP_FALSE}, // SUBSCRIPTION_STATE
        {SIP_TRUE,  SIP_TRUE }, // SUPPORTED
        {SIP_FALSE, SIP_FALSE}, // TIMESTAMP
        {SIP_FALSE, SIP_FALSE}, // TO
        {SIP_FALSE, SIP_TRUE }, // UNSUPPORTED
        {SIP_FALSE, SIP_TRUE }, // VIA
        {SIP_FALSE, SIP_TRUE }, // WARNING
        {SIP_FALSE, SIP_FALSE}, // WWW_AUTHENTICATE
        {SIP_TRUE,  SIP_TRUE }, // UNKNOWN
        {SIP_FALSE, SIP_FALSE}, // RETRY_AFTER_DATE
        {SIP_FALSE, SIP_FALSE}, // RETRY_AFTER_SEC
        {SIP_FALSE, SIP_FALSE}, // RETRY_AFTER_ANY
        {SIP_TRUE,  SIP_TRUE }, // P_EARLY_MEDIA
        {SIP_FALSE, SIP_TRUE }, // RESOURCE_PRIORITY
        {SIP_TRUE,  SIP_TRUE }, // ACCEPT_RESOURCE_PRIORITY
        {SIP_FALSE, SIP_FALSE}, // DATE
        {SIP_TRUE,  SIP_TRUE }, // ACCEPT_ENCODING
        {SIP_TRUE,  SIP_TRUE }, // ACCEPT_LANGUAGE
        {SIP_FALSE, SIP_TRUE }, // ALERT_INFO
        {SIP_FALSE, SIP_FALSE}, // ANSWER_MODE
        {SIP_FALSE, SIP_TRUE }, // AUTHENTICATION_INFO
        {SIP_FALSE, SIP_TRUE }, // CALL_INFO
        {SIP_FALSE, SIP_TRUE }, // CONTENT_LANGUAGE
        {SIP_FALSE, SIP_TRUE }, // ERROR_INFO
        {SIP_FALSE, SIP_FALSE}, // FLOW_TIMER
        {SIP_FALSE, SIP_FALSE}, // IDENTITY
        {SIP_FALSE, SIP_FALSE}, // IDENTITY_INFO
        {SIP_FALSE, SIP_TRUE }, // IN_REPLY_TO
        {SIP_TRUE,  SIP_FALSE}, // ORGANIZATION
        {SIP_FALSE, SIP_FALSE}, // P_ANSWER_STATE
        {SIP_FALSE, SIP_TRUE }, // PERMISSION_MISSING
        {SIP_FALSE, SIP_TRUE }, // P_MEDIA_AUTHORIZATION
        {SIP_FALSE, SIP_FALSE}, // P_PROFILE_KEY
        {SIP_FALSE, SIP_TRUE }, // P_REFUSED_URI_LIST
        {SIP_FALSE, SIP_FALSE}, // PRIORITY
        {SIP_FALSE, SIP_FALSE}, // PRIV_ANSWER_MODE
        {SIP_FALSE, SIP_TRUE }, // PROXY_REQUIRE
        {SIP_FALSE, SIP_FALSE}, // P_SERVED_USER
        {SIP_FALSE, SIP_FALSE}, // P_USER_DATABASE
        {SIP_FALSE, SIP_TRUE }, // REASON
        {SIP_FALSE, SIP_FALSE}, // REFER_SUB
        {SIP_FALSE, SIP_FALSE}, // REPLY_TO
        {SIP_FALSE, SIP_FALSE}, // RESPONSE_KEY
        {SIP_FALSE, SIP_FALSE}, // SERVER
        {SIP_TRUE,  SIP_FALSE}, // SUBJECT
        {SIP_FALSE, SIP_FALSE}, // SUPPRESS_IF_MATCH
        {SIP_FALSE, SIP_FALSE}, // TARGET_DIALOG
        {SIP_FALSE, SIP_TRUE }, // TRIGGER_CONSENT
        {SIP_FALSE, SIP_FALSE}, // USER_AGENT
        {SIP_FALSE, SIP_TRUE }, // FEATURE_CAPS
        {SIP_FALSE, SIP_TRUE }, // GEOLOCATION
        {SIP_FALSE, SIP_FALSE}, // GEOLOCATION_ERROR
        {SIP_FALSE, SIP_FALSE}, // GEOLOCATION_ROUTING
        {SIP_FALSE, SIP_FALSE}, // INFO_PACKAGE
        {SIP_FALSE, SIP_FALSE}, // MAX_BREADTH
        {SIP_FALSE, SIP_TRUE }, // P_ASSERTED_SERVICE
        {SIP_FALSE, SIP_TRUE }, // POLICY_CONTACT
        {SIP_FALSE, SIP_TRUE }, // POLICY_ID
        {SIP_FALSE, SIP_TRUE }, // P_PREFERRED_SERVICE
        {SIP_TRUE,  SIP_TRUE }, // RECV_INFO
        {SIP_FALSE, SIP_FALSE}, // SESSION_ID
};

SipHeaderBase::SipHeaderBase(SIP_INT32 eHdrType) :
        m_eHdrType(eHdrType),
        m_pszValue(SIP_NULL),
        m_pParameters(SIP_NULL)
{
}

SipHeaderBase::SipHeaderBase(const SipHeaderBase& objHeader) :
        m_eHdrType(objHeader.m_eHdrType),
        m_pszValue(SipPf_Strdup(objHeader.m_pszValue)),
        m_pParameters(SIP_NULL)
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

SIP_VOID SipHeaderBase::InitParameters(const SipParameters* pParameters)
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

SIP_BOOL SipHeaderBase::FindComment(const SIP_CHAR* pszStart, const SIP_CHAR* pszEnd,
        const SIP_CHAR*& pszCommentStart, const SIP_CHAR*& pszCommentEnd)
{
    const SIP_CHAR* pszCurrent = pszStart;
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
            if (nCount < 0)
            {
                break;
            }
        }
        pszCurrent++;
    }

    if (nCount != 0)
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaderBase::AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue /*= SIP_NULL*/)
{
    if (m_pParameters == SIP_NULL)
    {
        InitParameters(SIP_NULL);
    }
    return m_pParameters->AddParam(pszName, pszValue);
}

SIP_BOOL SipHeaderBase::SetParam(
        const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos /*= SIP_ZERO*/)
{
    if (m_pParameters == SIP_NULL)
    {
        InitParameters(SIP_NULL);
    }
    return m_pParameters->SetParam(pszName, pszValue, nPos);
}

SIP_BOOL SipHeaderBase::IsValidHeader() const
{
    if (m_pszValue != SIP_NULL)
    {
        return SIP_TRUE;
    }
    return IsEmptyHeaderBodyAllowed();
}

SIP_BOOL SipHeaderBase::SetValue(const SIP_CHAR* pszValue)
{
    if ((m_eHdrType == SipHeaderBase::ACCEPT_CONTACT) ||
            (m_eHdrType == SipHeaderBase::FEATURE_CAPS) ||
            (m_eHdrType == SipHeaderBase::REJECT_CONTACT))
    {
        if ((pszValue != SIP_NULL) && (SipPf_Strcmp(pszValue, "*") != 0))
        {
            return SIP_FALSE;
        }
    }

    SipMsgUtil::SetValue(pszValue, m_pszValue);
    return SIP_TRUE;
}

const SIP_CHAR* SipHeaderBase::GetValue() const
{
    return m_pszValue;
}

SIP_BOOL SipHeaderBase::EncodeHeaderParameters(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if (m_pParameters == SIP_NULL)
    {
        return SIP_TRUE;
    }

    if (bParams == SIP_TRUE)
    {
        return m_pParameters->Encode(ppCurrPos, SIP_SEMI);
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaderBase::EncodeParameters(AStringBuffer& objBuffer) const
{
    if (m_pParameters == SIP_NULL)
    {
        return SIP_TRUE;
    }

    return m_pParameters->Encode(objBuffer, SIP_SEMI);
}

SIP_BOOL SipHeaderBase::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Empty header body", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    objBuffer += m_pszValue;

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipHeaderBase::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*= SIP_TRUE*/)
{
    if (m_pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Empty header body", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    SipAbnfUtil::Append(*ppCurrPos, m_pszValue);

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipHeaderBase::DecodeHeaderParameters(
        const SIP_CHAR* pStart, const SIP_CHAR* pEnd, const SIP_CHAR cDelimiter)
{
    if (m_pParameters == SIP_NULL)
    {
        InitParameters(SIP_NULL);
    }

    if (m_pParameters == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pParameters->Decode(pStart, pEnd, cDelimiter) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipHeaderBase::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "zero length buffer", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;
    /*First Check the presence of params i.e. ";" and decode if present*/
    if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, SIP_SEMI) ==
            SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        pEndPt = pTempPre;
    }
    /*Now Decode the Value*/
    SIP_CHAR* pszValue = SipAbnfUtil::CreateString(pStartPt, pEndPt);
    if (pszValue == SIP_NULL)
    {
        return IsEmptyHeaderBodyAllowed();
    }

    if ((m_eHdrType == SipHeaderBase::ACCEPT_CONTACT) ||
            (m_eHdrType == SipHeaderBase::FEATURE_CAPS) ||
            (m_eHdrType == SipHeaderBase::REJECT_CONTACT))
    {
        if (SipPf_Strcmp(pszValue, "*") != 0)
        {
            return SIP_FALSE;
        }
    }

    if (SetValue(pszValue) == SIP_FALSE)
    {
        delete[] pszValue;
        return SIP_FALSE;
    }

    delete[] pszValue;
    return SIP_TRUE;
}

SIP_BOOL SipHeaderBase::IsEmptyHeaderBodyAllowed() const
{
    return IsHeaderTypeValid(m_eHdrType) ? gHeaderAttributes[m_eHdrType][HEADER_EMPTY_BODY_ALLOWED]
                                         : SIP_FALSE;
}

SIP_BOOL SipHeaderBase::IsHeaderTypeValid(SIP_INT32 eHdrType)
{
    return ((eHdrType > TYPE_INVALID) && eHdrType < SipHeaderBase::TYPE_END) ? SIP_TRUE : SIP_FALSE;
}

SIP_BOOL SipHeaderBase::IsMultiValueHeader(SIP_INT32 eHdrType)
{
    return IsHeaderTypeValid(eHdrType) ? gHeaderAttributes[eHdrType][HEADER_MULTI_VALUE_ALLOWED]
                                       : SIP_TRUE;
}

SipHeaderBase* SipHeaderBase::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipHeaderBase(*pHeader);
    }
    return new SipHeaderBase(eHeaderType);
}

SipNameAddrHeader::SipNameAddrHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_pNameAddr(SIP_NULL)
{
    m_pNameAddr = new SipNameAddr();
}

SipNameAddrHeader::SipNameAddrHeader(const SipNameAddrHeader& objSipNameAddrHeader) :
        SipHeaderBase(objSipNameAddrHeader),
        m_pNameAddr(SIP_NULL)
{
    if (objSipNameAddrHeader.m_pNameAddr != SIP_NULL)
    {
        m_pNameAddr = new SipNameAddr(*(objSipNameAddrHeader.m_pNameAddr));
    }
}

SipNameAddrHeader::~SipNameAddrHeader()
{
    if (m_pNameAddr != SIP_NULL)
    {
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

    m_pNameAddr->m_pAddrSpec = pAddrSpec;
    if (pAddrSpec)
    {
        pAddrSpec->Increment();
    }

    return SIP_TRUE;
}

SipNameAddr* SipNameAddrHeader::GetNameAddr()
{
    if (m_pNameAddr != SIP_NULL)
    {
        m_pNameAddr->Increment();
        return m_pNameAddr;
    }
    return SIP_NULL;
}

SIP_CHAR* SipNameAddrHeader::GetTag()
{
    return GetParamValue("tag");
}

SIP_BOOL SipNameAddrHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (m_pNameAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing name-addr", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pNameAddr->Encode(objBuffer, SIP_TRUE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encoding name-addr failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipNameAddrHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*= SIP_TRUE*/)
{
    if (m_pNameAddr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing name-addr", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pNameAddr->Encode(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encoding name-addr failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipNameAddrHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    if (GetHdrType() == SipHeaderBase::CONTACT)
    {
        pEndPt = SipAbnfUtil::SkipWhiteSpaceFromRight(pStartPt, pEndPt);

        if ((pStartPt == pEndPt) && (*pStartPt == ASTERISK))
        {
            if (SetValue("*") == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
            return SIP_TRUE;
        }
    }

    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

    if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, RIGHT_ANGLE) ==
            SIP_TRUE)
    {
        if (m_pNameAddr->Decode(pStartPt, pTempPre) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        pStartPt = pTempNext;
        pTempNext = SIP_NULL;
        if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempNext, SIP_SEMI) == SIP_TRUE)
        {
            pTempNext = pTempNext + SIP_TWO;
        }
    }
    else
    {
        const SIP_CHAR* pTempStart = pStartPt;
        SIP_INT32 nLen = nDecLen;

        if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, ATRATE) ==
                SIP_TRUE)
        {
            pTempStart = pTempNext;
            pTempPre = SIP_NULL;
            pTempNext = SIP_NULL;
        }

        if (SipAbnfUtil::FindActualPosition(pTempStart, pEndPt, pTempPre, pTempNext, SIP_SEMI) ==
                SIP_TRUE)
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

        if (pAddrSpec->Decode(pStartPt, nLen) == SIP_FALSE)
        {
            pAddrSpec->SipDelete();
            return SIP_FALSE;
        }
        m_pNameAddr->m_pAddrSpec = pAddrSpec;
    }

    if (GetHdrType() == SipHeaderBase::P_PREFERRED_IDENTITY ||
            GetHdrType() == SipHeaderBase::P_ASSERTED_IDENTITY)
    {
        // name-addr or addr-spec is only allowed.
        return SIP_TRUE;
    }

    return (pTempNext != SIP_NULL) ? DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) : SIP_TRUE;
}

SipHeaderBase* SipNameAddrHeader::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipNameAddrHeader(*reinterpret_cast<SipNameAddrHeader*>(pHeader));
    }
    return new SipNameAddrHeader(eHdr);
}
