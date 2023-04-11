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

#include "AString.h"
#include "ImsMap.h"
#include "CarrierConfig.h"
#include "Credential.h"
#include "IImsRadio.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "SipStatusCode.h"

#include "../../../config/interface/ImsServiceConfig.h"
#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISipConfigV.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"
#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"
#include "../../../engine/interface/registration/MockIRegSubscription.h"

#include "../../../enabler/interface/aos/AoSAppRequestType.h"
#include "../../../enabler/interface/aos/ImsAosParameter.h"

#include "../../interface/aos/MockIAosService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
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
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

const IMS_SINT32 SLOT_ID = 0;

enum
{
    MSG_REG_START = AOSMSG_SERVICE_INTERNAL,

    MSG_REG_REINITIATE,
    MSG_REG_UPDATE,
    MSG_REG_RECONFIG,

    MSG_REG_REQUIRED_WITH_WAIT_TIME,
    MSG_REG_REQUIRED_WITH_NEXT_PCSCF,
    MSG_REG_REINITIATE_WITH_REG_STATE,
    MSG_REG_TERMINATED_BY_NOTIFY,

    MSG_SUB_REINITIATE,
    MSG_SUB_TERMINATED,

    MSG_REG_EVENT_REGISTERED
};

enum
{
    TIMER_OFFLINE_RECOVER = 100,
    TIMER_STOP_RETRY,
    TIMER_REFRESH,
    TIMER_EXPIRED,
    TIMER_MODE,
    TIMER_TRANSACTION,
    TIMER_INTERNAL_ERROR
};

enum
{
    FEATURE_NONE = 0x0,
    FEATURE_SUBSCRIPTION = 0x01,
    FEATURE_IPSEC = 0x02
};

enum
{
    PENDING_NONE = 0x0,
    PENDING_START = 0x1,
    PENDING_TRANSACTION = 0x2,
    PENDING_UPDATE = 0x4,
    PENDING_RECONFIG = 0x8,
    PENDING_UPDATE_HELD_BY_CALL = 0x10,
    PENDING_PLMN_BLOCK_HELD_BY_CALL = 0x20,
    PENDING_SUBSCRIPTION = 0x40,
    PENDING_TERMINATED = 0x80
};

enum
{
    IPSEC_BLOCK_NONE = 0x0,
    IPSEC_BLOCK_ERROR = 0x1,
    IPSEC_BLOCK_AUTENTICATION = 0x2,
    IPSEC_BLOCK_NOT_ESTABLISHED = 0x4,
    IPSEC_BLOCK_ROAMING = 0x8
};

class TestAosIpsecHelper : public AosIpsecHelper
{
    inline explicit TestAosIpsecHelper(IN IRegContact* piContact, IN IRegParameter* piParameter,
            IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosIpsecHelper(piContact, piParameter, piAppContext, strRegId)
    {
    }

    friend class AosRegistrationTest;

public:
    inline IMS_BOOL ProcessAuthChallenged(IN IMS_SINT32) override { return IMS_FALSE; }
    inline IMS_BOOL Create(IN IMS_BOOL) override { return IMS_FALSE; }
    inline void CreateOnChallenging() override {}
    inline IMS_BOOL SetPcscfPortnSpi() override { return IMS_TRUE; }
    inline IMS_BOOL IsPcscfServerPortDifferent() override { return IMS_TRUE; }
    inline IMS_BOOL UpdatePreloadedRoute(IN const AString&) { return IMS_TRUE; }
    inline IMS_BOOL MakeSas(IN const AString&, IN const IpAddress&, IN const ByteArray&,
            IN const ByteArray&) override
    {
        return IMS_TRUE;
    }
    inline IMS_BOOL IsEstablished() override { return IMS_FALSE; }
    inline IMS_BOOL ProcessRegUpdated() override { return IMS_FALSE; }
};

class TestAosSubscription : public AosSubscription
{
    inline explicit TestAosSubscription(IN IAosAppContext* piContext,
            IN IRegSubscription* piRegSubscription, IN const AString& strAor,
            IN const SipAddress& objContactAddress) :
            AosSubscription(piContext, piRegSubscription, strAor, objContactAddress)
    {
    }

    friend class AosRegistrationTest;

public:
    inline void Initialize() override {}
    inline void SetListener(IN IAosSubscriptionListener*) override {}
    inline void Destroy() override {}
    inline IMS_BOOL Start(IN IMS_BOOL) override { return IMS_FALSE; }
    inline void Stop() override {}
};

class TestAosRegistration : public AosRegistration
{
    inline TestAosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosRegistration(piAppContext, strRegId),
            m_piMockRegistration(IMS_NULL),
            m_pAosSubscription(IMS_NULL)
    {
    }

    friend class AosRegistrationTest;
    FRIEND_TEST(AosRegistrationTest, Start);
    FRIEND_TEST(AosRegistrationTest, Destroy);
    FRIEND_TEST(AosRegistrationTest, Stop);
    FRIEND_TEST(AosRegistrationTest, Update);
    FRIEND_TEST(AosRegistrationTest, Reconfig);
    FRIEND_TEST(AosRegistrationTest, RequestCmd);
    FRIEND_TEST(AosRegistrationTest, CheckMode);
    FRIEND_TEST(AosRegistrationTest, GetProperty);
    FRIEND_TEST(AosRegistrationTest, CheckBool);
    FRIEND_TEST(AosRegistrationTest, FeatureTagForMtc);
    FRIEND_TEST(AosRegistrationTest, BlockChanged);
    FRIEND_TEST(AosRegistrationTest, IpsecSupported);
    FRIEND_TEST(AosRegistrationTest, UpdatePreloadedRoute);
    FRIEND_TEST(AosRegistrationTest, RetryCount);
    FRIEND_TEST(AosRegistrationTest, SpecificOperation);
    FRIEND_TEST(AosRegistrationTest, GetNetworkTypeForImsRegState);
    FRIEND_TEST(AosRegistrationTest, IsGeolocationInfoRequired);
    FRIEND_TEST(AosRegistrationTest, StartFailed_TxnTimeout);
    FRIEND_TEST(AosRegistrationTest, UpdateFailed_TxnTimeout);
    FRIEND_TEST(AosRegistrationTest, UpdateTransactionStarted);
    FRIEND_TEST(AosRegistrationTest, StandardPcscfSelection);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpired);
    FRIEND_TEST(AosRegistrationTest, Terminated);
    FRIEND_TEST(AosRegistrationTest, SendRegisterEx);
    FRIEND_TEST(AosRegistrationTest, UpdateFeatureTag);
    FRIEND_TEST(AosRegistrationTest, SetStaticIpQos);
    FRIEND_TEST(AosRegistrationTest, SetDynamicIpQos);
    FRIEND_TEST(AosRegistrationTest, GetActualWaitTime);
    FRIEND_TEST(AosRegistrationTest, TryNextPcscf);
    FRIEND_TEST(AosRegistrationTest, SetNextPcscf_SamePcscf);
    FRIEND_TEST(AosRegistrationTest, IsRetryOnSamePcscfRequired);
    FRIEND_TEST(AosRegistrationTest, OnMessage);
    FRIEND_TEST(AosRegistrationTest, CheckPending);
    FRIEND_TEST(AosRegistrationTest, ProcessPendingTransaction);
    FRIEND_TEST(AosRegistrationTest, ProcessRetryInRegStopped);
    FRIEND_TEST(AosRegistrationTest, ProcessReregister);
    FRIEND_TEST(AosRegistrationTest, ProcessRegTerminated);
    FRIEND_TEST(AosRegistrationTest, ProcessAuthenticationFailed);
    FRIEND_TEST(AosRegistrationTest, ProcessForbiddenFailed);
    FRIEND_TEST(AosRegistrationTest, ProcessRegRequiredWithAvailableNextPcscf);
    FRIEND_TEST(AosRegistrationTest, ProcessSubscriberFailed);
    FRIEND_TEST(AosRegistrationTest, ProcessSetIpsec);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_Start);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_Update);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountPerPcscfConfigured);
    FRIEND_TEST(AosRegistrationTest,
            ProcessStartFailed_TxnTimeout_RegRetryCountOnSinglePcscfConfigured);
    FRIEND_TEST(AosRegistrationTest, ProcessIpVersionChange);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_ProcessStartFailed_305);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Success);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Failure);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_HasNextPcscf);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_Others);
    FRIEND_TEST(AosRegistrationTest, ProcessUpdateFailed_StatusCode);
    FRIEND_TEST(AosRegistrationTest, ProcessUpdateFailed_StatusCode_ProcessUpdateFailed_305);
    FRIEND_TEST(AosRegistrationTest, Registration_AuthenticationChallenged);
    FRIEND_TEST(AosRegistrationTest, Registration_NotifyAkaResponse);
    FRIEND_TEST(AosRegistrationTest, Registration_Started);
    FRIEND_TEST(AosRegistrationTest, Registration_StartFailed);
    FRIEND_TEST(AosRegistrationTest, Registration_Updated);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_CallEndByReregErr);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming_HeldByCall);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_ProcessUpdateFailed_Others);
    FRIEND_TEST(AosRegistrationTest, Registration_Removed);
    FRIEND_TEST(AosRegistrationTest, StopTimer);
    FRIEND_TEST(AosRegistrationTest, ClearTimer);
    FRIEND_TEST(AosRegistrationTest, TimerExpired);
    FRIEND_TEST(AosRegistrationTest, ProcessOfflineRecoverTimerExpired);
    FRIEND_TEST(AosRegistrationTest, ProcessStopRetryTimerExpired);
    FRIEND_TEST(AosRegistrationTest, CreateAndDestroySubscription);
    FRIEND_TEST(AosRegistrationTest, Subscription_StateChanged);
    FRIEND_TEST(AosRegistrationTest, Subscription_CanBeTransmitted);
    FRIEND_TEST(AosRegistrationTest, Subscription_NotifyReceived);
    FRIEND_TEST(AosRegistrationTest, Subscription_Request);
    FRIEND_TEST(AosRegistrationTest, NConfiguration_NotifyConfigChanged);
    FRIEND_TEST(AosRegistrationTest, Transaction_OnConnectionFailed);
    FRIEND_TEST(AosRegistrationTest, MessageMediator_AdjustMessage);
    FRIEND_TEST(AosRegistrationTest, AddLocationHeaderBody);

protected:
    inline IRegistration* GetRegistration() override { return m_piMockRegistration; }
    inline void CreateIpsecHelper() override {}
    inline void DestroyIpsecHelper() override {}
    inline AosSubscription* GetSubscription(IN IRegSubscription*) override
    {
        return m_pAosSubscription;
    }

