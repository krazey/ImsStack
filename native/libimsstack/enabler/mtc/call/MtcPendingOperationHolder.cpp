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

#include "ISession.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/state/IMtcCallState.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcPendingOperationHolder::MtcPendingOperationHolder()
{
    IMS_TRACE_D("+MtcPendingOperationHolder", 0, 0, 0);
}

PUBLIC MtcPendingOperationHolder::~MtcPendingOperationHolder()
{
    IMS_TRACE_D("~MtcPendingOperationHolder ", 0, 0, 0);
    while (!m_objPendingOperations.IsEmpty())
    {
        PopPendingOperation();
    }
}

PUBLIC IMS_BOOL MtcPendingOperationHolder::HasPendingOperation() const
{
    return m_objPendingOperations.GetSize() > 0;
}

PUBLIC void MtcPendingOperationHolder::PushPendingOperation(
        IN const std::function<IMtcCall::State(IMtcCallState*)>& objPendingOperation)
{
    m_objPendingOperations.Append(objPendingOperation);
    IMS_TRACE_D("PushPendingOperation :: added", 0, 0, 0);
}

PUBLIC std::function<IMtcCall::State(IMtcCallState*)>
MtcPendingOperationHolder::PopPendingOperation()
{
    std::function<IMtcCall::State(IMtcCallState*)> objPendingOperation =
            m_objPendingOperations.GetAt(0);
    m_objPendingOperations.RemoveAt(0);
    IMS_TRACE_D("PopPendingOperation :: remain[%d]", m_objPendingOperations.GetSize(), 0, 0);
    return objPendingOperation;
}
