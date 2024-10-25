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
#ifndef __SIP_MSG_UTIL_H__
#define __SIP_MSG_UTIL_H__

#include "SipAbnfUtil.h"
#include "SipDatatypes.h"
#include "SipAddrSpec.h"

class SipMsgUtil
{
public:
    static const SIP_CHAR SIP_VERSION[];

    static const SIP_CHAR METHOD_INVITE[];
    static const SIP_CHAR METHOD_ACK[];
    static const SIP_CHAR METHOD_OPTION[];
    static const SIP_CHAR METHOD_BYE[];
    static const SIP_CHAR METHOD_CANCEL[];
    static const SIP_CHAR METHOD_REGISTER[];
    static const SIP_CHAR METHOD_INFO[];
    static const SIP_CHAR METHOD_PRACK[];
    static const SIP_CHAR METHOD_SUBSCRIBE[];
    static const SIP_CHAR METHOD_NOTIFY[];
    static const SIP_CHAR METHOD_UPDATE[];
    static const SIP_CHAR METHOD_MESSAGE[];
    static const SIP_CHAR METHOD_REFER[];
    static const SIP_CHAR METHOD_PUBLISH[];

    static const SIP_CHAR MULTIPART[];
    static const SIP_CHAR SDP[];

    enum
    {
        SIP_SC_INVALID = 0,
        SIP_SC_100 = 100,
        SIP_SC_200 = 200,
        SIP_SC_300 = 300,
        SIP_SC_400 = 400,
        SIP_SC_500 = 500,
        SIP_SC_600 = 600,
        SIP_SC_MAX = 700
    };

    inline static SIP_BOOL IsProvisionalResponse(SIP_UINT16 nResponseCode)
    {
        return ((nResponseCode >= SIP_SC_100) && (nResponseCode < SIP_SC_200)) ? SIP_TRUE
                                                                               : SIP_FALSE;
    }

    inline static SIP_BOOL IsSuccessfulResponse(SIP_UINT16 nResponseCode)
    {
        return ((nResponseCode >= SIP_SC_200) && (nResponseCode < SIP_SC_300)) ? SIP_TRUE
                                                                               : SIP_FALSE;
    }

    inline static SIP_BOOL IsRedirectionResponse(SIP_UINT16 nResponseCode)
    {
        return ((nResponseCode >= SIP_SC_300) && (nResponseCode < SIP_SC_400)) ? SIP_TRUE
                                                                               : SIP_FALSE;
    }

    inline static SIP_BOOL IsClientFailureResponse(SIP_UINT16 nResponseCode)
    {
        return ((nResponseCode >= SIP_SC_400) && (nResponseCode < SIP_SC_500)) ? SIP_TRUE
                                                                               : SIP_FALSE;
    }

    inline static SIP_BOOL IsServerFailureResponse(SIP_UINT16 nResponseCode)
    {
        return ((nResponseCode >= SIP_SC_500) && (nResponseCode < SIP_SC_600)) ? SIP_TRUE
                                                                               : SIP_FALSE;
    }

    inline static SIP_BOOL IsGlobalFailureResponse(SIP_UINT16 nResponseCode)
    {
        return ((nResponseCode >= SIP_SC_600) && (nResponseCode < SIP_SC_MAX)) ? SIP_TRUE
                                                                               : SIP_FALSE;
    }

    /* For all response except for SIP successful response */
    inline static SIP_BOOL IsFailureResponse(SIP_UINT16 nResponseCode)
    {
        return (nResponseCode >= SIP_SC_300) ? SIP_TRUE : SIP_FALSE;
    }

    inline static SIP_BOOL IsNonProvisionalResponse(SIP_UINT16 nResponseCode)
    {
        return (nResponseCode >= SIP_SC_200) ? SIP_TRUE : SIP_FALSE;
    }

    inline static SIP_VOID Encode(SIP_CHAR*& pMsg, SIP_CHAR cChar)
    {
        *pMsg = cChar;
        pMsg++;
    }

    inline static SIP_VOID EncodeCrlf(SIP_CHAR*& pMsg)
    {
        *pMsg = '\r';
        pMsg++;
        *pMsg = '\n';
        pMsg++;
    }

    static constexpr SIP_UINT32 CONTENT_HDR_COUNT = 5;
    static constexpr SIP_UINT32 MAX_INT_VALUE_LEN = 11;
    static constexpr SIP_UINT32 MAX_HDR_NAME_LEN = 32;
    static constexpr SIP_INT32 MAX_MSG_SIZE = 65535;

    static SIP_VOID SetValue(const SIP_CHAR* pszSrc, SIP_CHAR*& pszDst);

    static SIP_INT32 GetMsgType(const SIP_CHAR* pStartPt);

    static SipUri::UriType GetUriType(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    static SIP_INT32 GetHeaderType(const SIP_CHAR* pszHdrName);

    static SIP_INT32 CheckAndGetHeaderType(SIP_INT32 nType);
#ifdef SIP_STRICT_PARSING
    static SIP_BOOL IsValidAddress(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
#endif

    static const SIP_CHAR* FindMsgBodyEnd(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
            const SIP_CHAR* pszBoundary, SIP_BOOL& bBodyEnd);

    static SIP_INT32 GetMimeHeaderType(const SIP_CHAR* pszHdrName);
    static SIP_CHAR GetCompactHeaderName(SIP_INT32 nType);
};

class SIPHdrAccess
{
private:
    static SIP_INT32 GetHdrTypeCompact(SIP_CHAR cHdrName);

public:
    static void Init();
    static SIP_INT32 GetHeaderType(const SIP_CHAR* pszHdrName);
};
#endif  //__SIP_MSG_UTIL_H__
