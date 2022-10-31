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

#include "../../../config/interface/ImsServiceConfig.h"
#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISipConfigV.h"
#include "../../../engine/interface/sipcore/MockISipMessage.h"
#include "../../../engine/interface/registration/MockIRegistration.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"

#include "../../../enabler/interface/aos/ImsAosParameter.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistrationListener.h"
#include "interface/MockIAosService.h"
#include "interface/MockIAosSubscriber.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosRegistration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
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

class TestAosRegistration : public AosRegistration
{
    inline TestAosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosRegistration(piAppContext, strRegId),
            m_piMockRegistration(IMS_NULL)
    {
    }

    friend class AosRegistrationTest;
    FRIEND_TEST(AosRegistrationTest, StartAndDestroy);
    FRIEND_TEST(AosRegistrationTest, CheckMode);
    FRIEND_TEST(AosRegistrationTest, Stop);
    FRIEND_TEST(AosRegistrationTest, Update);
    FRIEND_TEST(AosRegistrationTest, Reconfig);
    FRIEND_TEST(AosRegistrationTest, RequestCmd);
    FRIEND_TEST(AosRegistrationTest, CheckBool);
    FRIEND_TEST(AosRegistrationTest, FeatureTagForMtc);
    FRIEND_TEST(AosRegistrationTest, BlockChanged);
    FRIEND_TEST(AosRegistrationTest, RetryCount);

protected:
    inline virtual IRegistration* GetRegistration() { return m_piMockRegistration; }
    inline virtual IMS_BOOL CheckRadioReadyAndSetTxnPending() { return IMS_TRUE; }

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

    inline IMS_BOOL IsRetryCountClear() { return m_nConsecutiveFailure == 0; }

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
    MockIAosNConfiguration m_objMockAosIAosNConfiguration;
    MockIAosNetTracker m_objMockAosINetTracker;
    MockIAosService m_objMockIAosService;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistrationListener m_objMockIAosRegistrationListener;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProflieType(AosStaticProfile::Type::NORMAL);

        IMSList<ImsServiceName> objServiceName =
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

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockAosIAosNConfiguration), SLOT_ID);

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockIAosService), SLOT_ID);

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationRetryBaseTime())
                .Times(AnyNumber())
                .WillRepeatedly(Return(30000));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationRetryMaxTime())
                .Times(AnyNumber())
                .WillRepeatedly(Return(1800000));

        EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosSubscriber));

        EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosPcscf));

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

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosBlock));

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
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);

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

TEST_F(AosRegistrationTest, StartAndDestroy)
{
    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsIpsecEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationPrivateHeader())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_WIFI));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetImsSignallingDscp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(46));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetSipPreferredTransport())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsContactUriValidationChecked())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsUserInfoInContactSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetSipMessageThresholdForTransportChange())
            .Times(AnyNumber())
            .WillRepeatedly(Return(340));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIRegistration, GetParameter())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIRegParameter));

    EXPECT_CALL(m_objMockIRegistration, GetPreviousRequest())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockISipMessage));

    EXPECT_CALL(m_objMockIRegistration, CreateContact(_, _, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIRegContact));

    EXPECT_CALL(m_objMockIRegistration, SetListener(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetSipMessageMediator(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetRefreshPolicy(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, CreateBinding(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetActiveBindingsRestorationUsage(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, SetSipProfile(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegistration, GetPreferredContact())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIRegContact));
    ;

    EXPECT_CALL(m_objMockIRegContact, SetUserInfo(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegContact, RemoveHeaderParameter(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegContact, AddService(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegContact, AddExtraCapability(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIRegContact, AddHeaderParameter(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    ;

    SipAddress objSipAddress("sip:1111@1.1.1.1");
    EXPECT_CALL(m_objMockIRegContact, GetContactAddress())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipAddress));
    ;

    EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIRegParameter, AddPreloadedRoute(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    AString strHeader = AString("regtest");
    EXPECT_CALL(m_objMockISipMessage, GetHeader(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strHeader));

    IMSMap<AString, IAosHandle*> objHandles;
    const IMSList<AosServiceProfile*>& objProfiles = m_pAosStaticProfile->GetServiceProfiles();
    AosServiceProfile* pServiceProfile = IMS_NULL;
    if (objProfiles.GetSize() > 0)
    {
        pServiceProfile = objProfiles.GetAt(0);
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
            .WillRepeatedly(Return(IAosHandle::ATTACH));

    EXPECT_CALL(m_objMockIAosHandle, SetRegBinded(_)).Times(AnyNumber());

    AosFeatureTagList objList;
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objList));

    AosFeatureTagList objBindedList;
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objBindedList));

    EXPECT_CALL(m_objMockIAosHandle, GetServiceType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IMS_UINT32>(ImsAosService::MTS)));

    EXPECT_CALL(m_objMockIAosHandle, SetNetworkRegBinded(_)).Times(AnyNumber());

    AStringArray objImpus;
    objImpus.AddElement(AString("sip:1111@ims.co.kr"));
    objImpus.AddElement(AString("sip:2222@ims.co.kr"));
    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objImpus));

    AStringArray objPcscf;
    objPcscf.AddElement(AString("192.168.0.100"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscf));

    IMSList<IMS_SINT32> objPcscfPorts;
    objPcscfPorts.Append(5060);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfsPorts())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfPorts));

    IPAddress objLocalIpa(AString("192.168.0.1"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objLocalIpa));

    EXPECT_CALL(m_objMockAosINetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

    // TEST_F : Start - StopTimer, CheckRadioReadyAndSetTxnPending
    // , CreateRegistration - PrepareRegistration (IsIpsecInit, DestroySocket)
    // , SetRetryTime, SetAor (IsFakeRegistration, )
    m_pTestAosRegistration->SetMockIRegistration(
            static_cast<IRegistration*>(&m_objMockIRegistration));

    m_pTestAosRegistration->Start();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERING);

    // Destroy()
    m_pTestAosRegistration->SetMockIRegistration(IMS_NULL);
    m_pTestAosRegistration->Destroy();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);
}

