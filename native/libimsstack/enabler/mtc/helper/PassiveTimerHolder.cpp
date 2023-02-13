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

#include "IMtcService.h"
#include "ImsMap.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/PassiveTimerHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
PassiveTimerHolder::PassiveTimerHolder() :
        m_pService(IMS_NULL),
        m_objTimers(ImsMap<IPassiveTimerHolder::Type, ITimer*>())
{
    IMS_TRACE_I("+PassiveTimerHolder", 0, 0, 0);
}

PUBLIC VIRTUAL PassiveTimerHolder::~PassiveTimerHolder()
{
    IMS_TRACE_I("~PassiveTimerHolder", 0, 0, 0);

    if (m_pService)
    {
        m_pService->RemoveAosStateListener(this);
    }

    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); i++)
    {
        ReleaseTimer(m_objTimers.GetValueAt(i));
    }
    m_objTimers.Clear();
}

PUBLIC VIRTUAL void PassiveTimerHolder::AddTimer(
        IN IPassiveTimerHolder::Type eType, IN IMS_UINT32 nTimeInMillis)
{
    if (IsActive(eType))
    {
        IMS_TRACE_D("AddTimer Type[%d] is ignored. Same type is active.", eType, 0, 0);
        return;
    }

    IMS_TRACE_D("AddTimer Type[%d] Duration[%d]", eType, nTimeInMillis, 0);

    ITimer* piTimer = TimerService::GetTimerService()->CreateTimer();
    m_objTimers.Add(eType, piTimer);
    piTimer->SetTimer(nTimeInMillis, this);
}

PUBLIC VIRTUAL IMS_BOOL PassiveTimerHolder::IsActive(IN IPassiveTimerHolder::Type eType) const
{
    return m_objTimers.GetIndexOfKey(eType) >= 0;
}

PUBLIC VIRTUAL void PassiveTimerHolder::OnAosStateChanged(
        IN IMtcService& /*objMtcService*/, IN MtcAosState eState, IN IMS_UINT32 /*eAosReason*/)
{
    // All timers are released by normal AoS disconnection. If aother policy comes up, This logic
    // needs to move into each class to handle the policies differently. Or, we can add differencial
    // logic into this api.
    if (eState == MtcAosState::DISCONNECTED)
    {
        IMS_TRACE_I("OnAosStateChanged : Release Timers size[%d]", m_objTimers.GetSize(), 0, 0);
        for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); i++)
        {
            ReleaseTimer(m_objTimers.GetValueAt(i));
        }
        m_objTimers.Clear();
    }
}

PUBLIC VIRTUAL void PassiveTimerHolder::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_SLONG nIndex = GetIndexOfTimer(piTimer);
    if (nIndex == -1)
    {
        return;
    }

    ReleaseTimer(piTimer);
    IMS_TRACE_I("Timer_TimerExpired : Type[%d]", m_objTimers.GetKeyAt(nIndex), 0, 0);
    m_objTimers.RemoveAt(nIndex);
}

PUBLIC
void PassiveTimerHolder::SetNormalService(IN IMtcService* pService)
{
    m_pService = pService;
    if (m_pService)
    {
        m_pService->AddAosStateListener(this);
    }
}

PRIVATE
IMS_SLONG PassiveTimerHolder::GetIndexOfTimer(IN const ITimer* piTimer)
{
    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
    {
        if (piTimer == m_objTimers.GetValueAt(i))
        {
            return (IMS_SLONG)i;
        }
    }

    return -1;
}

PRIVATE
void PassiveTimerHolder::ReleaseTimer(IN ITimer* piTimer)
{
    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
}
