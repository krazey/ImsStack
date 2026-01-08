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

#include "IMtcContext.h"
#include "ServiceTrace.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/EmergencyServiceController.h"
#include "emergency/MtcEmergencyServiceManager.h"
#include "emergency/NormalServiceController.h"
#include "helper/MtcLocationRefresher.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_pController(nullptr),
        m_objContext(objContext),
        m_bLocationUpdateRequested(IMS_FALSE)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC VIRTUAL void MtcEmergencyServiceManager::StartOpen(IN ServiceType eServiceType)
{
    IMS_TRACE_D("StartOpen Service=[%d]", eServiceType, 0, 0);

    RequestLocationUpdateIfRequired();

    if (!m_pController || m_pController->GetServiceType() != eServiceType)
    {
        m_pController.reset(CreateController(eServiceType));
    }

    if (m_pController)
    {
        m_pController->Start();
    }
}

PUBLIC VIRTUAL void MtcEmergencyServiceManager::StopOpen(IN IMS_BOOL bClose)
{
    IMS_TRACE_D("StopOpen", 0, 0, 0);

    if (bClose && m_pController)
    {
        m_pController->Close();
    }
    m_pController.reset();
    m_bLocationUpdateRequested = IMS_FALSE;
}

PRIVATE IEmergencyServiceController* MtcEmergencyServiceManager::CreateController(
        IN ServiceType eServiceType)
{
    if (eServiceType == ServiceType::NORMAL)
    {
        return new NormalServiceController(*this, m_objContext);
    }
    return new EmergencyServiceController(*this, m_objContext);
}

PRIVATE void MtcEmergencyServiceManager::RequestLocationUpdateIfRequired()
{
    if (m_bLocationUpdateRequested)
    {
        return;
    }

    IMS_SINT32 nWaitTime = m_objContext.GetConfigurationProxy().GetInt(
            ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT);
    if (nWaitTime > 0)
    {
        m_bLocationUpdateRequested = IMS_TRUE;
        m_objContext.GetLocationRefresher().RequestUpdate(nWaitTime);
    }
}
