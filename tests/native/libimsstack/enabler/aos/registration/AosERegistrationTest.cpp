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
#include "IImsRadio.h"
#include "IIpcan.h"
#include "ImsEventDef.h"
#include "SipStatusCode.h"

#include "../../../config/interface/ImsServiceConfig.h"
#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISipConfigV.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"
#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"

#include "../../../enabler/interface/aos/ImsAosParameter.h"

#include "../../interface/aos/MockIAosService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistrationListener.h"
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosTransaction.h"

#include "interface/AosInternalMsgDef.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosERegistration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

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

class TestAosERegistration : public AosERegistration
{
    inline TestAosERegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosERegistration(piAppContext, strRegId),
            piMockRegistration(IMS_NULL)
    {
    }

    inline IRegistration* GetRegistration() override { return piMockRegistration; }

    friend class AosERegistrationTest;
    FRIEND_TEST(AosERegistrationTest, Start);
    FRIEND_TEST(AosERegistrationTest, Update);
    FRIEND_TEST(AosERegistrationTest, RequestCmd);
    FRIEND_TEST(AosERegistrationTest, OnMessage);
    FRIEND_TEST(AosERegistrationTest, ProcessDefaultFlowRecovery_Start);
    FRIEND_TEST(AosERegistrationTest, ProcessStartFailed_StatusCode);
    FRIEND_TEST(AosERegistrationTest, ProcessDefaultFlowRecovery_Update);
    FRIEND_TEST(AosERegistrationTest, ProcessUpdateFailed_StatusCode);
    FRIEND_TEST(AosERegistrationTest, ProcessStopRetryTimerExpired);
    FRIEND_TEST(AosERegistrationTest, ProcessTransactionTimerExpired);
    FRIEND_TEST(AosERegistrationTest, UpdateTransactionStarted);
    FRIEND_TEST(AosERegistrationTest, Registration_RefreshTimerExpired);
    FRIEND_TEST(AosERegistrationTest, Registration_Started);
    FRIEND_TEST(AosERegistrationTest, Registration_Updated);
    FRIEND_TEST(AosERegistrationTest, Registration_StartFailed);
    FRIEND_TEST(AosERegistrationTest, Registration_Terminated);
    FRIEND_TEST(AosERegistrationTest, CallTracker_StateChanged);
    FRIEND_TEST(AosERegistrationTest, NConfiguration_NotifyConfigChanged);
    FRIEND_TEST(AosERegistrationTest, Event_NotifyEvent);
    FRIEND_TEST(AosERegistrationTest, Transaction_OnConnectionFailed);
    FRIEND_TEST(AosERegistrationTest, Transaction_OnConnectionSetupPrepared);
    FRIEND_TEST(AosERegistrationTest, Transaction_OnTrafficPriorityChanged);
    FRIEND_TEST(AosERegistrationTest, IsEcbmTimer);
    FRIEND_TEST(AosERegistrationTest, ProcessRearrangePcscf);
    FRIEND_TEST(AosERegistrationTest, ProcessReinitiateWithRegState);

public:
    inline void SetMockIRegistration(IN IRegistration* piRegistration)
    {
        piMockRegistration = piRegistration;
    }

    inline void SetIRegistration(IN IRegistration* piRegistration)
    {
        m_piRegistration = piRegistration;
    }

    inline void SetIRegContact(IN IRegContact* piRegContact) { m_piRegContact = piRegContact; }

    inline void SetRegType(IN AosRegistrationType eRegType) { m_eRegType = eRegType; }

    inline IAosRegistrationListener* GetListener() { return m_piListener; }

    inline void SetTerminated(IN IMS_BOOL bAdd)
    {
        if (bAdd)
        {
            m_pUtil->AddFeature(PENDING_TERMINATED, m_nTxnPending);
        }
        else
        {
            m_pUtil->RemoveFeature(PENDING_TERMINATED, m_nTxnPending);
        }
    }

    inline void SetISipConfigV(ISipConfigV* piSipConfigV) { m_pUtil->SetISipConfigV(piSipConfigV); }

    inline IMS_BOOL IsTxnPendingOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nTxnPending);
    }

    inline IMS_BOOL IsFeatureOn(IN IMS_UINT32 nFeature)
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nFeature);
    }

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
    IRegistration* piMockRegistration;
};