public:
    inline void SetMockIRegistration(IN IRegistration* piRegistration)
    {
        m_piMockRegistration = piRegistration;
    }

    inline void SetIRegistration(IN IRegistration* piRegistration)
    {
        m_piRegistration = piRegistration;
    }

    inline void SetIRegContact(IN IRegContact* piRegContact) { m_piRegContact = piRegContact; }

    inline void SetRegType(IN AosRegistrationType eRegType) { m_eRegType = eRegType; }

    inline void SetTransactionStarted(IN IMS_BOOL bIsTransactionStarted)
    {
        m_bIsTransactionStarted = bIsTransactionStarted;
    }

    inline void SetIpsecHelper(IN AosIpsecHelper* pIpsecHelper) { m_pIpsecHelper = pIpsecHelper; }

    inline void SetAosSubscription(IN AosSubscription* pAosSubscription)
    {
        m_pSubscription = pAosSubscription;
        m_pAosSubscription = pAosSubscription;
    }

    inline IAosRegistrationListener* GetListener() { return m_piListener; }

    inline void UpdateTxnPending(IN IMS_UINT32 nFeature, IN IMS_BOOL bEnable)
    {
        if (bEnable)
        {
            m_pUtil->AddFeature(nFeature, m_nTxnPending);
        }
        else
        {
            m_pUtil->RemoveFeature(nFeature, m_nTxnPending);
        }
    }

    inline IMS_BOOL IsTxnPendingOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nTxnPending);
    }

    inline IMS_BOOL IsFeatureOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nFeature);
    }

    inline void SetISipConfigV(ISipConfigV* piSipConfigV) { m_pUtil->SetISipConfigV(piSipConfigV); }

    inline IMS_BOOL IsRetryCountClear() { return m_nConsecutiveFailure == 0; }

    inline IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType)
    {
        if (nType == TIMER_OFFLINE_RECOVER)
        {
            return (m_piOfflineRecoverTimer != IMS_NULL);
        }

        if (nType == TIMER_STOP_RETRY)
        {
            return (m_piStopRetryTimer != IMS_NULL);
        }

        if (nType == TIMER_REFRESH)
        {
            return (m_piRefreshTimer != IMS_NULL);
        }

        if (nType == TIMER_EXPIRED)
        {
            return (m_piExpiredTimer != IMS_NULL);
        }

        if (nType == TIMER_MODE)
        {
            return (m_piModeTimer != IMS_NULL);
        }

        if (nType == TIMER_TRANSACTION)
        {
            return (m_piTransactionTimer != IMS_NULL);
        }

        if (nType == TIMER_INTERNAL_ERROR)
        {
            return (m_piInternalErrorTimer != IMS_NULL);
        }

        return IMS_FALSE;
    }

private:
    IRegistration* m_piMockRegistration;
    AosSubscription* m_pAosSubscription;
};

class AosRegistrationTest : public ::testing::Test
{
public:
    TestAosRegistration* m_pTestAosRegistration;
    TestAosIpsecHelper* m_pTestAosIpsecHelper;
    TestAosSubscription* m_pTestAosSubscription;
    AosStaticProfile* m_pAosStaticProfile;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosRetryRepository* m_piAosRetryRepository;
    IAosService* m_piAosService;
    IAosTransaction* m_piAosTransaction;

    MockISipConfigV m_objMockISipConfigV;
    MockIConfigurable m_objMockIConfigurable;
    MockISipMessage m_objMockISipMessage;
    MockIRegistration m_objMockIRegistration;
    MockIRegContact m_objMockIRegContact;
    MockIRegParameter m_objMockIRegParameter;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
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

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    SipAddress m_objSipAddress = SipAddress("sip:1111@1.1.1.1");
    ImsMap<AString, IAosHandle*> m_objHandles;
    AStringArray m_objImpus;
    AStringArray m_objPcscfs;
    ImsList<IMS_SINT32> m_objPcscfPorts;
    AosFeatureTagList m_objFeatureTagList;
    AosFeatureTagList m_objBindedList;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

        ImsList<ImsServiceName> objServiceName =
                ImsServiceConfig::GetServiceNames(ImsServiceConfig::GetServiceProfile());

        for (IMS_UINT32 i = 0; i < objServiceName.GetSize(); i++)
        {
            ImsServiceName objService = objServiceName.GetAt(i);
            m_pAosStaticProfile->AddService(objService.GetAppId(), objService.GetServiceId());
        }

        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(SLOT_ID));
        EXPECT_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_pAosStaticProfile));

        m_pTestAosRegistration =
                new TestAosRegistration(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetRegistrationId());

        m_pTestAosIpsecHelper =
                new TestAosIpsecHelper(static_cast<IRegContact*>(&m_objMockIRegContact),
                        static_cast<IRegParameter*>(&m_objMockIRegParameter),
                        static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetRegistrationId());
        m_pTestAosRegistration->SetIpsecHelper(m_pTestAosIpsecHelper);

        m_pTestAosSubscription =
                new TestAosSubscription(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        static_cast<IRegSubscription*>(&m_objMockIRegSubscription),
                        AString("sip:1234@ims.google.com:5060"), SipAddress());
        m_pTestAosRegistration->SetAosSubscription(m_pTestAosSubscription);

        m_piAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockIAosService), SLOT_ID);

        m_piAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(
                static_cast<IAosTransaction*>(&m_objMockIAosTransaction), SLOT_ID);

        EXPECT_CALL(m_objMockIAosTransaction, StartTraffic(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration), SLOT_ID);

        m_piAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(
                static_cast<IAosRetryRepository*>(&m_objMockIAosRetryRepository), SLOT_ID);

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
                .Times(AnyNumber())
                .WillRepeatedly(Return(30000));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
                .Times(AnyNumber())
                .WillRepeatedly(Return(1800000));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsContactUriValidationChecked())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPrivateHeader())
                .Times(AnyNumber())
                .WillRepeatedly(
                        Return(CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
                .Times(AnyNumber())
                .WillRepeatedly(
                        Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(46));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetSipPreferredTransport())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsUserInfoInContactSupported())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetSipMessageThresholdForTransportChange())
                .Times(AnyNumber())
                .WillRepeatedly(Return(340));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosSubscriber));

        m_objImpus.AddElement(AString("sip:1111@ims.co.kr"));
        m_objImpus.AddElement(AString("sip:2222@ims.co.kr"));
        EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objImpus));

        EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosPcscf));

        m_objPcscfs.AddElement(AString("192.168.0.100"));
        m_objPcscfs.AddElement(AString("192.168.0.101"));
        m_objPcscfs.AddElement(AString("192.168.0.102"));
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPcscfs));

        m_objPcscfPorts.Append(5060);
        m_objPcscfPorts.Append(5061);
        m_objPcscfPorts.Append(5062);
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfsPorts())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPcscfPorts));

        EXPECT_CALL(m_objMockIAosPcscf, GetCurrentIndex())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        EXPECT_CALL(m_objMockIAosPcscf, HasPcscf(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        m_objHandles.Add(m_strServiceId, &m_objMockIAosHandle);
        EXPECT_CALL(m_objMockIAosAppContext, GetHandles())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objHandles));

        EXPECT_CALL(m_objMockIAosHandle, GetAppId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_strAppId));

        EXPECT_CALL(m_objMockIAosHandle, GetServiceId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_strServiceId));

        EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objFeatureTagList));

        EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objBindedList));

        EXPECT_CALL(m_objMockIAosHandle, GetServiceType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<IMS_UINT32>(ImsAosService::MTS)));

        EXPECT_CALL(m_objMockIAosHandle, IsRegBinded())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IIpcan::CATEGORY_MOBILE));

        EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosConnection, GetMtu())
                .Times(AnyNumber())
                .WillRepeatedly(Return(1400));

        EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(IpAddress::LOOPBACK));

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosNetTracker));

        EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosBlock));

        EXPECT_CALL(m_objMockISipConfigV, GetConfigurable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIConfigurable));

        EXPECT_CALL(m_objMockIConfigurable, AddListener(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIConfigurable, Update(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIRegistration, Register(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_SUCCESS));

        EXPECT_CALL(m_objMockIRegistration, Deregister())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_SUCCESS));

        EXPECT_CALL(m_objMockIRegistration, CreateContact(_, _, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIRegContact));

        EXPECT_CALL(m_objMockIRegistration, GetPreferredContact())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIRegContact));

        EXPECT_CALL(m_objMockIRegContact, GetContactAddress())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objSipAddress));

        EXPECT_CALL(m_objMockIRegistration, GetPreviousRequest())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockISipMessage));

        EXPECT_CALL(m_objMockIRegistration, GetPreviousResponse())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockISipMessage));

        EXPECT_CALL(m_objMockIRegistration, GetParameter())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIRegParameter));

        EXPECT_CALL(m_objMockIRegParameter, AddPreloadedRoute(_, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        m_pTestAosRegistration->SetMockIRegistration(
                static_cast<IRegistration*>(&m_objMockIRegistration));
        m_pTestAosRegistration->SetListener(
                static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_piAosRetryRepository, SLOT_ID);

        if (m_pTestAosIpsecHelper)
        {
            delete m_pTestAosIpsecHelper;
            m_pTestAosIpsecHelper = IMS_NULL;
        }

        if (m_pTestAosSubscription)
        {
            delete m_pTestAosSubscription;
            m_pTestAosSubscription = IMS_NULL;
        }

        if (m_pTestAosRegistration)
        {
            delete m_pTestAosRegistration;
            m_pTestAosRegistration = IMS_NULL;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
            m_pAosStaticProfile = IMS_NULL;
        }
    }
};

TEST_F(AosRegistrationTest, Start)
{
    // transaction is not started
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->SetTransactionStarted(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(_)).Times(0);
    m_pTestAosRegistration->Start();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_START));

    // txn is pending due to radio
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->SetTransactionStarted(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->Start();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // fail to create registration
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_CALL(m_objMockIRegistration, CreateContact(_, _, _, _))
            .Times(AnyNumber())
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&m_objMockIRegContact));
    m_pTestAosRegistration->Start();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // start successfully
    EXPECT_CALL(m_objMockIRegistration, SetSipMessageMediator(_)).Times(1);
    m_pTestAosRegistration->Start();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, Destroy)
{
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(1);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Destroy();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, Stop)
{
    // STATE_REGISTERING
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE))
            .Times(1);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Stop();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // STATE_REGISTERED - fail to send DeRegister
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, Deregister())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FAILURE))
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE))
            .Times(1);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Stop();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // STATE_REGISTERED - succeed to send DeRegister
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_STOP))
            .Times(1);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Stop();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);

    // STATE_DEREGISTERING - do nothing
    m_pTestAosRegistration->Stop();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);
}

TEST_F(AosRegistrationTest, Update)
{
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    // Update()
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->Update(IMS_FALSE);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Update(IMS_FALSE);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Update();

    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    m_pTestAosRegistration->Destroy();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
}

TEST_F(AosRegistrationTest, Reconfig)
{
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));

    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    ImsMap<AString, IAosHandle*> objHandles;
    const ImsList<AosServiceProfile*>& objProfiles = m_pAosStaticProfile->GetServiceProfiles();

    if (objProfiles.GetSize() > 0)
    {
        AosServiceProfile* pServiceProfile = objProfiles.GetAt(0);
        objHandles.Add(pServiceProfile->GetServiceId(), &m_objMockIAosHandle);
        EXPECT_CALL(m_objMockIAosHandle, GetAppId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(pServiceProfile->GetAppId()));
        EXPECT_CALL(m_objMockIAosHandle, GetServiceId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(pServiceProfile->GetServiceId()));
    }

    EXPECT_CALL(m_objMockIAosAppContext, GetHandles())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objHandles));

    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosHandle::DETACH));

    EXPECT_CALL(m_objMockIRegistration, DestroyBinding(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIRegContact, RemoveService(_, _)).Times(AnyNumber());

    // Reconfig()
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->Reconfig();

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Reconfig();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Reconfig();

    m_pTestAosRegistration->SetIRegContact(IMS_NULL);
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    m_pTestAosRegistration->Destroy();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // STATE_OFFLINE - It doesn't do anything
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->Reconfig();

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
}

