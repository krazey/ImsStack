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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "AosCounter.h"
#include "AString.h"
#include "GeolocationHelper.h"
#include "ImsMap.h"
#include "CarrierConfig.h"
#include "Credential.h"
#include "IImsRadio.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "MockIPhoneInfoLocation.h"
#include "MockIThread.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestPhoneInfoService.h"
#include "TestThreadService.h"
#include "TestTimerService.h"
#include "TestUtilService.h"

#include "../../../config/interface/ImsServiceConfig.h"
#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISipConfigV.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"
#include "../../../engine/interface/sipcore/MockISipMessageBodyPart.h"
#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"

#include "../../../enabler/interface/aos/AosAppRequestType.h"
#include "../../../enabler/interface/aos/ImsAosParameter.h"

#include "../../interface/aos/MockIAosService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistrationListener.h"
#include "interface/MockIAosRetryRepository.h"
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosTransaction.h"
#include "registration/MockAosIpsecHelper.h"
#include "registration/MockAosSubscription.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosString.h"
#include "registration/AosIpsecHelper.h"
#include "registration/AosRegistration.h"
#include "registration/AosSubscription.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

#define DECLARE_USING(Base)                                       \
    using Base::AddAccesstypeFeatureTag;                          \
    using Base::AddFeatureTagForMtc;                              \
    using Base::AddLocationHeaderBody;                            \
    using Base::AddSpecificOperation;                             \
    using Base::Block_Changed;                                    \
    using Base::CallTracker_StateChanged;                         \
    using Base::CheckPending;                                     \
    using Base::CleanUp;                                          \
    using Base::ClearIpsecBlock;                                  \
    using Base::ClearPcscf;                                       \
    using Base::ClearRetryCount;                                  \
    using Base::ClearRetryTimers;                                 \
    using Base::ClearTimers;                                      \
    using Base::CreateSubscription;                               \
    using Base::DestroyRegistration;                              \
    using Base::DestroySubscription;                              \
    using Base::GetActualWaitTime;                                \
    using Base::GetNetworkTypeForImsRegState;                     \
    using Base::GetPdnReactivateWaitTime;                         \
    using Base::GetReasonCode;                                    \
    using Base::IncreaseConsecutiveFailCount;                     \
    using Base::Init;                                             \
    using Base::IsAppReady;                                       \
    using Base::IsBlocked;                                        \
    using Base::IsGeolocationInfoRequired;                        \
    using Base::IsImsCall;                                        \
    using Base::IsIpsecSupported;                                 \
    using Base::IsReregFailureReportOnIpcanChangeRequired;        \
    using Base::IsRetryOnSamePcscfRequired;                       \
    using Base::IsTransactionStarted;                             \
    using Base::MessageMediator_AdjustMessage;                    \
    using Base::NConfiguration_NotifyConfigChanged;               \
    using Base::NetTracker_StatusChanged;                         \
    using Base::NotifyFailureWithImsReason;                       \
    using Base::OnMessage;                                        \
    using Base::ProcessForbiddenFailed;                           \
    using Base::ProcessIpcanChanged;                              \
    using Base::ProcessIpVersionChange;                           \
    using Base::ProcessPendingPlmnBlockOnUpdateFailure;           \
    using Base::ProcessPendingTransaction;                        \
    using Base::ProcessPlmnBlockWithPcoLimitedModeOnStartFailure; \
    using Base::ProcessRegEventChange;                            \
    using Base::ProcessRegRequiredWithNextPcscf;                  \
    using Base::ProcessRegRequiredWithWaitTime;                   \
    using Base::ProcessRequiredWfcErrMessage;                     \
    using Base::ProcessReregister;                                \
    using Base::ProcessRetryInRegStopped;                         \
    using Base::ProcessStartFailed_305;                           \
    using Base::ProcessStartFailed_Others;                        \
    using Base::ProcessSubReinitiate;                             \
    using Base::ProcessSubscriberFailed;                          \
    using Base::ProcessUpdateFailed_Others;                       \
    using Base::Registration_AuthenticationChallenged;            \
    using Base::Registration_NotifyAkaResponse;                   \
    using Base::Registration_RefreshTimerExpired;                 \
    using Base::Registration_Removed;                             \
    using Base::Registration_Started;                             \
    using Base::Registration_StartFailed;                         \
    using Base::Registration_Terminated;                          \
    using Base::Registration_Updated;                             \
    using Base::Registration_UpdateFailed;                        \
    using Base::RemoveFeatureTagForMtc;                           \
    using Base::SendRegisterEx;                                   \
    using Base::SetBlocked;                                       \
    using Base::SetDynamicIpQos;                                  \
    using Base::SetFakeReg;                                       \
    using Base::SetHeldByCall;                                    \
    using Base::SetImsCall;                                       \
    using Base::SetMode;                                          \
    using Base::SetNextPcscf;                                     \
    using Base::SetPcscf;                                         \
    using Base::SetRadioWaiting;                                  \
    using Base::SetReregFailureReportOnIpcanChangeRequired;       \
    using Base::SetState;                                         \
    using Base::SetStaticIpQos;                                   \
    using Base::SetTraffic;                                       \
    using Base::SetTrafficListener;                               \
    using Base::SetTrafficPriorityBlocked;                        \
    using Base::StartSubscription;                                \
    using Base::StartTimer;                                       \
    using Base::StopSubscription;                                 \
    using Base::StopTimer;                                        \
    using Base::Subscription_CanBeTransmitted;                    \
    using Base::Subscription_NotifyReceived;                      \
    using Base::Subscription_Request;                             \
    using Base::Subscription_StateChanged;                        \
    using Base::Timer_TimerExpired;                               \
    using Base::Transaction_OnConnectionFailed;                   \
    using Base::Transaction_OnTrafficPriorityChanged;             \
    using Base::TryNextPcscf;                                     \
    using Base::UpdateFeatureTag;                                 \
    using Base::UpdateIpsecSupported;                             \
    using Base::UpdatePreloadedRoute;                             \
    using Base::UpdateRegIpcanCategory;                           \
    using Base::UpdateTransactionStarted;

const IMS_SINT32 SLOT_ID = 0;

class TestAosRegistration : public AosRegistration
{
public:
    DECLARE_USING(AosRegistration)

    explicit inline TestAosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosRegistration(piAppContext, strRegId),
            m_piRegistrationInstance(IMS_NULL),
            m_pSubscriptionInstance(IMS_NULL),
            m_pIpsecHelperInstance(IMS_NULL)
    {
        m_pCounter = new AosCounter();
    }
    inline ~TestAosRegistration() override { delete m_pCounter; }

    inline TestAosRegistration(IN const TestAosRegistration&) = delete;
    inline TestAosRegistration& operator=(IN const TestAosRegistration&) = delete;

    inline IRegistration* GetRegistration() override { return m_piRegistrationInstance; }
    inline AosSubscription* GetSubscription(IN IRegSubscription*) override
    {
        return m_pSubscriptionInstance;
    }
    inline void CreateIpsecHelper() override { m_pIpsecHelper = m_pIpsecHelperInstance; }
    inline void DestroyIpsecHelper() override { m_pIpsecHelper = IMS_NULL; }

    inline void UpdateRegInstances(IN IRegistration* piReg, IN AosSubscription* pSubs,
            IN IRegContact* piContact, IN IRegParameter* piParam, IN AosIpsecHelper* pIpsec)
    {
        m_piRegistrationInstance = piReg;
        m_pSubscriptionInstance = pSubs;
        m_pIpsecHelperInstance = pIpsec;
        m_piRegistration = piReg;
        m_piRegContact = piContact;
        m_piRegParameter = piParam;
        m_pSubscription = pSubs;
    }
    inline AosUtil* GetUtil() { return m_pUtil; }
    inline void SetFeature(IN IMS_UINT32 nFeature) { m_nFeature = nFeature; }
    inline IMS_BOOL IsFeatureOn(IN IMS_UINT32 nFeature) { return (m_nFeature & nFeature); }
    inline void SetTxnPending(IN IMS_UINT32 nFeature) { m_nTxnPending = nFeature; }
    inline IMS_BOOL IsTxnPendingOn(IN IMS_UINT32 nFeature) { return (m_nTxnPending & nFeature); }
    inline void SetTransactionStarted(IN IMS_BOOL bStarted) { m_bIsTransactionStarted = bStarted; }
    inline void SetRegType(IN AosRegistrationType objRegType) { m_eRegType = objRegType; }
    inline IpAddress GetIpAddress() { return m_objIpa; }
    inline void SetPcscfString(IN AString strPcscf) { m_strPcscf = strPcscf; }
    inline void SetPuid(IN AString strPuid) { m_strPuid = strPuid; }
    inline AString GetPcscfString() { return m_strPcscf; }
    inline IMS_UINT32 GetPcscfPort() { return m_nPcscfPort; }
    inline IMS_UINT32 GetRetryBaseTime() { return m_nRetryBaseTime; }
    inline IMS_UINT32 GetRetryMaxTime() { return m_nRetryMaxTime; }
    inline void SetConsecutiveFailureCount(IN IMS_UINT32 nCount) { m_nConsecutiveFailure = nCount; }
    inline IMS_UINT32 GetConsecutiveFailureCount() { return m_nConsecutiveFailure; }
    inline IMS_UINT32 GetConsecutiveFailureCountForPdn()
    {
        return m_nConsecutiveFailureForPdnReactivated;
    }
    ITimer* GetTimer(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosRegistration::TIMER_OFFLINE_RECOVER:
                return m_piOfflineRecoverTimer;
            case AosRegistration::TIMER_STOP_RETRY:
                return m_piStopRetryTimer;
            case AosRegistration::TIMER_REFRESH:
                return m_piRefreshTimer;
            case AosRegistration::TIMER_EXPIRED:
                return m_piExpiredTimer;
            case AosRegistration::TIMER_MODE:
                return m_piModeTimer;
            case AosRegistration::TIMER_TRANSACTION:
                return m_piTransactionTimer;
            case AosRegistration::TIMER_INTERNAL_ERROR:
                return m_piInternalErrorTimer;
            default:
                return IMS_NULL;
        }
    }
    inline IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) { return (GetTimer(nType) != IMS_NULL); }
    inline void SetMaxRetryCountForAuthentication()
    {
        m_nAuthChallengeCount = AosRegistration::AUTHENTICATION_RETRY_MAX_COUNT;
    }
    inline IMS_UINT32 GetAuthChallengeCount() { return m_nAuthChallengeCount; }
    inline void SetMaxErrorCountForServerSocket()
    {
        m_nErrorCountForServerSocket = AosRegistration::RECONNECT_SERVER_SOCKET_ERROR_MAX_COUNT;
    }
    inline IMS_UINT32 GetMaxErrorCountForServerSocket() { return m_nErrorCountForServerSocket; }
    inline void SetCallingNumberVerificationSupported(IMS_BOOL bSurpported)
    {
        m_bCallingNumberVerificationSupported = bSurpported;
    }
    inline IMS_UINT32 GetNetworkBindingFeatures() { return m_nNetworkBindingFeatures; }
    inline void SetEps5GsOnly(IMS_BOOL bEps5GsOnly) { m_bEps5GsOnly = bEps5GsOnly; }
    inline IMS_BOOL GetEps5GsOnly() { return m_bEps5GsOnly; }
    inline void SetIpsecBlockReason(IMS_UINT32 nReason) { m_nIpsecBlockReason = nReason; }
    inline IMS_UINT32 GetIpsecBlockReason() { return m_nIpsecBlockReason; }
    inline void SetImsRegState(IMS_UINT32 nState) { m_nImsRegState = nState; }
    inline IMS_UINT32 GetImsRegState() { return m_nImsRegState; }
    inline void SetImsReasonCode(AosReasonCode eCode) { m_eImsReasonCode = eCode; }
    inline IMS_UINT32 GetDefaultWaitTimeForConnectionFailure()
    {
        return AosRegistration::CONNECTION_FAILURE_RETRY_DEFAULT_WAIT_TIME;
    }

    IMS_UINT32 GetInvokedCount(IN const AString strName) { return m_pCounter->GetCount(strName); }
    IMS_BOOL CheckRadioReadyAndSetTxnPending() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        return AosRegistration::CheckRadioReadyAndSetTxnPending();
    }
    void Update(IN IMS_BOOL bIgnoreRetryTimer, IN IMS_BOOL bExplicitUpdate) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::Update(bIgnoreRetryTimer, bExplicitUpdate);
    }
    void ProcessReinitiate(IN IMS_BOOL bClearPcscf) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessReinitiate(bClearPcscf);
    }
    void ProcessRegTerminated() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessRegTerminated();
    }
    void ProcessAuthenticationFailed() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessAuthenticationFailed();
    }
    void ProcessRegRequiredWithAvailableNextPcscf(
            IN IMS_BOOL bSetCurrentPcscfInvalid, IN IMS_UINT32 nReconnectTime) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessRegRequiredWithAvailableNextPcscf(
                bSetCurrentPcscfInvalid, nReconnectTime);
    }
    void ProcessIpsecFallback(IN IMS_BOOL bIsIpsecRetry) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessIpsecFallback(bIsIpsecRetry);
    }
    void ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessDefaultFlowRecovery_Start(nStatusCode);
    }
    void ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(IN IMS_UINT32 nRetryAfter) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(nRetryAfter);
    }
    void ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(
            IN IMS_UINT32 nRetryAfter) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(nRetryAfter);
    }
    void ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessDefaultFlowRecovery_Update(nStatusCode);
    }
    void ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            IN IMS_SINT32 nStatusCode, IN IMS_UINT32 nRetryAfter) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
                nStatusCode, nRetryAfter);
    }
    void ProcessStandardPcscfSelection(IN IMS_UINT32 nRetryAfter) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessStandardPcscfSelection(nRetryAfter);
    }
    void ProcessRequiredWfcErrMessage_403() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessRequiredWfcErrMessage_403();
    }
    void ProcessRequiredWfcErrMessage_500() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessRequiredWfcErrMessage_500();
    }
    void ProcessRequiredWfcErrMessage_Others() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessRequiredWfcErrMessage_Others();
    }
    void ProcessStartFailed_StatusCode(IN IMS_SINT32 nReason) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessStartFailed_StatusCode(nReason);
    }
    void ProcessStartFailed_TxnTimeout() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessStartFailed_TxnTimeout();
    }
    void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosRegistration::ProcessUpdateFailed_StatusCode(nStatusCode);
    }

private:
    AosCounter* m_pCounter;
    IRegistration* m_piRegistrationInstance;
    AosSubscription* m_pSubscriptionInstance;
    AosIpsecHelper* m_pIpsecHelperInstance;
};

MATCHER_P(IsSameMsg, message, "")
{
    return arg.nMSG == message;
}

class AosRegistrationTest : public ::testing::Test
{
public:
    AosRegistrationTest() :
            m_pAosRegistration(IMS_NULL)
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

