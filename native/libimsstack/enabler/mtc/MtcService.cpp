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

#include "AString.h"
#include "CarrierConfig.h"
#include "Connector.h"
#include "Engine.h"
#include "ICapabilities.h"
#include "ICarrierConfig.h"
#include "IConfiguration.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IImsAos.h"
#include "IIpcan.h"
#include "IJniEnabler.h"
#include "IJniMtcServiceThread.h"
#include "IMtcCallController.h"
#include "IMtcContext.h"
#include "IMtcImsEventReceiver.h"
#include "IMtcService.h"
#include "IServiceFilterCriteria.h"
#include "ISipRoutingRejectNotifier.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsCore.h"
#include "ImsEventDef.h"
#include "ImsServiceConfig.h"
#include "JniEnablerConnector.h"
#include "MtcRoutingRejectHandler.h"
#include "MtcService.h"
#include "ServiceConfig.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipFactory.h"
#include "SipMethod.h"
#include "common/IAppConfig.h"
#include "common/ICoreServiceConfig.h"
#include "common/IMediaConfig.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include "helper/IMtcNetworkWatcherListener.h"
#include "helper/MtcAosConnector.h"
#include "helper/MtcAosEventHandler.h"
#include "helper/MtcCapabilityQueryHandler.h"
#include "helper/MtcNetworkWatcher.h"
#include "helper/SrvccStateManager.h"
#include "helper/SsacTimerHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL IMS_CHAR FEATURE_TAG_CALL_COMPOSER[] = "+g.gsma.callcomposer";

PUBLIC
MtcService::MtcService(IN IMtcContext& objContext, IN ServiceType eType) :
        ImsService(AString::ConstNull()),
        m_bFeatureAddedForCallComposer(IMS_FALSE),
        m_eType(eType),
        m_objContext(objContext),
        m_strServiceName(GetServiceName(eType)),
        m_eOldStatus(ServiceStatus::SERVICE_IDLE),
        m_eStatus(ServiceStatus::SERVICE_IDLE),
        m_piCoreService(IMS_NULL),
        m_pAosConnector(IMS_NULL),
        m_pAosEventHandler(IMS_NULL),
        m_pSrvccStateManager(IMS_NULL),
        m_pNetworkWatcher(IMS_NULL),
        m_pRoutingRejectHandler(IMS_NULL),
        m_objSsacTimerHandler(SsacTimerHandler(m_objContext)),
        m_eTbcwStatus(SuppStatus::UNPROVISIONED),
        m_eTirStatus(SuppStatus::UNPROVISIONED)
{
    IMS_TRACE_I("+MtcService [slot_%d][type:%d]", m_objContext.GetSlotId(), m_eType, 0);
    Init();
}

PUBLIC VIRTUAL MtcService::~MtcService()
{
    IMS_TRACE_I("~MtcService [slot_%d][type:%d]", m_objContext.GetSlotId(), m_eType, 0);

    if (m_eType == ServiceType::NORMAL)
    {
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                m_objContext.GetSlotId(), EnablerType::MTC_SERVICE, IMS_NULL);
    }

    if (m_piCoreService)
    {
        m_piCoreService->SetListener(IMS_NULL);
        m_piCoreService->Close();
        m_piCoreService = IMS_NULL;
    }

    SetAosReady(IMS_FALSE);
    delete m_pAosConnector;

    IImsAos* piImsAos = ImsAos::GetImsAos(ImsServiceConfig::GetAppName(ImsAppId::MTC),
            m_strServiceName, m_objContext.GetSlotId());
    if (piImsAos != IMS_NULL)
    {
        piImsAos->SetListener(IMS_NULL);
    }

    delete m_pAosEventHandler;
    delete m_pSrvccStateManager;

    if (m_pRoutingRejectHandler)
    {
        ISipRoutingRejectNotifier* piRoutingRejectNotifier =
                SipFactory::GetRoutingRejectNotifier(m_objContext.GetSlotId());
        piRoutingRejectNotifier->RemoveListener(m_pRoutingRejectHandler);
        delete m_pRoutingRejectHandler;
    }

    delete m_pNetworkWatcher;
}

PUBLIC VIRTUAL void MtcService::AddAosStateListener(IN IMtcAosStateListener* piListener)
{
    m_pAosEventHandler->AddListener(piListener);
}

PUBLIC VIRTUAL void MtcService::RemoveAosStateListener(IN IMtcAosStateListener* piListener)
{
    m_pAosEventHandler->RemoveListener(piListener);
}

PUBLIC VIRTUAL void MtcService::AddSrvccStateListener(IN ISrvccStateListener* piListener)
{
    m_pSrvccStateManager->AddListener(piListener);
}

PUBLIC VIRTUAL void MtcService::RemoveSrvccStateListener(IN ISrvccStateListener* piListener)
{
    m_pSrvccStateManager->RemoveListener(piListener);
}

