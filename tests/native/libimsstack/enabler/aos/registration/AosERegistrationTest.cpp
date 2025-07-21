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
#include "ImsMap.h"
#include "CarrierConfig.h"
#include "IImsRadio.h"
#include "IIpcan.h"
#include "PlatformContext.h"
#include "SipMessageBodyPart.h"
#include "SipStatusCode.h"
#include "TestPhoneInfoService.h"
#include "TestUtilService.h"

#include "../../../engine/interface/sipcore/MockISipMessage.h"
#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"

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

#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosEmergencyListener.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosERegistration.h"

using ::testing::_;
using ::testing::An;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base)                                                  \
    using Base::CallbackModeChanged;                                         \
    using Base::CallTracker_StateChanged;                                    \
    using Base::CleanUp;                                                     \
    using Base::ClearTimers;                                                 \
    using Base::CreateIpsecHelper;                                           \
    using Base::DestroyIpsecHelper;                                          \
    using Base::GetPreferredRegScheme;                                       \
    using Base::Init;                                                        \
    using Base::IsFakeModeCondition;                                         \
    using Base::IsFakeRegistration;                                          \
    using Base::IsImsCall;                                                   \
    using Base::IsRefreshRequiredByCbm;                                      \
    using Base::IsReinitiationRequested;                                     \
    using Base::IsReregFailureReportOnIpcanChangeRequired;                   \
    using Base::IsRetryAllowed;                                              \
    using Base::IsTransactionStarted;                                        \
    using Base::NConfiguration_NotifyConfigChanged;                          \
    using Base::OnMessage;                                                   \
    using Base::ProcessAuthenticationFailed;                                 \
    using Base::ProcessDefaultFlowRecovery_Start;                            \
    using Base::ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy; \
    using Base::ProcessDefaultFlowRecovery_Update;                           \
    using Base::ProcessModeTimerExpired;                                     \
    using Base::ProcessNormalDefaultFlowRecovery_Start;                      \
    using Base::ProcessRearrangePcscf;                                       \
    using Base::ProcessReinitiateWithRegState;                               \
    using Base::ProcessStartFailed_StatusCode;                               \
    using Base::ProcessStartFailed_TxnTimeout;                               \
    using Base::ProcessTransactionTimerExpired;                              \
    using Base::ProcessUpdateFailed_Others;                                  \
    using Base::ProcessUpdateFailed_StatusCode;                              \
    using Base::ProcessUpdateFailed_TxnTimeout;                              \
    using Base::Registration_RefreshTimerExpired;                            \
    using Base::Registration_StartFailed;                                    \
    using Base::Registration_Started;                                        \
    using Base::Registration_Terminated;                                     \
    using Base::Registration_Updated;                                        \
    using Base::SetFakeReg;                                                  \
    using Base::SetImsCall;                                                  \
    using Base::SetReinitiationRequested;                                    \
    using Base::SetReregFailureReportOnIpcanChangeRequired;                  \
    using Base::SetState;                                                    \
    using Base::StopTimer;                                                   \
    using Base::Transaction_OnConnectionFailed;                              \
    using Base::Transaction_OnConnectionSetupPrepared;                       \
    using Base::Transaction_OnTrafficPriorityChanged;                        \
    using Base::SetCallFailureCauseToProperty;                               \
    using Base::UpdateRegIpcanCategory;                                      \
    using Base::UpdateTransactionStarted;

const IMS_SINT32 SLOT_ID = 0;

class TestAosERegistration : public AosERegistration
{
public:
    DECLARE_USING(AosERegistration)

    inline TestAosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosERegistration(piAppContext, strRegId),
            m_piReg(IMS_NULL)
    {
        m_pCounter = new AosCounter();
    }
    inline ~TestAosERegistration() override { delete m_pCounter; }
    inline IRegistration* GetRegistration() override { return m_piReg; }
    inline void ClearEModeInfo()
    {
        if (m_pEModeInfo != IMS_NULL)
        {
            delete m_pEModeInfo;
            m_pEModeInfo = IMS_NULL;
        }
    }

    inline void UpdateRegInstances(IN IRegistration* piReg, IN IRegParameter* piParam)
    {
        m_piReg = piReg;
        m_piRegistration = piReg;
        m_piRegParameter = piParam;
    }

    inline EmergencyModeInfo* GetEModeInfo() { return m_pEModeInfo; }

    inline void SetConsecutiveFailure(IN IMS_UINT32 nValue) { m_nConsecutiveFailure = nValue; }

    inline ITimer* GetTransactionTimer() { return m_piTransactionTimer; }

    inline ITimer* GetModeTimer() { return m_piModeTimer; }

    inline void SetRegistration(IN IRegistration* piRegistration)
    {
        m_piRegistration = piRegistration;
    }

    inline void SetEModeInfo(IN EmergencyModeInfo* pEModeInfo) { m_pEModeInfo = pEModeInfo; }

    inline AosReasonCode GetImsReasonCode() { return m_eImsReasonCode; }

    inline void SetImsReasonCode(IN AosReasonCode eImsReasonCode)
    {
        m_eImsReasonCode = eImsReasonCode;
    }

    inline AosUtil* GetUtil() { return m_pUtil; }

    inline IMS_UINT32 GetFeature() { return m_nFeature; }

    inline void SetFeature(IN IMS_UINT32 nFeature) { m_nFeature = nFeature; }

    inline void SetRegContact(IN IRegContact* piRegContact) { m_piRegContact = piRegContact; }

    inline IMS_UINT32 GetTxnPending() { return m_nTxnPending; }

    inline IMS_UINT32 GetERegTimeoutFailureCause()
    {
        return AosERegistration::ECALL_FAILURE_CAUSE_EREG_TIMEOUT_DUE_TO_TCP_FAILURE;
    }

    IMS_UINT32 GetInvokedCount(IN const AString& strName) { return m_pCounter->GetCount(strName); }

    // Functions where calls are being counted
    void ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(
            IN IMS_UINT32 nRetryAfter) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosERegistration::ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(nRetryAfter);
    }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosERegistration::StartTimer(nType, nDuration);
    }

private:
    AosCounter* m_pCounter;
    IRegistration* m_piReg;
};

