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
#ifndef SERVICE_MEMORY_H_
#define SERVICE_MEMORY_H_

#include "ImsNew.h"

#ifdef IMS_DEBUG_MEM

#define IMS_MEM_Malloc(SIZE)       MemService::AllocDebug(SIZE, __IMS_LINE__, __IMS_FILE__)

#define IMS_MEM_Realloc(MEM, SIZE) MemService::ReallocDebug(MEM, SIZE, __IMS_LINE__, __IMS_FILE__)

#define IMS_MEM_Free(MEM)          MemService::FreeDebug(MEM, __IMS_LINE__, __IMS_FILE__)

#else  // IMS_DEBUG_MEM

#define IMS_MEM_Malloc(SIZE)       MemService::Alloc(SIZE)

#define IMS_MEM_Realloc(MEM, SIZE) MemService::Realloc(MEM, SIZE)

#define IMS_MEM_Free(MEM)          MemService::Free(MEM)

#endif  // IMS_DEBUG_MEM

class MemService
{
public:
    MemService() = delete;
    MemService(IN const MemService&) = delete;
    MemService& operator=(IN const MemService&) = delete;

public:
    static void* Alloc(IN IMS_SIZE_T nSize);
    static void* Realloc(IN void* pMem, IN IMS_SIZE_T nSize);
    static void Free(IN void* pMem);
    static void* AllocDebug(IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile);
    static void* ReallocDebug(
            IN void* pMem, IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile);
    static void FreeDebug(IN void* pMem, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile);
};

#endif
