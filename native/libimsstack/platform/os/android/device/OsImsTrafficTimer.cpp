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
#include "ImsMessageDef.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "device/OsImsTrafficTimer.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsImsTrafficTimer::OsImsTrafficTimer(
        IN IMS_SINT32 nSlot, IN IMS_UINT32 nType, IN IMS_UINT32 nDuration) :
        m_nSlotId(nSlot),
        m_nType(nType),
        m_nDuration(nDuration),
        m_piTimer(IMS_NULL),
        m_piTrafficListener(IMS_NULL)
{
    IMS_TRACE_D("OsImsTrafficTimer :: [%d] type = %d, duration = %d", nSlot, nType, nDuration);
}

PUBLIC VIRTUAL OsImsTrafficTimer::~OsImsTrafficTimer()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    }
}

PUBLIC VIRTUAL void OsImsTrafficTimer::Start()
{
    if (m_piTimer != IMS_NULL)
    {
        Stop();
    }

    m_piTimer = TimerService::GetTimerService()->CreateTimer();
    IMS_UINTP nID = m_piTimer->SetTimer(m_nDuration, this);

    IMS_TRACE_D("Start :: id (%p) , duration (%d)", nID, m_nDuration, 0);
}

PUBLIC VIRTUAL void OsImsTrafficTimer::Stop()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);

        IMS_TRACE_D("Stop", 0, 0, 0);
    }
}

PUBLIC VIRTUAL void OsImsTrafficTimer::SetListener(IN IImsTrafficTimerListener* piListener)
{
    m_piTrafficListener = piListener;
}

PUBLIC VIRTUAL void OsImsTrafficTimer::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL || m_piTimer == piTimer)
    {
        return;
    }

    Stop();

    if (m_piTrafficListener != IMS_NULL)
    {
        m_piTrafficListener->ImsTrafficTimer_Expired(m_nSlotId, m_nType);
    }
}