class AosERegistrationTest : public ::testing::Test
{
public:
    inline AosERegistrationTest() :
            m_pAosERegistration(IMS_NULL)
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
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
    }
    inline virtual ~AosERegistrationTest()
    {
        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);

        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker, SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_piAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);
    }

    TestAosERegistration* m_pAosERegistration;
    TestPhoneInfoService m_objPhoneInfoService;
    TestUtilService m_objUtilService;

    AosStaticProfile* m_pAosStaticProfile;
    IAosCallTracker* m_piAosCallTracker;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosRetryRepository* m_piAosRetryRepository;
    IAosService* m_piAosService;
    IAosTransaction* m_piAosTransaction;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosCallTracker m_objMockIAosCallTracker;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistrationListener m_objMockIAosRegistrationListener;
    MockIAosRetryRepository m_objMockIAosRetryRepository;
    MockIAosService m_objMockIAosService;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosTransaction m_objMockIAosTransaction;
    MockISipMessage m_objMockISipMessage;
    MockIRegContact m_objMockIRegContact;
    MockIRegParameter m_objMockIRegParameter;
    MockIRegistration m_objMockIRegistration;

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    SipAddress m_objSipAddress = SipAddress("sip:1111@1.1.1.1");
    AosFeatureTagList m_objFeatureTagList;
    AosFeatureTagList m_objBindedFeatureTagList;
    AStringArray m_objImpus;
    AStringArray m_objPcscfs;
    ImsList<IMS_SINT32> m_objPcscfPorts;
    ImsMap<AString, IAosHandle*> m_objHandles;
    ImsVector<IMS_SINT32> m_objEmptyErrCode;

protected:
    void SetUp() override
    {
        // IAosAppContext
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));
        ON_CALL(m_objMockIAosAppContext, GetSubscriber())
                .WillByDefault(Return(&m_objMockIAosSubscriber));
        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(Return(&m_objMockIAosBlock));
        m_objHandles.Add(m_strServiceId, &m_objMockIAosHandle);
        ON_CALL(m_objMockIAosAppContext, GetHandles()).WillByDefault(ReturnRef(m_objHandles));
        ON_CALL(m_objMockIAosAppContext, GetPcscf()).WillByDefault(Return(&m_objMockIAosPcscf));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetNetTracker())
                .WillByDefault(Return(&m_objMockIAosNetTracker));

        // IRegistration
        ON_CALL(m_objMockIRegistration, Register(_)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(m_objMockIRegistration, Deregister()).WillByDefault(Return(IMS_SUCCESS));
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
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
                .WillByDefault(Return(30000));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
                .WillByDefault(Return(1800000));
        ON_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsIpsecInitializedWithNewPcscf())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
                .WillByDefault(
                        Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));
        ON_CALL(m_objMockIAosNConfiguration, GetRegistrationPrivateHeader())
                .WillByDefault(Return(CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED));
        ON_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
                .WillByDefault(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
        ON_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp()).WillByDefault(Return(46));
        ON_CALL(m_objMockIAosNConfiguration, GetSipPreferredTransport())
                .WillByDefault(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP));
        ON_CALL(m_objMockIAosNConfiguration, IsContactUriValidationChecked())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt())
                .WillByDefault(Return(CarrierConfig::ImsEmergency::EREG_RETRY_MAX_CNT_NO_RETRY));
        ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
                .WillByDefault(Return(CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
        ON_CALL(m_objMockIAosNConfiguration, GetRoamingPreferredEmcReg())
                .WillByDefault(Return(
                        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED));
        ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
                .WillByDefault(Return(
                        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));
        ON_CALL(m_objMockIAosNConfiguration, GetERegErrCodeNotSupportedCommonPolicy())
                .WillByDefault(ReturnRef(m_objEmptyErrCode));

        // IAosSubscriber
        m_objImpus.AddElement(AString("sip:1111@ims.co.kr"));
        m_objImpus.AddElement(AString("sip:2222@ims.co.kr"));
        ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillByDefault(ReturnRef(m_objImpus));
        ON_CALL(m_objMockIAosSubscriber, GetFakeImpus()).WillByDefault(ReturnRef(m_objImpus));

        // IAosBlock
        ON_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillByDefault(Return(IMS_FALSE));

        // IAosHandle
        ON_CALL(m_objMockIAosHandle, GetAppId()).WillByDefault(ReturnRef(m_strAppId));
        ON_CALL(m_objMockIAosHandle, GetServiceId()).WillByDefault(ReturnRef(m_strServiceId));
        ON_CALL(m_objMockIAosHandle, GetRequestType()).WillByDefault(Return(IAosHandle::ATTACH));
        ON_CALL(m_objMockIAosHandle, GetFeatureTagList())
                .WillByDefault(ReturnRef(m_objFeatureTagList));
        ON_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
                .WillByDefault(ReturnRef(m_objBindedFeatureTagList));
        ON_CALL(m_objMockIAosHandle, GetServiceType())
                .WillByDefault(Return(static_cast<IMS_UINT32>(ImsAosService::MTS)));

        // IAosPcscf
        ON_CALL(m_objMockIAosPcscf, GetCurrentIndex()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosPcscf, HasPcscf(_)).WillByDefault(Return(IMS_TRUE));
        m_objPcscfs.AddElement(AString("192.168.0.100"));
        ON_CALL(m_objMockIAosPcscf, GetPcscfs()).WillByDefault(ReturnRef(m_objPcscfs));

        m_objPcscfPorts.Append(5060);
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
        ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_FALSE));

        // IRegContact
        ON_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIRegContact, GetContactAddress())
                .WillByDefault(ReturnRef(m_objSipAddress));

        // IRegParameter
        ON_CALL(m_objMockIRegParameter, AddPreloadedRoute(_, _, _)).WillByDefault(Return(IMS_TRUE));

        // ISipMessage
        ON_CALL(m_objMockISipMessage, GetHeader(_, _, _)).WillByDefault(Return(AString("regtest")));

        m_pAosERegistration = new TestAosERegistration(
                &m_objMockIAosAppContext, m_pAosStaticProfile->GetRegistrationId());
        m_pAosERegistration->UpdateRegInstances(&m_objMockIRegistration, &m_objMockIRegParameter);
        m_pAosERegistration->SetListener(&m_objMockIAosRegistrationListener);
    }

    void TearDown() override
    {
        if (m_pAosERegistration)
        {
            m_pAosERegistration->ClearEModeInfo();
            m_pAosERegistration->ClearTimers();
            m_pAosERegistration->StopTimer(TestAosERegistration::TIMER_OFFLINE_RECOVER);

            delete m_pAosERegistration;
            m_pAosERegistration = IMS_NULL;
        }
    }
};

