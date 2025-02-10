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
#include "helper/IPassiveTimerListener.h"
#include "helper/PassiveTimerHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
PassiveTimerHolder::PassiveTimerHolder() :
        m_piService(IMS_NULL),
        m_objTimerInfoByType(ImsMap<IPassiveTimerHolder::Type, TimerInfo*>())
{
    IMS_TRACE_I("+PassiveTimerHolder", 0, 0, 0);
}

PUBLIC VIRTUAL PassiveTimerHolder::~PassiveTimerHolder()
{
    IMS_TRACE_I("~PassiveTimerHolder", 0, 0, 0);

    if (m_piService)
    {
        m_piService->RemoveAosStateListener(this);
    }

    ReleaseAllTimerInfo();
}

PUBLIC VIRTUAL void PassiveTimerHolder::AddTimer(IN IPassiveTimerHolder::Type eType,
        IN IMS_SINT32 nTimeInMillis, IN IMS_BOOL bAllowReset /* = IMS_FALSE */)
{
    IMS_TRACE_D("AddTimer Type[%d] Duration[%d]", eType, nTimeInMillis, 0);

    if (nTimeInMillis < 0)
    {
        return;
    }

    if (IsActive(eType))
    {
        if (bAllowReset == IMS_FALSE)
        {
            IMS_TRACE_D("AddTimer Type[%d] is ignored. Same type is active.", eType, 0, 0);
            return;
        }

        ReleaseTimerInfo(eType);
    }

    ITimer* piTimer = TimerService::GetTimerService()->CreateTimer();
    m_objTimerInfoByType.Add(eType, new TimerInfo(piTimer));
    piTimer->SetTimer(nTimeInMillis, this);
}

PUBLIC VIRTUAL void PassiveTimerHolder::RemoveTimer(IN IPassiveTimerHolder::Type eType)
{
    IMS_TRACE_D("RemoveTimer Type[%d]", eType, 0, 0);

    if (IsActive(eType))
    {
        ReleaseTimerInfo(eType);
    }
}

PUBLIC VIRTUAL IMS_BOOL PassiveTimerHolder::IsActive(IN IPassiveTimerHolder::Type eType) const
{
    return m_objTimerInfoByType.GetIndexOfKey(eType) >= 0;
}

PUBLIC VIRTUAL void PassiveTimerHolder::OnAosStateChanged(
        IN IMtcService& /*objMtcService*/, IN MtcAosState eState, IN IMS_UINT32 /*eAosReason*/)
{
    // All timers are released by normal AoS disconnection. If aother policy comes up, This logic
    // needs to move into each class to handle the policies differently. Or, we can add differencial
    // logic into this api.
    if (eState == MtcAosState::DISCONNECTED)
    {
        ReleaseAllTimerInfo();
    }
}

PUBLIC VIRTUAL void PassiveTimerHolder::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_SLONG nIndex = GetIndexOfTimerInfo(piTimer);
    if (nIndex == -1)
    {
        return;
    }

    IPassiveTimerHolder::Type eType = m_objTimerInfoByType.GetKeyAt(nIndex);
    IMS_TRACE_I("Timer_TimerExpired : Type[%d]", eType, 0, 0);

    TimerInfo* pTimerInfo = m_objTimerInfoByType.GetValue(eType);
    pTimerInfo->SetTerminating();

    for (IMS_UINT32 i = 0; i < pTimerInfo->objListeners.GetSize(); i++)
    {
        pTimerInfo->objListeners.GetAt(i)->OnPassiveTimerExpired(eType);
    }

    ReleaseTimerInfo(eType);
}

PUBLIC
void PassiveTimerHolder::SetNormalService(IN IMtcService* pService)
{
    m_piService = pService;
    if (m_piService)
    {
        m_piService->AddAosStateListener(this);
    }
}

PUBLIC
void PassiveTimerHolder::AddListener(
        IN IPassiveTimerHolder::Type eType, IPassiveTimerListener* pPassiveTimerListener)
{
    if (m_objTimerInfoByType.GetIndexOfKey(eType) < 0)
    {
        return;
    }

    m_objTimerInfoByType.GetValue(eType)->AddListener(pPassiveTimerListener);
}

PUBLIC
void PassiveTimerHolder::RemoveListener(
        IN IPassiveTimerHolder::Type eType, IN IPassiveTimerListener* pPassiveTimerListener)
{
    if (m_objTimerInfoByType.GetIndexOfKey(eType) < 0)
    {
        return;
    }

    m_objTimerInfoByType.GetValue(eType)->RemoveListener(pPassiveTimerListener);
}

PRIVATE
IMS_SLONG PassiveTimerHolder::GetIndexOfTimerInfo(IN const ITimer* piTimer) const
{
    for (IMS_UINT32 i = 0; i < m_objTimerInfoByType.GetSize(); ++i)
    {
        if (piTimer == m_objTimerInfoByType.GetValueAt(i)->piTimer)
        {
            return (IMS_SLONG)i;
        }
    }

    return -1;
}

PRIVATE
void PassiveTimerHolder::ReleaseTimerInfo(IN IPassiveTimerHolder::Type eType)
{
    TimerInfo* pTimerInfo = m_objTimerInfoByType.GetValue(eType);
    pTimerInfo->piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(pTimerInfo->piTimer);
    delete pTimerInfo;
    m_objTimerInfoByType.Remove(eType);
}

PRIVATE
void PassiveTimerHolder::ReleaseAllTimerInfo()
{
    while (!m_objTimerInfoByType.IsEmpty())
    {
        TimerInfo* pTimerInfo = m_objTimerInfoByType.GetValueAt(0);
        pTimerInfo->piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(pTimerInfo->piTimer);
        delete pTimerInfo;
        m_objTimerInfoByType.RemoveAt(0);
    }
}
