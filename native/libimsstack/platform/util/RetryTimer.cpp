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
#include "IRetryTimerListener.h"
#include "RetryTimer.h"
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
RetryTimer::RetryTimer(IN IMS_BOOL bRepeatable /*= IMS_FALSE*/) :
        m_nState(STATE_INACTIVE),
        m_piTimer(IMS_NULL),
        m_nTracker(0),
        m_bRepeatable(bRepeatable),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RetryTimer::~RetryTimer()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    }

    IMS_TRACE_D("Destructor :: RetryTimer", 0, 0, 0);
}

/**
 * @brief Adds a time interval in milli-seconds.
 */
PUBLIC
IMS_BOOL RetryTimer::AddValue(IN IMS_UINT32 nInterval)
{
    if (m_nState == STATE_ACTIVE)
    {
        return IMS_FALSE;
    }

    return m_objIntervals.Append(nInterval);
}

/**
 * @brief Adds the set of time intervals in milli-seconds.
 */
PUBLIC
IMS_BOOL RetryTimer::AddValues(IN ImsList<IMS_UINT32>& objIntervals)
{
    if (m_nState == STATE_ACTIVE)
    {
        return IMS_FALSE;
    }

    return m_objIntervals.AppendList(objIntervals);
}

/**
 * @brief Returns the next time interval in milli-seconds.
 */
PUBLIC
IMS_UINT32 RetryTimer::GetNextInterval() const
{
    if (m_objIntervals.IsEmpty())
    {
        return 0;
    }

    if (m_nTracker >= m_objIntervals.GetSize())
    {
        return 0;
    }

    return m_objIntervals.GetAt(m_nTracker);
}

/**
 * @brief Resumes the retry timer.
 *        It can be invoked when receiving the result of the command.
 */
PUBLIC
IMS_BOOL RetryTimer::Resume()
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Retry Timer is already in ACTIVE state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_nState != STATE_PENDING)
    {
        IMS_TRACE_E(0, "Retry Timer can't be resumed not in PENDING state (%d)", m_nState, 0, 0);
        return IMS_FALSE;
    }

    if (m_objIntervals.IsEmpty())
    {
        IMS_TRACE_E(0, "Retry Timer has no intervals", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!StartTimer())
    {
        IMS_TRACE_E(0, "Starting the retry timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/**
 * @brief Starts the retry timer rule.
 */
PUBLIC
IMS_BOOL RetryTimer::Start()
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Retry Timer is already in ACTIVE state", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_objIntervals.IsEmpty())
    {
        IMS_TRACE_E(0, "Retry Timer has no intervals", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!StartTimer())
    {
        IMS_TRACE_E(0, "Starting the retry timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_nState = STATE_ACTIVE;

    return IMS_TRUE;
}

/**
 * @brief Terminates the retry timer.
 */
PUBLIC
void RetryTimer::Terminate()
{
    if (m_nState == STATE_INACTIVE)
    {
        return;
    }

    if (m_piTimer != IMS_NULL)
    {
        IMS_TRACE_D("Retry Timer (%p) is terminated (%02d-th)", m_piTimer, m_nTracker, 0);

        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    }

    m_nTracker = 0;
    m_nState = STATE_INACTIVE;
}

/**
 * @brief It is invoked when the timer expired.
 */
PRIVATE VIRTUAL void RetryTimer::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    if (!m_piTimer->Equals(piTimer))
    {
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);

    if (m_nTracker >= m_objIntervals.GetSize())
    {
        if (m_bRepeatable)
        {
            m_nTracker = m_objIntervals.GetSize() - 1;
        }
        else
        {
            m_nTracker = 0;
            m_nState = STATE_INACTIVE;

            if (m_piListener != IMS_NULL)
            {
                m_piListener->RetryTimer_OnFinalExpired(this);
            }

            return;
        }
    }

    // According to the result, re-start the timer
    if (m_piListener != IMS_NULL)
    {
        IMS_SINT32 nResult = m_piListener->RetryTimer_OnInterimExpired(this);

        if (nResult == RESULT_CONTINUE)
        {
            StartTimer();
        }
        else if (nResult == RESULT_PENDING)
        {
            // Do not start a timer & preserve the current state
            IMS_TRACE_I("Retry Timer is pending (%02d-th)", m_nTracker, 0, 0);

            m_nState = STATE_PENDING;
        }
        else
        {
            IMS_TRACE_I("Retry Timer is stopped (%02d-th)", m_nTracker, 0, 0);

            // Do not start a timer & clear the state
            m_nTracker = 0;
            m_nState = STATE_INACTIVE;
        }
    }
    else
    {
        StartTimer();
    }
}

/**
 * @brief It is invoked when the timer expired.
 */
PRIVATE VIRTUAL IMS_BOOL RetryTimer::StartTimer()
{
    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const IMS_UINT32& nInterval = m_objIntervals.GetAt(m_nTracker);

    m_piTimer->SetTimer(nInterval, this);

    ++m_nTracker;

    IMS_TRACE_I("Retry Timer (%p, %d) is started (%02d-th)", m_piTimer, nInterval, m_nTracker);

    return IMS_TRUE;
}
