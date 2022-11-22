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
#include <stdio.h>
#include <string.h>

#include "ImsLib.h"
#include "ImsStrLib.h"
#include "ServiceMemory.h"

/**
 * @brief Returns a length of the given string length.
 */
GLOBAL IMS_UINT32 IMS_StrLen(IN const IMS_CHAR* pszStr)
{
    if (pszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nCount = 0;

    while (*pszStr)
    {
        ++pszStr;
        ++nCount;
    }

    return nCount;
}

/**
 * @brief Safe string copy.
 *
 * @param nDestSize The buffer size including null('\0') character
 */
GLOBAL IMS_UINT32 IMS_StrCpy(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN const IMS_CHAR* pszSrc)
{
    if ((pszSrc == IMS_NULL) || (pszDest == IMS_NULL))
    {
        return 0;
    }

    IMS_SIZE_T nSrcLen = IMS_StrLen(pszSrc);

    pszDest[0] = '\0';

    if (nDestSize <= nSrcLen)
    {
        return 0;
    }

    IMS_MEM_Memcpy(pszDest, pszSrc, nSrcLen);
    pszDest[nSrcLen] = '\0';

    return nSrcLen;
}

/**
 * @brief Safe string printf.
 *
 * @param nBufSize The buffer size excluding null('\0') character
 * @return The number of characters written (>= 0) if operation was successful, -1 otherwise.
 */
GLOBAL IMS_SINT32 IMS_Sprintf(
        OUT IMS_CHAR* pszBuf, IN IMS_SIZE_T nBufSize, IN const IMS_CHAR* pszFormat, ...)
{
    (void)nBufSize;

    if (pszBuf == IMS_NULL || pszFormat == IMS_NULL)
    {
        return 0;
    }

    IMS_SINT32 nSize;
    va_list args;

    *pszBuf = '\0';

    va_start(args, pszFormat);
    nSize = vsnprintf(pszBuf, nBufSize, pszFormat, args);
    va_end(args);

    return nSize;
}

GLOBAL IMS_UINT32 IMS_Itoa(OUT IMS_CHAR* pszStr, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase)
{
    if (pszStr == IMS_NULL)
    {
        return 0;
    }

    *pszStr = '\0';

    switch (nBase)
    {
        case 8:
            sprintf(pszStr, "%o", nNum);
            break;
        case 10:
            sprintf(pszStr, "%d", nNum);
            break;
        case 16:
            sprintf(pszStr, "%x", nNum);
            break;
        default:
            return 0;
    }

    return IMS_StrLen(pszStr);
}

GLOBAL IMS_SINT32 IMS_Atoi(IN const IMS_CHAR* pszStr)
{
    if (pszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nStrLen = IMS_StrLen(pszStr);

    if ((nStrLen > 11) && (pszStr[0] == '-'))
    {
        nStrLen = 11;
    }
    else if (nStrLen > 10) /*2^31 = 2147483648*/
    {
        nStrLen = 10;
    }

    IMS_SINT32 nValue = 0;

    for (IMS_UINT32 i = 0; i < nStrLen; ++i)
    {
        if (pszStr[i] >= '0' && pszStr[i] <= '9')
        {
            nValue = nValue * 10 + (pszStr[i] - '0');
        }
        else if (pszStr[i] == '.')
        {
            break;
        }
    }

    if (pszStr[0] == '-')
    {
        nValue *= (-1);
    }

    return nValue;
}

GLOBAL IMS_CHAR* IMS_StrChr(IN const IMS_CHAR* pszStr, IN IMS_CHAR cChar)
{
    return (IMS_CHAR*)strchr(pszStr, cChar);
}

GLOBAL IMS_CHAR* IMS_StrRChr(IN const IMS_CHAR* pszStr, IN IMS_CHAR cChar)
{
    return (IMS_CHAR*)strrchr(pszStr, cChar);
}

GLOBAL IMS_CHAR* IMS_StrStr(IN const IMS_CHAR* pszA, IN const IMS_CHAR* pszB)
{
    return (IMS_CHAR*)strstr(pszA, pszB);
}

GLOBAL IMS_SINT32 IMS_StrCmp(IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB)
{
    if ((pszStrA == IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 0;
    }
    else if ((pszStrA == IMS_NULL) && (pszStrB != IMS_NULL))
    {
        return (-1);
    }
    else if ((pszStrA != IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 1;
    }

    while (*pszStrA && *pszStrB)
    {
        if (*pszStrA != *pszStrB)
        {
            return (*pszStrA) - (*pszStrB);
        }

        ++pszStrA;
        ++pszStrB;
    }

    if ((*pszStrA == 0) && (*pszStrB != 0))
    {
        return (-1);
    }
    else if ((*pszStrA != 0) && (*pszStrB == 0))
    {
        return 1;
    }

    return 0;
}

GLOBAL IMS_SINT32 IMS_StrNCmp(
        IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize)
{
    // 3 Please, modify this function according to the standard string manipulation
    IMS_SIZE_T nCount = nSize;

    if (pszStrA == IMS_NULL || pszStrB == IMS_NULL || nCount == 0)
    {
        return -1;
    }

    while ((*pszStrA) && (*pszStrB) && (nCount != 0))
    {
        if (*pszStrA != *pszStrB)
        {
            return (*pszStrA) - (*pszStrB);
        }

        ++pszStrA;
        ++pszStrB;
        nCount--;
    }

    return (nCount != 0) ? -1 : 0;
}

GLOBAL IMS_SINT32 IMS_StrICmp(IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB)
{
    IMS_CHAR ch1;
    IMS_CHAR ch2;

    if ((pszStrA == IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 0;
    }
    else if ((pszStrA == IMS_NULL) && (pszStrB != IMS_NULL))
    {
        return (-1);
    }
    else if ((pszStrA != IMS_NULL) && (pszStrB == IMS_NULL))
    {
        return 1;
    }

    while (*pszStrA && *pszStrB)
    {
        ch1 = IMS_TOLOWER(*pszStrA);
        ch2 = IMS_TOLOWER(*pszStrB);

        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStrA;
        ++pszStrB;
    }

    if ((*pszStrA == 0) && (*pszStrB != 0))
    {
        return (-1);
    }
    else if ((*pszStrA != 0) && (*pszStrB == 0))
    {
        return 1;
    }

    return 0;
}

GLOBAL IMS_SINT32 IMS_StrNICmp(
        IN const IMS_CHAR* pszStrA, IN const IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize)
{
    // 3 Please, modify this function according to the standard string manipulation
    IMS_SIZE_T nCount = nSize;

    if (pszStrA == IMS_NULL || pszStrB == IMS_NULL || nCount == 0)
    {
        return (-1);
    }

    while ((*pszStrA) && (*pszStrB) && (nCount != 0))
    {
        IMS_CHAR ch1 = IMS_TOLOWER(*pszStrA);
        IMS_CHAR ch2 = IMS_TOLOWER(*pszStrB);

        if (ch1 != ch2)
        {
            return ch1 - ch2;
        }

        ++pszStrA;
        ++pszStrB;
        nCount--;
    }

    return (nCount != 0) ? -1 : 0;
}

GLOBAL IMS_UINT32 IMS_UcStrLen(IN const IMS_WCHAR* pwszStr)
{
    if (pwszStr == IMS_NULL)
    {
        return 0;
    }

    IMS_UINT32 nCount = 0;

    while (*pwszStr)
    {
        ++pwszStr;
        ++nCount;
    }

    return nCount;
}

/**
 * @brief Safe UCS2 string copy.
 *
 * @param nDestSize The buffer size excluding null('\0') character
 */
GLOBAL IMS_UINT32 IMS_UcStrCpy(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN const IMS_WCHAR* pwszSrc)
{
    if ((pwszDest == IMS_NULL) || (pwszSrc == IMS_NULL))
    {
        return 0;
    }

    pwszDest[0] = 0;

    IMS_SIZE_T nSrcLen = IMS_UcStrLen(pwszSrc);

    if (nDestSize < nSrcLen)
    {
        return 0;
    }

    IMS_MEM_Memcpy(pwszDest, pwszSrc, nSrcLen * sizeof(IMS_WCHAR));
    pwszDest[nSrcLen] = 0;

    return nSrcLen;
}
