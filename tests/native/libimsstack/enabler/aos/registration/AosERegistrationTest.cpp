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
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistrationListener.h"
#include "interface/MockIAosService.h"
#include "interface/MockIAosSubscriber.h"

#include "interface/AosInternalMsgDef.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "handle/AosFeatureTag.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosERegistration.h"

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

    AosFeatureTagList m_objFeatureTagList;
    AosFeatureTagList m_objBindedFeatureTagList;

    IPAddress m_objIpa;
    SipAddress m_objSipAddress;
    AString m_strHeader;
    AStringArray m_objImpus;
    AStringArray m_objPcscfs;
    IMSList<IMS_SINT32> m_objPcscfPorts;
    IMSVector<IMS_SINT32> m_objWaitTime;
    IMSMap<AString, IAosHandle*> m_objHandles;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProflieType(AosStaticProfile::Type::EMERGENCY);

        IMSList<ImsServiceName> objServiceName =
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

        EXPECT_CALL(m_objMockAosIAosNConfiguration, IsIpsecEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationPreferredAccessTypeFeatureTag())
                .Times(AnyNumber())
                .WillRepeatedly(
                        Return(CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRegistrationPrivateHeader())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredImsDscp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_DSCP_NONE));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetImsSignallingDscp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(46));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetSipPreferredTransport())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, IsContactUriValidationChecked())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, IsUserInfoInContactSupported())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetEmergencyPcscfRetryWaitTime())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objWaitTime));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
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

        const IMSList<AosServiceProfile*>& objProfiles = m_pAosStaticProfile->GetServiceProfiles();
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

        IPAddress objLocalIpa(AString("192.168.0.1"));
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
    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));

    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetRoamingPreferredEmcReg())
            .Times(AnyNumber())
            .WillRepeatedly(Return(
                    CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED));

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
    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredEmergencyRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP));
    EXPECT_CALL(m_objMockAosIAosNConfiguration,
            IsEmergencyCallBasedOnPauOfNormalRegistrationSupported())
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
    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredEmergencyRegistration())
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
}

TEST_F(AosERegistrationTest, RequestCmd)
{
    // CMD_FAKE_MODE
    m_pTestAosERegistration->RequestCmd(IAosRegistration::CMD_FAKE_MODE);

    EXPECT_TRUE(m_pTestAosERegistration->IsFakeRegistration());
    EXPECT_EQ(m_pTestAosERegistration->GetMode(), IAosRegistration::MODE_FAKE);

    // CMD_ECALL_INIT
    m_pTestAosERegistration->RequestCmd(IAosRegistration::CMD_ECALL_INIT);

    // CMD_ESMS_INIT
    m_pTestAosERegistration->RequestCmd(IAosRegistration::CMD_ESMS_INIT);
}

TEST_F(AosERegistrationTest, OnMessage)
{
    EXPECT_CALL(m_objMockAosIAosNConfiguration, GetPreferredEmergencyRegistration())
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
}
