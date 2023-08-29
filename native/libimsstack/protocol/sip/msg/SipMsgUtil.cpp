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
#include "platform/SipMemory.h"
#include "platform/SipString.h"
#include "msg/SipMsgUtil.h"
#include "msg/SipMessage.h"
#include <ctype.h>

#define NAME_CONTENT_TRANSFER_ENCODING "Content-Transfer-Encoding"

// clang-format off
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
// clang-format on

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

struct HdrNameType
{
    SIP_INT32 HdrType;
    SIP_CHAR HdrName[SIP_MAX_HDR_LEN];
};

struct HdrLenRecord
{
    SIP_INT16 Hdrlen;
    SIP_INT16 NoOfEntries;
    HdrNameType objHeaders[SIP_MAX_HDR_LEN];
};

static HdrLenRecord* s_pHdrLenRecord = SIP_NULL;

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

SIP_INT32 SipGetMsgType(SIP_CHAR* pszStartPoint)
{
    return (SipPf_Strncmp(SIP_SIPVER, pszStartPoint, SIP_FOUR) == 0) ? SipMessage::RESP_TYPE
                                                                     : SipMessage::REQ_TYPE;
}

SIP_INT32 SipGetUriType(SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt)
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

SIP_INT32 SipGetHdrType(const SIP_CHAR* pszHdrName)
{
    return SIPHdrAccess::GetHdrType(pszHdrName);
}

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
    if (SipFindPostDelimiter(pStartPt, pEndPt, &pTempLoc, QMARK) == SIP_FALSE)
    {
        return SIP_TRUE;
    }

    pStartPt = pTempLoc;
    pTempLoc = SIP_NULL;

    if (SipFindPostDelimiter(pStartPt, pEndPt, &pTempLoc, PERCENT) == SIP_FALSE)
    {
        return SIP_TRUE;
    }
    return SIP_FALSE;
}
#endif

SIP_CHAR* SipFindBodyEnd(
        SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, SIP_CHAR* pszBoundary, SIP_BOOL& bBodyEnd)
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

SIP_INT32 SipGetMimeHdrType(SIP_CHAR* pszHdrName)
{
    if (pszHdrName == SIP_NULL)
    {
        return SipHeaderBase::UNKNOWN;
    }

    switch (pszHdrName[0])
    {
        case 'c':
        case 'C':
            if ((SipPf_Stricmp(pszHdrName, "c") == 0) ||
                    (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_TYPE]) == 0))
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
            else if ((SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_ENCODING]) ==
                             0) ||
                    (SipPf_Stricmp(pszHdrName, NAME_CONTENT_TRANSFER_ENCODING) == 0))
            {
                return SipHeaderBase::CONTENT_ENCODING;
            }
            else if (SipPf_Stricmp(pszHdrName, gaszSipHdr[SipHeaderBase::CONTENT_LANGUAGE]) == 0)
            {
                return SipHeaderBase::CONTENT_LANGUAGE;
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

void SIPHdrAccess::Init()
{
    if (s_pHdrLenRecord != SIP_NULL)
    {
        return;
    }

    s_pHdrLenRecord = new HdrLenRecord[SIP_MAX_HDR_LEN];
    memset(s_pHdrLenRecord, 0, sizeof(HdrLenRecord) * SIP_MAX_HDR_LEN);

    for (SIP_INT32 nHdrLenIndex = SIP_ZERO; nHdrLenIndex < SIP_MAX_HDR_LEN; nHdrLenIndex++)
    {
        SIP_INT32 nNoOfHdr = SIP_ZERO;
        s_pHdrLenRecord[nHdrLenIndex].NoOfEntries = SIP_ZERO;
        s_pHdrLenRecord[nHdrLenIndex].Hdrlen = nHdrLenIndex;

        for (SIP_INT32 nHdrIndex = SIP_ZERO; nHdrIndex < SipHeaderBase::TYPE_END; nHdrIndex++)
        {
            if (SipPf_Strlen(gaszSipHdr[nHdrIndex]) == nHdrLenIndex)
            {
                s_pHdrLenRecord[nHdrLenIndex].NoOfEntries++;
                s_pHdrLenRecord[nHdrLenIndex].objHeaders[nNoOfHdr].HdrType = nHdrIndex;

                SipPf_Memset(s_pHdrLenRecord[nHdrLenIndex].objHeaders[nNoOfHdr].HdrName, 0,
                        SIP_MAX_HDR_LEN);

                SipPf_Strncpy(s_pHdrLenRecord[nHdrLenIndex].objHeaders[nNoOfHdr].HdrName,
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
    if (SipPf_Strnicmp(pszRcvdHdrName, "Content", SIP_SEVEN) == SIP_ZERO)
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
    else if (SipPf_Strnicmp(pszRcvdHdrName, "Expires", SIP_SEVEN) == SIP_ZERO)
    {
        return SipHeaderBase::EXPIRES_SEC;
    }
    else if (SipPf_Strnicmp(pszRcvdHdrName, "Retry-After", SIP_11) == SIP_ZERO)
    {
        return SipHeaderBase::RETRY_AFTER_SEC;
    }

    if (s_pHdrLenRecord == SIP_NULL)
    {
        return SipHeaderBase::UNKNOWN;
    }

    for (SIP_INT32 nNoOfHdr = SIP_ZERO; nNoOfHdr < s_pHdrLenRecord[nlen].NoOfEntries; nNoOfHdr++)
    {
        if (SipPf_Stricmp(s_pHdrLenRecord[nlen].objHeaders[nNoOfHdr].HdrName, pszRcvdHdrName) ==
                SIP_ZERO)
        {
            return s_pHdrLenRecord[nlen].objHeaders[nNoOfHdr].HdrType;
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

    const SIP_CHAR* psztemp = gaszSipHdrCompact;
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
