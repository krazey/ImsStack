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
#include "msg/SipAuthInfoHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipAuthInfoHeader::SipAuthInfoHeader() :
        SipHeaderBase(SipHeaderBase::AUTHENTICATION_INFO),
        m_pAuthInfoList(SipVector<SipNameValue*>())
{
}
SipAuthInfoHeader::SipAuthInfoHeader(const SipAuthInfoHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pAuthInfoList(SipVector<SipNameValue*>())
{
    SIP_UINT32 nSize = objHeader.m_pAuthInfoList.GetSize();

    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SipNameValue* pElement = objHeader.m_pAuthInfoList.GetAt(nCount);

        if (pElement != SIP_NULL)
        {
            SipNameValue* pNameValue = new SipNameValue(*pElement);

            if (pNameValue != SIP_NULL)
            {
                m_pAuthInfoList.Add(pNameValue);
            }
        }
    }
}

SipAuthInfoHeader::~SipAuthInfoHeader()
{
    while (m_pAuthInfoList.IsEmpty() != SIP_TRUE)
    {
        m_pAuthInfoList.Top()->SipDelete();
        m_pAuthInfoList.Pop();
    }
}

SIP_BOOL SipAuthInfoHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Auth info missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nIndex = SIP_ZERO;
    SIP_UINT32 nSize = m_pAuthInfoList.GetSize();

    while (nIndex < nSize)
    {
        if (nIndex != SIP_ZERO)
        {
            objBuffer += COMMA;
        }

        SipNameValue* pNameValue = m_pAuthInfoList.GetAt(nIndex);

        if (pNameValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Name Value null", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNameValue->Encode(objBuffer) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        nIndex++;
    }

    return SIP_TRUE;
}

SIP_BOOL SipAuthInfoHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Auth info missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nIndex = SIP_ZERO;
    SIP_UINT32 nSize = m_pAuthInfoList.GetSize();

    while (nIndex < nSize)
    {
        if (nIndex != SIP_ZERO)
        {
            SipMsgUtil::Encode(*ppCurrPos, COMMA);
        }

        SipNameValue* pNameValue = m_pAuthInfoList.GetAt(nIndex);

        if (pNameValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Name Value null", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNameValue->Encode(ppCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Name Value Encode fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        nIndex++;
    }

    return SIP_TRUE;
}

const SipNameValue* SipAuthInfoHeader::GetAiInfoVal(SIP_UINT32 nIndex /*default value is zero*/)
{
    if (nIndex < m_pAuthInfoList.GetSize())
    {
        return m_pAuthInfoList.GetAt(nIndex);
    }
    return SIP_NULL;
}

SIP_BOOL SipAuthInfoHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    while (pStartPt <= pEndPt)
    {
        const SIP_CHAR* pTempPos = SIP_NULL;

        if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempPos, COMMA) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        SipNameValue* pNameValue = new SipNameValue();
        if (pNameValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNameValue->Decode(pStartPt, pTempPos, SIP_NULL) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Name value decode failed", SIP_ZERO, SIP_ZERO);
            pNameValue->SipDelete();
            return SIP_FALSE;
        }

        if (m_pAuthInfoList.Add(pNameValue) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Append in list failed", SIP_ZERO, SIP_ZERO);
            pNameValue->SipDelete();
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            return SIP_TRUE;
        }

        pStartPt = pTempPos + SIP_TWO;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipAuthInfoHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipAuthInfoHeader(*reinterpret_cast<SipAuthInfoHeader*>(pHeader));
    }
    return new SipAuthInfoHeader();
}
