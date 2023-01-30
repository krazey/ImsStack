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
#include "INetworkWatcher.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "MtcEmergencyServiceManager.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/EmergencyNormalRoutingHelper.h"
#include "helper/IMtcAosConnector.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcEmergencyServiceManager::MtcEmergencyServiceManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_pNormalRoutingHelper(nullptr),
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

    if (m_eState == EmergencyServiceState::OPENED || m_eState == EmergencyServiceState::IN_CALL)
    {
        SetState(EmergencyServiceState::OPENED);
        NotifyEmergencyServiceChanged(-1);
        return;
    }

    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    pAosConnector->Control(ImsAosControl::REGISTER_START);
    if (m_eState != EmergencyServiceState::OPENING)
    {
        SetState(EmergencyServiceState::OPENING);
        NotifyEmergencyServiceChanged(-1);
    }
}

PUBLIC VIRTUAL void MtcEmergencyServiceManager::OnAosStateChanged(
        IN IMtcService& /*objMtcService*/, IN MtcAosState eState, IN IMS_UINT32 eAosReason)
{
    IMS_TRACE_D("OnAosStateChanged :: %d", eState, 0, 0);

    IMS_BOOL bNeedToNotify = IMS_FALSE;

    switch (eState)
    {
        case MtcAosState::DISCONNECTED:
            if (IsRetryOverImsPdnRequired(eAosReason))
            {
                SetState(EmergencyServiceState::IDLE);
                return HandleEmergencyCallOverImsPdn();
            }
            HandleServiceIdle(bNeedToNotify);
            break;
        case MtcAosState::CONNECTED:
            HandleServiceActive(bNeedToNotify);
            break;
        default:
            return;
    }

    if (bNeedToNotify)
    {
        NotifyEmergencyServiceChanged(static_cast<IMS_SINT32>(eAosReason));
    }
}

PUBLIC VIRTUAL void MtcEmergencyServiceManager::OnNormalRoutingClosed()
{
    m_pNormalRoutingHelper.reset();
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceIdle(IN IMS_BOOL& bNeedToNotify)
{
    IMS_TRACE_D("HandleServiceIdle", 0, 0, 0);

    bNeedToNotify = m_eState != EmergencyServiceState::IDLE;
    if (m_eState == EmergencyServiceState::OPENING)
    {
        SetState(EmergencyServiceState::UNAVAILABLE);
    }
    else
    {
        SetState(EmergencyServiceState::IDLE);
    }
}

PRIVATE
void MtcEmergencyServiceManager::HandleServiceActive(IN IMS_BOOL& bNeedToNotify)
{
    IMS_TRACE_D("HandleServiceActive", 0, 0, 0);

    if (m_eState == EmergencyServiceState::OPENING)
    {
        bNeedToNotify = IMS_TRUE;
        SetState(EmergencyServiceState::OPENED);
        return;
    }
    bNeedToNotify = IMS_FALSE;
}

PRIVATE
void MtcEmergencyServiceManager::HandleEmergencyCallOverImsPdn()
{
    IMS_TRACE_D("HandleEmergencyCallOverImsPdn", 0, 0, 0);
    if (!m_pNormalRoutingHelper)
    {
        m_pNormalRoutingHelper =
                std::make_unique<EmergencyNormalRoutingHelper>(m_objContext, *this);
    }
    m_pNormalRoutingHelper->HandleEmergencyCall();
}

PRIVATE
IMS_BOOL MtcEmergencyServiceManager::IsRetryOverImsPdnRequired(IN IMS_SINT32 eAosReason) const
{
    // TODO: Move into EmergencyNormalRoutingHelper.
    if (m_eState == EmergencyServiceState::OPENING &&
            m_objContext.GetConfigurationProxy().Is(Feature::RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
    {
        INetworkWatcher* pNetworkWatcher =
                PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(
                        m_objContext.GetSlotId());
        return eAosReason == ImsAosReason::DATA_DISCONNECTED &&
                pNetworkWatcher->GetRoamingState() == 0;
    }

    return IMS_FALSE;
}

PRIVATE
void MtcEmergencyServiceManager::SetState(IN EmergencyServiceState eState)
{
    IMS_TRACE_D("SetState :: [%d] -> [%d]", m_eState, eState, 0);
    m_eState = eState;
}

PRIVATE
void MtcEmergencyServiceManager::NotifyEmergencyServiceChanged(IN IMS_SINT32 eReason)
{
    IMS_TRACE_D("NotifyEmergencyServiceChanged :: state=%d, reason=%d", m_eState, eReason, 0);
    IMtcService* pNormalService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (!pNormalService)
    {
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
