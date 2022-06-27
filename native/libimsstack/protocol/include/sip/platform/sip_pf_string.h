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
#ifndef __SIP_PF_STRING_H__
#define __SIP_PF_STRING_H__

#include "sip_pf_datatypes.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIP_TIME_MAX_SIZE 100
#define SIP_NUM_SEC_HOURS 100000000
#define SIP_NUM_SEC_MIN   60
#define SIP_NUM_1000      1000

#define SIP_TOLOWER(c)    ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))
#define SIP_TOUPPER(c)    ((((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c))

/* Structure of SystemTimer */
typedef struct _SipSt_Timestamp
{
    SIP_UINT16 wYear;
    SIP_UINT16 wMonth;
    SIP_UINT16 wDayOfWeek;
    SIP_UINT16 wDay;
    SIP_UINT16 wHour;
    SIP_UINT16 wMinute;
    SIP_UINT16 wSecond;
    SIP_UINT16 wMilliseconds;
} SipSt_Timestamp;

void SipPf_Snprintf(SIP_CHAR* pszBuffer, SIP_UINT32 nBuffSize, const SIP_CHAR* pszFormat, ...);
void SipPf_Sprintf(SIP_CHAR* pszBuffer, const SIP_CHAR* pszFormat, ...);
void SipPf_Printf(SIP_CHAR* pszFormat, ...);
void SipPf_Sscanf(SIP_CHAR* pszBuffer, SIP_CHAR* pszFormat, SIP_CHAR* pszCharAdd);
SIP_UINT32 SipPf_Rand();

