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
#include "msg/SipHeaderList.h"
#include "msg/SipMessage.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

extern SipHeaderBase* (*gaFactoryArray[SipHeaderBase::TYPE_END + SIP_ONE])(
        SIP_INT32, SipHeaderBase*);

SipHeaderList::SipHeaderList(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_objHeaderList(SipVector<SipHeaderBase*>())
{
}

SipHeaderList::SipHeaderList(const SipHeaderList& objHeaderList) :
        SipHeaderBase(objHeaderList)
{
    SIP_UINT32 nSize = objHeaderList.m_objHeaderList.GetSize();
    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SipHeaderBase* pOldHdrBase = objHeaderList.m_objHeaderList.GetAt(nCount);
        if (pOldHdrBase != SIP_NULL)
        {
            SipHeaderBase* pNewHdrBase = GetListObj(pOldHdrBase);
            if (pNewHdrBase != SIP_NULL)
            {
                if (m_objHeaderList.Add(pNewHdrBase) < SIP_ZERO)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Add to list fail", SIP_ZERO, SIP_ZERO);
                    pNewHdrBase->SipDelete();
                }
            }
        }
    }
}

SipHeaderList::~SipHeaderList()
{
    while (m_objHeaderList.IsEmpty() != SIP_TRUE)
    {
        SipHeaderBase* pHeaderBase = m_objHeaderList.Top();
        pHeaderBase->SipDelete();
        m_objHeaderList.Pop();
    }
}

SIP_BOOL SipHeaderList::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*= SIP_TRUE*/)
{
    return EncodeHdr(ppCurrPos, bParams, SipConfiguration::MSG_OPT_ENCODE_NONE);
}

SIP_BOOL SipHeaderList::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams, SIP_UINT32 nMsgOptions)
{
    if (m_objHeaderList.IsEmpty() == SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "List is Empty ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipHeaderBase* pHeader = m_objHeaderList.GetAt(SIP_ZERO);

    if (pHeader == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "pHeader is NULL", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pHeader->GetHdrType() == SipHeaderBase::AUTHENTICATION_INFO)
    {
        nMsgOptions = nMsgOptions | SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE;
    }

    if (pHeader->EncodeHdr(ppCurrPos, bParams) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objHeaderList.GetSize();

    /*Encoding of Next header elements in LIst*/
    for (SIP_UINT32 nCount = SIP_ONE; nCount < nSize; nCount++)
    {
        pHeader = m_objHeaderList.GetAt(nCount);
        if (pHeader == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "pHeader is NULL", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pHeader->GetHdrType() == SipHeaderBase::UNKNOWN)
        {
            SipMsgUtil::EncodeCrlf(*ppCurrPos);
        }
        /*case of Multiple line encoding*/
        else if ((nMsgOptions & SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE) ==
                SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE)
        {
            SipMsgUtil::EncodeCrlf(*ppCurrPos);
            if ((nMsgOptions & SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM) ==
                    SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM)
            {
                SipHeaders::SipEncodeShortHdrName(pHeader->GetHdrType(), ppCurrPos);
            }
            else
            {
                SipHeaders::SipEncodeHdrName(pHeader->GetHdrType(), ppCurrPos, nMsgOptions);
            }
        }
        else if (nCount < nSize)
        {
            SipMsgUtil::Encode(*ppCurrPos, COMMA);
        }

        if (pHeader->EncodeHdr(ppCurrPos, bParams) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

SipHeaderBase* SipHeaderList::GetListObj(SipHeaderBase* pHdr)
{
    SIP_INT32 eHdrType = GetHdrType();
    return gaFactoryArray[eHdrType](eHdrType, pHdr);
}

SIP_BOOL SipHeaderList::AddHeader(SipHeaderBase* pHeader)
{
    if (m_objHeaderList.Add(pHeader) < SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Adding in list failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pHeader->Increment();
    return SIP_TRUE;
}

SIP_BOOL SipHeaderList::InsertHdrAtPos(SipHeaderBase* pHeader, SIP_UINT32 nIndex)
{
    if (nIndex > m_objHeaderList.GetSize())
    {
        return SIP_FALSE;
    }

    m_objHeaderList.InsertAt(pHeader, nIndex);
    pHeader->Increment();
    return SIP_TRUE;
}

void SipHeaderList::RemoveHdr(SIP_UINT32 nIndex)
{
    if (nIndex < m_objHeaderList.GetSize())
    {
        SipHeaderBase* pHeaderBase = m_objHeaderList.GetAt(nIndex);
        pHeaderBase->SipDelete();
        m_objHeaderList.RemoveAt(nIndex);
    }
}

SipHeaderBase* SipHeaderList::GetObj(SIP_UINT32 nIndex)
{
    if (m_objHeaderList.GetSize() <= nIndex)
    {
        return SIP_NULL;
    }

    SipHeaderBase* pHdr = m_objHeaderList.GetAt(nIndex);
    if (pHdr != SIP_NULL)
    {
        pHdr->Increment();
    }

    return pHdr;
}

SIP_BOOL SipHeaderList::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return IsEmptyHeaderBodyAllowed();
    }

    if (GetHdrType() == SipHeaderBase::AUTHENTICATION_INFO)
    {
        SipHeaderBase* pHdrBase = GetListObj();
        if (pHdrBase == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pHdrBase->DecodeHdr(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Hdr Decoding Fail", SIP_ZERO, SIP_ZERO);
            pHdrBase->SipDelete();
            return SIP_FALSE;
        }

        if (AddHeader(pHdrBase) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Add to list Failed", SIP_ZERO, SIP_ZERO);
            pHdrBase->SipDelete();
            return SIP_FALSE;
        }

        return SIP_TRUE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    while (pStartPt <= pEndPt)
    {
        const SIP_CHAR* pTempPre = SIP_NULL;
        const SIP_CHAR* pTempNext = SIP_NULL;

        if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, COMMA) == SIP_FALSE)
        {
            pTempPre = pEndPt;
            pTempNext = pEndPt;
        }

        SipHeaderBase* pHdrBase = GetListObj();
        if (pHdrBase == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_UINT32 nLen = pTempPre - pStartPt + SIP_ONE;
        if (pHdrBase->DecodeHdr(pStartPt, nLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Hdr Decoding Fail", SIP_ZERO, SIP_ZERO);
            pHdrBase->SipDelete();
            return SIP_FALSE;
        }

        if (AddHeader(pHdrBase) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Add to list Failed", SIP_ZERO, SIP_ZERO);
            pHdrBase->SipDelete();
            return SIP_FALSE;
        }
        pHdrBase->SipDelete();
        pStartPt = (pTempNext == pEndPt) ? (pTempNext + SIP_ONE) : pTempNext;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipHeaderList::GetNewListObj(SIP_INT32 eHdr, SipHeaderBase* pHeaderList)
{
    if (pHeaderList != SIP_NULL)
    {
        return new SipHeaderList(*(static_cast<SipHeaderList*>(pHeaderList)));
    }
    return new SipHeaderList(eHdr);
}
