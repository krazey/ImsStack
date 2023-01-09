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

#ifndef MOCK_QOS_TIMER_H_
#define MOCK_QOS_TIMER_H_

#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "ServiceTimer.h"
#include "precondition/IQosTimerListener.h"
#include "precondition/QosTimer.h"
#include <gmock/gmock.h>

enum class QosTimerType;

class MockQosTimer : public QosTimer
{
public:
    explicit MockQosTimer(IN IQosTimerListener* pListener) :
            QosTimer(pListener)
    {
    }
    ~MockQosTimer() {}

    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piExpiredTimer), (override));
    MOCK_METHOD(void, StartQosTimer, (IN QosTimerType eType, IN IMS_SINT32 nDuration), (override));
    MOCK_METHOD(void, StopQosTimer, (IN QosTimerType eType), (override));
    MOCK_METHOD(IMS_BOOL, IsQosTimerActivated, (IN QosTimerType eType), (override));
};

#endif
