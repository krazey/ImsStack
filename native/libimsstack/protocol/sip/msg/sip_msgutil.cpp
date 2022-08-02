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
#include "sip_debug.h"
#include "platform/sip_pf_memory.h"
#include "platform/sip_pf_string.h"
#include "msg/sip_msgutil.h"
#include "msg/SipMessage.h"
#include <ctype.h>

#define NAME_CONTENT_TRANSFER_ENCODING "Content-Transfer-Encoding"

SIP_CHAR gaszSipHdr[][SIP_MAX_HDR_LEN] = {
        "Allow",  // 0
        "Allow-Events",
        "Authorization",
        "Call-ID",
        "Contact",
        "Contact",
        "Contact",
        "Content-Disposition",
        "Content-Encoding",
        "Content-Length",
        "Content-Type",  // 10
        "CSeq",
        "Event",
        "Expires",
        "Expires",
        "Expires",
        "Accept",
        "Min-Expires",
        "From",
        "Max-Forwards",
        "MIME-Version",  // 20
        "Privacy",
        "P-Preferred-Identity",
        "P-Asserted-Identity",
        "Min-SE",
        "Path",
        "P-Associated-URI",
        "P-Called-Party-ID",
        "P-Visited-Network-ID",
        "P-Charging-Function-Addresses",
        "P-Access-Network-Info",  // 30
        "P-Charging-Vector",
        "Service-Route",
        "History-Info",
        "Request-Disposition",
        "Accept-Contact",
        "Reject-Contact",
        "Join",
        "SIP-If-Match",
        "SIP-ETag",
        "Proxy-Authenticate",  // 40
        "Proxy-Authorization",
        "RAck",
        "Record-Route",
        "Referred-By",
        "Refer-To",
        "Replaces",
        "Require",
        "Route",
        "RSeq",
        "Security-Client",  // 50
        "Security-Verify",
        "Security-Server",
        "Session-Expires",
        "Subscription-State",
        "Supported",
        "Timestamp",
        "To",
        "Unsupported",
        "Via",
        "Warning",  // 60
        "WWW-Authenticate",
        "Unknown",
        "Retry-After",
        "Retry-After",
        "Retry-After",
        "P-Early-Media",
        "Resource-Priority",
        "Accept-Resource-Priority",
        "Date",
        "Accept-Encoding",  // 70
        "Accept-Language",
        "Alert-Info",
        "Answer-Mode",
        "Authentication-Info",
        "Call-Info",
        "Content-Language",
        "Error-Info",
        "Flow-Timer",
        "Identity",
        "Identity-Info",  // 80
        "In-Reply-To",
        "Organization",
        "P-Answer-State",
        "Permission-Missing",
        "P-Media-Authorization",
        "P-Profile-Key",
        "P-Refused-URI-List",
        "Priority",
        "Priv-Answer-Mode",
        "Proxy-Require",  // 90
        "P-Served-User",
        "P-User-Database",
        "Reason",
        "Refer-Sub",
        "Reply-To",
        "Response-Key",
        "Server",
        "Subject",
        "Suppress-If-Match",
        "Target-Dialog",  // 100
        "Trigger-Consent",
        "User-Agent",
        "Feature-Caps",
        "Geolocation",
        "Geolocation-Error",
        "Geolocation-Routing",
        "Info-Package",
        "Max-Breadth",
        "P-Asserted-Service",
        "Policy-Contact",  // 110
        "Policy-ID",
        "P-Preferred-Service",
        "Recv-Info",
        "Session-ID",
        "UNKNOWN",  // 115
};

const SIP_CHAR* gaszSipContentHdr[SIP_CONTENT_HDRS_LEN] = {
        "Content-Type",        /*CONTENT_TYPE*/
        "Content-Disposition", /*CONTENT_DISPOSITION*/
        "Content-Encoding",    /*CONTENT_TRANSFER_ENCODING*/
        "Content-ID",          /*CONTENT_ID*/
        "Content-Description"  /*CONTENT_DESCRIPTION*/
};

