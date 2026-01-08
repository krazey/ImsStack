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
#include "msg/SipInfoBase.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

SipInfoBase::SipInfoBase(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType)
{
}

SipInfoBase::SipInfoBase(const SipInfoBase& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipInfoBase::~SipInfoBase() {}

SIP_BOOL SipInfoBase::Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const
{
    const SIP_CHAR* pszValue = GetValue();

    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    objBuffer += LEFT_ANGLE;
    objBuffer += pszValue;
    objBuffer += RIGHT_ANGLE;

    return (bParams == SIP_TRUE) ? EncodeParameters(objBuffer) : SIP_TRUE;
}

SIP_BOOL SipInfoBase::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();

    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    **ppCurrPos = LEFT_ANGLE;
    (*ppCurrPos)++;

    SipAbnfUtil::Append(*ppCurrPos, pszValue);

    **ppCurrPos = RIGHT_ANGLE;
    (*ppCurrPos)++;

    **ppCurrPos = '\0';

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipInfoBase::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTemp = SIP_NULL;

    if (SipAbnfUtil::FindPostDelimiter(pStartPt, pEndPt, pTemp, LEFT_ANGLE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    pStartPt = pTemp;
    pTemp = SIP_NULL;

    if (SipAbnfUtil::FindPreDelimiter(pStartPt, pEndPt, pTemp, RIGHT_ANGLE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    SIP_CHAR* pszValue = SipAbnfUtil::CreateString(pStartPt, pTemp);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    SIP_INT32 nLen = pTemp - pStartPt;
    pStartPt = pTemp + SIP_TWO;

    pStartPt = SipAbnfUtil::SkipWhiteSpaceFromLeft(pStartPt, pEndPt);
    pTemp = SIP_NULL;

    if (SipAbnfUtil::FindPostDelimiter(pStartPt, pEndPt, pTemp, SIP_SEMI) &&
            ((*pStartPt) == SIP_SEMI) == SIP_TRUE)
    {
        return DecodeHeaderParameters(pTemp, pEndPt, SIP_SEMI);
    }

    if (nLen != (nDecLen - 3))
    {
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipInfoBase::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipInfoBase(*static_cast<SipInfoBase*>(pHeader));
    }
    return new SipInfoBase(eHeaderType);
}
