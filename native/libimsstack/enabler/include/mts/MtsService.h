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
#include "IMtsServiceListener.h"
#include "ImsService.h"
#include "IuMts.h"
#include "IuMtsService.h"

class IImsAos;
class JniMtsService;
class JniMtsServiceThread;
class MtsDynamicLoader;

class MtsService final :
        public ICoreServiceListener,
        public IImsAosListener,
        public IImsAosMonitor,
        public IMtsService,
        public ImsService
{
public:
    MtsService(IN IMS_SINT32 nSlotId, IN MtsDynamicLoader* pMtsDynamicLoader);
    ~MtsService();

public:
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
    void SetJniMtsService(IN JniMtsService* pJniMtsService) override;
    void SendMoSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
                IN const AString& strAddress, IN IMS_SINT32 nSeqId) override;
    void SendMtResult(IN IMS_BOOL bMtResult) override;

    void ReportMoStatus(IN IMS_UINT32 nReason, IN IMS_UINT32 nSmsformat, IN IMS_UINT8 nRetryAfter,
            IN IMS_SINT32 nSeqId);
    void ReportMtSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData);

    void RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType);
    void IMSAoSApp_NotifySpecificMessage(
            IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam);
    IMS_BOOL IsEpdgConnected();
    ICoreService* GetICoreService() const;
    inline MtsDynamicLoader* GetMtsUtils() { return m_pMtsDynamicLoader; }
    void SetListener(IN IMtsServiceListener* piMtsServiceListener);

protected:
    // ImsService
    inline IMS_BOOL OnPreprocess(IN IMSMSG& /*objMSG*/) { return IMS_TRUE; }
    inline IMS_BOOL OnMessage(IN IMSMSG& /*objMSG*/) { return IMS_TRUE; }
    inline IMS_BOOL OnPostprocess(IN IMSMSG& /*objMSG*/) { return IMS_TRUE; }

    const AString& GetAppId() const;

private:
    IMS_BOOL Attach();
    void AttachAosInterface();
    void AttachCoreServiceInterface();
    void Init();
    void DeInit();

private:
    IImsAos* m_piImsAos;
    AString m_strAppId;
    IMS_UINT32 m_nSlotId;
    ICoreService* m_piCoreService;
    IMtsServiceListener* m_piMtsServiceListener;
    JniMtsService* m_pJniMtsService;
    MtsDynamicLoader* m_pMtsDynamicLoader;
};

#endif
