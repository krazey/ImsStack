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
#include "ServiceTrace.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SessionInterfaceHolder::SessionInterfaceHolder(IN IInterfaceHolderListener& objListener) :
        m_objListener(objListener)
{
    IMS_TRACE_D("+SessionInterfaceHolder", 0, 0, 0);
}

PUBLIC
SessionInterfaceHolder::~SessionInterfaceHolder()
{
    IMS_TRACE_D("~SessionInterfaceHolder", 0, 0, 0);

    ClearISessions();

    for (IMS_SINT32 i = static_cast<IMS_SINT32>(m_objTerminatedGuardTimers.GetSize()) - 1; i >= 0;
            i--)
    {
        StopTimer(m_objTerminatedGuardTimers.GetKeyAt(i));
    }
    m_objTerminatedGuardTimers.Clear();
}

PUBLIC VIRTUAL void SessionInterfaceHolder::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);

    ReleaseISession(piSession, IMS_TRUE);
}

PUBLIC VIRTUAL void SessionInterfaceHolder::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

    IMS_SLONG nIndex = m_objTerminatedGuardTimers.GetIndexOfKey(piTimer);
    if (nIndex < 0)
    {
        return;
    }

    ISession* piSession = m_objTerminatedGuardTimers.GetValueAt(nIndex);

    if (piSession)
    {
        StopTimer(piTimer);
        ReleaseISession(piSession, IMS_TRUE);
    }
}

PUBLIC
ISession* SessionInterfaceHolder::GetISession(
        IN ICoreService* piCoreService, IN const AString& strFrom, IN const AString& strTo)
{
    IMS_TRACE_D("GetISession", 0, 0, 0);
    if (piCoreService == IMS_NULL)
    {
        return IMS_NULL;
    }

    ISession* piSession = piCoreService->CreateSession(strFrom, strTo);
    m_objISessions.Append(piSession);
    return piSession;
}

PUBLIC
void SessionInterfaceHolder::AddISession(IN ISession* piSession)
{
    IMS_TRACE_D("AddISession", 0, 0, 0);
    m_objISessions.Append(piSession);
}

PUBLIC
void SessionInterfaceHolder::ReleaseISession(
        IN ISession* piSession, IN IMS_BOOL bTerminated /* = IMS_FALSE*/)
{
    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_UINT32 nSize = m_objISessions.GetSize();
    IMS_TRACE_D("ReleaseISession size=[%d]", nSize, 0, 0);

    piSession->SetListener(this);

    if (bTerminated || IsReadyToDestroy(piSession))
    {
        for (IMS_UINT32 i = 0; i < nSize; i++)
        {
            if (m_objISessions.GetAt(i) == piSession)
            {
                ITimer* piTempTimer = GetTimer(piSession);
                StopTimer(piTempTimer);

                piSession->Destroy();
                m_objISessions.RemoveAt(i);
                IMS_TRACE_D("ReleaseISession remove index=[%d]", i, 0, 0);

                if (m_objISessions.GetSize() == 0)
                {
                    m_objListener.OnSessionInterfaceCleared();
                }
                break;
            }
        }
    }
    else
    {
        StartTimer(piSession, TIME_TRANSACTION_TERMINATED_GUARD);
    }
}

PRIVATE
IMS_BOOL SessionInterfaceHolder::IsReadyToDestroy(IN ISession* piSession)
{
    IMS_TRACE_D("IsReadyToDestroy [%d]", piSession->GetState(), 0, 0);

    if (piSession->GetState() == ISession::STATE_TERMINATED)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void SessionInterfaceHolder::ClearISessions()
{
    IMS_TRACE_D("ClearISessions [%d]", m_objISessions.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objISessions.GetSize(); i++)
    {
        ISession* piSession = m_objISessions.GetAt(i);
        if (piSession != IMS_NULL)
        {
            piSession->Destroy();
        }
    }

    m_objISessions.Clear();
}

PRIVATE
IMS_RESULT SessionInterfaceHolder::StartTimer(IN ISession* piSession, IN IMS_SINT32 nDuration)
{
    IMS_TRACE_D("StartTimer duration[%d]", nDuration, 0, 0);

    if (nDuration <= 0)
    {
        return IMS_FAILURE;
    }

    ITimer* piTempTimer = GetTimer(piSession);
    if (piTempTimer)
    {
        return IMS_FAILURE;
    }

    piTempTimer = TimerService::GetTimerService()->CreateTimer();
    m_objTerminatedGuardTimers.Add(piTempTimer, piSession);
    piTempTimer->SetTimer(nDuration, this);

    return IMS_SUCCESS;
}

PRIVATE
void SessionInterfaceHolder::StopTimer(IN ITimer* piTimer)
{
    IMS_TRACE_D("StopTimer", 0, 0, 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }

    m_objTerminatedGuardTimers.Remove(piTimer);
    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
}

PRIVATE
ITimer* SessionInterfaceHolder::GetTimer(IN const ISession* piSession) const
{
    IMS_UINT32 nSize = m_objTerminatedGuardTimers.GetSize();
    IMS_TRACE_D("GetTimer size = [%d]", nSize, 0, 0);

    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        if (piSession == m_objTerminatedGuardTimers.GetValueAt(i))
        {
            return m_objTerminatedGuardTimers.GetKeyAt(i);
        }
    }

    return IMS_NULL;
}
