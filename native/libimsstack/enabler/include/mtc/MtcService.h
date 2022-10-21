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

#ifndef MTC_SERVICE_H_
#define MTC_SERVICE_H_

#include "AString.h"
#include "ICoreService.h"
#include "ICoreServiceListener.h"
#include "IImsAosListener.h"
#include "IImsAosMonitor.h"
#include "ImsService.h"
#include "ImsTypeDef.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "MtcRoutingRejectHandler.h"
#include "helper/MtcAosConnector.h"
#include "helper/SrvccStateManager.h"

class IMtcAosConnector;
class MtcAosEventHandler;
class IJniMtcServiceThread;

class MtcService :
        public ImsService,
        public IMtcService,
        public ICoreServiceListener,
        public IImsAosListener,
        public IImsAosMonitor
{
public:
    MtcService(IN IMtcContext& objContext, IN ServiceType eType);
    virtual ~MtcService();
    MtcService(IN const MtcService&) = delete;
    MtcService& operator=(IN const MtcService&) = delete;

    // IMtcService implementation
    inline ServiceType GetServiceType() const override { return m_eType; }
    void AddSrvccStateListener(IN ISrvccStateListener* piListener) override;
    void RemoveSrvccStateListener(IN ISrvccStateListener* piListener) override;

    inline IMS_BOOL IsActive() const override { return m_eStatus == ServiceStatus::SERVICE_ACTIVE; }
    inline IMS_BOOL IsEmergency() const override { return m_eType == ServiceType::EMERGENCY; }
    IMS_BOOL IsWlanIpCanType() const override;
    inline ServiceStatus GetServiceStatus() const override { return m_eStatus; }
    inline ICoreService* GetICoreService() const override { return m_piCoreService; }
    inline IMtcAosConnector* GetAosConnector() const override { return m_pAosConnector; }
    inline SrvccState GetSrvccState() const override { return m_pSrvccStateManager->GetState(); }

    void UpdateSrvccState(IN SrvccState eState) override;
    void SetTerminalBasedCallWaiting(IN IMS_BOOL bEnabled) override;
    IMS_BOOL IsTerminalBasedCallWaitingEnabled() const override
    {
        return m_bTerminalBasedCallWaitingEnabled;
    }
    void OpenEmergencyService() override;

    inline void NotifyJniEnablerSet() override {}

    // ICoreServiceListener implementation
    inline void CoreService_PageMessageReceived(
            IN ICoreService*, IN IPageMessage*) override {}
    inline void CoreService_ReferenceReceived(
            IN ICoreService*, IN IReference*) override {};
    void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo) override;
    void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession) override;
    inline void CoreService_UnsolicitedNotifyReceived(
            IN ICoreService*, IN IMessage*) override {};
    void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities) override;

    void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    void ImsAos_Connecting() override;
    void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    void ImsAos_Resumed() override;

    // IIMSAoSAppMonitor implementation
    void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) override;
    void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

private:
    void Init();
    AString GetServiceName(IN ServiceType eType) const;
    void AttachCoreServiceInterface();
    void AttachAosInterface();
    IJniMtcServiceThread* GetJniThread();
    void SetServiceFilterCriteria();
    void SetAosReady(IN IMS_BOOL);

protected:
    ServiceType m_eType;
    IMtcContext& m_objContext;
    AString m_strServiceName;
    ServiceStatus m_eStatus;
    ICoreService* m_piCoreService;
    MtcAosConnector* m_pAosConnector;
    MtcAosEventHandler* m_pAosEventHandler;
    SrvccStateManager* m_pSrvccStateManager;
    MtcRoutingRejectHandler* m_pRoutingRejectHandler;
    IMS_BOOL m_bTerminalBasedCallWaitingEnabled;
};

#endif
