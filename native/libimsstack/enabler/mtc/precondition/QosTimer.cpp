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
#include "precondition/QosStringDef.h"
#include "precondition/QosTimer.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
QosTimer::QosTimer(IN IQosTimerListener* pListener) :
        m_objTimers(ImsMap<QosTimerType, ITimer*>()),
        m_pQosTimerListener(pListener)
{
    IMS_TRACE_D("+QosTimer", 0, 0, 0);
}

PUBLIC VIRTUAL QosTimer::~QosTimer()
{
    IMS_TRACE_D("~QosTimer", 0, 0, 0);

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

    if (m_pQosTimerListener)
    {
        m_pQosTimerListener = IMS_NULL;
    }
}

PUBLIC VIRTUAL void QosTimer::Timer_TimerExpired(IN ITimer* piExpiredTimer)
{
    if (!m_pQosTimerListener)
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

        QosTimerType eTimerType = m_objTimers.GetKeyAt(index);

        if (eTimerType == QosTimerType::WAIT_AVAILABLE)
        {
            m_pQosTimerListener->OnWaitTimerExpired(this);
        }
        else if (eTimerType == QosTimerType::GUARD_INACTIVE)
        {
            m_pQosTimerListener->OnGuardInactiveTimerExpired(this);
        }
        else if (eTimerType == QosTimerType::FORCE_AVAILABLE)
        {
            m_pQosTimerListener->OnForceAvailableTimerExpired(this);
        }
        else if (eTimerType == QosTimerType::WAIT_AVAILABLE_AFTER_HANDOVER)
        {
            m_pQosTimerListener->OnWaitTimerAfterHandOverExpired(this);
        }

        m_objTimers.Remove(eTimerType);
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);

        IMS_TRACE_D("Timer_TimerExpired : Type[%s] removed. Timers size[%d]",
                PS_QosTimerType(eTimerType), m_objTimers.GetSize(), 0);

        return;
    }

    IMS_TRACE_D("Timer_TimerExpired : can't find expired timer", 0, 0, 0);
}

PUBLIC
void QosTimer::StartQosTimer(IN QosTimerType eType, IN IMS_SINT32 nDuration)
{
    if (GetTimer(eType))
    {
        IMS_TRACE_D("StartQosTimer : It is already started.", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("StartQosTimer : Type[%s] Duration[%d]", PS_QosTimerType(eType), nDuration, 0);

    if (nDuration <= 0)
    {
        return;
    }

    ITimer* piTimer = TimerService::GetTimerService()->CreateTimer();

    m_objTimers.Add(eType, piTimer);
    piTimer->SetTimer(nDuration, this);

    IMS_TRACE_D("StartQosTimer : Timer size[%d]", m_objTimers.GetSize(), 0, 0);
}

PUBLIC
void QosTimer::StopQosTimer(IN QosTimerType eType)
{
    ITimer* piTimer = GetTimer(eType);

    if (!piTimer)
    {
        IMS_TRACE_D("StopQosTimer : Type[%s] no active timer", PS_QosTimerType(eType), 0, 0);
        return;
    }

    IMS_SLONG nIndex = m_objTimers.GetIndexOfKey(eType);
    m_objTimers.RemoveAt(nIndex);

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);

    IMS_TRACE_D("StopQosTimer : Type[%s]", PS_QosTimerType(eType), 0, 0);
    IMS_TRACE_D("StopQosTimer : Timer size[%d]", m_objTimers.GetSize(), 0, 0);
}

PUBLIC
IMS_BOOL QosTimer::IsQosTimerActivated(IN QosTimerType eType)
{
    IMS_BOOL bResult = (GetTimer(eType)) ? IMS_TRUE : IMS_FALSE;

    IMS_TRACE_D("IsQosTimerActivated : Type[%s] Result[%s]", PS_QosTimerType(eType),
            _TRACE_B_(bResult), 0);
    return bResult;
}

PRIVATE
ITimer* QosTimer::GetTimer(IN QosTimerType eType)
{
    IMS_SLONG nIndex = m_objTimers.GetIndexOfKey(eType);

    return (nIndex < 0) ? IMS_NULL : m_objTimers.GetValueAt(nIndex);
}
