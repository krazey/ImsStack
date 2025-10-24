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
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "ImsMessageDef.h"
#include "OsThread.h"
#include "ServiceConfig.h"
#include "ServiceImsRadio.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServicePhoneInfo.h"
#include "ServiceThread.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#define WAIT_TIMEOUT_FOR_IPC 1      // ms
#define WAIT_TIMEOUT_FOR_RUN 10000  // us

__IMS_TRACE_TAG_IPL__;

static void osThread_Run(IN OsThread* pThread)
{
    if (pThread == IMS_NULL)
    {
        return;
    }

    INativeThreadMethods* piNativeThreadMethods = ThreadService::GetNativeThreadMethods();

    if (piNativeThreadMethods != IMS_NULL)
    {
        piNativeThreadMethods->AttachNativeThread(pThread->GetName().GetStr());
        pThread->Run();
        piNativeThreadMethods->DetachNativeThread();
    }
    else
    {
        pThread->Run();
    }

    pthread_exit(NULL);
}

static IMS_PVOID osThread_ThreadProc(void* param)
{
    osThread_Run(reinterpret_cast<OsThread*>(param));
    return IMS_NULL;
}

PUBLIC
OsThread::OsThread() :
        ImsThread(),
        m_strName(AString::ConstNull()),
        m_nThreadId(0),
        m_bIsRunning(IMS_FALSE),
        m_bSignalFlag(IMS_FALSE),
        m_nProcessingMsgIndex(-1),
        m_piListener(IMS_NULL)
{
    if (pthread_cond_init(&m_stCond, NULL) == 0)
    {
        // OK
    }
}

PUBLIC VIRTUAL OsThread::~OsThread()
{
    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_D("Thread(%s, %x) is destroyed", m_strName.GetStr(), m_nThreadId, 0);
    }

    if (pthread_cond_destroy(&m_stCond) == 0)
    {
        // OK
    }
}

PUBLIC VIRTUAL IMS_BOOL OsThread::Activate()
{
    if (IsRunning())
    {
        IMS_TRACE_D("Thread is already running ...", 0, 0, 0);
        return IMS_TRUE;
    }

    m_nThreadId = CreateThread();

    if (m_nThreadId == 0)
    {
        m_bIsRunning = IMS_FALSE;
    }
    else
    {
        IMS_TRACE_D("Thread :: Created (%x)", m_nThreadId, 0, 0);

        m_bIsRunning = IMS_TRUE;

        // Set a start event
        usleep(WAIT_TIMEOUT_FOR_RUN);
        PostMessage(IMS_MSG_START);
    }

    return IsRunning();
}

PUBLIC VIRTUAL void OsThread::Deactivate()
{
    if (!IsRunning())
    {
        IMS_TRACE_D("Thread is not running ...", 0, 0, 0);
        return;
    }

    // Sends a TERMINATE message to this thread.
    PostMessage(IMS_MSG_TERMINATE);

    JoinThread();
}

PUBLIC VIRTUAL IMS_BOOL OsThread::Equals(IN const IThread* piThread) const
{
    const OsThread* pThread = DYNAMIC_CAST(const OsThread*, piThread);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return GetName().Equals(pThread->GetName());
}

PUBLIC VIRTUAL IMS_BOOL OsThread::PostMessageI(
        IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    ImsMessage objMsg(nMsg, nWparam, nLparam);

    return PostMessageI(objMsg);
}

PUBLIC VIRTUAL IMS_BOOL OsThread::PostMessageI(IN ImsMessage& objMsg)
{
    if (!IsRunning())
    {
        return IMS_FALSE;
    }

    m_objMsgQueueMutex.Lock();
    IMS_SLONG nExpectedIndex = m_objMsgQueue.GetSize();
    IMS_SLONG nAddedIndex = m_objMsgQueue.Add(objMsg);
    m_objMsgQueueMutex.Unlock();

    if (SendSignal())
    {
        IMS_TRACE_D("PostMessageI: msg=%u, p1=%" PFLS_u ", p2=%" PFLS_u, objMsg.GetName(),
                objMsg.nWparam, objMsg.nLparam);
    }

    return nExpectedIndex == nAddedIndex;
}

PUBLIC VIRTUAL IMS_SINT32 OsThread::RemoveMessages(IN ImsMessage::IMessageCallback* piCallback,
        OUT ImsList<ImsMessage>* pImsMsgs /*= IMS_NULL*/)
{
    IMS_SINT32 nRemovedMsgCount = 0;

    m_objMsgQueueMutex.Lock();
    nRemovedMsgCount += RemoveMessages(m_objMsgQueue, 0, piCallback, pImsMsgs);
    m_objMsgQueueMutex.Unlock();

    m_objProcessingMsgsMutex.Lock();
    nRemovedMsgCount +=
            RemoveMessages(m_objProcessingMsgs, m_nProcessingMsgIndex + 1, piCallback, pImsMsgs);
    m_objProcessingMsgsMutex.Unlock();

    return nRemovedMsgCount;
}

