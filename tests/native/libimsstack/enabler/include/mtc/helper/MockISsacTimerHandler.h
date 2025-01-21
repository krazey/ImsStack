/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_I_SSAC_TIMER_HANDLER_H_
#define MOCK_I_SSAC_TIMER_HANDLER_H_

#include "ImsTypeDef.h"
#include "helper/ISsacTimerHandler.h"
#include <gmock/gmock.h>

class MockISsacTimerHandler : public ISsacTimerHandler
{
public:
    virtual ~MockISsacTimerHandler() = default;

    MOCK_METHOD(IMS_BOOL, IsSsacTimerRunning, (IN CallType), (const, override));
    MOCK_METHOD(void, StartBarringTimer, (IN CallType), (override));
};

#endif
