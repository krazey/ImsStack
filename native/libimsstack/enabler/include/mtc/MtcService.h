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
#include "IMtcService.h"
#include "ImsService.h"
#include "ImsTypeDef.h"
#include "helper/SrvccStateManager.h"

class IJniMtcServiceThread;
class IMtcAosConnector;
class IMtcAosStateListener;
class IMtcContext;
class MtcAosEventHandler;
class MtcRoutingRejectHandler;

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
    void AddAosStateListener(IN IMtcAosStateListener* piListener) override;
    void RemoveAosStateListener(IN IMtcAosStateListener* piListener) override;
    void AddSrvccStateListener(IN ISrvccStateListener* piListener) override;
    void RemoveSrvccStateListener(IN ISrvccStateListener* piListener) override;

    inline IMS_BOOL IsActive() const override { return m_eStatus == ServiceStatus::SERVICE_ACTIVE; }
    inline IMS_BOOL IsEmergency() const override { return m_eType == ServiceType::EMERGENCY; }
    IMS_BOOL IsWlanIpCanType() const override;
    inline ServiceStatus GetOldStatus() const override { return m_eOldStatus; }
    inline ServiceStatus GetStatus() const override { return m_eStatus; }
    inline ICoreService* GetICoreService() const override { return m_piCoreService; }
    inline IMtcAosConnector* GetAosConnector() const override { return m_pAosConnector; }
    IJniMtcServiceThread* GetJniServiceThread() const override;
    inline SrvccState GetSrvccState() const override { return m_pSrvccStateManager->GetState(); }

    void UpdateSrvccState(IN SrvccState eState) override;
    void SetTerminalBasedCallWaiting(IN IMS_BOOL bEnabled) override;
    void OpenEmergencyService(IN IuMtcService::EmergencyCallRoutingPdn ePdn) override;
    void StopEmergencyService() override;
    void ProcessTestCommand(
            IN IMS_SINT32 nCommand, IN IMS_SINT32 nWParam, IN IMS_SINT32 nLParam) override;
    TbcwStatus GetTbcwStatus() const override { return m_eTbcwStatus; }

    inline void NotifyJniEnablerSet() override {}

    // ICoreServiceListener implementation
    inline void CoreService_PageMessageReceived(IN ICoreService*, IN IPageMessage*) override {}
    inline void CoreService_ReferenceReceived(IN ICoreService*, IN IReference*) override{};
    void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo) override;
    void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession) override;
    inline void CoreService_UnsolicitedNotifyReceived(IN ICoreService*, IN IMessage*) override{};
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
    IMS_BOOL m_bFeatureAddedForCallComposer;

    void Init();
    void SetStatus(IN ServiceStatus eStatus);
    static AString GetServiceName(IN ServiceType eType);
    void AttachCoreServiceInterface();
    void AttachAosInterface();
    void SetServiceFilterCriteria() const;
    void SetAosReady(IN IMS_BOOL);
    void UpdateCallComposerFeature(IN IMS_UINT32 nFeatures);

protected:
    ServiceType m_eType;
    IMtcContext& m_objContext;
    AString m_strServiceName;
    ServiceStatus m_eOldStatus;
    ServiceStatus m_eStatus;
    ICoreService* m_piCoreService;
    IMtcAosConnector* m_pAosConnector;
    MtcAosEventHandler* m_pAosEventHandler;
    SrvccStateManager* m_pSrvccStateManager;
    MtcRoutingRejectHandler* m_pRoutingRejectHandler;
    TbcwStatus m_eTbcwStatus;
    enum
    {
        TEST_COMMAND_AOS_CONNECTED = 0,
        TEST_COMMAND_AOS_DISCONNECTED = 1
    };
};

#endif
