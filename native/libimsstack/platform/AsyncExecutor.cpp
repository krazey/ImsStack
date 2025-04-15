/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "ServiceThread.h"

#include "AsyncExecutor.h"

PUBLIC
AsyncExecutor::AsyncExecutor(IN IMS_BOOL bAutoDestroy) :
        m_bAutoDestroy(bAutoDestroy),
        m_piExecutor(IMS_NULL),
        m_piListener(IMS_NULL)
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
}

PUBLIC
AsyncExecutor::AsyncExecutor(AsyncExecutor::IListener* piListener, IN IMS_BOOL bAutoDestroy) :
        m_bAutoDestroy(bAutoDestroy),
        m_piExecutor(IMS_NULL),
        m_piListener(piListener)
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
}

PUBLIC
AsyncExecutor::AsyncExecutor(IN IThread* piOwnerThread, IN IMS_BOOL bAutoDestroy) :
        m_bAutoDestroy(bAutoDestroy),
        m_piOwnerThread(piOwnerThread),
        m_piExecutor(IMS_NULL),
        m_piListener(IMS_NULL)
{
}

PUBLIC
AsyncExecutor::AsyncExecutor(
        IN IThread* piOwnerThread, AsyncExecutor::IListener* piListener, IN IMS_BOOL bAutoDestroy) :
        m_bAutoDestroy(bAutoDestroy),
        m_piOwnerThread(piOwnerThread),
        m_piExecutor(IMS_NULL),
        m_piListener(piListener)
{
}

PUBLIC VIRTUAL AsyncExecutor::~AsyncExecutor()
{
    if (m_piOwnerThread != IMS_NULL)
    {
        m_piOwnerThread->RemoveMessages(this);
    }
}

PUBLIC
void AsyncExecutor::Execute(IN IMS_UINTP nParam1, IN IMS_UINTP nParam2)
{
    ImsMessage objMsg(MSG_EXECUTE, nParam1, nParam2);
    objMsg.SetCallback(this);

    if (m_piOwnerThread != IMS_NULL)
    {
        m_piOwnerThread->PostMessageI(objMsg);
    }
    else
    {
        // Execute the message directly
        // if the caller thread is not created inside ImsStack platform.
        MessageCallback_OnMessage(objMsg);
    }
}

PUBLIC
void AsyncExecutor::Destroy()
{
    ImsMessage objMsg(MSG_DESTROY, 0, 0);
    objMsg.SetCallback(this);

    if (m_piOwnerThread != IMS_NULL)
    {
        m_piOwnerThread->PostMessageI(objMsg);
    }
    else
    {
        // Execute the message directly
        // if the caller thread is not created inside ImsStack platform.
        MessageCallback_OnMessage(objMsg);
    }
}

PROTECTED VIRTUAL void AsyncExecutor::MessageCallback_OnMessage(IN ImsMessage& objMsg)
{
    if (objMsg.GetName() == MSG_DESTROY)
    {
        delete this;
    }
    else if (objMsg.GetName() == MSG_EXECUTE)
    {
        OnExecute(objMsg.nWparam, objMsg.nLparam);

        if (m_piListener != IMS_NULL)
        {
            m_piListener->AsyncExecutor_OnExecuteCompleted(this);
        }

        if (m_bAutoDestroy)
        {
            delete this;
        }
    }
}
