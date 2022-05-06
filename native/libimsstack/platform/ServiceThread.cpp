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
#include "ImsThread.h"
#include "PlatformFactory.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceThread.h"

PRIVATE
ThreadService::ThreadService()
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
ThreadService::~ThreadService()
{
    if (m_piLock != IMS_NULL)
    {
        MutexService::GetMutexService()->DestroyMutex(m_piLock);
    }
}

PUBLIC
IThread* ThreadService::Create(IN const AString& strName, IN IMS_SINT32 nSlotId)
{
    ImsThread* pThread = PlatformFactory::CreateThread();

    IMS_ASSERT(pThread != IMS_NULL);

    if (pThread == IMS_NULL)
    {
        return IMS_NULL;
    }

    // Create a thread and manage it in thread pool.
    LockThreadPool();

    if (!pThread->CreateEx(strName, nSlotId))
    {
        delete pThread;

        UnlockThreadPool();
        return IMS_NULL;
    }

    m_objThreads.Append(pThread);

    UnlockThreadPool();

    return pThread;
}

PUBLIC
IThread* ThreadService::CreateEx(IN const AString& strName, IN IMS_SINT32 nSlotId)
{
    ImsThread* pThread = PlatformFactory::CreateThreadEx();

    IMS_ASSERT(pThread != IMS_NULL);

    if (pThread == IMS_NULL)
    {
        return IMS_NULL;
    }

    // Create a thread and manage it in thread pool.
    LockThreadPool();

    if (!pThread->CreateEx(strName, nSlotId))
    {
        delete pThread;

        UnlockThreadPool();
        return IMS_NULL;
    }

    m_objThreads.Append(pThread);

    UnlockThreadPool();

    return pThread;
}

PUBLIC
void ThreadService::Destroy(IN IThread*& piThread)
{
    ImsThread* pThread = DYNAMIC_CAST(ImsThread*, piThread);

    if (pThread == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bThreadFound = IMS_FALSE;

    LockThreadPool();

    for (IMS_UINT32 i = 0; i < m_objThreads.GetSize(); ++i)
    {
        IThread* piExThread = m_objThreads.GetAt(i);

        if (piExThread == piThread)
        {
            m_objThreads.RemoveAt(i);
            bThreadFound = IMS_TRUE;
            break;
        }
    }

    UnlockThreadPool();

    if (bThreadFound)
    {
        delete pThread;
        piThread = IMS_NULL;
    }
}

PUBLIC
IMS_BOOL ThreadService::Contains(IN const IThread* piThread) const
{
    if (piThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objThreads.GetSize(); ++i)
    {
        IThread* piExThread = m_objThreads.GetAt(i);

        IMS_ASSERT(piExThread != IMS_NULL);

        if (piExThread != IMS_NULL)
        {
            if (piExThread->Equals(piThread))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL ThreadService::ContainsLocked(IN const IThread* piThread) const
{
    if (piThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    LockThreadPool();

    IMS_BOOL bResult = Contains(piThread);

    UnlockThreadPool();

    return bResult;
}

PUBLIC
IThread* ThreadService::GetCurrentThread() const
{
    IMS_ULONG nCurrentThreadId = 0;

    LockThreadPool();

    // According to the platform specific API, we will find the current thread ...
    nCurrentThreadId = PlatformFactory::GetCurrentThreadId();

    if (nCurrentThreadId == 0)
    {
        UnlockThreadPool();
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objThreads.GetSize(); ++i)
    {
        ImsThread* pThread = DYNAMIC_CAST(ImsThread*, m_objThreads.GetAt(i));

        if (pThread != IMS_NULL)
        {
            if (nCurrentThreadId == pThread->GetThreadId())
            {
                UnlockThreadPool();
                return pThread;
            }
        }
    }

    UnlockThreadPool();

    return IMS_NULL;
}

PUBLIC
IThread* ThreadService::GetThread(IN const AString& strName) const
{
    for (IMS_UINT32 i = 0; i < m_objThreads.GetSize(); ++i)
    {
        IThread* piExThread = m_objThreads.GetAt(i);

        IMS_ASSERT(piExThread != IMS_NULL);

        if (piExThread != IMS_NULL)
        {
            if (piExThread->GetName().Equals(strName))
            {
                return piExThread;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
IThread* ThreadService::GetThreadLocked(IN const AString& strName) const
{
    LockThreadPool();

    IThread* piThread = GetThread(strName);

    UnlockThreadPool();

    return piThread;
}

PUBLIC GLOBAL ThreadService* ThreadService::GetThreadService()
{
    static ThreadService* s_pThreadService = IMS_NULL;

    if (s_pThreadService == IMS_NULL)
    {
        s_pThreadService = new ThreadService();
    }

    return s_pThreadService;
}

PUBLIC GLOBAL IMS_SINT32 ThreadService::GetCurrentSlotId(
        IN IMS_SINT32 nDefaultSlotId /* = IMS_SLOT_ANY*/)
{
    IThread* piThread = GetThreadService()->GetCurrentThread();
    return (piThread != IMS_NULL) ? piThread->GetSlotId() : nDefaultSlotId;
}

PRIVATE
void ThreadService::LockThreadPool() const
{
    m_piLock->Lock();
}

PRIVATE
void ThreadService::UnlockThreadPool() const
{
    m_piLock->Unlock();
}
