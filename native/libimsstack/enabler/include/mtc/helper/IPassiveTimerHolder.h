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

#ifndef INTERFACE_PASSIVE_TIMER_HOLDER_H_
#define INTERFACE_PASSIVE_TIMER_HOLDER_H_

#include "ImsTypeDef.h"

/**
 * This class holds timers with their duration period. When the timer expires, it is removed from
 * the internal list.
 * All timers in the list are released once a normal registration is disconnedted
 */
class IPassiveTimerHolder
{
public:
    enum class Type
    {
        CALL_BLOCKED_BY_RETRY_AFTER,
        SSAC_VOICE_BARRING,
        SSAC_VIDEO_BARRING,
    };

    virtual ~IPassiveTimerHolder() = default;

    /**
     * @brief Adds a timer with specific type.
     *
     * @param eType the type of the timer.
     * @param nTimeInMillis the duration time of the timer to be active.
     * @param bAllowReset Reset the active timer if it is true.
     */
    virtual void AddTimer(IN IPassiveTimerHolder::Type eType, IN IMS_UINT32 nTimeInMillis,
            IN IMS_BOOL bAllowReset = IMS_FALSE) = 0;

    /**
     * @brief Gets existence of a timer with specific type.
     *
     * @param eType the type of the timer.
     * @return True if the timer is active.
     */
    virtual IMS_BOOL IsActive(IN IPassiveTimerHolder::Type eType) const = 0;
};

#endif
