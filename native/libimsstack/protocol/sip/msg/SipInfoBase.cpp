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
#include "msg/SipInfoBase.h"
#include "sip_debug.h"
#include "msg/sip_msgutil.h"
#include "platform/sip_pf_string.h"

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

SIP_BOOL SipInfoBase::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();

    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Empty value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    **ppCurrPos = LEFT_ANGLE;
    (*ppCurrPos)++;

    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = RIGHT_ANGLE;
    (*ppCurrPos)++;

    **ppCurrPos = '\0';

    return EncodeHeaderParameters(ppCurrPos, bParams);
}

SIP_BOOL SipInfoBase::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTemp = SIP_NULL;

    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTemp, LEFT_ANGLE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    pStartPt = pTemp;
    pTemp = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTemp, RIGHT_ANGLE) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    SIP_CHAR* pszValue = sipCreateString(pStartPt, pTemp);
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

    pStartPt = sipSkipFwLWS(pStartPt, pEndPt);
    pTemp = SIP_NULL;

    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTemp, SIP_SEMI) &&
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
