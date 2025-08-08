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
#include "IFrameworkThreadListener.h"
#include "ImsFramework.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"

PUBLIC
ImsFramework::ImsFramework() :
        ImsAppThread(),
        m_piLock(IMS_NULL),
        m_objListeners(ImsList<IFrameworkThreadListener*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC VIRTUAL ImsFramework::~ImsFramework()
{
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
void ImsFramework::AddListener(IN IFrameworkThreadListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IFrameworkThreadListener* piThreadListener = m_objListeners.GetAt(i);

        if (piThreadListener == piListener)
        {
            // Listener is already registered.
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void ImsFramework::RemoveListener(IN const IFrameworkThreadListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        const IFrameworkThreadListener* piThreadListener = m_objListeners.GetAt(i);

        if (piThreadListener == piListener)
        {
            m_objListeners.RemoveAt(i);
            break;
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL ImsFramework::OnStart(IN ImsMessage& objMsg)
{
    ImsAppThread::OnStart(objMsg);

    NotifyThreadStarted();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL ImsFramework::OnTerminate(IN ImsMessage& objMsg)
{
    ImsAppThread::OnTerminate(objMsg);

    NotifyThreadTerminated();

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL ImsFramework::OnMessage(IN ImsMessage& objMsg)
{
    ImsAppThread::OnMessage(objMsg);

    return IMS_TRUE;
}

PRIVATE
void ImsFramework::NotifyThreadStarted()
{
    ImsList<IFrameworkThreadListener*> objThreadListeners;

    {
        LockGuard objLock(m_piLock);
        objThreadListeners = m_objListeners;
    }

    for (IMS_UINT32 i = 0; i < objThreadListeners.GetSize(); ++i)
    {
        IFrameworkThreadListener* piThreadListener = objThreadListeners.GetAt(i);

        if (piThreadListener != IMS_NULL)
        {
            piThreadListener->FrameworkThread_OnStarted();
        }
    }
}

PRIVATE
void ImsFramework::NotifyThreadTerminated()
{
    ImsList<IFrameworkThreadListener*> objThreadListeners;

    {
        LockGuard objLock(m_piLock);
        objThreadListeners = m_objListeners;
    }

    for (IMS_UINT32 i = 0; i < objThreadListeners.GetSize(); ++i)
    {
        IFrameworkThreadListener* piThreadListener = objThreadListeners.GetAt(i);

        if (piThreadListener != IMS_NULL)
        {
            piThreadListener->FrameworkThread_OnTerminated();
        }
    }
}