        m_objPhoneInfoService.SetLocationInfo(&m_objMockILocationInfo);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        m_objTimerService.SetTimer(&m_objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);

        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker(SLOT_ID);
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker, SLOT_ID);
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);
        m_piAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(&m_objMockIAosRetryRepository, SLOT_ID);
        m_piAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(&m_objMockIAosService, SLOT_ID);
        m_piAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(&m_objMockIAosTransaction, SLOT_ID);

        m_objHandles.Add(m_strServiceId, &m_objMockIAosHandle);
        m_objAvailableImpus.AddElement(AString("sip:1111@ims.co.kr"));
        m_objAvailableImpus.AddElement(AString("sip:2222@ims.co.kr"));
        m_objPcscfs.AddElement(AString("192.168.0.100"));
        m_objPcscfs.AddElement(AString("192.168.0.101"));
        m_objPcscfs.AddElement(AString("192.168.0.102"));
        m_objUris.AddElement(AString("TestUri"));
        m_objPcscfPorts.Append(5060);
        m_objPcscfPorts.Append(5061);
        m_objPcscfPorts.Append(5062);
    }
    virtual ~AosRegistrationTest()
    {
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_piAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker, SLOT_ID);

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }

    TestAosRegistration* m_pAosRegistration;
    TestPhoneInfoService m_objPhoneInfoService;
    TestThreadService m_objThreadService;
    TestTimerService m_objTimerService;
    TestUtilService m_objUtilService;
    AosStaticProfile* m_pAosStaticProfile;
    IAosCallTracker* m_piAosCallTracker;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosRetryRepository* m_piAosRetryRepository;
    IAosService* m_piAosService;
    IAosTransaction* m_piAosTransaction;

    MockAosIpsecHelper m_objMockAosIpsecHelper;
    MockAosSubscription m_objMockAosSubscription;
    MockISipConfigV m_objMockISipConfigV;
    MockIConfigurable m_objMockIConfigurable;
    MockISipMessage m_objMockISipMessage;
    MockILocationInfo m_objMockILocationInfo;
    MockILocationProperties m_objMockILocationProperties;
    MockIRegistration m_objMockIRegistration;
    MockIRegContact m_objMockIRegContact;
    MockIRegParameter m_objMockIRegParameter;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosCallTracker m_objMockIAosCallTracker;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosService m_objMockIAosService;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistrationListener m_objMockIAosRegistrationListener;
    MockIAosRetryRepository m_objMockIAosRetryRepository;
    MockIAosTransaction m_objMockIAosTransaction;
    MockIRegSubscription m_objMockIRegSubscription;
    MockIThread m_objMockThread;
    MockITimer m_objMockITimer;

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    AString m_strLocationProperties = AString("LocationProperties");
    SipAddress m_objSipAddress = SipAddress("sip:1111@1.1.1.1");
    ImsMap<AString, IAosHandle*> m_objHandles;
    AStringArray m_objEmptyImpus;
    AStringArray m_objAvailableImpus;
    AStringArray m_objPcscfs;
    AStringArray m_objUris;
    ImsList<IMS_SINT32> m_objPcscfPorts;
    ImsVector<IMS_SINT32> m_objEmptyMaxCount;
    ImsVector<IMS_SINT32> m_objEmptyErrCode;
    ImsVector<IMS_SINT32> m_objEmptyRegErrWaitTime;
    ImsVector<IMS_SINT32> m_objEmptyRegRetryInterval;
    AosFeatureTagList m_objEmptyFeatureTagList;

protected:
    virtual void SetUp() override
    {
        // IAosAppContext
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));
        ON_CALL(m_objMockIAosAppContext, GetSubscriber())
                .WillByDefault(Return(&m_objMockIAosSubscriber));
        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(Return(&m_objMockIAosBlock));
        ON_CALL(m_objMockIAosAppContext, GetHandles()).WillByDefault(ReturnRef(m_objHandles));
        ON_CALL(m_objMockIAosAppContext, GetPcscf()).WillByDefault(Return(&m_objMockIAosPcscf));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetNetTracker())
                .WillByDefault(Return(&m_objMockIAosNetTracker));

        // IRegistration
        ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(m_objMockIRegistration, Deregister()).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(m_objMockIRegistration, GetAssociatedUris()).WillByDefault(ReturnRef(m_objUris));
        ON_CALL(m_objMockIRegistration, GetPreferredContact())
                .WillByDefault(Return(&m_objMockIRegContact));
        ON_CALL(m_objMockIRegistration, CreateContact(_, _, _, _))
                .WillByDefault(Return(&m_objMockIRegContact));
        ON_CALL(m_objMockIRegistration, GetParameter())
                .WillByDefault(Return(&m_objMockIRegParameter));
        ON_CALL(m_objMockIRegistration, GetPreviousRequest())
                .WillByDefault(Return(&m_objMockISipMessage));
        ON_CALL(m_objMockIRegistration, GetPreviousResponse())
                .WillByDefault(Return(&m_objMockISipMessage));

        // IAosNConfiguration
        ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
                .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
        ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrWaitTime())
                .WillByDefault(ReturnRef(m_objEmptyRegErrWaitTime));
        ON_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp()).WillByDefault(Return(46));
        ON_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
                .WillByDefault(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
                .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithRetryAfterTime())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
                .WillByDefault(
                        Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationPrivateHeader())
                .WillByDefault(Return(CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
                .WillByDefault(Return(30000));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
                .WillByDefault(Return(1800000));
        ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
                .WillByDefault(ReturnRef(m_objEmptyMaxCount));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
                .WillByDefault(
                        Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
                .WillByDefault(ReturnRef(m_objEmptyRegRetryInterval));
        ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForImsPdnReactivation())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForInitRegWithAvailablePcscf())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));
        ON_CALL(m_objMockIAosNConfiguration, GetSipMessageThresholdForTransportChange())
                .WillByDefault(Return(340));
        ON_CALL(m_objMockIAosNConfiguration, GetSipPreferredTransport())
                .WillByDefault(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP));
        ON_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
                .WillByDefault(Return(CarrierConfig::Assets::USAT_REG_EVENT_NOT_DOWNLOAD));
        ON_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsContactUriValidationChecked())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsIpsecInitializedWithNewPcscf())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsUserInfoInContactSupported())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_FALSE));

        // IAosSubscriber
        ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
                .WillByDefault(ReturnRef(m_objAvailableImpus));
        ON_CALL(m_objMockIAosSubscriber, GetFakeImpus())
                .WillByDefault(ReturnRef(m_objAvailableImpus));

        // IAosHandle
        ON_CALL(m_objMockIAosHandle, GetAppId()).WillByDefault(ReturnRef(m_strAppId));
        ON_CALL(m_objMockIAosHandle, GetServiceId()).WillByDefault(ReturnRef(m_strServiceId));
        ON_CALL(m_objMockIAosHandle, GetRequestType()).WillByDefault(Return(IAosHandle::ATTACH));
        ON_CALL(m_objMockIAosHandle, GetFeatureTagList())
                .WillByDefault(ReturnRef(m_objEmptyFeatureTagList));
        ON_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
                .WillByDefault(ReturnRef(m_objEmptyFeatureTagList));
        ON_CALL(m_objMockIAosHandle, GetServiceType())
                .WillByDefault(Return(static_cast<IMS_UINT32>(ImsAosService::MTS)));
        ON_CALL(m_objMockIAosHandle, IsRegBinded()).WillByDefault(Return(IMS_TRUE));

        // IAosPcscf
        ON_CALL(m_objMockIAosPcscf, GetCurrentIndex()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosPcscf, HasPcscf(_)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosPcscf, GetPcscfs()).WillByDefault(ReturnRef(m_objPcscfs));
        ON_CALL(m_objMockIAosPcscf, GetPcscfsPorts()).WillByDefault(ReturnRef(m_objPcscfPorts));

        // IAosConnection
        ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
                .WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
        ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosConnection, GetMtu()).WillByDefault(Return(1400));
        ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
                .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

        // IAosNetTracker
        ON_CALL(m_objMockIAosNetTracker, GetNetworkType())
                .WillByDefault(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

        // IRegContact
        ON_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIRegContact, GetContactAddress())
                .WillByDefault(ReturnRef(m_objSipAddress));

        // IAosTransaction
        ON_CALL(m_objMockIAosTransaction, StartTraffic(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillByDefault(Return(IMS_TRUE));

        // ILocationInfo and ILocationProperties
        ON_CALL(m_objMockILocationInfo, GetLocationProperties(_))
                .WillByDefault(Return(&m_objMockILocationProperties));
        ON_CALL(m_objMockILocationProperties, GetLatitude())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetLongitude())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetRadius())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetShape())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetConfidence())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetCurrentTime())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetMethod())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetCountry())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetState())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetCity())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetPostal())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetAltitude())
                .WillByDefault(ReturnRef(m_strLocationProperties));
        ON_CALL(m_objMockILocationProperties, GetVerticalAccuracy())
                .WillByDefault(ReturnRef(m_strLocationProperties));

        // IRegParameter and ISipMessage
        ON_CALL(m_objMockIRegParameter, AddPreloadedRoute(_, _, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockISipMessage, GetHeader(_, _, _)).WillByDefault(Return(AString("regtest")));

        // ISipConfigV and IConfigurable
        ON_CALL(m_objMockISipConfigV, GetConfigurable())
                .WillByDefault(Return(&m_objMockIConfigurable));
        ON_CALL(m_objMockIConfigurable, AddListener(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(_, _)).WillByDefault(Return(IMS_TRUE));

        m_pAosRegistration = new TestAosRegistration(
                &m_objMockIAosAppContext, m_pAosStaticProfile->GetRegistrationId());

        m_pAosRegistration->UpdateRegInstances(&m_objMockIRegistration, &m_objMockAosSubscription,
                &m_objMockIRegContact, &m_objMockIRegParameter, &m_objMockAosIpsecHelper);
        m_pAosRegistration->SetListener(&m_objMockIAosRegistrationListener);
    }

    virtual void TearDown() override
    {
        if (m_pAosRegistration)
        {
            m_pAosRegistration->ClearTimers();
            m_pAosRegistration->StopTimer(AosRegistration::TIMER_OFFLINE_RECOVER);

            delete m_pAosRegistration;
            m_pAosRegistration = IMS_NULL;
        }
    }
};

TEST_F(AosRegistrationTest, DeferStartIfTransactionNotStarted)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);

    m_pAosRegistration->Start();

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_START));
}

TEST_F(AosRegistrationTest, DeferStartIfTransactionNotAllowed)
{
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->Start();

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
}

TEST_F(AosRegistrationTest, ReportFailureIfRegistrationFailsDuringStart)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(m_objEmptyImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Start();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, SetRegisteringStateIfSendingRegisterSuccessfullyDuringStart)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Start();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportImmediateSuccessIfNotRegisteredOnStop)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, ReportImmediateSuccessIfDeregistrationSendFailsDuringStop)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, SetDeregisteringStateIfDeregistrationSendSucceedsDuringStop)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_STOP));

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
}

TEST_F(AosRegistrationTest, IgnoreStopDuringDeregistration)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_DEREGISTERING);

    EXPECT_CALL(m_objMockIRegistration, Deregister()).Times(0);
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _)).Times(0);

    m_pAosRegistration->Stop();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
}

TEST_F(AosRegistrationTest, RetryRegistrationIfInRegstopStateOnUpdate)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReregisterIfInRegisteredStateOnUpdate)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, DeferUpdateIfInRegisteringState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, IgnoreUpdateIfInDeregisteringState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_DEREGISTERING);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->Update(IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, UpdateRegBindingIfInRegStopStateOnReconfig)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillRepeatedly(Return(IAosHandle::DETACH));
    EXPECT_CALL(m_objMockIRegContact, RemoveService(_, _));
    EXPECT_CALL(m_objMockIRegistration, DestroyBinding(_, _));
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities());

    m_pAosRegistration->Reconfig();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, ReregisterAfterUpdatingRegBindingIfInRegisteredStateOnReconfig)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillRepeatedly(Return(IAosHandle::ATTACH));
    AosFeatureTagList objFeatureTagList;
    objFeatureTagList.AddFeatureTag(FeatureTags::CDMALESS);
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
            .WillRepeatedly(ReturnRef(objFeatureTagList));
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities());

    m_pAosRegistration->Reconfig();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, DeferReconfigIfInRegisteringState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    m_pAosRegistration->Reconfig();

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, IgnoreReconfigIfInOfflineState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).Times(0);

    m_pAosRegistration->Reconfig();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, ResetIndexToFirstWhenRequestToInitPcscf)
{
    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex());

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_INIT_PCSCF, IAosRegistration::REASON_INIT_PCSCF_CLEAR);
}

TEST_F(AosRegistrationTest, SetRetryTimeWhenRequestToInitAwt)
{
    IMS_UINT32 nConfiguredBaseTime = 30000;
    IMS_UINT32 nConfiguredMaxTime = 1800000;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
            .WillOnce(Return(nConfiguredBaseTime));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
            .WillOnce(Return(nConfiguredMaxTime));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_INIT_AWT);

    EXPECT_EQ(m_pAosRegistration->GetRetryBaseTime(), nConfiguredBaseTime / 1000);
    EXPECT_EQ(m_pAosRegistration->GetRetryMaxTime(), nConfiguredMaxTime / 1000);
}

TEST_F(AosRegistrationTest, ResetConsecutiveFailureCountToZeroWhenRequestToClearRetryCount)

{
    m_pAosRegistration->IncreaseConsecutiveFailCount();

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT);

    EXPECT_EQ(m_pAosRegistration->GetConsecutiveFailureCount(), 0);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackWhenRequestToSetIpsecEnable)
{
    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_ENABLE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackWhenRequestToSetIpsecDisable)
{
    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_DISABLE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest, IgnoreIpsecSetRequestIfInvalidReason)
{
    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_INIT);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 0);
}

TEST_F(AosRegistrationTest, RecalculateCallerCapabilitiesWhenRequestToRefreshRegInfo)
{
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities());

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_REFRESH_REGINFO);
}

TEST_F(AosRegistrationTest, CreateRegBindingIfRegIsNotBindedWhenRequestToUpdateRegBinding)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillRepeatedly(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIRegistration, CreateBinding(_, _));
    EXPECT_CALL(m_objMockIAosHandle, SetRegBinded(IMS_TRUE));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_REG_BINDING);
}

TEST_F(AosRegistrationTest, DeferHandlingIpcanChangeIfCallExist)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsRegWithIpcanChangedDuringImsCallHeld())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE_HELD_BY_CALL));
}

TEST_F(AosRegistrationTest, TriggerUpdateWhenIpcanChangedRegardlessOfCallExistanceIfEmergencyType)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsRegWithIpcanChangedDuringImsCallHeld())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);

    // THEN
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE_HELD_BY_CALL));
    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("Update"), 1);
}

TEST_F(AosRegistrationTest, TriggerUpdateIfNoCallExistWhenRequestToHandleIpcanChange)
{
    m_pAosRegistration->SetImsCall(IMS_FALSE);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("Update"), 1);
}

TEST_F(AosRegistrationTest, UpdateBlockStatusWhenRequestToUpdateIpcan)
{
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_IPCAN);

    EXPECT_TRUE(m_pAosRegistration->IsBlocked());
}

TEST_F(AosRegistrationTest, SetCurrentPcscfInvalidForGivenTimeWhenRequestForScscfRestoration)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, 30));

    // WHEN
    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION, 30);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, SetCurrentPcscfInvalidWhenRequestForScscfRestorationWithoutTime)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_FALSE, 0));

    // WHEN
    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, DestroyRegistrationWhenRequestForScscfRestroration)
{
    // GIVEN
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_));
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL));

    // WHEN
    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);

    // THEN: Then GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, StartWithNextPcscfIfAvailableWhenRequestForScscfRestoration)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    // WHEN
    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);

    // THEN
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReconnectPdnIfNoAvailablePcscfWhenRequestForScscfRestoration)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    // WHEN
    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ClearErrorCountWhenRequestToClearServerSocketErrorCount)
{
    m_pAosRegistration->SetMaxErrorCountForServerSocket();

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT);

    EXPECT_EQ(m_pAosRegistration->GetMaxErrorCountForServerSocket(), 0);
}

TEST_F(AosRegistrationTest, UpdateDetailRegStateWhenRequestForUnavailableFeatureTag)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsRegState(AosRegistration::IMS_REG_STATE_REGISTERING);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG);

    EXPECT_EQ(m_pAosRegistration->GetImsRegState(), AosRegistration::IMS_REG_STATE_REGISTERED);
}

TEST_F(AosRegistrationTest, IncreaseCountWhenRequestToIncreaseFailureCountForPdnReactivation)
{
    IMS_UINT32 nCurrentCnt = m_pAosRegistration->GetConsecutiveFailureCountForPdn();
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED);

    EXPECT_EQ(m_pAosRegistration->GetConsecutiveFailureCountForPdn(), nCurrentCnt + 1);
}

TEST_F(AosRegistrationTest, UpdateSettingWhenRequestToSetEps5gsOnly)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_FALSE);

    m_pAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE);

    EXPECT_TRUE(m_pAosRegistration->GetEps5GsOnly());
}

TEST_F(AosRegistrationTest, IgnoreOnReceivingUninterestedRequest)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime()).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined()).Times(0);
    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy()).Times(0);

    m_pAosRegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);

    EXPECT_TRUE(m_pAosRegistration->GetEps5GsOnly());
}

