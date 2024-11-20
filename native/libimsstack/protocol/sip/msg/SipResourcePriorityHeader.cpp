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
#include "msg/SipResourcePriorityHeader.h"
#include "platform/SipString.h"

SipResourcePriorityHeader::SipResourcePriorityHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_pszNameSpace(SIP_NULL),
        m_pszRPriority(SIP_NULL)
{
}

SipResourcePriorityHeader::SipResourcePriorityHeader(const SipResourcePriorityHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszNameSpace(SipPf_Strdup(objHeader.m_pszNameSpace)),
        m_pszRPriority(SipPf_Strdup(objHeader.m_pszRPriority))
{
}

SipResourcePriorityHeader::~SipResourcePriorityHeader()
{
    if (m_pszNameSpace != SIP_NULL)
    {
        delete[] m_pszNameSpace;
    }

    if (m_pszRPriority != SIP_NULL)
    {
        delete[] m_pszRPriority;
    }
}

SIP_BOOL SipResourcePriorityHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    objBuffer += m_pszNameSpace;
    objBuffer += SIP_DOT;
    objBuffer += m_pszRPriority;

    return SIP_TRUE;
}

SIP_BOOL SipResourcePriorityHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    SipAbnfUtil::Append(*ppCurrPos, m_pszNameSpace);
    SipMsgUtil::Encode(*ppCurrPos, SIP_DOT);
    SipAbnfUtil::Append(*ppCurrPos, m_pszRPriority);

    return SIP_TRUE;
}

SIP_VOID SipResourcePriorityHeader::SetNameSpace(const SIP_CHAR* pszNameSpace)
{
    SipMsgUtil::SetValue(pszNameSpace, m_pszNameSpace);
}

SIP_VOID SipResourcePriorityHeader::SetRPriority(const SIP_CHAR* pszRPriority)
{
    SipMsgUtil::SetValue(pszRPriority, m_pszRPriority);
}

SIP_BOOL SipResourcePriorityHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;

    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempPre, SIP_DOT) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Dot missing in ResourcePriority", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pszNameSpace = SipAbnfUtil::CreateString(pStartPt, pTempPre);
    if (m_pszNameSpace == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pStartPt = pTempPre + SIP_TWO;
    m_pszRPriority = SipAbnfUtil::CreateString(pStartPt, pEndPt);
    if (m_pszRPriority == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipResourcePriorityHeader::IsValidHeader() const
{
    if (GetHdrType() == SipHeaderBase::RESOURCE_PRIORITY)
    {
        return ((m_pszNameSpace == SIP_NULL) || (m_pszRPriority == SIP_NULL)) ? SIP_FALSE
                                                                              : SIP_TRUE;
    }

    if (((m_pszNameSpace == SIP_NULL) && (m_pszRPriority == SIP_NULL)) ||
            ((m_pszNameSpace != SIP_NULL) && (m_pszRPriority != SIP_NULL)))
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SipHeaderBase* SipResourcePriorityHeader::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipResourcePriorityHeader(
                *reinterpret_cast<SipResourcePriorityHeader*>(pHeader));
    }
    return new SipResourcePriorityHeader(eHeaderType);
}
