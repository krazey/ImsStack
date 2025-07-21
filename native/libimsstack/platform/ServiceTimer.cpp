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
#include "ImsTimer.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTimer.h"

PRIVATE
TimerService::TimerService()
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
TimerService::~TimerService()
{
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
ITimer* TimerService::CreateTimer()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    ImsTimer* pTimer = piOsFactory->CreateTimer();

    IMS_ASSERT(pTimer != IMS_NULL);

    m_piLock->Lock();
    m_objTimers.Append(pTimer);
    m_piLock->Unlock();

    return pTimer;
}

PUBLIC
void TimerService::DestroyTimer(IN ITimer*& piTimer, IN IMS_BOOL bOnOwnerThread /* = IMS_TRUE*/)
{
    ImsTimer* pTimer = DYNAMIC_CAST(ImsTimer*, piTimer);

    if (pTimer == IMS_NULL)
    {
        return;
    }

    m_piLock->Lock();

    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
    {
        ITimer* piExTimer = m_objTimers.GetAt(i);

        if (piExTimer == IMS_NULL)
        {
            continue;
        }

        if (piExTimer->Equals(piTimer))
        {
            m_objTimers.RemoveAt(i);
            break;
        }
    }

    m_piLock->Unlock();

    if (bOnOwnerThread)
    {
        pTimer->Destroy();
    }
    else
    {
        delete pTimer;
    }

    piTimer = IMS_NULL;
}

PUBLIC
void TimerService::DispatchServiceMessage(IN const ImsMessage& objMsg)
{
    // FIX_TIMING_ISSUE: same timer id issue
    // If the internal timer id is MSG_PARAM_DESTROY,
    // then it indicates that the timer is killed and needs to be destroyed.
    if (objMsg.nLparam == ImsTimer::MSG_PARAM_DESTROY)
    {
        ImsTimer* pTimer = reinterpret_cast<ImsTimer*>(objMsg.nWparam);

        if (pTimer != IMS_NULL)
        {
            delete pTimer;
        }
        return;
    }

    // Check if the expired timer exists in the timer aggregate.
    m_piLock->Lock();

    for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
    {
        ITimer* piTimer = m_objTimers.GetAt(i);

        if (piTimer != IMS_NULL)
        {
            ImsTimer* pTimer = DYNAMIC_CAST(ImsTimer*, piTimer);
            // Compare Timer ID
            if (pTimer->GetTimerId() == objMsg.nWparam)
            {
                m_piLock->Unlock();

                pTimer->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
                return;
            }
        }
    }

    m_piLock->Unlock();
}

// Creates the singleton class and return it
PUBLIC GLOBAL TimerService* TimerService::GetTimerService()
{
    return DYNAMIC_CAST(TimerService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_TIMER));
}