TEST_F(AosRegistrationTest, RegistrationTypeIsUpdatedWhenSetRegType)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    AosRegistrationType nType = m_pAosRegistration->GetRegType();
    EXPECT_EQ(nType, AosRegistrationType::EMERGENCY);
}

TEST_F(AosRegistrationTest, IsTerminatedReturnsTrueIfTerminatedIsPending)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TERMINATED);

    IMS_BOOL bResult = m_pAosRegistration->IsTerminated();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, RegistrationModeIsUpdatedWhenSetMode)
{
    m_pAosRegistration->SetMode(IAosRegistration::MODE_NORMAL);
    EXPECT_EQ(m_pAosRegistration->GetMode(), IAosRegistration::MODE_NORMAL);

    m_pAosRegistration->SetMode(IAosRegistration::MODE_FAKE);
    EXPECT_EQ(m_pAosRegistration->GetMode(), IAosRegistration::MODE_FAKE);
}

TEST_F(AosRegistrationTest, ShouldUpdateLimitedModeToHandleWhenSetModeToLimited)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosHandle, Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD));

    // WHEN
    m_pAosRegistration->SetMode(IAosRegistration::MODE_LIMITED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldUpdateNormalModeToHandleWhenSetModeToNormal)
{
    // GIVEN
    EXPECT_CALL(
            m_objMockIAosHandle, Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE));

    // WHEN
    m_pAosRegistration->SetMode(IAosRegistration::MODE_NORMAL);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldUpdateNormalModeToHandleWhenSetModeToFake)
{
    // GIVEN
    EXPECT_CALL(
            m_objMockIAosHandle, Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE));

    // WHEN
    m_pAosRegistration->SetMode(IAosRegistration::MODE_FAKE);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, GetPropertyReturnsEachPropertyValue)
{
    IMS_UINT32 nValue;
    AString strValue;

    // PROPERTY_LOCAL_PORT
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_PORT, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->GetUtil()->GetLocalPort(SLOT_ID));

    // PROPERTY_LOCAL_ADDRESS
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), m_pAosRegistration->GetIpAddress().ToString().GetStr());

    // PROPERTY_PCSCF_PORT
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PCSCF_PORT, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->GetPcscfPort());

    // PROPERTY_PCSCF_ADDRESS
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PCSCF_ADDRESS, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), m_pAosRegistration->GetPcscfString().GetStr());

    // PROPERTY_ASSOCIATED_URI
    EXPECT_CALL(m_objMockIRegistration, GetAssociatedUris()).WillOnce(ReturnRef(m_objUris));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_ASSOCIATED_URI, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestUri");

    // PROPERTY_PATH
    AString strPath = AString("TestPath");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::PATH, _, _)).WillOnce(Return(strPath));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PATH, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestPath");

    // PROPERTY_SERVICE_ROUTE
    AString strServiceRoute = AString("TestServiceRoute");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::SERVICE_ROUTE, _, _))
            .WillOnce(Return(strServiceRoute));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_SERVICE_ROUTE, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestServiceRoute");

    // PROPERTY_LAST_PATH
    ImsList<AString> strPaths;
    strPaths.Append(AString("TestLastPath"));
    EXPECT_CALL(m_objMockISipMessage, GetHeaders(ISipHeader::PATH, _)).WillOnce(Return(strPaths));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_LAST_PATH, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestLastPath");

    // PROPERTY_SUPPORTED
    AString strSupported = AString("TestSupported");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::SUPPORTED, _, _))
            .WillOnce(Return(strSupported));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_SUPPORTED, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestSupported");

    // PROPERTY_PROTECTED - IpcanCategory is CATEGORY_MOBILE
    m_pAosRegistration->CreateIpsecHelper();
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_TRUE));
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AosRegProtectedType::REG_PROTECTED);

    // PROPERTY_PROTECTED - IpcanCategory is CATEGORY_WLAN
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillOnce(Return(IIpcan::CATEGORY_WLAN));
    m_pAosRegistration->UpdateRegIpcanCategory();
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AosRegProtectedType::REG_PROTECTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION - m_bCallingNumberVerificationSupported if false
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AosSupportability::NOT_SUPPORTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION - m_bCallingNumberVerificationSupported if true
    m_pAosRegistration->SetCallingNumberVerificationSupported(IMS_TRUE);
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AosSupportability::SUPPORTED);

    // PROPERTY_NETWORK_BINDING_FEATURES
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_NETWORK_BINDING_FEATURES, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->GetNetworkBindingFeatures());

    // PROPERTY_PDN_REACIVATE_WAIT_TIME
    m_pAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_PDN_REACIVATE_WAIT_TIME, nValue, strValue);
    EXPECT_EQ(nValue, 30);

    // PROPERTY_REG_FAILURE_COUNT
    m_pAosRegistration->GetProperty(IAosRegistration::PROPERTY_REG_FAILURE_COUNT, nValue, strValue);
    EXPECT_EQ(nValue, m_pAosRegistration->GetConsecutiveFailureCount());
}

TEST_F(AosRegistrationTest, CheckBool)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pAosRegistration->IsRegistered());

    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    EXPECT_TRUE(m_pAosRegistration->IsRefreshing());

    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 10000);
    EXPECT_TRUE(m_pAosRegistration->IsRetryTimer());
    m_pAosRegistration->StopTimer(AosRegistration::TIMER_STOP_RETRY);

    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_TRUE(m_pAosRegistration->IsRetryHeld());
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TERMINATED);
    EXPECT_TRUE(m_pAosRegistration->IsTerminated());
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_NONE);

    m_pAosRegistration->SetAppReady(IMS_TRUE);
    EXPECT_TRUE(m_pAosRegistration->IsAppReady());
    m_pAosRegistration->SetAppReady(IMS_FALSE);
    EXPECT_FALSE(m_pAosRegistration->IsAppReady());
}

TEST_F(AosRegistrationTest, IgnoreSetTrafficForInvalidRegType)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);

    IMS_BOOL bResult = m_pAosRegistration->SetTraffic(IMS_TRUE);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, IgnoreSetTrafficListenerForInvalidRegType)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);

    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _)).Times(0);

    m_pAosRegistration->SetTrafficListener(IMS_TRUE);
}

TEST_F(AosRegistrationTest, IgnoreSetTrafficListenerIfTransactionIsNull)
{
    AosProvider::GetInstance()->SetTransaction(IMS_NULL, SLOT_ID);

    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _)).Times(0);

    m_pAosRegistration->SetTrafficListener(IMS_TRUE);
}

TEST_F(AosRegistrationTest, IpsecIsNotSupportedWhenSetAsFakeRegistration)
{
    m_pAosRegistration->SetFakeReg(IMS_TRUE);

    IMS_BOOL bResult = m_pAosRegistration->IsIpsecSupported();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, IpsecIsNotSupportedWhenSetBlockReason)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->UpdateIpsecSupported(IMS_FALSE, AosRegistration::IPSEC_BLOCK_ERROR);

    IMS_BOOL bResult = m_pAosRegistration->IsIpsecSupported();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, IpsecIsNotSupportedWhenFeatureIsNotOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    IMS_BOOL bResult = m_pAosRegistration->IsIpsecSupported();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, IpsecIsSupportedWhenFeatureIsOnAndNoSetBlockReason)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->ClearIpsecBlock();

    IMS_BOOL bResult = m_pAosRegistration->IsIpsecSupported();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, IgnoreUpdateIpsecSupportedIfIpsecFeatureIsNotOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    m_pAosRegistration->UpdateIpsecSupported(IMS_TRUE, AosRegistration::IPSEC_BLOCK_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetIpsecBlockReason(), AosRegistration::IPSEC_BLOCK_NONE);
}

TEST_F(AosRegistrationTest, SucceedsAddIpsecBlockReasonWhenFeatureIsOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);

    m_pAosRegistration->UpdateIpsecSupported(IMS_FALSE, AosRegistration::IPSEC_BLOCK_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetIpsecBlockReason(), AosRegistration::IPSEC_BLOCK_ERROR);
}

TEST_F(AosRegistrationTest, SucceedsRemoveIpsecBlockReasonWhenFeatureIsOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->SetIpsecBlockReason(AosRegistration::IPSEC_BLOCK_ERROR);

    m_pAosRegistration->UpdateIpsecSupported(IMS_TRUE, AosRegistration::IPSEC_BLOCK_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetIpsecBlockReason(), AosRegistration::IPSEC_BLOCK_NONE);
}

TEST_F(AosRegistrationTest, GetNetworkTypeForImsRegStateAlwaysReturnsLteIfWifiTestIsOn)
{
    m_pAosRegistration->GetUtil()->SetWifiTest(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(0);

    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::LTE);
    m_pAosRegistration->GetUtil()->SetWifiTest(IMS_FALSE);
}

TEST_F(AosRegistrationTest, GetNetworkTypeForImsRegStateReturnsNetworkTypeGotFromAosNetTracker)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillOnce(Return(NW_REPORT_RADIO_WLAN))
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_NR))
            .WillOnce(Return(NW_REPORT_RADIO_WCDMA))
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::IWLAN);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::LTE);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::NR);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::UTRAN);
    EXPECT_EQ(m_pAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::NONE);
}

TEST_F(AosRegistrationTest, SetReasonToTimeoutWhenNotifyFailureWithImsReasonForNormalType)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::NORMAL);

    m_pAosRegistration->NotifyFailureWithImsReason(
            IRegistration::REASON_TRANSACTION_TIMEOUT, SipStatusCode::SC_INVALID);

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
}

TEST_F(AosRegistrationTest, DoNotSetReasonToTimeoutWhenNotifyFailureWithImsReasonForEmergencyType)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    m_pAosRegistration->NotifyFailureWithImsReason(
            IRegistration::REASON_TRANSACTION_TIMEOUT, SipStatusCode::SC_INVALID);

    EXPECT_NE(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
}

TEST_F(AosRegistrationTest, SetReasonToRegResp403WhenStatusCodeIs403)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::NORMAL);

    m_pAosRegistration->NotifyFailureWithImsReason(
            IRegistration::REASON_STATUS_CODE, SipStatusCode::SC_403);

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_403);
}

TEST_F(AosRegistrationTest, DoNotSetReasonToRegResp403WhenStatusCodeIsNot403)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::NORMAL);

    m_pAosRegistration->NotifyFailureWithImsReason(
            IRegistration::REASON_STATUS_CODE, SipStatusCode::SC_500);

    EXPECT_NE(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_403);
}

TEST_F(AosRegistrationTest, IgnoreUpdatePreloadedRouteIfRegParameterIsNull)
{
    m_pAosRegistration->Destroy();

    IMS_BOOL bResult = m_pAosRegistration->UpdatePreloadedRoute();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, ReplaceAllPreloadedRouteHeaderWhenUpdatePreloadedRoute)
{
    EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes());

    IMS_BOOL bResult = m_pAosRegistration->UpdatePreloadedRoute();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, RetryCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // TEST_F : IncreaseConsecutiveFailCount
    m_pAosRegistration->IncreaseConsecutiveFailCount();
    EXPECT_NE(m_pAosRegistration->GetConsecutiveFailureCount(), 0);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));

    // TEST_F : ClearRetryCount
    m_pAosRegistration->ClearRetryCount();
    EXPECT_NE(m_pAosRegistration->GetConsecutiveFailureCount(), 0);

    m_pAosRegistration->ClearRetryCount(IMS_TRUE);
    EXPECT_EQ(m_pAosRegistration->GetConsecutiveFailureCount(), 0);
}

TEST_F(AosRegistrationTest, AddSpecificOperationWhileInRoamingAddsIpsecBlockReason)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);

    EXPECT_CALL(m_objMockIRegContact, AddUriParameter(_, _));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSipOverIpsecInRoamingEnabled())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));

    m_pAosRegistration->AddSpecificOperation();

    EXPECT_EQ(m_pAosRegistration->GetIpsecBlockReason(), AosRegistration::IPSEC_BLOCK_ROAMING);
}

TEST_F(AosRegistrationTest, SetTcpTransportForEmergencyInRoaming)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIRegContact, AddUriParameter(_, _));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsERegWithOnlyTcpInRoaming())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSipOverIpsecInRoamingEnabled())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));
    EXPECT_CALL(m_objMockIRegParameter, SetTransportExt(Sip::TRANSPORT_EXT_TCP));

    m_pAosRegistration->AddSpecificOperation();
}

TEST_F(AosRegistrationTest, AddAccesstypeFeatureTagWithNumericalValue)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _));

    m_pAosRegistration->AddAccesstypeFeatureTag();
}

TEST_F(AosRegistrationTest, AddAccesstypeFeatureTagWithoutNumericalValue)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .WillOnce(Return(CarrierConfig::Ims::
                            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _));

    m_pAosRegistration->AddAccesstypeFeatureTag();
}

TEST_F(AosRegistrationTest, UpdateFeatureTagOptionsSucceedWhenAddFeatureTagForMtc)
{
    m_pAosRegistration->GetUtil()->SetISipConfigV(&m_objMockISipConfigV);

    EXPECT_CALL(m_objMockIConfigurable, Update(_, _)).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities());

    IMS_UINT32 nRegFeatures = ImsAosFeature::VIDEO | ImsAosFeature::TEXT;
    m_pAosRegistration->AddFeatureTagForMtc(nRegFeatures, IMS_FALSE);

    m_pAosRegistration->GetUtil()->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, UpdateFeatureTagOptionsFailWhenAddFeatureTagForMtc)
{
    m_pAosRegistration->GetUtil()->SetISipConfigV(&m_objMockISipConfigV);

    EXPECT_CALL(m_objMockIConfigurable, Update(_, _)).WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(0);

    IMS_UINT32 nRegFeatures = ImsAosFeature::VIDEO | ImsAosFeature::TEXT;
    m_pAosRegistration->AddFeatureTagForMtc(nRegFeatures, IMS_FALSE);

    m_pAosRegistration->GetUtil()->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, AddExtraCapabilityIfFinalFeatureTagWhenAddFeatureTagForMtc)
{
    m_pAosRegistration->GetUtil()->SetISipConfigV(&m_objMockISipConfigV);

    EXPECT_CALL(m_objMockIRegContact,
            AddExtraCapability(AString(AosString::STR_USSI_FEATURE), AString::ConstNull()));
    EXPECT_CALL(m_objMockIRegContact,
            AddExtraCapability(
                    AString(FeatureTags::CALL_COMPOSER_VIA_TELEPHONY), AString::ConstNull()));

    IMS_UINT32 nRegFeatures = ImsAosFeature::USSI | ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    m_pAosRegistration->AddFeatureTagForMtc(nRegFeatures, IMS_FALSE);

    m_pAosRegistration->GetUtil()->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, DoNotAddExtraCapabilityIfNotFinalFeatureTagWhenAddFeatureTagForMtc)
{
    m_pAosRegistration->GetUtil()->SetISipConfigV(&m_objMockISipConfigV);

    EXPECT_CALL(m_objMockIRegContact, AddExtraCapability(_, _)).Times(0);

    IMS_UINT32 nRegFeatures = ImsAosFeature::USSI | ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    m_pAosRegistration->AddFeatureTagForMtc(nRegFeatures, IMS_TRUE);

    m_pAosRegistration->GetUtil()->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, AddHeaderParameterWhenAddFeatureTagForMtc)
{
    m_pAosRegistration->GetUtil()->SetISipConfigV(&m_objMockISipConfigV);

    EXPECT_CALL(m_objMockIRegContact,
            AddHeaderParameter(AString(AosString::STR_VERSTAT_FEATURE), AString::ConstNull()));

    m_pAosRegistration->AddFeatureTagForMtc(ImsAosFeature::VERSTAT, IMS_TRUE);

    m_pAosRegistration->GetUtil()->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, UpdateFeatureTagOptionsWhenRemoveFeatureTagForMtc)
{
    m_pAosRegistration->GetUtil()->SetISipConfigV(&m_objMockISipConfigV);
    IMS_UINT32 nOldFeature = ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO |
            ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT;
    ON_CALL(m_objMockISipConfigV, GetFeatureTagOptions()).WillByDefault(Return(nOldFeature));

    IMS_UINT32 nRegFeatures = ImsAosFeature::VIDEO | ImsAosFeature::TEXT;
    IMS_BOOL bResult = m_pAosRegistration->RemoveFeatureTagForMtc(nRegFeatures);

    EXPECT_TRUE(bResult);
    m_pAosRegistration->GetUtil()->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, RemoveExtraCapabilityWhenRemoveFeatureTagForMtc)
{
    EXPECT_CALL(m_objMockIRegContact,
            RemoveExtraCapability(AString(AosString::STR_USSI_FEATURE), AString::ConstNull()));
    EXPECT_CALL(m_objMockIRegContact,
            RemoveExtraCapability(
                    AString(FeatureTags::CALL_COMPOSER_VIA_TELEPHONY), AString::ConstNull()));

    IMS_UINT32 nRegFeatures = ImsAosFeature::USSI | ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    m_pAosRegistration->RemoveFeatureTagForMtc(nRegFeatures);
}

