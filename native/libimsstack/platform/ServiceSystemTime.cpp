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
#include "ServiceSystemTime.h"

class SystemTimeServicePrivate
{
public:
    inline SystemTimeServicePrivate()
        : m_piSysTime(IMS_NULL)
    {}
    inline ~SystemTimeServicePrivate()
    {
        PlatformFactory::DestroySystemTime(m_piSysTime);
    }

    SystemTimeServicePrivate(IN const SystemTimeServicePrivate&) = delete;
    SystemTimeServicePrivate& operator=(IN const SystemTimeServicePrivate&) = delete;

public:
    inline ISystemTime* GetSystemTime()
    {
        if (m_piSysTime == IMS_NULL)
        {
            m_piSysTime = PlatformFactory::CreateSystemTime();
        }

        return m_piSysTime;
    }

private:
    ISystemTime* m_piSysTime;
};



PRIVATE
SystemTimeService::SystemTimeService()
    : m_pPrivate(new SystemTimeServicePrivate())
{
}

PRIVATE
SystemTimeService::~SystemTimeService()
{
    delete m_pPrivate;
}

PUBLIC
ISystemTime* SystemTimeService::GetSystemTime()
{
    return m_pPrivate->GetSystemTime();
}

PUBLIC GLOBAL
SystemTimeService* SystemTimeService::GetSystemTimeService()
{
    static SystemTimeService* s_pSystemService = IMS_NULL;

    if (s_pSystemService == IMS_NULL)
    {
        s_pSystemService = new SystemTimeService();
    }

    return s_pSystemService;
}
