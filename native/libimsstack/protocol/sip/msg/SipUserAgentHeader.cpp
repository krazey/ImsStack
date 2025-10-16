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
#include "msg/SipUserAgentHeader.h"
#include "platform/SipString.h"

SipUserAgentHeader::SipUserAgentHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_objProductList(SipVector<SIP_CHAR*>())
{
}

SipUserAgentHeader::SipUserAgentHeader(const SipUserAgentHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_objProductList(SipVector<SIP_CHAR*>())
{
    SIP_UINT32 nSize = objHeader.m_objProductList.GetSize();
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        const SIP_CHAR* pszTempszVal = objHeader.m_objProductList.GetAt(nCount);
        if (pszTempszVal != SIP_NULL)
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszTempszVal);
            m_objProductList.Add(pszVal);
        }
    }
}

SipUserAgentHeader::~SipUserAgentHeader()
{
    while (m_objProductList.IsEmpty() != SIP_TRUE)
    {
        delete m_objProductList.Top();
        m_objProductList.Pop();
    }
}

SIP_BOOL SipUserAgentHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (m_objProductList.IsEmpty() == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No header body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objProductList.GetSize();

    for (SIP_UINT32 i = SIP_ZERO; i < nSize; i++)
    {
        if (i != SIP_ZERO)
        {
            objBuffer += SPACE;
        }

        const SIP_CHAR* pszValue = m_objProductList.GetAt(i);

        objBuffer += pszValue;
    }

    return SIP_TRUE;
}

SIP_BOOL SipUserAgentHeader::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_objProductList.IsEmpty() == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No header body", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objProductList.GetSize();
    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nSize; nIndex++)
    {
        if (nIndex != SIP_ZERO)
        {
            SipMsgUtil::Encode(*ppCurrPos, SPACE);
        }

        SipAbnfUtil::Append(*ppCurrPos, m_objProductList.GetAt(nIndex));
    }

    return SIP_TRUE;
}

SIP_BOOL SipUserAgentHeader::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pTempPos = SIP_NULL;
    const SIP_CHAR* pCommentStart = SIP_NULL;
    const SIP_CHAR* pCommentEnd = SIP_NULL;
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    /*"UserAgent" HCOLON UserAgent-val *(LWS UserAgent-val) */
    while (pStartPt < pEndPt)
    {
        SIP_BOOL bStatus = FindComment(pStartPt, pEndPt, pCommentStart, pCommentEnd);

        if (bStatus == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid comment", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (SipAbnfUtil::FindWhiteSpace(pStartPt, pEndPt, pTempPos) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        // check LWS is between comment
        if ((pCommentStart != SIP_NULL) &&
                ((pCommentStart < pTempPos) && (pTempPos < pCommentEnd)) == SIP_TRUE)
        {
            pStartPt = pCommentStart;
            pTempPos = pCommentEnd;
        }

        SIP_CHAR* pszUserAgent = SipAbnfUtil::CreateString(pStartPt, pTempPos);
        if (pszUserAgent == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Put the value into list*/
        if (m_objProductList.Add(pszUserAgent) < SIP_ZERO)
        {
            delete[] pszUserAgent;
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Adding in list failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            pStartPt = pEndPt;
        }
        else
        {
            pTempPos = pTempPos + SIP_ONE;
            pStartPt = SipAbnfUtil::SkipWhiteSpaceFromLeft(pTempPos, pEndPt);
            pTempPos = SIP_NULL;
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipUserAgentHeader::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipUserAgentHeader(*reinterpret_cast<SipUserAgentHeader*>(pHeader));
    }
    return new SipUserAgentHeader(eHdr);
}
