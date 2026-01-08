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
#include "ImsFramework.h"
#include "ImsProcess.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

#define IMS_FRAMEWORK_THREAD "Framework"

class ImsThreadMap
{
public:
    inline ImsThreadMap(IN const AString& strName, IN BaseThread* pThread) :
            m_strName(strName),
            m_pThread(pThread)
    {
    }

    inline ~ImsThreadMap() {}

    ImsThreadMap(IN const ImsThreadMap&) = delete;
    ImsThreadMap& operator=(IN const ImsThreadMap&) = delete;

public:
    AString m_strName;
    BaseThread* m_pThread;
};

PRIVATE
ImsProcess::ImsProcess() :
        m_strFrameworkThreadName(IMS_FRAMEWORK_THREAD),
        m_piLock(IMS_NULL),
        m_objThreads(ImsList<ImsThreadMap*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
ImsProcess::~ImsProcess()
{
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC GLOBAL ImsProcess* ImsProcess::GetInstance()
{
    static ImsProcess* s_pImsProcess = IMS_NULL;

    if (s_pImsProcess == IMS_NULL)
    {
        s_pImsProcess = new ImsProcess();
    }

    return s_pImsProcess;
}

PUBLIC
const AString& ImsProcess::GetFrameworkThreadName() const
{
    return m_strFrameworkThreadName;
}

PUBLIC
IMS_BOOL ImsProcess::Initialize()
{
    const AString& strFwkThreadName = GetFrameworkThreadName();

    if (GetApplicationThread(strFwkThreadName) != IMS_NULL)
    {
        return IMS_TRUE;
    }

    // Start a default thread
    BaseThread* pThread = new ImsFramework();

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strFwkThreadName, pThread);

    pThread->Start(strFwkThreadName, IMS_SLOT_0);

    return IMS_TRUE;
}

PUBLIC
void ImsProcess::Uninitialize()
{
    IMS_TRACE_I("Uninitialize :: Thread count (%d)", m_objThreads.GetSize(), 0, 0);

    LockGuard objLock(m_piLock);

    while (!m_objThreads.IsEmpty())
    {
        ImsThreadMap* pThreadMap = m_objThreads.GetAt(0);
        if (pThreadMap->m_pThread != IMS_NULL)
        {
            pThreadMap->m_pThread->Terminate();
            delete pThreadMap->m_pThread;
        }
        delete pThreadMap;
        m_objThreads.RemoveAt(0);
    }
}

PUBLIC
IMS_BOOL ImsProcess::LoadThread(
        IN const AString& strThreadName, IN Thread_Entry pfnThreadEntry, IN IMS_SINT32 nSlotId)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    BaseThread* pThread = pfnThreadEntry();

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ImsProcess::LoadThreadWithParam(IN const AString& strThreadName,
        IN Thread_EntryEx pfnThreadEntryEx, IN void* pvParam, IN IMS_SINT32 nSlotId)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    BaseThread* pThread = pfnThreadEntryEx(pvParam);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
void ImsProcess::UnloadThread(IN const AString& strThreadName)
{
    BaseThread* pThread = GetThread(strThreadName);

    if (pThread != IMS_NULL)
    {
        pThread->Terminate();

        DetachThread(strThreadName);
        delete pThread;
    }
}

PUBLIC
BaseThread* ImsProcess::GetThread(IN const AString& strThreadName)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objThreads.GetSize(); ++i)
    {
        ImsThreadMap* pThreadMap = m_objThreads.GetAt(i);

        if (pThreadMap->m_strName.Equals(strThreadName))
        {
            return pThreadMap->m_pThread;
        }
    }

    IMS_TRACE_I("GetThread :: Thread(%s) not found", strThreadName.GetStr(), 0, 0);
    return IMS_NULL;
}

PUBLIC
IMS_BOOL ImsProcess::LoadAppThread(
        IN const AString& strThreadName, IN AppThread_Entry pfnThreadEntry, IN IMS_SINT32 nSlotId)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsAppThread* pThread = pfnThreadEntry();

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ImsProcess::LoadAppThreadWithParam(IN const AString& strThreadName,
        IN AppThread_EntryEx pfnThreadEntryEx, IN void* pvParam, IN IMS_SINT32 nSlotId)
{
    if (GetThread(strThreadName) != IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsAppThread* pThread = pfnThreadEntryEx(pvParam);

    if (pThread == IMS_NULL)
    {
        return IMS_FALSE;
    }

    (void)AttachThread(strThreadName, pThread);

    pThread->Start(strThreadName, nSlotId);

    return IMS_TRUE;
}

PUBLIC
void ImsProcess::UnloadAppThread(IN const AString& strThreadName)
{
    UnloadThread(strThreadName);
}

PUBLIC
ImsAppThread* ImsProcess::GetApplicationThread(IN const AString& strThreadName)
{
    return DYNAMIC_CAST(ImsAppThread*, GetThread(strThreadName));
}

PUBLIC
IImsActivityController* ImsProcess::GetController(IN const AString& strControllerName)
{
    AString strThreadName = GetThreadName(strControllerName);
    ImsAppThread* pAppThread = GetApplicationThread(strThreadName);

    IMS_TRACE_D("GetController :: Controller(%s), Thread(%s)", strControllerName.GetStr(),
            strThreadName.GetStr(), 0);

    if (pAppThread != IMS_NULL)
    {
        return pAppThread->GetActivityManager()->GetController(strControllerName);
    }

    return IMS_NULL;
}

PRIVATE
IMS_BOOL ImsProcess::AttachThread(IN const AString& strName, IN BaseThread* pThread)
{
    if ((pThread == IMS_NULL) || (strName.GetLength() == 0))
    {
        return IMS_FALSE;
    }

    ImsThreadMap* pThreadMap = new ImsThreadMap(strName, pThread);

    LockGuard objLock(m_piLock);

    m_objThreads.Append(pThreadMap);

    IMS_TRACE_I("AttachThread : Thread (%s)", strName.GetStr(), 0, 0);

    return IMS_TRUE;
}

PRIVATE
void ImsProcess::DetachThread(IN const AString& strName)
{
    if (strName.GetLength() == 0)
    {
        return;
    }

    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objThreads.GetSize(); ++i)
    {
        ImsThreadMap* pThreadMap = m_objThreads.GetAt(i);

        if (pThreadMap->m_strName.Equals(strName))
        {
            IMS_TRACE_I("DetachThread :: Thread (%s)", strName.GetStr(), 0, 0);

            delete pThreadMap;
            m_objThreads.RemoveAt(i);
            break;
        }
    }
}

PRIVATE
AString ImsProcess::GetThreadName(IN const AString& strTargetName)
{
    IMS_SINT32 nIndex = strTargetName.GetIndexOf('.');
    return strTargetName.GetSubStr(0, nIndex);
}