PUBLIC VIRTUAL void OsThread::PostMessage(IN IMS_UINT32 nMessage)
{
    ImsMessage objMsg(nMessage, 0, 0);
    PostMessageI(objMsg);
}

PUBLIC GLOBAL IMS_ULONG OsThread::GetCurrentThreadId()
{
    return pthread_self();
}

PUBLIC VIRTUAL IMS_ULONG OsThread::Run()
{
    IMS_BOOL bLoop = IMS_TRUE;

    IMS_TRACE_I("Thread :: Started (%s, %x)", GetName().GetStr(), m_nThreadId, 0);

    while (bLoop)
    {
        m_objMsgQueueMutex.Lock();
        IMS_UINT32 nMsgCount = m_objMsgQueue.GetSize();
        m_objMsgQueueMutex.Unlock();

        IMS_SINT32 nWaitResult = WaitForSignal(nMsgCount);

        if ((nWaitResult == 0) || (nWaitResult == ETIMEDOUT))
        {
            m_objMsgQueueMutex.Lock();
            m_objProcessingMsgsMutex.Lock();
            m_objProcessingMsgs = m_objMsgQueue;
            m_objProcessingMsgsMutex.Unlock();
            m_objMsgQueue.Clear();
            m_objMsgQueueMutex.Unlock();

            m_objProcessingMsgsMutex.Lock();

            for (IMS_UINT32 i = 0; i < m_objProcessingMsgs.GetSize(); i++)
            {
                m_nProcessingMsgIndex = i;
                ImsMessage& objMsg = m_objProcessingMsgs.GetAt(i);
                IMS_UINT32 nName = objMsg.GetName();

                if (nName == IMS_MSG_START)
                {
                    OnStart(objMsg);
                }
                else if (nName == IMS_MSG_TERMINATE)
                {
                    OnTerminate(objMsg);
                    bLoop = IMS_FALSE;
                    break;
                }
                else if (IsSystemMessage(nName))
                {
                    OnSystemMessage(objMsg);
                }
                else
                {
                    OnThreadMessage(objMsg);
                }
            }

            m_nProcessingMsgIndex = -1;
            m_objProcessingMsgs.Clear();
            m_objProcessingMsgsMutex.Unlock();
        }
    }

    IMS_TRACE_D("Thread :: Terminated (%x)", m_nThreadId, 0, 0);

    m_bIsRunning = IMS_FALSE;

    return m_nThreadId;
}

PROTECTED VIRTUAL void OsThread::OnStart(IN ImsMessage& objMsg)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("OnStart(%s) :: No listener", GetName().GetStr(), 0, 0);
        return;
    }

    m_piListener->Runnable_Run(objMsg);
}

PROTECTED VIRTUAL void OsThread::OnTerminate(IN ImsMessage& objMsg)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("OnTerminate(%s) :: No listener", GetName().GetStr(), 0, 0);
        return;
    }

    m_piListener->Runnable_Run(objMsg);
}

PROTECTED VIRTUAL void OsThread::OnSystemMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case IMS_MSG_NETWORK:
        case IMS_MSG_SOCKET:
            NetworkService::GetNetworkService()->DispatchServiceMessage(objMsg);
            break;

        case IMS_MSG_NETWORK_STATUS:
        case IMS_MSG_BATTERY:
        case IMS_MSG_WIFI_STATUS:
        case IMS_MSG_ISIM:
        case IMS_MSG_USIM:
            PhoneInfoService::GetPhoneInfoService()->DispatchServiceMessage(objMsg);
            break;

        case IMS_MSG_TIMER:
            TimerService::GetTimerService()->DispatchServiceMessage(objMsg);
            break;
        case IMS_MSG_CONFIGURATION:
            ConfigService::GetConfigService()->DispatchServiceMessage(objMsg);
            break;
        case IMS_MSG_RADIO:
        case IMS_MSG_TRAFFIC:
            ImsRadioService::GetImsRadioService()->DispatchServiceMessage(objMsg);
            break;
    }
}

PROTECTED VIRTUAL void OsThread::OnThreadMessage(IN ImsMessage& objMsg)
{
    if (objMsg.HasCallback())
    {
        objMsg.InvokeCallback();
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("OnThreadMessage(%s) :: No listener", GetName().GetStr(), 0, 0);
        return;
    }

    m_piListener->Runnable_Run(objMsg);
}

