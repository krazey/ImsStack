#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "ServiceSystemTime.h"

/******************************************************************************
 * Function name    : SipPf_Sprintf
 * Description      :
 * Return type      :
 *
 * Argument         :
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 *****************************************************************************/
void SipPf_Snprintf(SIP_CHAR* pszBuffer, SIP_UINT32 nBuffSize, const SIP_CHAR* pszFormat, ...)
{
    va_list arg;
    va_start(arg, pszFormat);
    vsnprintf(pszBuffer, nBuffSize, pszFormat, arg);
    va_end(arg);
}

void SipPf_Sprintf(SIP_CHAR* pszBuffer, const SIP_CHAR* pszFormat, ...)
{
    va_list arg;
    va_start(arg, pszFormat);
    vsprintf(pszBuffer, pszFormat, arg);
    va_end(arg);
    return;
}

void SipPf_Printf(SIP_CHAR* pszFormat, ...)
{
    va_list arg;
    va_start(arg, pszFormat);
    printf(pszFormat, arg);
    va_end(arg);
    return;
}

void SipPf_Sscanf(SIP_CHAR* pszBuffer, SIP_CHAR* pszFormat, SIP_CHAR* pszCharAdd)
{
    sscanf(pszBuffer, pszFormat, pszCharAdd);
    return;
}

SIP_UINT32 SipPf_Rand()
{
    return 0;
}

/******************************************************************************
 * Function name    : SipPf_Strlen
 * Description        :  This function gets the length of Source string
 * Return type        : SipDt _Int32
 *                      Length of Source string
 * Argument           : pszSource: IN
 *                      Input string

 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 *****************************************************************************/
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

/******************************************************************************
 * Function name    : SipPf_Strcpy
 * Description        :  This function copies source string to     destination
 string including '\0'
 * Return type        : SipDt _Char *
 *                      Pointer to destination string
 * Argument           : pszDest: InOut
 *                      Destination string
 * Argument           : pszSource: In
 *                      Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 *****************************************************************************/
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

/******************************************************************************
 * Function name    : SipPf_Strncpy
 * Description        :  This function copies at most iNumChars
 from pszSource to pszDest and terminates
 pszDest with '\0'
 * Return type        : SIP_CHAR *
 *                      Pointer to pszDest
 * Argument           : pszDest: <InOut>
 *                      Destination string
 * Argument           : pszSource: <In>
 *                      Source string
 * Argument           : iNumChars: <In>
 *                      Number of characters to copy
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/

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

/******************************************************************************
 * Function name    : SipPf_Strcat
 * Description        : This function concatenates source string
 to end of destination string.
 * Return type        : SIP_CHAR *
 *                      Pointer to pszDest
 * Argument           : pszDest: <InOut>
 *                      Destination string
 * Argument           : pszSource: <In>
 *                      <Source string>
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/
SIP_CHAR* SipPf_Strcat(SIP_CHAR* pszDest, const SIP_CHAR* pszSrc)
{
    if ((pszDest == SIP_NULL) || (pszDest == SIP_NULL))
    {
        return pszDest;
    }

    SIP_UINT32 nDestLen = SipPf_Strlen(pszDest);
    SIP_UINT32 nSrcLen = SipPf_Strlen(pszSrc);

    SipPf_Memcpy(pszDest + nDestLen, pszSrc, nSrcLen);
    pszDest[nDestLen + nSrcLen] = '\0';

    return pszDest;
}

/******************************************************************************
 * Function name    : SipPf_Strncat
 * Description        : This function concatenates ssNumChars
 of Source string to end of destination
 string
 * Return type        : SIP_CHAR*
 *                      Pointer to pszDest
 * Argument           : pszDest: <InOut>
 *                      Destination string
 * Argument           : pszSource: <In>
 *                      Source string
 * Argument           : ssNumChars: <In>
 *                      Number of characters to concatenate
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/
SIP_CHAR* SipPf_Strncat(SIP_CHAR* pszDest, const SIP_CHAR* pszSrc, SIP_UINT32 nNumChars)
{
    if ((pszDest == SIP_NULL) || (pszDest == SIP_NULL))
    {
        return pszDest;
    }

    SIP_UINT32 nDestLen = SipPf_Strlen(pszDest);
    SIP_UINT32 nSrcLen = SipPf_Strlen(pszSrc);

    SIP_UINT32 nCopySize = nNumChars;
    if (nSrcLen < nNumChars)
    {
        nCopySize = nSrcLen;
    }

    SipPf_Memcpy(pszDest + nDestLen, pszSrc, nCopySize);
    pszDest[nDestLen + nCopySize] = '\0';

    return pszDest;
}

/******************************************************************************
 * Function name    : SipPf_Strcmp
 * Description      : This function compares two strings. It is case sensitive comparison
 * Return type      : SIP_INT16
 > 0 : if pszStr1 > pszStr2
0 : if strings are same
< 0 : if pszStr1 < pszStr2
 * Argument          :
 * Preconditions/
 * Side Effects    : Preconditions for the function to be required, argument list that will be
updated
 *
 ****************************************************************************/
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

