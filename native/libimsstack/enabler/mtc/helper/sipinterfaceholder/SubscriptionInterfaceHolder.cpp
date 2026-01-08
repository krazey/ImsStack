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

#include "ICoreService.h"
#include "ISession.h"
#include "ISubscription.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SubscriptionInterfaceHolder::SubscriptionInterfaceHolder(IN IInterfaceHolderListener& objListener) :
        m_objListener(objListener)
{
    IMS_TRACE_D("+SubscriptionInterfaceHolder", 0, 0, 0);
}

PUBLIC
SubscriptionInterfaceHolder::~SubscriptionInterfaceHolder()
{
    IMS_TRACE_D("~SubscriptionInterfaceHolder", 0, 0, 0);

    ClearISubscriptions();

    for (IMS_SINT32 i =
                    static_cast<IMS_SINT32>(m_objSubscriptionTerminatedGuardTimers.GetSize()) - 1;
            i >= 0; i--)
    {
        StopTimer(m_objSubscriptionTerminatedGuardTimers.GetKeyAt(i));
    }
    m_objSubscriptionTerminatedGuardTimers.Clear();
}

PUBLIC VIRTUAL void SubscriptionInterfaceHolder::SubscriptionTerminated(
        IN ISubscription* piSubscription)
{
    IMS_TRACE_D("SubscriptionTerminated", 0, 0, 0);

    ReleaseISubscription(piSubscription, IMS_TRUE);
}

PUBLIC VIRTUAL void SubscriptionInterfaceHolder::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

    IMS_SLONG nIndex = m_objSubscriptionTerminatedGuardTimers.GetIndexOfKey(piTimer);
    if (nIndex < 0)
    {
        return;
    }

    ISubscription* piSubscription = m_objSubscriptionTerminatedGuardTimers.GetValueAt(nIndex);
    if (piSubscription)
    {
        StopTimer(piTimer);
        ReleaseISubscription(piSubscription, IMS_TRUE);
    }
}

PUBLIC
ISubscription* SubscriptionInterfaceHolder::GetISubscription(
        IN ISession* piSession, IN const AString& strEvent)
{
    IMS_TRACE_D("GetISubscription", 0, 0, 0);

    ISubscription* piSubscription = piSession->CreateSubscription(strEvent);
    m_objISubscriptions.Append(piSubscription);
    return piSubscription;
}

PUBLIC
ISubscription* SubscriptionInterfaceHolder::GetISubscription(IN ICoreService* piCoreService,
        IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent)
{
    IMS_TRACE_D("GetISubscription", 0, 0, 0);

    ISubscription* piSubscription = piCoreService->CreateSubscription(strFrom, strTo, strEvent);
    m_objISubscriptions.Append(piSubscription);
    return piSubscription;
}

PUBLIC
void SubscriptionInterfaceHolder::ReleaseISubscription(
        IN ISubscription* piSubscription, IN IMS_BOOL bTerminated /* = IMS_FALSE*/)
{
    IMS_TRACE_D("ReleaseISubscription", 0, 0, 0);

    if (piSubscription == IMS_NULL)
    {
        return;
    }

    piSubscription->SetListener(this);
    if (bTerminated || IsReadyToDestroy(piSubscription))
    {
        for (IMS_UINT32 i = 0; i < m_objISubscriptions.GetSize(); i++)
        {
            if (m_objISubscriptions.GetAt(i) == piSubscription)
            {
                ITimer* piTempTimer = GetTimer(piSubscription);
                StopTimer(piTempTimer);

                piSubscription->Destroy();
                m_objISubscriptions.RemoveAt(i);
                IMS_TRACE_D("ReleaseISubscription remove index=[%d]", i, 0, 0);

                if (m_objISubscriptions.GetSize() == 0)
                {
                    m_objListener.OnSubscriptionInterfaceCleared();
                }
                break;
            }
        }
    }
    else
    {
        StartTimer(piSubscription, TIME_TRANSACTION_TERMINATED_GUARD);
    }
}

PRIVATE
IMS_BOOL SubscriptionInterfaceHolder::IsReadyToDestroy(IN const ISubscription* piSubscription)
{
    IMS_TRACE_D("IsReadyToDestroy [%d]", piSubscription->GetState(), 0, 0);

    if (piSubscription->GetState() == ISubscription::STATE_INACTIVE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void SubscriptionInterfaceHolder::ClearISubscriptions()
{
    for (IMS_UINT32 i = 0; i < m_objISubscriptions.GetSize(); i++)
    {
        ISubscription* piSubscription = m_objISubscriptions.GetAt(i);
        if (piSubscription != IMS_NULL)
        {
            piSubscription->Destroy();
        }
    }

    m_objISubscriptions.Clear();
}

PRIVATE
IMS_RESULT SubscriptionInterfaceHolder::StartTimer(
        IN ISubscription* piSubscription, IN IMS_SINT32 nDuration)
{
    IMS_TRACE_D("StartTimer subscription duration[%d]", nDuration, 0, 0);

    if (nDuration <= 0)
    {
        return IMS_FAILURE;
    }

    ITimer* piTempTimer = GetTimer(piSubscription);
    if (piTempTimer != IMS_NULL)
    {
        return IMS_FAILURE;
    }

    piTempTimer = TimerService::GetTimerService()->CreateTimer();
    m_objSubscriptionTerminatedGuardTimers.Add(piTempTimer, piSubscription);
    piTempTimer->SetTimer(nDuration, this);

    return IMS_SUCCESS;
}

PRIVATE
void SubscriptionInterfaceHolder::StopTimer(IN ITimer* piTimer)
{
    IMS_TRACE_D("StopTimer", 0, 0, 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }

    m_objSubscriptionTerminatedGuardTimers.Remove(piTimer);
    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
}

PRIVATE
ITimer* SubscriptionInterfaceHolder::GetTimer(IN const ISubscription* piSubscription) const
{
    IMS_UINT32 nSize = m_objSubscriptionTerminatedGuardTimers.GetSize();
    IMS_TRACE_D("GetTimer subscription size = [%d]", nSize, 0, 0);

    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        if (piSubscription == m_objSubscriptionTerminatedGuardTimers.GetValueAt(i))
        {
            return m_objSubscriptionTerminatedGuardTimers.GetKeyAt(i);
        }
    }

    return IMS_NULL;
}
