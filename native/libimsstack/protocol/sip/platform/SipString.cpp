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
#include "ServiceSystemTime.h"
#include "platform/SipMemory.h"
#include "platform/SipString.h"

// NOLINTNEXTLINE(cert-dcl50-cpp)
void SipPf_Snprintf(SIP_CHAR* pszBuffer, SIP_UINT32 nBuffSize, const SIP_CHAR* pszFormat, ...)
{
    va_list arg;
    va_start(arg, pszFormat);
    vsnprintf(pszBuffer, nBuffSize, pszFormat, arg);
    va_end(arg);
}

// NOLINTNEXTLINE(cert-dcl50-cpp)
void SipPf_Sprintf(SIP_CHAR* pszBuffer, const SIP_CHAR* pszFormat, ...)
{
    va_list arg;
    va_start(arg, pszFormat);
    vsprintf(pszBuffer, pszFormat, arg);
    va_end(arg);
    return;
}

void SipPf_Sscanf(const SIP_CHAR* pszBuffer, const SIP_CHAR* pszFormat, SIP_CHAR* pszCharAdd)
{
    sscanf(pszBuffer, pszFormat, pszCharAdd);
    return;
}

SIP_INT32 SipPf_Strlen(const SIP_CHAR* pszStr)
{
    if (pszStr == SIP_NULL)
    {
        return 0;
    }

    SIP_UINT32 nCount = 0;
    while (*pszStr)
    {
        ++pszStr;
        ++nCount;
    }

    return nCount;
}

SIP_CHAR* SipPf_Strcpy(SIP_CHAR* pszDest, const SIP_CHAR* pszSrc)
{
    if ((pszDest == SIP_NULL) || (pszSrc == SIP_NULL))
    {
        return pszDest;
    }

    SIP_UINT32 nSrcLen = SipPf_Strlen(pszSrc);

    pszDest[0] = '\0';

    SipPf_Memcpy(pszDest, pszSrc, nSrcLen);
    pszDest[nSrcLen] = '\0';

    return pszDest;
}

SIP_CHAR* SipPf_Strncpy(SIP_CHAR* pszDest, const SIP_CHAR* pszSrc, SIP_UINT32 nNumChars)
{
    if ((pszDest == SIP_NULL) || (pszSrc == SIP_NULL))
    {
        return pszDest;
    }

    pszDest[0] = '\0';

    SIP_UINT32 nSrcLen = SipPf_Strlen(pszSrc);
    SIP_UINT32 nCopySize = nNumChars;

    if (nSrcLen < nNumChars)
    {
        nCopySize = nSrcLen;
    }

    SipPf_Memcpy(pszDest, pszSrc, nCopySize);
    pszDest[nCopySize] = '\0';

    return pszDest;
}

SIP_INT16 SipPf_Strcmp(const SIP_CHAR* pszStr1, const SIP_CHAR* pszStr2)
{
    if (pszStr1 == SIP_NULL || pszStr2 == SIP_NULL)
    {
        return -1;
    }

    while (*pszStr1 && *pszStr2)
    {
        if (*pszStr1 != *pszStr2)
        {
            return (*pszStr1) - (*pszStr2);
        }

        ++pszStr1;
        ++pszStr2;
    }

    return ((*pszStr1 == 0) && (*pszStr2 == 0)) ? 0 : -1;
}

SIP_INT16 SipPf_Stricmp(const SIP_CHAR* pszStr1, const SIP_CHAR* pszStr2)
{
    if (pszStr1 == SIP_NULL || pszStr2 == SIP_NULL)
    {
        return -1;
    }

    while (*pszStr1 && *pszStr2)
    {
        SIP_CHAR ch1 = SIP_TOLOWER(*pszStr1);
        SIP_CHAR ch2 = SIP_TOLOWER(*pszStr2);
        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStr1;
        ++pszStr2;
    }

    return ((*pszStr1) == 0 && (*pszStr2 == 0)) ? 0 : -1;
}

SIP_INT16 SipPf_Strncmp(const SIP_CHAR* pszStr1, const SIP_CHAR* pszStr2, SIP_UINT32 nNumChars)
{
    SIP_UINT32 nCount = nNumChars;

    if ((pszStr1 == SIP_NULL) || (pszStr2 == SIP_NULL) || (nCount == SIP_ZERO))
    {
        return -1;
    }

    while ((*pszStr1) && (*pszStr2) && (nCount > SIP_ZERO))
    {
        if (*pszStr1 != *pszStr2)
        {
            return (*pszStr1) - (*pszStr2);
        }

        ++pszStr1;
        ++pszStr2;
        nCount--;
    }

    return nCount ? -(SIP_ONE) : SIP_ZERO;
}

