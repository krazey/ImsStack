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
#include "msg/SipIdentityHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipIdentityHeader::SipIdentityHeader() :
        SipHeaderBase(SipHeaderBase::IDENTITY),
        m_pSignedIdentityDigest(SIP_NULL),
        m_pInfo(SIP_NULL)
{
}

SipIdentityHeader::SipIdentityHeader(const SipIdentityHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_pSignedIdentityDigest(SipPf_Strdup(objHeader.m_pSignedIdentityDigest)),
        m_pInfo(SipPf_Strdup(objHeader.m_pInfo))
{
}

SipIdentityHeader::~SipIdentityHeader()
{
    if (m_pSignedIdentityDigest != SIP_NULL)
    {
        delete[] m_pSignedIdentityDigest;
    }

    if (m_pInfo != SIP_NULL)
    {
        delete[] m_pInfo;
    }
}

SIP_VOID SipIdentityHeader::SetSignedIdentityDigest(const SIP_CHAR* pszSignedIdentiDigest)
{
    SipMsgUtil::SetValue(pszSignedIdentiDigest, m_pSignedIdentityDigest);
}

SIP_VOID SipIdentityHeader::SetInfo(const SIP_CHAR* pszInfo)
{
    SipMsgUtil::SetValue(pszInfo, m_pInfo);
}

SIP_BOOL SipIdentityHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Invalid header", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += m_pSignedIdentityDigest;

    objBuffer += SIP_SEMI;

    objBuffer += "info";
    objBuffer += EQUAL;
    objBuffer += LEFT_ANGLE;
    objBuffer += m_pInfo;
    objBuffer += RIGHT_ANGLE;

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipIdentityHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Invalid header", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pSignedIdentityDigest);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SipMsgUtil::Encode(*ppCurrPos, SIP_SEMI);

    SipPf_Strcpy(*ppCurrPos, "info");
    SipEnc_UpdateCurrPos(ppCurrPos);

    SipMsgUtil::Encode(*ppCurrPos, EQUAL);

    SipMsgUtil::Encode(*ppCurrPos, LEFT_ANGLE);

    SipPf_Strcpy(*ppCurrPos, m_pInfo);
    SipEnc_UpdateCurrPos(ppCurrPos);

    SipMsgUtil::Encode(*ppCurrPos, RIGHT_ANGLE);

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipIdentityHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Find the position of First DQUOTE*/
    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    m_pSignedIdentityDigest = SipCreateString(pStartPt, pTempPre);

    if (m_pSignedIdentityDigest == SIP_NULL)
    {
        return SIP_FALSE;
    }

    pStartPt = pTempNext;
    pTempPre = SIP_NULL;
    pTempNext = SIP_NULL;

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, EQUAL) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    if (SipPf_Strncmp(pStartPt, "info", (pTempPre - pStartPt)) != SIP_ZERO)
    {
        return SIP_FALSE;
    }

    pStartPt = pTempNext;
    pTempPre = SIP_NULL;
    pTempNext = SIP_NULL;

    if (SipFindPostDelimiter(pStartPt, pEndPt, &pTempPre, LEFT_ANGLE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    pStartPt = pTempPre;
    pTempPre = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPre, RIGHT_ANGLE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    m_pInfo = SipCreateString(pStartPt, pTempPre);

    if (m_pInfo == SIP_NULL)
    {
        return SIP_FALSE;
    }

    pStartPt = pTempPre + SIP_TWO;
    pTempPre = SIP_NULL;

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, SIP_SEMI) == SIP_TRUE)
    {
        return DecodeHeaderParameters(pTempNext, pEndPt, SIP_SEMI);
    }

    return SIP_TRUE;
}

SipHeaderBase* SipIdentityHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipIdentityHeader(*reinterpret_cast<SipIdentityHeader*>(pHeader));
    }
    return new SipIdentityHeader();
}