class AosERegistrationTest : public ::testing::Test
{
public:
    TestAosERegistration* m_pTestAosERegistration;
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
    MockIAosNetTracker m_objMockAosINetTracker;
    MockIAosService m_objMockIAosService;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistrationListener m_objMockIAosRegistrationListener;
    MockIAosTransaction m_objMockIAosTransaction;

    AosFeatureTagList m_objFeatureTagList;
    AosFeatureTagList m_objBindedFeatureTagList;

    IpAddress m_objIpa;
    SipAddress m_objSipAddress;
    AString m_strHeader;
    AStringArray m_objImpus;
    AStringArray m_objPcscfs;
    ImsList<IMS_SINT32> m_objPcscfPorts;
    ImsVector<IMS_SINT32> m_objWaitTime;
    ImsMap<AString, IAosHandle*> m_objHandles;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

        ImsList<ImsServiceName> objServiceName =
                ImsServiceConfig::GetServiceNames(ImsServiceConfig::GetEmergencyServiceProfile());

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

        m_pTestAosERegistration =
                new TestAosERegistration(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetRegistrationId());

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration), SLOT_ID);

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockIAosService), SLOT_ID);

        m_piAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(
                static_cast<IAosTransaction*>(&m_objMockIAosTransaction), SLOT_ID);

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryBaseTime())
                .Times(AnyNumber())
                .WillRepeatedly(Return(30000));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationRetryMaxTime())
                .Times(AnyNumber())
                .WillRepeatedly(Return(1800000));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsIpsecEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
                .Times(AnyNumber())
                .WillRepeatedly(
                        Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPrivateHeader())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredImsDscp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetImsSignallingDscp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(46));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetSipPreferredTransport())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsContactUriValidationChecked())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsUserInfoInContactSupported())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyPcscfRetryWaitTime())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objWaitTime));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosSubscriber));

        EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosPcscf));

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosBlock));

        EXPECT_CALL(m_objMockIAosPcscf, GetCurrentIndex())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));
        EXPECT_CALL(m_objMockIAosPcscf, HasPcscf(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIAosPcscf, SetFirstPcscfIndex()).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosPcscf, ResetAllPcscfTried()).Times(AnyNumber());

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

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockAosINetTracker));

        EXPECT_CALL(m_objMockISipConfigV, GetConfigurable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIConfigurable));

        EXPECT_CALL(m_objMockIConfigurable, AddListener(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIConfigurable, RemoveListener(_, _)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIConfigurable, Update(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIRegistration, Register(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_SUCCESS));

        EXPECT_CALL(m_objMockIRegistration, Deregister())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_SUCCESS));

        EXPECT_CALL(m_objMockIRegContact, RecalculateCallerCapabilities()).Times(AnyNumber());

        EXPECT_CALL(m_objMockIRegistration, GetParameter())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIRegParameter));

        EXPECT_CALL(m_objMockIRegistration, GetPreviousRequest())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockISipMessage));

        EXPECT_CALL(m_objMockIRegistration, GetPreviousResponse())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockISipMessage));

        EXPECT_CALL(m_objMockIRegistration, CreateContact(_, _, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIRegContact));

        EXPECT_CALL(m_objMockIRegistration, DestroyContact(_)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIRegistration, SetListener(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegistration, SetSipMessageMediator(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegistration, SetRefreshPolicy(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegistration, CreateBinding(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegistration, SetActiveBindingsRestorationUsage(_))
                .Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegistration, SetSipProfile(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegistration, GetPreferredContact())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIRegContact));

        EXPECT_CALL(m_objMockIRegContact, SetUserInfo(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegContact, RemoveHeaderParameter(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegContact, AddService(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegContact, AddExtraCapability(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIRegContact, AddUriParameter(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        SipAddress objSipAddress("sip:1111@1.1.1.1");
        m_objSipAddress = objSipAddress;
        EXPECT_CALL(m_objMockIRegContact, GetContactAddress())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objSipAddress));

        EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(AnyNumber());
        EXPECT_CALL(m_objMockIRegParameter, AddPreloadedRoute(_, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        m_strHeader = AString("regtest");
        EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_strHeader));

        const ImsList<AosServiceProfile*>& objProfiles = m_pAosStaticProfile->GetServiceProfiles();
        if (objProfiles.GetSize() > 0)
        {
            AosServiceProfile* pServiceProfile = objProfiles.GetAt(0);
            m_objHandles.Add(pServiceProfile->GetServiceId(), &m_objMockIAosHandle);
            EXPECT_CALL(m_objMockIAosHandle, GetAppId())
                    .Times(AnyNumber())
                    .WillRepeatedly(ReturnRef(pServiceProfile->GetAppId()));
            EXPECT_CALL(m_objMockIAosHandle, GetServiceId())
                    .Times(AnyNumber())
                    .WillRepeatedly(ReturnRef(pServiceProfile->GetServiceId()));
        }

        EXPECT_CALL(m_objMockIAosAppContext, GetHandles())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objHandles));

        EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IAosHandle::ATTACH));

        EXPECT_CALL(m_objMockIAosHandle, SetRegBinded(_)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objFeatureTagList));

        EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objBindedFeatureTagList));

        EXPECT_CALL(m_objMockIAosHandle, GetServiceType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<IMS_UINT32>(ImsAosService::MTS)));

        EXPECT_CALL(m_objMockIAosHandle, SetNetworkRegBinded(_)).Times(AnyNumber());

        m_objImpus.AddElement(AString("sip:1111@ims.co.kr"));
        m_objImpus.AddElement(AString("sip:2222@ims.co.kr"));
        EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objImpus));
        EXPECT_CALL(m_objMockIAosSubscriber, GetFakeImpus())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objImpus));

        m_objPcscfs.AddElement(AString("192.168.0.100"));
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPcscfs));

        m_objPcscfPorts.Append(5060);
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfsPorts())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPcscfPorts));

        IpAddress objLocalIpa(AString("192.168.0.1"));
        m_objIpa = objLocalIpa;
        EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objIpa));

        EXPECT_CALL(m_objMockAosINetTracker, GetNetworkType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

        EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockAosINetTracker, IsEmergencyLteAttach())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);

        if (m_pTestAosERegistration)
        {
            delete m_pTestAosERegistration;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }
};