/************************************************************
 * Function name    : SipPf_Strlen
 * Description        :  This function gets the length of
 Source string
 * Return type        : SipDt _Int32
 *                      Length of Source string
 * Argument           : pcSource: IN
 *                      Input string

 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_INT32 SipPf_Strlen(const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Strcpy
 * Description        :  This function copies source string to
 destination string including '\0'
 * Return type        : SipDt _Char *
 *                      Pointer to destination string
 * Argument           : pcDest: InOut
 *                      Destination string
 * Argument           : pcSource: In
 *                      Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/

SIP_CHAR* SipPf_Strcpy(SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Strncpy
 * Description        :  This function copies at most iNumChars
 from kpcSource to pcDest and terminates
 pcDest with '\0'
 * Return type        : SIP_CHAR *
 *                      Pointer to pcDest
 * Argument           : pcDest: <InOut>
 *                      Destination string
 * Argument           : pcSource: <In>
 *                      Source string
 * Argument           : iNumChars: <In>
 *                      Number of characters to copy
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/

SIP_CHAR* SipPf_Strncpy(SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);
/************************************************************
 * Function name    : SipPf_Strcat
 * Description        : This function concatenates source string
 to end of destination string.
 * Return type        : SIP_CHAR *
 *                      Pointer to pcDest
 * Argument           : pcDest: <InOut>
 *                      Destination string
 * Argument           : pcSource: <In>
 *                      <Source string>
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_CHAR* SipPf_Strcat(SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Strncat
 * Description        : This function concatenates ssNumChars
 of Source string to end of destination
 string
 * Return type        : SIP_CHAR*
 *                      Pointer to pcDest
 * Argument           : pcDest: <InOut>
 *                      Destination string
 * Argument           : pcSource: <In>
 *                      Source string
 * Argument           : ssNumChars: <In>
 *                      Number of characters to concatenate
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_CHAR* SipPf_Strncat(SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);

/************************************************************
 * Function name    : SipPf_Strcmp
 * Description        : This function compares source string
 with target string
 * Return type        : SIP_INT16
 *                      greater than 0 if pcDest > pcSource
 o if strings are same
 less than 0 if pcDest < pcSource
 * Argument           : pcDest: <In>
 *                      Source string
 * Argument           : pcSource: <In>
 *                      Target string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_INT16 SipPf_Strcmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Stricmp
 * Description        :  This function is used to Compare Source
 String with    Target String.
 * Return type        : SIP_INT16
 *                      greater than 0 if pcDest > pcSource
 o if strings are same
 less than 0 if pcDest < pcSource
 * Argument           : pcDest: <In>
 *                      Source string
 * Argument           : pcSource: <In>
 *                      Target string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_INT16 SipPf_Stricmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Strncmp
 * Description        :  This function compares specified number
 of characters of the passed strings.
 * Return type        : SIP_INT16
 *                          0 if strings are similar.
 *                            +ve integer if pcSource >pcDest
 *                            -ve integer if pcSource >pcDest
 * Argument           : pcDest: <In>
 *                      Target string
 * Argument           : pcSource: <In>
 *                        Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_INT16 SipPf_Strncmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);

/************************************************************
 * Function name    : SipPf_Strnicmp
 * Description        :  This function compares specified number
 of characters of the passed strings(case insensitive).
 * Return type        : SIP_INT16
 *                          0 if strings are similar.
 *                            +ve integer if pszSource >pszDest
 *                            -ve integer if pszSource >pszDest
 * Argument           : pszDest: <In>
 *                      Target string
 * Argument           : pszSource: <In>
 *                        Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_INT16 SipPf_Strnicmp(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource, SIP_UINT32 nNumChars);

/************************************************************
 * Function name    : SipPf_Strstr
 * Description        : This function is used to find the first
 occurrence of the string pcSource in
 the string  pcDest.
 * Return type        : SIP_CHAR
 *                      Pointer to First occurrence of Source String
 *                        in Destination String or NULL.
 * Argument           : pcDest: <In>
 *                      Target string
 * Argument           : pcSource: <In>
 *                      Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/

SIP_CHAR* SipPf_Strstr(const SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Strchr
 * Description        :This function is used to find the first
 occurrence of the character  ucChar in
 the string  pcDest.
 * Return type        : SIP_CHAR
 *                      Pointer to First occurrence of ucChar
 * Argument           : pcSource: <In>
 *                      Source string
 * Argument           : cChar: <In>
 *                      character to search
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_CHAR* SipPf_Strchr(const SIP_CHAR* pszSource, SIP_CHAR cChar);
/************************************************************
 * Function name    : SipPf_Strdup
 * Description        :  This function makes a copy of a string
 by allocating memory on the heap
 and copying pcSource to the newly
 allocated buffer.
 * Return type        : SIP_CHAR
 *                      pointer to new string
 * Argument           : pcSource: <In>
 *                      <Description>

 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_CHAR* SipPf_Strdup(const SIP_CHAR* pszSource);

/************************************************************
 * Function name    : SipPf_Atoi
 * Description        :This function is used to  convert
 Source String to its integer equivalent.
 * Return type        : SIP_INT32
 *                      Integer equivalent
 * Argument           : pcSource: <In>
 *                      Source string
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_INT32 SipPf_Atoi(const SIP_CHAR* pszSource);

SIP_UINT32 SipPf_Atoi_Unsigned(const SIP_CHAR* pszStr);

SIP_BOOL SipPf_Atoi_IsZero(const SIP_CHAR* pszStr);
/************************************************************
 * Function name    : SipPf_Itoa
 * Description        : This function is used to  convert Integer
 to    characters of String.
 * Return type        : SIP_CHAR*
 *                      pointer to String
 * Argument           : iVal: <In>
 *                      Number to be converted.
 * Argument           : pcDest: <InOut>
 *                      Pointer to String
 * Argument           : ucBase: <In>
 *                      Base of the number
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_CHAR* SipPf_Itoa(SIP_INT32 nVal, SIP_CHAR* pszDest, SIP_INT32 nBase);
/************************************************************
 * Function name    : SipPf_Strtok
 * Description        : This function is used to search
 destination String for tokens delimited
 by characters from Source String
 * Return type        : SIP_CHAR*
 *                      Pointer to String or NULL.
 * Argument           : pcDest: <In>
 *                      Constant Pointer to Destination String
 * Argument           : pcSource:
 Constant Pointer to source
 String
 *
 * Preconditions/
 * Side Effects    : Preconditions for the function to be
 *                       required, argument list that will be
 *                       updated
 *
 ************************************************************/
SIP_CHAR* SipPf_Strtok(SIP_CHAR* pszDest, const SIP_CHAR* pszSource);

/******************************************************************************
 * Function name    : SIP_Strrchr
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
SIP_CHAR* SipPf_Strrchr(SIP_CHAR* pszSource, SIP_CHAR cChar);
/******************************************************************************
 * Function name    : SIP_StripFileName
 * Description    : This function strips long file paths
 *                :
 *
 * Return type    : SIP_CHAR*

 Stripped file name
 *
 * Argument      :
 *    [IN]        : pcFileName[IN] - Filename
 *
 * Side Effects    :
 * NOTE             :
 ******************************************************************************/
SIP_CHAR* SipPf_StripFileName(SIP_CHAR* pszFileName);

SIP_BOOL SipPf_GetSystemTime(SipSt_Timestamp* pstTime);

SIP_VOID SipPf_GetTime(SIP_CHAR* pszTime);

SIP_VOID SipPf_GetRandomId(SIP_CHAR* pszRandomNum);

#endif /*__SIP_PF_STRING_H__ */
