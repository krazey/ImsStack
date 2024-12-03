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
#include "msg/SipMsgUtil.h"
#include "msg/SipWarningHeader.h"
#include "platform/SipString.h"

SipWarningHeader::SipWarningHeader() :
        SipHeaderBase(SipHeaderBase::WARNING),
        m_nWarnCode(SIP_ZERO),
        m_pszWarnAgent(SIP_NULL),
        m_pszWarnText(SIP_NULL)
{
}

SipWarningHeader::SipWarningHeader(const SipWarningHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_nWarnCode(objHeader.m_nWarnCode),
        m_pszWarnAgent(SipPf_Strdup(objHeader.m_pszWarnAgent)),
        m_pszWarnText(SipPf_Strdup(objHeader.m_pszWarnText))
{
}

SipWarningHeader::~SipWarningHeader()
{
    if (m_pszWarnAgent != SIP_NULL)
    {
        delete[] m_pszWarnAgent;
        m_pszWarnAgent = SIP_NULL;
    }
    if (m_pszWarnText != SIP_NULL)
    {
        delete[] m_pszWarnText;
        m_pszWarnText = SIP_NULL;
    }
}

SIP_BOOL SipWarningHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Missing warn-agent or warn-text", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_nWarnCode;
    objBuffer += SPACE;
    objBuffer += m_pszWarnAgent;
    objBuffer += SPACE;
    objBuffer += m_pszWarnText;

    return SIP_TRUE;
}

SIP_BOOL SipWarningHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Missing warn-agent or warn-text", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Sprintf(*ppCurrPos, "%u", m_nWarnCode);
    SipAbnfUtil::UpdateCurrentPosition(*ppCurrPos);
    SipMsgUtil::Encode(*ppCurrPos, SPACE);
    SipAbnfUtil::Append(*ppCurrPos, m_pszWarnAgent);
    SipMsgUtil::Encode(*ppCurrPos, SPACE);
    SipAbnfUtil::Append(*ppCurrPos, m_pszWarnText);

    return SIP_TRUE;
}

SIP_VOID SipWarningHeader::SetWarnAgent(const SIP_CHAR* pszWarnAgent)
{
    SipMsgUtil::SetValue(pszWarnAgent, m_pszWarnAgent);
}

SIP_VOID SipWarningHeader::SetWarnText(const SIP_CHAR* pszWarnText)
{
    const SIP_CHAR* pszTempWarnText = pszWarnText;
    SIP_INT32 nLength = SipPf_Strlen(pszTempWarnText);

    if (nLength == SIP_ZERO)
    {
        pszTempWarnText = "\"\"";
        nLength = 2;
    }

    const SIP_CHAR* pszEnd = pszTempWarnText + nLength - SIP_ONE;

    if (IS_DQUOTE(*pszTempWarnText) && IS_DQUOTE(*pszEnd))
    {
        SipMsgUtil::SetValue(pszTempWarnText, m_pszWarnText);
    }
    else
    {
        // 2 DQUOTE + null character.
        SIP_CHAR* pszNewWarnText = new SIP_CHAR[nLength + 3];
        SipPf_Snprintf(pszNewWarnText, nLength + 3, "\"%s\"", pszWarnText);
        SipMsgUtil::SetValue(pszNewWarnText, m_pszWarnText);
        delete[] pszNewWarnText;
    }
}

SIP_BOOL SipWarningHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempLoc = SIP_NULL;

    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Space not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszWarnCode = SipAbnfUtil::CreateString(pStartPt, pTempLoc);
    if (pszWarnCode == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nWarnCode = SipPf_Atoi(pszWarnCode);
    delete[] pszWarnCode;
    if ((m_nWarnCode < MIN_WARNCODE) || (m_nWarnCode > MAX_WARNCODE))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Warn code value is not valid", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*update the start point to the start of Warn Agent*/
    pStartPt = pTempLoc + SIP_TWO;
    pTempLoc = SIP_NULL;
    /*Find the endpoint of Warn Agent*/
    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempLoc, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Space not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszWarnAgent = SipAbnfUtil::CreateString(pStartPt, pTempLoc);
    if (m_pszWarnAgent == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of Warn text*/
    pStartPt = pTempLoc + SIP_TWO;

    if (!IS_DQUOTE(*pStartPt) || !IS_DQUOTE(*pEndPt))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid warn-text", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszWarnText = SipAbnfUtil::CreateString(pStartPt, pEndPt);
    if (m_pszWarnText == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipWarningHeader::IsValidHeader() const
{
    if (((m_nWarnCode < MIN_WARNCODE) || (m_nWarnCode > MAX_WARNCODE)) ||
            (m_pszWarnAgent == SIP_NULL) || (m_pszWarnText == SIP_NULL))
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipWarningHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipWarningHeader(*reinterpret_cast<SipWarningHeader*>(pHeader));
    }
    return new SipWarningHeader();
}
