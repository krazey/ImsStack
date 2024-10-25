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
#include "msg/SipMessage.h"
#include "msg/SipMsgUtil.h"
#include "msg/SipStatusLine.h"
#include "platform/SipString.h"

SipStatusLine::SipStatusLine(const SIP_CHAR* pszStatusCode, const SIP_CHAR* pszReasonPhrase) :
        m_pszSipVersion(SipPf_Strdup(SIP_SIPVER)),
        m_pszStatusCode(SipPf_Strdup(pszStatusCode)),
        m_pszReasonPhrase(SipPf_Strdup(pszReasonPhrase))
{
}

SipStatusLine::SipStatusLine(const SIP_CHAR* pszSipVersion, const SIP_CHAR* pszStatusCode,
        const SIP_CHAR* pszReasonPhrase) :
        m_pszSipVersion(SipPf_Strdup(pszSipVersion)),
        m_pszStatusCode(SipPf_Strdup(pszStatusCode)),
        m_pszReasonPhrase(SipPf_Strdup(pszReasonPhrase))
{
}

SipStatusLine::SipStatusLine(const SipStatusLine& objHeader) :
        m_pszSipVersion(SipPf_Strdup(objHeader.m_pszSipVersion)),
        m_pszStatusCode(SipPf_Strdup(objHeader.m_pszStatusCode)),
        m_pszReasonPhrase(SipPf_Strdup(objHeader.m_pszReasonPhrase))
{
}

SipStatusLine::~SipStatusLine()
{
    if (m_pszSipVersion != SIP_NULL)
    {
        delete[] m_pszSipVersion;
    }
    if (m_pszStatusCode != SIP_NULL)
    {
        delete[] m_pszStatusCode;
    }
    if (m_pszReasonPhrase != SIP_NULL)
    {
        delete[] m_pszReasonPhrase;
    }
}

SIP_BOOL SipStatusLine::EncodeStatusLine(SIP_CHAR** ppCurrPos)
{
    /*check for existence of version, status code and reason phrase*/
    if (m_pszStatusCode == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Status code missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (m_pszReasonPhrase == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Reason Phrase missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (m_pszSipVersion == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Sip Version missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Encode Sip Version*/
    SipPf_Strcpy(*ppCurrPos, m_pszSipVersion);
    /*Update the Msg Buffer's current position*/
    SipEnc_UpdateCurrPos(ppCurrPos);

    /* Put a space */
    SipMsgUtil::Encode(*ppCurrPos, SPACE);

    /*Encode Status Code*/
    SipPf_Strcpy(*ppCurrPos, m_pszStatusCode);
    /*Update the Msg Buffer's current position*/
    SipEnc_UpdateCurrPos(ppCurrPos);

    /* Put a space */
    SipMsgUtil::Encode(*ppCurrPos, SPACE);

    SipPf_Strcpy(*ppCurrPos, m_pszReasonPhrase);
    /*Update the Msg Buffer's current position*/
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_VOID SipStatusLine::SetStatusCode(const SIP_CHAR* pszStatusCode)
{
    SipMsgUtil::SetValue(pszStatusCode, m_pszStatusCode);
}

SIP_VOID SipStatusLine::SetSipVersion(const SIP_CHAR* pszVer)
{
    SipMsgUtil::SetValue(pszVer, m_pszSipVersion);
}

SIP_VOID SipStatusLine::SetReasonPhrase(const SIP_CHAR* pszReasonPhrase)
{
    SipMsgUtil::SetValue(pszReasonPhrase, m_pszReasonPhrase);
}

SIP_BOOL SipStatusLine::GetStatusCode(SIP_INT16* pnStatusCode) const
{
    if (m_pszStatusCode != SIP_NULL)
    {
        *pnStatusCode = SipPf_Atoi(m_pszStatusCode);
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_UINT16 SipStatusLine::GetStatusCodeAsInt() const
{
    return (m_pszStatusCode != SIP_NULL) ? SipPf_Atoi(m_pszStatusCode) : SipMsgUtil::SIP_SC_INVALID;
}

SIP_BOOL SipStatusLine::DecodeStatusLine(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempLoc = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipStatusLine::DecodeStatusLine: Space Not Found",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszSipVersion = SipCreateString(pStartPt, pTempLoc);
    if (m_pszSipVersion == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*update the start point to the start of Status Code*/
    pStartPt = pTempLoc + SIP_TWO;
    pTempLoc = SIP_NULL;
    /*Find the endpoint of status code*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "SipStatusLine::DecodeStatusLine: Space Not Found",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszStatusCode = SipCreateString(pStartPt, pTempLoc);
    if (m_pszStatusCode == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of reason phrase*/
    pStartPt = pTempLoc + SIP_TWO;
    m_pszReasonPhrase = SipCreateString(pStartPt, pEndPt);
    if (m_pszReasonPhrase == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipStatusLine::DecodeStatusLine: No Reason phrase present in response line",
                SIP_ZERO, SIP_ZERO);
    }

    return SIP_TRUE;
}