TEST_F(AosRegistrationTest, SetListener)
{
    m_pTestAosRegistration->SetListener(IMS_NULL);

    EXPECT_EQ(m_pTestAosRegistration->GetListener(), nullptr);

    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

    EXPECT_EQ(m_pTestAosRegistration->GetListener(),
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));
}

TEST_F(AosRegistrationTest, RequestCmd)
{
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));

    m_pTestAosRegistration->RequestCmd(
            IAosRegistration::CMD_INIT_PCSCF, IAosRegistration::REASON_INIT_PCSCF_CLEAR);

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_INIT_AWT);

    // CMD_CLEAR_RETRY_COUNT - ClearRetryCount
    m_pTestAosRegistration->IncreaseConsecutiveFailCount();
    EXPECT_FALSE(m_pTestAosRegistration->IsRetryCountClear());
    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT);
    EXPECT_TRUE(m_pTestAosRegistration->IsRetryCountClear());

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_REFRESH_REGINFO);

    // CMD_UPDATE_REG_BINDING - UpdateRegBinding
    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_REG_BINDING);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithIpcanChangedDuringImsCallHeld())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);

    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockIAosPcscf, RemoveCurrentPcscf()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);
    m_pTestAosRegistration->SetListener(IMS_NULL);

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT);

    m_pTestAosRegistration->SetIRegContact(IMS_NULL);

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);

    // CMD_UPDATE_IPCAN
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_FALSE(m_pTestAosRegistration->IsBlocked());

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_UPDATE_IPCAN);

    EXPECT_TRUE(m_pTestAosRegistration->IsBlocked());
    EXPECT_FALSE(m_pTestAosRegistration->IsTransactionStarted());

    // CMD_UNAVAILABLE_FEATURE_TAG - UpdateDetailState - GetRegFeatures
    m_pTestAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;
    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG);
    EXPECT_EQ(m_pTestAosRegistration->m_nImsRegState,
            TestAosRegistration::IMS_REG_STATE_DEREGISTERED);

    // CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    IMS_UINT32 nCurrentCnt = m_pTestAosRegistration->m_nConsecutiveFailureForPdnReactivated;
    m_pTestAosRegistration->RequestCmd(
            IAosRegistration::CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED);
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailureForPdnReactivated, nCurrentCnt + 1);

    // CMD_SET_EPS_5GS_ONLY
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_FALSE;
    m_pTestAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_EPS_5GS_ONLY, AosRegistration::REASON_SET_ENABLE);
    EXPECT_TRUE(m_pTestAosRegistration->m_bEps5GsOnly);

    // CMD_ECALL_INIT - It doesn't do anything
    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);
}

TEST_F(AosRegistrationTest, CheckMode)
{
    m_pTestAosRegistration->SetMode(IAosRegistration::MODE_NORMAL);
    EXPECT_EQ(m_pTestAosRegistration->GetMode(), IAosRegistration::MODE_NORMAL);

    m_pTestAosRegistration->SetMode(IAosRegistration::MODE_FAKE);
    EXPECT_EQ(m_pTestAosRegistration->GetMode(), IAosRegistration::MODE_FAKE);
}

TEST_F(AosRegistrationTest, GetProperty)
{
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    IMS_UINT32 nValue;
    AString strValue;

    // PROPERTY_LOCAL_PORT
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_PORT, nValue, strValue);
    EXPECT_EQ(nValue, m_pTestAosRegistration->m_pUtil->GetLocalPort(SLOT_ID));

    // PROPERTY_LOCAL_ADDRESS
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), m_pTestAosRegistration->m_objIpa.ToString().GetStr());

    // PROPERTY_PCSCF_PORT
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_PCSCF_PORT, nValue, strValue);
    EXPECT_EQ(nValue, m_pTestAosRegistration->m_nPcscfPort);

    // PROPERTY_PCSCF_ADDRESS
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_PCSCF_ADDRESS, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), m_pTestAosRegistration->m_strPcscf.GetStr());

    // PROPERTY_ASSOCIATED_URI
    AStringArray objUris;
    objUris.AddElement(AString("TestUri"));
    EXPECT_CALL(m_objMockIRegistration, GetAssociatedUris()).WillOnce(ReturnRef(objUris));
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_ASSOCIATED_URI, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestUri");

    // PROPERTY_PATH
    AString strPath = AString("TestPath");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::PATH, _, _))
            .Times(AnyNumber())
            .WillOnce(Return(strPath));
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_PATH, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestPath");

    // PROPERTY_SERVICE_ROUTE
    AString strServiceRoute = AString("TestServiceRoute");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::SERVICE_ROUTE, _, _))
            .Times(AnyNumber())
            .WillOnce(Return(strServiceRoute));
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_SERVICE_ROUTE, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestServiceRoute");

    // PROPERTY_LAST_PATH
    ImsList<AString> strPaths;
    strPaths.Append(AString("TestLastPath"));
    EXPECT_CALL(m_objMockISipMessage, GetHeaders(ISipHeader::PATH, _))
            .Times(AnyNumber())
            .WillOnce(Return(strPaths));
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_LAST_PATH, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestLastPath");

    // PROPERTY_SUPPORTED
    AString strSupported = AString("TestSupported");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::SUPPORTED, _, _))
            .Times(AnyNumber())
            .WillOnce(Return(strSupported));
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_SUPPORTED, nValue, strValue);
    EXPECT_STREQ(strValue.GetStr(), "TestSupported");

    // PROPERTY_PROTECTED - IpcanCategory is CATEGORY_MOBILE
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AoSRegProtectedType::REG_UNPROTECTED);

    // PROPERTY_PROTECTED - IpcanCategory is CATEGORY_WLAN
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIpcan::CATEGORY_WLAN));
    m_pTestAosRegistration->UpdateRegIpcanCategory();
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AoSRegProtectedType::REG_PROTECTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION - m_bCallingNumberVerificationSupported if false
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AoSSupportability::NOT_SUPPORTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION - m_bCallingNumberVerificationSupported if true
    m_pTestAosRegistration->m_bCallingNumberVerificationSupported = true;
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AoSSupportability::SUPPORTED);

    // PROPERTY_NETWORK_BINDING_FEATURES
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_NETWORK_BINDING_FEATURES, nValue, strValue);
    EXPECT_EQ(nValue, m_pTestAosRegistration->m_nNetworkBindingFeatures);

    // PROPERTY_PDN_REACIVATE_WAIT_TIME
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_PDN_REACIVATE_WAIT_TIME, nValue, strValue);
    EXPECT_EQ(nValue, 30);

    // PROPERTY_REG_FAILURE_COUNT
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_REG_FAILURE_COUNT, nValue, strValue);
    EXPECT_EQ(nValue, 0);
}

TEST_F(AosRegistrationTest, CheckBool)
{
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pTestAosRegistration->IsRegistered());

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    EXPECT_TRUE(m_pTestAosRegistration->IsRefreshing());

    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 10000);
    EXPECT_TRUE(m_pTestAosRegistration->IsRetryTimer());
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_TRUE(m_pTestAosRegistration->IsRetryHeld());
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    m_pTestAosRegistration->UpdateTxnPending(PENDING_TERMINATED, IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTerminated());
    m_pTestAosRegistration->UpdateTxnPending(PENDING_TERMINATED, IMS_FALSE);

    m_pTestAosRegistration->SetAppReady(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsAppReady());
    m_pTestAosRegistration->SetAppReady(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosRegistration->IsAppReady());

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
}

TEST_F(AosRegistrationTest, FeatureTagForMtc)
{
    m_pTestAosRegistration->SetISipConfigV(static_cast<ISipConfigV*>(&m_objMockISipConfigV));
    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));

    EXPECT_CALL(m_objMockISipConfigV, GetFeatureTagOptions())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISipConfigV::FEATURE_TAG_MEDIA_STREAM));

    EXPECT_TRUE(m_pTestAosRegistration->AddFeatureTagForMtc(ImsAosFeature::VIDEO, IMS_FALSE));
    EXPECT_TRUE(m_pTestAosRegistration->AddFeatureTagForMtc(ImsAosFeature::TEXT, IMS_FALSE));
    EXPECT_TRUE(m_pTestAosRegistration->AddFeatureTagForMtc(
            ImsAosFeature::VIDEO | ImsAosFeature::TEXT, IMS_FALSE));

    EXPECT_CALL(m_objMockISipConfigV, GetFeatureTagOptions())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO |
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT));

    EXPECT_TRUE(m_pTestAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_pTestAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::TEXT));
    EXPECT_TRUE(m_pTestAosRegistration->RemoveFeatureTagForMtc(
            ImsAosFeature::VIDEO | ImsAosFeature::TEXT));

    EXPECT_CALL(m_objMockIRegContact,
            AddExtraCapability(
                    AString(FeatureTags::CALL_COMPOSER_VIA_TELEPHONY), AString::ConstNull()))
            .Times(1);
    EXPECT_CALL(m_objMockIRegContact,
            RemoveExtraCapability(
                    AString(FeatureTags::CALL_COMPOSER_VIA_TELEPHONY), AString::ConstNull()))
            .Times(1);
    m_pTestAosRegistration->AddFeatureTagForMtc(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY, IMS_FALSE);
    m_pTestAosRegistration->AddFeatureTagForMtc(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY, IMS_TRUE);
    m_pTestAosRegistration->RemoveFeatureTagForMtc(ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY);

    m_pTestAosRegistration->SetIRegContact(IMS_NULL);
    m_pTestAosRegistration->SetISipConfigV(IMS_NULL);
}

TEST_F(AosRegistrationTest, BlockChanged)
{
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosRegistration->SetAppReady(IMS_FALSE);

    EXPECT_FALSE(m_pTestAosRegistration->IsBlocked());

    m_pTestAosRegistration->AosRegistration::Block_Changed();

    EXPECT_TRUE(m_pTestAosRegistration->IsBlocked());
    EXPECT_FALSE(m_pTestAosRegistration->IsTransactionStarted());
}

TEST_F(AosRegistrationTest, IpsecSupported)
{
    // precondition for checking Ipsec supported - IsFakeRegistration()
    m_pTestAosRegistration->SetFakeReg(IMS_FALSE);

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // FEATURE_IPSEC is not set
    m_pTestAosRegistration->InitFeatures();

    m_pTestAosRegistration->ClearIpsecBlock();
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_ERROR);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_ERROR);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    // FEATURE_IPSEC is set
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_ERROR);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_AUTENTICATION);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_AUTENTICATION);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_ERROR);
    EXPECT_TRUE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_ROAMING);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_FALSE, IPSEC_BLOCK_NOT_ESTABLISHED);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_NOT_ESTABLISHED);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    m_pTestAosRegistration->UpdateIpsecSupported(IMS_TRUE, IPSEC_BLOCK_ROAMING);
    EXPECT_TRUE(m_pTestAosRegistration->IsIpsecSupported());
}

TEST_F(AosRegistrationTest, UpdatePreloadedRoute)
{
    // m_piRegParameter is null
    EXPECT_FALSE(m_pTestAosRegistration->UpdatePreloadedRoute(AString::ConstNull(), 0));

    // m_piRegParameter is not null
    m_pTestAosRegistration->m_piRegParameter = &m_objMockIRegParameter;
    EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(1);
    EXPECT_TRUE(m_pTestAosRegistration->UpdatePreloadedRoute(AString("192.168.0.1"), 80));
}