const SIP_CHAR gaszSipHdrCompact[21] = "abcdefijklmnorstuvxy";
const SIP_INT16 gaszSipHdrCompactEnum[20] = {SipHeaderBase::ACCEPT_CONTACT,
        SipHeaderBase::REFERRED_BY, SipHeaderBase::CONTENT_TYPE, SipHeaderBase::REQUEST_DISPOSITION,
        SipHeaderBase::CONTENT_ENCODING, SipHeaderBase::FROM, SipHeaderBase::CALL_ID,
        SipHeaderBase::REJECT_CONTACT, SipHeaderBase::SUPPORTED, SipHeaderBase::CONTENT_LENGTH,
        SipHeaderBase::CONTACT, SipHeaderBase::IDENTITY_INFO, SipHeaderBase::EVENT,
        SipHeaderBase::REFER_TO, SipHeaderBase::SUBJECT, SipHeaderBase::TO,
        SipHeaderBase::ALLOW_EVENTS, SipHeaderBase::VIA, SipHeaderBase::SESSION_EXPIRES,
        SipHeaderBase::IDENTITY};

/*****************************************************************************
 * Function name      : SetCharVar
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_BOOL SetCharVar(const SIP_CHAR* pszSource, SIP_CHAR*& pszDestination)
{
    if (pszSource == SIP_NULL)
    {
        return SIP_FALSE;
    }

    if (pszDestination != SIP_NULL)
    {
        delete[] pszDestination;
    }

    pszDestination = SipPf_Strdup(pszSource);
    if (pszDestination == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

/*****************************************************************************
 * Function name      : HasSpace
 *
 * Description        : Returns SUCCESS if the character string has Space
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
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

/*****************************************************************************
 * Function name      : sipGetMsgType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetMsgType(SIP_CHAR* pszStartPoint)
{
    return (SipPf_Strncmp(SIP_SIPVER, pszStartPoint, SIP_FOUR) == 0) ? SipMessage::RESP_TYPE
                                                                     : SipMessage::REQ_TYPE;
}

/*****************************************************************************
 * Function name      : sipFindTerminatingCRLF
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_BOOL sipFindTerminatingCRLF(
        SIP_CHAR* pStartPoint, SIP_CHAR* pEndPoint, SIP_CHAR** ppLocation, SIP_BOOL* pbHdrEnd)
{
    /* To check two consecutive CRLF */
    SIP_UINT32 nStrlen = pEndPoint - pStartPoint;
    if (nStrlen < SIP_THREE)
    {
        return SIP_FALSE;
    }
    SIP_CHAR* pPrevPoint2 = pStartPoint;
    SIP_CHAR* pPrevPoint1 = pStartPoint + SIP_ONE;
    pStartPoint = pStartPoint + SIP_TWO;
    /*Check for CRLF*/
    SIP_CHAR* pNextPoint = pStartPoint + SIP_ONE;

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

