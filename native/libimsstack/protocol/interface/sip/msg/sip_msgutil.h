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
#ifndef __SIP_MSGUTIL_H__
#define __SIP_MSGUTIL_H__

#include "sip_pf_datatypes.h"

#include "msg/sip_comdef.h"
#include "sip_abnfUtil.h"
#include "msg/SipHeaderBase.h"
#include "sip_error.h"

#define SIP_SIP_VERSION "SIP/2.0"
#define ZERO            "0"

#define SIP_ENC_SP(pC) \
    *(pC) = 32;        \
    (pC)++
#define SIP_ENC_CRLF(pC) \
    *(pC) = 13;          \
    (pC)++;              \
    *(pC) = 10;          \
    (pC)++
#define SIP_ENC_DASH(pC) \
    *(pC) = 45;          \
    (pC)++;              \
    *(pC) = 45;          \
    (pC)++
#define SIP_ENC_STAR(pC) \
    *(pC) = 42;          \
    (pC)++
#define SIP_ENC_SLASH(pC) \
    *(pC) = 47;           \
    (pC)++
#define SIP_ENC_EQUAL(pC) \
    *(pC) = 61;           \
    (pC)++
#define SIP_ENC_LPAREN(pC) \
    *(pC) = 40;            \
    (pC)++
#define SIP_ENC_RPAREN(pC) \
    *(pC) = 41;            \
    (pC)++
#define SIP_ENC_LAQUOT(pC) \
    *(pC) = 60;            \
    (pC)++
#define SIP_ENC_RAQUOT(pC) \
    *(pC) = 62;            \
    (pC)++
#define SIP_ENC_COMMA(pC) \
    *(pC) = 44;           \
    (pC)++
#define SIP_ENC_SEMI(pC) \
    *(pC) = 59;          \
    (pC)++
#define SIP_ENC_COLON(pC) \
    *(pC) = 58;           \
    (pC)++
#define SIP_ENC_LDQUOT(pC) \
    *(pC) = 34;            \
    (pC)++
#define SIP_ENC_RDQUOT(pC) \
    *(pC) = 34;            \
    (pC)++
#define SIP_ENC_ATTHERATE(pC) \
    *(pC) = 64;               \
    (pC)++
#define SIP_ENC_QMARK(pC) \
    *(pC) = 63;           \
    (pC)++
#define SIP_ENC_AMPERSAND(pC) \
    *(pC) = 38;               \
    (pC)++
#define SIP_ENC_BSLASH(pC) \
    *(pC) = 92;            \
    (pC)++
#define SIP_ENC_DOT(pC) \
    *(pC) = 46;         \
    (pC)++

#define SIP_MAX_HDR_LEN       32
#define SIP_MAX_MTD_LEN       16
#define SIP_MAX_RESP_CODE_LEN 4
#define MAX_RESP_PHRS_LEN     38
#define SIP_MAX_URI_LEN       6
#define SIP_CONTENT_HDRS_LEN  5
#define SIP_CONTLEN_LEN       12

// keeping same SIP message buffer size which is mentioned in SipMessageBuffer.h
#define SIP_MAX_MSG_SIZE      65535

#define SIP_ENC_SHORT_ACCEPT_CONTACT(ch) \
    *(ch) = 'a';                         \
    (ch)++
#define SIP_ENC_SHORT_REFERRED_BY(ch) \
    *(ch) = 'b';                      \
    (ch)++
#define SIP_ENC_SHORT_CONTENT_TYPE(ch) \
    *(ch) = 'c';                       \
    (ch)++
#define SIP_ENC_SHORT_REQUEST_DISPOSITION(ch) \
    *(ch) = 'd';                              \
    (ch)++
#define SIP_ENC_SHORT_CONTENT_ENCODING(ch) \
    *(ch) = 'e';                           \
    (ch)++
#define SIP_ENC_SHORT_FROM(ch) \
    *(ch) = 'f';               \
    (ch)++
#define SIP_ENC_SHORT_CALLID(ch) \
    *(ch) = 'i';                 \
    (ch)++
#define SIP_ENC_SHORT_REJECT_CONTACT(ch) \
    *(ch) = 'j';                         \
    (ch)++
#define SIP_ENC_SHORT_SUPPORTED(ch) \
    *(ch) = 'k';                    \
    (ch)++
#define SIP_ENC_SHORT_CONTENT_LENGTH(ch) \
    *(ch) = 'l';                         \
    (ch)++
