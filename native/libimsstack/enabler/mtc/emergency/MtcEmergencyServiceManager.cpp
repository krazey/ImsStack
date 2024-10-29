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
#include "emergency/EmergencyServiceController.h"
#include "emergency/MtcEmergencyServiceManager.h"
#include "emergency/NormalServiceController.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_pController(nullptr),
        m_objContext(objContext)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC VIRTUAL void MtcEmergencyServiceManager::StartOpen(IN EmergencyCallRoutingPdn ePdn)
{
    IMS_TRACE_D("StartOpen PDN=[%d]", ePdn, 0, 0);

    EmergencyCallRoutingPdn eRefinedPdn =
            (ePdn == EmergencyCallRoutingPdn::UNKNOWN ? EmergencyCallRoutingPdn::EMERGENCY : ePdn);
    if (!m_pController || m_pController->GetRoutingPdnType() != eRefinedPdn)
    {
        m_pController.reset(CreateController(eRefinedPdn));
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
}

PRIVATE IEmergencyServiceController* MtcEmergencyServiceManager::CreateController(
        IN EmergencyCallRoutingPdn ePdn)
{
    if (ePdn == EmergencyCallRoutingPdn::NORMAL)
    {
        return new NormalServiceController(*this, m_objContext);
    }
    return new EmergencyServiceController(*this, m_objContext);
}
