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

#include "IJniMtcServiceThread.h"
#include "IMtcContext.h"
#include "ImsAosParameter.h"
#include "MtcEmergencyServiceManager.h"
#include "ServiceTrace.h"
#include "helper/IMtcAosConnector.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_eState(IuMtcService::EmergencyServiceState::IDLE)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC
void MtcEmergencyServiceManager::OpenEmergencyService()
{
    IMS_TRACE_D("OpenEmergencyService", 0, 0, 0);

    IMS_BOOL bStateChanged = IMS_FALSE;
    if (m_eState == IuMtcService::EmergencyServiceState::OPENED ||
            m_eState == IuMtcService::EmergencyServiceState::IN_CALL)
    {
        SetState(IuMtcService::EmergencyServiceState::OPENED, bStateChanged);
        NotifyEmergencyServiceChanged(-1);
        return;
    }

    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(ImsAosControl::REGISTER_START);
    SetState(IuMtcService::EmergencyServiceState::OPENING, bStateChanged);
    if (bStateChanged)
    {
        NotifyEmergencyServiceChanged(-1);
    }
}

PUBLIC
void MtcEmergencyServiceManager::HandleServiceStatus(IN ServiceStatus eStatus)
{
    IMS_TRACE_D("HandleServiceStatus :: %d", eStatus, 0, 0);

    IMS_BOOL bStateChanged = IMS_FALSE;

    switch (eStatus)
    {
        case ServiceStatus::SERVICE_IDLE:
            HandleServiceIdle(bStateChanged);
            break;
        case ServiceStatus::SERVICE_ACTIVE:
            HandleServiceActive(bStateChanged);
            break;
        default:
            break;
    }

    if (bStateChanged)
    {
        NotifyEmergencyServiceChanged(-1);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceIdle(IN IMS_BOOL& bStateChanged)
{
    IMS_TRACE_D("HandleServiceIdle", 0, 0, 0);

    if (m_eState == IuMtcService::EmergencyServiceState::OPENING)
    {
        SetState(IuMtcService::EmergencyServiceState::UNAVAILABLE, bStateChanged);
    }
    else
    {
        SetState(IuMtcService::EmergencyServiceState::IDLE, bStateChanged);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceActive(IN IMS_BOOL& bStateChanged)
{
    IMS_TRACE_D("HandleServiceActive", 0, 0, 0);

    if (m_eState == IuMtcService::EmergencyServiceState::OPENING)
    {
        SetState(IuMtcService::EmergencyServiceState::OPENED, bStateChanged);
    }
}

PRIVATE
void MtcEmergencyServiceManager::SetState(
        IN IuMtcService::EmergencyServiceState eState, OUT IMS_BOOL& bChanged)
{
    IMS_TRACE_D("SetState :: [%d] -> [%d]", m_eState, eState, 0);

    bChanged = (m_eState != eState);
    m_eState = eState;
}

PRIVATE
void MtcEmergencyServiceManager::NotifyEmergencyServiceChanged(IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("NotifyEmergencyServiceChanged :: state=%d, reason=%d", m_eState, eReason, 0);
    IJniMtcServiceThread* pThread =
            m_objContext.GetServiceByType(ServiceType::NORMAL)->GetJniServiceThread();
    if (pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "IJniMtcServiceThread is null", 0, 0, 0);
        return;
    }

    pThread->OnEmergencyServiceChanged(
            static_cast<IMS_SINT32>(m_eState), -1, static_cast<IMS_SINT32>(ServiceType::EMERGENCY));
}
