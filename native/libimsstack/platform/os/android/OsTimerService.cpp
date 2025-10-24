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
#include "OsTimerService.h"
#include "PlatformContext.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

static void osTimerService_AddListener(IN ISystemListener* piListener)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    piSystem->AddListener(SystemConstants::CATEGORY_TIMER, piListener, IMS_SLOT_0);
}

static void osTimerService_RemoveListener(IN ISystemListener* piListener)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    piSystem->RemoveListener(SystemConstants::CATEGORY_TIMER, piListener, IMS_SLOT_0);
}

static IMS_BOOL osTimerService_KillTimer(IN IMS_UINTP nTimerId)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    IMS_SINT32 nResult = piSystem->KillTimer(nTimerId);

    return (nResult == 0) ? IMS_FALSE : IMS_TRUE;
}

static IMS_BOOL osTimerService_SetTimer(IN IMS_UINTP nTimerId, IN IMS_UINT32 nDuration)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    IMS_SINT32 nResult = piSystem->SetTimer(nDuration, nTimerId);

    return (nResult == 0) ? IMS_FALSE : IMS_TRUE;
}

class OsTimerWrapper
{
public:
    inline explicit OsTimerWrapper(IN OsTimer* pTimer) :
            m_bTimerExpired(IMS_FALSE),
            m_pTimer(pTimer)
    {
    }
    inline ~OsTimerWrapper() {}

public:
    inline IMS_UINTP GetTimerId() const
    {
        return (m_pTimer != IMS_NULL) ? m_pTimer->GetTimerId() : 0;
    }

public:
    IMS_BOOL m_bTimerExpired;
    OsTimer* m_pTimer;
};

PUBLIC GLOBAL OsTimerService* OsTimerService::s_pTimerService = IMS_NULL;

PUBLIC
OsTimerService::OsTimerService()
{
    osTimerService_AddListener(this);
}

PUBLIC VIRTUAL OsTimerService::~OsTimerService()
{
    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
    {
        OsTimerWrapper* pTimerWrapper = m_objTimers.GetAt(i);

        if (pTimerWrapper != IMS_NULL)
        {
            osTimerService_KillTimer(pTimerWrapper->GetTimerId());
            delete pTimerWrapper;
        }
    }

    m_objTimers.Clear();

    osTimerService_RemoveListener(this);

    IMS_TRACE_D("Destructor :: TimerService", 0, 0, 0);
}

PUBLIC
void OsTimerService::KillTimer(IN const OsTimer* pTimer)
{
    if (pTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Timer is null", 0, 0, 0);
        return;
    }

    IMS_UINTP nTimerId = 0;
    IMS_BOOL bTimerExpired = IMS_FALSE;

    m_objLockTimer.Lock();

    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
    {
        OsTimerWrapper* pTimerWrapper = m_objTimers.GetAt(i);

        if (pTimerWrapper == IMS_NULL)
        {
            continue;
        }

        if (pTimerWrapper->GetTimerId() == pTimer->GetTimerId())
        {
            nTimerId = pTimerWrapper->GetTimerId();
            bTimerExpired = pTimerWrapper->m_bTimerExpired;
            m_objTimers.RemoveAt(i);
            delete pTimerWrapper;
            break;
        }
    }

    m_objLockTimer.Unlock();

    if ((nTimerId != 0) && !bTimerExpired)
    {
        osTimerService_KillTimer(nTimerId);
    }

    if (m_objTimers.IsEmpty())
    {
        IMS_TRACE_D("No active timers", 0, 0, 0);
    }
}

PUBLIC
IMS_BOOL OsTimerService::SetTimer(IN IMS_UINT32 nDuration, IN OsTimer* pTimer)
{
    if (pTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "Timer is null", 0, 0, 0);
        return IMS_FALSE;
    }

    OsTimerWrapper* pTimerWrapper = new OsTimerWrapper(pTimer);

    if (pTimerWrapper == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_objLockTimer.Lock();

    osTimerService_SetTimer(pTimerWrapper->GetTimerId(), nDuration);

    IMS_BOOL bResult = m_objTimers.Append(pTimerWrapper);

    m_objLockTimer.Unlock();

    if (!bResult)
    {
        osTimerService_KillTimer(pTimerWrapper->GetTimerId());

        delete pTimerWrapper;
    }

    return bResult;
}

PUBLIC GLOBAL OsTimerService* OsTimerService::GetTimerService()
{
    if (s_pTimerService == IMS_NULL)
    {
        s_pTimerService = new OsTimerService();
    }

    return s_pTimerService;
}

PUBLIC GLOBAL void OsTimerService::CleanUp()
{
    if (s_pTimerService != IMS_NULL)
    {
        delete s_pTimerService;
        s_pTimerService = IMS_NULL;
    }
}

PUBLIC GLOBAL void OsTimerService::StartUp()
{
    OsTimerService::GetTimerService();
}

PRIVATE VIRTUAL void OsTimerService::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nLParam;

    if (nEvent != IMS_SYSTEM_TIMER_EXPIRED)
    {
        IMS_TRACE_D("Event(%d) will be ignored", nEvent, 0, 0);
        return;
    }

    IMS_TRACE_D("TimerExpired (timerId=%" PFLS_u ")", nWParam, 0, 0);

    m_objLockTimer.Lock();

    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
    {
        OsTimerWrapper* pTimerWrapper = m_objTimers.GetAt(i);

        if ((pTimerWrapper != IMS_NULL) && (pTimerWrapper->GetTimerId() == nWParam))
        {
            pTimerWrapper->m_bTimerExpired = IMS_TRUE;
            NotifyTimerExpired(pTimerWrapper);
            break;
        }
    }

    m_objLockTimer.Unlock();
}

PRIVATE
void OsTimerService::NotifyTimerExpired(IN OsTimerWrapper* pTimerWrapper)
{
    if (pTimerWrapper == IMS_NULL)
    {
        IMS_TRACE_E(0, "OsTimerWrapper is null", 0, 0, 0);
        return;
    }

    if (pTimerWrapper->m_pTimer == IMS_NULL)
    {
        IMS_TRACE_E(0, "OsTimer is null", 0, 0, 0);
        return;
    }

    IThread* piOwnerThread = pTimerWrapper->m_pTimer->GetOwner();

    if (piOwnerThread == IMS_NULL)
    {
        IMS_TRACE_D("Timer (%" PFLS_u ") is already stopped", pTimerWrapper->GetTimerId(), 0, 0);
        return;
    }

    ImsMessage objMsg(IMS_MSG_TIMER, pTimerWrapper->m_pTimer->GetTimerId(),
            pTimerWrapper->m_pTimer->GetInternalTimerId());

    IMS_TRACE_D("Timer (%" PFLS_u ", %d) is expired; size=%d",
            pTimerWrapper->m_pTimer->GetTimerId(), pTimerWrapper->m_pTimer->GetInternalTimerId(),
            m_objTimers.GetSize());

    MessageService::PostMessageThread(piOwnerThread, objMsg);
}
