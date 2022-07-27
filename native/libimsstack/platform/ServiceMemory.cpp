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
#include <stdlib.h>

#include "ServiceMemory.h"

PUBLIC
void* MemService::Alloc(IN IMS_SIZE_T nSize)
{
    return malloc(nSize);
}

PUBLIC
void* MemService::Realloc(IN void* pMem, IN IMS_SIZE_T nSize)
{
    return realloc(pMem, nSize);
}

PUBLIC
void MemService::Free(IN void* pMem)
{
    if (pMem == IMS_NULL)
    {
        return;
    }

    free(pMem);
    pMem = IMS_NULL;
}

PUBLIC
void* MemService::AllocDebug(IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile)
{
    (void)nLine;
    (void)pszFile;

    return Alloc(nSize);
}

PUBLIC
void* MemService::ReallocDebug(
        IN void* pMem, IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile)
{
    (void)nLine;
    (void)pszFile;

    return Realloc(pMem, nSize);
}

PUBLIC
void MemService::FreeDebug(IN void* pMem, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile)
{
    (void)nLine;
    (void)pszFile;

    Free(pMem);
}
