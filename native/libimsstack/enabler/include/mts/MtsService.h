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

#ifndef MTS_SERVICE_H_
#define MTS_SERVICE_H_

#include "ICoreServiceListener.h"
#include "IImsAosListener.h"
#include "IMtsService.h"
#include "ImsService.h"
#include "IMtsTrafficListener.h"
#include "MtsDef.h"
#include <memory>

class IImsAos;
class IImsRadio;
class IJniMtsServiceThread;
class IMtsContext;
class IMtsTraffic;
class INetworkWatcher;

class MtsService final :
        public ICoreServiceListener,
        public IImsAosListener,
        public IMtsService,
        public IMtsTrafficListener,
        public ImsService
{
public:
    explicit MtsService(IN IMtsContext& objContext);
    virtual ~MtsService();
    MtsService(IN const MtsService&) = delete;
    MtsService& operator=(IN const MtsService&) = delete;

    void Init();

    // ICoreServiceListener
    void CoreService_PageMessageReceived(
            IN ICoreService* piService, IN IPageMessage* piMessage) override;
    void CoreService_ReferenceReceived(
            IN ICoreService* piService, IN IReference* piReference) override;
    void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo) override;
    void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession) override;
    void CoreService_UnsolicitedNotifyReceived(
            IN ICoreService* piService, IN IMessage* piNotify) override;
    void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities) override;

    // IImsAosListener
    void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    void ImsAos_Connecting() override;
    void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    void ImsAos_Resumed() override;

    // IMtsService
    ICoreService* GetICoreService(IN IMS_BOOL bEmergency) const override;
    inline IMtsServiceState* GetIMtsServiceState() override { return m_piMtsServiceState; }
    IJniMtsServiceThread* GetJniServiceThread() const override;
    void RequestRegistrationRecovery(IN IMS_UINT32 nRecoveryType) override;
    void RequestRegisterWithNextPcscf(IN const IMS_UINT32 nRetryAfterValue) override;
    inline void NotifyJniEnablerSet() override {}
    void SendMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency) override;

    // IMtsTrafficListener
    void Traffic_OnConnectionFailed(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection,
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection) override;
    void Traffic_GuardTimerExpired(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection) override;

    // Test-Purpose
    void InitMtsServiceState();
    inline void SetIImsAos(IN IImsAos* piImsAos) { m_piImsAos = piImsAos; }
    inline void SetIImsEmergencyAos(IN IImsAos* piImsEmergencyAos)
    {
        m_piImsEmergencyAos = piImsEmergencyAos;
    }

private:
    void AttachJni();
    void AttachAos();
    void AttachCoreService();
    IMS_UINT32 ConvertToAccessNetworkType(
            IN IMS_UINT32 nTrafficType, IN IMS_SINT32 nReportedNetwork);
    IMtsTraffic* GetTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nDirection);
    IMS_BOOL IsEmergencySmsOverImsSupported() const;
    IMS_BOOL IsEmergencySmsReadyToSend() const;
    MtsTrafficStartResult StartMtTraffic(IN ICoreService* piService);
    MtsTrafficStartResult StartMoTrafficIfNeeded(IN IMS_BOOL bEmergency);

    void DeInit();

    IMtsContext& m_objContext;
    IImsAos* m_piImsAos;
    IImsAos* m_piImsEmergencyAos;
    INetworkWatcher* m_piNetWatcherInfo;
    AString m_strAppId;
    ICoreService* m_piCoreService;
    ICoreService* m_piEmergencyCoreService;
    IMtsServiceState* m_piMtsServiceState;
    IImsRadio* m_piImsRadio;
    ImsList<IMtsTraffic*> m_objMtsTraffics;
    std::unique_ptr<SmsSendRequestInfo> m_pSmsInfo;
};

#endif