TEST_F(AosERegistrationTest, ReportFailureWhenStartWithFakeModeForEmergencySms)
{
    ON_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetECall(IMS_FALSE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosERegistration->Start();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, StartWhenInFakeModeCondition_SetFakeMode)
{
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).Times(0);

    m_pAosERegistration->Start();

    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_EQ(m_pAosERegistration->GetTransactionTimer(), nullptr);
}

TEST_F(AosERegistrationTest, Start_SetNormalModeEmergRetrySupported)
{
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
            .WillByDefault(Return(2000));
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryTimerMillis()).WillByDefault(Return(500));
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallBasedOnPauOfNormalRegistrationSupported())
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf());

    m_pAosERegistration->Start();

    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_NORMAL);
    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_NE(m_pAosERegistration->GetTransactionTimer(), nullptr);
}

TEST_F(AosERegistrationTest, Start_SetNormalModeEmergRetryNotSupported)
{
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
            .WillByDefault(Return(2000));
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryTimerMillis()).WillByDefault(Return(0));
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallBasedOnPauOfNormalRegistrationSupported())
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf());

    m_pAosERegistration->Start();

    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_NORMAL);
    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_EQ(m_pAosERegistration->GetTransactionTimer(), nullptr);
}

TEST_F(AosERegistrationTest, Start_SetNormalModeCreateRegistrationFailure)
{
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
            .WillByDefault(Return(2000));
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryTimerMillis()).WillByDefault(Return(500));
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallBasedOnPauOfNormalRegistrationSupported())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosPcscf, HasPcscf(_)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf());

    m_pAosERegistration->Start();

    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_NORMAL);
    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pAosERegistration->GetTransactionTimer(), nullptr);
}

TEST_F(AosERegistrationTest, UpdateWhenInRegStopState_ProcessRetry)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosERegistration->Update();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosERegistrationTest, UpdateWhenInRegisteredState_ProcessReregister)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosERegistration->Update();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosERegistrationTest, UpdateWhenInOfflineState_Ignored)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(0);

    m_pAosERegistration->Update();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, ReportFailureWhenRequestFakeModeCmdAndRegRequestedByESms)
{
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetECall(IMS_FALSE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_SAME_PCSCF);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, RequestFakeModeCmdWithSamePcscfReason_HandleFakeMode)
{
    EXPECT_CALL(m_objMockIAosPcscf, RemoveCurrentPcscf()).Times(0);

    m_pAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_SAME_PCSCF);

    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_TRUE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, RequestFakeModeCmdWithNextPcscfReason_HandleFakeMode)
{
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, RemoveCurrentPcscf()).Times(1);

    m_pAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF);

    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_TRUE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, ReportFailureWithGeneral_HandleFakeMode)
{
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosPcscf, RemoveCurrentPcscf());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF);

    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, ReportFailureWithNoPcscf_HandleFakeModeAndNoPcscf)
{
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsKeepEPdnUponPcscfUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, RemoveCurrentPcscf());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_NO_PCSCF_AVAILABLE));

    m_pAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF);

    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, RequestECallCmd_HandleECallState)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);

    EXPECT_TRUE(m_pAosERegistration->GetEModeInfo()->IsECall());
}

TEST_F(AosERegistrationTest, RequestECallCmdWhenEmergencyCallbackModeNotSupported)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosERegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsECall());
}

TEST_F(AosERegistrationTest, RequestESmsCmd_HandleESmsState)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencySmsOverImsSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->RequestCmd(IAosRegistration::CMD_ESMS_INIT);

    EXPECT_TRUE(m_pAosERegistration->GetEModeInfo()->IsESms());
}

TEST_F(AosERegistrationTest, RequestESmsCmdhenEmergencyCallbackModeNotSupported)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencySmsOverImsSupported())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosERegistration->RequestCmd(IAosRegistration::CMD_ESMS_INIT);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsESms());
}

TEST_F(AosERegistrationTest, RequestUnspecifiedCmd_ProcessedByAosRegistration)
{
    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(1);

    m_pAosERegistration->RequestCmd(
            IAosRegistration::CMD_INIT_PCSCF, IAosRegistration::REASON_INIT_PCSCF_CLEAR);
}

TEST_F(AosERegistrationTest, MsgRegReinitiate_ProcessReinitiate)
{
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    ImsMessage objMessage(TestAosERegistration::MSG_REG_REINITIATE, 0, 0);
    m_pAosERegistration->OnMessage(objMessage);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, MsgRegReinitiateWithRegState_ProcessReinitiateWithRegState)
{
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    ImsMessage objMessage(TestAosERegistration::MSG_REG_REINITIATE_WITH_REG_STATE, 1, 0);
    m_pAosERegistration->OnMessage(objMessage);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, ProcessReinitiateWithRegState_FailToCreateRegistration)
{
    AStringArray objEmptyImpus;
    ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillByDefault(ReturnRef(objEmptyImpus));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL))
            .Times(1);

    m_pAosERegistration->ProcessReinitiateWithRegState(IMS_TRUE);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, ProcessReinitiateWithRegStateWhenNotRegisteredBefore)
{
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosERegistration->ProcessReinitiateWithRegState(IMS_FALSE);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosERegistrationTest, UninterestedMsg_Ignored)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy()).Times(0);

    ImsMessage objMessage(TestAosERegistration::MSG_REG_EVENT_REGISTERED, 0, 0);
    EXPECT_TRUE(m_pAosERegistration->OnMessage(objMessage));
}

TEST_F(AosERegistrationTest, Initialize_SetListenersSuccessfully)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosEmergencyListener*>()));
    EXPECT_CALL(m_objMockIAosCallTracker, SetListener(m_pAosERegistration));
    EXPECT_CALL(m_objMockIAosTransaction, SetListener(_, _)).Times(1);

    m_pAosERegistration->Init();

    EXPECT_NE(m_pAosERegistration->GetEModeInfo(), nullptr);
}

