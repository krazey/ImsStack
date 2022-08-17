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

#include "ServiceTrace.h"
#include "utility/MtsTimer.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsTimer::MtsTimer() :
        m_objTimers(IMSMap<MtsTimerType, ITimer*>()),
        m_piTimerListener(IMS_NULL)
{
    IMS_TRACE_D("+MtsTimer", 0, 0, 0);
}

PUBLIC VIRTUAL MtsTimer::~MtsTimer()
{
    IMS_TRACE_D("~MtsTimer", 0, 0, 0);

    for (IMS_UINT32 index = 0; index < m_objTimers.GetSize(); index++)
    {
        ITimer* piTimer = m_objTimers.GetValueAt(index);

        if (piTimer)
        {
            piTimer->KillTimer();
            TimerService::GetTimerService()->DestroyTimer(piTimer);
            piTimer = IMS_NULL;
        }
    }

    m_objTimers.Clear();

    if (m_piTimerListener)
    {
        m_piTimerListener = IMS_NULL;
    }
}

PUBLIC VIRTUAL void MtsTimer::Timer_TimerExpired(IN ITimer* piExpiredTimer)
{
    if (!m_piTimerListener)
    {
        return;
    }

    for (IMS_UINT32 index = 0; index < m_objTimers.GetSize(); index++)
    {
        ITimer* piTimer = m_objTimers.GetValueAt(index);
        if (piTimer != piExpiredTimer)
        {
            continue;
        }

        MtsTimerType eTimerType = m_objTimers.GetKeyAt(index);

        if (eTimerType == MtsTimerType::TIMER_SMS_CALLBACK_MODE)
        {
            m_piTimerListener->Timer_TimerExpired(piTimer);
        }
        else if (eTimerType == MtsTimerType::TIMER_RETRY_AFTER)
        {
            m_piTimerListener->Timer_TimerExpired(piTimer);
        }

        m_objTimers.Remove(eTimerType);
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);

        IMS_TRACE_D("Timer_TimerExpired : Type[%d] removed. Timers size[%d]", eTimerType,
                m_objTimers.GetSize(), 0);

        return;
    }

    IMS_TRACE_D("Timer_TimerExpired : can't find expired timer", 0, 0, 0);
}

PUBLIC void MtsTimer::StartTimer(IN MtsTimerType eType, IN IMS_SINT32 nDuration)
{
    // TODO: add implementation
    (void)eType;
    (void)nDuration;
}

PUBLIC void MtsTimer::StopTimer(IN MtsTimerType eType)
{
    // TODO: add implementation
    (void)eType;
}
