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
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "ImsMessageDef.h"
#include "IMSQueue.h"
#include "OsMutex.h"
#include "OsThread.h"
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "ServiceVoNr.h"

#define WAIT_TIMEOUT_FOR_IPC        1              // ms
#define WAIT_TIMEOUT_FOR_RUN        10000          // us

extern void JNI_AttachNativeThread(const char* threadName);
extern void JNI_DetachNativeThread(void);

__IMS_TRACE_TAG_ADAPT__;



LOCAL
IMS_PVOID osThread_Run(IN OsThread* pThread)
{
    if (pThread == IMS_NULL)
    {
        return IMS_NULL;
    }

    JNI_AttachNativeThread(pThread->GetName().GetStr());

    IMS_PVOID pvThread = reinterpret_cast<IMS_PVOID>(pThread->Run());

    JNI_DetachNativeThread();

    pthread_exit(NULL);

    return pvThread;
}

LOCAL
IMS_PVOID osThread_ThreadProc(void* lpParam)
{
    return osThread_Run(reinterpret_cast<OsThread*>(lpParam));
}



class OsThreadPrivate
{
public:
    OsThreadPrivate();
    virtual ~OsThreadPrivate();

public:
    IMS_UINT32 GetQueueSize();
    IMS_BOOL Enqueue(IN const ImsMessage& objMsg);
    ImsMessage Dequeue();
    void CleanUp();

    // Internal signal flag to avoid timing issue
    inline void SetSignal()
    { m_bSignalFlag = IMS_TRUE; }
    inline void ClearSignal()
    { m_bSignalFlag = IMS_FALSE; }
    inline IMS_BOOL IsSignaled() const
    { return m_bSignalFlag; }

public:
    IMS_BOOL m_bIsRunning;

    pthread_t m_nThreadId;
    pthread_cond_t m_stIpcThreadCond;
    IMS_BOOL m_bSignalFlag;

    OsMutex m_objMsgQMutex;
    OsMutex m_objIpcMutex;

    // Name of this thread
    AString m_strName;
    IMSQueue<ImsMessage> m_objMsgQ;
};



PUBLIC
OsThreadPrivate::OsThreadPrivate()
    : m_bIsRunning(IMS_FALSE)
    , m_nThreadId(0)
    , m_bSignalFlag(IMS_FALSE)
    , m_strName(AString::ConstNull())
{
    if (pthread_cond_init(&m_stIpcThreadCond, NULL) == 0)
    {
        // OK
    }
}

PUBLIC
OsThreadPrivate::~OsThreadPrivate()
{
    if (pthread_cond_destroy(&m_stIpcThreadCond) == 0)
    {
        // OK
    }
}

PUBLIC
IMS_UINT32 OsThreadPrivate::GetQueueSize()
{
    IMS_UINT32 nSize = 0;

    m_objMsgQMutex.Lock();
    nSize = m_objMsgQ.GetSize();
    m_objMsgQMutex.Unlock();

    return nSize;
}

PUBLIC
IMS_BOOL OsThreadPrivate::Enqueue(IN const ImsMessage& objMsg)
{
    IMS_UINT32 nOldSize = 0;
    IMS_UINT32 nNewSize = 0;

    m_objMsgQMutex.Lock();
    nOldSize = m_objMsgQ.GetSize();
    m_objMsgQ.Push(objMsg);
    nNewSize = m_objMsgQ.GetSize();
    m_objMsgQMutex.Unlock();

    return (nNewSize > nOldSize);
}

PUBLIC
ImsMessage OsThreadPrivate::Dequeue()
{
    ImsMessage objMsg;

    m_objMsgQMutex.Lock();
    objMsg = m_objMsgQ.GetFront();
    m_objMsgQ.Pop();
    m_objMsgQMutex.Unlock();

    return objMsg;
}

PUBLIC
void OsThreadPrivate::CleanUp()
{
    m_nThreadId = 0;
    m_bIsRunning = IMS_FALSE;
}



PUBLIC
OsThread::OsThread()
    : ImsThread()
    , m_piListener(IMS_NULL)
    , m_pThreadP(new OsThreadPrivate())
{
}

PUBLIC VIRTUAL
OsThread::~OsThread()
{
    if (IMS_UTIL_SYS_PROP_IS_DEBUG_MODE())
    {
        IMS_TRACE_D("Thread(%s, %x) is destroyed",
                m_pThreadP->m_strName.GetStr(), m_pThreadP->m_nThreadId, 0);
    }

    CleanUp();

    if (m_pThreadP != IMS_NULL)
    {
        delete m_pThreadP;
        m_pThreadP = IMS_NULL;
    }
}