TEST_F(AosERegistrationTest, CleanUp_RemoveListenersSuccessfully)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());

    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosEmergencyListener*>()));
    EXPECT_CALL(m_objMockIAosCallTracker, RemoveListener(m_pAosERegistration));
    EXPECT_CALL(m_objMockIAosTransaction, RemoveListener(_, _)).Times(1);

    m_pAosERegistration->CleanUp();
}

TEST_F(AosERegistrationTest, ReportFailureWhenAuthenticationFailedInCall)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->StartTimer(TestAosERegistration::TIMER_TRANSACTION, 10000);
    m_pAosERegistration->SetImsCall(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(1);

    m_pAosERegistration->ProcessAuthenticationFailed();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
    EXPECT_EQ(m_pAosERegistration->GetTransactionTimer(), nullptr);
}

TEST_F(AosERegistrationTest, StartFailedWithTxnTimeoutWhenReinitiationIsRequested)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetReinitiationRequested(IMS_TRUE);

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(0);

    m_pAosERegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosERegistrationTest, ReportFailureWhenStartFailedAndRegRequestedByESms)
{
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetECall(IMS_FALSE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(500);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, StartWithSpecifiedIntervalPolicytWhenRetryRuleForERegIsTrue)
{
    ON_CALL(m_objMockIAosNConfiguration, IsRegRetryRuleForERegUsed())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL));
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));

    ImsVector<IMS_SINT32> objInterval;
    objInterval.Add(10000);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .WillByDefault(ReturnRef(objInterval));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(400);

    EXPECT_EQ(m_pAosERegistration->GetInvokedCount(
                      "ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy"),
            1);
}

TEST_F(AosERegistrationTest, DoNotInvokedStartWithSpecifiedIntervalPolicytWhenIsRoaming)
{
    ON_CALL(m_objMockIAosNConfiguration, IsRegRetryRuleForERegUsed())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL));
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(403);

    EXPECT_EQ(m_pAosERegistration->GetInvokedCount(
                      "ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy"),
            0);
}

TEST_F(AosERegistrationTest, DefaultFlowRecoveryDuringStartWhenFakeRegistration)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetFakeReg(IMS_TRUE);

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL))
            .Times(1);

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(400);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, DefaultFlowRecoveryDuringStartWhenConfiguredAsFallback)
{
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));
    ON_CALL(m_objMockIAosNConfiguration, IsAnonymousECallActionSupported())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(400);

    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_TRUE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest,
        DefaultFlowRecoveryWhenPreferredEmergnecyRegistrationFallbackAndSupportAnonymousECallAction)
{
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));
    ON_CALL(m_objMockIAosNConfiguration, IsAnonymousECallActionSupported())
            .WillByDefault(Return(IMS_TRUE));

    ImsList<ISipMessageBodyPart*> objBodyParts;
    SipMessageBodyPart objBodyPart;
    ISipMessageBodyPart* piBodyPart = &objBodyPart;
    AString strContent = "";
    strContent.Append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    strContent.Append("<ims-3gpp version=\"1\"><alternative-service>");
    strContent.Append("<type>restoration</type>");
    strContent.Append("<reason></reason>");
    strContent.Append("<action>anonymous-emergencycall</action>");
    strContent.Append("</alternative-service></ims-3gpp>");
    ByteArray objContent(strContent);
    piBodyPart->SetContent(objContent);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, "application/3gpp-ims+xml");
    objBodyParts.Append(piBodyPart);
    ON_CALL(m_objMockISipMessage, GetBodyParts()).WillByDefault(Return(objBodyParts));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(403);

    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_TRUE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, DoNotProceedsFakeModeWhenSupportAnonymousECallActionButNoXmlBody)
{
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));
    ON_CALL(m_objMockIAosNConfiguration, IsAnonymousECallActionSupported())
            .WillByDefault(Return(IMS_TRUE));

    ImsList<ISipMessageBodyPart*> objBodyParts;
    ON_CALL(m_objMockISipMessage, GetBodyParts()).WillByDefault(Return(objBodyParts));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(403);

    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_NE(m_pAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, DoNotProceedsFakeModeWhenSupportAnonymousECallActionButNot403)
{
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));
    ON_CALL(m_objMockIAosNConfiguration, IsAnonymousECallActionSupported())
            .WillByDefault(Return(IMS_TRUE));

    ImsList<ISipMessageBodyPart*> objBodyParts;
    SipMessageBodyPart objBodyPart;
    ISipMessageBodyPart* piBodyPart = &objBodyPart;
    AString strContent = "";
    strContent.Append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    strContent.Append("<ims-3gpp version=\"1\"><alternative-service>");
    strContent.Append("<type>restoration</type>");
    strContent.Append("<reason></reason>");
    strContent.Append("<action>anonymous-emergencycall</action>");
    strContent.Append("</alternative-service></ims-3gpp>");
    ByteArray objContent(strContent);
    piBodyPart->SetContent(objContent);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, "application/3gpp-ims+xml");
    objBodyParts.Append(piBodyPart);
    ON_CALL(m_objMockISipMessage, GetBodyParts()).WillByDefault(Return(objBodyParts));

    m_pAosERegistration->ProcessDefaultFlowRecovery_Start(500);

    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_NE(m_pAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, DefaultFlowRecoveryDuringUpdateWhenNeitherEcbmNorScbm)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL))
            .Times(1);

    m_pAosERegistration->ProcessDefaultFlowRecovery_Update();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, StartRetryTimeIfPossibleToIncreaseCountWithRetryAfter)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    EXPECT_EQ(m_pAosERegistration->GetInvokedCount("StartTimer"), 1);
}

TEST_F(AosERegistrationTest, StartRetryTimerIfPossibleToIncreaseCountWithSpecifiedInterval)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Add(10000);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .WillOnce(ReturnRef(objInterval));

    m_pAosERegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);

    EXPECT_EQ(m_pAosERegistration->GetInvokedCount("StartTimer"), 1);
}

TEST_F(AosERegistrationTest, StartRetryTimerIfNotPossibleToIncreaseCountAndHasNextPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    EXPECT_EQ(m_pAosERegistration->GetInvokedCount("StartTimer"), 1);
}

