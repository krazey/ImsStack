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
#ifndef SERVICE_SYSTEM_TIME_H_
#define SERVICE_SYSTEM_TIME_H_

#include "ISystemTime.h"
#include "PlatformService.h"

class SystemTimeServicePrivate;

class SystemTimeService : public PlatformService
{
public:
    SystemTimeService();
    SystemTimeService(IN const SystemTimeService&) = delete;
    SystemTimeService& operator=(IN const SystemTimeService&) = delete;

protected:
    virtual ~SystemTimeService();

public:
    virtual ISystemTime* GetSystemTime();

    static SystemTimeService* GetSystemTimeService();

private:
    SystemTimeServicePrivate* m_pPrivate;
};

#define IMS_SYS_GetDate() SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetDate()

#define IMS_SYS_GetLocalTime() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetLocalTime()

#define IMS_SYS_GetRandom0() SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom()

#define IMS_SYS_GetRandom(R) \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom(R)

#define IMS_SYS_GetSRandom0() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom()

#define IMS_SYS_GetSRandom(R) \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom(R)

#define IMS_SYS_GetTickCount() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTickCount()

#define IMS_SYS_GetTimeInSeconds() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeInSeconds()

#define IMS_SYS_GetTimeInMicroSeconds() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeInMicroSeconds()

#define IMS_SYS_GetTimeInMilliSeconds() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeInMilliSeconds()

#define IMS_SYS_GetTimeString() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeString()

#define IMS_SYS_GetTimeStringEx() \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeStringEx()

#define IMS_SYS_Sleep(MS) SystemTimeService::GetSystemTimeService()->GetSystemTime()->Sleep(MS)

#define IMS_SYS_GetDiffGmTime(BEGIN, END) \
    SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetDiffGmTime(BEGIN, END);

#endif