PUBLIC VIRTUAL
IMS_BOOL OsThread::Activate()
{
    if (m_pThreadP->m_bIsRunning)
    {
        IMS_TRACE_D("Thread is already running ...", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nResult = pthread_create(
            &(m_pThreadP->m_nThreadId),
            IMS_NULL,
            osThread_ThreadProc,
            reinterpret_cast<void*>(this));

    // Check the result
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

        m_pThreadP->m_nThreadId = 0x00;
        m_pThreadP->m_bIsRunning = IMS_FALSE;

        return IMS_FALSE;
    }
    else
    {
        IMS_TRACE_D("Thread :: Created (%x)", m_pThreadP->m_nThreadId, 0, 0);

        m_pThreadP->m_bIsRunning = IMS_TRUE;

        // Set a start event
        usleep(WAIT_TIMEOUT_FOR_RUN);
        PostMessage(IMS_MSG_START);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL
void OsThread::Deactivate()
{
    if (!m_pThreadP->m_bIsRunning)
    {
        IMS_TRACE_D("Thread is not running ...", 0, 0, 0);
        return;
    }

    // Terminate the thread
    ImsMessage objMsg(IMS_MSG_TERMINATE, 0, 0);
    PostMessageI(objMsg);

    IMS_PVOID pvReturnValue;
    IMS_SINT32 nResult = pthread_join(m_pThreadP->m_nThreadId, &pvReturnValue);

    if (nResult == 0)
    {
        IMS_TRACE_D("pthread_join - success", 0, 0, 0);
    }
    else if (nResult == EINVAL)
    {
        IMS_TRACE_E(0, "pthread_join - failed (does not refer to a joinable thread)", 0, 0, 0);
    }
    else if (nResult == ESRCH)
    {
        IMS_TRACE_E(0, "pthread_join - failed (can not found thread id (%x))",
                m_pThreadP->m_nThreadId, 0, 0);
    }
    else
    {
        IMS_TRACE_E(0, "pthread_join - failed (%d)", nResult, 0, 0);
    }

    // RACE_CONDITION_HANDLING
    // To avoid the race condition between "thread exit" and "message posting"
    m_pThreadP->m_objIpcMutex.Lock();
    m_pThreadP->m_objIpcMutex.Unlock();
}

PUBLIC VIRTUAL
IMS_BOOL OsThread::Equals(IN const IThread* piThread) const
{
    const OsThread* pThread = DYNAMIC_CAST(const OsThread*, piThread);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return GetName().Equals(pThread->GetName());
}

PUBLIC VIRTUAL
const AString& OsThread::GetName() const
{
    return m_pThreadP->m_strName;
}

PUBLIC VIRTUAL
IMS_BOOL OsThread::IsRunning() const
{
    return m_pThreadP->m_bIsRunning;
}

PUBLIC VIRTUAL
void OsThread::SetRunnable(IN IRunnable* piListener)
{
    m_piListener = piListener;
}

PUBLIC VIRTUAL
IMS_BOOL OsThread::PostMessageI(IN IMS_UINT32 nMsg,
        IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    ImsMessage objMsg(nMsg, nWparam, nLparam);

    return PostMessageI(objMsg);
}

PUBLIC VIRTUAL
IMS_BOOL OsThread::PostMessageI(IN ImsMessage& objMsg)
{
    if (!m_pThreadP->m_bIsRunning)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bResult = m_pThreadP->Enqueue(objMsg);

    // Wake up the thread to run the message loop
    m_pThreadP->m_objIpcMutex.Lock();

    m_pThreadP->SetSignal();

    IMS_SINT32 nResult = pthread_cond_signal( &(m_pThreadP->m_stIpcThreadCond) );

    if (nResult != 0)
    {
        IMS_TRACE_E(0, "pthread_cond_signal - error (%d)", nResult, 0, 0);
    }
    else
    {
        IMS_TRACE_D("PostMessageI() - event (%u), w (%" PFLS_u "), l (%" PFLS_u ")",
                objMsg.GetName(), objMsg.nWparam, objMsg.nLparam);
    }

    // Unlock the mutex
    m_pThreadP->m_objIpcMutex.Unlock();

    return bResult;
}

PUBLIC VIRTUAL
void OsThread::PostMessage(IN IMS_UINT32 nMessage)
{
    ImsMessage objMsg(nMessage, 0, 0);

    PostMessageI(objMsg);
}

PUBLIC VIRTUAL
IMS_BOOL OsThread::Create(IN const AString& strName)
{
    m_pThreadP->m_strName = strName;

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_ULONG OsThread::GetThreadId() const
{
    return static_cast<IMS_ULONG>(m_pThreadP->m_nThreadId);
}

PUBLIC GLOBAL
IMS_ULONG OsThread::GetCurrentThreadId()
{
    return pthread_self();
}

PROTECTED VIRTUAL
IMS_ULONG OsThread::Run()
{
    ImsMessage objMsg;
    IMS_BOOL bLoop = IMS_TRUE;
    IMS_UINT32 nMsgCount = 0;
    IMS_SINT32 nWaitResult = 0;

    IMS_TRACE_I("Thread :: Started (%s, %x)", GetName().GetStr(), m_pThreadP->m_nThreadId, 0);

    while (bLoop)
    {
        nMsgCount = m_pThreadP->GetQueueSize();

        // Lock the mutex
        m_pThreadP->m_objIpcMutex.Lock();

        if (nMsgCount == 0 && !m_pThreadP->IsSignaled())
        {
            nWaitResult = pthread_cond_wait(
                    &(m_pThreadP->m_stIpcThreadCond),
                    reinterpret_cast<pthread_mutex_t*>(m_pThreadP->m_objIpcMutex.GetMutexObj()));
        }
        else
        {
#if 1 // FOR_O_OS : defined(__IMS_LP64__)
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
                    &(m_pThreadP->m_stIpcThreadCond),
                    reinterpret_cast<pthread_mutex_t*>(m_pThreadP->m_objIpcMutex.GetMutexObj()),
                    &ts);
#else
            nWaitResult = pthread_cond_timeout_np(
                    &(m_pThreadP->m_stIpcThreadCond),
                    reinterpret_cast<pthread_mutex_t*>(m_pThreadP->m_objIpcMutex.GetMutexObj()),
                    WAIT_TIMEOUT_FOR_IPC);
#endif
        }

        m_pThreadP->ClearSignal();

        // Unlock the mutex
        m_pThreadP->m_objIpcMutex.Unlock();

        if ((nWaitResult == 0) || (nWaitResult == ETIMEDOUT))
        {
            nMsgCount = m_pThreadP->GetQueueSize();

            for (IMS_UINT32 i = 0; i < nMsgCount; i++)
            {
                objMsg = m_pThreadP->Dequeue();

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
        }
        else if (nWaitResult == EINVAL)
        {
            IMS_TRACE_E(0, "pthread_cond_wait failed - invalid parameter", 0, 0, 0);
        }
        else
        {
            IMS_TRACE_E(0, "pthread_cond_wait failed - error(%d)", nWaitResult, 0, 0);
        }
    }

    IMS_TRACE_D("Thread :: Terminated (%x)", m_pThreadP->m_nThreadId, 0, 0);

    m_pThreadP->m_bIsRunning = IMS_FALSE;

    return m_pThreadP->m_nThreadId;
}

PROTECTED VIRTUAL
void OsThread::OnStart(IN ImsMessage &objMsg)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("OnStart(%s) :: No listener", GetName().GetStr(), 0, 0);
        return;
    }

    m_piListener->Runnable_Run(objMsg);
}

PROTECTED VIRTUAL
void OsThread::OnTerminate(IN ImsMessage& objMsg)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("OnTerminate(%s) :: No listener", GetName().GetStr(), 0, 0);
        return;
    }

    m_piListener->Runnable_Run(objMsg);
}