TEST_F(AosERegistrationTest, Start)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRoamingPreferredEmcReg())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2000));

    m_pTestAosERegistration->SetMockIRegistration(
            static_cast<IRegistration*>(&m_objMockIRegistration));

    // Start() - Normal
    m_pTestAosERegistration->Start();
    EXPECT_EQ(m_pTestAosERegistration->GetMode(), IAosRegistration::MODE_NORMAL);
    EXPECT_FALSE(m_pTestAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // Destroy()
    m_pTestAosERegistration->SetMockIRegistration(IMS_NULL);
    m_pTestAosERegistration->Destroy();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // Start() - Fake
    m_pTestAosERegistration->SetMockIRegistration(
            static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP));
    EXPECT_CALL(
            m_objMockIAosNConfiguration, IsEmergencyCallBasedOnPauOfNormalRegistrationSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosERegistration->Start();
    EXPECT_EQ(m_pTestAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);
    EXPECT_TRUE(m_pTestAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // Destroy()
    m_pTestAosERegistration->SetMockIRegistration(IMS_NULL);
    m_pTestAosERegistration->Destroy();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, Update)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));

    // STATE_REGSTOP
    m_pTestAosERegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGSTOP);

    m_pTestAosERegistration->Update();

    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    m_pTestAosERegistration->SetIRegistration(IMS_NULL);
    m_pTestAosERegistration->Destroy();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    m_pTestAosERegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    // STATE_REGISTERED
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pTestAosERegistration->Update();

    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);

    m_pTestAosERegistration->SetIRegistration(IMS_NULL);
    m_pTestAosERegistration->Destroy();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // STATE_OFFLINE - It doesn't do anything
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_OFFLINE);
    m_pTestAosERegistration->Update();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, RequestCmd)
{
    // CMD_FAKE_MODE with REASON_FAKE_MODE_SAME_PCSCF
    m_pTestAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_SAME_PCSCF);

    EXPECT_TRUE(m_pTestAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pTestAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);

    // CMD_FAKE_MODE with REASON_FAKE_MODE_NEXT_PCSCF - fail to SetFirstPcscf
    m_pTestAosERegistration->RequestCmd(
            IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // CMD_ECALL_INIT
    m_pTestAosERegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);

    // CMD_ESMS_INIT
    m_pTestAosERegistration->RequestCmd(IAosRegistration::CMD_ESMS_INIT);

    // CMD_IPCAN_CHANGED - It doesn't do anything
    m_pTestAosERegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);
}

