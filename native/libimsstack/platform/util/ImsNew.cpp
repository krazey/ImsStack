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
#include <string.h>

#include "ImsNew.h"
#include "ServiceMemory.h"

#undef new
#undef delete

// clang-format off
#ifdef IMS_DEBUG_MEM

GLOBAL
void* operator new(IN IMS_SIZE_T nSize, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine)
{
    return MemService::AllocDebug(nSize, nLine, pszFile);
}

GLOBAL
void* operator new[](IN IMS_SIZE_T nSize, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine)
{
    return operator new(nSize, nLine, pszFile);
}

GLOBAL
void operator delete(
        IN void* pMem, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    MemService::FreeDebug(pMem, nLine, pszFile);
}

GLOBAL
void operator delete[](
        IN void* pMem, IN const IMS_CHAR* pszFile, IN IMS_UINT16 nLine) IMS_NO_EXCEPTION
{
    operator delete(pMem, nLine, pszFile);
}

GLOBAL
void operator delete(IN void* pMem) IMS_NO_EXCEPTION
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    MemService::FreeDebug(pMem, __IMS_LINE__, __IMS_FILE__);
}

GLOBAL
void operator delete[](IN void* pMem) IMS_NO_EXCEPTION
{
    operator delete(pMem);
}

#else   // IMS_DEBUG_MEM

GLOBAL
void* operator new(IN IMS_SIZE_T nSize)
{
    return MemService::Alloc(nSize);
}

GLOBAL
void* operator new[](IN IMS_SIZE_T nSize)
{
    return operator new(nSize);
}

GLOBAL
void operator delete(IN void* pMem) IMS_NO_EXCEPTION
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    MemService::Free(pMem);
}

GLOBAL
void operator delete[](IN void* pMem) IMS_NO_EXCEPTION
{
    operator delete(pMem);
}
#endif  // IMS_DEBUG_MEM

GLOBAL void* IMS_MEM_Memset(IN_OUT void* pvMem, IN IMS_SINT32 nChar, IN IMS_SIZE_T nCount)
{
    return memset(pvMem, nChar, nCount);
}

GLOBAL void* IMS_MEM_Memcpy(IN_OUT void* pvDst, IN const void* pvSrc, IN IMS_SIZE_T nCount)
{
    return memcpy(pvDst, pvSrc, nCount);
}

GLOBAL void* IMS_MEM_Memmove(IN_OUT void* pvDst, IN const void* pvSrc, IN IMS_SIZE_T nCount)
{
    return memmove(pvDst, pvSrc, nCount);
}

GLOBAL IMS_SINT32 IMS_MEM_Memcmp(IN const void* pvMem1, IN const void* pvMem2, IN IMS_SIZE_T nCount)
{
    return memcmp(pvMem1, pvMem2, nCount);
}
// clang-format on