/******************************************************************************
 * Function name    : SipPf_Stricmp
 * Description      : This function compares two strings. It is non-case sensitive comparison
 * Return type      : SIP_INT16
 > 0 : if pszStr1 > pszStr2
0 : if strings are same
< 0 : if pszStr1 < pszStr2
 * Argument          :
 * Preconditions/
 * Side Effects    : Preconditions for the function to be required, argument list that will be
updated
 *
 ****************************************************************************/
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

/******************************************************************************
 * Function name    : SipPf_Strncmp
 * Description      : This function compares two strings for specified number of chars.
 *                      It is case sensitive comparison
 * Return type      : SIP_INT16
 > 0 : if pszStr1 > pszStr2
0 : if strings are same
< 0 : if pszStr1 < pszStr2
 * Argument          :
 * Preconditions/
 * Side Effects    : Preconditions for the function to be required, argument list that will be
updated
 *
 ****************************************************************************/
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

/******************************************************************************
 * Function name    : SipPf_Stricmp
 * Description      : This function compares two strings for specified number of chars.
 *                      It is non-case sensitive comparison
 * Return type      : SIP_INT16
 > 0 : if pszStr1 > pszStr2
0 : if strings are same
< 0 : if pszStr1 < pszStr2
 * Argument          :
 * Preconditions/
 * Side Effects    : Preconditions for the function to be required, argument list that will be
updated
 *
 ****************************************************************************/
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

/******************************************************************************
 * Function name    : SipPf_Strstr
 * Description        : This function is used to find the first
 occurrence of the string pszSource in
 the string  pszDest.
 * Return type        : SIP_CHAR
 *                      Pointer to First occurrence of Source String
 *                        in Destination String or NULL.
 * Argument           : pszDest: <In>
 *                      Target string
 * Argument           : pszSource: <In>
 *                      Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/

SIP_CHAR* SipPf_Strstr(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource)
{
    return (SIP_CHAR*)strstr(pszDest, pszSource);
}

/******************************************************************************
 * Function name    : SipPf_Strchr
 * Description        :This function is used to find the first
 occurrence of the character  ucChar in
 the string  pszDest.
 * Return type        : SIP_CHAR
 *                      Pointer to First occurrence of ucChar
 * Argument           : pszSource: <In>
 *                      Source string
 * Argument           : cChar: <In>
 *                      character to search
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/

SIP_CHAR* SipPf_Strchr(const SIP_CHAR* pszSource, SIP_CHAR cChar)
{
    return (SIP_CHAR*)strchr(pszSource, cChar);
}

/******************************************************************************
 * Function name    : SipPf_Strdup
 * Description        :  This function makes a copy of a string
 by allocating memory on the heap
 and copying pszSource to the newly
 allocated buffer.
 * Return type        : SIP_CHAR
 *                      pointer to new string
 * Argument           : pszSource: <In>
 *                      <Description>

 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/

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

/******************************************************************************
 * Function name    : SipPf_Atoi
 * Description        :This function is used to  convert
 Source String to its integer equivalent.
 * Return type        : SIP_INT32
 *                      Integer equivalent
 * Argument           : pszSource: <In>
 *                      Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/
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

SIP_UINT32 SipPf_Atoi_Unsigned(const SIP_CHAR* pszStr)
{
    if (pszStr == SIP_NULL)
    {
        return SIP_ZERO;
    }

    SIP_UINT32 nStrLen = SipPf_Strlen(pszStr);

    if (nStrLen > 10) /*2^32 = 4294967296*/
    {
        return SIP_ZERO;
    }

    SIP_UINT32 nValue = SIP_ZERO;
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

    return nValue;
}