PROTECTED VIRTUAL
void OsThread::OnSystemMessage(IN ImsMessage& objMsg)
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
        case IMS_MSG_TRM_PRIORITY_STATUS:
            PhoneInfoService::GetPhoneInfoService()->DispatchServiceMessage(objMsg);
            break;

        case IMS_MSG_TIMER:
            TimerService::GetTimerService()->DispatchServiceMessage(objMsg);
            break;
        case IMS_MSG_CONFIGURATION:
            ConfigService::GetConfigService()->DispatchServiceMessage(objMsg);
            break;
        case IMS_MSG_VONR:
            VoNrService::GetVoNrService()->DispatchServiceMessage(objMsg);
            break;
    }
}

PROTECTED VIRTUAL
void OsThread::OnThreadMessage(IN ImsMessage& objMsg)
{
    if (objMsg.HasCallback())
    {
        InvokeMessageCallback(objMsg);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("OnThreadMessage(%s) :: No listener", GetName().GetStr(), 0, 0);
        return;
    }

    m_piListener->Runnable_Run(objMsg);
}

PROTECTED
void OsThread::CleanUp()
{
    m_pThreadP->CleanUp();
}

PROTECTED GLOBAL
IMS_BOOL OsThread::IsSystemMessage(IN IMS_SINT32 nMsg)
{
    return ((nMsg == IMS_MSG_NETWORK)
            || (nMsg == IMS_MSG_SOCKET)
            || (nMsg == IMS_MSG_BATTERY)
            || (nMsg == IMS_MSG_NETWORK_STATUS)
            || (nMsg == IMS_MSG_TIMER)
            || (nMsg == IMS_MSG_CONFIGURATION)
            || (nMsg == IMS_MSG_WIFI_STATUS)
            || (nMsg == IMS_MSG_ISIM)
            || (nMsg == IMS_MSG_USIM)
            || (nMsg == IMS_MSG_TRM_PRIORITY_STATUS)
            || (nMsg == IMS_MSG_VONR));
}
