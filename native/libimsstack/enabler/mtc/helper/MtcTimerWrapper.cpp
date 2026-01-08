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

#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "helper/IMtcTimerListener.h"
#include "helper/MtcTimerWrapper.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcTimerWrapper::MtcTimerWrapper() :
        m_lstTimers(ImsList<MtcTimer*>()),
        m_piListener(IMS_NULL)
{
    IMS_TRACE_D("+MtcTimerWrapper", 0, 0, 0);
}

PUBLIC
MtcTimerWrapper::~MtcTimerWrapper()
{
    IMS_TRACE_D("~MtcTimerWrapper", 0, 0, 0);
    Clear();
}

PUBLIC VIRTUAL void MtcTimerWrapper::Timer_TimerExpired(IN ITimer* piTimer)
{
    for (IMS_UINT32 i = 0; i < m_lstTimers.GetSize(); i++)
    {
        MtcTimer* pTimer = m_lstTimers.GetAt(i);
        if (pTimer->piTimer == piTimer)
        {
            IMS_UINT32 eType = pTimer->eType;
            delete pTimer;
            m_lstTimers.RemoveAt(i);
            m_piListener->OnTimerExpired(eType);
            break;
        }
    }
}

PUBLIC
void MtcTimerWrapper::SetListener(IN IMtcTimerListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
void MtcTimerWrapper::Start(IN IMS_UINT32 eType, IN IMS_SINT32 nDuration)
{
    if (IsActive(eType))
    {
        IMS_TRACE_I("Start : Type:[%d] timer is already running >> re-start", eType, 0, 0);
        Stop(eType);
    }

    if (nDuration < 0)
    {
        return;
    }

    MtcTimer* pTimer = new MtcTimer(eType);
    pTimer->piTimer->SetTimer(nDuration, this);
    m_lstTimers.Append(pTimer);

    IMS_TRACE_I("Start : Type[%d] Duration[%d]", eType, nDuration, 0);
}

PUBLIC
void MtcTimerWrapper::Stop(IN IMS_UINT32 eType)
{
    for (IMS_UINT32 i = 0; i < m_lstTimers.GetSize(); i++)
    {
        MtcTimer* pTimer = m_lstTimers.GetAt(i);

        if (eType != pTimer->eType)
        {
            continue;
        }

        IMS_TRACE_I("Stop : Type[%d]", eType, 0, 0);

        delete pTimer;
        m_lstTimers.RemoveAt(i);

        return;
    }

    IMS_TRACE_I("Stop : Not Found Type[%d]", eType, 0, 0);
}

PUBLIC
void MtcTimerWrapper::StopAll()
{
    Clear();
}

PUBLIC
IMS_BOOL MtcTimerWrapper::IsActive(IN IMS_UINT32 eType)
{
    for (IMS_UINT32 i = 0; i < m_lstTimers.GetSize(); i++)
    {
        if (eType == m_lstTimers.GetAt(i)->eType)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
void MtcTimerWrapper::Clear()
{
    IMS_UINT32 nSize = m_lstTimers.GetSize();

    IMS_TRACE_I("StopAll : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        delete m_lstTimers.GetAt(i);
    }

    m_lstTimers.Clear();
}
