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
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosTransaction.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosRegistration.h"

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

class TestAosRegistration : public AosRegistration
{
    inline TestAosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosRegistration(piAppContext, strRegId),
            m_piMockRegistration(IMS_NULL)
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
    FRIEND_TEST(AosRegistrationTest, RetryCount);
    FRIEND_TEST(AosRegistrationTest, SpecificOperation);
    FRIEND_TEST(AosRegistrationTest, StartFailed_TxnTimeout);
    FRIEND_TEST(AosRegistrationTest, UpdateFailed_TxnTimeout);
    FRIEND_TEST(AosRegistrationTest, UpdateTransactionStarted);
    FRIEND_TEST(AosRegistrationTest, StandardPcscfSelection);
    FRIEND_TEST(AosRegistrationTest, RefreshTimerExpired);
    FRIEND_TEST(AosRegistrationTest, Terminated);
    FRIEND_TEST(AosRegistrationTest, SetNextPcscf_SamePcscf);
    FRIEND_TEST(AosRegistrationTest, IsRetryOnSamePcscfRequired);
    FRIEND_TEST(AosRegistrationTest, OnMessage);
    FRIEND_TEST(AosRegistrationTest, ProcessSetIpsec);
    FRIEND_TEST(AosRegistrationTest, ProcessDefaultFlowRecovery_Start_NotSamePcscf);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_TxnTimeout_RegRetryCountPerPcscfConfigured);
    FRIEND_TEST(AosRegistrationTest,
            ProcessStartFailed_TxnTimeout_RegRetryCountOnSinglePcscfConfigured);
    FRIEND_TEST(AosRegistrationTest, ProcessIpVersionChange);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Success);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_Failure);
    FRIEND_TEST(AosRegistrationTest, ProcessStartFailed_StatusCode_IpVersionChange_HasNextPcscf);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_CallEndByReregErr);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming);
    FRIEND_TEST(AosRegistrationTest, Registration_UpdateFailed_FailOnRoaming_HeldByCall);

protected:
    inline IRegistration* GetRegistration() override { return m_piMockRegistration; }
    //    inline IMS_BOOL CheckRadioReadyAndSetTxnPending() override { return IMS_TRUE; }

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

    inline IAosRegistrationListener* GetListener() { return m_piListener; }

    inline void UpdateTxnFeature(IN IMS_UINT32 nFeature, IN IMS_BOOL bEnable)
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

    inline IMS_BOOL IsTxnFeatureOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nTxnPending);
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
};

class AosRegistrationTest : public ::testing::Test
{
public:
    TestAosRegistration* m_pTestAosRegistration;
    AosStaticProfile* m_pAosStaticProfile;
    IAosNConfiguration* m_piAosNConfiguration;
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
    MockIAosTransaction m_objMockIAosTransaction;

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
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);

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
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnFeatureOn(PENDING_START));

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
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

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

    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

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

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
}

TEST_F(AosRegistrationTest, SetListener)
{
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

    EXPECT_EQ(m_pTestAosRegistration->GetListener(),
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

    m_pTestAosRegistration->SetListener(IMS_NULL);
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
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));
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

    // CMD_ECALL_INIT - won't handled
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

    // PROPERTY_PROTECTED
    m_pTestAosRegistration->GetProperty(IAosRegistration::PROPERTY_PROTECTED, nValue, strValue);
    EXPECT_EQ(nValue, AoSRegProtectedType::REG_UNPROTECTED);

    // PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, nValue, strValue);
    EXPECT_EQ(nValue, AoSSupportability::NOT_SUPPORTED);

    // PROPERTY_NETWORK_BINDING_FEATURES
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_NETWORK_BINDING_FEATURES, nValue, strValue);
    EXPECT_EQ(nValue, m_pTestAosRegistration->m_nNetworkBindingFeatures);

    // PROPERTY_PDN_REACIVATE_WAIT_TIME
    m_pTestAosRegistration->GetProperty(
            IAosRegistration::PROPERTY_PDN_REACIVATE_WAIT_TIME, nValue, strValue);
    EXPECT_EQ(nValue, 30);
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

    m_pTestAosRegistration->UpdateTxnFeature(PENDING_TERMINATED, IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTerminated());
    m_pTestAosRegistration->UpdateTxnFeature(PENDING_TERMINATED, IMS_FALSE);

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
            .WillRepeatedly(Return(IMS_FALSE));

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
    objReregErrCodeForInitRegWithAvailablePcscf.Add(0);
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
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->SetBlocked(IMS_FALSE);
    m_pTestAosRegistration->SetRadioWaiting(IMS_FALSE);
    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());

    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);

    // IsCallEndAndPdnReactivationByRegTerminated - IMS_FALSE
    m_pTestAosRegistration->Registration_Terminated(IRegistration::REASON_REFRESH_TIMEOUT);
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
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

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
    m_pTestAosRegistration->UpdateTxnFeature(PENDING_UPDATE, IMS_TRUE);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnFeatureOn(PENDING_UPDATE));

    // MSG_REG_RECONFIG - ProcessReconfigPending
    objMsg.nMSG = MSG_REG_RECONFIG;
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pTestAosRegistration->UpdateTxnFeature(PENDING_RECONFIG, IMS_TRUE);
    m_pTestAosRegistration->UpdateTxnFeature(PENDING_UPDATE, IMS_TRUE);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_FALSE(m_pTestAosRegistration->IsTxnFeatureOn(PENDING_UPDATE));
    EXPECT_TRUE(m_pTestAosRegistration->IsTxnFeatureOn(PENDING_RECONFIG));
    m_pTestAosRegistration->UpdateTxnFeature(PENDING_RECONFIG, IMS_FALSE);

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
    EXPECT_EQ(m_pTestAosRegistration->m_pSubscription, nullptr);
    // Registered
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->OnMessage(objMsg);
    EXPECT_EQ(m_pTestAosRegistration->m_pSubscription, nullptr);

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

    // MSG_REG_START - won't handled
    objMsg.nMSG = MSG_REG_START;
    m_pTestAosRegistration->OnMessage(objMsg);
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

TEST_F(AosRegistrationTest, ProcessDefaultFlowRecovery_Start_NotSamePcscf)
{
    EXPECT_CALL(m_objMockIAosPcscf, IncreaseCurrentPcscfTriedCount()).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegActualWaitTimePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::AWT_POLICY_RFC_RULE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegErrCodeWithRetryAfterTimeOnlyDefined())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    EXPECT_CALL(m_objMockIAosPcscf, SetCurrentPcscfInvalid(IMS_TRUE, _)).Times(1);

    m_pTestAosRegistration->ProcessDefaultFlowRecovery_Start();
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
            .WillRepeatedly(Return(0));
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

    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));
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

    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

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
            .WillRepeatedly(Return(0));
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

    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

    m_pTestAosRegistration->m_nState = IAosRegistration::STATE_REGISTERING;

    m_pTestAosRegistration->ProcessStartFailed_StatusCode(500);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosRegistrationTest, Registration_UpdateFailed_CallEndByReregErr)
{
    // BEGIN uninteresting preparation
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

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
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

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
            .WillRepeatedly(Return(0));

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

    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

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
            .WillRepeatedly(Return(0));

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

    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));

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