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
        m_pServiceThread(IMS_NULL),
        m_eState(IuMtcService::EmergencyServiceState::IDLE)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);

    m_objContext.GetSlotId(); // TODO
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
}

PUBLIC
void MtcEmergencyServiceManager::OpenEmergencyService()
{
    IMS_TRACE_D("OpenEmergencyService", 0, 0, 0);

    if (m_eState == IuMtcService::EmergencyServiceState::OPENED ||
            m_eState == IuMtcService::EmergencyServiceState::IN_CALL)
    {
        SetState(IuMtcService::EmergencyServiceState::OPENED, IMS_TRUE);
        return;
    }

    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(ImsAosControl::REGISTER_START);
    SetState(IuMtcService::EmergencyServiceState::OPENING);
}

PUBLIC
void MtcEmergencyServiceManager::HandleServiceStatus(IN ServiceStatus eStatus)
{
    IMS_TRACE_D("HandleServiceStatus :: %d", eStatus, 0, 0);

    switch (eStatus)
    {
        case ServiceStatus::SERVICE_IDLE:
            HandleServiceIdle();
            break;
        case ServiceStatus::SERVICE_ACTIVE:
            HandleServiceActive();
            break;
        case ServiceStatus::SERVICE_SUSPENDED:
            HandleServiceSuspended();
            break;
        default:
            break;
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceIdle()
{
    IMS_TRACE_D("HandleServiceIdle", 0, 0, 0);

    if (m_eState == IuMtcService::EmergencyServiceState::OPENING)
    {
        SetState(IuMtcService::EmergencyServiceState::UNAVAILABLE);
    }
    else
    {
        SetState(IuMtcService::EmergencyServiceState::IDLE);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceActive()
{
    IMS_TRACE_D("HandleServiceActive", 0, 0, 0);

    if (m_eState == IuMtcService::EmergencyServiceState::OPENING)
    {
        SetState(IuMtcService::EmergencyServiceState::OPENED);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceSuspended()
{
    IMS_TRACE_D("HandleServiceSuspended", 0, 0, 0);
}

PRIVATE
void MtcEmergencyServiceManager::SetState(IN IuMtcService::EmergencyServiceState eState,
            IN IMS_BOOL bForceNotify /* = IMS_FALSE */)
{
    IMS_TRACE_D("SetState :: [%d] -> [%d]", m_eState, eState, 0);

    if ((m_eState == eState) && !bForceNotify)
    {
        return;
    }

    m_eState = eState;

    NotifyEmergencyServiceChanged(-1);
}

PRIVATE
void MtcEmergencyServiceManager::NotifyEmergencyServiceChanged(IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("NotifyEmergencyServiceChanged :: %d, %d", m_eState, eReason, 0);

    if (m_pServiceThread == IMS_NULL)
    {
        return;
    }

    ServiceType eServiceType = ServiceType::EMERGENCY;

    m_pServiceThread->OnEmergencyServiceChanged(static_cast<IMS_SINT32>(m_eState), -1,
            static_cast<IMS_SINT32>(eServiceType));
}