TEST_F(AosRegistrationTest, RemoveHeaderParameterWhenRemoveFeatureTagForMtc)
{
    EXPECT_CALL(m_objMockIRegContact,
            RemoveHeaderParameter(AString(AosString::STR_VERSTAT_FEATURE), AString::ConstNull()));

    m_pAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::VERSTAT);
}

TEST_F(AosRegistrationTest, IgnoreUpdateFeatureTagIfSameWithBindedOne)
{
    AosFeatureTagList objFeatureTagList;
    AosFeatureTagList objBindedList;
    ON_CALL(m_objMockIAosHandle, GetFeatureTagList()).WillByDefault(ReturnRef(objFeatureTagList));
    ON_CALL(m_objMockIAosHandle, GetBindedFeatureTagList()).WillByDefault(ReturnRef(objBindedList));

    IMS_BOOL bResult = m_pAosRegistration->UpdateFeatureTag(&m_objMockIAosHandle);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, UpdateFeatureTagIfDifferentFromBindedOne)
{
    AosFeatureTagList objFeatureTagList;
    AosFeatureTagList objBindedList;
    objFeatureTagList.AddFeatureTag(FeatureTags::CDMALESS);
    objBindedList.AddFeatureTag(FeatureTags::VIDEO);
    ON_CALL(m_objMockIAosHandle, GetFeatureTagList()).WillByDefault(ReturnRef(objFeatureTagList));
    ON_CALL(m_objMockIAosHandle, GetBindedFeatureTagList()).WillByDefault(ReturnRef(objBindedList));

    IMS_BOOL bResult = m_pAosRegistration->UpdateFeatureTag(&m_objMockIAosHandle);

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, IgnoreSetStaticIpQosIfPreferredDscpIsNone)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

    m_pAosRegistration->SetStaticIpQos();
}

TEST_F(AosRegistrationTest, IgnoreSetStaticIpQosIfSignallingDscpIsZero)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp()).WillOnce(Return(0));

    m_pAosRegistration->SetStaticIpQos();
}

TEST_F(AosRegistrationTest, IgnoreSetStaticIpQosIfPreferredDscpIsDifferentFromConnection)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but ePDG is enabled
    m_pAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_WIFI but ePDG is not enabled
    m_pAosRegistration->SetStaticIpQos();
}

TEST_F(AosRegistrationTest, IgnoreSetDynamicIpQosIfPreferredDscpIsNone)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

    m_pAosRegistration->SetDynamicIpQos();
}

TEST_F(AosRegistrationTest, IgnoreSetDynamicIpQosIfSignallingDscpIsZero)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp()).WillOnce(Return(0));

    m_pAosRegistration->SetDynamicIpQos();
}

TEST_F(AosRegistrationTest, IgnoreSetDynamicIpQosIfPreferredDscpIsDifferentFromConnection)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but ePDG is enabled
    m_pAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_WIFI but ePDG is not enabled
    m_pAosRegistration->SetDynamicIpQos();
}

TEST_F(AosRegistrationTest, UpdateTransactionStartedSucceed)
{
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);
    m_pAosRegistration->SetBlocked(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .WillOnce(Return(IMS_TRUE));

    m_pAosRegistration->UpdateTransactionStarted();

    EXPECT_TRUE(m_pAosRegistration->IsTransactionStarted());
}

TEST_F(AosRegistrationTest, UpdateTransactionStatusWhenDestroyRegistrationWithNormalType)
{
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);
    m_pAosRegistration->SetRadioWaiting(IMS_TRUE);

    m_pAosRegistration->DestroyRegistration();

    EXPECT_TRUE(m_pAosRegistration->IsTransactionStarted());
}

TEST_F(AosRegistrationTest, SendRegisterExReturnsFalseIfRegistrationIsNull)
{
    m_pAosRegistration->Destroy();

    IMS_BOOL bResult = m_pAosRegistration->SendRegisterEx(1800, IMS_TRUE);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, SendRegisterExReturnsFalseIfRegisterFail)
{
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    IMS_BOOL bResult = m_pAosRegistration->SendRegisterEx(1800, IMS_FALSE);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, SendRegisterExReturnsTrueIfRegisterSucceed)
{
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    IMS_BOOL bResult = m_pAosRegistration->SendRegisterEx(1800, IMS_FALSE);

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, UseConfiguredValueIfPolicyIsSpecifiedIntervalWhenGetActualWaitTime)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    m_pAosRegistration->SetConsecutiveFailureCount(2);

    ImsVector<IMS_SINT32> objInterval;
    objInterval.Add(1000);
    objInterval.Add(2000);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .WillOnce(ReturnRef(objInterval));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRandomRetryIntervals())
            .WillOnce(ReturnRef(objInterval));

    m_pAosRegistration->GetActualWaitTime();
}

TEST_F(AosRegistrationTest, NotUseConfiguredValueIfPolicyIsRfcRuleWhenGetActualWaitTime)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals()).Times(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRandomRetryIntervals()).Times(0);

    m_pAosRegistration->GetActualWaitTime();
}

TEST_F(AosRegistrationTest, UseRegDefaultWaitTimeIfConfigured)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetRegDefaultWaitTime()).WillByDefault(Return(60));

    // WHEN & THEN
    EXPECT_EQ(m_pAosRegistration->GetActualWaitTime(), 60);
}

TEST_F(AosRegistrationTest, StartRetryTimerIfRetryAfterIsNotZeroWhenTryNextPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillRepeatedly(Return(AString("60")));

    m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, StopRegistrationIfSendingRegisterFailWhenTryNextPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));

    m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, TryRegistrationIfSendingRegisterSucceedWhenTryNextPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryForStartIfFailToSetNextPcscfWhenTryNextPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    m_pAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, StopRegistrationIfFailToSetNextPcscfWhenTryNextPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    m_pAosRegistration->TryNextPcscf(IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, UseCurrentPcscfIfRetryOnSamePcscfIsRequiredWhenSetNextPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillByDefault(Return(0));

    IMS_BOOL bResult = m_pAosRegistration->SetNextPcscf();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, RetryOnSamePcscfIsRequiredIfTriedCountIsBelowConfiguredLimitPerPcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(0));

    IMS_BOOL bResult = m_pAosRegistration->IsRetryOnSamePcscfRequired();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest,
        RetryOnSamePcscfIsRequiredIfTriedCountIsBelowConfiguredLimitForSinglePcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillOnce(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(0));

    IMS_BOOL bResult = m_pAosRegistration->IsRetryOnSamePcscfRequired();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, TriggerReinitiateOnReceivingRegReinitiateMessage)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    ImsMessage objMsg(AosRegistration::MSG_REG_REINITIATE, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, TriggerUpdateOnReceivingRegUpdateMessage)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_UPDATE);

    ImsMessage objMsg(AosRegistration::MSG_REG_UPDATE, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE));
}

TEST_F(AosRegistrationTest, TriggerReconfigOnReceivingRegReconfigMessage)
{
    IMS_UINT32 pending = AosRegistration::PENDING_RECONFIG | AosRegistration::PENDING_UPDATE;
    m_pAosRegistration->SetTxnPending(pending);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);

    ImsMessage objMsg(AosRegistration::MSG_REG_RECONFIG, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE));
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_RECONFIG));
}

TEST_F(AosRegistrationTest, StartOfflineRecoverTimerOnReceivingMessageThatRegRequiredWithWaitTime)
{
    IMS_SINT32 nWaitTime = 30;
    m_pAosRegistration->SetAppReady(IMS_TRUE);

    EXPECT_CALL(m_objMockITimer, SetTimer(nWaitTime * 1000, _));

    ImsMessage objMsg(AosRegistration::MSG_REG_REQUIRED_WITH_WAIT_TIME, nWaitTime, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, TryRegistrationOnReceivingMessageThatRegRequiredWithNextPcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillOnce(Return(0));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    ImsMessage objMsg(AosRegistration::MSG_REG_REQUIRED_WITH_NEXT_PCSCF, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, TryRegistrationOnReceivingMessageThatRegRequiredWithScscfRestoration)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    ImsMessage objMsg(AosRegistration::MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportFailureOnReceivingMessageThatRegTerminatedByNotify)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN));

    ImsMessage objMsg(AosRegistration::MSG_REG_TERMINATED_BY_NOTIFY, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, CreateSubscriptionOnReceivingSubReinitiateMessage)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    EXPECT_CALL(m_objMockAosSubscription, Destroy());
    EXPECT_CALL(m_objMockIRegistration, CreateSubscription(_))
            .WillOnce(Return(&m_objMockIRegSubscription));
    EXPECT_CALL(m_objMockAosSubscription, Start(_));

    ImsMessage objMsg(AosRegistration::MSG_SUB_REINITIATE, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, DestroySubscriptionOnReceivingSubTerminatedMessage)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    EXPECT_CALL(m_objMockAosSubscription, Destroy());

    ImsMessage objMsg(AosRegistration::MSG_SUB_TERMINATED, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, ClearRetryCountOnReceivingMessageThatNotifyRegistered)
{
    m_pAosRegistration->SetConsecutiveFailureCount(2);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_NOTIFY));

    ImsMessage objMsg(AosRegistration::MSG_REG_EVENT_REGISTERED, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);

    EXPECT_EQ(m_pAosRegistration->GetConsecutiveFailureCount(), 0);
}

TEST_F(AosRegistrationTest, IgnoreUninterestingMessage)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy()).Times(0);

    ImsMessage objMsg(AosRegistration::MSG_REG_START, 0, 0);
    m_pAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, InitializeFeaturesAndListenersOnInit)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, SetListener(_));
    EXPECT_CALL(m_objMockIAosCallTracker, SetListener(_));
    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _));

    m_pAosRegistration->Init();

    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(AosRegistration::FEATURE_SUBSCRIPTION));
    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(AosRegistration::FEATURE_IPSEC));
}

TEST_F(AosRegistrationTest, RemoveListenersOnCleanUp)
{
    EXPECT_CALL(m_objMockIAosBlock, RemoveListener(_));
    EXPECT_CALL(m_objMockIAosCallTracker, RemoveListener(_));
    EXPECT_CALL(m_objMockIAosTransaction, RemoveListener(_, _));

    m_pAosRegistration->CleanUp();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsSupportedForCellular)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsSupportedForWifi)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosRegistration->UpdateRegIpcanCategory();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForEmergencyTypeIfPidfIsSupportedForCellular)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForEmergencyTypeIfPidfIsSupportedForWifi)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosRegistration->UpdateRegIpcanCategory();
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsNotRequiredForFakeType)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).Times(0);

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsRequiredForNormalTypeIfPidfIsNotSupportedForCellular)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).Times(0);

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsNotRequiredIfIpsecHelperIsNull)
{
    m_pAosRegistration->DestroyIpsecHelper();
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, GeolocationInfoIsNotRequiredIfIpsecIsNotEstablished)
{
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_FALSE));

    IMS_BOOL bResult = m_pAosRegistration->IsGeolocationInfoRequired();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, SendRegReconfigMessageWhenCheckPendingIfPendingReconfigExist)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_RECONFIG);

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_REG_RECONFIG)));

    m_pAosRegistration->CheckPending();
}

TEST_F(AosRegistrationTest, SendRegUpdateMessageWhenCheckPendingIfPendingUpdateExist)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_UPDATE);

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_REG_UPDATE)));

    m_pAosRegistration->CheckPending();
}

TEST_F(AosRegistrationTest, AddTxnPendingFeatureWhenHandlePendingPlmnBlockOnUpdateFailure)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming()).WillOnce(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosRegistration->ProcessPendingPlmnBlockOnUpdateFailure();

    EXPECT_TRUE(bResult);
    EXPECT_TRUE(
            m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL));
}

TEST_F(AosRegistrationTest, ReportFailureWhenPlmnBlockWithPcoLimitedModeOnStartFailure)
{
    m_pAosRegistration->SetMode(IAosRegistration::MODE_LIMITED);

    EXPECT_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PCO_LIMITED_SERVICE));

    IMS_BOOL bResult = m_pAosRegistration->ProcessPlmnBlockWithPcoLimitedModeOnStartFailure();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, StartRegisterIfPendingStartExistWhenHandlePendingTransaction)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_START);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_START));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, UpdatesRegisterIfInRefreshStopStateWhenHandlePendingTransaction)
{
    IMS_UINT32 pending = AosRegistration::PENDING_TRAFFIC | AosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->SetTxnPending(pending);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, StartRegisterIfInOfflineStateWhenHandlePendingTransaction)
{
    IMS_UINT32 pending = AosRegistration::PENDING_TRAFFIC | AosRegistration::PENDING_TRANSACTION;
    m_pAosRegistration->SetTxnPending(pending);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, DoNothingIfRetryTimerExistWhenHandlePendingTransaction)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 10000);

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest,
        ReportFailureIfFailToRegisterWhenHandlePendingTransactionWhileRetryIsHeld)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest,
        TryRegistrationIfSucceedToSendRegisterWhenHandlePendingTransactionWhileRetryIsHeld)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToRegisterWhenHandlePendingTransactionInOfflineState)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RetryRegisterWhenHandlePendingTransactionInOfflineState)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest,
        RemoveTxnPendingFeatureWhenHandlePendingTransactionWhileRetryIsNotHeldAndNotInOfflineState)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, TriggerStartSubscriptionWhenHandlePendingSubscription)
{
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_SUBSCRIPTION);
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    EXPECT_CALL(m_objMockAosSubscription, Start(_));

    m_pAosRegistration->ProcessPendingTransaction();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_SUBSCRIPTION));
}

TEST_F(AosRegistrationTest, NotHandleRetryInRegStoppedIfRetryNotHeldAndIgnoreTimerIsTrue)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_TRUE);
}

TEST_F(AosRegistrationTest, NotHandleRetryInRegStoppedIfIgnoreTimerIsFalseAndRetryTimerExist)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 10000);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, AddTxnPendingFeatureIfTransactionNotStartedWhenRetryInRegStopped)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
}

TEST_F(AosRegistrationTest, AddTxnPendingFeatureIfTransactionNotAllowedWhenRetryInRegStopped)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToRegisterWhenRetryInRegStopped)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, IgnoreReregisterIfNotInRegisteredState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    m_pAosRegistration->ProcessReregister();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, StopReregisterIfTransactionIsNotStarted)
{
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pAosRegistration->ProcessReregister();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, StopReregisterIfRadioIsNotReady)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessReregister();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, StopReregisterIfRadioIsWaiting)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetRadioWaiting(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessReregister();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, StopReregisterIfFailToRegisterDueToCall)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));

    m_pAosRegistration->ProcessReregister();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToRegisterDuringReregister)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_FALSE);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessReregister();
}

TEST_F(AosRegistrationTest, TryingRegRefreshIfSucceedToSendRegisterDuringReregister)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE));

    m_pAosRegistration->ProcessReregister();
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToCreateRegistrationDuringReinitiate)
{
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(m_objEmptyImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessReinitiate(IMS_TRUE);
}

TEST_F(AosRegistrationTest, AddTxnPendingFeatureIfCallExistWhenRegTerminated)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _)).Times(0);

    m_pAosRegistration->ProcessRegTerminated();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, NotReportFailureIfRetryTimerExistWhenRegTerminated)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 10000);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _)).Times(0);

    m_pAosRegistration->ProcessRegTerminated();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryForStartWhenAuthenticationFailedWhileRegistering)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    m_pAosRegistration->ProcessAuthenticationFailed();

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryForUpdateWhenAuthenticationFailedWhileRefreshing)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessAuthenticationFailed();
}

