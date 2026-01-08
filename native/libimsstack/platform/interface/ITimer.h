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
#ifndef INTERFACE_TIMER_H_
#define INTERFACE_TIMER_H_

#include "ImsTypeDef.h"

class ITimerListener;

class ITimer
{
protected:
    virtual ~ITimer() = default;

public:
    /**
     * @brief Checks if the specified timer is equal to the current object.
     *
     * @param piTimer The pointer to ITimer to be checked
     * @return IMS_TRUE if the specified timer is equal, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Equals(IN const ITimer* piTimer) const = 0;

    /**
     * @brief Starts a timer.
     *
     * @param nDuration The timer duration (milli-seconds)
     * @param piListener The listener interface pointer, supplier should implement its listener
     *                   to receive notification of the timer expiration.
     * @return A timer id to identify this object.
     */
    virtual IMS_UINTP SetTimer(IN IMS_SINT64 nDuration, IN ITimerListener* piListener) = 0;

    /**
     * @brief Stops the running timer if it is not expired.
     *
     * NOTE: The application MUST destroy the pointer of ITimer
     *       with TimerService::GetTimerService()->Destroy() after calling this method.
     */
    virtual void KillTimer() = 0;
};

class ITimerListener
{
protected:
    virtual ~ITimerListener() = default;

public:
    /**
     * @brief Notifies the application that the specified timer is expired.
     *
     * @param piTimer The timer that is expired
     */
    virtual void Timer_TimerExpired(IN ITimer* piTimer) = 0;
};

#endif