TEST_F(AosERegistrationTest, OnMessage)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));

    // MSG_REG_REINITIATE
    m_pTestAosERegistration->SetMockIRegistration(
            static_cast<IRegistration*>(&m_objMockIRegistration));

    ImsMessage objMessage(MSG_REG_REINITIATE, 0, 0);
    m_pTestAosERegistration->OnMessage(objMessage);

    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    m_pTestAosERegistration->SetMockIRegistration(IMS_NULL);
    m_pTestAosERegistration->Destroy();

    // MSG_REG_REINITIATE_WITH_REG_STATE
    m_pTestAosERegistration->SetMockIRegistration(
            static_cast<IRegistration*>(&m_objMockIRegistration));

    ImsMessage objMessage2(MSG_REG_REINITIATE_WITH_REG_STATE, 1, 0);
    m_pTestAosERegistration->OnMessage(objMessage2);

    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);

    m_pTestAosERegistration->SetMockIRegistration(IMS_NULL);
    m_pTestAosERegistration->Destroy();

    // MSG_REG_START - It doesn't do anything
    ImsMessage objMessage3(MSG_REG_START, 0, 0);
    m_pTestAosERegistration->OnMessage(objMessage3);
}

TEST_F(AosERegistrationTest, ProcessDefaultFlowRecovery_Start)
{
    m_pTestAosERegistration->SetReinitiationRequested(IMS_FALSE);
    m_pTestAosERegistration->SetFakeReg(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL))
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));

    // ProcessAuthenticationFailed - ProcessDefaultFlowRecovery_Start
    // IsReinitiationRequested and IsFakeRegistration return false
    // GetPreferredEmergencyRegistration is not PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
    m_pTestAosERegistration->ProcessAuthenticationFailed();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGSTOP);

    // ProcessStartFailed_StatusCode - ProcessDefaultFlowRecovery_Start
    // IsReinitiationRequested and IsFakeRegistration return false
    // GetPreferredEmergencyRegistration is PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
    m_pTestAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_600);
    EXPECT_EQ(m_pTestAosERegistration->IsFakeRegistration(), IMS_TRUE);

    // ProcessStartFailed_TxnTimeout - ProcessDefaultFlowRecovery_Start
    // IsReinitiationRequested return false but IsFakeRegistration return true
    m_pTestAosERegistration->SetReinitiationRequested(IMS_FALSE);
    m_pTestAosERegistration->SetFakeReg(IMS_TRUE);
    m_pTestAosERegistration->ProcessStartFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // ProcessStartFailed_Others - ProcessDefaultFlowRecovery_Start
    // IsReinitiationRequested return true
    m_pTestAosERegistration->SetReinitiationRequested(IMS_TRUE);
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosERegistration->ProcessStartFailed_Others(IRegistration::REASON_STATUS_CODE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosERegistrationTest, ProcessStartFailed_StatusCode)
{
    // SipStatusCode::SC_423
    m_pTestAosERegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    m_pTestAosERegistration->ProcessStartFailed_StatusCode(SipStatusCode::SC_423);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}

TEST_F(AosERegistrationTest, ProcessDefaultFlowRecovery_Update)
{
    // ProcessUpdateFailed_StatusCode - ProcessDefaultFlowRecovery_Update
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pTestAosERegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_600);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // ProcessUpdateFailed_TxnTimeout - ProcessDefaultFlowRecovery_Update
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pTestAosERegistration->ProcessUpdateFailed_TxnTimeout();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // ProcessUpdateFailed_Others - ProcessDefaultFlowRecovery_Update
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REFRESHING);
    m_pTestAosERegistration->ProcessUpdateFailed_Others(IRegistration::REASON_STATUS_CODE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);
}