TEST_F(AosRegistrationTest,
        ReportFailureWhenAuthenticationFailedIfExtraRegErrPolicyNotIncludePdnReactivated)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_AUTHENTICATION));

    m_pAosRegistration->ProcessAuthenticationFailed();
}

TEST_F(AosRegistrationTest, ShouldCheckUsimAuthHandlingNeededWhenProcessAuthenticationFailed)
{
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    EXPECT_CALL(m_objMockIAosSubscriber, IsUsim()).WillOnce(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_USIM_AUTHENTICATION);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillOnce(ReturnRef(objErrCode));

    m_pAosRegistration->ProcessAuthenticationFailed();
}

TEST_F(AosRegistrationTest, TriggerReinitiateWhenRegRequiredWithZeroWaitTime)
{
    m_pAosRegistration->ProcessRegRequiredWithWaitTime(0);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessReinitiate"), 1);
}

TEST_F(AosRegistrationTest, ReportFailureIfAppIsNotReadyWhenRegRequiredWithWaitTime)
{
    m_pAosRegistration->SetAppReady(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    m_pAosRegistration->ProcessRegRequiredWithWaitTime(10);
}

TEST_F(AosRegistrationTest, StartOfflineRecoverTimerWhenRegRequiredWithWaitTimeWhileInCall)
{
    m_pAosRegistration->SetAppReady(IMS_TRUE);
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    m_pAosRegistration->ProcessRegRequiredWithWaitTime(10);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToCreateRegistrationWhenRegRequiredWithNextPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(0));
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(1));

    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(m_objEmptyImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessRegRequiredWithNextPcscf();
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetNextPcscfWhenRegRequiredWithNextPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    m_pAosRegistration->ProcessRegRequiredWithNextPcscf();
}

TEST_F(AosRegistrationTest,
        StartOfflineRecoverTimerWithRetryAfterValueWhenRegRequiredWithAvailableNextPcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillOnce(Return(AString("60")));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, StartOfflineRecoverTimerWithAwtWhenRegRequiredWithAvailableNextPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest,
        ReportFailureIfFailToCreateRegistrationWhenRegRequiredWithAvailableNextPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(m_objEmptyImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);
}

TEST_F(AosRegistrationTest,
        ReportFailureWithAwtIfFailToSetNextPcscfWhenRegRequiredWithAvailableNextPcscf)
{
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 10);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetNextPcscfWhenRegRequiredWithAvailableNextPcscf)
{
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);
}

TEST_F(AosRegistrationTest, TryRegistrationWhenRegRequiredWithAvailableNextPcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_TRUE, 0);
}

TEST_F(AosRegistrationTest, ShouldNotCreateSubscriptionIfNotRegisteredWhenSubReinitiate)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, CreateSubscription(_)).Times(0);

    m_pAosRegistration->ProcessSubReinitiate();
}

TEST_F(AosRegistrationTest, IgnoreForbiddenFailedIfErrorCodeIsNotPermanentCode)
{
    ImsVector<IMS_SINT32> objErrCode;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillOnce(ReturnRef(objErrCode));

    IMS_BOOL bResult = m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, ReportFailureWhenForbiddenFailIfForbiddenCountIsGreaterThanMaxCount)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_4XX);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .WillByDefault(ReturnRef(objCount));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_CRITICAL));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN));

    m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryForStartWhenForbiddenFailWhileRegistering)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(403);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Add(5);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .WillByDefault(ReturnRef(objCount));

    m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryForUpdateWhenForbiddenFailWhileRefreshing)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(403);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Add(5);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .WillByDefault(ReturnRef(objCount));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, IgnoreSubscriberFailIfExtraRegErrorPolicyIsNotSubscriberFailed)
{
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    IMS_BOOL bResult = m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest,
        IgnoreSubscriberFailIfErrorCodeIsNotInExtraReregConfigWhileRegisteredState)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));

    ImsVector<IMS_SINT32> objReregErrCode;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .WillOnce(ReturnRef(objReregErrCode));

    IMS_BOOL bResult = m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, TriggerReinitiationIfConsecutiveFailureIsOneWhenSubscriberFailed)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pAosRegistration->SetConsecutiveFailureCount(1);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));

    ImsVector<IMS_SINT32> objReregErrCode;
    objReregErrCode.Add(403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .WillOnce(ReturnRef(objReregErrCode));
    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REINITIATE)));

    IMS_BOOL bResult = m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403);

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest,
        IgnoreSubscriberFailIfErrorCodeIsNotInExtraRegConfigWhileNotRegisteredState)
{
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));

    ImsVector<IMS_SINT32> objExtraRegErrCode;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillOnce(ReturnRef(objExtraRegErrCode));

    IMS_BOOL bResult = m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, StartRetryTimerIfSucceedToSetNextPcscfWhenSubscriberFailed)
{
    m_pAosRegistration->SetPuid(m_objAvailableImpus.GetElementAt(0));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));
    ImsVector<IMS_SINT32> objReregErrCode;
    objReregErrCode.Add(403);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objReregErrCode));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .WillOnce(ReturnRef(m_objAvailableImpus));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403);

    EXPECT_TRUE(bResult);
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, TriggerImsiBasedSubscriberUsingNextPuidWhenSubscriberFailed)
{
    m_pAosRegistration->SetPuid(m_objAvailableImpus.GetElementAt(0));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));
    ImsVector<IMS_SINT32> objReregErrCode;
    objReregErrCode.Add(403);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objReregErrCode));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .WillOnce(ReturnRef(m_objAvailableImpus));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIRegistration, SetAor(_, _));

    IMS_BOOL bResult = m_pAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403);

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToCreateRegistrationWhenIpsecFallback)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessIpsecFallback(IMS_FALSE);

    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, TryRegistrationIfSucceedToCreateRegistrationWhenIpsecFallback)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessIpsecFallback(IMS_FALSE);

    EXPECT_FALSE(m_pAosRegistration->IsIpsecSupported());
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest,
        TriggerFlowRecoveryForStartIfIpsecSupportingStatusIsNotChangedWhenIpsecFallback)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);

    m_pAosRegistration->ProcessIpsecFallback(IMS_TRUE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerStartWithEveryPcscfPolicyWhenFlowRecoveryForStart)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    ON_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Add(SipStatusCode::SC_300);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithRetryAfterTime())
            .WillByDefault(ReturnRef(objErrorCode));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(Return(AString("60")));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount(
                      "ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy"),
            1);
}

TEST_F(AosRegistrationTest, TriggerStartWithSpecifiedIntervalPolicyWhenFlowRecoveryForStart)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount(
                      "ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy"),
            1);
}

TEST_F(AosRegistrationTest, WaitRetryForNextPcscfIfFollowEachPcscfPolicyWhenFlowRecoveryForStart)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EACH_PCSCF));

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, WaitRetryForNextPcscfIfRetryAfterIsZeroWhenFlowRecoveryForStart)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_503);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest,
        TryRegistrationWithNextPcscfIfRetryAfterIsGreaterThanZeroWhenFlowRecoveryForStart)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(Return(AString("60")));

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetNextPcscfWhenFlowRecoveryForStart)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EACH_PCSCF));

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);
}

TEST_F(AosRegistrationTest, ReportFailureWithAwtIfTryingNextPcscfFailWhenFlowRecoveryForStart)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(Return(AString("60")));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    // WHEN
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ReportFailureWithAwtIfSetNextPcscfFailWhenFlowRecoveryForStart)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    // WHEN
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, SetPdnReactivateWaitTimeWithRetryAfterWhenFlowRecoveryForStart)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(Return(AString("60")));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // THEN
    EXPECT_EQ(m_pAosRegistration->GetPdnReactivateWaitTime(), 60);
}

TEST_F(AosRegistrationTest, SetPdnReactivateWaitTimeInAwtRangeWhenFlowRecoveryForStart)
{
    // GIVEN
    m_pAosRegistration->SetConsecutiveFailureCount(2);
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // THEN
    EXPECT_GE(m_pAosRegistration->GetPdnReactivateWaitTime(), 120);
    EXPECT_LE(m_pAosRegistration->GetPdnReactivateWaitTime(), 240);
}

TEST_F(AosRegistrationTest, BlockPcscfIfConfiguredWhenFlowRecoveryForStartWithoutRetryAfter)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockIAosNConfiguration, IsBlockPcscfOnRegFailure())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _));

    // WHEN
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, BlockPcscfIfConfiguredWhenFlowRecoveryForStartWithRetryAfter)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(Return(AString("60")));
    ON_CALL(m_objMockIAosNConfiguration, IsBlockPcscfOnRegFailure())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, 60));

    // WHEN
    m_pAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, WaitRetryForNextPcscfWhenStartWithEveryPcscfPolicy)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    IMS_UINT32 retryAfter = 10;
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(retryAfter);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSendRegisterWhenStartWithEveryPcscfPolicy)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    IMS_UINT32 retryAfter = 0;
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(retryAfter);
}

TEST_F(AosRegistrationTest, TryRegistrationWithNextPcscfWhenStartWithEveryPcscfPolicy)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    IMS_UINT32 retryAfter = 0;
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(retryAfter);
}

TEST_F(AosRegistrationTest, WaitRetryIfSucceedToSetFirstPcscfWhenStartWithEveryPcscfPolicy)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    IMS_UINT32 retryAfter = 0;
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(retryAfter);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetFirstPcscfWhenStartWithEveryPcscfPolicy)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    IMS_UINT32 retryAfter = 0;
    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(retryAfter);
}

TEST_F(AosRegistrationTest, StartRetryTimerIfCanIncreaseCountWhenStartWithNonZeroInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, StartRetryTimerIfCanIncreaseCountWhenStartWithZeroInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, WaitRetryForNextPcscfWhenStartWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetNextPcscfWhenStartWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);
}

TEST_F(AosRegistrationTest,
        WaitRetryForNextPcscfIfNotShareRetryCounterWhenStartWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, ResetPcscfTriedIfAllPcscfTriedWhenStartWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosPcscf, IsAllPcscfTried()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTime()).WillByDefault(Return(10));

    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTried());

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);
}

TEST_F(AosRegistrationTest,
        ReportFailureDueToNoPcscfIfRetryCounterIsNotSharedWhenStartWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    m_pAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);
}

TEST_F(AosRegistrationTest, TriggerOfflineRecoverWhenFlowRecoveryForUpdate)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    ON_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Add(SipStatusCode::SC_600);
    ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeWithRetryAfterTime())
            .WillByDefault(ReturnRef(objErrorCode));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(Return(AString("60")));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToCreateRegistrationWhenFlowRecoveryForUpdate)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    ON_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIRegistration, GetPreviousResponse()).WillByDefault(ReturnNull());
    ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .WillByDefault(ReturnRef(m_objEmptyImpus));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);
}

TEST_F(AosRegistrationTest, TryRegistrationIfSucceedToCreateRegistrationWhenFlowRecoveryForUpdate)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    ON_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIRegistration, GetPreviousResponse()).WillByDefault(ReturnNull());
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);
}

TEST_F(AosRegistrationTest, TriggerStartWithSpecifiedIntervalPolicyWhenFlowRecoveryForUpdate)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_300);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount(
                      "ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy"),
            1);
}

TEST_F(AosRegistrationTest, TriggerRegWithAvailableNextPcscfWhenFlowRecoveryForUpdate)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_300);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegRequiredWithAvailableNextPcscf"), 1);
}

TEST_F(AosRegistrationTest,
        TriggerRegWithAvailableNextPcscfForReregErrorCodeWhenUpdateWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Add(SipStatusCode::SC_600);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .WillByDefault(ReturnRef(objErrorCode));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegRequiredWithAvailableNextPcscf"), 1);
}

TEST_F(AosRegistrationTest, WaitRetryIfRetryCountCanBeIncreasedWhenUpdateWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest,
        TriggerRegWithAvailableNextPcscfIfRetryCountReachedMaxWhenUpdateWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_FALSE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegRequiredWithAvailableNextPcscf"), 1);
}

TEST_F(AosRegistrationTest, TriggerReinitiateIfStatusCodeIs481WhenUpdateWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_481, 0);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessReinitiate"), 1);
}

TEST_F(AosRegistrationTest, StartRetryTimerIfRegIsNotExpiredWhenUpdateWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(0));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest,
        TriggerOfflineRecoverIfNotFailedConsecutivelyWhenUpdateWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(1000));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 1000);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, ReconnectPdnIfFailToSetNextPcscfWhenUpdateWithSpecifiedInterval)
{
    m_pAosRegistration->SetConsecutiveFailureCount(2);
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    m_pAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);
}

TEST_F(AosRegistrationTest, DoNothingIfPolicyIsDefaultWhenStartFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));

    IMS_BOOL bResult = m_pAosRegistration->ProcessStartFailed_305();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, ShouldSetReasonCodeToNotSupportedCountryWhen403WithNotSupportedCountry)
{
    ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(), GetPersistent(_, _))
            .WillByDefault(Return(AString("NotSupportedCountry")));

    m_pAosRegistration->ProcessRequiredWfcErrMessage_403();

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(),
            AosReasonCode::WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY);
}

TEST_F(AosRegistrationTest, ShouldSetReasonCodeTo403WhenSupportedCountry)
{
    ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objUtilService.GetMockPrivateProperty(), GetPersistent(_, _))
            .WillByDefault(Return(m_strLocationProperties));

    m_pAosRegistration->ProcessRequiredWfcErrMessage_403();

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::WFC_REG_RESP_403);
}

TEST_F(AosRegistrationTest, ShouldSetReasonCodeTo500WhenWfcErrorMessageSupported)
{
    ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessRequiredWfcErrMessage_500();

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::WFC_REG_RESP_500);
}

TEST_F(AosRegistrationTest, ShouldSetReasonCodeToOtherFailuresWhenWfcErrorMessageSupported)
{
    ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessRequiredWfcErrMessage_Others();

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::WFC_REG_RESP_OTHER_FAILURES);
}

TEST_F(AosRegistrationTest, DoNotNotifySameReasonCodeWhenWfcErrMessageRequiredForOthers)
{
    m_pAosRegistration->SetImsReasonCode(AosReasonCode::WFC_REG_RESP_OTHER_FAILURES);
    ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _, _)).Times(0);

    m_pAosRegistration->ProcessRequiredWfcErrMessage_Others();
}

TEST_F(AosRegistrationTest,
        DoNotNotifyDeregisteredIfWfcErrorMessageNotSupportedWhenWfcErrMessageRequiredForOthers)
{
    ON_CALL(m_objMockIAosNConfiguration, IsWfcErrorMessageSupported(_))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _, _)).Times(0);

    m_pAosRegistration->ProcessRequiredWfcErrMessage_Others();
}

TEST_F(AosRegistrationTest,
        TriggerPcscfSelectionIfConfiguredToPcscfDiscoveryWhenStartFailedWithTxnTimeout)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillByDefault(Return(1));
    ImsVector<IMS_SINT32> objErrCodeForPcscfDiscovery;
    objErrCodeForPcscfDiscovery.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objErrCodeForPcscfDiscovery));
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessStandardPcscfSelection"), 1);
}

TEST_F(AosRegistrationTest, ReconnectPdnWhenStartFailedWithTxnTimeoutIfPdnReactivateIsRequired)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillOnce(ReturnRef(objExtraRegErrCode));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();
}

