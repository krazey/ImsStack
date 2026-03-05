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

#ifndef INTERFACE_MTC_TIMER_LISTENER_H_
#define INTERFACE_MTC_TIMER_LISTENER_H_

/**
 * @brief Interface for listening to timer expiration events.
 *
 * Classes that require timer functionality within the MTC module should implement
 * this interface to receive notifications from {@link MtcTimerWrapper} when a specific timer
 * expires.
 */
class IMtcTimerListener
{
public:
    virtual ~IMtcTimerListener() = default;

    /**
     * @brief Notifies that a specific timer has expired from {@link MtcTimerWrapper}.
     *
     * This callback is invoked by the timer mechanism when the duration of a started timer has
     * elapsed.
     *
     * @param nType The unique identifier of the expired timer. This value corresponds to the
     *              timer type passed when starting the timer (e.g., MtcCallState::TimerType).
     */
    virtual void OnTimerExpired(IN IMS_SINT32 nType) = 0;
};
#endif
