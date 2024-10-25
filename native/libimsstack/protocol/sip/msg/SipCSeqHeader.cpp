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
#include "msg/SipCSeqHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipCSeqHeader::SipCSeqHeader() :
        SipHeaderBase(SipHeaderBase::CSEQ),
        m_pszMethod(SIP_NULL),
        m_nSeq(SIP_ZERO)
{
}

SipCSeqHeader::SipCSeqHeader(const SipCSeqHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszMethod(SipPf_Strdup(objHeader.m_pszMethod)),
        m_nSeq(objHeader.m_nSeq)
{
}

SipCSeqHeader::~SipCSeqHeader()
{
    if (m_pszMethod != SIP_NULL)
    {
        delete[] m_pszMethod;
    }
}

SIP_BOOL SipCSeqHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing CSeq method", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_nSeq;
    objBuffer += SPACE;
    objBuffer += m_pszMethod;

    return SIP_TRUE;
}

SIP_BOOL SipCSeqHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing CSeq Method", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_UINT16 MAX_CSEQ_LEN = 11;
    SIP_CHAR szBuf[MAX_CSEQ_LEN];

    SipPf_Sprintf(szBuf, "%u", m_nSeq);
    SipPf_Strcpy(*ppCurrPos, szBuf);
    SipEnc_UpdateCurrPos(ppCurrPos);
    SipMsgUtil::Encode(*ppCurrPos, SPACE);
    SipPf_Strcpy(*ppCurrPos, m_pszMethod);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_VOID SipCSeqHeader::SetMethod(const SIP_CHAR* pszMethod)
{
    SipMsgUtil::SetValue(pszMethod, m_pszMethod);
}

SIP_BOOL SipCSeqHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;

    if (SipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "LWS missing in Cseq", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszSeq = SipCreateString(pStartPt, pTempPre);
    if (pszSeq == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (SipPf_Atoi_Unsigned(pszSeq, m_nSeq) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid CSeq Value", SIP_ZERO, SIP_ZERO);
        delete[] pszSeq;
        return SIP_FALSE;
    }

    delete[] pszSeq;

    pTempPre = pTempPre + SIP_ONE;
    pStartPt = SipSkipFwLWS(pTempPre, pEndPt);

    m_pszMethod = SipCreateString(pStartPt, pEndPt);
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipCSeqHeader::IsValidHeader() const
{
    if (m_pszMethod == SIP_NULL)
    {
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

SipHeaderBase* SipCSeqHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipCSeqHeader(*reinterpret_cast<SipCSeqHeader*>(pHeader));
    }
    return new SipCSeqHeader();
}
