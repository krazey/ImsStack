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
    explicit MtsService(IN IMtsContext& objContext, IN MtsServiceType eServiceType);
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
    void ImsAos_Disconnected(IN IMS_UINT32 nReason, IN IMS_SINT32 nDataFailureReason) override;
    void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    void ImsAos_Resumed() override;

    // IMtsService
    inline ICoreService* GetICoreService() const override { return m_piCoreService; }
    inline IMtsServiceState* GetIMtsServiceState() const override { return m_piMtsServiceState; }
    void RequestRegistrationRecovery(IN IMS_UINT32 nRecoveryType) const override;
    void RequestRegisterWithNextPcscf(IN const IMS_UINT32 nRetryAfterValue) const override;
    void SendMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency,
            IN IMS_UINT32 nRetryCount) override;

    // IMtsTrafficListener
    void Traffic_OnConnectionFailed(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection,
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection) override;
    void Traffic_GuardTimerExpired(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection) override;

    // Test-Purpose
    void InitMtsServiceState();
    inline void SetIImsAos(IN IImsAos* piImsAos) { m_piImsAos = piImsAos; }

private:
    void AttachAos();
    void AttachCoreService();
    IMS_UINT32 ConvertToAccessNetworkType(IN IMS_SINT32 nReportedNetwork);
    IMtsTraffic* GetTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nDirection);
    IMS_UINT32 GetTrafficTypeOfService() const;
    IMS_BOOL IsEmergencySmsOverImsSupported() const;
    IMS_BOOL IsEmergencySmsReadyToSend() const;
    MtsTrafficStartResult StartMtTraffic();
    MtsTrafficStartResult StartMoTrafficIfNeeded();

    void DeInit();

    IMtsContext& m_objContext;
    MtsServiceType m_eServiceType;
    IImsAos* m_piImsAos;
    INetworkWatcher* m_piNetWatcherInfo;
    AString m_strAppId;
    ICoreService* m_piCoreService;
    IMtsServiceState* m_piMtsServiceState;
    IImsRadio* m_piImsRadio;
    ImsList<IMtsTraffic*> m_objMtsTraffics;
    std::unique_ptr<SmsSendRequestInfo> m_pSmsInfo;
};

#endif
