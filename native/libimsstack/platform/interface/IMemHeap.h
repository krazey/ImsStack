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
#ifndef INTERFACE_MEM_HEAP_H_
#define INTERFACE_MEM_HEAP_H_

#include "ImsTypeDef.h"

class IMemHeap
{
public:
    virtual void* Alloc(IN IMS_SIZE_T nSize) = 0;
    virtual void* Realloc(IN void* pMem, IN IMS_SIZE_T nSize) = 0;
    virtual void Free(IN void* pMem) = 0;

    virtual void* AllocDebug(
            IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile) = 0;
    virtual void* ReallocDebug(IN void* pMem, IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine,
            IN const IMS_CHAR* pszFile) = 0;
    virtual void FreeDebug(IN void* pMem, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile) = 0;

    virtual void EnableHeapDebug(IN IMS_BOOL bEnable) = 0;
    virtual void PrintHeapLeakage() = 0;
    virtual void GetHeapUsage(OUT IMS_SIZE_T& nTotalBlock, OUT IMS_SIZE_T& nTotalBytes,
            OUT IMS_SIZE_T& nUsedBytes, OUT IMS_SIZE_T& nMaxUsedBytes,
            OUT IMS_SIZE_T& nMaxRequested) = 0;
};

#endif
