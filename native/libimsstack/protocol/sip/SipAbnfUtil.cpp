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
#include "SipAbnfUtil.h"
#include "platform/SipMemory.h"

SIP_BOOL HasSpace(const SIP_CHAR* pszValue)
{
    while (*pszValue)
    {
        if (*pszValue == ' ')
        {
            return SIP_TRUE;
        }
        pszValue++;
    }
    return SIP_FALSE;
}

const SIP_CHAR* SipSkipRwWSP(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*NULL validation*/
    if ((pStartPt == SIP_NULL) || (pEndPt == SIP_NULL))
    {
        return SIP_NULL;
    }
    while ((pEndPt > pStartPt) && IS_WSP(*pEndPt))
    {
        pEndPt--;
    }
    return pEndPt;
}

SIP_BOOL SipFindActualPos(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
        const SIP_CHAR** ppTempPre, const SIP_CHAR** ppTempNext, const SIP_CHAR cDelimiter)
{
    SIP_BOOL bDQuote = SIP_FALSE;
    const SIP_CHAR* pPrevPt = pStartPt;
    const SIP_CHAR* pStartTemp = pStartPt;

    while (pStartPt <= pEndPt)
    {
        /*Preventing the case of Feature Prm and quoted text*/
        if ((*pStartPt == DQUOTE) && (*pPrevPt != BACKSLASH))
        {
            bDQuote = static_cast<SIP_BOOL>(!bDQuote);
        }
        if (*pStartPt == cDelimiter)
        {
            /*for the case of quoted string we have to
              prevent returning in case of " ," and " ;" */
            if (bDQuote == SIP_FALSE)
            {
                *ppTempPre = pStartPt - SIP_ONE;
                *ppTempNext = pStartPt + SIP_ONE;

                /*remove the LWS from Back*/
                if (cDelimiter != RIGHT_ANGLE)
                {
                    *ppTempPre = SipSkipRwLWS(pStartTemp, *ppTempPre);
                }
                /*remove the LWS from start*/
                if (cDelimiter != LEFT_ANGLE)
                {
                    *ppTempNext = SipSkipFwLWS(*ppTempNext, pEndPt);
                }

                /*Remove the LWS form both the side to get the actual pos*/
                return SIP_TRUE;
            }
        }
        pPrevPt = pStartPt;
        pStartPt = pStartPt + SIP_ONE;
    }
    return SIP_FALSE;
}

SIP_VOID SipEnc_UpdateCurrPos(IN_OUT SIP_CHAR** ppMsgBuffer)
{
    while (**ppMsgBuffer != '\0')
    {
        (*ppMsgBuffer)++;
    }
}

