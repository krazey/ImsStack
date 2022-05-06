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
#include "PlatformFactory.h"
#include "ServiceMemory.h"

PRIVATE
MemService::MemService() {}

PRIVATE
MemService::~MemService() {}

PUBLIC
IMemHeap* MemService::GetMemHeap()
{
    IMemHeap* piHeap = PlatformFactory::GetHeap();

    IMS_ASSERT(piHeap != IMS_NULL);

    return piHeap;
}

PUBLIC GLOBAL MemService* MemService::GetMemService()
{
    static MemService* s_pMemService = IMS_NULL;

    if (s_pMemService == IMS_NULL)
    {
        IMemHeap* piHeap = PlatformFactory::GetHeap();
        void* pvMemService = piHeap->Alloc(sizeof(MemService));

        s_pMemService = new (pvMemService) MemService();
    }

    return s_pMemService;
}
