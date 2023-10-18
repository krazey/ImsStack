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
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceSystemTime.h"

class SystemTimeServicePrivate
{
public:
    inline SystemTimeServicePrivate() :
            m_piSysTime(IMS_NULL)
    {
    }
    inline ~SystemTimeServicePrivate()
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        piOsFactory->DestroySystemTime(m_piSysTime);
    }

    SystemTimeServicePrivate(IN const SystemTimeServicePrivate&) = delete;
    SystemTimeServicePrivate& operator=(IN const SystemTimeServicePrivate&) = delete;

public:
    inline ISystemTime* GetSystemTime()
    {
        if (m_piSysTime == IMS_NULL)
        {
            IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
            m_piSysTime = piOsFactory->CreateSystemTime();
        }

        return m_piSysTime;
    }

private:
    ISystemTime* m_piSysTime;
};

PRIVATE
SystemTimeService::SystemTimeService() :
        m_pPrivate(new SystemTimeServicePrivate())
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

PUBLIC GLOBAL SystemTimeService* SystemTimeService::GetSystemTimeService()
{
    return DYNAMIC_CAST(SystemTimeService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_SYSTEM_TIME));
}
