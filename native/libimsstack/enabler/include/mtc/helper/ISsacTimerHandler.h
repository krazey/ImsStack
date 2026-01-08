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

#ifndef INTERFACE_SSAC_TIMER_HANDLER_H_
#define INTERFACE_SSAC_TIMER_HANDLER_H_

#include "ImsTypeDef.h"

enum class CallType;

/**
 * @brief Interface for SSAC timer handler.
 */
class ISsacTimerHandler
{
public:
    virtual ~ISsacTimerHandler() = default;

    /**
     * @brief Check if SSAC timer is running.
     *
     * @param eCallType Type of the call.
     * @return true if the timer is running, false otherwise.
     */
    virtual IMS_BOOL IsSsacTimerRunning(IN CallType eCallType) const = 0;

    /**
     * @brief Start barring timer if possible.
     *
     * @param eCallType Type of the call.
     */
    virtual void StartBarringTimer(IN CallType eCallType) = 0;
};

#endif
