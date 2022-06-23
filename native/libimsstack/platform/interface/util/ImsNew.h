/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100128  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMS_NEW_H_
#define _IMS_NEW_H_

#include "IMSTypeDef.h"

// 160409: "noexcept" specifier is introduced since c++ 11
#if defined(__IMS_CLANG__)
#define IMS_NO_EXCEPTION noexcept
#else
#define IMS_NO_EXCEPTION
#endif

// clang-format off
#ifdef IMS_DEBUG_MEM

GLOBAL void* operator new(IN IMS_SIZE_T nSize, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine);
GLOBAL void* operator new[](IN IMS_SIZE_T nSize, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine);
GLOBAL void operator delete(
        IN void* pMem, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION;
GLOBAL void operator delete[](
        IN void* pMem, IN CONST IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION;
GLOBAL void operator delete(IN void* pMem) IMS_NO_EXCEPTION;
GLOBAL void operator delete[](IN void* pMem) IMS_NO_EXCEPTION;

#define DBG_NEW new (__IMS_FILE__, __IMS_LINE__)
#define new DBG_NEW

#define DBG_DELETE delete (__IMS_FILE__, __IMS_LINE__)
#define delete DBG_DELETE

#else  // IMS_DEBUG_MEM

GLOBAL void* operator new(IN IMS_SIZE_T nSize);
GLOBAL void* operator new[](IN IMS_SIZE_T nSize);
GLOBAL void operator delete(IN void* pMem) IMS_NO_EXCEPTION;
GLOBAL void operator delete[](IN void* pMem) IMS_NO_EXCEPTION;

#define new new
#define delete delete
#endif  // IMS_DEBUG_MEM

GLOBAL void* IMS_MEM_Memset(IN_OUT void* pvMem, IN IMS_SINT32 nChar, IN IMS_SIZE_T nCount);
GLOBAL void* IMS_MEM_Memcpy(IN_OUT void* pvDest, IN CONST void* pvSrc, IN IMS_SIZE_T nCount);
GLOBAL void* IMS_MEM_Memmove(IN_OUT void* pvDest, IN CONST void* pvSrc, IN IMS_SIZE_T nCount);
GLOBAL IMS_SINT32 IMS_MEM_Memcmp(
        IN CONST void* pvMem1, IN CONST void* pvMem2, IN IMS_SIZE_T nCount);

// clang-format on

#endif  // _IMS_NEW_H_
