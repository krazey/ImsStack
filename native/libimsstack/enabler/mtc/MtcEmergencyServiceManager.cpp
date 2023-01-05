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
#include "ImsAosReason.h"
#include "MtcEmergencyServiceManager.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_eState(EmergencyServiceState::IDLE)
{
    IMS_TRACE_I("+MtcEmergencyServiceManager", 0, 0, 0);
    IMtcService* pEmergencyService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pEmergencyService)
    {
        pEmergencyService->AddAosStateListener(this);
    }
}

PUBLIC VIRTUAL MtcEmergencyServiceManager::~MtcEmergencyServiceManager()
{
    IMS_TRACE_I("~MtcEmergencyServiceManager", 0, 0, 0);
    IMtcService* pEmergencyService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
    if (pEmergencyService)
    {
        pEmergencyService->RemoveAosStateListener(this);
    }
}

PUBLIC
void MtcEmergencyServiceManager::OpenEmergencyService(IN EmergencyCallRoutingPdn ePdn)
{
    IMS_TRACE_D("OpenEmergencyService PDN=[%d]", ePdn, 0, 0);
    if (ePdn == EmergencyCallRoutingPdn::NORMAL)
    {
        return HandleEmergencyCallOverImsPdn();
    }

    IMS_BOOL bStateChanged = IMS_FALSE;
    if (m_eState == EmergencyServiceState::OPENED || m_eState == EmergencyServiceState::IN_CALL)
    {
        SetState(EmergencyServiceState::OPENED, bStateChanged);
        NotifyEmergencyServiceChanged(-1);
        return;
    }

    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(ImsAosControl::REGISTER_START);
    SetState(EmergencyServiceState::OPENING, bStateChanged);
    if (bStateChanged)
    {
        NotifyEmergencyServiceChanged(-1);
    }
}

PUBLIC VIRTUAL void MtcEmergencyServiceManager::OnAosStateChanged(
        IN IMtcService& /*objMtcService*/, IN MtcAosState eState, IN IMS_UINT32 eAosReason)
{
    IMS_TRACE_D("OnAosStateChanged :: %d", eState, 0, 0);

    IMS_BOOL bStateChanged = IMS_FALSE;

    switch (eState)
    {
        case MtcAosState::DISCONNECTED:
            if (IsRetryOverImsPdnRequired(eAosReason))
            {
                return HandleEmergencyCallOverImsPdn();
            }
            HandleServiceIdle(bStateChanged);
            break;
        case MtcAosState::CONNECTED:
            HandleServiceActive(bStateChanged);
            break;
        default:
            return;
    }

    if (bStateChanged)
    {
        NotifyEmergencyServiceChanged(static_cast<IMS_SINT32>(eAosReason));
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceIdle(IN IMS_BOOL& bStateChanged)
{
    IMS_TRACE_D("HandleServiceIdle", 0, 0, 0);

    if (m_eState == EmergencyServiceState::OPENING)
    {
        SetState(EmergencyServiceState::UNAVAILABLE, bStateChanged);
    }
    else
    {
        SetState(EmergencyServiceState::IDLE, bStateChanged);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceActive(IN IMS_BOOL& bStateChanged)
{
    IMS_TRACE_D("HandleServiceActive", 0, 0, 0);

    if (m_eState == EmergencyServiceState::OPENING)
    {
        SetState(EmergencyServiceState::OPENED, bStateChanged);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleEmergencyCallOverImsPdn()
{
    IMS_TRACE_D("HandleEmergencyCallOverImsPdn", 0, 0, 0);
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (!pNormalService)
    {
        // TODO: change IMtcContext#GetServiceByType to return reference.
        return;
    }

    IJniMtcServiceThread* pThread = pNormalService->GetJniServiceThread();
    if (pThread)
    {
        IMS_SINT32 eState = pNormalService->GetStatus() == ServiceStatus::SERVICE_ACTIVE
                ? static_cast<IMS_SINT32>(EmergencyServiceState::OPENED)
                : static_cast<IMS_SINT32>(EmergencyServiceState::UNAVAILABLE);
        pThread->OnEmergencyServiceChanged(
                eState, -1, static_cast<IMS_SINT32>(ServiceType::NORMAL));
    }
}

PRIVATE
IMS_BOOL MtcEmergencyServiceManager::IsRetryOverImsPdnRequired(IN IMS_SINT32 eAosReason) const
{
    if (m_objContext.GetConfigurationProxy().Is(Feature::RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
    {
        return eAosReason == ImsAosReason::DATA_DISCONNECTED;
    }

    return IMS_FALSE;
}

PRIVATE
void MtcEmergencyServiceManager::SetState(IN EmergencyServiceState eState, OUT IMS_BOOL& bChanged)
{
    IMS_TRACE_D("SetState :: [%d] -> [%d]", m_eState, eState, 0);

    bChanged = (m_eState != eState);
    m_eState = eState;
}

PRIVATE
void MtcEmergencyServiceManager::NotifyEmergencyServiceChanged(IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("NotifyEmergencyServiceChanged :: state=%d, reason=%d", m_eState, eReason, 0);
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (!pNormalService)
    {
        // TODO: change IMtcContext#GetServiceByType to return reference.
        return;
    }

    IJniMtcServiceThread* pThread = pNormalService->GetJniServiceThread();
    if (pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "IJniMtcServiceThread is null", 0, 0, 0);
        return;
    }

    pThread->OnEmergencyServiceChanged(static_cast<IMS_SINT32>(m_eState), eReason,
            static_cast<IMS_SINT32>(ServiceType::EMERGENCY));
}
