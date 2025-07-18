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
#ifndef SERVICE_MUTEX_H_
#define SERVICE_MUTEX_H_

#include "AString.h"
#include "IMutex.h"
#include "PlatformService.h"

class MutexService : public PlatformService
{
public:
    MutexService();
    MutexService(IN const MutexService&) = delete;
    MutexService& operator=(IN const MutexService&) = delete;

protected:
    ~MutexService() override;

public:
    virtual IMutex* CreateMutex(IN const AString& strName = AString::ConstNull());
    virtual void DestroyMutex(IN IMutex*& piMutex);

    static MutexService* GetMutexService();
};

#endif