TEST_F(AosERegistrationTest, StartRetryTimerIfNotPossibleToIncreaseCountAndNoPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    EXPECT_EQ(m_pAosERegistration->GetInvokedCount("StartTimer"), 1);
}

TEST_F(AosERegistrationTest, ReportFailureInternalIfNotPossibleToIncreaseCountAndNoPcscf)
{
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosERegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);
}

TEST_F(AosERegistrationTest, ReturnTrueWhenFollowingNoramlRulesWith305Policy3GPP)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP));

    EXPECT_TRUE(m_pAosERegistration->ProcessNormalDefaultFlowRecovery_Start(305));
}

TEST_F(AosERegistrationTest, ReturnFalseWhenFollowingNoramlRulesWith305PolicyDefault)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .WillByDefault(Return(CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT));
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Ims::AWT_POLICY_RFC_RULE));

    EXPECT_FALSE(m_pAosERegistration->ProcessNormalDefaultFlowRecovery_Start(305));
}

TEST_F(AosERegistrationTest, ReturnFalseWhenFollowingNoramlRuleAndSharedCntNotUsed)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .WillByDefault(Return(CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL));
    ON_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(m_pAosERegistration->ProcessNormalDefaultFlowRecovery_Start(400));
}

TEST_F(AosERegistrationTest, SetRegstopStateWhenStartFailedWith420AndNotSupportedGibaInRoaming)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGibaSupportedForERegInRoaming())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosERegistrationTest, SetRegstopStateWhenStartFailedWith420AndNotRoaming)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGibaSupportedForERegInRoaming())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosERegistrationTest, SetRegstopStateWhenStartFailedWith420AndNotIncludedSecAgree)
{
    ON_CALL(m_objMockIAosNConfiguration, IsGibaSupportedForERegInRoaming())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::UNSUPPORTED, _, _))
            .WillByDefault(Return(AString("sec-agree")));

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosERegistrationTest, SetRegisteringStateWhenStartFailedWith423)
{
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .WillByDefault(Return(AString("60")));

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosERegistrationTest, SetRegstopStateWhenStartFailedWith423AndRequiredNotSupportCommonPolicy)
{
    ImsVector<IMS_SINT32> objErrCodeNotSupporteCommonPolicy;
    objErrCodeNotSupporteCommonPolicy.Add(SipStatusCode::SC_423);
    ON_CALL(m_objMockIAosNConfiguration, GetERegErrCodeNotSupportedCommonPolicy())
            .WillByDefault(ReturnRef(objErrCodeNotSupporteCommonPolicy));

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic());
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));

    m_pAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosERegistrationTest, StartFailedWithOtherStatusCode)
{
    m_pAosERegistration->StartTimer(TestAosERegistration::TIMER_TRANSACTION, 10000);
    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(1);

    m_pAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_500);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
    EXPECT_EQ(m_pAosERegistration->GetTransactionTimer(), nullptr);
}

TEST_F(AosERegistrationTest, UpdateFailedWithStatusCode423)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    ON_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .WillByDefault(Return(AString("60")));

    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));

    m_pAosERegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosERegistrationTest, UpdateFailedWithOtherStatusCode)
{
    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);

    m_pAosERegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_500);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosERegistrationTest, UpdateFailedWithTxnTimeout)
{
    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);

    m_pAosERegistration->ProcessUpdateFailed_TxnTimeout();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosERegistrationTest, UpdateFailedWithOthers)
{
    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);

    m_pAosERegistration->ProcessUpdateFailed_Others(IRegistration::REASON_INTERNAL_ERROR);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosERegistrationTest, ClearCbmWhenModeTimerExpired)
{
    m_pAosERegistration->StartTimer(TestAosERegistration::TIMER_MODE, 3000);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetScbm(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->ProcessModeTimerExpired();

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsScbm());
    EXPECT_EQ(m_pAosERegistration->GetModeTimer(), nullptr);
}

TEST_F(AosERegistrationTest, TransactionTimerExpiredWhenNotRegisteringState)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGSTOP);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration()).Times(0);

    m_pAosERegistration->ProcessTransactionTimerExpired();
}

TEST_F(AosERegistrationTest, TransactionTimerExpiredWhenRetryIsAllowed)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetConsecutiveFailure(1);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt()).WillByDefault(Return(2));
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(3));
    ON_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillByDefault(Return(1));

    EXPECT_CALL(m_objUtilService.GetMockSystemProperty(), Set(_, _)).Times(0);
    EXPECT_CALL(m_objMockIRegistration, Register(_)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration()).Times(0);

    m_pAosERegistration->ProcessTransactionTimerExpired();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
}

TEST_F(AosERegistrationTest, TransactionTimerExpiredWhenRetryIsNotAllowedAndConfiguredAsFallback)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetConsecutiveFailure(2);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt()).WillByDefault(Return(2));
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    EXPECT_CALL(m_objUtilService.GetMockSystemProperty(), Set(_, _)).Times(0);

    m_pAosERegistration->ProcessTransactionTimerExpired();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_TRUE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest,
        TransactionTimerExpiredWhenRetryIsNotAllowedAndConfiguredAsFallbackWhenNwTimeout)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetConsecutiveFailure(2);
    m_pAosERegistration->SetImsReasonCode(AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt()).WillByDefault(Return(2));
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    EXPECT_CALL(m_objUtilService.GetMockSystemProperty(), Set(_, _));
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosERegistration->ProcessTransactionTimerExpired();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, TransactionTimerExpiredWhenRetryIsNotAllowedAndConfiguredAsNotFallback)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetConsecutiveFailure(2);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt()).WillByDefault(Return(2));
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));

    EXPECT_CALL(m_objUtilService.GetMockSystemProperty(), Set(_, _)).Times(0);
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL))
            .Times(1);

    m_pAosERegistration->ProcessTransactionTimerExpired();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_FALSE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_FALSE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, ReportFailureWhenTransactionTimerExpiredAndRegRequestedByESms)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetECall(IMS_FALSE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));

    m_pAosERegistration->ProcessTransactionTimerExpired();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, ProcessFakeModeeWhenTransactionTimerExpiredAndRegRequestedByECall)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetECall(IMS_TRUE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    m_pAosERegistration->ProcessTransactionTimerExpired();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pAosERegistration->IsFakeRegistration());
    EXPECT_TRUE(m_pAosERegistration->IsReinitiationRequested());
}

