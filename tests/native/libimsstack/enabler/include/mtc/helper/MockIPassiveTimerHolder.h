/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef MOCK_I_PASSIVE_TIMER_HOLDER_H_
#define MOCK_I_PASSIVE_TIMER_HOLDER_H_

#include "ImsTypeDef.h"
#include "helper/IPassiveTimerHolder.h"
#include <gmock/gmock.h>

class MockIPassiveTimerHolder : public IPassiveTimerHolder
{
public:
    virtual ~MockIPassiveTimerHolder() = default;

    MOCK_METHOD(
            void, AddTimer, (IN IPassiveTimerHolder::Type, IN IMS_SINT32, IN IMS_BOOL), (override));
    MOCK_METHOD(void, RemoveTimer, (IN IPassiveTimerHolder::Type), (override));
    MOCK_METHOD(IMS_BOOL, IsActive, (IN IPassiveTimerHolder::Type), (const, override));
    MOCK_METHOD(void, AddListener, (IN IPassiveTimerHolder::Type, IN IPassiveTimerListener*),
            (override));
    MOCK_METHOD(void, RemoveListener, (IN IPassiveTimerHolder::Type, IN IPassiveTimerListener*),
            (override));
};

#endif
