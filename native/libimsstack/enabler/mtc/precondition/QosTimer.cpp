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
#include "precondition/QosStringUtils.h"
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
        }
    }

    m_objTimers.Clear();
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
        m_objTimers.Remove(eTimerType);
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        IMS_TRACE_D("Timer_TimerExpired : Type[%s] removed. Timers size[%d]",
                QosStringUtils::ConvertQosTimerType(eTimerType), m_objTimers.GetSize(), 0);

        m_pQosTimerListener->OnTimerExpired(this, eTimerType);
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

    IMS_TRACE_D("StartQosTimer : Type[%s] Duration[%d]", QosStringUtils::ConvertQosTimerType(eType),
            nDuration, 0);

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
        return;
    }

    IMS_SLONG nIndex = m_objTimers.GetIndexOfKey(eType);
    m_objTimers.RemoveAt(nIndex);

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);

    IMS_TRACE_D("StopQosTimer : Type[%s]", QosStringUtils::ConvertQosTimerType(eType), 0, 0);
    IMS_TRACE_D("StopQosTimer : Timer size[%d]", m_objTimers.GetSize(), 0, 0);
}

PUBLIC
IMS_BOOL QosTimer::IsQosTimerActivated(IN QosTimerType eType)
{
    IMS_BOOL bResult = (GetTimer(eType)) ? IMS_TRUE : IMS_FALSE;

    IMS_TRACE_D("IsQosTimerActivated : Type[%s] Result[%s]",
            QosStringUtils::ConvertQosTimerType(eType), _TRACE_B_(bResult), 0);
    return bResult;
}

PRIVATE
ITimer* QosTimer::GetTimer(IN QosTimerType eType)
{
    IMS_SLONG nIndex = m_objTimers.GetIndexOfKey(eType);

    return (nIndex < 0) ? IMS_NULL : m_objTimers.GetValueAt(nIndex);
}
