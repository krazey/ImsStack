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
#ifndef IMS_STR_LIB_H_
#define IMS_STR_LIB_H_

#include <stdarg.h>

#include "ImsTypeDef.h"

GLOBAL IMS_UINT32 IMS_StrLen(IN const IMS_CHAR* pszStr);

GLOBAL IMS_UINT32 IMS_StrCpy(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN const IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_StrNCpy(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN const IMS_CHAR* pszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_UINT32 IMS_StrCat(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN const IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_StrNCat(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN const IMS_CHAR* pszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_SINT32 IMS_Sprintf(
        OUT IMS_CHAR* pszBuf, IN IMS_SIZE_T nBufSize, IN const IMS_CHAR* pszFormat, ...);

GLOBAL IMS_CHAR* IMS_StrDup(IN const IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_Itoa(OUT IMS_CHAR* pszStr, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase);

GLOBAL IMS_SINT32 IMS_Atoi(IN const IMS_CHAR* pszStr);

GLOBAL IMS_CHAR* IMS_StrChr(IN const IMS_CHAR* pszStr, IN IMS_CHAR cChar);

GLOBAL IMS_CHAR* IMS_StrRChr(IN const IMS_CHAR* pszStr, IN IMS_CHAR cChar);

GLOBAL IMS_CHAR* IMS_StrStr(IN const IMS_CHAR* pszA, IN const IMS_CHAR* pszB);

GLOBAL IMS_SINT32 IMS_StrCmp(IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB);

GLOBAL IMS_SINT32 IMS_StrNCmp(
        IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize);

GLOBAL IMS_SINT32 IMS_StrICmp(IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB);

GLOBAL IMS_SINT32 IMS_StrNICmp(
        IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize);

GLOBAL IMS_SINT32 IMS_StrCmpUppercase(IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB);

GLOBAL IMS_BOOL IMS_StrToLowerCase(OUT IMS_CHAR* pszOutStr, IN const IMS_CHAR* pszInStr);

GLOBAL IMS_BOOL IMS_StrToUpperCase(OUT IMS_CHAR* pszOutStr, IN const IMS_CHAR* pszInStr);

//
// APIs for unicode string manipulation
//
GLOBAL IMS_UINT32 IMS_UcStrLen(IN const IMS_WCHAR* pwszStr);

GLOBAL IMS_UINT32 IMS_StrToUcStr(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN const IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_UcStrToStr(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN const IMS_WCHAR* pwszSrc, IN IMS_CHAR cDefaultChar);

GLOBAL IMS_UINT32 IMS_UcStrCpy(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN const IMS_WCHAR* pwszSrc);

GLOBAL IMS_UINT32 IMS_UcStrNCpy(OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize,
        IN const IMS_WCHAR* pwszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_UINT32 IMS_UcStrCat(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN const IMS_WCHAR* pwszSrc);

GLOBAL IMS_UINT32 IMS_UcStrNCat(OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize,
        IN const IMS_WCHAR* pwszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_SINT32 IMS_UcSprintf(OUT IMS_WCHAR* pwszBuf, IN IMS_SIZE_T nBufSize,
        IN const IMS_WCHAR* pwszFormat, IN va_list args);

GLOBAL IMS_UINT32 IMS_UcItoa(
        OUT IMS_WCHAR* pwszBuf, IN IMS_SIZE_T nBufSize, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase);

GLOBAL IMS_SINT32 IMS_UcAtoi(IN const IMS_WCHAR* pwszStr);

GLOBAL IMS_WCHAR* IMS_UcStrStr(IN const IMS_WCHAR* pwszA, IN const IMS_WCHAR* pwszB);

GLOBAL IMS_SINT32 IMS_UcStrCmp(IN const IMS_WCHAR* pwszStrA, IN const IMS_WCHAR* pwszStrB);

GLOBAL IMS_SINT32 IMS_UcStrNCmp(
        IN const IMS_WCHAR* pwszStrA, IN const IMS_WCHAR* pwszStrB, IN IMS_SIZE_T nSize);

GLOBAL IMS_BOOL IMS_UcStrToLowerCase(OUT IMS_WCHAR* pwszOutStr, IN const IMS_WCHAR* pwszInStr);

GLOBAL IMS_BOOL IMS_UcStrToUpperCase(OUT IMS_WCHAR* pwszOutStr, IN const IMS_WCHAR* pwszInStr);

GLOBAL IMS_SINT32 IMS_UcStrCmpUppercase(IN const IMS_WCHAR* pwszStrA, IN const IMS_WCHAR* pwszStrB);

GLOBAL IMS_UINT32 IMS_Utf8ToUcs(OUT IMS_WCHAR* pwszUcs, IN const IMS_CHAR* pszUtf8);

GLOBAL IMS_UINT32 IMS_UcsToUtf8(OUT IMS_CHAR* pszUtf8, IN const IMS_WCHAR* pwszUcs);

GLOBAL IMS_UINT32 IMS_Utf8ToEuckr(OUT IMS_CHAR* pszEuckr, IN const IMS_CHAR* pszUtf8);

GLOBAL IMS_UINT32 IMS_EuckrToUtf8(OUT IMS_CHAR* pszUtf8, IN const IMS_CHAR* pszEuckr);

GLOBAL IMS_UINT32 IMS_UcsToEuckr(
        OUT IMS_CHAR* pszEuckr, IN IMS_SIZE_T nEuckrSize, IN const IMS_WCHAR* pwszUcs);

GLOBAL IMS_UINT32 IMS_EuckrToUcs(
        OUT IMS_WCHAR* pwszUcs, IN IMS_SIZE_T nUcsSize, IN const IMS_CHAR* pszEuckr);

#endif
