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
#ifndef TEST_TIMER_SERVICE_H_
#define TEST_TIMER_SERVICE_H_

#include "MockITimer.h"
#include "ServiceTimer.h"

class TestTimerService : public TimerService
{
public:
    inline TestTimerService() :
            TimerService(),
            m_piTimer(&m_objTimer)
    {
    }

    inline ITimer* CreateTimer() override { return m_piTimer; }
    inline void DestroyTimer(
            IN ITimer*& /*piTimer*/, IN IMS_BOOL /*bOnOwnerThread = IMS_TRUE*/) override
    {
    }

    inline MockITimer& GetMockTimer() { return m_objTimer; }
    inline void SetTimer(IN ITimer* piTimer) { m_piTimer = piTimer; }

private:
    MockITimer m_objTimer;

    ITimer* m_piTimer;
};

#endif
