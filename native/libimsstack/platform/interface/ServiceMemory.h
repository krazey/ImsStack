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

#include "IMemHeap.h"
#include "IMSNew.h"

#ifdef IMS_DEBUG_MEM

#define IMS_MEM_Malloc(SIZE) \
        MemService::GetMemService()->GetMemHeap()->AllocDebug(SIZE,__IMS_LINE__,__IMS_FILE__)

#define IMS_MEM_Realloc(MEM, SIZE) \
        MemService::GetMemService()->GetMemHeap()->ReallocDebug(MEM,SIZE,__IMS_LINE__,__IMS_FILE__)

#define IMS_MEM_Free(MEM) \
        MemService::GetMemService()->GetMemHeap()->FreeDebug(MEM,__IMS_LINE__,__IMS_FILE__)

#else // IMS_DEBUG_MEM

#define IMS_MEM_Malloc(SIZE) \
        MemService::GetMemService()->GetMemHeap()->Alloc(SIZE)

#define IMS_MEM_Realloc(MEM, SIZE) \
        MemService::GetMemService()->GetMemHeap()->Realloc(MEM, SIZE)

#define IMS_MEM_Free(MEM) \
        MemService::GetMemService()->GetMemHeap()->Free(MEM)

#endif // IMS_DEBUG_MEM

class MemService
{
private:
    MemService();
    ~MemService();

public:
    MemService(IN const MemService&) = delete;
    MemService& operator=(IN const MemService&) = delete;

public:
    IMemHeap* GetMemHeap();

    static MemService* GetMemService();
};

#endif
