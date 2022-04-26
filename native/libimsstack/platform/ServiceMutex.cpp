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
#include "ImsMutex.h"
#include "PlatformFactory.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"

PUBLIC
MutexService::MutexService()
{
}

PUBLIC
MutexService::~MutexService()
{
}

PUBLIC
IMutex* MutexService::CreateMutex(IN const AString& strName /*= AString::ConstNull()*/)
{
    ImsMutex* pMutex = PlatformFactory::CreateMutex(strName);

    IMS_ASSERT(pMutex != IMS_NULL);

    return pMutex;
}

PUBLIC
void MutexService::DestroyMutex(IN IMutex*& piMutex)
{
    ImsMutex* pMutex = DYNAMIC_CAST(ImsMutex*, piMutex);

    if (pMutex != IMS_NULL)
    {
        delete pMutex;
        piMutex = IMS_NULL;
    }
}

PUBLIC GLOBAL
MutexService* MutexService::GetMutexService()
{
    static MutexService* s_pMutexService = IMS_NULL;

    if (s_pMutexService == IMS_NULL)
    {
        s_pMutexService = new MutexService();
    }

    return s_pMutexService;
}
