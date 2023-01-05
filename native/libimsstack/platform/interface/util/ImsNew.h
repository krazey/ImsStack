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
#ifndef IMS_NEW_H_
#define IMS_NEW_H_

#include "ImsTypeDef.h"

// "noexcept" specifier is introduced since c++ 11
#if defined(__IMS_CLANG__)
#define IMS_NO_EXCEPTION noexcept
#else
#define IMS_NO_EXCEPTION
#endif

// clang-format off
#ifdef IMS_DEBUG_MEM

GLOBAL void* operator new(IN IMS_SIZE_T nSize, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine);
GLOBAL void* operator new[](IN IMS_SIZE_T nSize, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine);
GLOBAL void operator delete(
        IN void* pMem, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION;
GLOBAL void operator delete[](
        IN void* pMem, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION;
GLOBAL void operator delete(IN void* pMem) IMS_NO_EXCEPTION;
GLOBAL void operator delete[](IN void* pMem) IMS_NO_EXCEPTION;

#define DBG_NEW     new (__IMS_FILE__, __IMS_LINE__)
#define new         DBG_NEW

#define DBG_DELETE  delete (__IMS_FILE__, __IMS_LINE__)
#define delete      DBG_DELETE

#else  // IMS_DEBUG_MEM

GLOBAL void* operator new(IN IMS_SIZE_T nSize);
GLOBAL void* operator new[](IN IMS_SIZE_T nSize);
GLOBAL void operator delete(IN void* pMem) IMS_NO_EXCEPTION;
GLOBAL void operator delete[](IN void* pMem) IMS_NO_EXCEPTION;

#define new new
#define delete delete
#endif  // IMS_DEBUG_MEM

GLOBAL void* IMS_MEM_Memset(IN_OUT void* pvMem, IN IMS_SINT32 nChar, IN IMS_SIZE_T nCount);
GLOBAL void* IMS_MEM_Memcpy(IN_OUT void* pvDst, IN const void* pvSrc, IN IMS_SIZE_T nCount);
GLOBAL void* IMS_MEM_Memmove(IN_OUT void* pvDst, IN const void* pvSrc, IN IMS_SIZE_T nCount);
GLOBAL IMS_SINT32 IMS_MEM_Memcmp(
        IN const void* pvMem1, IN const void* pvMem2, IN IMS_SIZE_T nCount);

// clang-format on

#endif
