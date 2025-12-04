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
#include "EnablerUtils.h"
#include "IThread.h"
#include "ImsMessage.h"
#include "ImsProcess.h"
#include "ServiceTrace.h"
#include "helper/OperationAsyncRunner.h"
#include <functional>
#include <utility>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
OperationAsyncRunner::OperationAsyncRunner(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_bOperationStarted(IMS_FALSE),
        m_objOperation(IMS_NULL),
        m_objRemoveCallback(IMS_NULL)
{
    IMS_TRACE_D("+OperationAsyncRunner", 0, 0, 0);
}

PUBLIC
OperationAsyncRunner::~OperationAsyncRunner()
{
    BaseThread* pThread =
            ImsProcess::GetInstance()->GetThread(EnablerUtils::GetEnablerThreadName(m_nSlotId));
    if (pThread != IMS_NULL)
    {
        IThread* piThread = pThread->GetThread();
        if (piThread != IMS_NULL)
        {
            IMS_SINT32 nRemovedSize = piThread->RemoveMessages(this);
            IMS_TRACE_D("~OperationAsyncRunner removed message size[%d]", nRemovedSize, 0, 0);
        }
    }
    m_objOperation = {};
}

PUBLIC
void OperationAsyncRunner::SetOperation(
        IN std::function<void()> objOperation, std::function<void()> objRemoveCallback)
{
    IMS_ASSERT(objOperation);
    IMS_ASSERT(objRemoveCallback);
    m_objOperation = std::move(objOperation);
    m_objRemoveCallback = std::move(objRemoveCallback);

    ImsMessage objMsg(0, 0, 0, this);
    BaseThread* pThread =
            ImsProcess::GetInstance()->GetThread(EnablerUtils::GetEnablerThreadName(m_nSlotId));
    if (pThread != IMS_NULL)
    {
        IThread* piThread = pThread->GetThread();
        if (piThread != IMS_NULL)
        {
            piThread->PostMessageI(objMsg);
            return;
        }
    }

    IMS_TRACE_E(0, "Thread is null", 0, 0, 0);
    m_objRemoveCallback();
}

PUBLIC VIRTUAL void OperationAsyncRunner::MessageCallback_OnMessage(IN ImsMessage& /*objMsg*/)
{
    IMS_TRACE_D("MessageCallback_OnMessage", 0, 0, 0);
    m_bOperationStarted = IMS_TRUE;
    m_objOperation();
    m_objRemoveCallback();
}