TEST_F(AosRegistrationTest, RetryCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // TEST_F : IncreaseConsecutiveFailCount
    m_pTestAosRegistration->IncreaseConsecutiveFailCount();
    EXPECT_FALSE(m_pTestAosRegistration->IsRetryCountClear());

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));

    // TEST_F : ClearRetryCount
    m_pTestAosRegistration->ClearRetryCount();
    EXPECT_FALSE(m_pTestAosRegistration->IsRetryCountClear());

    m_pTestAosRegistration->ClearRetryCount(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsRetryCountClear());
}

TEST_F(AosRegistrationTest, SpecificOperation)
{
    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
    m_pTestAosRegistration->SetFakeReg(IMS_FALSE);

    IMS_BOOL bIsIpsecSupported = m_pTestAosRegistration->IsIpsecSupported();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSipOverIpsecInRoamingEnabled())
            .Times(5)
            .WillRepeatedly(Return(IMS_FALSE));

    // IMS_SINT32 nAccessType = CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .Times(5)
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED))
            .WillRepeatedly(Return(CarrierConfig::Ims::
                            PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE));

    m_pTestAosRegistration->AddSpecificOperation();

    EXPECT_EQ(bIsIpsecSupported, m_pTestAosRegistration->IsIpsecSupported());

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));

    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _))
            .Times(4)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pTestAosRegistration->AddSpecificOperation();
    m_pTestAosRegistration->AddSpecificOperation();
    m_pTestAosRegistration->AddSpecificOperation();
    m_pTestAosRegistration->AddSpecificOperation();
}

TEST_F(AosRegistrationTest, GetNetworkTypeForImsRegState)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillOnce(Return(NW_REPORT_RADIO_WLAN))
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_NR))
            .WillOnce(Return(NW_REPORT_RADIO_WCDMA))
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    EXPECT_EQ(m_pTestAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::IWLAN);
    EXPECT_EQ(m_pTestAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::LTE);
    EXPECT_EQ(m_pTestAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::NR);
    EXPECT_EQ(m_pTestAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::UTRAN);
    EXPECT_EQ(m_pTestAosRegistration->GetNetworkTypeForImsRegState(), AosNetworkType::NONE);
}

TEST_F(AosRegistrationTest, IsGeolocationInfoRequired)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // IpcanCategory is CATEGORY_MOBILE - AosRegistrationType::NORMAL
    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_FALSE(m_pTestAosRegistration->IsGeolocationInfoRequired());
    EXPECT_TRUE(m_pTestAosRegistration->IsGeolocationInfoRequired());
    EXPECT_FALSE(m_pTestAosRegistration->IsGeolocationInfoRequired());

    // IpcanCategory is CATEGORY_MOBILE - AosRegistrationType::EMERGENCY
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pTestAosRegistration->IsGeolocationInfoRequired());
    EXPECT_FALSE(m_pTestAosRegistration->IsGeolocationInfoRequired());

    // IpcanCategory is CATEGORY_WLAN - AosRegistrationType::NORMAL
    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIpcan::CATEGORY_WLAN));
    m_pTestAosRegistration->UpdateRegIpcanCategory();

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pTestAosRegistration->IsGeolocationInfoRequired());
    EXPECT_FALSE(m_pTestAosRegistration->IsGeolocationInfoRequired());

    // IpcanCategory is CATEGORY_WLAN - AosRegistrationType::EMERGENCY
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pTestAosRegistration->IsGeolocationInfoRequired());
    EXPECT_FALSE(m_pTestAosRegistration->IsGeolocationInfoRequired());

    // AosRegistrationType::FAKE
    m_pTestAosRegistration->SetRegType(AosRegistrationType::FAKE);
    EXPECT_FALSE(m_pTestAosRegistration->IsGeolocationInfoRequired());
}

TEST_F(AosRegistrationTest, StartFailed_TxnTimeout)
{
    // Covers GetRegErrCodeForPcscfDiscovery() == CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC))
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(1)
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
}

TEST_F(AosRegistrationTest, UpdateFailed_TxnTimeout)
{
    // Covers GetReregErrCodeForInitRegWithAvailablePcscf() ==
    // CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    ImsVector<IMS_SINT32> objReregErrCodeForInitRegWithAvailablePcscf;
    objReregErrCodeForInitRegWithAvailablePcscf.Clear();
    objReregErrCodeForInitRegWithAvailablePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForInitRegWithAvailablePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForInitRegWithAvailablePcscf));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC))
            .WillOnce(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    m_pTestAosRegistration->ProcessUpdateFailed_TxnTimeout();

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(1)
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    m_pTestAosRegistration->ProcessUpdateFailed_TxnTimeout();
}

TEST_F(AosRegistrationTest, UpdateTransactionStarted)
{
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->UpdateTransactionStarted();
    EXPECT_TRUE(m_pTestAosRegistration->IsTransactionStarted());

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    m_pTestAosRegistration->SetHeldByCall(IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);
    m_pTestAosRegistration->SetBlocked(IMS_FALSE);
    m_pTestAosRegistration->SetRadioWaiting(IMS_FALSE);
    m_pTestAosRegistration->UpdateTransactionStarted();
    EXPECT_TRUE(m_pTestAosRegistration->IsTransactionStarted());
}

TEST_F(AosRegistrationTest, StandardPcscfSelection)
{
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT))
            .Times(AnyNumber());

    m_pTestAosRegistration->ProcessStandardPcscfSelection();

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT))
            .Times(AnyNumber());

    m_pTestAosRegistration->ProcessStandardPcscfSelection(30);
}

TEST_F(AosRegistrationTest, RefreshTimerExpired)
{
    // m_piRegistration == IMS_NULL
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    m_pTestAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_FALSE(bDoImplicitRefresh);

    // GetState() == STATE_OFFLINE return;
    EXPECT_CALL(m_objMockIRegistration, GetState()).Times(0);

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    bDoImplicitRefresh = IMS_TRUE;
    m_pTestAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_FALSE(bDoImplicitRefresh);

    // GetRegOutOfServicePolicy
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegOutOfServicePolicy())
            .Times(AnyNumber())
            .WillOnce(
                    Return(static_cast<IMS_SINT32>(CarrierConfig::Assets::REG_OOS_POLICY_DESTROY)))
            .WillRepeatedly(
                    Return(static_cast<IMS_SINT32>(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT)));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_TRUE));

    // Destroy() - SetContactAddressConfiguration()
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(1);
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(1);

    bDoImplicitRefresh = IMS_TRUE;
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pTestAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // !IsTransactionStarted , AddFeature return;
    bDoImplicitRefresh = IMS_TRUE;
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pTestAosRegistration->SetTransactionStarted(IMS_FALSE);

    m_pTestAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_FALSE(bDoImplicitRefresh);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // IsTransactionStarted , SetState  STATE_REFRESHING;
    m_pTestAosRegistration->SetTransactionStarted(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    bDoImplicitRefresh = IMS_TRUE;
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    m_pTestAosRegistration->SetFakeReg(IMS_TRUE);

    m_pTestAosRegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_TRUE(bDoImplicitRefresh);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, Terminated)
{
    // IsHandlingServerSocketErrorRequired - IMS_TRUE
    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pTestAosRegistration->IsReconnectingServerSocketErrorAllowed());

    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);
    EXPECT_EQ(m_pTestAosRegistration->m_nErrorCountForServerSocket, 1);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_INTERNAL_ERROR));

    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_SERVER_SOCKET_ERROR);
    EXPECT_EQ(m_pTestAosRegistration->m_nErrorCountForServerSocket, 1);
    m_pTestAosRegistration->StopTimer(TIMER_INTERNAL_ERROR);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    // IsHandlingServerSocketErrorRequired - IMS_FALSE
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    // IsCallEndAndPdnReactivationByRegTerminated - IMS_TRUE
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT))
            .Times(1);
    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED))
            .Times(1);
    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    // IsCallEndAndPdnReactivationByRegTerminated - IMS_FALSE
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED))
            .Times(1);
    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);
}

TEST_F(AosRegistrationTest, SendRegisterEx)
{
    // m_piRegistration is null
    m_pTestAosRegistration->SetIRegistration(nullptr);
    EXPECT_FALSE(m_pTestAosRegistration->SendRegisterEx(1800, IMS_TRUE));

    // m_piRegistration is not null
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, Register(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FAILURE))
            .WillOnce(Return(IMS_SUCCESS));

    // fail to Register
    EXPECT_FALSE(m_pTestAosRegistration->SendRegisterEx(1800, IMS_FALSE));

    // succeed to Register
    EXPECT_TRUE(m_pTestAosRegistration->SendRegisterEx(0, IMS_FALSE));
}

TEST_F(AosRegistrationTest, UpdateFeatureTag)
{
    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objFeatureTagList));
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objBindedList));

    // same with binded feature tag
    EXPECT_FALSE(m_pTestAosRegistration->UpdateFeatureTag(&m_objMockIAosHandle));

    // Not same with binded feature tag
    m_objFeatureTagList.AddFeatureTag(FeatureTags::CDMALESS);
    m_objBindedList.AddFeatureTag(FeatureTags::VIDEO);
    EXPECT_CALL(m_objMockIRegContact, RemoveHeaderParameter(_, _)).Times(1);
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _)).Times(1);
    EXPECT_TRUE(m_pTestAosRegistration->UpdateFeatureTag(&m_objMockIAosHandle));
}

TEST_F(AosRegistrationTest, SetStaticIpQos)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(46));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but GetImsSignallingDscp is 0
    m_pTestAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but ePDG is enabled
    m_pTestAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_WIFI but ePDG is not enabled
    m_pTestAosRegistration->SetStaticIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_NONE
    m_pTestAosRegistration->SetStaticIpQos();
}

TEST_F(AosRegistrationTest, SetDynamicIpQos)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(46));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI))
            .WillOnce(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but GetImsSignallingDscp is 0
    m_pTestAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_CELLULAR but ePDG is enabled
    m_pTestAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_WIFI but ePDG is not enabled
    m_pTestAosRegistration->SetDynamicIpQos();

    // nPreferredImsDscp is PREFERRED_DSCP_NONE
    m_pTestAosRegistration->SetDynamicIpQos();
}

TEST_F(AosRegistrationTest, GetActualWaitTime)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    m_pTestAosRegistration->m_nConsecutiveFailure = 2;

    // size of RegRetryIntervals and RegRandomRetryIntervals is different
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Add(1000);
    objInterval.Add(2000);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));
    ImsVector<IMS_SINT32> objRandomInterval;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRandomRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRandomInterval));
    m_pTestAosRegistration->GetActualWaitTime();

    // size of RegRetryIntervals and RegRandomRetryIntervals is same
    objRandomInterval.Add(1000);
    objRandomInterval.Add(2000);
    m_pTestAosRegistration->GetActualWaitTime();
}

