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
#ifndef __SIP_STRING_H__
#define __SIP_STRING_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SipDatatypes.h"

inline SIP_CHAR SIP_TOLOWER(SIP_CHAR c)
{
    return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : (c);
}

inline SIP_CHAR SIP_TOUPPER(SIP_CHAR c)
{
    return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : (c);
}

void SipPf_Snprintf(SIP_CHAR* pszBuffer, SIP_UINT32 nBuffSize, const SIP_CHAR* pszFormat, ...);
void SipPf_Sprintf(SIP_CHAR* pszBuffer, const SIP_CHAR* pszFormat, ...);
void SipPf_Sscanf(const SIP_CHAR* pszBuffer, const SIP_CHAR* pszFormat, SIP_CHAR* pszCharAdd);

SIP_INT32 SipPf_Strlen(const SIP_CHAR* pszSource);

SIP_CHAR* SipPf_Strcpy(SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

SIP_CHAR* SipPf_Strncpy(SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);

SIP_INT16 SipPf_Strcmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

SIP_INT16 SipPf_Stricmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

SIP_INT16 SipPf_Strncmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);

SIP_INT16 SipPf_Strnicmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);

SIP_CHAR* SipPf_Strstr(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

SIP_CHAR* SipPf_Strdup(const SIP_CHAR* pszSource);

SIP_INT32 SipPf_Atoi(const SIP_CHAR* pszSource);

SIP_BOOL SipPf_Atoi_Unsigned(const SIP_CHAR* pszStr, SIP_UINT32& nValue);

SIP_CHAR* SipPf_Strrchr(SIP_CHAR* pszSource, SIP_CHAR cChar);

SIP_CHAR* SipPf_StripFileName(SIP_CHAR* pszFileName);

#endif  //__SIP_STRING_H__