PROTECTED VIRTUAL pthread_t OsThread::CreateThread()
{
    pthread_t nThreadId = 0;
    IMS_SINT32 nResult = pthread_create(
            &nThreadId, IMS_NULL, osThread_ThreadProc, reinterpret_cast<void*>(this));

    if (nResult != 0)
    {
        if (nResult == EAGAIN)
        {
            IMS_TRACE_E(0, "pthread_create - Failed (lack of resource)", 0, 0, 0);
        }
        else if (nResult == EINVAL)
        {
            IMS_TRACE_E(0, "pthread_create - Failed (invalid attribute)", 0, 0, 0);
        }
        else if (nResult == EPERM)
        {
            IMS_TRACE_E(0, "pthread_create - Failed (inappropriate permission)", 0, 0, 0);
        }
        else
        {
            IMS_TRACE_E(0, "pthread_create - Failed (%d)", nResult, 0, 0);
        }
    }

    return nThreadId;
}

PROTECTED VIRTUAL void OsThread::JoinThread()
{
    IMS_SINT32 nResult = pthread_join(m_nThreadId, IMS_NULL);

    if (nResult == 0)
    {
        IMS_TRACE_D("pthread_join - %lu", m_nThreadId, 0, 0);
    }
    else if (nResult == EINVAL)
    {
        IMS_TRACE_E(0, "pthread_join - failed (does not refer to a joinable thread)", 0, 0, 0);
    }
    else if (nResult == ESRCH)
    {
        IMS_TRACE_E(0, "pthread_join - failed (can not found thread id (%x))", m_nThreadId, 0, 0);
    }
    else
    {
        IMS_TRACE_E(0, "pthread_join - failed (%d)", nResult, 0, 0);
    }

    // RACE_CONDITION_HANDLING
    // To avoid the race condition between "thread exit" and "message posting"
    m_objCondMutex.Lock();
    m_objCondMutex.Unlock();
}

PROTECTED VIRTUAL IMS_BOOL OsThread::SendSignal()
{
    // Wake up this thread to run the message loop
    m_objCondMutex.Lock();

    SetSignal();

    IMS_SINT32 nResult = pthread_cond_signal(&m_stCond);

    if (nResult != 0)
    {
        IMS_TRACE_E(0, "pthread_cond_signal - error (%d)", nResult, 0, 0);
    }

    m_objCondMutex.Unlock();

    return (nResult == 0);
}

PROTECTED VIRTUAL IMS_SINT32 OsThread::WaitForSignal(IN IMS_SINT32 nMsgCount)
{
    IMS_SINT32 nWaitResult = 0;

    m_objCondMutex.Lock();

    if (nMsgCount == 0 && !IsSignaled())
    {
        nWaitResult = pthread_cond_wait(
                &m_stCond, reinterpret_cast<pthread_mutex_t*>(m_objCondMutex.GetMutexObj()));
    }
    else
    {
        struct timeval now;
        struct timespec ts;

        now.tv_sec = 0;
        now.tv_usec = 0;
        gettimeofday(&now, NULL);

        // 1 milli-second
        long nsec = (now.tv_usec + (1000 * WAIT_TIMEOUT_FOR_IPC)) * 1000;

        ts.tv_sec = now.tv_sec + (nsec / 1000000000L);
        ts.tv_nsec = (nsec % 1000000000L);

        nWaitResult = pthread_cond_timedwait(
                &m_stCond, reinterpret_cast<pthread_mutex_t*>(m_objCondMutex.GetMutexObj()), &ts);
    }

    ClearSignal();
    m_objCondMutex.Unlock();

    if ((nWaitResult == 0) || (nWaitResult == ETIMEDOUT))
    {
        // Thread is being waken up successfully.
    }
    else if (nWaitResult == EINVAL)
    {
        IMS_TRACE_E(0, "pthread_cond_wait failed - invalid parameter", 0, 0, 0);
    }
    else
    {
        IMS_TRACE_E(0, "pthread_cond_wait failed - error(%d)", nWaitResult, 0, 0);
    }

    return nWaitResult;
}

PROTECTED GLOBAL IMS_BOOL OsThread::IsSystemMessage(IN IMS_SINT32 nMsg)
{
    return (nMsg > IMS_MSG_SYSTEM_BASE) && (nMsg < IMS_MSG_SYSTEM_MAX);
}

PRIVATE
IMS_SINT32 OsThread::RemoveMessages(IN_OUT ImsVector<ImsMessage>& objMsgQueue,
        IN IMS_SINT32 nStartingIndex, IN const ImsMessage::IMessageCallback* piCallback,
        OUT ImsList<ImsMessage>* pImsMsgs)
{
    IMS_SINT32 nRemovedMsgCount = 0;

    for (IMS_UINT32 i = nStartingIndex; i < objMsgQueue.GetSize();)
    {
        const ImsMessage& objMsg = objMsgQueue.GetAt(i);

        if (objMsg.IsSameCallback(piCallback))
        {
            if (pImsMsgs != IMS_NULL)
            {
                pImsMsgs->Append(objMsg);
            }

            nRemovedMsgCount++;

            objMsgQueue.RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }

    return nRemovedMsgCount;
}