TEST_F(AosRegistrationTest, Stop)
{
    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsContactUriValidationChecked())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    // Stop()
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGSTOP);
    m_pTestAosRegistration->Stop();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationPrivateHeader())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO));

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);
    m_pTestAosRegistration->Stop();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_DEREGISTERING);

    m_pTestAosRegistration->SetIRegistration(IMS_NULL);
    m_pTestAosRegistration->Destroy();
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_OFFLINE);

    m_pTestAosRegistration->SetRegType(AosRegistrationType::NORMAL);
}

TEST_F(AosRegistrationTest, Update)
{
    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsContactUriValidationChecked())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

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
    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsContactUriValidationChecked())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredImsDscp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

    m_pTestAosRegistration->SetIRegistration(static_cast<IRegistration*>(&m_objMockIRegistration));

    m_pTestAosRegistration->SetIRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));

    m_pTestAosRegistration->SetRegType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockIRegistration, Restore()).Times(AnyNumber());

    IMSMap<AString, IAosHandle*> objHandles;
    const IMSList<AosServiceProfile*>& objProfiles = m_pAosStaticProfile->GetServiceProfiles();

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

    EXPECT_CALL(m_objMockIAosHandle, GetServiceType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(ImsAosService::MTS));

    EXPECT_CALL(m_objMockIAosHandle, SetRegBinded(_)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    AosFeatureTagList objList;
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objList));

    AosFeatureTagList objBindedList;
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objBindedList));

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

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_REFRESH_REGINFO);

    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsRegWithIpcanChangedDuringImsCallHeld())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosRegistration->SetImsCall(IMS_TRUE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_REGISTERED);

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED);
    EXPECT_EQ(m_pTestAosRegistration->GetState(), IAosRegistration::STATE_REGISTERED);

    m_pTestAosRegistration->SetImsCall(IMS_FALSE);
    m_pTestAosRegistration->SetState(IAosRegistration::STATE_OFFLINE);

    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsContactUriValidationChecked())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosPcscf, RemoveCurrentPcscf()).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfCount()).Times(AnyNumber()).WillRepeatedly(Return(0));
    m_pTestAosRegistration->SetListener(
            static_cast<IAosRegistrationListener*>(&m_objMockIAosRegistrationListener));
    EXPECT_CALL(m_objMockIAosRegistrationListener, Registration_StateChanged(_, _))
            .Times(AnyNumber());
    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION);
    m_pTestAosRegistration->SetListener(IMS_NULL);

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT);

    m_pTestAosRegistration->RequestCmd(IAosRegistration::CMD_REPORT_REG_STATE);

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
}

TEST_F(AosRegistrationTest, CheckMode)
{
    m_pTestAosRegistration->SetMode(IAosRegistration::MODE_NORMAL);
    EXPECT_EQ(m_pTestAosRegistration->GetMode(), IAosRegistration::MODE_NORMAL);

    m_pTestAosRegistration->SetMode(IAosRegistration::MODE_FAKE);
    EXPECT_EQ(m_pTestAosRegistration->GetMode(), IAosRegistration::MODE_FAKE);
}

// TEST_F(AosRegistrationTest, GetProperty)

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

    m_pTestAosRegistration->SetTerminated(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsTerminated());
    m_pTestAosRegistration->SetTerminated(IMS_FALSE);

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

TEST_F(AosRegistrationTest, RetryCount)
{
    EXPECT_CALL(m_objMockAosIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // TEST_F : IncreaseConsecutiveFailCount
    m_pTestAosRegistration->IncreaseConsecutiveFailCount();
    EXPECT_FALSE(m_pTestAosRegistration->IsRetryCountClear());

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegRetryCountResetPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION));

    // TEST_F : ClearRetryCount
    m_pTestAosRegistration->ClearRetryCount();
    EXPECT_FALSE(m_pTestAosRegistration->IsRetryCountClear());

    m_pTestAosRegistration->ClearRetryCount(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosRegistration->IsRetryCountClear());
}