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
#include "ImsMessageDef.h"
#include "OsTimer.h"
#include "OsTimerService.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_ADAPT__;



PRIVATE GLOBAL
IMS_UINT32 OsTimer::s_nInternalTimerId = 1;

PUBLIC
OsTimer::OsTimer()
    : m_nState(STATE_INACTIVE)
    , m_nTimerId(0)
    , m_nInternalTimerId(0)
{
    m_piOwner = ThreadService::GetThreadService()->GetCurrentThread();

    m_nInternalTimerId = s_nInternalTimerId++;

    if (s_nInternalTimerId == 0xFFFFFFFE)
    {
        s_nInternalTimerId = 1;
    }
}

PUBLIC VIRTUAL
OsTimer::~OsTimer()
{
    if (m_nState == STATE_ACTIVE)
    {
        KillTimer();
    }
}

PUBLIC VIRTUAL
IMS_BOOL OsTimer::Equals(IN const ITimer* piTimer) const
{
    const OsTimer* pTimer = DYNAMIC_CAST(const OsTimer*, piTimer);

    if (pTimer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (this == pTimer);
}

/**
 * @brief Returns an identifier for the timer.
 *
 * If successful; otherwise, an error(0) is returned.
 */
PUBLIC VIRTUAL
IMS_UINTP OsTimer::SetTimer(IN IMS_UINT32 nDuration, IN ITimerListener* piListener)
{
    if (m_nState == STATE_ACTIVE)
    {
        IMS_TRACE_D("Timer (id=%" PFLS_u ",%d) is already active",
                m_nTimerId, m_nInternalTimerId, 0);
        return 0;
    }

    m_nState = STATE_ACTIVE;
    m_nTimerId = CreateTimerId();
    m_piListener = piListener;

    OsTimerService::GetTimerService()->SetTimer(nDuration, this);

    IMS_TRACE_I("Timer :: Set (id=%" PFLS_u ",%d; duration=%d)",
            m_nTimerId, m_nInternalTimerId, nDuration);

    return m_nTimerId;
}

PUBLIC VIRTUAL
void OsTimer::KillTimer()
{
    m_piListener = IMS_NULL;

    if (m_nState == STATE_ACTIVE)
    {
        OsTimerService::GetTimerService()->KillTimer(this);
        m_nState = STATE_INACTIVE;
        m_nTimerId = 0;
    }
}

PUBLIC VIRTUAL
void OsTimer::Destroy()
{
    if (m_piOwner != IMS_NULL)
    {
        m_piOwner->PostMessageI(IMS_MSG_TIMER,
                reinterpret_cast<IMS_UINTP>(this), MSG_PARAM_DESTROY);
    }
    else
    {
        ImsTimer::Destroy();
    }
}

PUBLIC VIRTUAL
void OsTimer::DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    (void)nWparam;
    (void)nLparam;

    if (m_nInternalTimerId != LONG_TO_INT(nLparam))
    {
        IMS_TRACE_D("Timer :: Timer id(%" PFLS_u ") is matched, but internal timer id(%d)" \
                " is not matched; ignored", m_nTimerId, m_nInternalTimerId, 0);
        return;
    }

    if (m_piListener != IMS_NULL)
    {
        m_piListener->Timer_TimerExpired(this);
    }
}

PRIVATE
IMS_UINTP OsTimer::CreateTimerId()
{
    return reinterpret_cast<IMS_UINTP>(this);
}