/******************************************************************************
 * Function name      : sipFindActualPos
 *
 * Description     : this Api will find the delimiter and Remove LWS form
 *                    both the Side
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindActualPos(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempPre,
        SIP_CHAR** ppTempNext, SIP_CHAR cDelimiter)
{
    SIP_BOOL bDQuote = SIP_FALSE;
    SIP_CHAR* pPrevPt = pStartPt;
    SIP_CHAR* pStartTemp = pStartPt;

    while (pStartPt <= pEndPt)
    {
        /*Preventing the case of Feature Prm and quoted text*/
        if ((*pStartPt == DQUOTE) && (*pPrevPt != BACKSLASH))
        {
            bDQuote = (SIP_BOOL)!bDQuote;
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
                    *ppTempPre = sipSkipRwLWS(pStartTemp, *ppTempPre);
                }
                /*remove the LWS from start*/
                if (cDelimiter != LEFT_ANGLE)
                {
                    *ppTempNext = sipSkipFwLWS(*ppTempNext, pEndPt);
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

/******************************************************************************
 * Function name  : SipEnc_UpdateCurrPos
 * Description     :  This api will update the current position of the sip msg
 *****************************************************************************/
SIP_VOID SipEnc_UpdateCurrPos(IN_OUT SIP_CHAR** ppMsgBuffer)
{
    while (**ppMsgBuffer != '\0')
    {
        (*ppMsgBuffer)++;
    }
}

/******************************************************************************
 * Function name      : sipFindCRLF
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL sipFindCrlf(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc)
{
    SIP_CHAR* pNext1Pt = pStartPt + SIP_ONE;
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

/******************************************************************************
 * Function name      : sipFindBodyEnd
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_CHAR* sipFindBodyEnd(
        SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR* pszBoundary, SIP_BOOL& bBodyEnd)
{
    if (pStartPt == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_UINT16 nBoundaryLen = SipPf_Strlen(pszBoundary);
    SIP_CHAR* pNextPt = pStartPt + SIP_ONE;
    SIP_CHAR* pTempEndPt = pStartPt + nBoundaryLen + SIP_TWO;
    SIP_CHAR* pEndNext = pTempEndPt + SIP_ONE;

    while (pEndNext <= pEndPt)
    {
        if (IS_HYPHEN(*pStartPt) && IS_HYPHEN(*pNextPt))
        {
            SIP_CHAR* pTempStartPt = pStartPt + SIP_TWO;
            if (SipPf_Strncmp(pTempStartPt, pszBoundary, nBoundaryLen) == SIP_ZERO)
            {
                if (IS_HYPHEN(*pTempEndPt) && IS_HYPHEN(*pEndNext))
                {
                    bBodyEnd = SIP_TRUE;
                }

                // Remove preceding CRLF: CRLF--boundary
                // Start pointer: first '-'
                return (pStartPt - SIP_TWO);
            }
        }
        pStartPt++;
        pNextPt = pStartPt + SIP_ONE;
        pTempEndPt = pStartPt + nBoundaryLen + SIP_TWO;
        pEndNext = pTempEndPt + SIP_ONE;
    }
    return SIP_NULL;
}

/*****************************************************************************
 * Function name      : sipGetMIMEHdrType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetMimeHdrType(SIP_CHAR* pszHdrName)
{
    if (pszHdrName == SIP_NULL)
    {
        return SipHeaderBase::UNKNOWN;
    }

    switch (pszHdrName[0])
    {
        case 'c':
        case 'C':
            if (SipPf_Stricmp(pszHdrName, "c") == 0)
            {
                return SipHeaderBase::CONTENT_TYPE;
            }
            else if (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_TYPE]) == 0)
            {
                return SipHeaderBase::CONTENT_TYPE;
            }
            else if (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_LENGTH]) == 0)
            {
                return SipHeaderBase::CONTENT_LENGTH;
            }
            else if (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_DISPOSITION]) == 0)
            {
                return SipHeaderBase::CONTENT_DISPOSITION;
            }
            else if (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_ENCODING]) == 0)
            {
                return SipHeaderBase::CONTENT_ENCODING;
            }
            else if (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_LANGUAGE]) == 0)
            {
                return SipHeaderBase::CONTENT_LANGUAGE;
            }
            else if (SipPf_Stricmp(pszHdrName, NAME_CONTENT_TRANSFER_ENCODING) == 0)
            {
                return SipHeaderBase::CONTENT_ENCODING;
            }
            break;

            /*Apply same for all other headers*/

        default:
            break;

            /*treat as unknown header*/
    }
    return SipHeaderBase::UNKNOWN;
    /* go for unknown header check*/
}

static HdrLenRecord s_objHdrLenRecord[SIP_MAX_HDR_LEN] = {
        {0, 0, {{0, {0}}}}
};

void SIPHdrAccess::Init()
{
    memset(s_objHdrLenRecord, 0, sizeof(s_objHdrLenRecord));
    for (SIP_INT32 nHdrLenIndex = SIP_ZERO; nHdrLenIndex < SIP_MAX_HDR_LEN; nHdrLenIndex++)
    {
        SIP_INT32 nNoOfHdr = SIP_ZERO;
        s_objHdrLenRecord[nHdrLenIndex].NoOfEntries = SIP_ZERO;
        s_objHdrLenRecord[nHdrLenIndex].Hdrlen = nHdrLenIndex;

        for (SIP_INT32 nHdrIndex = SIP_ZERO; nHdrIndex < SipHeaderBase::TYPE_END; nHdrIndex++)
        {
            if (SipPf_Strlen(gaszSipHdr[nHdrIndex]) == nHdrLenIndex)
            {
                s_objHdrLenRecord[nHdrLenIndex].NoOfEntries++;
                s_objHdrLenRecord[nHdrLenIndex].objHeaders[nNoOfHdr].HdrType = nHdrIndex;

                SipPf_Memset(s_objHdrLenRecord[nHdrLenIndex].objHeaders[nNoOfHdr].HdrName, 0,
                        SIP_MAX_HDR_LEN);

                SipPf_Strncpy(s_objHdrLenRecord[nHdrLenIndex].objHeaders[nNoOfHdr].HdrName,
                        gaszSipHdr[nHdrIndex], SipPf_Strlen(gaszSipHdr[nHdrIndex]));
                nNoOfHdr++;
            }
        }
    }
}

