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
#ifndef RETRY_TIMER_H_
#define RETRY_TIMER_H_

#include "ITimer.h"
#include "ImsList.h"

class IRetryTimerListener;

class RetryTimer : public ITimerListener
{
public:
    explicit RetryTimer(IN IMS_BOOL bRepeatable = IMS_FALSE);
    virtual ~RetryTimer();

public:
    // Add a time interval; in milli-seconds
    IMS_BOOL AddValue(IN IMS_UINT32 nInterval);
    // Adds the set of time intervals; in milli-seconds
    IMS_BOOL AddValues(IN ImsList<IMS_UINT32>& objIntervals);
    // Returns the next time interval; in milli-seconds
    IMS_UINT32 GetNextInterval() const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    // Resume the timer in PENDING state
    IMS_BOOL Resume();
    // Sets a listener when the timer expired
    inline void SetListener(IN IRetryTimerListener* piListener) { m_piListener = piListener; }
    // Apply the retry rule according to the timer
    IMS_BOOL Start();
    // Terminates the retry operation
    void Terminate();

private:
    // ITimerListener interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    IMS_BOOL StartTimer();

public:
    /// Return value for the method of IRetryTimerListener
    enum
    {
        /// Apply the timer rule continuously
        RESULT_CONTINUE = 0,
        /// Do not apply the timer rule & store the current state
        /// The timer will be re-started when receiving the result of the command
        RESULT_PENDING,
        /// Do not apply the timer rule & go to the initial state
        RESULT_STOP
    };

    enum
    {
        /// It is an initial state & the timer is not started
        STATE_INACTIVE = 0,
        /// The timer is pending until the command is completed
        /// After receiving the result of command, the timer will be re-started
        STATE_PENDING,
        /// The timer is started
        STATE_ACTIVE
    };

private:
    // State of RetryTimer
    IMS_SINT32 m_nState;

    // Pointer to the timer interface
    ITimer* m_piTimer;
    // To track the count of retry operation
    IMS_UINT32 m_nTracker;
    // List of time intervals
    ImsList<IMS_UINT32> m_objIntervals;

    // Flag to indicate if the repeatable(infinite) timer needs to be started
    // The last interval value will be used for the repeatable(infinite) timer
    IMS_BOOL m_bRepeatable;

    IRetryTimerListener* m_piListener;
};

#endif
