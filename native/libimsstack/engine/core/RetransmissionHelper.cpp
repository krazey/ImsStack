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
#include "ImsLib.h"
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "IRetransmissionHelperListener.h"
#include "RetransmissionHelper.h"
#include "Service.h"
#include "SipConfigProxy.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
RetransmissionHelper::RetransmissionHelper(
        IN Service* pService, IN IMS_BOOL bIntervalCap /*= IMS_TRUE*/) :
        m_nDuration(TIMER_T1),
        m_nCumulativeDuration(TIMER_T1),
        m_nMaxDuration(TIMER_MAX),
        m_nIntervalCap(TIMER_MAX),
        m_piTimer(IMS_NULL),
        m_piListener(IMS_NULL)
{
    const ISipConfigV* piSipConfigV = pService->GetISipConfigV();

    m_nDuration = SipConfigProxy::GetTimerValueT1(
            pService->GetSlotId(), pService->GetSipProfile(), piSipConfigV);

    m_nCumulativeDuration = m_nDuration;
    m_nMaxDuration = m_nDuration * 64;

    if (bIntervalCap)
    {
        // Interval cap will be read from the timer T2...
        m_nIntervalCap = SipConfigProxy::GetTimerValueT2(
                pService->GetSlotId(), pService->GetSipProfile(), piSipConfigV);
    }
    else
    {
        m_nIntervalCap = m_nMaxDuration;
    }
}

PUBLIC VIRTUAL RetransmissionHelper::~RetransmissionHelper()
{
    IMS_TRACE_D("Destructor :: RetransmissionHelper", 0, 0, 0);

    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
    }
}

PUBLIC
void RetransmissionHelper::SetMaxDuration(IN IMS_SINT32 nValue)
{
    if (m_piTimer != IMS_NULL)
    {
        return;
    }

    m_nMaxDuration = nValue;
}

PUBLIC
IMS_RESULT RetransmissionHelper::Start()
{
    if (m_piTimer != IMS_NULL)
    {
        IMS_TRACE_D("Retransmission timer is already running ...", 0, 0, 0);
        return IMS_SUCCESS;
    }

    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a retransmission timer failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_piTimer->SetTimer(m_nDuration, this);

    IMS_TRACE_I("Retransmission timer (%p) is started .....", m_piTimer, 0, 0);

    return IMS_SUCCESS;
}

PUBLIC
void RetransmissionHelper::Stop()
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("Retransmission timer (%p) is stopped .....", m_piTimer, 0, 0);

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}

PROTECTED VIRTUAL void RetransmissionHelper::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piTimer != piTimer)
    {
        return;
    }

    if (m_nCumulativeDuration == m_nMaxDuration)
    {
        if (m_piListener != IMS_NULL)
        {
            if (m_piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_TIMER_EXPIRED) !=
                    IMS_SUCCESS)
            {
                return;
            }
        }

        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);
        m_piTimer = IMS_NULL;
        return;
    }

    if (m_piListener != IMS_NULL)
    {
        if (m_piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_RETRANSMIT) != IMS_SUCCESS)
        {
            return;
        }
    }

    IMS_SINT32 nTempDuration = m_nDuration << 1;
    IMS_SINT32 nNextDuration = IMS_MIN(nTempDuration, m_nIntervalCap);

    nTempDuration = nNextDuration + m_nCumulativeDuration;

    if (nTempDuration <= m_nMaxDuration)
    {
        m_nDuration = nNextDuration;
        m_nCumulativeDuration = nTempDuration;
    }
    else
    {
        m_nDuration = m_nMaxDuration - m_nCumulativeDuration;
        m_nCumulativeDuration = m_nMaxDuration;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);

    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a retransmission timer falied", 0, 0, 0);

        if (m_piListener != IMS_NULL)
        {
            m_piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_INTERNAL_ERROR);
        }

        return;
    }

    if (!m_piTimer->SetTimer(m_nDuration, this))
    {
        IMS_TRACE_E(0, "Starting a retransmission timer falied", 0, 0, 0);

        if (m_piListener != IMS_NULL)
        {
            m_piListener->RetransmissionHelper_NotifyStatus(NOTIFICATION_INTERNAL_ERROR);
        }

        return;
    }
}
