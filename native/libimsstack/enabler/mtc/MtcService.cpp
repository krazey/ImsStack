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
#include "Connector.h"
#include "ICapabilities.h"
#include "ICoreService.h"
#include "IImsAos.h"
#include "IIpcan.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsCore.h"
#include "ImsServiceConfig.h"
#include "IMtcCallController.h"
#include "IMtcService.h"
#include "INetworkWatcher.h"
#include "IServiceFilterCriteria.h"
#include "ISipRoutingRejectNotifier.h"
#include "JniConnectorFactory.h"
#include "JniMtcService.h"
#include "JniMtcServiceThread.h"
#include "MtcEmergencyServiceManager.h"
#include "MtcService.h"
#include "ServiceTrace.h"
#include "SipFactory.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcAosConnector.h"
#include "helper/MtcCapabilityQueryHandler.h"
#include "helper/SrvccEventHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcService::MtcService(IN IMtcContext& objContext, IN ServiceType eType) :
        ImsService(AString::ConstNull()),
        m_eType(eType),
        m_objContext(objContext),
        m_strServiceName(GetServiceName(eType)),
        m_eStatus(ServiceStatus::SERVICE_IDLE),
        m_piCoreService(IMS_NULL),
        m_pAosConnector(IMS_NULL),
        m_objAosEventHandler(MtcAosEventHandler(*this, objContext.GetConfigurationProxy())),
        m_objSrvccEventHandler(SrvccEventHandler(objContext)),
        m_pJniService(IMS_NULL),
        m_pRoutingRejectHandler(nullptr),
        m_bTerminalBasedCallWaitingEnabled(IMS_TRUE/*m_objContext.GetConfigurationProxy().Is(
                Feature::TERMINAL_BASED_CALL_WAIT_DEFAULT_ENABLED)*/)
{
    IMS_TRACE_I("+MtcService [slot_%d][type:%d]", m_objContext.GetSlotId(), m_eType, 0);
    Init();
}

PUBLIC VIRTUAL MtcService::~MtcService()
{
    IMS_TRACE_I("~MtcService [slot_%d][type:%d]", m_objContext.GetSlotId(), m_eType, 0);
    if (m_pJniService)
    {
        m_pJniService->SetMtcService(IMS_NULL);
        m_pJniService = IMS_NULL;
    }

    if (m_eType == ServiceType::NORMAL)
    {
        // TODO: temp to fix crash issue
        JniConnectorFactory::GetInstance()
                ->GetMtcServiceConnector(m_objContext.GetSlotId())
                ->SetEnablerService(IMS_NULL);
    }

    if (m_piCoreService)
    {
        m_piCoreService->SetListener(this);
        m_piCoreService->Close();
        m_piCoreService = IMS_NULL;
    }

    SetAosReady(IMS_FALSE);
    delete m_pAosConnector;
    m_pAosConnector = IMS_NULL;

    IImsAos* piImsAos = ImsAos::GetImsAos(ImsServiceConfig::GetAppName(ImsAppId::MTC),
            m_strServiceName, m_objContext.GetSlotId());
    if (piImsAos != IMS_NULL)
    {
        piImsAos->SetListener(IMS_NULL);
    }

    if (m_pRoutingRejectHandler)
    {
        ISipRoutingRejectNotifier* piRoutingRejectNotifier =
                SipFactory::GetRoutingRejectNotifier(m_objContext.GetSlotId());
        piRoutingRejectNotifier->RemoveListener(m_pRoutingRejectHandler.get());
    }
}

PUBLIC VIRTUAL void MtcService::AddSrvccStateListener(IN ISrvccStateListener* piListener)
{
    m_objSrvccEventHandler.AddListener(piListener);
}

