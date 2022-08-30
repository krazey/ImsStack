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
#include "msg/SipPAssertedServiceHeader.h"
#include "SipDebug.h"
#include "platform/SipString.h"
#include "msg/SipMsgUtil.h"

SipPAssertedServiceHeader::SipPAssertedServiceHeader() :
        SipHeaderBase(SipHeaderBase::P_ASSERTED_SERVICE)
{
}

SipPAssertedServiceHeader::SipPAssertedServiceHeader(const SipPAssertedServiceHeader& objHeader) :
        SipHeaderBase(objHeader)
{
}

SipPAssertedServiceHeader::~SipPAssertedServiceHeader() {}

SIP_BOOL SipPAssertedServiceHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Case of nothing is present*/
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "SipPAssertedServiceHeader::DecodeHdr", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SipSkipFwLWS(pStartPt, pEndPt);

    // validate the service id value
    SIP_CHAR* pszTempString = SipCreateString(pStartPt, pStartPt + SIP_NINE);
    if (SipPf_Stricmp("urn:urn-7:", pszTempString) != SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipPAssertedServiceHeader::DecodeHdr:value must start by urn:urn-7:", SIP_ZERO,
                SIP_ZERO);
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

    SIP_CHAR* pTempCurr = pStartPt + SIP_TEN;

    // Service id Value Validations   toplebel.subservice id
    // fIND dot and validate topLevel ID and SubService Id
    SIP_UINT16 nTopLevelIDLen = 0;
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    if (SipFindActualPos(pTempCurr, pEndPt, &pTempPre, &pTempNext, SIP_DOT) == SIP_TRUE)
    {
        nTopLevelIDLen = pTempPre - pTempCurr + 1;
    }
    else
    {
        nTopLevelIDLen = pEndPt - pTempCurr + 1;
    }

    if ((nTopLevelIDLen > MAXLETDIG) || (nTopLevelIDLen <= 0))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipPAssertedServiceHeader::DecodeHdr:invalid subService id", SIP_ZERO, SIP_ZERO);
    }

    if (pTempNext != SIP_NULL)
    {
        while (pTempNext <= pEndPt)
        {
            if ((IS_ALPHANUM(*pTempNext) == SIP_FALSE) && !IS_HYPHEN(*pTempNext) &&
                    (*pTempNext != SIP_DOT))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipPAssertedServiceHeader::DecodeHdr:invalid subService id", SIP_ZERO,
                        SIP_ZERO);
            }
            if ((*pTempNext == SIP_DOT) && (pTempNext == pEndPt))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipPAssertedServiceHeader::DecodeHdr:invalid subService id", SIP_ZERO,
                        SIP_ZERO);
            }
            pTempNext = pTempNext + SIP_ONE;
        }
    }

    SIP_UINT16 nCounter = 0;
    // Validate Top Lebel
    while ((nCounter < MAXLETDIG) && (pTempCurr < pTempPre))
    {
        if ((IS_ALPHANUM(*pTempCurr) == SIP_FALSE) && !IS_HYPHEN(*pTempCurr))
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipPAssertedServiceHeader::DecodeHdr:Top lebel is invalid", SIP_ZERO,
                    SIP_ZERO);
        }
        pTempCurr = pTempCurr + SIP_ONE;
        nCounter++;
    }

    // create and copy the service id
    SIP_CHAR* pszValue = SipCreateString(pStartPt, pEndPt);
    if (SetValue(pszValue) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipPAssertedServiceHeader::DecodeHdr:Memory Allocation Failed", SIP_ZERO,
                SIP_ZERO);
        if (pszValue != SIP_NULL)
        {
            delete[] pszValue;
        }
        return SIP_FALSE;
    }
    delete[] pszValue;

    return SIP_TRUE;
}

SipHeaderBase* SipPAssertedServiceHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipPAssertedServiceHeader(
                *reinterpret_cast<SipPAssertedServiceHeader*>(pHeader));
    }
    return new SipPAssertedServiceHeader();
}