TEST_F(AosERegistrationTest, RegistrationRefreshTimerExpiredWhenRegistrationIsNull)
{
    m_pAosERegistration->Destroy();
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosERegistrationTest, RegistrationRefreshTimerExpiredWhenTransactionIsNotStarted)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());

    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillOnce(Return(IMS_TRUE));

    m_pAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_FALSE(m_pAosERegistration->IsTransactionStarted());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosERegistrationTest, RegistrationRefreshTimerExpiredButFailToCreateIpsec)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosERegistration->SetImsCall(IMS_TRUE);
    m_pAosERegistration->SetFeature(
            m_pAosERegistration->GetFeature() | TestAosERegistration::FEATURE_IPSEC);
    m_pAosERegistration->CreateIpsecHelper();

    m_pAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    m_pAosERegistration->DestroyIpsecHelper();
}

TEST_F(AosERegistrationTest, RegistrationRefreshTimerExpired_DoRefresh)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosERegistration->SetImsCall(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE))
            .Times(1);

    m_pAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);

    EXPECT_TRUE(bDoImplicitRefresh);
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosERegistrationTest, RegistrationStarted_ChangeStateToRegistered)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->StartTimer(TestAosERegistration::TIMER_TRANSACTION, 10000);

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE))
            .Times(1);

    m_pAosERegistration->Registration_Started();

    EXPECT_EQ(m_pAosERegistration->GetTransactionTimer(), nullptr);
    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosERegistrationTest, RegistrationUpdated_ChangeStateToRegistered)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REFRESHING);

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE))
            .Times(1);

    m_pAosERegistration->Registration_Updated();

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosERegistrationTest, RegistrationStartFailed_ChangeStateToRegstop)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(1);

    m_pAosERegistration->Registration_StartFailed(IRegistration::REASON_SERVER_SOCKET_ERROR);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosERegistrationTest, RegistrationStartFailed_failedWithOthers)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->StartTimer(TestAosERegistration::TIMER_TRANSACTION, 10000);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(0);

    m_pAosERegistration->Registration_StartFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_NE(m_pAosERegistration->GetTransactionTimer(), nullptr);
    EXPECT_EQ(m_pAosERegistration->GetImsReasonCode(), AosReasonCode::REG_RESP_NETWORK_TIMEOUT);
}

TEST_F(AosERegistrationTest, RegistrationTerminatedDuringCall_PendingTerminated)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosERegistration->SetImsCall(IMS_TRUE);
    m_pAosERegistration->SetFeature(
            m_pAosERegistration->GetFeature() | TestAosERegistration::FEATURE_IPSEC);
    m_pAosERegistration->CreateIpsecHelper();

    m_pAosERegistration->Registration_Terminated(IRegistration::REASON_NONE);

    IMS_BOOL bIsFeatureOn = m_pAosERegistration->GetUtil()->IsFeatureOn(
            TestAosERegistration::PENDING_TERMINATED, m_pAosERegistration->GetTxnPending());
    EXPECT_TRUE(bIsFeatureOn);

    m_pAosERegistration->DestroyIpsecHelper();
}

TEST_F(AosERegistrationTest, RegistrationTerminated_ChangeStateToOffline)
{
    m_pAosERegistration->SetRegistration(&m_objMockIRegistration);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(1);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED))
            .Times(1);

    m_pAosERegistration->Registration_Terminated(IRegistration::REASON_NONE);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, ClearCbmWhenRegistrationTerminated)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetEcbm(IMS_TRUE);
    m_pAosERegistration->GetEModeInfo()->SetScbm(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsScbm());
}

TEST_F(AosERegistrationTest, CallTrackerStateChangedForNonEmergencyType_Ignored)
{
    m_pAosERegistration->SetImsCall(IMS_FALSE);

    m_pAosERegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::NEW);

    EXPECT_FALSE(m_pAosERegistration->IsImsCall());
}

TEST_F(AosERegistrationTest, CallTrackerStateChangedForEmergencyType_UpdateCallStatus)
{
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);

    m_pAosERegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::NEW);

    EXPECT_TRUE(m_pAosERegistration->IsImsCall());
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, ClearCbmWhenCallTrackerStateChanged)
{
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetEcbm(IMS_TRUE);
    m_pAosERegistration->GetEModeInfo()->SetScbm(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::NEW);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsScbm());
}

TEST_F(AosERegistrationTest, NotifyConfigChangedWhenEmergencyCallbackModeSupported)
{
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosEmergencyListener*>()));

    m_pAosERegistration->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosERegistrationTest, NotifyConfigChangedWhenEmergencyCallbackModeNotSupported)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosEmergencyListener*>()));

    m_pAosERegistration->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosERegistrationTest, TransactionConnectionFailedWhenRegistered_Ignored)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pAosERegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_REGISTERED);
}

TEST_F(AosERegistrationTest, TransactionConnectionFailedWhenNotRegistered_Destroy)
{
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL))
            .Times(1);

    m_pAosERegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);

    EXPECT_EQ(m_pAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, TransactionConnectionSetupPrepared_DoNothing)
{
    m_pAosERegistration->Transaction_OnConnectionSetupPrepared();
}

TEST_F(AosERegistrationTest, TransactionTrafficPriorityChanged_DoNothing)
{
    m_pAosERegistration->Transaction_OnTrafficPriorityChanged();
}

TEST_F(AosERegistrationTest, SetCallFailureCauseToProperty_SetProperty)
{
    EXPECT_CALL(m_objUtilService.GetMockSystemProperty(), Set(_, _));

    m_pAosERegistration->SetCallFailureCauseToProperty(
            m_pAosERegistration->GetERegTimeoutFailureCause());
}

TEST_F(AosERegistrationTest, CallbackModeChangedWhenEmergencyCallbackModeNotSupported)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::START, 300);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsScbm());
}

