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

#include "CallReasonInfo.h"
#include "IMtcContext.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "ect/ConsultativeTransferController.h"
#include "ect/EctReference.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConsultativeTransferController::ConsultativeTransferController(IN IMtcContext& objContext,
        IN CallKey nCallKey, IN IEctControllerListener& objListener, IN EctFactory& objFactory) :
        EctController(objContext, nCallKey, objListener, objFactory),
        m_nTransferTargetKey(IMtcCall::CALL_KEY_INVALID)
{
    IMS_TRACE_D("+ConsultativeTransferController", 0, 0, 0);
}

PUBLIC
ConsultativeTransferController::~ConsultativeTransferController()
{
    IMS_TRACE_D("~ConsultativeTransferController", 0, 0, 0);
}

PUBLIC VIRTUAL void ConsultativeTransferController::Transfer()
{
    IMS_TRACE_D("Transfer - consultative", 0, 0, 0);

    if (IsValid() == IMS_FALSE)
    {
        OnFailed();
        return;
    }

    CreateReference();
    FindTransferTarget();
    if (m_pReference->SendInvite(m_nTransferTargetKey) == IMS_FAILURE)
    {
        OnFailed();
        return;
    }

    StartTimer();
}

PROTECTED VIRTUAL void ConsultativeTransferController::OnCompleted()
{
    // just in case EctManager directly delete controller once OnCompleted().
    TerminateTransferTargetCall();
    EctController::OnCompleted();
}

PRIVATE
IMS_BOOL ConsultativeTransferController::IsValid() const
{
    if (m_objContext.GetCallManager().GetCalls().GetSize() == 2)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void ConsultativeTransferController::FindTransferTarget()
{
    ImsList<IMtcCall*> objCalls = m_objContext.GetCallManager().GetCalls();
    for (IMS_UINT32 i = 0; i < objCalls.GetSize(); i++)
    {
        CallKey nTempKey = objCalls.GetAt(i)->GetKey();
        if (nTempKey != m_nTransfereeKey)
        {
            m_nTransferTargetKey = nTempKey;
            return;
        }
    }
}

PRIVATE
void ConsultativeTransferController::TerminateTransferTargetCall()
{
    IMS_TRACE_I("TerminateTransferTargetCall", 0, 0, 0);

    // TODO: this needs to be checked using the carrier network.
    // according to 3GPP 24.629, the TransferTarget sends BYE
    // so Transferror should wait.
    // It could be 'not safe' to send BYE to TransferTarget.
    m_objContext.GetCallManager()
            .GetCallByCallKey(m_nTransferTargetKey)
            ->Terminate(CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_ECT));
}