TEST_F(AosRegistrationTest,
        TriggerFlowRecoveryForStartIfRegErrPolicyIsPdnReactivatedWhenStartFailedWithTxnTimeout)
{
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest,
        TriggerIpsecFallbackIfConfiguredToAttemptWithoutIpsecWhenStartFailedWithTxnTimeout)
{
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .WillByDefault(ReturnRef(objErrWithoutIpsec));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest,
        TriggerFlowRecoveryForStartIfExtraRegErrorCodeIncludeTimerFWhenStartFailedWithTxnTimeout)
{
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objExtraRegErrCode));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TryNextPcscfWhenStartFailedWithTxnTimeout)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillByDefault(Return(1));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount());
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToTryNextPcscfWhenStartFailedWithTxnTimeout)
{
    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount());
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ShouldBlockPcscfIfConfiguredToBlockWhenStartFailedWithTxnTimeout)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsBlockPcscfOnRegFailure())
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _));

    // WHEN
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldNotBlockPcscfIfConfiguredToNotBlockWhenStartFailedWithTxnTimeout)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsBlockPcscfOnRegFailure())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _)).Times(0);

    // WHEN
    m_pAosRegistration->ProcessStartFailed_TxnTimeout();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ProcessIpVersionChangeReturnsFalseIfTargetIsNotMatchedWithLocalAddress)
{
    m_pAosRegistration->SetPcscfString("192.168.0.1");
    IpAddress objLocalIpv4Addr(AString("192.186.0.100"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objLocalIpv4Addr));

    IMS_BOOL bResult = m_pAosRegistration->ProcessIpVersionChange();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, ProcessIpVersionChangeReturnsFalseIfNoPcscfMatchedWithTarget)
{
    m_pAosRegistration->SetPcscfString("192.168.0.1");
    IpAddress objLocalIpv6Addr(AString("fc01:cafe::100"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objLocalIpv6Addr));
    AStringArray objEmptyPcscfs;
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objEmptyPcscfs));

    IMS_BOOL bResult = m_pAosRegistration->ProcessIpVersionChange();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, ProcessIpVersionChangeReturnsTrueIfSucceedToUpdatePcscf)
{
    m_pAosRegistration->SetPcscfString("192.168.0.1");
    IpAddress objLocalIpv6Addr(AString("fc01:cafe::100"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objLocalIpv6Addr));
    AStringArray objPcscfs;
    objPcscfs.AddElement("fc01:cafe::1");
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objPcscfs));

    IMS_BOOL bResult = m_pAosRegistration->ProcessIpVersionChange();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, NotifyRegEventStateWhenRegEventChange)
{
    ON_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD));

    EXPECT_CALL(
            m_objMockIAosService, NotifyRegEventState(SipStatusCode::SC_403, ImsList<AString>()));

    m_pAosRegistration->ProcessRegEventChange(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, DoNothingIfAosServiceIsNullWhenRegEventChange)
{
    AosProvider::GetInstance()->SetService(IMS_NULL, SLOT_ID);
    ON_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD));

    EXPECT_CALL(m_objMockIAosService, NotifyRegEventState(_, _)).Times(0);

    m_pAosRegistration->ProcessRegEventChange(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, DoNothingIfRegTypeIsNotNormalWhenRegEventChange)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    ON_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD));

    EXPECT_CALL(m_objMockIAosService, NotifyRegEventState(_, _)).Times(0);

    m_pAosRegistration->ProcessRegEventChange(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, DoNothingIfConfiguredPolicyIsNotDownloadEventWhenRegEventChange)
{
    ON_CALL(m_objMockIAosNConfiguration, GetUsatRegEventDownloadPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::USAT_REG_EVENT_NOT_DOWNLOAD));

    EXPECT_CALL(m_objMockIAosService, NotifyRegEventState(_, _)).Times(0);

    m_pAosRegistration->ProcessRegEventChange(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, TriggerPcscfSelectionWhenStartFailedWithStatusCodeForPcscfDiscovery)
{
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_300);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objRegErrCodeForPcscfDiscovery));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_300);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessStandardPcscfSelection"), 1);
}

TEST_F(AosRegistrationTest, TriggerForbiddenFailHandlingWhenStartFailedWithPermanentStatusCode)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(SipStatusCode::SC_403);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, TriggerPlmnBlockHandlingWhenStartFailedWithPcoLimitedMode)
{
    m_pAosRegistration->SetMode(IAosRegistration::MODE_LIMITED);
    ON_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PCO_LIMITED_SERVICE));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_300);
}

TEST_F(AosRegistrationTest, TriggerStandardPcscfSelectionIfPolicyIs3gppWhenStartFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessStandardPcscfSelection"), 1);
}

TEST_F(AosRegistrationTest, TriggerStandardPcscfSelectionWhenStartFailedWith305ForFirstPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessStandardPcscfSelection"), 1);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetFirstPcscfWhenStartFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, ReportFailureIfSendRegisterFailWhenStartFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, TryRegistrationIfSucceedToSetFirstPcscfWhenStartFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackWhenStartFailedWithRetryRequiredWithoutIpsec)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Add(SipStatusCode::SC_406);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .WillByDefault(ReturnRef(objErrWithoutIpsec));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_406);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest,
        TriggerFlowRecoveryIfIpVersionChangeFailedWhenStartFailedWithIpVersionFallback)
{
    m_pAosRegistration->SetPcscfString("192.168.0.1");
    ON_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .WillByDefault(Return(IMS_TRUE));
    IpAddress objLocalIpv4Addr(AString("192.186.0.100"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objLocalIpv4Addr));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_504);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfHasNextPcscfWhenStartFailedWithIpVersionFallback)
{
    m_pAosRegistration->SetPcscfString("192.168.0.1");
    ON_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, HasNextPcscf()).WillByDefault(Return(IMS_TRUE));
    IpAddress objLocalIpv6Addr(AString("fc01:cafe::100"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objLocalIpv6Addr));
    AStringArray objPcscfs;
    objPcscfs.AddElement("fc01:cafe::1");
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objPcscfs));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_504);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest,
        TriggerOfflineRecoverIfIpVersionChangeSucceedWhenStartFailedWithIpVersionFallback)
{
    m_pAosRegistration->SetPcscfString("192.168.0.1");
    ON_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .WillByDefault(Return(IMS_TRUE));
    IpAddress objLocalIpv6Addr(AString("fc01:cafe::100"));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objLocalIpv6Addr));
    AStringArray objPcscfs;
    objPcscfs.AddElement("fc01:cafe::1");
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(objPcscfs));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_504);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfNoRetryAfterWhenStartFailedWith503)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip503CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_503_CODE_POLICY_3GPP));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .WillByDefault(ReturnNull());

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_503);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerPdnReactivationWhenStartFailedWithFinalResponse)
{
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_4XX);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objExtraRegErrCode));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillByDefault(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_408);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackIfIncludeUnsupportedHeaderWhenStartFailedWith420)
{
    ImsList<AString> objHeaders;
    objHeaders.Append(AString(AosString::STR_SEC_AGREE));
    ON_CALL(m_objMockISipMessage, GetHeaders(ISipHeader::UNSUPPORTED, _))
            .WillByDefault(Return(objHeaders));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfNotIncludeUnsupportedHeaderWhenStartFailedWith420)
{
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackIfIncludeRequireHeaderWhenStartFailedWith421)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    ImsList<AString> objHeaders;
    objHeaders.Append(AString(AosString::STR_SEC_AGREE));
    ON_CALL(m_objMockISipMessage, GetHeaders(ISipHeader::REQUIRE, _))
            .WillByDefault(Return(objHeaders));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_421);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfNotIncludeRequireHeaderWhenStartFailedWith421)
{
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_421);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, SendRegisterIfIncludeMinExpiresWhenStartFailedWith423)
{
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .WillByDefault(Return(AString("180")));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportFailureIfSendRegisterFailWhenStartFailedWith423)
{
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .WillByDefault(Return(AString("180")));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfNotIncludeMinExpiresWhenStartFailedWith423)
{
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryWhenStartFailedWithOtherResponse)
{
    m_pAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_580);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerPendingPlmnBlockIfCallExistWhenUpdateFailedInRoaming)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_TRUE));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, TriggerPlmnBlockWhenUpdateFailedInRoaming)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfPolicyIs3gppWhenUpdateFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetNextPcscfWhenUpdateFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSetFirstPcscfWhenUpdateFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToCreateRegistrationWhenUpdateFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, TryRegistrationIfSucceedToCreateRegistrationWhenUpdateFailedWith305)
{
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    ON_CALL(m_objMockIAosPcscf, IsFirstPcscf()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackWhenUpdateFailedWithRetryRequiredWithoutIpsec)
{
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Add(SipStatusCode::SC_305);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .WillByDefault(ReturnRef(objErrWithoutIpsec));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest, ReportFailureWhenUpdateFailedWithPdnReactivationRequiredStatusCode)
{
    ImsVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    objReregErrCodeForImsPdnReactivation.Add(SipStatusCode::SC_407);
    ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForImsPdnReactivation())
            .WillByDefault(ReturnRef(objReregErrCodeForImsPdnReactivation));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_407);
}

TEST_F(AosRegistrationTest, TriggerForbiddenFailHandlingWhenUpdateFailedWithPermanentStatusCode)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(SipStatusCode::SC_403);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfConfiguredToRetryWithSamePcscfWhenUpdateFailed)
{
    ImsVector<IMS_SINT32> ReregRetryErrCodeForInitRegWithSamePcscf;
    ReregRetryErrCodeForInitRegWithSamePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_ALL_RESP);
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
            .WillByDefault(ReturnRef(ReregRetryErrCodeForInitRegWithSamePcscf));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, ReportFailureIfPdnReactivationIsRequiredWhenUpdateFailed)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillByDefault(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfPdnReactivationIsNotRequiredWhenUpdateFailed)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, TriggerReinitiateIfNoSpecifiedAwtPolicyWhenUpdateFailedWith403)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_403);
}

TEST_F(AosRegistrationTest, SendRegisterIfIncludeMinExpiresWhenUpdateFailedWith423)
{
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .WillByDefault(Return(AString("180")));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, ReportFailureIfSendRegisterFailWhenUpdateFailedWith423)
{
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .WillByDefault(Return(AString("180")));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfNotIncludeMinExpiresWhenUpdateFailedWith423)
{
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryWhenUpdateFailedWithOtherResponse)
{
    m_pAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_600);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfFollowSpecifiedIntervalAwtPolicyWhenRegUpdateFail)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));

    m_pAosRegistration->ProcessUpdateFailed_Others(IRegistration::REASON_NONE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount(
                      "ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy"),
            1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfConfiguredToRetryWithSamePcscfWhenRegUpdateFail)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    ON_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
            .WillByDefault(ReturnRef(objErrCode));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessUpdateFailed_Others(IRegistration::REASON_NONE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, TriggerPdnReactivationIfRequiredWhenRegUpdateFailWithSocketError)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objExtraRegErrCode));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillByDefault(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessUpdateFailed_Others(IRegistration::REASON_CLIENT_SOCKET_ERROR);
}

TEST_F(AosRegistrationTest,
        TriggerFlowRecoveryIfPdnReactivationIsNotRequiredWhenRegUpdateFailWithSocketError)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objExtraRegErrCode));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillByDefault(Return(2));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->ProcessUpdateFailed_Others(IRegistration::REASON_CLIENT_SOCKET_ERROR);
}

TEST_F(AosRegistrationTest, TriggerAwtRecoveryWhenRegUpdateFail)
{
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosRegistration->ProcessUpdateFailed_Others(IRegistration::REASON_NONE);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, TryNextPcscfWhenStandardPcscfSelection)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    m_pAosRegistration->ProcessStandardPcscfSelection(0);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ReportFailureIfTryNextPcscfFailWhenStandardPcscfSelection)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->ProcessStandardPcscfSelection(0);
}

TEST_F(AosRegistrationTest, ReportFailureIfTryNextPcscfFailWhenStandardPcscfSelectionWithRetryAfter)
{
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    m_pAosRegistration->ProcessStandardPcscfSelection(30);
}

TEST_F(AosRegistrationTest, AuthenticationChallengedIsNotHandledWhenRegistrationIsNull)
{
    m_pAosRegistration->Destroy();

    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    m_pAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);

    EXPECT_FALSE(bResponseToChallenge);
    EXPECT_EQ(m_pAosRegistration->GetAuthChallengeCount(), 0);
}

TEST_F(AosRegistrationTest, AuthenticationChallengedIsNotHandledWhenMoreAuthChallengeIsNotAllowed)
{
    m_pAosRegistration->SetMaxRetryCountForAuthentication();
    IMS_UINT32 nCurrentCount = m_pAosRegistration->GetAuthChallengeCount();

    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    m_pAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);

    EXPECT_FALSE(bResponseToChallenge);
    EXPECT_EQ(m_pAosRegistration->GetAuthChallengeCount(), nCurrentCount + 1);
}

TEST_F(AosRegistrationTest, TriggerRegReInitateIfFailToProcessAuthenticationChallenged)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();

    EXPECT_CALL(m_objMockAosIpsecHelper, ProcessAuthChallenged(_)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountWithIpsecOnAuthFailure())
            .WillOnce(Return(0));

    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    m_pAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);

    EXPECT_FALSE(bResponseToChallenge);
    EXPECT_EQ(
            m_pAosRegistration->GetIpsecBlockReason(), AosRegistration::IPSEC_BLOCK_AUTENTICATION);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseIsNotHandledWhenIpsecIsNotSupported)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_FALSE(bResultOfSA);
}

TEST_F(AosRegistrationTest, HandleRegTerminatedWhenNotifyAkaResponseIfIpsecHelperFailToCreate)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillByDefault(Return(IMS_FALSE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED,
            objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegTerminated"), 1);
}

TEST_F(AosRegistrationTest, HandleAuthenticationFailureWhenNotifyAkaResponseWithMacInvalid)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosSubscriber, IsUsim()).WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_USIM_AUTHENTICATION);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessAuthenticationFailed"), 1);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseUpdateResultOfSaToTrueWhenResultIsNokSqnSyncFailed)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillByDefault(Return(IMS_TRUE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED,
            objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_TRUE(bResultOfSA);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseUpdateResultOfSaToTrueWhenUsimAuthHandlingIsNotNeeded)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosSubscriber, IsUsim()).WillByDefault(Return(IMS_FALSE));
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_USIM_AUTHENTICATION);
    ON_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .WillByDefault(ReturnRef(objErrCode));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_TRUE(bResultOfSA);
}

TEST_F(AosRegistrationTest, TriggerRegTerminatedIfFailToSetPcscfPortAndSpiWhenNotifyAkaResponse)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    m_pAosRegistration->SetMaxRetryCountForAuthentication();
    ON_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillByDefault(Return(IMS_FALSE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegTerminated"), 1);
}

TEST_F(AosRegistrationTest, TriggerRegTerminatedIfFailToUpdatePreloadedRouteWhenNotifyAkaResponse)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosIpsecHelper, IsPcscfServerPortDifferent()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosIpsecHelper, UpdatePreloadedRoute(_)).WillByDefault(Return(IMS_FALSE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegTerminated"), 1);
}

TEST_F(AosRegistrationTest, TriggerRegTerminatedIfFailToMakeSasWhenNotifyAkaResponse)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosIpsecHelper, IsPcscfServerPortDifferent()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockAosIpsecHelper, MakeSas(_, _, _, _)).WillByDefault(Return(IMS_FALSE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegTerminated"), 1);
}

TEST_F(AosRegistrationTest, NotifyAkaResponseReturnsSaResultAsTrueWhenSucceedToMakeSas)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();

    EXPECT_CALL(m_objMockAosIpsecHelper, SetPcscfPortnSpi()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, IsPcscfServerPortDifferent()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosIpsecHelper, MakeSas(_, _, _, _)).WillOnce(Return(IMS_TRUE));

    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;
    m_pAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);

    EXPECT_TRUE(bResultOfSA);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredIsNotHandledWhenRegistrationIsNull)
{
    m_pAosRegistration->Destroy();
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredIsNotHandledWhenNotRegisteredState)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredReportsFailureWhenSuspended)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DESTROY));
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredStopsRefreshWhenTransactionIsNotStarted)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, RefreshTimerExpiredStopsRefreshWhenTransactionIsNotAllowed)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest,
        RefreshTimerExpiredStopsRefreshWhenTransactionIsStoppedAndRadioIsWaiting)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetRadioWaiting(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, TriggerRegTerminatedIfFailToCreateIpsecWhenRefreshTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();
    ON_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillByDefault(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    ON_CALL(m_objMockAosIpsecHelper, Create(IMS_FALSE)).WillByDefault(Return(IMS_FALSE));

    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRegTerminated"), 1);
}

TEST_F(AosRegistrationTest, TriggerRegRefreshWhenRefreshTimerExpired)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE));

    m_pAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_TRUE(bDoImplicitRefresh);
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, DoNothingIfRegistrationIsNullWhenRegistrationStarted)
{
    m_pAosRegistration->Destroy();

    EXPECT_CALL(m_objMockIAosPcscf, ResetCurrentPcscfTriedCount()).Times(0);

    m_pAosRegistration->Registration_Started();
}