SIP_INT32 SIPHdrAccess::GetHdrType(const SIP_CHAR* pszRcvdHdrName)
{
    if (pszRcvdHdrName == SIP_NULL)
    {
        return SipHeaderBase::TYPE_INVALID;
    }

    SIP_INT32 nlen = SipPf_Strlen(pszRcvdHdrName);
    if (nlen >= SIP_MAX_HDR_LEN)
    {
        return SipHeaderBase::UNKNOWN;
    }
    else if (nlen == SIP_ONE)
    {
        return GetHdrTypeCompact(pszRcvdHdrName[0]);
    }

    /*Content header are separately parsed based Content headers array gaszSipContentHdr
      and treated as known headers */
    if (SipPf_Strnicmp(pszRcvdHdrName, (SIP_CHAR*)"Content", SIP_SEVEN) == SIP_ZERO)
    {
        SIP_BOOL isContHdrFound = SIP_FALSE;
        for (SIP_INT32 nNContHdr = SIP_ZERO; nNContHdr < SIP_CONTENT_HDRS_LEN; nNContHdr++)
        {
            if (SipPf_Stricmp(gaszSipContentHdr[nNContHdr], pszRcvdHdrName) == SIP_ZERO)
            {
                isContHdrFound = SIP_TRUE;
                break;
            }
        }
        // Other Content headers treated as unknown headers like Content-Length.
        if (isContHdrFound == SIP_FALSE)
        {
            return SipHeaderBase::UNKNOWN;
        }
    }  // Conversion for Expires / Retry-After Headers
    else if (SipPf_Strnicmp(pszRcvdHdrName, (SIP_CHAR*)"Expires", SIP_SEVEN) == SIP_ZERO)
    {
        return SipHeaderBase::EXPIRES_SEC;
    }
    else if (SipPf_Strnicmp(pszRcvdHdrName, (SIP_CHAR*)"Retry-After", SIP_11) == SIP_ZERO)
    {
        return SipHeaderBase::RETRY_AFTER_SEC;
    }
    else if (SipPf_Strnicmp(pszRcvdHdrName, (SIP_CHAR*)"Feature-Caps", SIP_12) == SIP_ZERO)
    {
        return SipHeaderBase::UNKNOWN;
    }

    for (SIP_INT32 nNoOfHdr = SIP_ZERO; nNoOfHdr < s_objHdrLenRecord[nlen].NoOfEntries; nNoOfHdr++)
    {
        if (SipPf_Stricmp(s_objHdrLenRecord[nlen].objHeaders[nNoOfHdr].HdrName, pszRcvdHdrName) ==
                SIP_ZERO)
        {
            return s_objHdrLenRecord[nlen].objHeaders[nNoOfHdr].HdrType;
        }
    }
    return SipHeaderBase::UNKNOWN;
}

SIP_INT32 SIPHdrAccess::GetHdrTypeCompact(SIP_CHAR RcvdHdrName)
{
    SIP_CHAR lowHdrName = tolower(RcvdHdrName);

    /*Content-Length (l) header to be considered as unknown header to synch with Engine.*/
    /*Other content headers which has compact form (type - c & encoding - e) are known headers in
     * engine*/
    if (lowHdrName == 'l')
    {
        return SipHeaderBase::UNKNOWN;
    }

    SIP_CHAR* psztemp = (SIP_CHAR*)gaszSipHdrCompact;
    for (SIP_INT32 i = 0; (*psztemp != '\0'); i++)
    {
        if (*psztemp == lowHdrName)
        {
            return gaszSipHdrCompactEnum[i];
        }
        psztemp++;
    }
    return SipHeaderBase::UNKNOWN;
}
