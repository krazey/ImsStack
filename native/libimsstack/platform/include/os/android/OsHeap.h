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
#ifndef OS_HEAP_H_
#define OS_HEAP_H_

#include "IMemHeap.h"

class OsHeap : public IMemHeap
{
private:
    OsHeap();
    virtual ~OsHeap();

public:
    void* Alloc(IN IMS_SIZE_T nSize) override;

    void* Realloc(IN void* pMem, IN IMS_SIZE_T nSize) override;

    void Free(IN void* pMem) override;

    void* AllocDebug(IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile) override;

    void* ReallocDebug(IN void* pMem, IN IMS_SIZE_T nSize, IN IMS_UINT16 nLine,
            IN const IMS_CHAR* pszFile) override;

    void FreeDebug(IN void* pMem, IN IMS_UINT16 nLine, IN const IMS_CHAR* pszFile) override;

    void EnableHeapDebug(IN IMS_BOOL bEnable) override;

    void PrintHeapLeakage() override;

    void GetHeapUsage(OUT IMS_SIZE_T& nTotalBlock, OUT IMS_SIZE_T& nTotalBytes,
            OUT IMS_SIZE_T& nUsedBytes, OUT IMS_SIZE_T& nMaxUsedBytes,
            OUT IMS_SIZE_T& nMaxRequested) override;

    static OsHeap* GetHeap();
    static void Initialize();

private:
    IMS_BOOL m_bDebugEnabled;
};

#endif