TEST_F(AosRegistrationTest, TriggerReinitiateIfIpsecIsNotEstablishedWhenRegistrationStarted)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();

    EXPECT_CALL(m_objMockIAosPcscf, ResetCurrentPcscfTriedCount());
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REINITIATE)));

    m_pAosRegistration->Registration_Started();
}

TEST_F(AosRegistrationTest, ReportSuccessWhenRegistrationStarted)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();

    EXPECT_CALL(m_objMockIAosPcscf, ResetCurrentPcscfTriedCount());
    EXPECT_CALL(m_objMockAosIpsecHelper, IsEstablished()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosIpsecHelper, ProcessRegStarted());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Registration_Started();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosRegistrationTest, DoNothingIfRegistrationIsNullWhenRegistrationStartFailed)
{
    m_pAosRegistration->Destroy();

    EXPECT_CALL(m_objMockIRegistration, GetPreviousResponse()).Times(0);

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);
}

TEST_F(AosRegistrationTest,
        HandleAuthenticationFailureIfMoreAuthChallengeIsNotAllowedWhenRegistrationStartFailed)
{
    m_pAosRegistration->SetMaxRetryCountForAuthentication();

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessAuthenticationFailed"), 1);
}

TEST_F(AosRegistrationTest, HandleProcessWhenRegistrationStartFailedOnStatusCodeReason)
{
    // GetResponseCode
    ON_CALL(m_objMockIRegistration, GetPreviousResponse())
            .WillByDefault(Return(&m_objMockISipMessage));
    ON_CALL(m_objMockISipMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_403));
    // ProcessStartFailed_StatusCode
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objRegErrCodeForPcscfDiscovery));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessStartFailed_StatusCode"), 1);
}

TEST_F(AosRegistrationTest, HandleProcessWhenRegistrationStartFailedOnTransactionTimeout)
{
    // ProcessStartFailed_TxnTimeout
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objRegErrCodeForPcscfDiscovery));
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::TIMER_F_POLICY_NONE));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessStartFailed_TxnTimeout"), 1);
}

TEST_F(AosRegistrationTest, SetReasonToTimeoutWhenStartFailedWithTransactionTimeout)
{
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryIfErrorCodeOtherConfiguredWhenStartFailedWithOthers)
{
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillByDefault(ReturnRef(objExtraRegErrCode));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_NONE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, TriggerPdnReactivationIfRequiredWhenStartFailedWithSocketError)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillByDefault(Return(1));
    ON_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillByDefault(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);
}

TEST_F(AosRegistrationTest,
        TriggerFlowRecoveryIfPdnReactivationNotRequiredWhenStartFailedWithSocketError)
{
    m_pAosRegistration->SetEps5GsOnly(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Start"), 1);
}

TEST_F(AosRegistrationTest, SetReasonToTimeoutWhenStartFailedWithClientSocketError)
{
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
}

TEST_F(AosRegistrationTest, SetReasonToTimeoutWhenStartFailedWithServerSocketError)
{
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_SERVER_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetReasonCode(), AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
}

TEST_F(AosRegistrationTest, StartRetryTimerIfAwtRecoveryRequiredWhenStartFailedWithOthers)
{
    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_NONE);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, TriggerWfcErrMessageIfEpdgConnectedWhenStartFailedWith403)
{
    ON_CALL(m_objMockISipMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_403));
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objRegErrCodeForPcscfDiscovery));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRequiredWfcErrMessage_403"), 1);
}

TEST_F(AosRegistrationTest, TriggerWfcErrMessageIfEpdgConnectedWhenStartFailedWith500)
{
    ON_CALL(m_objMockISipMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_500));
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_500);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objRegErrCodeForPcscfDiscovery));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRequiredWfcErrMessage_500"), 1);
}

TEST_F(AosRegistrationTest, TriggerWfcErrMessageIfEpdgConnectedWhenStartFailedWithOthers)
{
    ON_CALL(m_objMockISipMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_600));
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_600);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .WillByDefault(ReturnRef(objRegErrCodeForPcscfDiscovery));

    m_pAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRequiredWfcErrMessage_Others"), 1);
}

TEST_F(AosRegistrationTest, DoNothingIfEmergencyTypeWhenWfcErrMessageHandledWith403)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    m_pAosRegistration->ProcessRequiredWfcErrMessage(SipStatusCode::SC_403);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessRequiredWfcErrMessage_403"), 0);
}

TEST_F(AosRegistrationTest, DoNothingIfRegistrationIsNullWhenRegistrationUpdated)
{
    m_pAosRegistration->Destroy();
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();

    EXPECT_CALL(m_objMockAosIpsecHelper, ProcessRegUpdated()).Times(0);

    m_pAosRegistration->Registration_Updated();
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToUpdateIpsecWhenRegistrationUpdated)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    m_pAosRegistration->CreateIpsecHelper();

    EXPECT_CALL(m_objMockAosIpsecHelper, ProcessRegUpdated()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Registration_Updated();
}

TEST_F(AosRegistrationTest, CreateIfSubscriptionNotExistWhenRegistrationUpdated)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->DestroySubscription();

    EXPECT_CALL(m_objMockIRegistration, CreateSubscription(_))
            .WillOnce(Return(&m_objMockIRegSubscription));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Registration_Updated();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosRegistrationTest, StartExistingSubscriptionWhenRegistrationUpdated)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_SUBSCRIPTION);

    EXPECT_CALL(m_objMockAosSubscription, Start(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Registration_Updated();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosRegistrationTest,
        HandleAuthenticationFailureIfMoreAuthChallengeIsNotAllowedWhenRegistrationUpdateFailed)
{
    m_pAosRegistration->SetMaxRetryCountForAuthentication();

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessAuthenticationFailed"), 1);
}

TEST_F(AosRegistrationTest, HandleProcessWhenRegistrationUpdateFailedOnStatusCodeReason)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    // GetResponseCode
    ON_CALL(m_objMockIRegistration, GetPreviousResponse())
            .WillByDefault(Return(&m_objMockISipMessage));
    ON_CALL(m_objMockISipMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_403));
    // ProcessUpdateFailed_StatusCode
    ImsVector<IMS_SINT32> objReregErrCodeForCallEnd;
    objReregErrCodeForCallEnd.Add(SipStatusCode::SC_403);
    ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
            .WillByDefault(ReturnRef(objReregErrCodeForCallEnd));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessUpdateFailed_StatusCode"), 1);
}

TEST_F(AosRegistrationTest, StopRefreshIfHeldByCallWhenUpdateFailedWithTxnTimeout)
{
    // ProcessUpdateFailed_TxnTimeout - ProcessUnpredictableFailureHeldByCall returns true
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosRegistrationTest, ReportFailureWhenUpdateFailedWithTxnTimeoutWhileInRoaming)
{
    // ProcessUpdateFailed_TxnTimeout - ProcessPlmnBlockOnUpdateFailure returns true
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest,
        TriggerRegWithNextPcscfIncludingReconnectTimeWhenUpdateFailedWithTxnTimeout)
{
    // ProcessUpdateFailed_TxnTimeout
    ImsVector<IMS_SINT32> objReregErrCode;
    objReregErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForInitRegWithAvailablePcscf())
            .WillByDefault(ReturnRef(objReregErrCode));
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    // ProcessRegRequiredWithAvailableNextPcscf with reconnect time - SetNextPcscf fails
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);
}

TEST_F(AosRegistrationTest, TriggerRegWithNextPcscfWhenUpdateFailedWithTxnTimeout)
{
    // ProcessUpdateFailed_TxnTimeout
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForInitRegWithAvailablePcscf())
            .WillByDefault(ReturnRef(objErrCode));
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::TIMER_F_POLICY_NONE));

    // ProcessRegRequiredWithAvailableNextPcscf without reconnect time - SetNextPcscf fails
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);
}

TEST_F(AosRegistrationTest, TriggerIpsecFallbackWhenUpdateFailedWithTxnTimeout)
{
    // ProcessUpdateFailed_TxnTimeout
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_IPSEC);
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    ON_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .WillByDefault(ReturnRef(objErrCode));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessIpsecFallback"), 1);
}

TEST_F(AosRegistrationTest, ReportFailureWithPdnReconnectWhenUpdateFailedWithTxnTimeout)
{
    // ProcessUpdateFailed_TxnTimeout
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));

    // IsPdnReactivationRequired returns true
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryForUpdateWhenUpdateFailedWithTxnTimeout)
{
    // ProcessUpdateFailed_TxnTimeout - ProcessDefaultFlowRecovery_Update
    ON_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillByDefault(Return(IMS_FALSE));
    // ProcessRegRequiredWithAvailableNextPcscf without actual wait time - SetNextPcscf fail
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("ProcessDefaultFlowRecovery_Update"), 1);
}

TEST_F(AosRegistrationTest, TriggerFlowRecoveryWhenRegistrationUpdateFailedWithOtherReason)
{
    // ProcessUpdateFailed_Others
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_NONE);
}

TEST_F(AosRegistrationTest, RegistrationRemovedDestroysRegistration)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_DEREGISTERING);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));

    m_pAosRegistration->Registration_Removed();

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, DoNotStartInternalErrorTimerAgainIfExistWhenRegTerminated)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_INTERNAL_ERROR, 3000);

    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetMaxErrorCountForServerSocket(), 0);
}

TEST_F(AosRegistrationTest, StartInternalErrorTimerIfNotExistWhenRegTerminated)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockITimer, SetTimer(_, _));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_INTERNAL_ERROR));
    EXPECT_EQ(m_pAosRegistration->GetMaxErrorCountForServerSocket(), 1);
}

TEST_F(AosRegistrationTest, TriggerReinitiateIfReconnectingServerIsNotAllowedWhenRegTerminated)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetMaxErrorCountForServerSocket();

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REINITIATE)));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetMaxErrorCountForServerSocket(), 0);
}

TEST_F(AosRegistrationTest, ReportFailureAsPdnReconnectReasonWhenRegTerminatedWhileInCall)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, ReportFailureAsTerminatedReasonWhenRegTerminated)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));

    m_pAosRegistration->Registration_Terminated(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, DoNothingIfRegistrationIsNullWhenSubscriptionStateChanged)
{
    m_pAosRegistration->Destroy();
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->SetConsecutiveFailureCount(1);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy()).Times(0);

    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_ESTABLISHED);

    EXPECT_EQ(m_pAosRegistration->GetConsecutiveFailureCount(), 1);
}

TEST_F(AosRegistrationTest, ClearRetryCountIfSubscriptionEstablishedWhenSubscriptionStateChanged)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->SetConsecutiveFailureCount(1);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));

    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_ESTABLISHED);

    EXPECT_EQ(m_pAosRegistration->GetConsecutiveFailureCount(), 0);
}

TEST_F(AosRegistrationTest, DoNothingIfSubscriptionFailedWhenSubscriptionStateChanged)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy()).Times(0);
    EXPECT_CALL(m_objMockThread, PostMessageI(_)).Times(0);

    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_FAILED);
}

TEST_F(AosRegistrationTest,
        SendMessageForHandlingIfSubscriptionTerminatedWhenSubscriptionStateChanged)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_SUB_TERMINATED)));

    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_TERMINATED);
}

TEST_F(AosRegistrationTest, DoNothingWhenSubscriptionStateChangedOnUnknownReason)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy()).Times(0);
    EXPECT_CALL(m_objMockThread, PostMessageI(_)).Times(0);

    m_pAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_NONE);
}

TEST_F(AosRegistrationTest, SubscriptionCanBeTransmittedWhileTransactionIsNotStartedReturnsFalse)
{
    m_pAosRegistration->SetTransactionStarted(IMS_FALSE);

    IMS_BOOL bResult = m_pAosRegistration->Subscription_CanBeTransmitted();

    EXPECT_FALSE(bResult);
    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_SUBSCRIPTION));
}

TEST_F(AosRegistrationTest, SubscriptionCanBeTransmittedWhileTransactionIsStartedReturnsTrue)
{
    IMS_BOOL bResult = m_pAosRegistration->Subscription_CanBeTransmitted();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, DoNothingIfRegistrationIsNullWhenSubscriptionNotifyReceived)
{
    m_pAosRegistration->Destroy();

    EXPECT_CALL(m_objMockThread, PostMessageI(_)).Times(0);

    m_pAosRegistration->Subscription_NotifyReceived(AosSubscription::EVENT_REGISTERED);
}

TEST_F(AosRegistrationTest, SendMessageForHandlingWhenSubscriptionNotifyReceivedAsRegistered)
{
    EXPECT_CALL(
            m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_REG_EVENT_REGISTERED)));

    m_pAosRegistration->Subscription_NotifyReceived(AosSubscription::EVENT_REGISTERED);
}

TEST_F(AosRegistrationTest, RegRequiredSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread,
            PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REQUIRED_WITH_WAIT_TIME)));

    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_REG_REQUIRED, 0, IMS_TRUE);
}

TEST_F(AosRegistrationTest, RegRequiredWithNextPcscfSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread,
            PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REQUIRED_WITH_NEXT_PCSCF)));

    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_NEXT_PCSCF, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest,
        RegRequiredWithScscfRestorationSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread,
            PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION)));

    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_SCSCF_RESTORATION, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest, RegRequiredWithSub403MsgSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread,
            PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REQUIRED_WITH_WAIT_TIME)));

    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_SUB_403_MSG, 0, IMS_TRUE);
}

TEST_F(AosRegistrationTest,
        RegRequiredWithNotifyTerminatedMsgSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread,
            PostMessageI(IsSameMsg(AosRegistration::MSG_REG_REQUIRED_WITH_WAIT_TIME)));

    m_pAosRegistration->Subscription_Request(
            AosSubscription::CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG, 0, IMS_TRUE);
}

TEST_F(AosRegistrationTest, RegTerminatedSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread,
            PostMessageI(IsSameMsg(AosRegistration::MSG_REG_TERMINATED_BY_NOTIFY)));

    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_REG_TERMINATED, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest, SubRequiredSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_SUB_REINITIATE)));

    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_SUB_REQUIRED, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest, SubTerminatedSubscriptionCommandSendsMessageForHandling)
{
    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(AosRegistration::MSG_SUB_TERMINATED)));

    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_SUB_TERMINATED, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest, UnknownSubscriptionCommandDoesNothing)
{
    EXPECT_CALL(m_objMockThread, PostMessageI(_)).Times(0);

    m_pAosRegistration->Subscription_Request(AosSubscription::CMD_NONE, 0, IMS_FALSE);
}

TEST_F(AosRegistrationTest, UpdatesBlockedStatusWhenBlockChanged)
{
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->Block_Changed();

    EXPECT_TRUE(m_pAosRegistration->IsBlocked());
}

TEST_F(AosRegistrationTest, CallTrackerStateChangedForEmergencyTypeDoesNothing)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    m_pAosRegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);

    EXPECT_TRUE(m_pAosRegistration->IsImsCall());
}

TEST_F(AosRegistrationTest, CallTrackerStateChangedHandlesPendingPlmnBlock)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->SetHeldByCall(IMS_TRUE);
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL);
    m_pAosRegistration->SetEps5GsOnly(IMS_TRUE);
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_FALSE(
            m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL));
}

TEST_F(AosRegistrationTest, CallTrackerStateChangedTriggersReinitiateRegIfRequired)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->SetHeldByCall(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRequiredAfterImsCallEndOnRegHeld())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, CallTrackerStateChangedHandlesPendingUpdate)
{
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->SetHeldByCall(IMS_FALSE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_UPDATE_HELD_BY_CALL);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_UPDATE_HELD_BY_CALL));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, NotifyConfigChangedUpdatesFeaturesWhileIpsecAndSubscriptionIsSupported)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_TRUE));

    m_pAosRegistration->NConfiguration_NotifyConfigChanged();

    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(AosRegistration::FEATURE_SUBSCRIPTION));
    EXPECT_TRUE(m_pAosRegistration->IsFeatureOn(AosRegistration::FEATURE_IPSEC));
}

