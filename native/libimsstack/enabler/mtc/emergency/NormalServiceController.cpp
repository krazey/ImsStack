/*
 * Copyright (C) 2023 The Android Open Source Project
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
 * See the License for the specific language governing permissions andˆ
 * limitations under the License.
 */

#include "IJniMtcServiceThread.h"
#include "IMtcContext.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "emergency/NormalServiceController.h"
#include "helper/ICallStateProxy.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
NormalServiceController::NormalServiceController(
        IN IMtcEmergencyServiceManager& objServiceManager, IN IMtcContext& objContext) :
        m_objServiceManager(objServiceManager),
        m_objContext(objContext)
{
    IMS_TRACE_I("+NormalServiceController", 0, 0, 0);
}

PUBLIC VIRTUAL NormalServiceController::~NormalServiceController()
{
    IMS_TRACE_I("~NormalServiceController", 0, 0, 0);
    RemoveListeners();
}

PUBLIC VIRTUAL void NormalServiceController::Start()
{
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (pNormalService && pNormalService->GetStatus() == ServiceStatus::SERVICE_ACTIVE)
    {
        AddListeners();
        Notify(EmergencyServiceState::OPENED);
    }
    else
    {
        Notify(EmergencyServiceState::UNAVAILABLE);
        Finish();
    }
}

PUBLIC VIRTUAL void NormalServiceController::OnCallStateChanged(IN CallKey nCallKey,
        IN IMtcCall::State eState, IN Type /* eType */, IN IMS_BOOL /* bEmergency */,
        IN IMS_SINT32 /* nReason */)
{
    IMtcCall* piCall = m_objContext.GetCallManager().GetCallByCallKey(nCallKey);
    if (piCall->GetCallContext().GetCallInfo().eEmergencyType == EmergencyType::NONE)
    {
        return;
    }

    if (eState == State::TERMINATING)
    {
        Notify(EmergencyServiceState::IDLE);
        Finish();
    }
}

PRIVATE void NormalServiceController::AddListeners()
{
    m_objContext.GetCallStateProxy().AddListener(this);
}

PRIVATE void NormalServiceController::RemoveListeners()
{
    m_objContext.GetCallStateProxy().RemoveListener(this);
}

PRIVATE
void NormalServiceController::Notify(IN EmergencyServiceState eState, IN IMS_SINT32 eReason) const
{
    IMS_TRACE_D("Notify :: state=%d, reason=%d", eState, eReason, 0);

    IMtcService* pService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    IJniMtcServiceThread* pThread = pService ? pService->GetJniServiceThread() : IMS_NULL;
    if (pThread == IMS_NULL)
    {
        return;
    }

    pThread->OnEmergencyServiceChanged(eState, eReason, ServiceType::NORMAL);
}

PRIVATE void NormalServiceController::Finish()
{
    m_objContext.GetAsyncRunner(
            [&]()
            {
                m_objServiceManager.StopOpen(IMS_FALSE);
            });
}
