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
#include "msg/SipPPreferredServiceHeader.h"
#include "platform/SipString.h"

SipPPreferredServiceHeader::SipPPreferredServiceHeader(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType)
{
}

SipPPreferredServiceHeader::SipPPreferredServiceHeader(
        const SipPPreferredServiceHeader& objHeader) :
        SipHeaderBase(objHeader)

{
}

SipPPreferredServiceHeader::~SipPPreferredServiceHeader() {}

SIP_BOOL SipPPreferredServiceHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SipSkipFwLWS(pStartPt, pEndPt);

    // validate urn:urn-7 mandatory prefix
    SIP_CHAR* pszTempString = SipCreateString(pStartPt, pStartPt + SIP_NINE);
    if (SipPf_Stricmp("urn:urn-7:", pszTempString) != SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "DecodeHdr:value must start by urn:urn-7:", SIP_ZERO, SIP_ZERO);
        if (pszTempString != SIP_NULL)
        {
            delete[] pszTempString;
        }
#ifdef SIP_STRICT_PARSING
        return SIP_FALSE;
#endif
    }
    else if (pszTempString != SIP_NULL)
    {
        delete[] pszTempString;
    }

    const SIP_CHAR* pTempCurr = pStartPt + SIP_TEN;
    const SIP_CHAR* pTempPre = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;
    // Find First dot and validate SubService Id
    if (SipFindActualPos(pTempCurr, pEndPt, &pTempPre, &pTempNext, SIP_DOT) == SIP_TRUE)
    {
        while (pTempNext <= pEndPt)
        {
            if ((IS_ALPHANUM(*pTempNext) == SIP_FALSE) && !IS_HYPHEN(*pTempNext) &&
                    (*pTempNext != SIP_DOT))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr:invalid subService id", SIP_ZERO,
                        SIP_ZERO);
            }
            if ((*pTempNext == SIP_DOT) && (pTempNext == pEndPt))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeHdr:invalid subService id", SIP_ZERO,
                        SIP_ZERO);
            }
            pTempNext = pTempNext + SIP_ONE;
        }
    }
    SIP_UINT16 nCounter = SIP_ZERO;
    // Validate Top level
    while (pTempCurr < pTempPre)
    {
        if (((IS_ALPHANUM(*pTempCurr) == SIP_FALSE) && !IS_HYPHEN(*pTempCurr)) ||
                (nCounter > MAX_LET_DIG))
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecodeHdr:Top level is invalid", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pTempCurr = pTempCurr + SIP_ONE;
        nCounter++;
    }

    // create the service and copy
    SIP_CHAR* pszValue = SipCreateString(pStartPt, pEndPt);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecodeHdr:Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;
    return SIP_TRUE;
}

SipHeaderBase* SipPPreferredServiceHeader::GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPPreferredServiceHeader(
                *reinterpret_cast<SipPPreferredServiceHeader*>(pHeader));
    }
    return new SipPPreferredServiceHeader(eHeaderType);
}
