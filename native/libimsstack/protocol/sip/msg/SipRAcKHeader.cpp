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
#include "msg/SipRAcKHeader.h"
#include "platform/SipString.h"

SipRAcKHeader::SipRAcKHeader() :
        SipHeaderBase(SipHeaderBase::RACK),
        m_nResponseNum(SIP_ZERO),
        m_nCSeqNum(SIP_ZERO),
        m_pszMethod(SIP_NULL)
{
}

SipRAcKHeader::SipRAcKHeader(const SipRAcKHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_nResponseNum(objHeader.m_nResponseNum),
        m_nCSeqNum(objHeader.m_nCSeqNum),
        m_pszMethod(SipPf_Strdup(objHeader.m_pszMethod))
{
}

SipRAcKHeader::~SipRAcKHeader()
{
    if (m_pszMethod != SIP_NULL)
    {
        delete[] m_pszMethod;
    }
}

SIP_BOOL SipRAcKHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing method", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    AString strValue;
    strValue.Sprintf("%u %u %s", m_nResponseNum, m_nCSeqNum, m_pszMethod);

    objBuffer += strValue;

    return SIP_TRUE;
}

SIP_BOOL SipRAcKHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Method missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Sprintf(*ppCurrPos, "%u %u %s", m_nResponseNum, m_nCSeqNum, m_pszMethod);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_VOID SipRAcKHeader::SetMethod(const SIP_CHAR* pszMethod)
{
    SetCharVar(pszMethod, m_pszMethod);
}

SIP_BOOL SipRAcKHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
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
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: LWS missing in RAcK", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszResponseNum = SipCreateString(pStartPt, pTempPre);
    if (pszResponseNum == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nResponseNum = SipPf_Atoi(pszResponseNum);
    delete[] pszResponseNum;

    /*Skip Fw LWS And Get the Start of CSeq Num
      i.e. sent-by = host [ COLON port ]  */
    pTempPre = pTempPre + SIP_ONE;  // point to the start of LWS
    pStartPt = SipSkipFwLWS(pTempPre, pEndPt);
    pTempPre = SIP_NULL;

    /*Now find the end of CSeq Num*/
    if (SipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: LWS missing in RAcK", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszCSeqNum = SipCreateString(pStartPt, pTempPre);
    if (pszCSeqNum == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nCSeqNum = SipPf_Atoi(pszCSeqNum);
    delete[] pszCSeqNum;

    /*Update the start point*/
    pTempPre = pTempPre + SIP_ONE;  // point to the start of LWS
    pStartPt = SipSkipFwLWS(pTempPre, pEndPt);
    pTempPre = SIP_NULL;

    m_pszMethod = SipCreateString(pStartPt, pEndPt);
    if (m_pszMethod == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipRAcKHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipRAcKHeader(*reinterpret_cast<SipRAcKHeader*>(pHeader));
    }
    return new SipRAcKHeader();
}