SIP_BOOL SipFindLWS(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR** ppTempLoc)
{
    const SIP_CHAR* pNext1Pt = pStartPt + SIP_ONE;
    const SIP_CHAR* pNext2Pt = pStartPt + SIP_TWO;

    while (((pNext2Pt <= pEndPt) || (IS_WSP(*pStartPt))))
    {
        if ((IS_WSP(*pStartPt) || ((IS_CRLF(*pStartPt, *pNext1Pt) && (IS_WSP(*pNext2Pt))))))
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

const SIP_CHAR* SipSkipRwLWS(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    if ((pStartPt == SIP_NULL) || (pEndPt == SIP_NULL))
    {
        return SIP_NULL;
    }

    const SIP_CHAR* pBuffTemp = pEndPt;

    while ((pBuffTemp > pStartPt) && IS_WSP(*pBuffTemp))
    {
        pBuffTemp--;
    }

    const SIP_CHAR* pBuffPrev = pBuffTemp - SIP_ONE;

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

const SIP_CHAR* SipSkipFwLWS(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*    LWS = [*WSP   CRLF]   1*WSP  ; linear whitespace  */

    if ((pStartPt == SIP_NULL) || (pEndPt == SIP_NULL))
    {
        return SIP_NULL;
    }

    const SIP_CHAR* pBuffTemp = pStartPt;

    while ((pBuffTemp <= pEndPt) && IS_WSP(*pBuffTemp))
    {
        pBuffTemp++;
    }

    const SIP_CHAR* pBuffNext = pBuffTemp + SIP_ONE;

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

SIP_BOOL SipFindPostDelimiter(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
        const SIP_CHAR** ppTempLoc, const SIP_CHAR cDelimiter)
{
    while (pStartPt <= pEndPt)
    {
        if (*pStartPt == cDelimiter)
        {
            *ppTempLoc = pStartPt + SIP_ONE;
            return SIP_TRUE;
        }
        pStartPt = pStartPt + SIP_ONE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipFindPreDelimiter(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
        const SIP_CHAR** ppTempLoc, const SIP_CHAR cDelimiter)
{
    SIP_BOOL bDQuote = SIP_FALSE;
    const SIP_CHAR* pPrevPt = pStartPt;

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

SIP_CHAR* SipCreateString(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
{
    /*Validate the input*/
    if (pEndPt < pStartPt)
    {
        return SIP_NULL;
    }
    /*Take one extraa memory for NULL*/
    SIP_UINT32 nSize = (pEndPt - pStartPt) + SIP_TWO;
    SIP_CHAR* pString = new SIP_CHAR[nSize];
    if (pString == SIP_NULL)
    {
        return pString;
    }
    SipPf_Memset(pString, SIP_NULL, nSize);
    /*Leave the last point as NULL*/
    SipPf_Memcpy(pString, pStartPt, (nSize - SIP_ONE));
    return pString;
}

SIP_BOOL SipFindCrlf(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR** ppTempLoc)
{
    const SIP_CHAR* pNext1Pt = pStartPt + SIP_ONE;
    while (pNext1Pt <= pEndPt)
    {
        if (IS_CRLF(*pStartPt, *pNext1Pt))
        {
            *ppTempLoc = pStartPt - SIP_ONE;
            return SIP_TRUE;
        }
        pStartPt++;
        pNext1Pt++;
    }
    return SIP_FALSE;
}

SIP_BOOL SipFindTerminatingCRLF(const SIP_CHAR* pStartPoint, const SIP_CHAR* pEndPoint,
        const SIP_CHAR** ppLocation, SIP_BOOL* pbHdrEnd)
{
    /* To check two consecutive CRLF */
    SIP_UINT32 nStrlen = pEndPoint - pStartPoint;
    if (nStrlen < SIP_THREE)
    {
        return SIP_FALSE;
    }
    const SIP_CHAR* pPrevPoint2 = pStartPoint;
    const SIP_CHAR* pPrevPoint1 = pStartPoint + SIP_ONE;
    pStartPoint = pStartPoint + SIP_TWO;
    /*Check for CRLF*/
    const SIP_CHAR* pNextPoint = pStartPoint + SIP_ONE;

    while (pNextPoint <= pEndPoint)
    {
        if (IS_CRLF(*pPrevPoint2, *pPrevPoint1) && (IS_WSP(*pStartPoint) == SIP_FALSE))
        {
            *ppLocation = pPrevPoint2 - SIP_ONE;
            if (IS_CRLF(*pStartPoint, *pNextPoint))
            {
                *pbHdrEnd = SIP_TRUE;
            }
            return SIP_TRUE;
        }
        pPrevPoint2 = pPrevPoint1;
        pPrevPoint1 = pStartPoint;
        pStartPoint++;
        pNextPoint = pStartPoint + SIP_ONE;
    }
    /*Check For one CRLF at Extream End*/
    if (IS_CRLF(*pPrevPoint2, *pPrevPoint1) && (IS_WSP(*pStartPoint) == SIP_FALSE))
    {
        *ppLocation = pPrevPoint2 - SIP_ONE;
        return SIP_TRUE;
    }

    if (IS_CRLF(*pPrevPoint1, *pStartPoint))
    {
        *ppLocation = pPrevPoint1 - SIP_ONE;
        return SIP_TRUE;
    }

    return SIP_FALSE;
}

const SIP_CHAR* SkipConsecutiveCRLFs(const SIP_CHAR* pStartPt)
{
    const SIP_CHAR* pNext = (pStartPt != SIP_NULL) ? (pStartPt + 1) : SIP_NULL;
    while ((pNext != SIP_NULL) && (IS_CRLF(*pStartPt, *pNext)))
    {
        pStartPt = pStartPt + SIP_TWO;
        pNext = (pStartPt != SIP_NULL) ? (pStartPt + 1) : SIP_NULL;
    }
    return pStartPt;
}
