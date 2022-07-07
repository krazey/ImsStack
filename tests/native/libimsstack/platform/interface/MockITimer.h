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

#ifndef MOCK_I_TIMER_H_
#define MOCK_I_TIMER_H_

#include <gmock/gmock.h>
#include "ImsTypeDef.h"
#include "ITimer.h"

class ITimerListener;

class MockITimer : public ITimer
{
public:
    MOCK_METHOD(IMS_BOOL, Equals, (IN const ITimer*), (const, override));
    MOCK_METHOD(IMS_UINTP, SetTimer, (IN IMS_UINT32, IN ITimerListener*), (override));
    MOCK_METHOD(void, KillTimer, (), (override));
};

class MockITimerListener : public ITimerListener
{
public:
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer*), (override));
};

#endif // MOCK_I_TIMER_H_
