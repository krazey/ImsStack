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
#include "msg/SipAllowEventsHeader.h"
#include "SipDebug.h"
#include "platform/SipString.h"
#include "msg/SipMsgUtil.h"

SipAllowEventsHeader::SipAllowEventsHeader() :
        SipHeaderBase(SipHeaderBase::ALLOW_EVENTS),
        m_pEventTemplateList(SIP_NULL)
{
}

SipAllowEventsHeader::SipAllowEventsHeader(const SipAllowEventsHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pEventTemplateList(SIP_NULL)
{
    if (objHeader.m_pEventTemplateList != SIP_NULL)
    {
        m_pEventTemplateList = new SipParameterList(*(objHeader.m_pEventTemplateList));
    }
}

SipAllowEventsHeader::~SipAllowEventsHeader()
{
    if (m_pEventTemplateList != SIP_NULL)
    {
        m_pEventTemplateList->SipDelete();
    }
}

SIP_BOOL SipAllowEventsHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    const SIP_CHAR* pszValue = GetValue();

    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing event package", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += pszValue;

    return (m_pEventTemplateList != SIP_NULL) ? m_pEventTemplateList->Encode(objBuffer, SIP_DOT)
                                              : SIP_TRUE;
}

SIP_BOOL SipAllowEventsHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Event package", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return (m_pEventTemplateList != SIP_NULL) ? m_pEventTemplateList->Encode(ppCurrPos, SIP_DOT)
                                              : SIP_TRUE;
}

SIP_BOOL SipAllowEventsHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPos = SIP_NULL;

    /*Case of having event template*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SIP_DOT) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }

    SIP_CHAR* pszValue = SipCreateString(pStartPt, pTempPos);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    if (pTempPos != pEndPt)
    {
        m_pEventTemplateList = new SipParameterList();
        if (m_pEventTemplateList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Update the tempPos to the start of eventTamplate*/
        pTempPos = pTempPos + SIP_TWO;
        if (m_pEventTemplateList->Decode(pTempPos, pEndPt, SIP_DOT) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Hdr Prm Decoding Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

SipHeaderBase* SipAllowEventsHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipAllowEventsHeader(*reinterpret_cast<SipAllowEventsHeader*>(pHeader));
    }
    return new SipAllowEventsHeader();
}