TEST_F(AosRegistrationTest, TryNextPcscf)
{
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    // succeed to SetNextPcscf - bHonorRetryAfter is true
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pTestAosRegistration->TryNextPcscf(IMS_TRUE, IMS_TRUE));
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);

    // succeed to SetNextPcscf - bHonorRetryAfter is false - succeed to SendRegister
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(1);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(m_pTestAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // succeed to SetNextPcscf - bHonorRetryAfter is false - fail to SendRegister
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, Register(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FAILURE));
    EXPECT_FALSE(m_pTestAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // fail to SetNextPcscf - bFlowRecoveryOnAllFail is true
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));

    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _)).Times(1).WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pTestAosRegistration->TryNextPcscf(IMS_TRUE, IMS_FALSE));

    // fail to SetNextPcscf - bFlowRecoveryOnAllFail is false
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_FALSE(m_pTestAosRegistration->TryNextPcscf(IMS_FALSE, IMS_FALSE));
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, SetNextPcscf_SamePcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

    ImsList<IMS_SINT32> objPcscfPorts;
    objPcscfPorts.Append(5060);
    objPcscfPorts.Append(5060);
    objPcscfPorts.Append(5060);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfsPorts())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfPorts));

    m_pTestAosRegistration->SetPcscf();

    ASSERT_TRUE(m_pTestAosRegistration->m_strPcscf.Equals(m_objPcscfs.GetElementAt(0)));
    ASSERT_EQ(m_pTestAosRegistration->m_nPcscfPort, objPcscfPorts.GetAt(0));

    AString strCurrentPcscf = m_pTestAosRegistration->m_strPcscf;
    IMS_UINT32 nCurrentPort = m_pTestAosRegistration->m_nPcscfPort;

    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    m_pTestAosRegistration->SetNextPcscf();

    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPort, m_pTestAosRegistration->m_nPcscfPort);
}

TEST_F(AosRegistrationTest, IsRetryOnSamePcscfRequired)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(3)
            .WillOnce(Return(0))
            .WillOnce(Return(2))
            .WillOnce(Return(2));

    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(2)
            .WillOnce(Return(3))
            .WillOnce(Return(0));

    EXPECT_FALSE(m_pTestAosRegistration->IsRetryOnSamePcscfRequired());
    EXPECT_FALSE(m_pTestAosRegistration->IsRetryOnSamePcscfRequired());
    EXPECT_TRUE(m_pTestAosRegistration->IsRetryOnSamePcscfRequired());
}

TEST_F(AosRegistrationTest, OnMessage)
{
    // MSG_REG_REINITIATE - ProcessReinitiate - CreateRegistration (fail)
    ImsMessage objMsg(MSG_REG_REINITIATE, 0, 0);
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objEmptyImpus));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // MSG_REG_UPDATE - ProcessUpdatePending
    objMsg.nMSG = MSG_REG_UPDATE;
    m_pTestAosRegistration->UpdateTxnPending(PENDING_UPDATE, IMS_TRUE);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_UPDATE));

    // MSG_REG_RECONFIG - ProcessReconfigPending
    objMsg.nMSG = MSG_REG_RECONFIG;
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pTestAosRegistration->UpdateTxnPending(PENDING_RECONFIG, IMS_TRUE);
    m_pTestAosRegistration->UpdateTxnPending(PENDING_UPDATE, IMS_TRUE);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_UPDATE));
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_RECONFIG));
    m_pTestAosRegistration->UpdateTxnPending(PENDING_RECONFIG, IMS_FALSE);

    // MSG_REG_REQUIRED_WITH_WAIT_TIME - ProcessRegRequiredWithWaitTime
    objMsg.nMSG = MSG_REG_REQUIRED_WITH_WAIT_TIME;
    // nWaitTime is less than 0 - ProcessReinitiate
    objMsg.nWparam = 0;
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    // App is not ready
    objMsg.nWparam = 30;
    m_pTestAosRegistration->SetAppReady(IMS_FALSE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    // there is no Ims call
    m_pTestAosRegistration->SetAppReady(IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    // there is Ims call
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    m_pTestAosRegistration->StopTimer(TIMER_OFFLINE_RECOVER);

    // MSG_REG_REQUIRED_WITH_NEXT_PCSCF - ProcessRegRequiredWithNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillOnce(Return(2))
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    objMsg.nMSG = MSG_REG_REQUIRED_WITH_NEXT_PCSCF;
    // fail to set next PCSCF
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_TERMINATED));
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    // succeed to set next PCSCF - fail to create registration (ProcessUnpredictableFailure)
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    // succeed to set next PCSCF and to create registration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // MSG_REG_TERMINATED_BY_NOTIFY - ProcessRegTerminatedByNotify
    objMsg.nMSG = MSG_REG_TERMINATED_BY_NOTIFY;
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN));
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // MSG_SUB_REINITIATE - ProcessSubReinitiate
    objMsg.nMSG = MSG_SUB_REINITIATE;
    m_pTestAosRegistration->OnMessage(objMsg);
    // Not Registered
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->OnMessage(objMsg);
    // Registered
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->OnMessage(objMsg);

    // MSG_SUB_TERMINATED - ProcessSubscription_Terminated
    objMsg.nMSG = MSG_SUB_TERMINATED;
    m_pTestAosRegistration->OnMessage(objMsg);

    // MSG_REG_EVENT_REGISTERED - ProcessRegEventRegistered
    objMsg.nMSG = MSG_REG_EVENT_REGISTERED;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_NOTIFY));
    m_pTestAosRegistration->m_nConsecutiveFailure = 2;
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailure, 0);

    // MSG_REG_START - It doesn't do anything
    objMsg.nMSG = MSG_REG_START;
    m_pTestAosRegistration->OnMessage(objMsg);
}

TEST_F(AosRegistrationTest, CheckPending)
{
    m_pTestAosRegistration->UpdateTxnPending(PENDING_RECONFIG, IMS_TRUE);
    m_pTestAosRegistration->UpdateTxnPending(PENDING_UPDATE, IMS_TRUE);
    m_pTestAosRegistration->CheckPending();
}

TEST_F(AosRegistrationTest, ProcessPendingTransaction)
{
    // PENDING_START
    m_pTestAosRegistration->UpdateTxnPending(PENDING_START, IMS_TRUE);
    m_pTestAosRegistration->SetTransactionStarted(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));
    m_pTestAosRegistration->ProcessPendingTransaction();
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_START));

    // PENDING_TRANSACTION - RetryHeld - failt to SendRegister
    m_pTestAosRegistration->UpdateTxnPending(PENDING_TRANSACTION, IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessPendingTransaction();
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));

    // PENDING_TRANSACTION - RetryHeld - succeed to SendRegister
    m_pTestAosRegistration->UpdateTxnPending(PENDING_TRANSACTION, IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessPendingTransaction();
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // PENDING_TRANSACTION - Not RetryHeld - fail to CreateRegistration
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    m_pTestAosRegistration->UpdateTxnPending(PENDING_TRANSACTION, IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessPendingTransaction();
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));

    // PENDING_TRANSACTION - Not RetryHeld - succeed to CreateRegistration
    m_pTestAosRegistration->UpdateTxnPending(PENDING_TRANSACTION, IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessPendingTransaction();
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));

    // PENDING_SUBSCRIPTION
    m_pTestAosRegistration->UpdateTxnPending(PENDING_SUBSCRIPTION, IMS_TRUE);
    m_pTestAosRegistration->ProcessPendingTransaction();
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_SUBSCRIPTION));
}

TEST_F(AosRegistrationTest, ProcessRetryInRegStopped)
{
    // bIgnoreTimer is true but IsRetryHeld is false
    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 10000);
    m_pTestAosRegistration->ProcessRetryInRegStopped(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // bIgnoreTimer is false and IsRetryStopped is false
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // CheckRadioReadyAndSetTxnPending() is false
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // fail to SendRegister
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // succeed to SendRegister
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessRetryInRegStopped(IMS_FALSE);
}

TEST_F(AosRegistrationTest, ProcessReregister)
{
    // not STATE_REGISTERED
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->ProcessReregister();
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG)).Times(0);

    // CheckRadioReadyAndSetTxnPending() is false
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosTransaction, IsTransactionAllowed(IAosTransaction::TYPE_REG))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->ProcessReregister();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // fail to SendRegister - ProcessUnpredictableFailureHeldByCall return true
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->ProcessReregister();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // fail to SendRegister - ProcessUnpredictableFailureHeldByCall return false
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    m_pTestAosRegistration->ProcessReregister();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // succeed to SendRegister
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_UPDATE));
    m_pTestAosRegistration->ProcessReregister();
}

TEST_F(AosRegistrationTest, ProcessRegTerminated)
{
    // there is Ims call
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->ProcessRegTerminated();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));
    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    m_pTestAosRegistration->UpdateTxnPending(PENDING_TRANSACTION, IMS_FALSE);

    // Retry timer in running
    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 10000);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->ProcessRegTerminated();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, ProcessAuthenticationFailed)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED))
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED))
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);

    // ERROR_POLICY_PDN_REACTIVATED - STATE_REGISTERING
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->ProcessAuthenticationFailed();

    // ERROR_POLICY_PDN_REACTIVATED - STATE_REFRESHING
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pTestAosRegistration->ProcessAuthenticationFailed();

    // Not ERROR_POLICY_PDN_REACTIVATED
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_AUTHENTICATION));
    m_pTestAosRegistration->ProcessAuthenticationFailed();
}

TEST_F(AosRegistrationTest, ProcessForbiddenFailed)
{
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objCount));

    // Error code is exist in RegPermanentErrCode
    EXPECT_FALSE(m_pTestAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));

    // m_nForbiddenCount is greater than or equal to nMaxCount
    objErrCode.Add(403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_TYPE_CRITICAL));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_FORBIDDEN))
            .Times(AnyNumber());
    EXPECT_TRUE(m_pTestAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));

    // m_nForbiddenCount is less than nMaxCount - Not Registered
    objCount.Add(5);
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    EXPECT_TRUE(m_pTestAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));

    // m_nForbiddenCount is less than nMaxCount - Registered
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_TRUE(m_pTestAosRegistration->ProcessForbiddenFailed(SipStatusCode::SC_403));
}

TEST_F(AosRegistrationTest, ProcessRegRequiredWithAvailableNextPcscf)
{
    // fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pTestAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    // succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    // fail to CreateRegistration
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    // succeed to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);

    // nRetryAfter > 0
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessRegRequiredWithAvailableNextPcscf(IMS_FALSE, 0);
}

TEST_F(AosRegistrationTest, ProcessSubscriberFailed)
{
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_SUBSCRIBER_FAILED));
    ImsVector<IMS_SINT32> objReregErrCode;
    objReregErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCode));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));

    // STATE_REGISTERED - GetExtraReregErrCode does not have ErrCode
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(0);
    EXPECT_FALSE(m_pTestAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));

    // STATE_REFRESHING - m_nConsecutiveFailure is 1
    objReregErrCode.Add(403);
    m_pTestAosRegistration->m_nConsecutiveFailure = 1;
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(0);
    EXPECT_TRUE(m_pTestAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailure, 0);

    // STATE_OFFLINE - GetExtraRegErrCode does not have ErrCode
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(0);
    EXPECT_FALSE(m_pTestAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));

    // there are PUIDs more than 1 - succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->m_strPuid = m_objImpus.GetElementAt(0);
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(m_objImpus));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    ImsVector<IMS_SINT32> objRegErrWaitTime;
    objRegErrWaitTime.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrWaitTime())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrWaitTime));
    EXPECT_TRUE(m_pTestAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));

    // there is not PUID more than 1
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillOnce(ReturnRef(objEmptyImpus));
    EXPECT_TRUE(m_pTestAosRegistration->ProcessSubscriberFailed(SipStatusCode::SC_403));
}

