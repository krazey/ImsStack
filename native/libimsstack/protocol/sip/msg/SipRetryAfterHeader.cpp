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
#include "msg/SipRetryAfterHeader.h"
#include "platform/SipString.h"

SipRetryAfterHeader::SipRetryAfterHeader() :
        SipHeaderBase(SipHeaderBase::RETRY_AFTER_SEC),
        m_nDeltaSec(SIP_ZERO),
        m_pszComment(SIP_NULL)
{
}

SipRetryAfterHeader::SipRetryAfterHeader(const SipRetryAfterHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_nDeltaSec(objHeader.m_nDeltaSec),
        m_pszComment(SipPf_Strdup(objHeader.m_pszComment))
{
}

SipRetryAfterHeader::~SipRetryAfterHeader()
{
    if (m_pszComment != SIP_NULL)
    {
        delete[] m_pszComment;
    }
}

SIP_BOOL SipRetryAfterHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    objBuffer += m_nDeltaSec;

    if (m_pszComment != SIP_NULL)
    {
        objBuffer += LPARAN;
        objBuffer += m_pszComment;
        objBuffer += RPARAN;
    }

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipRetryAfterHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    const SIP_UINT16 MAX_RETRY_AFTER_LEN = 11;
    SIP_CHAR szLen[MAX_RETRY_AFTER_LEN];
    SipPf_Sprintf(szLen, "%u", m_nDeltaSec);

    SipPf_Strcpy(*ppCurrPos, szLen);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_pszComment != SIP_NULL)
    {
        SIP_ENC_LPAREN(*ppCurrPos);
        SipPf_Strcpy(*ppCurrPos, m_pszComment);
        SipEnc_UpdateCurrPos(ppCurrPos);
        SIP_ENC_RPAREN(*ppCurrPos);
    }

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipRetryAfterHeader::SetComment(const SIP_CHAR* pszComment)
{
    return SetCharVar(pszComment, m_pszComment);
}

SIP_BOOL SipRetryAfterHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pCommentStart = SIP_NULL;
    const SIP_CHAR* pCommentEnd = SIP_NULL;
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

    SIP_BOOL bStatus = FindComment(pStartPt, pEndPt, pCommentStart, pCommentEnd);

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid comment", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        if ((pCommentEnd == SIP_NULL) || ((pTempPre + 1) > pCommentEnd))
        {
            if (DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI) == SIP_FALSE)
            {
                return SIP_FALSE;
            }
            pEndPt = pTempPre;
            pTempPre = SIP_NULL;
            pTempNext = SIP_NULL;
        }
    }
    else
    {
        // if there is some extra string after comment ends
        if ((pCommentEnd != SIP_NULL) && (pCommentEnd != pEndPt))
        {
            return SIP_FALSE;
        }
    }

    // if comment exists
    if (pCommentStart != SIP_NULL)
    {
        if ((pCommentStart + SIP_ONE) == pCommentEnd)
        {
            SetCharVar("", m_pszComment);
        }
        else
        {
            m_pszComment = SipCreateString(pCommentStart + SIP_ONE, pCommentEnd - SIP_ONE);
        }

        if (m_pszComment == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pEndPt = pCommentStart - 1;
    }

    pEndPt = SipSkipRwLWS(pStartPt, pEndPt);
    /*Now decode the delta sec value*/
    SIP_CHAR* pszValue = SipCreateString(pStartPt, pEndPt);
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (SipPf_Atoi_Unsigned(pszValue, m_nDeltaSec) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr:Retry After value is not valid",
                SIP_ZERO, SIP_ZERO);
        delete[] pszValue;
        return SIP_FALSE;
    }

    delete[] pszValue;

    return SIP_TRUE;
}

SipHeaderBase* SipRetryAfterHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipRetryAfterHeader(*reinterpret_cast<SipRetryAfterHeader*>(pHeader));
    }
    return new SipRetryAfterHeader();
}