SIP_INT16 SipPf_Strnicmp(const SIP_CHAR* pszStr1, const SIP_CHAR* pszStr2, SIP_UINT32 nNumChars)
{
    SIP_UINT32 nCount = nNumChars;

    if ((pszStr1 == SIP_NULL) || (pszStr2 == SIP_NULL) || (nCount == SIP_ZERO))
    {
        return -(SIP_ONE);
    }

    while ((*pszStr1) && (*pszStr2) && (nCount > SIP_ZERO))
    {
        SIP_CHAR ch1 = SIP_TOLOWER(*pszStr1);
        SIP_CHAR ch2 = SIP_TOLOWER(*pszStr2);
        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStr1;
        ++pszStr2;
        nCount--;
    }

    return nCount ? -(SIP_ONE) : SIP_ZERO;
}

SIP_CHAR* SipPf_Strstr(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource)
{
    return (SIP_CHAR*)strstr(pszDest, pszSource);
}

SIP_CHAR* SipPf_Strdup(const SIP_CHAR* pszSrc)
{
    if (pszSrc == SIP_NULL)
    {
        return SIP_NULL;
    }

    SIP_UINT32 nSrcLen = SipPf_Strlen(pszSrc);
    SIP_CHAR* pszTarget = new SIP_CHAR[nSrcLen + 1];

    if (pszTarget == SIP_NULL)
    {
        return SIP_NULL;
    }

    *pszTarget = '\0';
    SipPf_Memcpy(pszTarget, pszSrc, nSrcLen + 1);

    return pszTarget;
}

SIP_INT32 SipPf_Atoi(const SIP_CHAR* pszStr)
{
    if (pszStr == SIP_NULL)
    {
        return SIP_ZERO;
    }

    SIP_UINT32 nStrLen = SipPf_Strlen(pszStr);

    if ((nStrLen > 11) && (pszStr[SIP_ZERO] == '-'))
    {
        nStrLen = 11;
    }
    else if (nStrLen > 10) /*2^31 = 2147483648*/
    {
        nStrLen = 10;
    }

    SIP_INT32 nValue = SIP_ZERO;
    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nStrLen; ++nIndex)
    {
        if ((pszStr[nIndex] >= '0') && (pszStr[nIndex] <= '9'))
        {
            nValue = nValue * 10 + (pszStr[nIndex] - '0');
        }
        else if (pszStr[nIndex] == '.')
        {
            break;
        }
    }

    if (pszStr[SIP_ZERO] == '-')
    {
        nValue *= (-1);
    }

    return nValue;
}

SIP_BOOL SipPf_Atoi_Unsigned(const SIP_CHAR* pszStr, SIP_UINT32& nValue)
{
    nValue = SIP_ZERO;

    if (pszStr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SIP_UINT32 nStrLen = SipPf_Strlen(pszStr);

    if (nStrLen > 10) /*2^32 = 4294967296*/
    {
        return SIP_FALSE;
    }

    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nStrLen; ++nIndex)
    {
        if (pszStr[nIndex] >= '0' && pszStr[nIndex] <= '9')
        {
            nValue = nValue * 10 + (pszStr[nIndex] - '0');
        }
        else if (pszStr[nIndex] == '.')
        {
            break;
        }
    }

    return SIP_TRUE;
}

SIP_CHAR* SipPf_Strrchr(SIP_CHAR* pszSrc, SIP_CHAR cChar)
{
    SIP_CHAR* pszDest = SIP_NULL;

    do
    {
        if (*pszSrc == cChar)
        {
            pszDest = pszSrc;
        }

        if ((*pszSrc) == SIP_NULL)
        {
            break;
        }
        pszSrc++;

    } while (SIP_ONE);

    return (pszDest);
}

SIP_CHAR* SipPf_StripFileName(SIP_CHAR* pszFileName)
{
    SIP_CHAR* pTemp = SIP_NULL;

    // Device file path name will come with forward slash
    pTemp = SipPf_Strrchr(pszFileName, '/');
    pTemp = pTemp + SIP_ONE;

    return pTemp;
}