TEST_F(AosERegistrationTest, EcbmIsTrueAndTimerIsStartedWhenCallbackModeChangedAsStartForCallType)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::START, 300);

    EXPECT_TRUE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsScbm());
    EXPECT_EQ(m_pAosERegistration->GetInvokedCount("StartTimer"), 1);
}

TEST_F(AosERegistrationTest, ScbmIsTrueCallbackModeChangedAsStartForSmsTypeDuringRegisteredState)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pAosERegistration->SetRegContact(&m_objMockIRegContact);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(700));

    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::SMS, EmergencyCallbackMode::START, 300);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
    EXPECT_TRUE(m_pAosERegistration->GetEModeInfo()->IsScbm());
    EXPECT_FALSE(m_pAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, EcbmIsFalseWhenStoppingEcbmCalled)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetEcbm(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::STOP, 300);

    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
}

TEST_F(AosERegistrationTest, ScbmIsOnlyFalseWhenStoppingScbmCalledAfterBothEcbmAndScbmAreStarted)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetEcbm(IMS_TRUE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::SMS, EmergencyCallbackMode::STOP, 300);

    EXPECT_TRUE(m_pAosERegistration->GetEModeInfo()->IsEcbm());
    EXPECT_FALSE(m_pAosERegistration->GetEModeInfo()->IsScbm());
}

TEST_F(AosERegistrationTest, RefreshIsRequiredByCbmIfESmsIsSet)
{
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetScbm(IMS_TRUE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    EXPECT_TRUE(m_pAosERegistration->IsRefreshRequiredByCbm());
}

TEST_F(AosERegistrationTest, RefreshIsRequiredByCbmWhenReRegTried)
{
    m_pAosERegistration->SetRegContact(&m_objMockIRegContact);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetReRegTryTime(IMS_SYS_GetTimeInSeconds() - 100);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(700));
    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::START, 300);

    EXPECT_TRUE(m_pAosERegistration->IsRefreshRequiredByCbm());
}

TEST_F(AosERegistrationTest, RefreshIsNotRequiredByCbmWhenWhenReRegTried)
{
    m_pAosERegistration->SetRegContact(&m_objMockIRegContact);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetReRegTryTime(IMS_SYS_GetTimeInSeconds() - 10);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(700));
    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::START, 300);

    EXPECT_FALSE(m_pAosERegistration->IsRefreshRequiredByCbm());
}

TEST_F(AosERegistrationTest, RefreshIsRequiredByCbmWhenWhenReRegNotTried)
{
    m_pAosERegistration->SetRegContact(&m_objMockIRegContact);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(400));
    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::START, 300);

    EXPECT_TRUE(m_pAosERegistration->IsRefreshRequiredByCbm());
}

TEST_F(AosERegistrationTest, RefreshIsNotRequiredByCbmWhenWhenReRegNotTried)
{
    m_pAosERegistration->SetRegContact(&m_objMockIRegContact);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIRegContact, GetExpires()).WillByDefault(Return(800));
    m_pAosERegistration->CallbackModeChanged(
            EmergencyCallbackModeType::CALL, EmergencyCallbackMode::START, 300);

    EXPECT_FALSE(m_pAosERegistration->IsRefreshRequiredByCbm());
}

TEST_F(AosERegistrationTest, FakeModeConditionIfEmergencyRegistrationSkipIsConfigured)
{
    ON_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP));

    EXPECT_TRUE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, FakeModeConditionIfSubscriberIncompleted)
{
    // GIVEN
    ON_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, NormalModeConditionIfEmergencyLteAttachForEmergencySms)
{
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_TRUE));
    m_pAosERegistration->SetImsCall(IMS_FALSE);
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    m_pAosERegistration->GetEModeInfo()->SetECall(IMS_FALSE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    EXPECT_FALSE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, FakeModeConditionIfEmergencyLteAttach)
{
    // GIVEN
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportERegWhenEAttachWithValidSim())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, FakeModeConditionIfEmergencyLteAttachWithNullSecuAlgoAndSupportEReg)
{
    // GIVEN
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportERegWhenEAttachWithValidSim())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosService, IsNasSecurityAlgorithmNull()).WillByDefault(Return(IMS_TRUE));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, NotFakeModeConditionIfEmergencyLteAttachAndEpdgConnected)
{
    // GIVEN
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportERegWhenEAttachWithValidSim())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, NotFakeModeConditionIfEmergencyLteAttachAndSupportEReg)
{
    // GIVEN
    ON_CALL(m_objMockIAosNetTracker, IsEmergencyAttach()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportERegWhenEAttachWithValidSim())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosService, IsNasSecurityAlgorithmNull()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, NotFakeModeConditionIfNotSupportLimitedAdminSmsMode)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, NotFakeModeConditionIf2ImpusAndNotPco5)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillByDefault(Return(IMS_FALSE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, FakeModeConditionIf1Impu)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("sip:1111@ims.co.kr"));
    ON_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillByDefault(ReturnRef(objImpus));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, FakeModeConditionIfPco5)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).WillByDefault(Return(IMS_TRUE));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosERegistration->IsFakeModeCondition());
}

TEST_F(AosERegistrationTest, IsRetryAllowedWhenPcscfAvailable)
{
    m_objPcscfs.AddElement(AString("192.168.0.101"));  // Adding 2nd P-CSCF
    m_pAosERegistration->SetConsecutiveFailure(1);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::EREG_RETRY_MAX_CNT_EVERY_PCSCF_RETRY));

    IMS_BOOL bResult = m_pAosERegistration->IsRetryAllowed();

    EXPECT_TRUE(bResult);
}

TEST_F(AosERegistrationTest, IsRetryAllowedWhenNoAvailablePcscfs)
{
    m_objPcscfs.AddElement(AString("192.168.0.101"));  // Adding 2nd P-CSCF
    m_pAosERegistration->SetConsecutiveFailure(2);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::EREG_RETRY_MAX_CNT_EVERY_PCSCF_RETRY));

    IMS_BOOL bResult = m_pAosERegistration->IsRetryAllowed();

    EXPECT_FALSE(bResult);
}

TEST_F(AosERegistrationTest, IsRetryAllowedWhenNotReachMaximumRetries)
{
    m_pAosERegistration->SetConsecutiveFailure(2);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt()).WillByDefault(Return(2));

    IMS_BOOL bResult = m_pAosERegistration->IsRetryAllowed();

    EXPECT_TRUE(bResult);
}

