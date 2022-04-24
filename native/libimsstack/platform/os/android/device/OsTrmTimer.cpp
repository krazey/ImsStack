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
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "device/OsTrmTimer.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsTrmTimer::OsTrmTimer(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    : m_nSlotId(nSlotId)
    , m_nType(nType)
    , m_nDuration(nDuration)
    , m_piTrmTimer(IMS_NULL)
    , m_piListener(IMS_NULL)
{
    IMS_TRACE_D("OsTrmTimer :: [%d] duration = %d", nSlotId, nDuration, 0);
}

PUBLIC VIRTUAL
OsTrmTimer::~OsTrmTimer()
{
    if (m_piTrmTimer == IMS_NULL)
    {
        return;
    }

    m_piTrmTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTrmTimer);
    m_piTrmTimer = IMS_NULL;
}

PUBLIC VIRTUAL
void OsTrmTimer::SetListener(IN ITrmTimerListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC VIRTUAL
void OsTrmTimer::Start()
{
    if (m_piTrmTimer != IMS_NULL)
    {
        Stop();
    }

    m_piTrmTimer = TimerService::GetTimerService()->CreateTimer();
    IMS_UINTP nTid = m_piTrmTimer->SetTimer(m_nDuration, this);

    IMS_TRACE_D("Start :: id (%p) , duration (%d)", nTid, m_nDuration, 0);
}

PUBLIC VIRTUAL
void OsTrmTimer::Stop()
{
    if (m_piTrmTimer == IMS_NULL)
    {
        return;
    }

    m_piTrmTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTrmTimer);
    m_piTrmTimer = IMS_NULL;

    IMS_TRACE_I("Stop", 0, 0, 0);
}

PUBLIC VIRTUAL
void OsTrmTimer::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL || m_piTrmTimer != piTimer)
    {
        return;
    }

    Stop();

    if (m_piListener != IMS_NULL)
    {
        m_piListener->TrmTimer_TimerExpired(m_nSlotId, m_nType);
    }
}
