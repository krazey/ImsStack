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
#include "msg/SipEventHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipEventHeader::SipEventHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_objEventTemplates(SipVector<SIP_CHAR*>())
{
}

SipEventHeader::SipEventHeader(const SipEventHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_objEventTemplates(SipVector<SIP_CHAR*>())
{
    SIP_UINT32 nSize = objHeader.m_objEventTemplates.GetSize();

    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nSize; nIndex++)
    {
        SIP_CHAR* pTemplate = objHeader.m_objEventTemplates.GetAt(nIndex);

        if (pTemplate != SIP_NULL)
        {
            SIP_CHAR* pEventTemplate = SipPf_Strdup(pTemplate);

            if (pEventTemplate != SIP_NULL)
            {
                m_objEventTemplates.Add(pEventTemplate);
            }
        }
    }
}

SipEventHeader::~SipEventHeader()
{
    while (m_objEventTemplates.IsEmpty() != SIP_TRUE)
    {
        delete[] m_objEventTemplates.Top();
        m_objEventTemplates.Pop();
    }
}

SIP_BOOL SipEventHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    const SIP_CHAR* pszValue = GetValue();

    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing event package", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += pszValue;

    SIP_UINT32 nSize = m_objEventTemplates.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        objBuffer += SIP_DOT;
        objBuffer += m_objEventTemplates.GetAt(nIndex);
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipEventHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams)
{
    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Event package", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipAbnfUtil::Append(*ppCurrPos, pszValue);

    SIP_UINT32 nSize = m_objEventTemplates.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        *(*ppCurrPos) = SIP_DOT;
        (*ppCurrPos)++;

        SipAbnfUtil::Append(*ppCurrPos, m_objEventTemplates.GetAt(nIndex));
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipEventHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

    if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, SIP_SEMI) ==
            SIP_TRUE)
    {
        if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
        {
            return SIP_FALSE;
        }
        pEndPt = pTempPre;
    }

    // Header ends with DOT. Example : "event-package.event-template."
    if (*pEndPt == SIP_DOT)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Template ends with DOT", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pTempPos = SIP_NULL;
    /*Case of having event template*/
    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempPos, SIP_DOT) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }

    SIP_CHAR* pszValue = SipAbnfUtil::CreateString(pStartPt, pTempPos);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    // No event templates
    if (pTempPos == pEndPt)
    {
        return SIP_TRUE;
    }

    /*Update the start position to the start of eventTamplate*/
    pStartPt = pTempPos + SIP_TWO;
    pTempPos = SIP_NULL;

    while (pStartPt < pEndPt)
    {
        pTempNext = SIP_NULL;
        if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPos, pTempNext, SIP_DOT) ==
                SIP_FALSE)
        {
            pTempPos = pEndPt;
            pTempNext = pEndPt;
        }

        // Consecutive DOTS present without template. Example : "package.templ1..templ2"
        if ((pTempPos == pStartPt) && (*pTempPos == SIP_DOT))
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty template", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_CHAR* pTemplate = SipAbnfUtil::CreateString(pStartPt, pTempPos);
        if (pTemplate == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        m_objEventTemplates.Add(pTemplate);

        pStartPt = pTempNext;
        pTempPos = SIP_NULL;
    }

    return SIP_TRUE;
}

SIP_BOOL SipEventHeader::IsTemplatePresent(const SIP_CHAR* pTemplateName) const
{
    SIP_UINT32 nSize = m_objEventTemplates.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        const SIP_CHAR* pEventTemplate = m_objEventTemplates.GetAt(nIndex);
        if (SipPf_Strcmp(pTemplateName, pEventTemplate) == 0)
        {
            return SIP_TRUE;
        }
    }
    return SIP_FALSE;
}

SIP_INT32 SipEventHeader::GetTemplateIndex(const SIP_CHAR* pTemplateName) const
{
    SIP_UINT32 nSize = m_objEventTemplates.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        const SIP_CHAR* pEventTemplate = m_objEventTemplates.GetAt(nIndex);
        if (SipPf_Strcmp(pTemplateName, pEventTemplate) == 0)
        {
            return nIndex;
        }
    }
    return -1;
}

SIP_VOID SipEventHeader::AddTemplate(const SIP_CHAR* pTemplateName)
{
    SIP_CHAR* pTemplate = SipPf_Strdup(pTemplateName);

    if (pTemplate == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return;
    }
    m_objEventTemplates.Add(pTemplate);
}

SIP_VOID SipEventHeader::RemoveTemplate(const SIP_CHAR* pTemplateName)
{
    SIP_UINT32 nSize = m_objEventTemplates.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        SIP_CHAR* pEventTemplate = m_objEventTemplates.GetAt(nIndex);
        if (SipPf_Strcmp(pTemplateName, pEventTemplate) == 0)
        {
            delete[] pEventTemplate;
            m_objEventTemplates.RemoveAt(nIndex);
            return;
        }
    }
}

SipHeaderBase* SipEventHeader::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipEventHeader(*reinterpret_cast<SipEventHeader*>(pHeader));
    }
    return new SipEventHeader(eHeaderType);
}
