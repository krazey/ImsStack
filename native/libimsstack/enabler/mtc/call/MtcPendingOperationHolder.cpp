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
    for (IMS_UINT32 nIndex = 0; nIndex < m_objPendingOperations.GetSize(); nIndex++)
    {
        PopPendingOperation();
    }
}

PUBLIC IMS_BOOL MtcPendingOperationHolder::IsNeedToAdd(
        IN IMtcCall::State eState, IMtcCallContext& objCallContext) const
{
    if (eState == IMtcCall::State::UPDATING)
    {
        IMS_TRACE_D("IsNeedToAdd :: updating", 0, 0, 0);
        return IMS_TRUE;
    }

    IMtcSession* piMtcSession = objCallContext.GetSession();
    if (piMtcSession && piMtcSession->GetISession().IsSessionRefreshInProgress())
    {
        IMS_TRACE_D("IsNeedToAdd :: refreshing", 0, 0, 0);
        return IMS_TRUE;
    }

    if (eState == IMtcCall::State::OUTGOING || eState == IMtcCall::State::ALERTING ||
            eState == IMtcCall::State::INCOMING)
    {
        IMS_TRACE_D(
                "IsNeedToAdd :: ongoing setup, state[%d]", static_cast<IMS_SINT32>(eState), 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MtcPendingOperationHolder::HasPendingOperation() const
{
    return m_objPendingOperations.GetSize() > 0;
}

PUBLIC void MtcPendingOperationHolder::PushPendingOperation(
        IN std::function<IMtcCall::State(IMtcCallState*)> objPendingOperation)
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
    return objPendingOperation;
}
