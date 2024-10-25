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
#include "msg/SipTimeStampHeader.h"
#include "platform/SipString.h"

SipTimeStampHeader::SipTimeStampHeader() :
        SipHeaderBase(SipHeaderBase::TIMESTAMP),
        m_pszTimeVal(SIP_NULL),
        m_pszDelay(SIP_NULL)
{
}

SipTimeStampHeader::SipTimeStampHeader(const SipTimeStampHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pszTimeVal(SipPf_Strdup(objHeader.m_pszTimeVal)),
        m_pszDelay(SipPf_Strdup(objHeader.m_pszDelay))
{
}

SipTimeStampHeader::~SipTimeStampHeader()
{
    if (m_pszTimeVal != SIP_NULL)
    {
        delete[] m_pszTimeVal;
        m_pszTimeVal = SIP_NULL;
    }
    if (m_pszDelay != SIP_NULL)
    {
        delete[] m_pszDelay;
        m_pszDelay = SIP_NULL;
    }
}

SIP_BOOL SipTimeStampHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode: Missing timestamp", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pszTimeVal;

    if (m_pszDelay != SIP_NULL)
    {
        objBuffer += SPACE;
        objBuffer += m_pszDelay;
    }

    return SIP_TRUE;
}

SIP_BOOL SipTimeStampHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    /*Encoding of header Value  i.e.
      "Timestamp" HCOLON 1*(DIGIT) [ "." *(DIGIT) ] [ LWS delay ]   */
    /*Encoding of DIGIT*/
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeHdr: Missing TimeStamp ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszTimeVal);
    SipEnc_UpdateCurrPos(ppCurrPos);

    /*Encoding of Delay*/
    if (m_pszDelay != SIP_NULL)
    {
        SipMsgUtil::Encode(*ppCurrPos, SPACE);

        SipPf_Strcpy(*ppCurrPos, m_pszDelay);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return SIP_TRUE;
}

SIP_VOID SipTimeStampHeader::SetTimeVal(const SIP_CHAR* pszTimeVal)
{
    SipMsgUtil::SetValue(pszTimeVal, m_pszTimeVal);
}

SIP_VOID SipTimeStampHeader::SetDelay(const SIP_CHAR* pszDelay)
{
    SipMsgUtil::SetValue(pszDelay, m_pszDelay);
}

SIP_BOOL SipTimeStampHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    /*Find the LWS i.e. End of Transport*/
    if (SipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        pTempPre = pEndPt;
    }

    m_pszTimeVal = SipCreateString(pStartPt, pTempPre);
    if (m_pszTimeVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTempPre != pEndPt)
    {
        /*point to the start of the LWS*/
        pTempPre = pTempPre + SIP_ONE;
        pStartPt = SipSkipFwLWS(pTempPre, pEndPt);
        m_pszDelay = SipCreateString(pStartPt, pEndPt);
        if (m_pszDelay == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

SipHeaderBase* SipTimeStampHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipTimeStampHeader(*reinterpret_cast<SipTimeStampHeader*>(pHeader));
    }
    return new SipTimeStampHeader();
}
