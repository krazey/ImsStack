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
#include "SipDatatypes.h"
#include "SipDebug.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipMsgUtil.h"
#include "msg/SipRequestLine.h"
#include "platform/SipString.h"

SipRequestLine::SipRequestLine() :
        m_pszMethod(SIP_NULL),
        m_pReqUri(SIP_NULL),
        m_pszSipVersion(SIP_NULL)
{
}

SipRequestLine::SipRequestLine(
        const SIP_CHAR* pszMethod, SipAddrSpec* pReqUri, const SIP_CHAR* /*pszSipVersion*/) :
        m_pszMethod(SipPf_Strdup(pszMethod)),
        m_pReqUri(pReqUri),
        m_pszSipVersion(SipPf_Strdup(SIP_SIPVER))
{
}

SipRequestLine::SipRequestLine(const SipRequestLine& objHeader) :
        m_pszMethod(SipPf_Strdup(objHeader.m_pszMethod)),
        m_pReqUri(SIP_NULL),
        m_pszSipVersion(SipPf_Strdup(objHeader.m_pszSipVersion))
{
    if (objHeader.m_pReqUri != SIP_NULL)
    {
        m_pReqUri = new SipAddrSpec(*(objHeader.m_pReqUri));
    }
}

SipRequestLine::~SipRequestLine()
{
    if (m_pszMethod != SIP_NULL)
    {
        delete[] m_pszMethod;
    }
    if (m_pReqUri != SIP_NULL)
    {
        m_pReqUri->SipDelete();
    }
    if (m_pszSipVersion != SIP_NULL)
    {
        delete[] m_pszSipVersion;
    }
}

SIP_BOOL SipRequestLine::Encode(SIP_CHAR** ppCurrPos)
{
    /*check for existence of Method, request uri and version */
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Method missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (m_pReqUri == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Request Uri missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (m_pszSipVersion == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Sip Version missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipAbnfUtil::Append(*ppCurrPos, m_pszMethod);
    SipMsgUtil::Encode(*ppCurrPos, SPACE);
    m_pReqUri->Encode(ppCurrPos);
    SipMsgUtil::Encode(*ppCurrPos, SPACE);
    SipAbnfUtil::Append(*ppCurrPos, m_pszSipVersion);

    return SIP_TRUE;
}

SIP_VOID SipRequestLine::SetMethod(const SIP_CHAR* pMethod)
{
    SipMsgUtil::SetValue(pMethod, m_pszMethod);
}

SIP_VOID SipRequestLine::SetSipVersion(const SIP_CHAR* pszVer)
{
    SipMsgUtil::SetValue(pszVer, m_pszSipVersion);
}

SIP_VOID SipRequestLine::SetReqUri(SipAddrSpec* pAddrSpec)
{
    if (m_pReqUri != SIP_NULL)
    {
        m_pReqUri->SipDelete();
    }

    if (pAddrSpec)
    {
        m_pReqUri = pAddrSpec;
        m_pReqUri->Increment();
    }
}

SipAddrSpec* SipRequestLine::GetReqUri()
{
    if (m_pReqUri != SIP_NULL)
    {
        m_pReqUri->Increment();
    }

    return m_pReqUri;
}

SIP_BOOL SipRequestLine::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    const SIP_CHAR* pTempLoc = SIP_NULL;
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    /*find first space i.e. end of Method*/
    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Space not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    /*Create a NULL terminated String of Method*/
    m_pszMethod = SipAbnfUtil::CreateString(pStartPt, pTempLoc);
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Set the method Req line member*/
    /*update the start point */
    pStartPt = pTempLoc + SIP_TWO;
    pTempLoc = SIP_NULL;
    /*find Second space i.e. end of Req URI*/
    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Space not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Decode the request URI*/
    SIP_UINT32 nTempLen = pTempLoc - pStartPt + SIP_ONE;
    m_pReqUri = new SipAddrSpec();
    if (m_pReqUri == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Check for validity of Address Spec of Req URI */
#ifdef SIP_STRICT_PARSING
    if (IsValidAddress(pStartPt, nTempLen) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Addr-spec is invalid", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

#endif

    if (m_pReqUri->Decode(pStartPt, nTempLen) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Addr-spec decode failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    /*Take the ptr to the start of  Sip Version*/
    pStartPt = pTempLoc + SIP_TWO;
    pTempLoc = SIP_NULL;
    m_pszSipVersion = SipAbnfUtil::CreateString(pStartPt, pEndPt);
    if (m_pszSipVersion == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
