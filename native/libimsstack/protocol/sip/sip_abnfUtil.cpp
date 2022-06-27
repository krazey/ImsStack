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
#include "sip_abnfUtil.h"
#include "platform/sip_pf_memory.h"

SIP_CHAR* sipSkipRwLWS(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    if ((pStartPt == SIP_NULL) || (pEndPt == SIP_NULL))
    {
        return SIP_NULL;
    }

    SIP_CHAR* pBuffTemp = pEndPt;

    while ((pBuffTemp > pStartPt) && IS_WSP(*pBuffTemp))
    {
        pBuffTemp--;
    }

    SIP_CHAR* pBuffPrev = pBuffTemp - SIP_ONE;

    /*Allow one CRLF*/
    if ((pBuffPrev > pStartPt) && IS_CRLF(*pBuffPrev, *pBuffTemp))
    {
        pBuffTemp = pBuffTemp - SIP_TWO;
    }

    while ((pBuffTemp > pStartPt) && IS_WSP(*pBuffTemp))
    {
        pBuffTemp--;
    }

    return pBuffTemp;
}

SIP_CHAR* sipCreateString(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*Validate the input*/
    if (pEndPt < pStartPt)
    {
        return SIP_NULL;
    }
    /*Take one extraa memory for NULL*/
    SIP_UINT32 nSize = (pEndPt - pStartPt) + SIP_TWO;
    SIP_CHAR* pString =  new SIP_CHAR[nSize];
    if (pString == SIP_NULL)
    {
        return pString;
    }
    SipPf_Memset(pString, SIP_NULL, nSize);
    /*Leave the last point as NULL*/
    SipPf_Memcpy(pString, pStartPt,(nSize -SIP_ONE));
    return pString;
}

SIP_CHAR* sipSkipFwLWS(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    /*    LWS = [*WSP   CRLF]   1*WSP  ; linear whitespace  */

    if ((pStartPt == SIP_NULL) || (pEndPt == SIP_NULL))
    {
        return SIP_NULL;
    }

    SIP_CHAR* pBuffTemp = pStartPt;

    while ((pBuffTemp <= pEndPt) && IS_WSP(*pBuffTemp))
    {
        pBuffTemp++;
    }

    SIP_CHAR* pBuffNext = pBuffTemp + SIP_ONE;

    /*Allow one CRLF*/
    if ((pBuffNext < pEndPt) && IS_CRLF(*pBuffTemp, *pBuffNext))
    {
        pBuffTemp = pBuffTemp + SIP_TWO;
    }

    while ((pBuffTemp <= pEndPt) && IS_WSP(*pBuffTemp))
    {
        pBuffTemp++;
    }

    return pBuffTemp;
}

SIP_BOOL sipFindLWS(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc)
{
    SIP_CHAR* pNext1Pt = pStartPt + SIP_ONE;
    SIP_CHAR* pNext2Pt = pStartPt + SIP_TWO;

    //added for case aplphanum+space+alphanum+CRLF by Ajay Bukan on 17-jan-2013
    while (((pNext2Pt <= pEndPt) || (IS_WSP(*pStartPt))))
    {
        if ((IS_WSP(*pStartPt) ||
                ((IS_CRLF(*pStartPt, *pNext1Pt) && (IS_WSP(*pNext2Pt))))))
        {
            *ppTempLoc = pStartPt - SIP_ONE;
            return SIP_TRUE;
        }
        pStartPt++;
        pNext1Pt = pStartPt + SIP_ONE;
        pNext2Pt = pStartPt + SIP_TWO;
    }
    return SIP_FALSE;
}

SIP_BOOL sipFindDelimiter(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc,
        SIP_CHAR cDelimiter)
{
    while (pStartPt <= pEndPt)
    {
        if (*pStartPt == cDelimiter)
        {
            *ppTempLoc = pStartPt - SIP_ONE;
            return SIP_TRUE;
        }
        pStartPt = pStartPt + SIP_ONE;
    }
    return SIP_FALSE;
}

SIP_BOOL sipFindPreDelimiter(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc,
        SIP_CHAR cDelimiter)
{
    SIP_BOOL bDQuote = SIP_FALSE;
    SIP_CHAR* pPrevPt = pStartPt;

    while (pStartPt <= pEndPt)
    {
        if ((*pStartPt == DQUOTE) && (*pPrevPt != BACKSLASH))
        {
            bDQuote = (SIP_BOOL)!bDQuote;
        }

        if (*pStartPt == cDelimiter)
        {
            /*for the case of quoted string we have to
              prevent returning in case of " , " and " ; " */
            if (bDQuote == SIP_FALSE)
            {
                *ppTempLoc = pStartPt - SIP_ONE;
                return SIP_TRUE;
            }
        }
        pPrevPt = pStartPt;
        pStartPt = pStartPt + SIP_ONE;
    }
    return SIP_FALSE;
}

SIP_CHAR* SkipConsecutiveCRLFs(SIP_CHAR* pStartPt)
{
    while ((pStartPt != SIP_NULL && (pStartPt+1) != SIP_NULL)
        && (IS_CRLF(*pStartPt, *(pStartPt+1))))
    {
        pStartPt = pStartPt + SIP_TWO;
    }
    return pStartPt;
}
