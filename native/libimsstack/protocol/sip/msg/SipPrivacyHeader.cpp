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
#include "msg/SipPrivacyHeader.h"
#include "platform/SipString.h"

SipPrivacyHeader::SipPrivacyHeader() :
        SipHeaderBase(SipHeaderBase::PRIVACY),
        m_objPrivacyList(SipVector<SIP_CHAR*>())
{
}

SipPrivacyHeader::SipPrivacyHeader(const SipPrivacyHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_objPrivacyList(SipVector<SIP_CHAR*>())
{
    SIP_UINT32 nSize = objHeader.m_objPrivacyList.GetSize();
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        const SIP_CHAR* pszTempVal = objHeader.m_objPrivacyList.GetAt(nCount);
        if (pszTempVal != SIP_NULL)
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszTempVal);
            m_objPrivacyList.Add(pszVal);
        }
    }
}

SipPrivacyHeader::~SipPrivacyHeader()
{
    while (m_objPrivacyList.IsEmpty() != SIP_TRUE)
    {
        delete m_objPrivacyList.Top();
        m_objPrivacyList.Pop();
    }
}

SIP_BOOL SipPrivacyHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    SIP_UINT32 nSize = m_objPrivacyList.GetSize();

    if (nSize == 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty privacy values", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    for (SIP_UINT32 i = SIP_ZERO; i < nSize; i++)
    {
        const SIP_CHAR* pszPrivacy = m_objPrivacyList.GetAt(i);

        if (i != SIP_ZERO)
        {
            objBuffer += SIP_SEMI;
        }

        objBuffer += pszPrivacy;
    }

    return SIP_TRUE;
}

SIP_BOOL SipPrivacyHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    SIP_UINT32 nCount = m_objPrivacyList.GetSize();

    if (nCount == 0)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty privacy values", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nCount; nIndex++)
    {
        const SIP_CHAR* pszPrivacy = m_objPrivacyList.GetAt(nIndex);
        if (nIndex != SIP_ZERO)
        {
            SipMsgUtil::Encode(*ppCurrPos, SIP_SEMI);
        }
        SipAbnfUtil::Append(*ppCurrPos, pszPrivacy);
    }
    return SIP_TRUE;
}

SIP_BOOL SipPrivacyHeader::AddPrivacy(const SIP_CHAR* pszPrivacy)
{
    if (pszPrivacy == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempPrivacy = SIP_NULL;
    SipMsgUtil::SetValue(pszPrivacy, pszTempPrivacy);
    return (m_objPrivacyList.Add(pszTempPrivacy) >= 0) ? SIP_TRUE : SIP_FALSE;
}

SIP_BOOL SipPrivacyHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*"Privacy" HCOLON priv-value *(";" priv-value)*/
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Privacy value missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    while (pStartPt < pEndPt)
    {
        const SIP_CHAR* pTempPos = SIP_NULL;

        if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTempPos, SIP_SEMI) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        SIP_CHAR* pszPrivacy = SipAbnfUtil::CreateString(pStartPt, pTempPos);
        if (pszPrivacy == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Put the value into list*/
        if (m_objPrivacyList.Add(pszPrivacy) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Adding in list failed", SIP_ZERO, SIP_ZERO);
            delete[] pszPrivacy;
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            break;
        }
        else
        {
            pStartPt = pTempPos + SIP_TWO;
            pStartPt = SipAbnfUtil::SkipWhiteSpaceFromLeft(pStartPt, pEndPt);
            if (pStartPt > pEndPt)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "No Parameter Present", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipPrivacyHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPrivacyHeader(*reinterpret_cast<SipPrivacyHeader*>(pHeader));
    }
    return new SipPrivacyHeader();
}