TEST_F(AosERegistrationTest, ProcessUpdateFailed_StatusCode)
{
    // SipStatusCode::SC_423
    m_pTestAosERegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    EXPECT_CALL(m_objMockISipMessage, GetHeader(ISipHeader::MIN_EXPIRES, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(AString("60")));
    m_pTestAosERegistration->ProcessUpdateFailed_StatusCode(SipStatusCode::SC_423);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosERegistrationTest, ProcessStopRetryTimerExpired)
{
    // Retry is allowed
    m_objWaitTime.Add(10);
    m_objWaitTime.Add(20);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(3));
    EXPECT_CALL(m_objMockIAosPcscf, GetCurrentPcscfTriedCount())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pTestAosERegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosERegistration->ProcessStopRetryTimerExpired();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
    EXPECT_EQ(m_pTestAosERegistration->IsFakeRegistration(), IMS_FALSE);

    // Retry is not allowed
    m_objWaitTime.Clear();
    m_pTestAosERegistration->ProcessStopRetryTimerExpired();
    EXPECT_EQ(m_pTestAosERegistration->IsFakeRegistration(), IMS_TRUE);
}

TEST_F(AosERegistrationTest, ProcessTransactionTimerExpired)
{
    // m_nState is not STATE_REGISTERING
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGSTOP);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration()).Times(0);
    m_pTestAosERegistration->ProcessTransactionTimerExpired();

    // GetPreferredEmergencyRegistration is PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillOnce(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK))
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));
    m_pTestAosERegistration->ProcessTransactionTimerExpired();
    EXPECT_EQ(m_pTestAosERegistration->IsFakeRegistration(), IMS_TRUE);

    // GetPreferredEmergencyRegistration is not PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
    m_pTestAosERegistration->ProcessTransactionTimerExpired();
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, UpdateTransactionStarted)
{
    m_pTestAosERegistration->SetImsCall(IMS_FALSE);
    m_pTestAosERegistration->UpdateTransactionStarted();
    EXPECT_FALSE(m_pTestAosERegistration->IsTransactionStarted());

    m_pTestAosERegistration->SetImsCall(IMS_TRUE);
    m_pTestAosERegistration->UpdateTransactionStarted();
    EXPECT_TRUE(m_pTestAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, Registration_RefreshTimerExpired)
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    // m_piRegistration is null
    m_pTestAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);

    // m_piRegistration is not null and Transaction is not started
    m_pTestAosERegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosERegistration->SetImsCall(IMS_FALSE);
    m_pTestAosERegistration->UpdateTransactionStarted();
    bDoImplicitRefresh = IMS_TRUE;

    m_pTestAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_EQ(bDoImplicitRefresh, IMS_FALSE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHSTOP);

    // m_piRegistration is not null and Transaction is started
    m_pTestAosERegistration->SetImsCall(IMS_TRUE);
    m_pTestAosERegistration->UpdateTransactionStarted();

    m_pTestAosERegistration->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    EXPECT_EQ(bDoImplicitRefresh, IMS_TRUE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);
}

TEST_F(AosERegistrationTest, Registration_Started)
{
    m_pTestAosERegistration->StartTimer(TIMER_STOP_RETRY, 10000);
    m_pTestAosERegistration->StartTimer(TIMER_TRANSACTION, 10000);

    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);
    m_pTestAosERegistration->Registration_Started();
    EXPECT_FALSE(m_pTestAosERegistration->IsTimerRunning(TIMER_STOP_RETRY));
    EXPECT_FALSE(m_pTestAosERegistration->IsTimerRunning(TIMER_TRANSACTION));
}

TEST_F(AosERegistrationTest, Registration_Updated)
{
    EXPECT_CALL(m_objMockIAosTransaction, StopEmergencyTraffic()).Times(1);
    m_pTestAosERegistration->Registration_Updated();
}

TEST_F(AosERegistrationTest, Registration_StartFailed)
{
    m_pTestAosERegistration->StartTimer(TIMER_TRANSACTION, 10000);
    m_pTestAosERegistration->Registration_StartFailed(IRegistration::REASON_NONE);
    EXPECT_FALSE(m_pTestAosERegistration->IsTimerRunning(TIMER_TRANSACTION));
}

TEST_F(AosERegistrationTest, Registration_Terminated)
{
    // there is Ims call
    m_pTestAosERegistration->SetImsCall(IMS_TRUE);
    m_pTestAosERegistration->Registration_Terminated(IRegistration::REASON_NONE);
    EXPECT_TRUE(m_pTestAosERegistration->IsTxnPendingOn(PENDING_TERMINATED));

    // there is no Ims call
    m_pTestAosERegistration->SetImsCall(IMS_FALSE);
    m_pTestAosERegistration->Registration_Terminated(IRegistration::REASON_NONE);
    EXPECT_FALSE(m_pTestAosERegistration->IsTxnPendingOn(PENDING_TERMINATED));
}

