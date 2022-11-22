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

GLOBAL IMS_SINT32 IMS_Sprintf(
        OUT IMS_CHAR* pszBuf, IN IMS_SIZE_T nBufSize, IN const IMS_CHAR* pszFormat, ...);

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

GLOBAL IMS_UINT32 IMS_UcStrLen(IN const IMS_WCHAR* pwszStr);

GLOBAL IMS_UINT32 IMS_UcStrCpy(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN const IMS_WCHAR* pwszSrc);
#endif
