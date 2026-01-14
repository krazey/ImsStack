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

#include "Engine.h"
#include "IConfiguration.h"
#include "ICoreService.h"
#include "ISession.h"
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ITimer.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ServiceThread.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include <algorithm>
#include <unordered_map>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SessionInterfaceHolder::SessionInterfaceHolder() :
        m_nTransactionGuardTimeMillis(0),
        m_objListeners(ImsList<IInterfaceHolderListener*>())
{
    IMS_TRACE_D("+SessionInterfaceHolder", 0, 0, 0);
    m_nTransactionGuardTimeMillis =
            Engine::GetConfiguration()
                    ->GetSipConfig(ThreadService::GetCurrentSlotId(IMS_SLOT_0))
                    ->GetSipConfigV()
                    ->GetTimerValue(ISipConfigV::TIMER_F) +
            TERMINATED_TRANSACTION_MARGIN_MS;
}

PUBLIC
SessionInterfaceHolder::~SessionInterfaceHolder()
{
    IMS_TRACE_D("~SessionInterfaceHolder", 0, 0, 0);

    for (const auto& sessionRecord : m_objSessionRecords)
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

    auto pRecord = std::find_if(m_objSessionRecords.begin(), m_objSessionRecords.end(),
            [&piTimer](const std::pair<CallKey, SessionRecord*>& record)
            {
                return record.second->piTimer == piTimer;
            });
    if (pRecord != m_objSessionRecords.end())
    {
        ISession* piSession = pRecord->second->piSession;
        StopTimer(piTimer);
        ReleaseISession(piSession, IMS_TRUE, IMS_FALSE);
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
            delete pRecord->second;
            m_objSessionRecords.erase(pRecord);

            if (m_objSessionRecords.count(nKey) == 0)
            {
                auto objListenersForNotify = m_objListeners;
                for (IMS_UINT32 i = 0; i < objListenersForNotify.GetSize(); ++i)
                {
                    objListenersForNotify.GetAt(i)->OnSessionInterfaceReleased(nKey, *piSession);
                }
                objListenersForNotify.Clear();
            }
            piSession->Destroy();
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
        IN const ISession* piSession, IN IMS_BOOL bSessionTerminatedOrStartFailed)
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
void SessionInterfaceHolder::StartTimer(IN const ISession* piSession)
{
    IMS_TRACE_D("StartTimer [%d]", m_nTransactionGuardTimeMillis, 0, 0);

    for (const auto& sessionRecord : m_objSessionRecords)
    {
        if (sessionRecord.second->piSession == piSession)
        {
            if (sessionRecord.second->piTimer)
            {
                return;
            }

            sessionRecord.second->piTimer = TimerService::GetTimerService()->CreateTimer();
            sessionRecord.second->piTimer->SetTimer(m_nTransactionGuardTimeMillis, this);
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

    auto pRecord = std::find_if(m_objSessionRecords.begin(), m_objSessionRecords.end(),
            [&piTimer](const std::pair<const CallKey, SessionRecord*>& record)
            {
                return record.second->piTimer == piTimer;
            });
    if (pRecord != m_objSessionRecords.end())
    {
        pRecord->second->piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        pRecord->second->piTimer = IMS_NULL;
    }
}

PRIVATE
ITimer* SessionInterfaceHolder::GetTimer(IN const ISession* piSession) const
{
    auto pRecord = std::find_if(m_objSessionRecords.begin(), m_objSessionRecords.end(),
            [&piSession](const std::pair<const CallKey, SessionRecord*>& record)
            {
                return record.second->piSession == piSession;
            });

    return (pRecord != m_objSessionRecords.end()) ? pRecord->second->piTimer : IMS_NULL;
}
