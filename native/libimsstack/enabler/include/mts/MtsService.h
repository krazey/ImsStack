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
#include "IImsAosMonitor.h"
#include "IMtsService.h"
#include "ImsService.h"
#include "IMtsTrafficListener.h"

class IImsAos;
class IImsRadio;
class IJniMtsServiceThread;
class IMtsTraffic;
class INetworkWatcher;

class MtsService final :
        public ICoreServiceListener,
        public IImsAosListener,
        public IImsAosMonitor,
        public IMtsService,
        public IMtsTrafficListener,
        public ImsService
{
public:
    MtsService(IN IMS_SINT32 nSlotId);
    virtual ~MtsService();

    // ICoreServiceListener
    void CoreService_PageMessageReceived(IN ICoreService* piService, IN IPageMessage* piMessage);
    void CoreService_ReferenceReceived(IN ICoreService* piService, IN IReference* piReference);
    void CoreService_ServiceClosed(IN ICoreService* piService, IN IReasonInfo* piReasonInfo);
    void CoreService_SessionInvitationReceived(IN ICoreService* piService, IN ISession* piSession);
    void CoreService_UnsolicitedNotifyReceived(IN ICoreService* piService, IN IMessage* piNotify);
    void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities);

    // IImsAosListener
    void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    void ImsAos_Connecting() override;
    void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    void ImsAos_Resumed() override;

    // IImsAosMonitor
    void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) override;
    void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

    // IMtsService
    ICoreService* GetICoreService(IN IMS_BOOL bEmergency) const override;
    inline IMtsServiceState* GetIMtsServiceState() override { return m_piMtsServiceState; }
    void ReportMoStatus(IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat,
            IN IMS_UINT8 nRetryAfter, IN IMS_SINT32 nSeqId) override;
    void ReportMtSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objData) override;
    void RequestRegistrationRecovery(IN IMS_UINT32 nRecoveryType) override;
    void SetListener(IN IMtsServiceListener* piMtsServiceListener) override;
    inline void NotifyJniEnablerSet() override {}
    void SendMoSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId) override;
    void SendMtResult(IN IMS_BOOL bMtResult) override;
    void SendScbmNotification(IN IMS_UINT32 nScbmState) override;

    // IMtsTrafficListener
    void Traffic_OnConnectionFailed(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection,
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void Traffic_OnConnectionSetupPrepared(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection) override;
    void Traffic_GuardTimerExpired(IN IMS_UINT32 nType, IN IMS_UINT32 nDirection) override;

    // TODO: need to check if it is deprecated or not
    void IMSAoSApp_NotifySpecificMessage(
            IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam);

    // Test-Purpose
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
    IJniMtsServiceThread* GetJniThread();
    IMS_BOOL IsEccNumber(IN const AString& strDstAddr);
    void StartRadioTraffic(IN IMtsTraffic* piMtsTraffic);

    void Init();
    void DeInit();

    IMS_BOOL m_bEmergencyActived;
    IImsAos* m_piImsAos;
    IImsAos* m_piImsEmergencyAos;
    INetworkWatcher* m_piNetWatcherInfo;
    AString m_strAppId;
    IMS_UINT32 m_nSlotId;
    ICoreService* m_piCoreService;
    ICoreService* m_piEmergencyCoreService;
    IMtsServiceListener* m_piMtsServiceListener;
    IMtsServiceState* m_piMtsServiceState;
    IImsRadio* m_piImsRadio;
    ImsList<IMtsTraffic*> m_objMtsTraffics;
    SmsSendRequestInfo* m_pSmsInfo;
};

#endif