TEST_F(AosRegistrationTest, ProcessSetIpsec)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();

    // REASON_SET_IPSEC_ENABLE
    m_pTestAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_ENABLE);
    EXPECT_TRUE(m_pTestAosRegistration->IsIpsecSupported());

    // REASON_SET_IPSEC_DISABLE
    m_pTestAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_DISABLE);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());

    // invalid command
    m_pTestAosRegistration->RequestCmd(
            IAosRegistration::CMD_SET_IPSEC, IAosRegistration::REASON_SET_IPSEC_INIT);
    EXPECT_FALSE(m_pTestAosRegistration->IsIpsecSupported());
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_Start)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Clear();
    objErrorCode.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithRetryAfterTime())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrorCode));

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EVERY_PCSCF
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_FALSE, 0)).Times(1);
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_SPECIFIED_INTERVAL
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfTried()).Times(1);
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EACH_PCSCF - succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EACH_PCSCF));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EACH_PCSCF - fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_RFC_RULE
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _)).Times(1);

    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Start(SipStatusCode::SC_600);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy)
{
    // succeed to SetNextPcscf - nRetryAfter is greater than 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(10);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // succeed to SetNextPcscf - nRetryAfter is 0 - fail to SendRegister
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);

    // succeed to SetNextPcscf - nRetryAfter is 0 - succeed to SendRegister
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    EXPECT_CALL(m_objMockIRegistration, Register(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);

    // fail to SetNextPcscf - succeed to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pTestAosRegistration->m_piRegParameter = &m_objMockIRegParameter;
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // fail to SetNextPcscf - fail to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(0);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy)
{
    // retry counter is shared between REGISTER and SUBSCRIBE - retry count can be increased
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(2);
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // retry counter is shared between REGISTER and SUBSCRIBE - retry count reaches max count
    // succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    // retry counter is shared between REGISTER and SUBSCRIBE - retry count reaches max count
    // fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(4));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);

    // retry counter is not shared between REGISTER and SUBSCRIBE - Retry count can be increased
    // succeed to SetNextPcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(0);

    // retry counter is not shared between REGISTER and SUBSCRIBE - Retry count can be increased
    // fail to SetNextPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount()).WillOnce(Return(4));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(10);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_Update)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Clear();
    objErrorCode.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeWithRetryAfterTime())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrorCode));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EVERY_PCSCF - nRetryAfter is less than 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_FAILURE_TO_EVERY_PCSCF));
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));

    // fail to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);

    // succeed to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);

    // ActualWaitTimePolicy is AWT_POLICY_FAILURE_TO_EVERY_PCSCF - nRetryAfter is greater than 0
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::RETRY_AFTER_SEC, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_600);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    m_pTestAosRegistration->StopTimer(TIMER_OFFLINE_RECOVER);

    // ActualWaitTimePolicy is AWT_POLICY_SPECIFIED_INTERVAL
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_300);

    // ActualWaitTimePolicy is AWT_POLICY_RFC_RULE
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Update(SipStatusCode::SC_300);
}

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy)
{
    ImsVector<IMS_SINT32> objErrorCode;
    objErrorCode.Clear();
    objErrorCode.Add(SipStatusCode::SC_600);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraReregErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrorCode));
    ImsVector<IMS_SINT32> objInterval;
    objInterval.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryIntervals())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInterval));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    // retry counter is shared between REGISTER and SUBSCRIBE
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // GetExtraReregErrCode have nStatusCode
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_600, 0);

    // GetExtraReregErrCode dose not have nStatusCode - retry count can be increased
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(_))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);

    // GetExtraReregErrCode dose not have nStatusCode - retry count reaches max count
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);

    // retry counter is not shared between REGISTER and SUBSCRIBE
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // nStatusCode is SC_481
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_481, 0);

    // m_nConsecutiveFailure is 1 and Reg is not Expired During Awt
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL));
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailure, 1);

    // m_nConsecutiveFailure is 1 and Reg is Expired During Awt
    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));
    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(AnyNumber()).WillRepeatedly(Return(100));
    m_pTestAosRegistration->m_nConsecutiveFailure = 0;
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 100);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailure, 1);

    // m_nConsecutiveFailure is 2
    m_pTestAosRegistration->ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            SipStatusCode::SC_300, 0);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailure, 2);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountPerPcscfConfigured)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    m_pTestAosRegistration->m_piRegParameter = &m_objMockIRegParameter;

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());

    m_pTestAosRegistration->SetPcscf();

    AString strCurrentPcscf = m_pTestAosRegistration->m_strPcscf;
    IMS_UINT32 nCurrentPcscfPort = m_pTestAosRegistration->m_nPcscfPort;

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _)).Times(2);

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgReferee<0>(m_objPcscfs.GetElementAt(1)),
                    SetArgReferee<1>(m_objPcscfPorts.GetAt(1)), Return(IMS_TRUE)))
            .WillOnce(Return(IMS_FALSE));

    // 1st Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 2nd Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 3rd Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 4th Try failed (on next pcscf)
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_FALSE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_NE(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);
    strCurrentPcscf = m_pTestAosRegistration->m_strPcscf;
    nCurrentPcscfPort = m_pTestAosRegistration->m_nPcscfPort;

    // 5th Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 6th Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 7th Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 8th Try failed (failed next pcscf)
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountOnSinglePcscfConfigured)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    ImsVector<IMS_SINT32> objExtraRegErrCodeList;
    objExtraRegErrCodeList.Add(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCodeList));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::TIMER_F_POLICY_SPEC_WITH_AWT));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());
    // END uninteresting preparation

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillRepeatedly(Return(1));

    m_pTestAosRegistration->SetPcscf();

    AString strCurrentPcscf = m_pTestAosRegistration->m_strPcscf;
    IMS_UINT32 nCurrentPcscfPort = m_pTestAosRegistration->m_nPcscfPort;

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(1).WillOnce(Return(IMS_FALSE));

    // 1st Try failed
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_TRUE(strCurrentPcscf.Equals(m_pTestAosRegistration->m_strPcscf));
    EXPECT_EQ(nCurrentPcscfPort, m_pTestAosRegistration->m_nPcscfPort);

    // 2nd Try failed (failed next pcscf)
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pTestAosRegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ProcessIpVersionChange)
{
    AString strIpv4Addr = "192.168.0.1";
    AString strIpv6Addr = "fc01:cafe::1";
    m_pTestAosRegistration->m_strPcscf = strIpv4Addr;

    IpAddress objLocalIpv4Addr(AString("192.186.0.100"));
    IpAddress objLocalIpv6Addr(AString("fc01:abab:cdcd:efe0::1"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv4Addr));
    EXPECT_FALSE(m_pTestAosRegistration->ProcessIpVersionChange());

    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv6Addr));

    AStringArray objPcscfs;
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfs));
    EXPECT_FALSE(m_pTestAosRegistration->ProcessIpVersionChange());

    objPcscfs.AddElement(strIpv6Addr);
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfs));

    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(2);
    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(2);
    EXPECT_TRUE(m_pTestAosRegistration->ProcessIpVersionChange());

    m_pTestAosRegistration->m_strPcscf = strIpv6Addr;
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv4Addr));
    objPcscfs.RemoveAllElements();
    objPcscfs.AddElement(strIpv4Addr);
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfs));
    EXPECT_TRUE(m_pTestAosRegistration->ProcessIpVersionChange());
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode)
{
    // ProcessStandardPcscfSelection
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_300);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);

    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_300);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessForbiddenFailed return true and ProcessSubscriberFailed return false
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Clear();
    objErrCode.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objCount));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_403);

    // ProcessIpsecFallback
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    objErrWithoutIpsec.Add(SipStatusCode::SC_406);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_406);

    // PDN reactivation is required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED))
            .WillOnce(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED))
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Clear();
    objExtraRegErrCode.Add(SipStatusCode::SC_408);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_TRUE;
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_408);

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    // ProcessStartFailed_503 - nRetryAfter is 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip503CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_503_CODE_POLICY_3GPP));
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_503);

    // ProcessStartFailed_420 - bIsExtensionUnsupported is false
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_420);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessStartFailed_421 - bIsExtensionRequired  is false
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_421);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessStartFailed_423 - nMinTime is less than 0
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // Other 4xx, 5xx, 6xx response
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_580);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_ProcessStartFailed_305)
{
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));

    // SIP_305_CODE_POLICY_3GPP
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - first PCSCF
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    EXPECT_CALL(m_objMockIAosPcscf, IsFirstPcscf())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - fail to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT));
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - succeed to SetFirstPcscf
    // fail to SendRegister
    m_pTestAosRegistration->m_piRegParameter = &m_objMockIRegParameter;
    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    // succeed to SendRegister
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_DEFAULT
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    objErrWithoutIpsec.Add(SipStatusCode::SC_305);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Success)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());

    AStringArray objPcscfsIpv6;
    objPcscfsIpv6.AddElement(AString("fc01:cafe::1"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::2"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::3"));

    AStringArray objPcscfsIpv4;
    objPcscfsIpv4.AddElement(AString("192.168.0.1"));
    objPcscfsIpv4.AddElement(AString("192.168.0.2"));
    objPcscfsIpv4.AddElement(AString("192.168.0.3"));

    IpAddress objLocalIpa = IpAddress(AString("192.168.0.100"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpa));

    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfsIpv4));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(100);

    for (IMS_SINT32 i = SipStatusCode::SC_500; i < SipStatusCode::SC_600; i++)
    {
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objPcscfsIpv6));

        m_pTestAosRegistration->SetPcscf();

        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objPcscfsIpv4));

        m_pTestAosRegistration->ProcessStartFailed_StatusCode(i);
        EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    }
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Failure)
{
    // BEGIN uninteresting preparation
    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetrySip503CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    AString strIpv4Addr = "192.168.0.1";
    m_pTestAosRegistration->m_strPcscf = strIpv4Addr;
    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REGISTERING;

    IpAddress objLocalIpv4Addr(AString("192.186.0.100"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpv4Addr));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT))
            .Times(1);
    m_pTestAosRegistration->ProcessStartFailed_StatusCode(500);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_HasNextPcscf)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    EXPECT_CALL(m_objMockIAosService, NotifyRegistering(_, _, _)).Times(AnyNumber());

    m_pTestAosRegistration->m_piRegParameter = &m_objMockIRegParameter;

    ImsVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));
    // END uninteresting preparation

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());

    AStringArray objPcscfsIpv6;
    objPcscfsIpv6.AddElement(AString("fc01:cafe::1"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::2"));
    objPcscfsIpv6.AddElement(AString("fc01:cafe::3"));

    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscf(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgReferee<0>(objPcscfsIpv6.GetElementAt(1)),
                    SetArgReferee<1>(m_objPcscfPorts.GetAt(1)), Return(IMS_TRUE)));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegRetryWithIpVerFallback())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegistration, Register(_)).Times(1).WillRepeatedly(Return(IMS_SUCCESS));

    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REGISTERING;

    m_pTestAosRegistration->ProcessStartFailed_StatusCode(500);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, ProcessStartFailed_Others)
{
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_FALSE;

    // ProcessAwtRecovery
    m_pTestAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_NONE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // REASON_CLIENT_SOCKET_ERROR - PDN reactivation is not required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    m_pTestAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    // REASON_CLIENT_SOCKET_ERROR - PDN reactivation is required
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));
    m_pTestAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    // REG_ERROR_CODE_OTHER is exist in ExtraRegErrCode
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    m_pTestAosRegistration->ProcessStartFailed_Others(IRegistration::REASON_NONE);
}

