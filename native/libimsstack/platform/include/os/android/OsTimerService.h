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
#ifndef OS_TIMER_SERVICE_H_
#define OS_TIMER_SERVICE_H_

#include "ImsList.h"
#include "OsMutex.h"
#include "system-intf/ISystemListener.h"

class OsTimer;
class OsTimerWrapper;

class OsTimerService : public ISystemListener
{
public:
    OsTimerService();
    virtual ~OsTimerService();

    OsTimerService(IN const OsTimerService&) = delete;
    OsTimerService& operator=(IN const OsTimerService&) = delete;

public:
    void KillTimer(IN OsTimer* pTimer);
    IMS_BOOL SetTimer(IN IMS_UINT32 nDuration, IN OsTimer* pTimer);

    static void CleanUp();
    static void StartUp();
    static OsTimerService* GetTimerService();

private:
    // ISystemListener class
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;
    void NotifyTimerExpired(IN OsTimerWrapper* pTimerWrapper);

private:
    static OsTimerService* s_pTimerService;

    OsMutex m_objLockTimer;
    IMSList<OsTimerWrapper*> m_objTimers;
};

#endif
