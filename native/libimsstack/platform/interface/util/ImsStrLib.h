/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20060102  yhrhee@                   Initial creation
    20070711  JHS                       move file and modify function name
    20080102  yhrhee@                   LH2000 Adaptation
    20090302  bluable@                  Porting
    </table>

Description
String
*/

#ifndef _IMS_STR_LIB_H_
#define _IMS_STR_LIB_H_

#define USE_SAFE_STRING

#include "IMSTypeDef.h"
#include <stdarg.h>

GLOBAL IMS_UINT32 IMS_StrLen(IN CONST IMS_CHAR* pszStr);

GLOBAL IMS_UINT32 IMS_StrCpy(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_StrNCpy(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_CHAR* pszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_UINT32 IMS_StrCat(
        OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_StrNCat(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_CHAR* pszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_SINT32 IMS_Sprintf(
        OUT IMS_CHAR* pszBuf, IN IMS_SIZE_T nBufSize, IN CONST IMS_CHAR* pszFormat, ...);

GLOBAL IMS_CHAR* IMS_StrDup(IN CONST IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_Itoa(OUT IMS_CHAR* pszStr, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase);

GLOBAL IMS_SINT32 IMS_Atoi(IN CONST IMS_CHAR* pszStr);

GLOBAL IMS_CHAR* IMS_StrChr(IN CONST IMS_CHAR* pszStr, IN IMS_CHAR cChar);

GLOBAL IMS_CHAR* IMS_StrRChr(IN CONST IMS_CHAR* pszStr, IN IMS_CHAR cChar);

GLOBAL IMS_CHAR* IMS_StrStr(IN CONST IMS_CHAR* pszA, IN CONST IMS_CHAR* pszB);

GLOBAL IMS_SINT32 IMS_StrCmp(IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB);

GLOBAL IMS_SINT32 IMS_StrNCmp(
        IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize);

GLOBAL IMS_SINT32 IMS_StrICmp(IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB);

GLOBAL IMS_SINT32 IMS_StrNICmp(
        IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB, IN IMS_SIZE_T nSize);

GLOBAL IMS_SINT32 IMS_StrCmpUppercase(IN CONST IMS_CHAR* pszStrA, IN CONST IMS_CHAR* pszStrB);

GLOBAL IMS_BOOL IMS_StrToLowerCase(OUT IMS_CHAR* pszOutStr, IN CONST IMS_CHAR* pszInStr);

GLOBAL IMS_BOOL IMS_StrToUpperCase(OUT IMS_CHAR* pszOutStr, IN CONST IMS_CHAR* pszInStr);

//
// APIs for unicode string manipulation
//
GLOBAL IMS_UINT32 IMS_UcStrLen(IN CONST IMS_WCHAR* pwszStr);

GLOBAL IMS_UINT32 IMS_StrToUcStr(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_CHAR* pszSrc);

GLOBAL IMS_UINT32 IMS_UcStrToStr(OUT IMS_CHAR* pszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_WCHAR* pwszSrc, IN IMS_CHAR cDefaultChar);

GLOBAL IMS_UINT32 IMS_UcStrCpy(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_WCHAR* pwszSrc);

GLOBAL IMS_UINT32 IMS_UcStrNCpy(OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_WCHAR* pwszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_UINT32 IMS_UcStrCat(
        OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize, IN CONST IMS_WCHAR* pwszSrc);

GLOBAL IMS_UINT32 IMS_UcStrNCat(OUT IMS_WCHAR* pwszDest, IN IMS_SIZE_T nDestSize,
        IN CONST IMS_WCHAR* pwszSrc, IN IMS_SIZE_T nSrcSize);

GLOBAL IMS_SINT32 IMS_UcSprintf(OUT IMS_WCHAR* pwszBuf, IN IMS_SIZE_T nBufSize,
        IN CONST IMS_WCHAR* pwszFormat, IN va_list args);

GLOBAL IMS_UINT32 IMS_UcItoa(
        OUT IMS_WCHAR* pwszBuf, IN IMS_SIZE_T nBufSize, IN IMS_SINT32 nNum, IN IMS_UINT32 nBase);

GLOBAL IMS_SINT32 IMS_UcAtoi(IN CONST IMS_WCHAR* pwszStr);

GLOBAL IMS_WCHAR* IMS_UcStrStr(IN CONST IMS_WCHAR* pwszA, IN CONST IMS_WCHAR* pwszB);

GLOBAL IMS_SINT32 IMS_UcStrCmp(IN CONST IMS_WCHAR* pwszStrA, IN CONST IMS_WCHAR* pwszStrB);

GLOBAL IMS_SINT32 IMS_UcStrNCmp(
        IN CONST IMS_WCHAR* pwszStrA, IN CONST IMS_WCHAR* pwszStrB, IN IMS_SIZE_T nSize);

GLOBAL IMS_BOOL IMS_UcStrToLowerCase(OUT IMS_WCHAR* pwszOutStr, IN CONST IMS_WCHAR* pwszInStr);

GLOBAL IMS_BOOL IMS_UcStrToUpperCase(OUT IMS_WCHAR* pwszOutStr, IN CONST IMS_WCHAR* pwszInStr);

GLOBAL IMS_SINT32 IMS_UcStrCmpUppercase(IN CONST IMS_WCHAR* pwszStrA, IN CONST IMS_WCHAR* pwszStrB);

GLOBAL IMS_UINT32 IMS_Utf8ToUcs(OUT IMS_WCHAR* pwszUcs, IN CONST IMS_CHAR* pszUtf8);

GLOBAL IMS_UINT32 IMS_UcsToUtf8(OUT IMS_CHAR* pszUtf8, IN CONST IMS_WCHAR* pwszUcs);

GLOBAL IMS_UINT32 IMS_Utf8ToEuckr(OUT IMS_CHAR* pszEuckr, IN CONST IMS_CHAR* pszUtf8);

GLOBAL IMS_UINT32 IMS_EuckrToUtf8(OUT IMS_CHAR* pszUtf8, IN CONST IMS_CHAR* pszEuckr);

GLOBAL IMS_UINT32 IMS_UcsToEuckr(
        OUT IMS_CHAR* pszEuckr, IN IMS_SIZE_T nEuckrSize, IN CONST IMS_WCHAR* pwszUcs);

GLOBAL IMS_UINT32 IMS_EuckrToUcs(
        OUT IMS_WCHAR* pwszUcs, IN IMS_SIZE_T nUcsSize, IN CONST IMS_CHAR* pszEuckr);

#endif  // _IMS_STR_LIB_H_