PUBLIC VIRTUAL void MtcService::RemoveSrvccStateListener(IN ISrvccStateListener* piListener)
{
    m_objSrvccEventHandler.RemoveListener(piListener);
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsWlanIpCanType() const
{
    if (m_pAosConnector == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pAosConnector->GetIpcanType() == IIpcan::CATEGORY_WLAN;
}

PUBLIC VIRTUAL IMS_BOOL MtcService::IsWifiRegistered() const
{
    if (m_pAosConnector == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pAosConnector->GetRegisteredNetworkType() == NW_REPORT_RADIO_WLAN;
}

PUBLIC VIRTUAL void MtcService::UpdateSrvccState(IN SrvccState eState)
{
    IMS_TRACE_I("UpdateSrvccState", 0, 0, 0);
    m_objSrvccEventHandler.UpdateSrvccState(eState);
}

PUBLIC VIRTUAL void MtcService::SetJniService(IN JniMtcService* pJniService)
{
    m_pJniService = pJniService;
    if (pJniService)
    {
        IMS_TRACE_I("SetJniService [slot_%d]", m_objContext.GetSlotId(), 0, 0);
        m_objContext.GetEmergencyServiceManager()->SetJniServiceThread(m_pJniService->GetThread());
    }
    else
    {
        IMS_TRACE_I("SetJniService Null [slot_%d]", m_objContext.GetSlotId(), 0, 0);
        m_objContext.GetEmergencyServiceManager()->SetJniServiceThread(IMS_NULL);

        // TODO: temp to fix crash issue
        JniConnectorFactory::GetInstance()
                ->GetMtcServiceConnector(m_objContext.GetSlotId())
                ->SetEnablerService(IMS_NULL);
    }
}

PUBLIC VIRTUAL void MtcService::SetTerminalBasedCallWaiting(
        IN IMS_BOOL bProvisioned, IN IMS_BOOL bEnabled)
{
    IMS_TRACE_I("SetTerminalBasedCallWaiting bProvisioned[%s] bEnabled[%s]",
            _TRACE_B_(bProvisioned), _TRACE_B_(bEnabled), 0);
    if (bProvisioned)
    {
        if (bEnabled)
        {
            m_bTerminalBasedCallWaitingEnabled = IMS_TRUE;
        }
        else
        {
            m_bTerminalBasedCallWaitingEnabled = IMS_FALSE;
        }
    }
}

PUBLIC VIRTUAL void MtcService::OpenEmergencyService()
{
    m_objContext.GetEmergencyServiceManager()->OpenEmergencyService();
}

PUBLIC VIRTUAL void MtcService::CoreService_PageMessageReceived(
        IN ICoreService* /*piService*/, IN IPageMessage* /*piMessage*/)
{
}

PUBLIC VIRTUAL void MtcService::CoreService_ReferenceReceived(
        IN ICoreService* /*piService*/, IN IReference* /*piReference*/)
{
}

PUBLIC VIRTUAL void MtcService::CoreService_ServiceClosed(
        IN ICoreService* /*piService*/, IN IReasonInfo* /*piReasonInfo*/)
{
}

PUBLIC VIRTUAL void MtcService::CoreService_SessionInvitationReceived(
        IN ICoreService* piService, IN ISession* piSession)
{
    (void)piService;
    IMS_TRACE_I("CoreService_SessionInvitationReceived", 0, 0, 0);

    if (m_pJniService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pJniService is NULL", 0, 0, 0);
        m_objContext.GetCallController().HandleIncoming(this, piSession, IMS_NULL);
        return;
    }

    m_objContext.GetCallController().HandleIncoming(this, piSession, m_pJniService->GetThread());
}

PUBLIC VIRTUAL void MtcService::CoreService_UnsolicitedNotifyReceived(
        IN ICoreService* /*piService*/, IN IMessage* /*piNotify*/)
{
}

PUBLIC VIRTUAL void MtcService::CoreService_CapabilityQueryReceived(
        IN ICoreService* piService, IN ICapabilities* piCapabilities)
{
    MtcCapabilityQueryHandler::HandleIncomingCapabilityQuery(piService, piCapabilities,
            ImsServiceConfig::GetAppName(ImsAppId::MTC), m_strServiceName,
            m_objContext.GetSlotId());
}

PUBLIC VIRTUAL void MtcService::ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan)
{
    m_eStatus = ServiceStatus::SERVICE_ACTIVE;
    m_objAosEventHandler.OnConnected(nFeatures, nIpcan,
            m_pJniService ? m_pJniService->GetThread() : IMS_NULL,
            m_objContext.GetEmergencyServiceManager(), m_objContext.GetCallController());
    SetAosReady(IMS_TRUE);
}

PUBLIC VIRTUAL void MtcService::ImsAos_Connecting() {}

PUBLIC VIRTUAL void MtcService::ImsAos_Disconnecting(IN IMS_UINT32 nReason)
{
    m_objAosEventHandler.OnDisconnecting(nReason, m_objContext.GetCallController());
}

PUBLIC VIRTUAL void MtcService::ImsAos_Disconnected(IN IMS_UINT32 nReason)
{
    m_eStatus = ServiceStatus::SERVICE_IDLE;
    m_objAosEventHandler.OnDisconnected(nReason, m_objContext.GetCallController(),
            m_pJniService ? m_pJniService->GetThread() : IMS_NULL,
            m_objContext.GetEmergencyServiceManager());
}

PUBLIC VIRTUAL void MtcService::ImsAos_Suspended(IN IMS_UINT32 nReason)
{
    m_eStatus = ServiceStatus::SERVICE_SUSPENDED;
    m_objAosEventHandler.OnSuspended(nReason, m_objContext.GetCallController());
}

PUBLIC VIRTUAL void MtcService::ImsAos_Resumed()
{
    m_eStatus = ServiceStatus::SERVICE_ACTIVE;
    m_objAosEventHandler.OnResumed();
}

PUBLIC VIRTUAL void MtcService::ImsAosMonitor_Connected(
        IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan)
{
    m_objAosEventHandler.OnServiceConnected(nServices, nIpcan);
}

PUBLIC VIRTUAL void MtcService::ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    m_objAosEventHandler.OnEventNotify(nType, nState);
}

PRIVATE
void MtcService::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    if (m_eType == ServiceType::NORMAL)
    {
        // TODO: emergency service connector.
        JniConnectorFactory::GetInstance()
                ->GetMtcServiceConnector(m_objContext.GetSlotId())
                ->SetEnablerService(this);
    }
    AttachCoreServiceInterface();
    AttachAosInterface();

    if (m_objContext.GetConfigurationProxy().Is(Feature::
            USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION))
    {
        m_pRoutingRejectHandler = std::make_unique<MtcRoutingRejectHandler>(m_objContext);

        ISipRoutingRejectNotifier* piRoutingRejectNotifier =
                SipFactory::GetRoutingRejectNotifier(m_objContext.GetSlotId());
        piRoutingRejectNotifier->AddListener(m_pRoutingRejectHandler.get());
    }
}

PRIVATE
AString MtcService::GetServiceName(IN ServiceType eType) const
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
void MtcService::SetServiceFilterCriteria()
{
    IServiceFilterCriteria* piSfc = m_piCoreService->GetFilterCriteria();

    if (piSfc == IMS_NULL)
    {
        return;
    }

    SipMethod objInviteMethod(SipMethod::INVITE);
    TriggerPoint objInviteTp(objInviteMethod);
    piSfc->AddTriggerPoint(objInviteTp);

    IMS_TRACE_D("SetServiceFilterCriteria", 0, 0, 0);
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
