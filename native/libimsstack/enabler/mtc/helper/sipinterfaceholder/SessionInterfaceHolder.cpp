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
#include "ITimer.h"
#include "ImsList.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include <unordered_map>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SessionInterfaceHolder::SessionInterfaceHolder() :
        m_objListeners(ImsList<IInterfaceHolderListener*>())
{
    IMS_TRACE_D("+SessionInterfaceHolder", 0, 0, 0);
}

PUBLIC
SessionInterfaceHolder::~SessionInterfaceHolder()
{
    IMS_TRACE_D("~SessionInterfaceHolder", 0, 0, 0);

    for (auto& sessionRecord : m_objSessionRecords)
    {
        if (sessionRecord.second->piSession != IMS_NULL)
        {
            sessionRecord.second->piSession->Destroy();
        }
        StopTimer(sessionRecord.second->piTimer);
        delete sessionRecord.second;
    }

    m_objSessionRecords.clear();
}

PUBLIC VIRTUAL void SessionInterfaceHolder::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStarted", 0, 0, 0);
    piSession->SendAck();
    piSession->Terminate();
}

PUBLIC VIRTUAL void SessionInterfaceHolder::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_D("SessionStartFailed", 0, 0, 0);
    ReleaseISession(piSession, IMS_TRUE, IMS_TRUE);
}

PUBLIC VIRTUAL void SessionInterfaceHolder::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_D("SessionTerminated", 0, 0, 0);
    ReleaseISession(piSession, IMS_TRUE, IMS_TRUE);
}

PUBLIC VIRTUAL void SessionInterfaceHolder::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

    for (auto pRecord = m_objSessionRecords.begin(); pRecord != m_objSessionRecords.end();
            ++pRecord)
    {
        if (pRecord->second->piTimer == piTimer)
        {
            ISession* piSession = pRecord->second->piSession;
            StopTimer(piTimer);
            ReleaseISession(piSession, IMS_TRUE, IMS_FALSE);
            break;
        }
    }
}

PUBLIC
void SessionInterfaceHolder::AddListener(IN IInterfaceHolderListener* piListener)
{
    m_objListeners.Append(piListener);
}

PUBLIC
void SessionInterfaceHolder::RemoveListener(IN IInterfaceHolderListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        if (m_objListeners.GetAt(i) == piListener)
        {
            m_objListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC
ISession* SessionInterfaceHolder::GetISession(IN CallKey nKey, IN ICoreService* piCoreService,
        IN const AString& strFrom, IN const AString& strTo)
{
    IMS_TRACE_D("GetISession", 0, 0, 0);
    if (piCoreService == IMS_NULL)
    {
        return IMS_NULL;
    }

    ISession* piSession = piCoreService->CreateSession(strFrom, strTo);
    AddISession(nKey, piSession);
    return piSession;
}

PUBLIC
void SessionInterfaceHolder::AddISession(CallKey nKey, IN ISession* piSession)
{
    IMS_TRACE_D("AddISession [%d]", nKey, 0, 0);
    SessionRecord* pSessionRecord = new SessionRecord(piSession);
    m_objSessionRecords.emplace(nKey, pSessionRecord);
}

PUBLIC
void SessionInterfaceHolder::ReleaseISession(IN ISession* piSession)
{
    ReleaseISession(piSession, IMS_FALSE, IMS_FALSE);
}

PUBLIC
void SessionInterfaceHolder::ReleaseISession(IN ISession* piSession, IN IMS_BOOL bEnforceDestroy,
        IN IMS_BOOL bSessionTerminatedOrStartFailed)
{
    if (piSession == IMS_NULL)
    {
        return;
    }

    IMS_UINT32 nSize = m_objSessionRecords.size();
    IMS_TRACE_D("ReleaseISession size=[%d] EnforceDestroy[%s] SessionTerminated(%s)", nSize,
            _TRACE_B_(bEnforceDestroy), _TRACE_B_(bSessionTerminatedOrStartFailed));

    piSession->SetListener(this);

    if (bEnforceDestroy || IsReadyToDestroy(piSession, bSessionTerminatedOrStartFailed))
    {
        auto pRecord = std::find_if(m_objSessionRecords.begin(), m_objSessionRecords.end(),
                [&piSession](const std::pair<CallKey, SessionRecord*>& record)
                {
                    return record.second->piSession == piSession;
                });
        if (pRecord != m_objSessionRecords.end())
        {
            CallKey nKey = pRecord->first;
            IMS_TRACE_D("ReleaseISession remove call key=[%d]", nKey, 0, 0);

            ITimer* piTempTimer = pRecord->second->piTimer;
            StopTimer(piTempTimer);

            piSession->Destroy();
            delete pRecord->second;
            m_objSessionRecords.erase(pRecord);
            if (m_objSessionRecords.count(nKey) == 0)
            {
                for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
                {
                    m_objListeners.GetAt(i)->OnSessionInterfaceReleased(nKey);
                }
            }
        }
        else
        {
            IMS_TRACE_D("ReleaseISession not found", 0, 0, 0);
        }
    }
    else
    {
        StartTimer(piSession);
    }
}

PRIVATE
IMS_BOOL SessionInterfaceHolder::IsReadyToDestroy(
        IN ISession* piSession, IN IMS_BOOL bSessionTerminatedOrStartFailed)
{
    IMS_TRACE_D("IsReadyToDestroy [%d]", piSession->GetState(), 0, 0);
    if (bSessionTerminatedOrStartFailed)
    {
        if (piSession->GetState() != ISession::STATE_TERMINATING)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
void SessionInterfaceHolder::StartTimer(IN ISession* piSession)
{
    IMS_TRACE_D("StartTimer", 0, 0, 0);

    for (auto& sessionRecord : m_objSessionRecords)
    {
        if (sessionRecord.second->piSession == piSession)
        {
            if (sessionRecord.second->piTimer)
            {
                return;
            }

            sessionRecord.second->piTimer = TimerService::GetTimerService()->CreateTimer();
            sessionRecord.second->piTimer->SetTimer(TIME_TRANSACTION_TERMINATED_GUARD, this);
            return;
        }
    }
}

PRIVATE
void SessionInterfaceHolder::StopTimer(IN ITimer* piTimer)
{
    IMS_TRACE_D("StopTimer", 0, 0, 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }

    for (auto& sessionRecord : m_objSessionRecords)
    {
        if (sessionRecord.second->piTimer == piTimer)
        {
            sessionRecord.second->piTimer->KillTimer();
            TimerService::GetTimerService()->DestroyTimer(piTimer);
            sessionRecord.second->piTimer = IMS_NULL;
            break;
        }
    }
}

PRIVATE
ITimer* SessionInterfaceHolder::GetTimer(IN const ISession* piSession) const
{
    for (auto& sessionRecord : m_objSessionRecords)
    {
        if (sessionRecord.second->piSession == piSession)
        {
            return sessionRecord.second->piTimer;
        }
    }
    return IMS_NULL;
}