TEST_F(AosRegistrationTest, ProcessUpdateFailed_StatusCode)
{
    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // SipStatusCode is in GetRegErrCodeWithoutIpsec
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Add(SipStatusCode::SC_406);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_406);
    objErrWithoutIpsec.Clear();

    // SipStatusCode is in GetReregErrCodeForImsPdnReactivation
    ImsVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    objReregErrCodeForImsPdnReactivation.Add(SipStatusCode::SC_407);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForImsPdnReactivation())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForImsPdnReactivation));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_407);
    objReregErrCodeForImsPdnReactivation.Clear();

    // ProcessForbiddenFailed return true and ProcessSubscriberFailed return false
    ImsVector<IMS_SINT32> objErrCode;
    objErrCode.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrCode));
    ImsVector<IMS_SINT32> objCount;
    objCount.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegPermanentErrMaxCount())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objCount));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_403);
    objErrCode.Clear();

    // REG_ERROR_CODE_ALL_RESP is in GetReregRetryErrCodeForInitRegWithSamePcscf
    ImsVector<IMS_SINT32> ReregRetryErrCodeForInitRegWithSamePcscf;
    ReregRetryErrCodeForInitRegWithSamePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_ALL_RESP);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(ReregRetryErrCodeForInitRegWithSamePcscf));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);
    ReregRetryErrCodeForInitRegWithSamePcscf.Clear();

    // ExtraRegErrPolicy is ERROR_POLICY_PDN_REACTIVATED - m_bEps5GsOnly is false
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_FALSE;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);

    // ExtraRegErrPolicy is ERROR_POLICY_PDN_REACTIVATED - m_bEps5GsOnly is true
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_TRUE;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_302);

    // ExtraRegErrPolicy is AWT_POLICY_RFC_RULE  - SipStatusCode::SC_403
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_403);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // ExtraRegErrPolicy is AWT_POLICY_RFC_RULE  - SipStatusCode::SC_600
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(4));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_600);

    // ProcessUpdateFailed_423
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosRegistrationTest, ProcessUpdateFailed_StatusCode_ProcessUpdateFailed_305)
{
    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // SIP_305_CODE_POLICY_3GPP
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - first PCSCF
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF));
    EXPECT_CALL(m_objMockIAosPcscf, IsFirstPcscf())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - fail to SetFirstPcscf
    EXPECT_CALL(m_objMockIAosPcscf, GetFirstPcscf(_, _))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - succeed to SetFirstPcscf
    // fail to CreateRegistration
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF - not first PCSCF - succeed to SetFirstPcscf
    // succeed to CreateRegistration
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);

    // SIP_305_CODE_POLICY_DEFAULT
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetrySip305CodePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT));
    ImsVector<IMS_SINT32> objErrWithoutIpsec;
    objErrWithoutIpsec.Clear();
    objErrWithoutIpsec.Add(SipStatusCode::SC_305);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeWithoutIpsec())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objErrWithoutIpsec));
    m_pTestAosRegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_305);
}

TEST_F(AosRegistrationTest, Registration_AuthenticationChallenged)
{
    IMS_BOOL bResponseToChallenge = IMS_FALSE;

    // m_piRegistration is null
    m_pTestAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);
    ASSERT_EQ(bResponseToChallenge, IMS_FALSE);

    // IsAuthChallengeMoreAllowed return false
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->m_nAuthChallengeCount = 100;
    m_pTestAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);
    ASSERT_EQ(bResponseToChallenge, IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountWithIpsecOnAuthFailure())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->m_nAuthChallengeCount = 0;

    m_pTestAosRegistration->Registration_AuthenticationChallenged(
            Credential::TYPE_MD5, bResponseToChallenge);
    ASSERT_EQ(bResponseToChallenge, IMS_FALSE);
}

TEST_F(AosRegistrationTest, Registration_NotifyAkaResponse)
{
    ImsSaKey objSaKey;
    IMS_BOOL bResultOfSA = IMS_FALSE;

    // IPSec is not supported
    m_pTestAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);
    ASSERT_FALSE(bResultOfSA);

    // IPSec is supported - nResult is not RESULT_OK
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();
    m_pTestAosRegistration->SetImsCall(IMS_FALSE);

    m_pTestAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_NOK_MAC_INVALID, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);
    ASSERT_TRUE(bResultOfSA);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // IPSec is supported - nResult is RESULT_OK
    m_pTestAosRegistration->m_nAuthChallengeCount = 3;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPrivateHeader())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED));
    m_pTestAosRegistration->Registration_NotifyAkaResponse(
            ImsAkaParam::RESULT_OK, objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSA);
    ASSERT_TRUE(bResultOfSA);
}

TEST_F(AosRegistrationTest, Registration_Started)
{
    // m_piRegistration is null
    m_pTestAosRegistration->Registration_Started();

    // m_piRegistration is not null
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    // IPSec is not supported
    AStringArray objEmptyUris;
    EXPECT_CALL(m_objMockIRegistration, GetAssociatedUris()).WillOnce(ReturnRef(objEmptyUris));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Registration_Started();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);

    // IPSec is supported
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Registration_Started();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, Registration_StartFailed)
{
    // m_piRegistration is null
    m_pTestAosRegistration->Registration_StartFailed(IRegistration::REASON_NONE);

    // ProcessAuthenticationFailed
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->m_nAuthChallengeCount = 100;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Registration_StartFailed(IRegistration::REASON_NONE);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // REASON_STATUS_CODE
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->m_nAuthChallengeCount = 0;
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(SipStatusCode::SC_403));
    ImsVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    objRegErrCodeForPcscfDiscovery.Add(SipStatusCode::SC_403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegErrCodeForPcscfDiscovery())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegErrCodeForPcscfDiscovery));
    m_pTestAosRegistration->Registration_StartFailed(IRegistration::REASON_STATUS_CODE);

    // REASON_TRANSACTION_TIMEOUT
    objRegErrCodeForPcscfDiscovery.Clear();
    objRegErrCodeForPcscfDiscovery.Add(CarrierConfig::Assets::REG_ERROR_CODE_TIMER_F);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryTimerFPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::TIMER_F_POLICY_NONE));
    m_pTestAosRegistration->Registration_StartFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);

    // REASON_NONE
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Clear();
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    m_pTestAosRegistration->Registration_StartFailed(IRegistration::REASON_NONE);
}

TEST_F(AosRegistrationTest, Registration_Updated)
{
    // m_piRegistration is null
    m_pTestAosRegistration->Registration_Updated();

    // m_piRegistration is not null
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    // IPSec is not supported
    AStringArray objEmptyUris;
    EXPECT_CALL(m_objMockIRegistration, GetAssociatedUris()).WillOnce(ReturnRef(objEmptyUris));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Registration_Updated();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);

    // IPSec is supported - m_pSubscription is not null
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();
    m_pTestAosRegistration->UpdateTxnPending(PENDING_SUBSCRIPTION, IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosRegistration->Registration_Updated();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // IPSec is supported - m_pSubscription is null
    m_pTestAosRegistration->SetAosSubscription(IMS_NULL);
    m_pTestAosRegistration->Registration_Updated();
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_CallEndByReregErr)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTried()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(403));

    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(AnyNumber());

    // SetState - for ignoring UpdateDetailState()
    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);
    // END uninteresting preparation

    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;

    // IsImsCall() GetReregErrCodeForCallEnd()
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    ImsVector<IMS_SINT32> objReregErrCodeForCallEnd;
    objReregErrCodeForCallEnd.Clear();
    objReregErrCodeForCallEnd.Add(403);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForCallEnd));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(IAosRegistration::RESULT_FAILURE,
                    IAosRegistration::REASON_FAILURE_REG_TERMINATING))
            .Times(1);
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);

    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(AnyNumber());

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(480));
    // END uninteresting preparation

    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;
    m_pTestAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_TRUE;

    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(2);

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, AosReasonCode::PLMN_BLOCK)).Times(2);

    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pTestAosRegistration->m_nImsRegState,
            TestAosRegistration::IMS_REG_STATE_DEREGISTERED);

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;
    m_pTestAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;

    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_TRANSACTION_TIMEOUT);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_EQ(m_pTestAosRegistration->m_nImsRegState,
            TestAosRegistration::IMS_REG_STATE_DEREGISTERED);
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming_HeldByCall)
{
    // BEGIN uninteresting preparation
    ImsVector<IMS_SINT32> objReregErrCodeForCallEnd;
    objReregErrCodeForCallEnd.Clear();
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregErrCodeForCallEnd())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objReregErrCodeForCallEnd));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION));

    EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTriedCount()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetListener(IMS_NULL)).Times(AnyNumber());

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetStatusCode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(480));
    // END uninteresting preparation

    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REFRESHING;
    m_pTestAosRegistration->m_nImsRegState = TestAosRegistration::IMS_REG_STATE_REGISTERING;
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_TRUE;

    EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraReregErrInRoamingAsFailureHandled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_STATUS_CODE);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
    EXPECT_TRUE(m_pTestAosRegistration->IsHeldByCall());
    EXPECT_TRUE((m_pTestAosRegistration->m_nTxnPending &
                        TestAosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL) > 0);

    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_GENERAL))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, AosReasonCode::PLMN_BLOCK)).Times(1);

    m_pTestAosRegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_FALSE(m_pTestAosRegistration->IsHeldByCall());
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    EXPECT_FALSE((m_pTestAosRegistration->m_nTxnPending &
                         TestAosRegistration::PENDING_PLMN_BLOCK_HELD_BY_CALL) > 0);
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_ProcessUpdateFailed_Others)
{
    // m_nAuthChallengeCount is greater than AUTHENTICATION_RETRY_MAX_COUNT
    // Can not invoke ProcessUpdateFailed_Others
    m_pTestAosRegistration->m_nAuthChallengeCount = 100;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED));
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->m_nAuthChallengeCount = 0;

    // AWT policy is AWT_POLICY_SPECIFIED_INTERVAL
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::AWT_POLICY_SPECIFIED_INTERVAL))
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);

    // REG_ERROR_CODE_OTHER is in GetReregRetryErrCodeForInitRegWithSamePcscf
    ImsVector<IMS_SINT32> ReregRetryErrCodeForInitRegWithSamePcscf;
    ReregRetryErrCodeForInitRegWithSamePcscf.Add(CarrierConfig::Assets::REG_ERROR_CODE_OTHER);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetReregRetryErrCodeForInitRegWithSamePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(ReregRetryErrCodeForInitRegWithSamePcscf));
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);
    ReregRetryErrCodeForInitRegWithSamePcscf.Clear();

    // ProcessAwtRecovery
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_INTERNAL_ERROR);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);

    // nReason is REASON_CLIENT_SOCKET_ERROR - PDN reactivation is not required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_POLICY_PDN_REACTIVATED));
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    objExtraRegErrCode.Add(CarrierConfig::Assets::REG_ERROR_CODE_TRANSPORT);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrCode())
            .WillRepeatedly(ReturnRef(objExtraRegErrCode));
    m_pTestAosRegistration->m_bEps5GsOnly = IMS_FALSE;
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);

    // nReason is REASON_CLIENT_SOCKET_ERROR - PDN reactivation is required
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached())
            .Times(AnyNumber())
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillOnce(Return(1));
    m_pTestAosRegistration->Registration_UpdateFailed(IRegistration::REASON_CLIENT_SOCKET_ERROR);
}

