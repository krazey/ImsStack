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
#include "BaseThread.h"
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
BaseThread::BaseThread() :
        m_strName(AString::ConstNull()),
        m_piThread(IMS_NULL)
{
}

PUBLIC VIRTUAL BaseThread::~BaseThread()
{
    Terminate();
}

PUBLIC
IMS_BOOL BaseThread::Start(IN const AString& strName, IN IMS_SINT32 nSlotId)
{
    if (m_piThread != IMS_NULL)
    {
        IMS_TRACE_I("Start :: Thread(%s) is already running ...", strName.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    m_strName = strName;

    m_piThread = ThreadService::GetThreadService()->CreateThread(m_strName, nSlotId);

    if (m_piThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "Start :: Creating a thread(%s) failed ...", m_strName.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_piThread->SetRunnable(this);

    if (!m_piThread->Activate())
    {
        IMS_TRACE_E(0, "Start :: Starting a thread(%s) failed ...", m_strName.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void BaseThread::Terminate()
{
    if (m_piThread == IMS_NULL)
    {
        return;
    }

    // Method call will not be returned util the current thread is exited in platform layer.
    m_piThread->Deactivate();

    ThreadService::GetThreadService()->DestroyThread(m_piThread);
    m_piThread = IMS_NULL;
}

PROTECTED VIRTUAL IMS_BOOL BaseThread::Runnable_Run(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case IMS_MSG_START:
            if (!Initialize())
            {
                return IMS_FALSE;
            }

            return OnStart(objMsg);
        case IMS_MSG_TERMINATE:
            OnTerminate(objMsg);
            Uninitialize();
            return IMS_TRUE;
        default:
            break;
    }

    if (!IsThreadMessage(objMsg))
    {
        IMS_TRACE_D("Message(%d) is not for thread(%s)", objMsg.GetName(), GetName().GetStr(), 0);
        return IMS_FALSE;
    }

    return OnMessage(objMsg);
}

PROTECTED
IMS_BOOL BaseThread::IsThreadMessage(IN ImsMessage& objMsg) const
{
    const IMS_CHAR* pszTargetName = objMsg.GetTargetName();

    if ((pszTargetName != IMS_NULL) && !m_strName.Equals(pszTargetName))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