PUBLIC VIRTUAL void MtcService::AddNetworkWatcherListener(IN IMtcNetworkWatcherListener* piListener)
{
    m_pNetworkWatcher->AddListener(*piListener);
}

PUBLIC VIRTUAL void MtcService::RemoveNetworkWatcherListener(
        IN IMtcNetworkWatcherListener* piListener)
{
    m_pNetworkWatcher->RemoveListener(*piListener);
}

PUBLIC VIRTUAL IMS_SINT32 MtcService::GetRatType() const
{
    return m_pNetworkWatcher->GetRatType();
}

PUBLIC VIRTUAL IMS_SINT32 MtcService::GetMobileRatType() const
{
    return m_pNetworkWatcher->GetMobileRatType();
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsNr() const
{
    if (IsWlanIpCanType())
    {
        return IMS_FALSE;
    }

    return PhoneInfoService::GetPhoneInfoService()
                   ->GetNetworkWatcher(m_objContext.GetSlotId())
                   ->GetNetRadioTechType() == NW_REPORT_RADIO_NR;
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsEpsOnlyAttach() const
{
    return PhoneInfoService::GetPhoneInfoService()
                    ->GetNetworkWatcher(m_objContext.GetSlotId())
                    ->GetNetRadioTechType() == NW_REPORT_RADIO_LTE &&
            m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_LTE_INFO) ==
            IMS_LTE_INFO_EPS_ONLY_ATTACHED;
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsEpsCombinedAttach() const
{
    return PhoneInfoService::GetPhoneInfoService()
                    ->GetNetworkWatcher(m_objContext.GetSlotId())
                    ->GetNetRadioTechType() == NW_REPORT_RADIO_LTE &&
            m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_LTE_INFO) ==
            IMS_LTE_INFO_COMBINED_ATTACHED;
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsRoaming() const
{
    return m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_ROAMING_STATE) ==
            IMS_ROAMING_STATE_ON;
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsWlanIpCanType() const
{
    if (m_pAosConnector == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pAosConnector->GetIpcanType() == IIpcan::CATEGORY_WLAN;
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsCsfbAvailable() const
{
    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                ConfigVoice::CSFB_BLOCK_CONDITION_IF_EPS_ONLY_ATTACH) &&
            IsEpsOnlyAttach())
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                ConfigVoice::CSFB_BLOCK_CONDITION_IN_NR) &&
            IsNr())
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                ConfigVoice::CSFB_BLOCK_CONDITION_IN_WIFI) &&
            IsWlanIpCanType())
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                ConfigVoice::CSFB_BLOCK_CONDITION_IN_ROAMING) &&
            IsRoaming())
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                ConfigVoice::CSFB_BLOCK_CONDITION_IN_HOME) &&
            !IsRoaming())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IJniMtcServiceThread* MtcService::GetJniServiceThread() const
{
    IJniEnabler* piJniEnabler = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_objContext.GetSlotId(), EnablerType::MTC_SERVICE);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcServiceThread is null", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtcServiceThread*>(piJniEnabler->GetJniThread());
}

PUBLIC VIRTUAL void MtcService::UpdateSrvccState(IN SrvccState eState)
{
    IMS_TRACE_I("UpdateSrvccState", 0, 0, 0);
    m_pSrvccStateManager->UpdateSrvccState(eState);
    if (m_eType == ServiceType::NORMAL)
    {
        // UpdateSrvccState is invoked only for ServiceType::NORMAL.
        IMtcService* piEmergencyService = m_objContext.GetServiceByType(ServiceType::EMERGENCY);
        if (piEmergencyService)
        {
            piEmergencyService->UpdateSrvccState(eState);
        }
    }
}

PUBLIC VIRTUAL void MtcService::OpenEmergencyService(IN ServiceType eServiceType)
{
    m_objContext.GetEmergencyServiceManager().StartOpen(eServiceType);
}

PUBLIC VIRTUAL void MtcService::StopEmergencyService()
{
    m_objContext.GetEmergencyServiceManager().StopOpen(IMS_TRUE);
}

PUBLIC VIRTUAL void MtcService::ProcessTestCommand(
        IN IMS_SINT32 nCommand, IN IMS_SINT32 nWParam, IN IMS_SINT32 nLParam)
{
    IMS_TRACE_I("ProcessTestCommand [%d %d %d]", nCommand, nWParam, nLParam);
    switch (nCommand)
    {
        case static_cast<IMS_SINT32>(TestCommand::AOS_CONNECTED):
            ImsAos_Connected((IMS_UINT32)nWParam, (IMS_UINT32)nLParam);
            break;
        case static_cast<IMS_SINT32>(TestCommand::AOS_DISCONNECTED):
            ImsAos_Disconnected((IMS_UINT32)nWParam);
            break;
        case static_cast<IMS_SINT32>(TestCommand::RAT_CHANGED):
            m_pNetworkWatcher->SetTestRatChanged((IMS_SINT32)nWParam);
            break;
        default:
            break;
    }
}