TEST_F(AosRegistrationTest, Registration_Removed)
{
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_SUCCESS, IAosRegistration::REASON_NONE));
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Registration_Removed();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, StopTimer)
{
    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_OFFLINE_RECOVER);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));

    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_STOP_RETRY);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    m_pTestAosRegistration->StartTimer(TIMER_REFRESH, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_REFRESH);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_REFRESH));

    m_pTestAosRegistration->StartTimer(TIMER_EXPIRED, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_EXPIRED);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_EXPIRED));

    m_pTestAosRegistration->StartTimer(TIMER_MODE, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_MODE);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_MODE));

    m_pTestAosRegistration->StartTimer(TIMER_TRANSACTION, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_TRANSACTION);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_TRANSACTION));

    m_pTestAosRegistration->StartTimer(TIMER_INTERNAL_ERROR, 5000);
    m_pTestAosRegistration->StopTimer(TIMER_INTERNAL_ERROR);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, ClearTimer)
{
    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->StartTimer(TIMER_REFRESH, 5000);
    m_pTestAosRegistration->StartTimer(TIMER_EXPIRED, 5000);
    m_pTestAosRegistration->StartTimer(TIMER_MODE, 5000);
    m_pTestAosRegistration->StartTimer(TIMER_TRANSACTION, 5000);
    m_pTestAosRegistration->StartTimer(TIMER_INTERNAL_ERROR, 5000);

    m_pTestAosRegistration->ClearTimers();

    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_REFRESH));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_EXPIRED));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_MODE));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_TRANSACTION));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_INTERNAL_ERROR));

    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->ClearRetryTimers();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
}

TEST_F(AosRegistrationTest, TimerExpired)
{
    m_pTestAosRegistration->Timer_TimerExpired(IMS_NULL);

    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piOfflineRecoverTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));

    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piStopRetryTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    m_pTestAosRegistration->StartTimer(TIMER_REFRESH, 5000);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piRefreshTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_REFRESH));

    m_pTestAosRegistration->StartTimer(TIMER_EXPIRED, 5000);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piExpiredTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_EXPIRED));

    m_pTestAosRegistration->StartTimer(TIMER_MODE, 5000);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piModeTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_MODE));

    m_pTestAosRegistration->StartTimer(TIMER_TRANSACTION, 5000);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piTransactionTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_TRANSACTION));

    m_pTestAosRegistration->StartTimer(TIMER_INTERNAL_ERROR, 5000);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, RestoreActiveBindings()).Times(1);
    m_pTestAosRegistration->Timer_TimerExpired(m_pTestAosRegistration->m_piInternalErrorTimer);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_INTERNAL_ERROR));
}

TEST_F(AosRegistrationTest, ProcessOfflineRecoverTimerExpired)
{
    // Not STATE_OFFLINE
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));

    // Not AppReady
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->SetAppReady(IMS_FALSE);
    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));

    // State is STATE_OFFLINE and AppReady
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->SetAppReady(IMS_TRUE);

    // Transaction is started - fail to CreateRegistration
    m_pTestAosRegistration->SetTransactionStarted(IMS_TRUE);
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));

    // Transaction is started - succeed to CreateRegistration
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));

    // Transaction is not started
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->SetTransactionStarted(IMS_FALSE);

    m_pTestAosRegistration->StartTimer(TIMER_OFFLINE_RECOVER, 5000);
    m_pTestAosRegistration->ProcessOfflineRecoverTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, ProcessStopRetryTimerExpired)
{
    // Retry is not held
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // Transaction is started - fail to SendRegister
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->SetTransactionStarted(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_FAILURE, IAosRegistration::REASON_FAILURE_INTERNAL));
    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // Transaction is started - succeed to SendRegister
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START));
    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // Transaction is not started
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->SetTransactionStarted(IMS_FALSE);

    m_pTestAosRegistration->StartTimer(TIMER_STOP_RETRY, 5000);
    m_pTestAosRegistration->ProcessStopRetryTimerExpired();
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_STOP_RETRY));
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_TRANSACTION));
}

TEST_F(AosRegistrationTest, CreateAndDestroySubscription)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->InitFeatures();

    // StartSubscription - m_pSubscription is not null
    EXPECT_TRUE(m_pTestAosRegistration->StartSubscription(IMS_TRUE));

    // StopSubscription - m_pSubscription is not null
    EXPECT_TRUE(m_pTestAosRegistration->StopSubscription());

    // DestroySubscription - m_pSubscription is not null
    EXPECT_TRUE(m_pTestAosRegistration->DestroySubscription());

    // StartSubscription - m_pSubscription is null
    EXPECT_FALSE(m_pTestAosRegistration->StartSubscription(IMS_TRUE));

    // StopSubscription - m_pSubscription is null
    EXPECT_FALSE(m_pTestAosRegistration->StopSubscription());

    // CreateSubscription - m_piRegistration is null
    EXPECT_FALSE(m_pTestAosRegistration->CreateSubscription());

    // CreateSubscription - piRegSubscription is null
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, CreateSubscription(_))
            .Times(AnyNumber())
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&m_objMockIRegSubscription));
    EXPECT_FALSE(m_pTestAosRegistration->CreateSubscription());

    // CreateSubscription - m_piRegistration and piRegSubscription is not null
    EXPECT_TRUE(m_pTestAosRegistration->CreateSubscription());
}

TEST_F(AosRegistrationTest, Subscription_StateChanged)
{
    // m_piRegistration is null
    m_pTestAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_NONE);

    // ProcessSubscription_Success
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(1)
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));
    m_pTestAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_ESTABLISHED);
    EXPECT_EQ(m_pTestAosRegistration->m_nConsecutiveFailure, 0);

    // ProcessSubscription_Failed
    m_pTestAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_FAILED);

    // MSG_SUB_TERMINATED
    m_pTestAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_SUB_TERMINATED);

    // reason is REASON_NONE - It doesn't do anything
    m_pTestAosRegistration->Subscription_StateChanged(
            AosSubscription::STATE_SUBSCRIBING, AosSubscription::REASON_NONE);
}

TEST_F(AosRegistrationTest, Subscription_CanBeTransmitted)
{
    // Transaction is not started
    m_pTestAosRegistration->SetTransactionStarted(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosRegistration->Subscription_CanBeTransmitted());
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnPendingOn(PENDING_SUBSCRIPTION));

    // Transaction is started
    m_pTestAosRegistration->SetTransactionStarted(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->Subscription_CanBeTransmitted());
}

TEST_F(AosRegistrationTest, Subscription_NotifyReceived)
{
    // m_piRegistration is null
    m_pTestAosRegistration->Subscription_NotifyReceived(AosSubscription::EVENT_UNKNOWN);

    // EVENT_REGISTERED
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosRegistration->Subscription_NotifyReceived(AosSubscription::EVENT_REGISTERED);
}

TEST_F(AosRegistrationTest, Subscription_Request)
{
    // COMMAND_REG_REQUIRED
    m_pTestAosRegistration->Subscription_Request(AosSubscription::COMMAND_REG_REQUIRED, 0);

    // COMMAND_REG_REQUIRED_WITH_NOTI_NO_911_ADDR
    m_pTestAosRegistration->Subscription_Request(
            AosSubscription::COMMAND_REG_REQUIRED_WITH_NOTI_NO_911_ADDR, 0);

    // COMMAND_REG_REQUIRED_WITH_NEXT_PCSCF
    m_pTestAosRegistration->Subscription_Request(
            AosSubscription::COMMAND_REG_REQUIRED_WITH_NEXT_PCSCF, 0);

    // COMMAND_REG_REQUIRED_WITH_REG_RETRY_TIME
    m_pTestAosRegistration->Subscription_Request(
            AosSubscription::COMMAND_REG_REQUIRED_WITH_REG_RETRY_TIME, 0);

    // COMMAND_REG_REQUIRED_WITH_NOTI_NO_911_ADDR_WITH_REG_RETRY_TIME
    m_pTestAosRegistration->Subscription_Request(
            AosSubscription::COMMAND_REG_REQUIRED_WITH_NOTI_NO_911_ADDR_WITH_REG_RETRY_TIME, 0);

    // COMMAND_REG_TERMINATED
    m_pTestAosRegistration->Subscription_Request(AosSubscription::COMMAND_REG_TERMINATED, 0);

    // COMMAND_SUB_REQUIRED
    m_pTestAosRegistration->Subscription_Request(AosSubscription::COMMAND_SUB_REQUIRED, 0);

    // COMMAND_SUB_TERMINATED
    m_pTestAosRegistration->Subscription_Request(AosSubscription::COMMAND_SUB_TERMINATED, 0);
}

TEST_F(AosRegistrationTest, NConfiguration_NotifyConfigChanged)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSubscription())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    // FEATURE_SUBSCRIPTION is enabled and FEATURE_IPSEC is disabled
    m_pTestAosRegistration->NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(m_pTestAosRegistration->IsFeatureOn(FEATURE_SUBSCRIPTION));
    EXPECT_FALSE(m_pTestAosRegistration->IsFeatureOn(FEATURE_IPSEC));

    // FEATURE_SUBSCRIPTION is disabled and FEATURE_IPSEC is enabled
    m_pTestAosRegistration->NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(m_pTestAosRegistration->IsFeatureOn(FEATURE_SUBSCRIPTION));
    EXPECT_TRUE(m_pTestAosRegistration->IsFeatureOn(FEATURE_IPSEC));
}

TEST_F(AosRegistrationTest, Transaction_OnConnectionFailed)
{
    // BEGIN uninteresting preparation
    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));
    // END uninteresting preparation

    // IImsRadio::REASON_ACCESS_DENIED
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START))
            .Times(1);

    m_pTestAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    m_pTestAosRegistration->StopTimer(TIMER_OFFLINE_RECOVER);

    // IImsRadio::REASON_NAS_FAILURE
    m_pTestAosRegistration->SetRadioWaiting(IMS_TRUE);
    m_pTestAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_NAS_FAILURE, 0, 15000);
    EXPECT_FALSE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    EXPECT_FALSE(m_pTestAosRegistration->IsRadioWaiting());

    // IImsRadio::REASON_RRC_REJECT
    EXPECT_CALL(m_objMockIAosRegistrationListener,
            Registration_StateChanged(
                    IAosRegistration::RESULT_TRYING, IAosRegistration::REASON_TRYING_START))
            .Times(1);
    m_pTestAosRegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_RRC_REJECT, 0, 20000);
    EXPECT_TRUE(m_pTestAosRegistration->IsTimerRunning(TIMER_OFFLINE_RECOVER));
    m_pTestAosRegistration->StopTimer(TIMER_OFFLINE_RECOVER);
}

TEST_F(AosRegistrationTest, MessageMediator_AdjustMessage)
{
    // piSipMsg is null
    EXPECT_EQ(m_pTestAosRegistration->MessageMediator_AdjustMessage(
                      IMS_NULL, IMessageMediator::MESSAGE_NORMAL),
            IMS_FAILURE);

    // fail to AddLocationHeaderBody
    EXPECT_EQ(m_pTestAosRegistration->MessageMediator_AdjustMessage(
                      static_cast<ISipMessage*>(&m_objMockISipMessage),
                      IMessageMediator::MESSAGE_NORMAL),
            IMS_FAILURE);
}

TEST_F(AosRegistrationTest, AddLocationHeaderBody)
{
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // IsGeolocationInfoRequired return false
    EXPECT_FALSE(m_pTestAosRegistration->AddLocationHeaderBody(
            static_cast<ISipMessage*>(&m_objMockISipMessage), IMessageMediator::MESSAGE_NORMAL));

    // IsGeolocationInfoRequired return true
    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGeolocationPidfSupported(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISipMessage, RemoveBodyParts()).Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetGeolocationPidfFormingPolicy()).Times(0);
    EXPECT_FALSE(m_pTestAosRegistration->AddLocationHeaderBody(
            static_cast<ISipMessage*>(&m_objMockISipMessage), IMessageMediator::MESSAGE_RESUBMIT));
}
