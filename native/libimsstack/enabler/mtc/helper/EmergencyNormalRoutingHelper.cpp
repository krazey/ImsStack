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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IJniMtcServiceThread.h"
#include "IMtcCallStateListener.h"
#include "IMtcContext.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/EmergencyNormalRoutingHelper.h"
#include "helper/ICallStateProxy.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EmergencyNormalRoutingHelper::EmergencyNormalRoutingHelper(
        IN IMtcContext& objContext, IN IEmergencyNormalRoutingHelperListener& objListener) :
        m_objContext(objContext),
        m_objListener(objListener)
{
    IMS_TRACE_I("+EmergencyNormalRoutingHelper", 0, 0, 0);
    m_objContext.GetCallStateProxy().AddListener(this);
}

PUBLIC EmergencyNormalRoutingHelper::~EmergencyNormalRoutingHelper()
{
    IMS_TRACE_I("~EmergencyNormalRoutingHelper", 0, 0, 0);
    m_objContext.GetCallStateProxy().RemoveListener(this);
}

PUBLIC VIRTUAL void EmergencyNormalRoutingHelper::OnCallStateChanged(IN CallKey /*nCallKey*/,
        IN State eState, IN Type /*eType*/, IN IMS_BOOL bEmergency, IN IMS_SINT32 /*nReason*/)
{
    if (!bEmergency)
    {
        return;
    }

    if (eState == State::TERMINATING)
    {
        IMS_TRACE_D("OnCallStateChanged to TERMINATING", 0, 0, 0);
        NotifyServiceChanged(EmergencyServiceState::IDLE);
        m_objListener.OnNormalRoutingClosed();
    }
}

PUBLIC VIRTUAL void EmergencyNormalRoutingHelper::OnTotalCallStateChanged(IN State eState)
{
    if (eState == State::TERMINATING)
    {
        IMS_TRACE_D("OnTotalCallStateChanged exeptional case. E-Call wasn't terminated", 0, 0, 0);
        NotifyServiceChanged(EmergencyServiceState::IDLE);
        m_objListener.OnNormalRoutingClosed();
    }
}

PUBLIC
void EmergencyNormalRoutingHelper::HandleEmergencyCall()
{
    IMS_TRACE_D("HandleEmergencyCall", 0, 0, 0);
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (!pNormalService)
    {
        return;
    }

    EmergencyServiceState eState = pNormalService->GetStatus() == ServiceStatus::SERVICE_ACTIVE
            ? EmergencyServiceState::OPENED
            : EmergencyServiceState::UNAVAILABLE;
    NotifyServiceChanged(eState);
}

PRIVATE
void EmergencyNormalRoutingHelper::NotifyServiceChanged(IN EmergencyServiceState eState)
{
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (!pNormalService)
    {
        return;
    }

    IJniMtcServiceThread* pThread = pNormalService->GetJniServiceThread();
    if (pThread)
    {
        pThread->OnEmergencyServiceChanged(
                static_cast<IMS_SINT32>(eState), -1, static_cast<IMS_SINT32>(ServiceType::NORMAL));
    }
}
