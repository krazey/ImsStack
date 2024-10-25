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
#include "msg/SipContentTypeHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipContentTypeHeader::SipContentTypeHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_pszMType(SIP_NULL),
        m_pszMSubType(SIP_NULL)
{
}

SipContentTypeHeader::SipContentTypeHeader(const SipContentTypeHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszMType(SipPf_Strdup(objHeader.m_pszMType)),
        m_pszMSubType(SipPf_Strdup(objHeader.m_pszMSubType))
{
}

SipContentTypeHeader::~SipContentTypeHeader()
{
    if (m_pszMType != SIP_NULL)
    {
        delete[] m_pszMType;
    }
    if (m_pszMSubType != SIP_NULL)
    {
        delete[] m_pszMSubType;
    }
}

SIP_BOOL SipContentTypeHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if ((m_pszMType == SIP_NULL) && (m_pszMSubType == SIP_NULL))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "No media type and media subtype", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    if ((m_pszMType == SIP_NULL) || (m_pszMSubType == SIP_NULL))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Missing media type or media subtype", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszMType;
    objBuffer += SLASH;
    objBuffer += m_pszMSubType;

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipContentTypeHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    if ((m_pszMType == SIP_NULL) && (m_pszMSubType == SIP_NULL))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "No media type and media subtype", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    if ((m_pszMType == SIP_NULL) || (m_pszMSubType == SIP_NULL))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Missing media type or media subtype", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszMType);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SipMsgUtil::Encode(*ppCurrPos, SLASH);

    SipPf_Strcpy(*ppCurrPos, m_pszMSubType);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_VOID SipContentTypeHeader::SetMediaType(const SIP_CHAR* pszMType)
{
    SipMsgUtil::SetValue(pszMType, m_pszMType);
}

SIP_VOID SipContentTypeHeader::SetSubMediaType(const SIP_CHAR* pszMSubType)
{
    SipMsgUtil::SetValue(pszMSubType, m_pszMSubType);
}

SIP_CHAR* SipContentTypeHeader::GetBoundary()
{
    SIP_CHAR* pszVal = GetParamValue("boundary");
    SIP_CHAR* pszStripDquoteVal = StripDQUOTE(pszVal);

    if (pszStripDquoteVal == SIP_NULL)
    {
        return pszVal;
    }

    delete[] pszVal;
    return pszStripDquoteVal;
}

SIP_BOOL SipContentTypeHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;
    /*Find the SLASH*/
    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SLASH) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SLASH missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszMType = SipCreateString(pStartPt, pTempPre);
    if (m_pszMType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = pTempNext;
    pTempNext = SIP_NULL;
    pTempPre = SIP_NULL;

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_FALSE)
    {
        pTempPre = pEndPt;
    }

    m_pszMSubType = SipCreateString(pStartPt, pTempPre);
    if (m_pszMSubType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Invalid media type or media subtype", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTempNext != SIP_NULL)
    {
        return DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI);
    }
    return SIP_TRUE;
}

SIP_BOOL SipContentTypeHeader::IsValidHeader() const
{
    if ((m_pszMType == SIP_NULL) && (m_pszMSubType == SIP_NULL))
    {
        return IsEmptyHeaderBodyAllowed();
    }

    if ((m_pszMType == SIP_NULL) || (m_pszMSubType == SIP_NULL))
    {
        return SIP_FALSE;
    }

    if ((GetHdrType() == SipHeaderBase::ACCEPT) && (SipPf_Strcmp(m_pszMType, "*") == 0) &&
            (SipPf_Strcmp(m_pszMSubType, "*") != 0))
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipContentTypeHeader::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipContentTypeHeader(*reinterpret_cast<SipContentTypeHeader*>(pHeader));
    }
    return new SipContentTypeHeader(eHeaderType);
}

SIP_CHAR* SipContentTypeHeader::StripDQUOTE(const SIP_CHAR* pszStr)
{
    SIP_INT32 nStrLen = SipPf_Strlen(pszStr);
    if (nStrLen <= SIP_ONE)
    {
        return SIP_NULL;
    }
    const SIP_CHAR* pEndPtr = pszStr + nStrLen - SIP_ONE;
    if (IS_DQUOTE(*pszStr) && IS_DQUOTE(*pEndPtr))
    {
        return SipCreateString(pszStr + SIP_ONE, pEndPtr - SIP_ONE);
    }
    return SIP_NULL;
}