#define SIP_ENC_SHORT_CONTACT(ch) \
    *(ch) = 'm';                  \
    (ch)++
#define SIP_ENC_SHORT_IDENTITY_INFO(ch) \
    *(ch) = 'n';                        \
    (ch)++
#define SIP_ENC_SHORT_EVENT(ch) \
    *(ch) = 'o';                \
    (ch)++
#define SIP_ENC_SHORT_REFER_TO(ch) \
    *(ch) = 'r';                   \
    (ch)++
#define SIP_ENC_SHORT_SUBJECT(ch) \
    *(ch) = 's';                  \
    (ch)++
#define SIP_ENC_SHORT_TO(ch) \
    *(ch) = 't';             \
    (ch)++
#define SIP_ENC_SHORT_ALLOW_EVENTS(ch) \
    *(ch) = 'u';                       \
    (ch)++
#define SIP_ENC_SHORT_VIA(ch) \
    *(ch) = 'v';              \
    (ch)++
#define SIP_ENC_SHORT_SESSION_EXPIRES(ch) \
    *(ch) = 'x';                          \
    (ch)++
#define SIP_ENC_SHORT_IDENTITY(ch) \
    *(ch) = 'y';                   \
    (ch)++

#define MULTIPART "Multipart"
#define MIXED     "Mixed"
#define SDP       "Sdp"
#define SIP_ENC_GMT(ch) \
    *(ch) = 'G';        \
    (ch)++;             \
    *(ch) = 'M';        \
    (ch)++;             \
    *(ch) = 'T';        \
    (ch)++

#define SIP_AINFO_CNT      5

#define SIP_METHOD_COUNT   14

#define SIP_AINFO_LEN      20

#define MAXLETDIG          27

#define MIN_WARNCODE       100

#define MAX_WARNCODE       999

#define MAX_EXPIRES        4294967295

#define MAX_MAXFD          255

#define MAX_ERROR_CODE     9999

#define MAX_CIDLEN         48

#define MAX_FEIDLEN        16

#define SIP_DIRECTIVE_SIZE 12

#define SIP_DIRECTIVE_LEN  11

SIP_BOOL SetCharVar(const SIP_CHAR* pszValue, SIP_CHAR*& pszVar);

SIP_BOOL HasSpace(const SIP_CHAR* pszValue);

SIP_INT32 sipGetMsgType(SIP_CHAR* pStartPoint);

SIP_BOOL sipFindTerminatingCRLF(
        SIP_CHAR* pStartPoint, SIP_CHAR* pEndPoint, SIP_CHAR** ppLocation, SIP_BOOL* pbHdrEnd);

SIP_CHAR* sipSkipFwSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

SIP_CHAR* sipSkipRwSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

SIP_CHAR* sipSkipFwWSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

SIP_CHAR* sipSkipRwWSP(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

SIP_CHAR* sipSkipFwLWS(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

SIP_BOOL sipFindPostDelimiter(
        SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc, SIP_CHAR cDelimiter);

SIP_BOOL sipFindPreDelimiter(
        SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc, SIP_CHAR cDelimiter);

SIP_BOOL sipFindActualPos(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempPre,
        SIP_CHAR** ppTempNext, SIP_CHAR cDelimiter);

SIP_VOID SipEnc_UpdateCurrPos(SIP_CHAR** ppMsgBuffer /*in -out param*/);

SIP_INT32 sipGetUriType(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

SIP_INT32 sipGetHdrType(const SIP_CHAR* pszHdrName);

SIP_INT32 CheckAndGetHdrEnumType(SIP_INT32 nType);
#ifdef SIP_STRICT_PARSING
SIP_BOOL IsValidAddress(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
#endif
SIP_BOOL sipFindCrlf(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR** ppTempLoc);

SIP_CHAR* sipFindBodyEnd(
        SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR* pszBoundary, SIP_BOOL& bBodyEnd);

SIP_INT32 sipGetMimeHdrType(SIP_CHAR* pszHdrName);

const SIP_INT16 arrSipHeadersType[SipHeaderBase::TYPE_END + 1] = {1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1,
        0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
        1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0};

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

class SIPHdrAccess
{
private:
    static SIP_INT32 GetHdrTypeCompact(SIP_CHAR cHdrName);

public:
    static void Init();
    static SIP_INT32 GetHdrType(const SIP_CHAR* pszHdrName);
};
#endif
