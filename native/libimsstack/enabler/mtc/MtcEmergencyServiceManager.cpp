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

#include "helper/IMtcAosConnector.h"
#include "ImsAosParameter.h"
#include "MtcEmergencyServiceManager.h"
#include "ServiceTrace.h"
#include "JniMtcServiceThread.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_eStatus(IuMtcService::EmergencyServiceStatus::IDLE)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);

    m_objContext.GetSlotId(); // TODO
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC
void MtcEmergencyServiceManager::OpenEmergencyService(IN JniMtcServiceThread* pServiceThread)
{
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(ImsAosControl::REGISTER_START);
    SetStatus(IuMtcService::EmergencyServiceStatus::OPENING, pServiceThread);
}

PUBLIC
void MtcEmergencyServiceManager::ProcessServiceStatus(IN ServiceStatus eStatus,
        IN JniMtcServiceThread* pServiceThread)
{
    if (eStatus == ServiceStatus::SERVICE_ACTIVE)
    {
        SetStatus(IuMtcService::EmergencyServiceStatus::OPENED, pServiceThread);
    }
    else
    {
        SetStatus(IuMtcService::EmergencyServiceStatus::IDLE, pServiceThread);
    }
}

PUBLIC
void MtcEmergencyServiceManager::SetStatus(IN IuMtcService::EmergencyServiceStatus eStatus,
        IN JniMtcServiceThread* pServiceThread)
{
    if (m_eStatus == eStatus)
    {
        return;
    }

    m_eStatus = eStatus;

    if (pServiceThread == IMS_NULL)
    {
        return;
    }

    pServiceThread->OnEmergencyServiceChanged(static_cast<IMS_SINT32>(eStatus), -1);
}
