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
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcUiNotifier.h"
#include "ect/EctController.h"
#include "ect/EctFactory.h"
#include "ect/EctReference.h"
#include "ect/IEctControllerListener.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EctController::EctController(IN IMtcContext& objContext, IN CallKey nCallKey,
        IN IEctControllerListener& objListener, IN EctFactory& objFactory) :
        m_objContext(objContext),
        m_nTransfereeKey(nCallKey),
        m_objListener(objListener),
        m_objFactory(objFactory),
        m_pReference(nullptr),
        m_piTimer(IMS_NULL)
{
    IMS_TRACE_D("+EctController", 0, 0, 0);
}

PUBLIC
EctController::~EctController()
{
    IMS_TRACE_D("~EctController", 0, 0, 0);
    StopTimer();
}

PUBLIC VIRTUAL void EctController::OnReferenceStarted()
{
    IMS_TRACE_D("OnReferenceStarted", 0, 0, 0);
}

PUBLIC VIRTUAL void EctController::OnReferenceStartFailed()
{
    IMS_TRACE_D("OnReferenceStartFailed", 0, 0, 0);
    OnFailed();
}

PUBLIC VIRTUAL void EctController::OnReferenceUpdated(IN IMS_SINT32 nSipFragCode)
{
    IMS_TRACE_D("OnReferenceUpdated", 0, 0, 0);
    if (SipStatusCode::IsFinalSuccess(nSipFragCode))
    {
        OnSuccess();
    }
    else if (SipStatusCode::IsFinalFailure(nSipFragCode))
    {
        OnFailed();
    }
}

PUBLIC VIRTUAL void EctController::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);
    if (m_piTimer != piTimer)
    {
        return;
    }

    OnFailed();
}

PROTECTED
IMtcCall* EctController::GetTransferee() const
{
    IMtcCall* piTransferee = m_objContext.GetCallManager().GetCallByCallKey(m_nTransfereeKey);
    return piTransferee;
}

PROTECTED VIRTUAL void EctController::OnSuccess()
{
    NotifyResult(IMS_SUCCESS, CODE_USER_TERMINATED);
    TerminateTransfereeCall();
    StopTimer();
    m_objListener.OnEctCompleted();
}

PROTECTED VIRTUAL void EctController::OnFailed()
{
    // TODO: Recover()?
    NotifyResult(IMS_FAILURE, CODE_USER_TERMINATED);
    StopTimer();
    m_objListener.OnEctCompleted();
}

PROTECTED
void EctController::NotifyResult(
        IN IMS_RESULT nResult, IN IMS_SINT32 nReason /* = CODE_NONE*/) const
{
    IMS_TRACE_D("NotifyResult [%d]", nResult, 0, 0);
    // TODO: is reason meaningful? what kind of reason to be used for ECT failure?
    IMtcUiNotifier& objNotifier = GetTransferee()->GetCallContext().GetUiNotifier();
    objNotifier.SendEctCompleted(nResult, CallReasonInfo(nReason));
}

PROTECTED
void EctController::CreateReference()
{
    IMS_TRACE_I("CreateReference", 0, 0, 0);

    m_pReference = m_objFactory.CreateReference(m_objContext, m_nTransfereeKey, *this);
}

PROTECTED
void EctController::TerminateTransfereeCall() const
{
    IMS_TRACE_I("TerminateTransfereeCall", 0, 0, 0);
    GetTransferee()->Terminate(CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_ECT));
}

PROTECTED
void EctController::StartTimer()
{
    m_piTimer = TimerService::GetTimerService()->CreateTimer();
    m_piTimer->SetTimer(TIME_WAIT_OPERATION_COMPLETE, this);
}

PROTECTED
void EctController::StopTimer()
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}
