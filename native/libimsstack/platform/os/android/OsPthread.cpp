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

#include "ImsQueue.h"
#include "OsMutex.h"
#include "OsPthread.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_IPL__;

static void osPthread_Run(IN OsPthread* pThread)
{
    if (pThread == IMS_NULL)
    {
        return;
    }

    pThread->Run();
    pthread_exit(NULL);
}

static IMS_PVOID osPthread_ThreadProc(void* param)
{
    osPthread_Run(reinterpret_cast<OsPthread*>(param));
    return IMS_NULL;
}

class OsPthreadPrivate
{
public:
    OsPthreadPrivate();
    ~OsPthreadPrivate();

public:
    void CleanUp();

public:
    IMS_BOOL m_bIsRunning;

    pthread_t m_nThreadId;
    // Name of this thread
    AString m_strName;
};

PUBLIC
OsPthreadPrivate::OsPthreadPrivate() :
        m_bIsRunning(IMS_FALSE),
        m_nThreadId(0),
        m_strName(AString::ConstNull())
{
}

PUBLIC
OsPthreadPrivate::~OsPthreadPrivate() {}

PUBLIC
void OsPthreadPrivate::CleanUp()
{
    m_nThreadId = 0;
    m_bIsRunning = IMS_FALSE;
}

PUBLIC
OsPthread::OsPthread() :
        ImsThread(),
        m_pThreadP(new OsPthreadPrivate()),
        m_piImpListener(IMS_NULL)
{
}

PUBLIC VIRTUAL OsPthread::~OsPthread()
{
    CleanUp();

    if (m_pThreadP != IMS_NULL)
    {
        delete m_pThreadP;
    }

    m_piImpListener = IMS_NULL;
    m_pThreadP = IMS_NULL;
}

PUBLIC VIRTUAL IMS_BOOL OsPthread::Activate()
{
    if (m_pThreadP->m_bIsRunning)
    {
        return IMS_TRUE;
    }

    IMS_SINT32 nResult = pthread_create(
            &(m_pThreadP->m_nThreadId), NULL, osPthread_ThreadProc, reinterpret_cast<void*>(this));

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
        m_pThreadP->m_bIsRunning = IMS_TRUE;

        IMS_TRACE_D("PThread :: Created (%x)", m_pThreadP->m_nThreadId, 0, 0);

        nResult = pthread_detach(m_pThreadP->m_nThreadId);

        if (nResult != 0)
        {
            if (nResult == EINVAL)
            {
                IMS_TRACE_E(0, "pthread_detach - Failed (does not refer to a joinable thread)", 0,
                        0, 0);
            }
            else if (nResult == ESRCH)
            {
                IMS_TRACE_E(0, "pthread_detach - Failed (can not found thread id (%x))",
                        m_pThreadP->m_nThreadId, 0, 0);
            }
            else
            {
                IMS_TRACE_E(0, "pthread_detach - Failed (%d)", nResult, 0, 0);
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsPthread::Deactivate()
{
    if (!(m_pThreadP->m_bIsRunning))
    {
        IMS_TRACE_D("Thread is not running", 0, 0, 0);
        return;
    }

    IMS_SINT32 nResult = pthread_join(m_pThreadP->m_nThreadId, IMS_NULL);

    if (nResult == 0)
    {
        IMS_TRACE_D("pthread_join - %lu", m_pThreadP->m_nThreadId, 0, 0);
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
}

PUBLIC VIRTUAL IMS_BOOL OsPthread::Equals(IN const IThread* piThread) const
{
    const OsPthread* pThread = DYNAMIC_CAST(OsPthread*, piThread);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return GetName().Equals(pThread->GetName());
}

PUBLIC VIRTUAL const AString& OsPthread::GetName() const
{
    return m_pThreadP->m_strName;
}

PUBLIC VIRTUAL IMS_BOOL OsPthread::IsRunning() const
{
    return m_pThreadP->m_bIsRunning;
}

PUBLIC VIRTUAL void OsPthread::SetImpListener(IN IThreadImpListener* piListener)
{
    m_piImpListener = piListener;
}

PUBLIC VIRTUAL void OsPthread::SetRunnable(IN IRunnable* /*piListener*/)
{
    // no-op (PThread doesn't require IRunnable interface)
}

PUBLIC VIRTUAL IMS_BOOL OsPthread::PostMessageI(IN ImsMessage& /*objMsg*/)
{
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL OsPthread::PostMessageI(
        IN IMS_UINT32 /*nMsg*/, IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/)
{
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL OsPthread::Create(IN const AString& strName)
{
    m_pThreadP->m_strName = strName;

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_ULONG OsPthread::GetThreadId() const
{
    return static_cast<IMS_ULONG>(m_pThreadP->m_nThreadId);
}

PUBLIC GLOBAL IMS_ULONG OsPthread::GetCurrentThreadId()
{
    return static_cast<IMS_ULONG>(pthread_self());
}

PROTECTED VIRTUAL IMS_ULONG OsPthread::Run()
{
    IMS_TRACE_I("PThread :: Started (%s, %x, %p)", GetName().GetStr(), m_pThreadP->m_nThreadId,
            m_piImpListener);

    if (m_piImpListener != IMS_NULL)
    {
        m_piImpListener->RunImp();
    }

    if (m_pThreadP == IMS_NULL)
    {
        IMS_TRACE_D("PThread is destroyed", 0, 0, 0);
        return 0x00;
    }

    IMS_ULONG nThread = m_pThreadP->m_nThreadId;

    m_pThreadP->m_nThreadId = 0x00;
    m_pThreadP->m_bIsRunning = IMS_FALSE;

    return nThread;
}

PROTECTED
void OsPthread::CleanUp()
{
    m_pThreadP->CleanUp();
}
