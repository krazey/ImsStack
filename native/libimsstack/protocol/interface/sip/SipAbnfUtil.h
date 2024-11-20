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
#ifndef __SIP_ABNF_UTIL_H__
#define __SIP_ABNF_UTIL_H__

#include "SipDatatypes.h"

#define SPACE                      ' '
#define COLON                      ':'
#define SIP_DOT                    '.'
#define SIP_DQUOTE                 '"'
#define SIP_SEMI                   ';'
#define LPARAN                     '('
#define RPARAN                     ')'
#define COMMA                      ','
#define ATRATE                     '@'
#define ASTERISK                   '*'
#define SLASH                      '/'
#define QMARK                      '?'
#define EQUAL                      '='
#define DQUOTE                     '"'
#define AMPERSAND                  '&'
#define BACKSLASH                  '\\'
#define LEFT_ANGLE                 '<'
#define RIGHT_ANGLE                '>'
#define LEFT_SQUARE                '['
#define RIGHT_SQUARE               ']'
#define PERCENT                    '%'
#define PLUS                       '+'
#define HYPHEN                     '-'

#define EXCLAMATION                '!'
#define HASHH                      '#'

#define SIP_SIP                    "sip"
#define SIP_SIPS                   "sips"
#define SIP_SIPVER                 "SIP/2.0"
#define SIP_USER                   "User"
#define SIP_PASSWORD               "Password"
#define SIP_HOST                   "Host"
#define SIP_PORT                   "Port"
#define SIP_USER_PRM               "user"
#define SIP_METHOD                 "method"
#define SIP_MADDR_PRM              "maddr"
#define SIP_TTL_PRM                "ttl"
#define SIP_TRNSPORT_PRM           "transport"
#define SIP_LR_PRM                 "lr"
#define SIP_OTHER_PRM              "other-param"
#define SIP_HEADERS                "headers"

#define IS_ALPHA(ch)               (((ch) >= 97 && (ch) <= 122) || ((ch) >= 65 && (ch) <= 90))
#define IS_DIGIT(ch)               ((ch) >= 48 && (ch) <= 57)
#define IS_OCTAL(ch)               ((ch) >= 48 && (ch) <= 55)
#define IS_BINARY(ch)              ((ch) >= 48 && (ch) <= 49)
// clang-format off
#define IS_HEXDIG(ch)              (((ch) >= 65 && (ch) <= 70) || \
                                   ((ch) >= 97 && (ch) <= 102) || IS_DIGIT((ch)))
// clang-format on
#define IS_HEXL(ch)                (((ch) >= 0x61 && (ch) <= 0x66) || IS_DIGIT((ch)))
#define IS_ALPHANUM(ch)            (IS_ALPHA((ch)) || IS_DIGIT((ch)))

#define IS_SPACE(ch)               ((ch) == ' ')
#define IS_ASTERISK(ch)            ((ch) == 42)
#define IS_DOT(ch)                 ((ch) == '.')
#define IS_SLASH(ch)               ((ch) == '/')
#define IS_LSQURE(ch)              ((ch) == '[')
#define IS_RSQURE(ch)              ((ch) == ']')
#define IS_NOT_SPACE(ch)           ((ch) != ' ')
#define IS_NOT_DIGIT(ch)           ((ch) < 48 && (ch) > 57)

#define IS_EQUALS(ch)              ((ch) == '=')
#define IS_ATRATE(ch)              ((ch) == '@')
#define IS_AMPERSAND(ch)           ((ch) == '&')
#define IS_DOLLAR(ch)              ((ch) == '$')

#define IS_PLUS(ch)                ((ch) == '+')
#define IS_SEMI_COLON(ch)          ((ch) == ';')

#define IS_CR(ch)                  ((ch) == 13)
#define IS_COLON(ch)               ((ch) == 58)
#define IS_COMMA(ch)               ((ch) == ',')

#define IS_LF(ch)                  ((ch) == 10)
#define IS_HTAB(ch)                ((ch) == 9)
#define IS_DQUOTE(ch)              ((ch) == 34)
#define IS_HYPHEN(ch)              ((ch) == 45)
#define IS_LHEX(ch)                (IS_DIGIT((ch)) || ((ch) >= 97 && ((ch)) <= 102))
#define IS_WSP(ch)                 (IS_SPACE((ch)) || IS_HTAB((ch)))
#define IS_CRLF(ch1, ch2)          (IS_CR((ch1)) && IS_LF((ch2)))

