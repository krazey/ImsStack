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
#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "SipPrivate.h"
#include "SipStack.h"
#include "SipStackState.h"
#include "SipTransactionTimer.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipTransactionTimer::SipTransactionTimer(
        IN SipTimeoutData* pData, IN SipTimerCallback pfnTimerCallback) :
        m_piTimer(IMS_NULL),
        m_pData(pData),
        m_pfnTimerCallback(pfnTimerCallback)
{
}

PUBLIC
SipTransactionTimer::~SipTransactionTimer()
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);

        m_piTimer = IMS_NULL;
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipTransactionTimer", 0, 0, 0);
#endif
}

PUBLIC
IMS_BOOL SipTransactionTimer::Start(IN IMS_SINT32 nDuration)
{
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);

        m_piTimer = IMS_NULL;
    }

    m_piTimer = TimerService::GetTimerService()->CreateTimer();

    if (m_piTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Opening a transaction timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piTimer->SetTimer(nDuration, this);

    IMS_TRACE_I(">>> START TIMER: TIMER(%p|%s|%d)", m_piTimer,
            SipStack::GetTimerTypeAsString(m_pData), nDuration);

    return IMS_TRUE;
}

PUBLIC
void SipTransactionTimer::Stop(OUT SipTimeoutData*& pData)
{
    if (m_piTimer != IMS_NULL)
    {
        IMS_TRACE_I(">>> STOP TIMER: TIMER(%p|%s)", m_piTimer,
                SipStack::GetTimerTypeAsString(m_pData), 0);

        m_piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piTimer);

        m_piTimer = IMS_NULL;
    }

    pData = m_pData;

    m_pData = IMS_NULL;
    m_pfnTimerCallback = IMS_NULL;
}

PUBLIC GLOBAL void SipTransactionTimer::FreeTimer(IN void* pvTimerHandle)
{
    SipTransactionTimer* pTimer = static_cast<SipTransactionTimer*>(pvTimerHandle);

    if (pTimer != IMS_NULL)
    {
        delete pTimer;
    }
}

PUBLIC GLOBAL void SipTransactionTimer::TimerExpired(IN IMS_SINT32 eTimerType)
{
    (void)eTimerType;
    IMS_TRACE_I("___ EXPIRED TIMER (%s) ___", SipStack::GetTimerTypeAsString(eTimerType), 0, 0);
}

PRIVATE VIRTUAL void SipTransactionTimer::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    if (m_piTimer != piTimer)
    {
        return;
    }

    IMS_TRACE_I(">>> TIMER TIMEOUT: TIMER(%p|%s)", m_piTimer,
            SipStack::GetTimerTypeAsString(m_pData), 0);

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;

    if (m_pfnTimerCallback != IMS_NULL)
    {
        SipStack::InvokeTimerCallback(
                m_pfnTimerCallback, m_pData, reinterpret_cast<IMS_PVOID>(this));
    }
}