TEST_F(AosRegistrationTest,
        NotifyConfigChangedUpdatesFeaturesWhileIpsecAndSubscriptionIsNotSupported)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillOnce(Return(IMS_FALSE));

    m_pAosRegistration->NConfiguration_NotifyConfigChanged();

    EXPECT_FALSE(m_pAosRegistration->IsFeatureOn(AosRegistration::FEATURE_SUBSCRIPTION));
    EXPECT_FALSE(m_pAosRegistration->IsFeatureOn(AosRegistration::FEATURE_IPSEC));
}

TEST_F(AosRegistrationTest, TransactionOnConnectionFailedWithAccessDeniedDestroysRegistration)
{
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest,
        TransactionOnConnectionFailedWithRrcRejectDestroysRegistrationWithoutPcscfClear)
{
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(0);
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    IMS_UINT32 nExceededWaitTime =
            (m_pAosRegistration->GetDefaultWaitTimeForConnectionFailure() + 1) * 1000;
    m_pAosRegistration->Transaction_OnConnectionFailed(
            IImsRadio::REASON_RRC_REJECT, 0, nExceededWaitTime);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest,
        TransactionOnConnectionFailedWithOtherReasonHandlesConnectionSetupPrepared)
{
    m_pAosRegistration->SetRadioWaiting(IMS_TRUE);
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_TRANSACTION);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);

    m_pAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_INTERNAL_ERROR, 0, 0);

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, TransactionOnTrafficPriorityChangedTriggersPendingTransaction)
{
    m_pAosRegistration->SetTrafficPriorityBlocked(IMS_TRUE);
    m_pAosRegistration->SetTxnPending(AosRegistration::PENDING_START);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosRegistration->Transaction_OnTrafficPriorityChanged();

    EXPECT_FALSE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_START));
    EXPECT_EQ(m_pAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, MessageMediatorAdjustMessageReturnsFailureIfSipMessageIsNull)
{
    IMS_RESULT nResult = m_pAosRegistration->MessageMediator_AdjustMessage(
            IMS_NULL, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(AosRegistrationTest, MessageMediatorAdjustMessageReturnsFailureIfFailToAddLocationHeaderBody)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);

    IMS_RESULT nResult = m_pAosRegistration->MessageMediator_AdjustMessage(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(AosRegistrationTest,
        MessageMediatorAdjustMessageReturnsSuccessIfSucceedToAddLocationHeaderBody)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);

    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::GEOLOCATION_POLICY_WITHOUT_POSITION));
    MockISipMessageBodyPart objMockISipMessageBodyPart;
    ON_CALL(m_objMockISipMessage, CreateBodyPart())
            .WillByDefault(Return(&objMockISipMessageBodyPart));

    IMS_RESULT nResult = m_pAosRegistration->MessageMediator_AdjustMessage(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_EQ(nResult, IMS_SUCCESS);
    GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBodyReturnsFalseIfGeoLocationInfoIsNotRequired)
{
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);

    IMS_BOOL bResult = m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, RemovePreviousBodyWhenAddLocationHeaderBodyWithResubmit)
{
    ON_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISipMessage, RemoveBodyParts());

    m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_RESUBMIT);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBodyReturnsFalseIfFailToCreatePidfWithoutPosition)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::GEOLOCATION_POLICY_WITHOUT_POSITION));
    ON_CALL(m_objMockILocationInfo, GetLocationProperties(_)).WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_FALSE(bResult);
    GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBodyReturnsFalseIfFailToCreatePidfWithPosition)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::GEOLOCATION_POLICY_WITH_POSITION));
    ON_CALL(m_objMockILocationInfo, GetLocationProperties(_)).WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_FALSE(bResult);
    GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
}

TEST_F(AosRegistrationTest,
        AddLocationHeaderBodyReturnsFalseIfFailToCreatePidfWithPositionAndCountry)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy())
            .WillByDefault(
                    Return(CarrierConfig::Assets::GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY));
    ON_CALL(m_objMockILocationInfo, GetLocationProperties(_)).WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_FALSE(bResult);
    GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBodyReturnsFalseIfFailToCreatePidfWithoutCivic)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy())
            .WillByDefault(Return(CarrierConfig::Assets::GEOLOCATION_POLICY_WITHOUT_CIVIC));
    ON_CALL(m_objMockILocationInfo, GetLocationProperties(_)).WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_FALSE(bResult);
    GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBodyReturnsFalseIfPidfFormingPolicyIsInvalid)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    IMS_SINT32 nInvalidPidfFormingPolicy = 0;
    ON_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy())
            .WillByDefault(Return(nInvalidPidfFormingPolicy));

    IMS_BOOL bResult = m_pAosRegistration->AddLocationHeaderBody(
            &m_objMockISipMessage, IMessageMediator::MESSAGE_NORMAL);

    EXPECT_FALSE(bResult);
    GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
}

TEST_F(AosRegistrationTest, InvokingClearTimersStopsAllTimersExceptOfflineRecoverTimer)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_REFRESH, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_EXPIRED, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_MODE, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_TRANSACTION, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_INTERNAL_ERROR, 5000);

    m_pAosRegistration->ClearTimers();

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_REFRESH));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_EXPIRED));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_MODE));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_TRANSACTION));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, InvokingClearRetryTimersStopsOfflineRecoverTimerAndStopRetryTimer)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_REFRESH, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_EXPIRED, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_MODE, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_TRANSACTION, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_INTERNAL_ERROR, 5000);

    m_pAosRegistration->ClearRetryTimers();

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_REFRESH));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_EXPIRED));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_MODE));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_TRANSACTION));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, DoNothingIfExpiredTimerIsNullWhenTimerExpired)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_REFRESH, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_EXPIRED, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_MODE, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_TRANSACTION, 5000);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_INTERNAL_ERROR, 5000);

    m_pAosRegistration->Timer_TimerExpired(IMS_NULL);

    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_REFRESH));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_EXPIRED));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_MODE));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_TRANSACTION));
    EXPECT_TRUE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest,
        DoNothingExceptStopTimerIfStateIsNotOfflineWhenOfflineRecoverTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_OFFLINE_RECOVER));

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("CheckRadioReadyAndSetTxnPending"), 0);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, DoNothingExceptStopTimerIfAppIsNotReadyWhenOfflineRecoverTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetAppReady(IMS_FALSE);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_OFFLINE_RECOVER));

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("CheckRadioReadyAndSetTxnPending"), 0);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest,
        AddTrafficPendingIfTransactionIsNotStartedWhenOfflineRecoverTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetAppReady(IMS_TRUE);
    m_pAosRegistration->SetImsCall(IMS_TRUE);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_OFFLINE_RECOVER));

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("CheckRadioReadyAndSetTxnPending"), 0);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest,
        TryRegistrationIfSucceedToCreateRegistrationWhenOfflineRecoverTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetAppReady(IMS_TRUE);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .WillByDefault(ReturnRef(m_objAvailableImpus));
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_OFFLINE_RECOVER));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToCreateRegistrationWhenOfflineRecoverTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pAosRegistration->SetAppReady(IMS_TRUE);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_OFFLINE_RECOVER, 5000);
    ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .WillByDefault(ReturnRef(m_objEmptyImpus));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_OFFLINE_RECOVER));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_OFFLINE_RECOVER));
}

TEST_F(AosRegistrationTest, DoNothingExceptStopTimerIfRetryWasNotHeldWhenStopRetryTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_STOP_RETRY));

    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("CheckRadioReadyAndSetTxnPending"), 0);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, AddTrafficPendingIfTransactionIsNotStartedWhenStopRetryTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);
    m_pAosRegistration->SetImsCall(IMS_TRUE);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_STOP_RETRY));

    EXPECT_TRUE(m_pAosRegistration->IsTxnPendingOn(AosRegistration::PENDING_TRAFFIC));
    EXPECT_EQ(m_pAosRegistration->GetInvokedCount("CheckRadioReadyAndSetTxnPending"), 0);
    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, TryRegistrationIfSucceedToSendRegisterWhenStopRetryTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_STOP_RETRY));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, ReportFailureIfFailToSendRegisterWhenStopRetryTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_STOP_RETRY, 5000);
    ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_STOP_RETRY));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, StopRefreshTimerWhenRefreshTimerExpired)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_REFRESH, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_REFRESH));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_REFRESH));
}

TEST_F(AosRegistrationTest, StopExpiredTimerWhenExpiredTimerExpired)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_EXPIRED, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_EXPIRED));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_EXPIRED));
}

TEST_F(AosRegistrationTest, StopModeTimerWhenModeTimerExpired)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_MODE, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_MODE));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_MODE));
}

TEST_F(AosRegistrationTest, StopTransactionTimerWhenTransactionTimerExpired)
{
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_TRANSACTION, 5000);

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_TRANSACTION));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_TRANSACTION));
}

TEST_F(AosRegistrationTest, RestoreActiveBindingsIfRegisteredWhenInternalTimerExpired)
{
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->StartTimer(AosRegistration::TIMER_INTERNAL_ERROR, 5000);

    EXPECT_CALL(m_objMockIRegistration, RestoreActiveBindings());

    m_pAosRegistration->Timer_TimerExpired(
            m_pAosRegistration->GetTimer(AosRegistration::TIMER_INTERNAL_ERROR));

    EXPECT_FALSE(m_pAosRegistration->IsTimerRunning(AosRegistration::TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, CreateSubscriptionReturnsFalseIfFeatureIsNotOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    IMS_BOOL bResult = m_pAosRegistration->CreateSubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, CreateSubscriptionReturnsFalseIfRegistrationIsNull)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->UpdateRegInstances(IMS_NULL, &m_objMockAosSubscription,
            &m_objMockIRegContact, &m_objMockIRegParameter, &m_objMockAosIpsecHelper);

    IMS_BOOL bResult = m_pAosRegistration->CreateSubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, CreateSubscriptionReturnsFalseIfFailToCreate)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    ON_CALL(m_objMockIRegistration, CreateSubscription(_)).WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosRegistration->CreateSubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, CreateSubscriptionReturnsTrueIfSucceedToCreate)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    ON_CALL(m_objMockIRegistration, CreateSubscription(_))
            .WillByDefault(Return(&m_objMockIRegSubscription));

    IMS_BOOL bResult = m_pAosRegistration->CreateSubscription();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, DestroySubscriptionReturnsFalseIfFeatureIsNotOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    IMS_BOOL bResult = m_pAosRegistration->DestroySubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, DestroySubscriptionReturnsFalseIfSubscriptionIsNull)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->UpdateRegInstances(&m_objMockIRegistration, IMS_NULL, &m_objMockIRegContact,
            &m_objMockIRegParameter, &m_objMockAosIpsecHelper);

    IMS_BOOL bResult = m_pAosRegistration->DestroySubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, DestroySubscriptionReturnsTrueIfSucceedToDestroy)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    IMS_BOOL bResult = m_pAosRegistration->DestroySubscription();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, StartSubscriptionReturnsFalseIfFeatureIsNotOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    IMS_BOOL bResult = m_pAosRegistration->StartSubscription(IMS_TRUE);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, StartSubscriptionReturnsFalseIfSubscriptionIsNull)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->UpdateRegInstances(&m_objMockIRegistration, IMS_NULL, &m_objMockIRegContact,
            &m_objMockIRegParameter, &m_objMockAosIpsecHelper);

    IMS_BOOL bResult = m_pAosRegistration->StartSubscription(IMS_TRUE);

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, StartSubscriptionReturnsTrueIfSucceedToStart)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    IMS_BOOL bResult = m_pAosRegistration->StartSubscription(IMS_TRUE);

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, StopSubscriptionReturnsFalseIfFeatureIsNotOn)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_NONE);

    IMS_BOOL bResult = m_pAosRegistration->StopSubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, StopSubscriptionReturnsFalseIfSubscriptionIsNull)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);
    m_pAosRegistration->UpdateRegInstances(&m_objMockIRegistration, IMS_NULL, &m_objMockIRegContact,
            &m_objMockIRegParameter, &m_objMockAosIpsecHelper);

    IMS_BOOL bResult = m_pAosRegistration->StopSubscription();

    EXPECT_FALSE(bResult);
}

TEST_F(AosRegistrationTest, StopSubscriptionReturnsTrueIfSucceedToStop)
{
    m_pAosRegistration->SetFeature(AosRegistration::FEATURE_SUBSCRIPTION);

    IMS_BOOL bResult = m_pAosRegistration->StopSubscription();

    EXPECT_TRUE(bResult);
}

TEST_F(AosRegistrationTest, ShouldNotifyRegisteredForEmergencyTypeWhenRegistrationSucceeds)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(m_objMockIAosService,
            NotifyRegistered(IAosRegistration::IMS_REG_TYPE_EMERGENCY, _, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest,
        ShouldNotifyRegisteredForEmergencyTypeAsFakeTypeIfFakeRegWhenRegistrationSucceeds)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetFakeReg(IMS_TRUE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(
            m_objMockIAosService, NotifyRegistered(IAosRegistration::IMS_REG_TYPE_FAKE, _, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldNotifyRegisteredForFakeTypeWhenRegistrationSucceeds)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(
            m_objMockIAosService, NotifyRegistered(IAosRegistration::IMS_REG_TYPE_FAKE, _, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldNotifyRegisteringForEmergencyTypeWhenTryRegistration)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIAosService,
            NotifyRegistering(IAosRegistration::IMS_REG_TYPE_EMERGENCY, _, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest,
        ShouldNotifyRegisteringForEmergencyTypeAsFakeTypeIfFakeRegWhenTryRegistration)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetFakeReg(IMS_TRUE);

    EXPECT_CALL(
            m_objMockIAosService, NotifyRegistering(IAosRegistration::IMS_REG_TYPE_FAKE, _, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldNotifyRegisteringForFakeTypeWhenTryRegistration)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);

    EXPECT_CALL(
            m_objMockIAosService, NotifyRegistering(IAosRegistration::IMS_REG_TYPE_FAKE, _, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldNotifyDeregisteredForEmergencyTypeWhenRegistrationTerminated)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_EMERGENCY, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest,
        ShouldNotifyDeregisteredForEmergencyTypeAsFakeTypeIfFakeRegWhenRegistrationTerminated)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pAosRegistration->SetFakeReg(IMS_TRUE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(
            m_objMockIAosService, NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_FAKE, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, ShouldNotifyDeregisteredForFakeTypeWhenRegistrationTerminated)
{
    // GIVEN
    m_pAosRegistration->SetRegType(AosRegistrationType::FAKE);
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(
            m_objMockIAosService, NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_FAKE, _, _));

    // WHEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosRegistrationTest, SetReregFailureReportOnIpcanChangeRequiredToTrueAfterHandover)
{
    // GIVEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosRegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_FALSE);

    // WHEN
    m_pAosRegistration->ProcessIpcanChanged();

    // THEN
    EXPECT_TRUE(m_pAosRegistration->IsReregFailureReportOnIpcanChangeRequired());
}

TEST_F(AosRegistrationTest,
        ResetReregFailureReportOnIpcanChangeRequiredToFalseIfReregistrationSucceeds)
{
    // GIVEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pAosRegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);

    // WHEN
    m_pAosRegistration->Registration_Updated();

    // THEN
    EXPECT_FALSE(m_pAosRegistration->IsReregFailureReportOnIpcanChangeRequired());
}

TEST_F(AosRegistrationTest,
        ResetReregFailureReportOnIpcanChangeRequiredToFalseWhenDestroyRegistration)
{
    // GIVEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pAosRegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);

    // WHEN
    m_pAosRegistration->DestroyRegistration();

    // THEN
    EXPECT_FALSE(m_pAosRegistration->IsReregFailureReportOnIpcanChangeRequired());
}

TEST_F(AosRegistrationTest,
        NotifyTechnologyChangeFailedIfReregistrationFailsWhileTheFlagIsSetToTrue)
{
    // GIVEN
    m_pAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pAosRegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosService, NotifyTechnologyChangeFailed(_, _, _));

    // WHEN
    m_pAosRegistration->Registration_UpdateFailed(IRegistration::REASON_NONE);

    // THEN: The GIVEN condition should be met
}

TEST_F(AosRegistrationTest, ShouldSetAllPcscfValidWhenClearPcscf)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosPcscf, SetAllPcscfValid());

    // WHEN
    m_pAosRegistration->ClearPcscf();

    // THEN: The GIVEN condition should be met.
}
