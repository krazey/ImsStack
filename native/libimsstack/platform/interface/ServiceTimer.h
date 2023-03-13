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
#ifndef SERVICE_TIMER_H_
#define SERVICE_TIMER_H_

#include "ITimer.h"
#include "ImsList.h"
#include "ImsMessage.h"
#include "PlatformService.h"

class IMutex;

class TimerService : public PlatformService
{
public:
    TimerService();
    TimerService(IN const TimerService&) = delete;
    TimerService& operator=(IN const TimerService&) = delete;

protected:
    virtual ~TimerService();

public:
    virtual ITimer* CreateTimer();
    virtual void DestroyTimer(IN ITimer*& piTimer, IN IMS_BOOL bOnOwnerThread = IMS_TRUE);

    void DispatchServiceMessage(IN ImsMessage& objMsg);

    static TimerService* GetTimerService();

private:
    IMutex* m_piLock;
    ImsList<ITimer*> m_objTimers;
};

#endif
