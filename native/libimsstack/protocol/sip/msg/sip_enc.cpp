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
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "msg/SipHeaders.h"

SIP_CHAR* SipPercentEncoding::DoPercentDecoding(SIP_CHAR* pszString)
{
    SIP_INT32 nLength = SipPf_Strlen(pszString);

    SIP_CHAR* pszDecodedString = new SIP_CHAR[(nLength + SIP_ONE)];
    SipPf_Memset(pszDecodedString, SIP_NULL, (nLength + SIP_ONE));
    SIP_CHAR* pszReturnString = pszDecodedString;
    SIP_CHAR* pszStringTemp = pszString;
    SIP_BOOL bIsPercentDecoded = SIP_FALSE;
    while (*pszStringTemp != SIP_NULL)
    {
        /*Search for the % char*/
        if ((*pszStringTemp == PERCENT) && (nLength >= SIP_THREE))
        {
            SIP_INT32 nConvertedChar = SIP_ZERO;
            SipPf_Sscanf(pszStringTemp + SIP_ONE, (SIP_CHAR*)"%2X", (SIP_CHAR*)&nConvertedChar);
            SipPf_Sprintf(pszDecodedString, (SIP_CHAR*)"%c", (SIP_CHAR)nConvertedChar);
            nLength = nLength - SIP_THREE;
            pszStringTemp = pszStringTemp + SIP_THREE;
            bIsPercentDecoded = SIP_TRUE;
        }
        else
        {
            *pszDecodedString = *pszStringTemp;
            pszStringTemp++;
            nLength--;
        }
        pszDecodedString = pszDecodedString + SIP_ONE;
    }

    delete[] pszString;

    if (bIsPercentDecoded == SIP_TRUE)
    {
        nLength = SipPf_Strlen(pszReturnString);
        pszDecodedString = new SIP_CHAR[(nLength + SIP_ONE)];
        SipPf_Memset(pszDecodedString, SIP_NULL, (nLength));
        SipPf_Strcpy(pszDecodedString, pszReturnString);
        delete[] pszReturnString;
    }
    else
    {
        pszDecodedString = pszReturnString;
    }

    return pszDecodedString;
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_UserAndHeader(SIP_CHAR* pszString, SIP_CHAR* pType)
{
    /**
     * user = 1*( unreserved / escaped / user-unreserved )
     * unreserved = alphanum / mark
     * mark = "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
     * escaped = "%"   HEXDIG   HEXDIG
     * user-unreserved = "&" / "=" / "+" / "$" / "," / ";" / "?" / "/"
     *
     * header 1*( hnv-unreserved / unreserved / escaped )
     * hnv-unreserved = "[" / "]" / "/" / "?" / ":" / "+" / "$"
     */
    SIP_CHAR* pCurrPt = pszString;
    SIP_INT32 nLength = SipPf_Strlen(pszString);
    SIP_CHAR* pEndPt = pCurrPt + nLength;

    SIP_CHAR* pEncodedString = new SIP_CHAR[(3 * nLength)];
    SipPf_Memset(pEncodedString, SIP_NULL, (3 * nLength));
    SIP_CHAR* pszReturnString = pEncodedString;

    while (*(pCurrPt) != SIP_NULL)
    {
        if (IS_UNRESERVED(*pCurrPt) ||
                ((SipPf_Strcmp(pType, (SIP_CHAR*)SIP_USER) == 0) && IS_USER_UNRESERVED(*pCurrPt)) ||
                ((SipPf_Strcmp(pType, (SIP_CHAR*)SIP_HEADERS) == 0) && IS_HNV_UNRESERVED(*pCurrPt)))
        {
            *pEncodedString = *pCurrPt;
            pCurrPt++;
            pEncodedString++;
        }
        else if ((pCurrPt + SIP_TWO <= pEndPt) &&
                (IS_ESCAPED(*pCurrPt, *(pCurrPt + SIP_ONE), *(pCurrPt + SIP_TWO))))
        {
            *pEncodedString = *pCurrPt;
            *(pEncodedString + SIP_ONE) = *(pCurrPt + SIP_ONE);
            *(pEncodedString + SIP_TWO) = *(pCurrPt + SIP_TWO);
            pCurrPt += 3;
            pEncodedString += 3;
        }
        else
        {
            *pEncodedString = PERCENT;
            pEncodedString++;
            SipPf_Sprintf(pEncodedString, (SIP_CHAR*)"%02X", *pCurrPt);
            SipEnc_UpdateCurrPos(&pEncodedString);
            pCurrPt++;
        }
    }
    return pszReturnString;
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_Password(SIP_CHAR* pszString)
{
    /**
     * password = *( unreserved / escaped / "&" / "=" / "+" / "$"/ "," )
     * * unreserved = alphanum / mark
     * mark = "-" / "_" / "." / "!" / "~" / "*" / "'" / "(" / ")"
     * escaped = "%"   HEXDIG   HEXDIG
     */
    SIP_CHAR* pCurrPt = pszString;
    SIP_INT32 nLength = SipPf_Strlen(pszString);
    SIP_CHAR* pEndPt = pCurrPt + nLength;

    SIP_CHAR* pEncodedString = new SIP_CHAR[(3 * nLength)];
    SipPf_Memset(pEncodedString, SIP_NULL, (3 * nLength));
    SIP_CHAR* pszReturnString = pEncodedString;

    while (*(pCurrPt) != SIP_NULL)
    {
        if (IS_UNRESERVED(*pCurrPt) || IS_AMPERSAND(*pCurrPt) || IS_EQUALS(*pCurrPt) ||
                IS_PLUS(*pCurrPt) || IS_DOLLAR(*pCurrPt) || IS_COMMA(*pCurrPt))
        {
            *pEncodedString = *pCurrPt;
            pCurrPt++;
            pEncodedString++;
        }
        else if ((pCurrPt + SIP_TWO <= pEndPt) &&
                (IS_ESCAPED(*pCurrPt, *(pCurrPt + SIP_ONE), *(pCurrPt + SIP_TWO))))
        {
            *pEncodedString = *pCurrPt;
            *(pEncodedString + SIP_ONE) = *(pCurrPt + SIP_ONE);
            *(pEncodedString + SIP_TWO) = *(pCurrPt + SIP_TWO);
            pCurrPt += 3;
            pEncodedString += 3;
        }
        else
        {
            *pEncodedString = PERCENT;
            pEncodedString++;
            SipPf_Sprintf(pEncodedString, (SIP_CHAR*)"%02X", *pCurrPt);
            SipEnc_UpdateCurrPos(&pEncodedString);
            pCurrPt++;
        }
    }
    return pszReturnString;
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_Host(SIP_CHAR* pszString)
{
    // host = hostname /  IPv4address /  IPv6reference
    // host : 1*(ALPHANUM , "." , ":" , "[" , "]" , "-")
    SIP_CHAR* pCurrPt = pszString;
    SIP_INT32 nLength = SipPf_Strlen(pszString);

    SIP_CHAR* pEncodedString = new SIP_CHAR[(3 * nLength)];
    SipPf_Memset(pEncodedString, SIP_NULL, (3 * nLength));
    SIP_CHAR* pszReturnString = pEncodedString;

    while (*(pCurrPt) != SIP_NULL)
    {
        if (IS_ALPHANUM(*pCurrPt) || IS_DOT(*pCurrPt) || IS_COLON(*pCurrPt) ||
                IS_LSQURE(*pCurrPt) || IS_RSQURE(*pCurrPt) || IS_HYPHEN(*pCurrPt))
        {
            *pEncodedString = *pCurrPt;
            pCurrPt++;
            pEncodedString++;
        }
        else
        {
            *pEncodedString = PERCENT;
            pEncodedString++;
            SipPf_Sprintf(pEncodedString, (SIP_CHAR*)"%02X", *pCurrPt);
            SipEnc_UpdateCurrPos(&pEncodedString);
            pCurrPt++;
        }
    }
    return pszReturnString;
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_TokenParam(SIP_CHAR* pszString)
{
    // token = 1*( alphanum / "-" / "." / "!" / "%" / "*" / "_" / "+" / "`" / "'" / "~" )
    SIP_CHAR* pCurrPt = pszString;
    SIP_INT32 nLength = SipPf_Strlen(pszString);

    SIP_CHAR* pEncodedString = new SIP_CHAR[(3 * nLength)];
    SipPf_Memset(pEncodedString, SIP_NULL, (3 * nLength));
    SIP_CHAR* pszReturnString = pEncodedString;

    while (*(pCurrPt) != SIP_NULL)
    {
        if (IS_TOKEN(*pCurrPt))
        {
            *pEncodedString = *pCurrPt;
            pCurrPt++;
            pEncodedString++;
        }
        else
        {
            *pEncodedString = PERCENT;
            pEncodedString++;
            SipPf_Sprintf(pEncodedString, (SIP_CHAR*)"%02X", *pCurrPt);
            SipEnc_UpdateCurrPos(&pEncodedString);
            pCurrPt++;
        }
    }
    return pszReturnString;
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_TtlParam(SIP_CHAR* pszString)
{
    // ttl = 1*3DIGIT     ; 0 to 255
    SIP_CHAR* pCurrPt = pszString;
    SIP_INT32 nLength = SipPf_Strlen(pszString);

    SIP_CHAR* pEncodedString = new SIP_CHAR[(3 * nLength)];
    SipPf_Memset(pEncodedString, SIP_NULL, (3 * nLength));
    SIP_CHAR* pszReturnString = pEncodedString;

    while (*(pCurrPt) != SIP_NULL)
    {
        if (IS_DIGIT(*pCurrPt))
        {
            *pEncodedString = *pCurrPt;
            pCurrPt++;
            pEncodedString++;
        }
        else
        {
            *pEncodedString = PERCENT;
            pEncodedString++;
            SipPf_Sprintf(pEncodedString, (SIP_CHAR*)"%02X", *pCurrPt);
            SipEnc_UpdateCurrPos(&pEncodedString);
            pCurrPt++;
        }
    }
    return pszReturnString;
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_MddrParam(SIP_CHAR* pszString)
{
    // "maddr="   host
    return DoPerEnc_Host(pszString);
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_OtherParam(SIP_CHAR* pszString)
{
    // other-param : ( param-unreserved / unreserved /  escaped )
    // param-unreserved = "[" / "]" / "/" / ":" / "&" / "+"/ "$"
    return DoPerEnc_TokenParam(pszString);
}

SIP_CHAR* SipPercentEncoding::DoPerEnc_Param(SIP_CHAR* pszName, SIP_CHAR* pszValue)
{
    if (SipPf_Stricmp(pszName, SIP_USER_PRM) == 0)
    {
        return DoPerEnc_TokenParam(pszValue);
    }
    else if (SipPf_Stricmp(pszName, SIP_METHOD) == 0)
    {
        return DoPerEnc_TokenParam(pszValue);
    }
    else if (SipPf_Stricmp(pszName, SIP_MADDR_PRM) == 0)
    {
        return DoPerEnc_MddrParam(pszValue);
    }
    else if (SipPf_Stricmp(pszName, SIP_TTL_PRM) == 0)
    {
        return DoPerEnc_TtlParam(pszValue);
    }
    else if (SipPf_Stricmp(pszName, SIP_TRNSPORT_PRM) == 0)
    {
        return DoPerEnc_TokenParam(pszValue);
    }
    else if (SipPf_Stricmp(pszName, SIP_LR_PRM) == 0)
    {
        return DoPerEnc_TokenParam(pszValue);
    }
    else if (SipPf_Stricmp(pszName, SIP_HEADERS) == 0)
    {
        return DoPerEnc_UserAndHeader(pszValue, (SIP_CHAR*)SIP_HEADERS);
    }
    return DoPerEnc_OtherParam(pszValue);
}