TEST_F(AosERegistrationTest, IsRetryAllowedWhenMaximumNumberOfRetries)
{
    m_pAosERegistration->SetConsecutiveFailure(3);
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt()).WillByDefault(Return(2));

    IMS_BOOL bResult = m_pAosERegistration->IsRetryAllowed();

    EXPECT_FALSE(bResult);
}

TEST_F(AosERegistrationTest, IsRetryAllowedWhenMaximumCountIsMinusOne)
{
    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryMaxCnt())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::EREG_RETRY_MAX_CNT_NO_RETRY));

    IMS_BOOL bResult = m_pAosERegistration->IsRetryAllowed();

    EXPECT_FALSE(bResult);
}

TEST_F(AosERegistrationTest, ProcessRearrangePcscfWhenRearrangeNotSupported)
{
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(0);

    m_pAosERegistration->ProcessRearrangePcscf();
}

TEST_F(AosERegistrationTest, ProcessRearrangePcscfWhenPcscfCountIsLessThanTwo)
{
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(0);

    m_pAosERegistration->ProcessRearrangePcscf();
}

TEST_F(AosERegistrationTest, ProcessRearrangePcscfWhenPcscfCountIsTwo)
{
    m_objPcscfs.AddElement(AString("192.168.0.101"));  // Adding 2nd P-CSCF
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(1);

    m_pAosERegistration->ProcessRearrangePcscf();
}

TEST_F(AosERegistrationTest, GetPreferredRegSchemeWhenRoamingSchemeIsConfigured)
{
    MockINetworkWatcher objMockINetworkWatcher;
    m_objPhoneInfoService.SetNetworkWatcher(&objMockINetworkWatcher);
    ON_CALL(objMockINetworkWatcher, GetRoamingState()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, GetRoamingPreferredEmcReg())
            .WillByDefault(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));

    EXPECT_EQ(m_pAosERegistration->GetPreferredRegScheme(),
            CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL);
}

TEST_F(AosERegistrationTest,
        ShouldNotSetReregFailureReportOnIpcanChangeRequiredValueIfTheConfigIsFalse)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyReregSupportedOnIpcanChange())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosERegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);

    // THEN
    EXPECT_FALSE(m_pAosERegistration->IsReregFailureReportOnIpcanChangeRequired());
}

TEST_F(AosERegistrationTest, ShouldSetReregFailureReportOnIpcanChangeRequiredValueIfTheConfigIsTrue)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyReregSupportedOnIpcanChange())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosERegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);

    // THEN
    EXPECT_TRUE(m_pAosERegistration->IsReregFailureReportOnIpcanChangeRequired());
}

TEST_F(AosERegistrationTest, ShouldUpdateTransactionStartedAsTrueIfCallIsActive)
{
    // GIVEN
    m_pAosERegistration->SetImsCall(IMS_TRUE);

    // WHEN
    m_pAosERegistration->UpdateTransactionStarted();

    // THEN
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, ShouldUpdateTransactionStartedAsTrueIfRefreshRequiredByCbm)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyCallbackModeSupported())
            .WillByDefault(Return(IMS_TRUE));
    m_pAosERegistration->SetEModeInfo(new EmergencyModeInfo());
    ASSERT_NE(m_pAosERegistration->GetEModeInfo(), nullptr);
    m_pAosERegistration->GetEModeInfo()->SetEcbm(IMS_TRUE);
    m_pAosERegistration->GetEModeInfo()->SetESms(IMS_TRUE);

    // WHEN
    m_pAosERegistration->UpdateTransactionStarted();

    // THEN
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, ShouldUpdateTransactionStartedAsTrueIfReregIsRequiredOnIpcanChange)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyReregSupportedOnIpcanChange())
            .WillByDefault(Return(IMS_TRUE));
    m_pAosERegistration->SetReregFailureReportOnIpcanChangeRequired(IMS_TRUE);

    // WHEN
    m_pAosERegistration->UpdateTransactionStarted();

    // THEN
    EXPECT_TRUE(m_pAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, ShouldUpdateTransactionStartedAsFalseIfNoActiveCallAndReregNotRequired)
{
    // WHEN
    m_pAosERegistration->UpdateTransactionStarted();

    // THEN
    EXPECT_FALSE(m_pAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest,
        ShouldSetNextPcscfWithoutRearrangeIfSinglePcscfFailedWhenTxnTimerExpired)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosERegistration->UpdateRegIpcanCategory();
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetConsecutiveFailure(0);

    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(1);

    // WHEN
    m_pAosERegistration->ProcessTransactionTimerExpired();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosERegistrationTest,
        ShouldSetNextPcscfWithoutRearrangeIf1Of3PcscfsFailedWhenTxnTimerExpired)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosERegistration->UpdateRegIpcanCategory();
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetConsecutiveFailure(0);
    m_objPcscfs.AddElement(AString("192.168.0.101"));
    m_objPcscfs.AddElement(AString("192.168.0.102"));

    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(1);

    // WHEN
    m_pAosERegistration->ProcessTransactionTimerExpired();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosERegistrationTest,
        ShouldSetNextPcscfWithoutRearrangeIf2Of3PcscfsFailedWhenTxnTimerExpired)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosERegistration->UpdateRegIpcanCategory();
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetConsecutiveFailure(1);
    m_objPcscfs.AddElement(AString("192.168.0.101"));
    m_objPcscfs.AddElement(AString("192.168.0.102"));

    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(1);

    // WHEN
    m_pAosERegistration->ProcessTransactionTimerExpired();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosERegistrationTest, ShouldSetNextPcscfWithRearrangeIfAllPcscfsFailedWhenTxnTimerExpired)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsEmcRegOnRandomPcscf()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    m_pAosERegistration->UpdateRegIpcanCategory();
    m_pAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pAosERegistration->SetConsecutiveFailure(2);
    m_objPcscfs.AddElement(AString("192.168.0.101"));
    m_objPcscfs.AddElement(AString("192.168.0.102"));

    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(1);

    // WHEN
    m_pAosERegistration->ProcessTransactionTimerExpired();

    // THEN: The GIVEN condition should be met.
}