#define IS_ALPHANUM_DASH(ch1, ch2) (IS_ALPHANUM((ch1)) && IS_HYPHEN((ch2)))

#define IS_MARK(ch)                                                                       \
    (((ch) == '-') || ((ch) == '_') || ((ch) == '.') || ((ch) == '!') || ((ch) == '~') || \
            ((ch) == '*') || ((ch) == '(') || ((ch) == ')') || ((ch) == 39))

#define IS_RESERVED(ch)                                                                   \
    (((ch) == ';') || ((ch) == '/') || ((ch) == '?') || ((ch) == ':') || ((ch) == '@') || \
            ((ch) == '&') || ((ch) == '=') || ((ch) == '+') || ((ch) == '$') || ((ch) == ','))

#define IS_PCHAR(ch)                                                                      \
    (((ch) == ':') || ((ch) == '@') || ((ch) == '&') || ((ch) == '=') || ((ch) == '+') || \
            ((ch) == '$') || ((ch) == ','))

#define IS_URIC_NO_SLASH(ch)                                                              \
    (((ch) == ';') || ((ch) == '?') || ((ch) == ':') || ((ch) == '@') || ((ch) == '&') || \
            ((ch) == '=') || ((ch) == '+') || ((ch) == '$') || ((ch) == ','))

#define IS_SEPARATORS(ch)                                                                 \
    (((ch) == '(') || ((ch) == '/') || ((ch) == '?') || ((ch) == ':') || ((ch) == '@') || \
            ((ch) == '&') || ((ch) == '=') || ((ch) == '+') || ((ch) == '$') || ((ch) == ','))

#define IS_UNRESERVED(ch) (IS_ALPHANUM((ch)) || IS_MARK((ch)))

#define IS_PASSWORD(ch) \
    (((ch) == '&') || ((ch) == '=') || ((ch) == '+') || ((ch) == '$') || ((ch) == ','))

#define IS_REGNAME(ch)                                                                    \
    (((ch) == '$') || ((ch) == ',') || ((ch) == ';') || ((ch) == ':') || ((ch) == '@') || \
            ((ch) == '&') || ((ch) == '=') || ((ch) == '+'))

#define IS_USER_UNRESERVED(ch)                                                            \
    (((ch) == '&') || ((ch) == '=') || ((ch) == '+') || ((ch) == '$') || ((ch) == ',') || \
            ((ch) == ';') || ((ch) == '?') || ((ch) == '/'))

#define IS_TOKEN(ch)                                                                              \
    (IS_ALPHANUM((ch)) || ((ch) == '-') || ((ch) == '.') || ((ch) == '!') || ((ch) == '%') ||     \
            ((ch) == '_') || ((ch) == '+') || ((ch) == '`') || ((ch) == '\'') || ((ch) == '~') || \
            ((ch) == '*'))

#define IS_TOKEN_NO_BANG(ch)                                                                  \
    (IS_ALPHANUM((ch)) || ((ch) == '-') || ((ch) == '.') || ((ch) == '%') || ((ch) == '_') || \
            ((ch) == '+') || ((ch) == '`') || ((ch) == '\'') || ((ch) == '~') || ((ch) == '*'))

#define IS_TOKEN_NO_DOT(ch)                                                                   \
    (IS_ALPHANUM((ch)) || ((ch) == '-') || ((ch) == '!') || ((ch) == '%') || ((ch) == '_') || \
            ((ch) == '+') || ((ch) == '`') || ((ch) == '\'') || ((ch) == '~') || ((ch) == '*'))

#define IS_WORD(ch)                                                                               \
    (IS_TOKEN((ch)) || ((ch) == '(') || ((ch) == ')') || ((ch) == '<') || ((ch) == '>') ||        \
            ((ch) == ':') || ((ch) == '\\') || ((ch) == '"') || ((ch) == '/') || ((ch) == '[') || \
            ((ch) == ']') || ((ch) == '?') || ((ch) == '{') || ((ch) == '}'))