SIP_BOOL SipPf_Atoi_IsZero(const SIP_CHAR* pszStr)
{
    if (pszStr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SIP_UINT32 nStrLen = SipPf_Strlen(pszStr);

    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nStrLen; ++nIndex)
    {
        if (pszStr[nIndex] != '0')
        {
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name    : SipPf_Itoa
 * Description        : This function is used to  convert Integer
 to    characters of String.
 * Return type        : SIP_CHAR*
 *                      pointer to String
 * Argument           : iVal: <In>
 *                      Number to be converted.
 * Argument           : pszDest: <InOut>
 *                      Pointer to String
 * Argument           : ucBase: <In>
 *                      Base of the number
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/
SIP_CHAR* SipPf_Itoa(SIP_INT32 nVal, SIP_CHAR* pszDest, SIP_INT32 nBase)
{
    if (pszDest == SIP_NULL)
    {
        return pszDest;
    }

    *pszDest = '\0';

    switch (nBase)
    {
        case 8:
            SipPf_Sprintf(pszDest, (SIP_CHAR*)"%o", nVal);
            break;
        case 10:
            SipPf_Sprintf(pszDest, (SIP_CHAR*)"%d", nVal);
            break;
        case 16:
            SipPf_Sprintf(pszDest, (SIP_CHAR*)"%x", nVal);
            break;
        default:
            break;
    }
    return pszDest;
}

/******************************************************************************
 * Function name    : SipPf_Strtok
 * Description        : This function is used to search
 destination String for tokens delimited
 by characters from Source String
 * Return type        : SIP_CHAR*
 *                      Pointer to String or NULL.
 * Argument           : pszDest: <In>
 *                      Constant Pointer to Destination String
 * Argument           : pszSource:
 Constant Pointer to source
 String
 *
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ****************************************************************************/

SIP_CHAR* SipPf_Strtok(SIP_CHAR* pszDest, const SIP_CHAR* pszSource)
{
    return strtok(pszDest, pszSource);
}

/******************************************************************************
 * Function name    : SipPf_Strrchr
 * Description    : This function locates last occurrence of a character in a
 string
 *                :
 *
 * Return type    : SIP_CHAR*

 Pointer to last occurrence of character in string
 *
 * Argument      :
 *    [IN]        : pszSource[IN] - String to parse
 *    [IN]        : cChar[IN]       - character to check.
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_CHAR* SipPf_Strrchr(SIP_CHAR* pszSrc, SIP_CHAR cChar)
{
    SIP_CHAR* pszDest = SIP_NULL;

    do
    {
        if (*pszSrc == cChar)
        {
            pszDest = (SIP_CHAR*)pszSrc;
        }

        if ((*pszSrc) == SIP_NULL)
        {
            break;
        }
        pszSrc++;

    } while (SIP_ONE);

    return (pszDest);
}

/******************************************************************************
 * Function name    : SipPfStripFileName
 * Description    : This function strips long file paths
 *                :
 *
 * Return type    : SIP_CHAR*

 Stripped file name
 *
 * Argument      :
 *    [IN]        : pFileName[IN] - Filename
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_CHAR* SipPf_StripFileName(SIP_CHAR* pszFileName)
{
    SIP_CHAR* pTemp = SIP_NULL;

    // Device file path name will come with forward slash
    pTemp = SipPf_Strrchr(pszFileName, '/');
    pTemp = pTemp + SIP_ONE;

    return pTemp;
}

SIP_BOOL SipPf_GetSystemTime(SipSt_Timestamp* pstTime)
{
    ISystemTime* pSysTime = SystemTimeService::GetSystemTimeService()->GetSystemTime();
    ImsDate Date = pSysTime->GetDate();
    ImsTime Time = pSysTime->GetLocalTime();

    pstTime->wYear = Date.nYear;
    pstTime->wMonth = Date.nMonth;
    ;
    pstTime->wDayOfWeek = 0;
    pstTime->wDay = Date.nDay;
    pstTime->wHour = Time.nHour;
    pstTime->wMinute = Time.nMinute;
    pstTime->wSecond = Time.nSecond;
    pstTime->wMilliseconds = 0;

    return SIP_TRUE;
}

SIP_VOID SipPf_GetTime(SIP_CHAR* pszTime)
{
    SipSt_Timestamp stTime;

    SipPf_GetSystemTime(&stTime);

    SipPf_Sprintf(pszTime, (SIP_CHAR*)"[%d/%d/%d][%d:%d:%d:%d]", stTime.wYear, stTime.wMonth,
            stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds);
    return;
}

SIP_VOID SipPf_GetRandomId(SIP_CHAR* pszRandomNum)
{
    SipSt_Timestamp stTime = {
            SIP_ZERO, SIP_ZERO, SIP_ZERO, SIP_ZERO, SIP_ZERO, SIP_ZERO, SIP_ZERO, SIP_ZERO};

    /* calculate sys time and Get Random Number */
    SipPf_GetSystemTime(&stTime);
    /* Algorithm for generating random number */
    SIP_INT32 nTotalMillisec = (stTime.wHour * SIP_NUM_SEC_HOURS) +
            (stTime.wHour * SIP_NUM_SEC_MIN * SIP_NUM_SEC_MIN * SIP_NUM_1000) +
            (stTime.wMinute * SIP_NUM_SEC_MIN * SIP_NUM_1000) + (stTime.wSecond * SIP_NUM_1000) +
            stTime.wMilliseconds * SipPf_Rand();
    SIP_UINT32 nIdvalue = (SipPf_Rand() + nTotalMillisec);
    SipPf_Itoa(nIdvalue, (SIP_CHAR*)pszRandomNum, SIP_TEN);
    return;
}
