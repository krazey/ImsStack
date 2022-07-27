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
#include "msg/sip_msgutil.h"
#include "sip_pf_datatypes.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "msg/SipHeaders.h"
#include "msg/SipMessage.h"
#include "msg/SipHeaderBase.h"

#define MAX__CONTACT_EXPIRES 4294967295
#define SIP_MAX_HDR_LEN      32

/******************************************************************************
 * Function name      : sipSkipRwWSP
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_CHAR* sipSkipRwWSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
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

/******************************************************************************
 * Function name      : sipFindPostDelimiter
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindPostDelimiter(
        SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc, SIP_CHAR cDelimiter)
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

/*****************************************************************************
 * Function name      : sipGetUriType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetUriType(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    SIP_UINT32 nSize = (pEndPt - pStartPt) + SIP_ONE;
    if (SipPf_Memcmp(pStartPt, SIP_SIP, nSize) == 0)
    {
        return SipUri::SCHEME_SIP;
    }
    else if (SipPf_Memcmp(pStartPt, SIP_SIPS, nSize) == 0)
    {
        return SipUri::SCHEME_SIPS;
    }
    return SipUri::SCHEME_ABS;
}

/*****************************************************************************
 * Function name      : sipGetHdrType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetHdrType(const SIP_CHAR* pszHdrName)
{
    return SIPHdrAccess::GetHdrType(pszHdrName);
}

/*****************************************************************************
 * Function name      : CheckAndGetHdrEnumType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 CheckAndGetHdrEnumType(SIP_INT32 nType)
{
    // support EXPIRES_ANY & EXPIRES_DATE
    if ((nType == SipHeaderBase::EXPIRES_ANY) || (nType == SipHeaderBase::EXPIRES_DATE))
    {
        nType = SipHeaderBase::EXPIRES_SEC;
    }  // support CONTACT_ANY & CONTACT_WILD
    else if ((nType == SipHeaderBase::CONTACT_ANY) || (nType == SipHeaderBase::CONTACT_WILD))
    {
        nType = SipHeaderBase::CONTACT;
    }  // Support for Retry-After Any & Sec header
    else if ((nType == SipHeaderBase::RETRY_AFTER_ANY) ||
            (nType == SipHeaderBase::RETRY_AFTER_DATE))
    {
        nType = SipHeaderBase::RETRY_AFTER_SEC;
    }

    return nType;
}

#ifdef SIP_STRICT_PARSING
SIP_BOOL IsValidAddress(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    SIP_CHAR* pTempLoc = SIP_NULL;
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;

    /*Find the Start of header parm*/
    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTempLoc, QMARK) == SIP_FALSE)
    {
        return SIP_TRUE;
    }

    pStartPt = pTempLoc;
    pTempLoc = SIP_NULL;

    if (sipFindPostDelimiter(pStartPt, pEndPt, &pTempLoc, PERCENT) == SIP_FALSE)
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}
#endif