#define IS_HNV_UNRESERVED(ch)                                                             \
    (((ch) == '[') || ((ch) == ']') || ((ch) == '/') || ((ch) == '?') || ((ch) == ':') || \
            ((ch) == '+') || ((ch) == '$'))

#define IS_ESCAPED(ch1, ch2, ch3) (((ch1) == '%') && (IS_HEXDIG((ch2))) && (IS_HEXDIG((ch3))))

// clang-format off
#define IS_SCHEME(ch)             (IS_ALPHANUM((ch)) || \
                                  ((ch) == '+') || ((ch) == '-') || ((ch) == '.'))
// clang-format on

#define SIP_COLLECT_TEMP_POS(pch1, pch2) \
    (pch1) = (pch2);                     \
    (pch2) = SIP_NULL

#define IS_QdText(ch) \
    (((ch) == 0x21) || (((ch) >= 0x23) && ((ch) <= 0x5B)) || (((ch) >= 0x5D) && ((ch) <= 0x7E)))

#define IS_QdTextNoAbkt(ch)                                                    \
    (((ch) == 0x21) || (((ch) >= 0x23) && ((ch) <= 0x3B)) || ((ch) == 0x3D) || \
            (((ch) >= 0x3F) && ((ch) <= 0x5B)) || (((ch) >= 0x5D) && ((ch) <= 0x7E)))

#define IS_QUOTED_PAIR(ch)                                                       \
    ((((ch) >= 0x00) && ((ch) <= 0x09)) || (((ch) >= 0x0B) && ((ch) <= 0x0C)) || \
            (((ch) >= 0x0E) && ((ch) <= 0x7F)))

/*  %80 - BF*/
#define IS_UTF8_CONT(ch) (((ch) >= 0x80) && ((ch) <= 0xBF))
/*
   ctext = %x21-27 / %x2A-5B / %x5D-7E / UTF8-NONASCII/ LWS
//Check UTF8-NONASCII using validateUTF8-NONASCII()
 */

#define IS_QVAL(ch)      (((ch) == 0x30) || ((ch) == 0x31))
#define IS_CTEXT(ch)                                                             \
    ((((ch) >= 0x21) && ((ch) <= 0x27)) || (((ch) >= 0x2A) && ((ch) <= 0x5B)) || \
            (((ch) >= 0x5D) && ((ch) <= 0x7E)))

// #define IS_LHEX(ch) ((((ch) >= 48) && ((ch) <= 57)) ||((((ch) >= 61) && ((ch) <= 66))) )
class SipAbnfUtil
{
public:
    static SIP_BOOL HasSpace(const SIP_CHAR* pszValue);

    static const SIP_CHAR* SkipRightWhiteSpace(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    static SIP_BOOL FindActualPosition(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
            const SIP_CHAR*& pTempPre, const SIP_CHAR*& pTempNext, const SIP_CHAR cDelimiter);

    static SIP_VOID UpdateCurrentPosition(SIP_CHAR*& pMsgBuffer);

    static SIP_VOID Append(SIP_CHAR*& pMsgBuffer, const SIP_CHAR* pszSrc);

    static SIP_BOOL FindWhiteSpace(
            const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR*& pTempLoc);

    static const SIP_CHAR* SkipWhiteSpaceFromLeft(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    static const SIP_CHAR* SkipWhiteSpaceFromRight(
            const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    static SIP_BOOL FindPostDelimiter(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
            const SIP_CHAR*& pTempLoc, const SIP_CHAR cDelimiter);

    static SIP_BOOL FindPreDelimiter(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
            const SIP_CHAR*& pTempLoc, const SIP_CHAR cDelimiter);

    static SIP_CHAR* CreateString(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    static SIP_BOOL FindCrlf(
            const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR*& pTempLoc);

    static SIP_BOOL FindTerminatingCrlf(const SIP_CHAR* pStartPoint, const SIP_CHAR* pEndPoint,
            const SIP_CHAR*& pLocation, SIP_BOOL& bHdrEnd);

    static const SIP_CHAR* SkipConsecutiveCrlf(const SIP_CHAR* pStartPt);
};

#endif  //__SIP_ABNF_UTIL_H__