PUBLIC VIRTUAL void MtcService::SetTerminalBasedCallWaiting(IN IMS_BOOL bEnabled)
{
    IMS_TRACE_I("SetTerminalBasedCallWaiting bEnabled[%s]", _TRACE_B_(bEnabled), 0, 0);

    m_eTbcwStatus = bEnabled ? SuppStatus::PROVISIONED_ENABLED : SuppStatus::PROVISIONED_DISABLED;
}

PUBLIC VIRTUAL void MtcService::SetTerminalBasedTir(IN IMS_BOOL bEnabled)
{
    IMS_TRACE_I("SetTerminalBasedTir bEnabled[%s]", _TRACE_B_(bEnabled), 0, 0);

    m_eTirStatus = bEnabled ? SuppStatus::PROVISIONED_ENABLED : SuppStatus::PROVISIONED_DISABLED;
}

PUBLIC VIRTUAL void MtcService::CoreService_ServiceClosed(
        IN ICoreService* /*piService*/, IN IReasonInfo* /*piReasonInfo*/)
{
    IMS_TRACE_I("CoreService_ServiceClosed", 0, 0, 0);
}

PUBLIC VIRTUAL void MtcService::CoreService_SessionInvitationReceived(
        IN ICoreService* piService, IN ISession* piSession)
{
    (void)piService;
    IMS_TRACE_I("CoreService_SessionInvitationReceived", 0, 0, 0);
    m_objContext.GetCallController().HandleIncoming(this, piSession);
}

PUBLIC VIRTUAL void MtcService::CoreService_CapabilityQueryReceived(
        IN ICoreService* piService, IN ICapabilities* piCapabilities)
{
    const IAppConfig* piAppConfig = Engine::GetConfiguration()->GetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTC), m_objContext.GetSlotId());
    const ICoreServiceConfig* piCoreServiceConfig =
            piAppConfig ? piAppConfig->GetCoreServiceConfig(m_strServiceName) : IMS_NULL;
    const IMediaConfig* piMediaConfig =
            Engine::GetConfiguration()->GetMediaConfig(m_objContext.GetSlotId());
    IMS_UINT32 nFeatures = m_pAosConnector ? m_pAosConnector->GetFeatures() : 0;

    MtcCapabilityQueryHandler(m_objContext, piCoreServiceConfig, piMediaConfig)
            .HandleIncomingCapabilityQuery(piService, piCapabilities, nFeatures);
}

PUBLIC VIRTUAL void MtcService::ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan)
{
    IMS_TRACE_I("ImsAos_Connected", 0, 0, 0);
    SetStatus(ServiceStatus::SERVICE_ACTIVE);
    if (!IsEmergency())
    {
        UpdateCallComposerFeature(nFeatures);
    }

    m_pNetworkWatcher->OnServiceConnected(nIpcan);

    m_pAosEventHandler->OnConnected(nFeatures);
    SetAosReady(IMS_TRUE);
}

PUBLIC VIRTUAL void MtcService::ImsAos_Connecting() {}

PUBLIC VIRTUAL void MtcService::ImsAos_Disconnecting(IN IMS_UINT32 nReason)
{
    m_pAosEventHandler->OnDisconnecting(nReason);
}

PUBLIC VIRTUAL void MtcService::ImsAos_Disconnected(IN IMS_UINT32 nReason)
{
    SetStatus(ServiceStatus::SERVICE_IDLE);
    m_pAosEventHandler->OnDisconnected(nReason);
}

PUBLIC VIRTUAL void MtcService::ImsAos_Suspended(IN IMS_UINT32 nReason)
{
    SetStatus(ServiceStatus::SERVICE_SUSPENDED);
    m_pAosEventHandler->OnSuspended(nReason);
}

PUBLIC VIRTUAL void MtcService::ImsAos_Resumed()
{
    SetStatus(ServiceStatus::SERVICE_ACTIVE);
    m_pAosEventHandler->OnResumed();
}

PUBLIC VIRTUAL void MtcService::ImsAosMonitor_Connected(
        IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan)
{
    m_pAosEventHandler->OnServiceConnected(nServices, nIpcan);
}

PUBLIC VIRTUAL void MtcService::ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    m_pAosEventHandler->OnEventNotify(nType, nState);
}