TEST_F(AosERegistrationTest, CallTracker_StateChanged)
{
    // nType is not TYPE_EMERGENCY
    m_pTestAosERegistration->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE);

    // nType is TYPE_EMERGENCY - ProcessECallStarted
    m_pTestAosERegistration->SetImsCall(IMS_FALSE);
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REFRESHSTOP);

    m_pTestAosERegistration->CallTracker_StateChanged(
            IAosCallTracker::TYPE_EMERGENCY, CallState::NEW);
    EXPECT_TRUE(m_pTestAosERegistration->IsImsCall());
    EXPECT_TRUE(m_pTestAosERegistration->IsTransactionStarted());
}

TEST_F(AosERegistrationTest, NConfiguration_NotifyConfigChanged)
{
    m_pTestAosERegistration->NConfiguration_NotifyConfigChanged();
    m_pTestAosERegistration->CleanUp();
}

TEST_F(AosERegistrationTest, Event_NotifyEvent)
{
    m_pTestAosERegistration->Event_NotifyEvent(IMS_EVENT_ECM_STATE, IMS_ECM_STATE_ON, 0);
}

TEST_F(AosERegistrationTest, Transaction_OnConnectionFailed)
{
    // STATE_REGISTERED
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosERegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERED);

    // Not STATE_REGISTERED
    m_pTestAosERegistration->SetState(IAosRegistration::STATE_REGISTERING);
    m_pTestAosERegistration->Transaction_OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 0, 0);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosERegistrationTest, Transaction_OnConnectionSetupPrepared)
{
    m_pTestAosERegistration->Transaction_OnConnectionSetupPrepared();
}

TEST_F(AosERegistrationTest, Transaction_OnTrafficPriorityChanged)
{
    m_pTestAosERegistration->Transaction_OnTrafficPriorityChanged();
}

TEST_F(AosERegistrationTest, IsEcbmTimer)
{
    EXPECT_FALSE(m_pTestAosERegistration->IsEcbmTimer());
}

TEST_F(AosERegistrationTest, ProcessRearrangePcscf)
{
    // nRetryMaxCount is 0
    m_pTestAosERegistration->ProcessRearrangePcscf();
    EXPECT_FALSE(m_pTestAosERegistration->IsTimerRunning(TIMER_STOP_RETRY));

    // number of PCSCFs are less than nRetryMaxCount
    m_objWaitTime.Add(10);
    m_objWaitTime.Add(20);
    m_pTestAosERegistration->ProcessRearrangePcscf();
    EXPECT_TRUE(m_pTestAosERegistration->IsTimerRunning(TIMER_STOP_RETRY));
    m_pTestAosERegistration->StopTimer(TIMER_STOP_RETRY);

    // number of PCSCFs are equal to nRetryMaxCount
    m_objPcscfs.AddElement(AString("192.168.0.101"));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(1);
    m_pTestAosERegistration->ProcessRearrangePcscf();

    m_pTestAosERegistration->StopTimer(TIMER_STOP_RETRY);
    m_objWaitTime.Clear();
    m_objPcscfs.RemoveAllElements();
}

TEST_F(AosERegistrationTest, ProcessReinitiateWithRegState)
{
    AStringArray objEmptyImpus;
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objEmptyImpus))
            .WillRepeatedly(ReturnRef(m_objImpus));

    // fail to CreateRegistration
    m_pTestAosERegistration->ProcessReinitiateWithRegState(IMS_TRUE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    // succeed to CreateRegistration - bIsRegistered is true
    m_pTestAosERegistration->SetMockIRegistration(
            static_cast<IRegistration*>(&m_objMockIRegistration));
    m_pTestAosERegistration->ProcessReinitiateWithRegState(IMS_TRUE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REFRESHING);

    // succeed to CreateRegistration - bIsRegistered is false
    m_pTestAosERegistration->ProcessReinitiateWithRegState(IMS_FALSE);
    EXPECT_EQ(m_pTestAosERegistration->GetState(), IAosRegistration::STATE_REGISTERING);
}