PRIVATE
void MtcService::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    if (m_eType == ServiceType::NORMAL)
    {
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                m_objContext.GetSlotId(), EnablerType::MTC_SERVICE, this);
    }

    m_pAosEventHandler = new MtcAosEventHandler(*this, m_objContext.GetConfigurationProxy());
    m_pSrvccStateManager = new SrvccStateManager();
    m_pNetworkWatcher = new MtcNetworkWatcher(*this, m_objContext.GetSlotId());

    AttachCoreServiceInterface();
    AttachAosInterface();

    if (m_objContext.GetConfigurationProxy().GetBoolean(ConfigVoice::
                        KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL))
    {
        m_pRoutingRejectHandler = new MtcRoutingRejectHandler(m_objContext,
                *PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(
                        m_objContext.GetSlotId()));

        ISipRoutingRejectNotifier* piRoutingRejectNotifier =
                SipFactory::GetRoutingRejectNotifier(m_objContext.GetSlotId());
        piRoutingRejectNotifier->AddListener(m_pRoutingRejectHandler);
    }
}

PRIVATE
void MtcService::SetStatus(IN ServiceStatus eStatus)
{
    m_eOldStatus = m_eStatus;
    m_eStatus = eStatus;
}

PRIVATE
AString MtcService::GetServiceName(IN ServiceType eType)
{
    if (eType == ServiceType::EMERGENCY)
    {
        return ImsServiceConfig::GetServiceName(ImsServiceId::MTC_EMERGENCY);
    }
    return ImsServiceConfig::GetServiceName(ImsServiceId::MTC);
}

PRIVATE
void MtcService::AttachCoreServiceInterface()
{
    AString strParams;
    strParams.Sprintf("%s=%s", "serviceId", m_strServiceName.GetStr());
    AString strAppName = ImsServiceConfig::GetAppName(ImsAppId::MTC);

    m_piCoreService = reinterpret_cast<ICoreService*>(
            Connector::Open(ImsCore::CONNECTION_SCHEME, strAppName, strParams));

    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_piCoreService is NULL", 0, 0, 0);
        return;
    }
    m_piCoreService->SetListener(this);
    SetServiceFilterCriteria();

    IMS_TRACE_I("AttachCoreServiceInterface : AppName[%s] StrParams[%s]", strAppName.GetStr(),
            strParams.GetStr(), 0);
}

PRIVATE
void MtcService::AttachAosInterface()
{
    IImsAos* piImsAos = ImsAos::GetImsAos(ImsServiceConfig::GetAppName(ImsAppId::MTC),
            m_strServiceName, m_objContext.GetSlotId());

    if (piImsAos == IMS_NULL)
    {
        IMS_TRACE_E(0, "piImsAos is NULL", 0, 0, 0);
        return;
    }
    piImsAos->SetListener(this);
    m_pAosConnector = new MtcAosConnector(*piImsAos, *(piImsAos->GetAosInfo()));
}

PRIVATE
void MtcService::SetServiceFilterCriteria() const
{
    IServiceFilterCriteria* piSfc = m_piCoreService->GetFilterCriteria();

    if (piSfc == IMS_NULL)
    {
        return;
    }

    SipMethod objInviteMethod(SipMethod::INVITE);
    TriggerPoint objInviteTp(objInviteMethod);
    piSfc->AddTriggerPoint(objInviteTp);

    SipMethod objOptionsMethod(SipMethod::OPTIONS);
    TriggerPoint objOptionsTp(objOptionsMethod);
    piSfc->AddTriggerPoint(objOptionsTp);
}

PRIVATE
void MtcService::SetAosReady(IN IMS_BOOL bReady)
{
    if (m_pAosConnector == IMS_NULL)
    {
        return;
    }

    if (m_eType == ServiceType::NORMAL)
    {
        m_pAosConnector->SetReady(bReady, ImsAosService::MTC);
    }
    else
    {
        m_pAosConnector->SetReady(bReady, ImsAosService::EMERGENCY_MTC);
    }
}

PRIVATE
void MtcService::UpdateCallComposerFeature(IN IMS_UINT32 nFeatures)
{
    ICoreService* pCoreService = GetICoreService();
    IFeatureCaps* pFeatureCapabilities =
            pCoreService != IMS_NULL ? pCoreService->GetFeatureCaps() : IMS_NULL;
    if (pFeatureCapabilities == IMS_NULL)
    {
        return;
    }

    if (nFeatures & ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY)
    {
        if (!m_bFeatureAddedForCallComposer)
        {
            pFeatureCapabilities->AddFeature(
                    FEATURE_TAG_CALL_COMPOSER, AString::ConstEmpty(), SipMethod::INVITE);
            m_bFeatureAddedForCallComposer = IMS_TRUE;
        }
    }
    else
    {
        if (m_bFeatureAddedForCallComposer)
        {
            pFeatureCapabilities->RemoveFeature(
                    FEATURE_TAG_CALL_COMPOSER, AString::ConstEmpty(), SipMethod::INVITE);
            m_bFeatureAddedForCallComposer = IMS_FALSE;
        }
    }
}
