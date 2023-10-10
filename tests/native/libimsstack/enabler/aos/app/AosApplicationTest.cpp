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
#include "AStringArray.h"
#include "ImsEventDef.h"
#include "ImsMap.h"
#include "INetworkWatcher.h"
#include "ServiceNetworkPolicy.h"

#include "../../../config/interface/CarrierConfig.h"
#include "../../../config/interface/ImsServiceConfig.h"

#include "../../interface/aos/MockIAosService.h"
#include "../../interface/aos/MockIImsAosMonitor.h"

#include "app/MockAosAppContext.h"
#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosLocationStarter.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosRegStateManager.h"
#include "interface/MockIAosRetryRepository.h"

#include "AoSAppRequestType.h"
#include "AosReason.h"
#include "IImsAosInfo.h"
#include "ImsAosParameter.h"
#include "app/AosApplication.h"
#include "condition/AosCondition.h"
#include "connection/AosConnection.h"
#include "connection/AosConnector.h"
#include "handle/AosFeatureTag.h"
#include "registration/AosRegistration.h"
#include "interface/AosInternalMsgDef.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistrationControlListener.h"

#include "provider/AosProvider.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosStaticProfile.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

enum
{
    TIMER_INVALID = -1,
    TIMER_RECONFIG_GUARD = 0,
    TIMER_MSG_CONITION,
    TIMER_REG_STOP,
    TIMER_REG_BLOCKED,
    TIMER_APP_ACTIVATED,
    TIMER_APP_CONNECTED,
    TIMER_APP_TERMINATED,
    TIMER_PDN_BLOCKED,
    TIMER_IMS_ESTABLISHMENT
};

enum
{
    // State-Machine MSG
    MSG_CONDITION = AOSMSG_SERVICE_INTERNAL,
    MSG_CONNECTION,
    MSG_REGISTRATION,

    // NO State-Machine MSG
    MSG_INIT = AOSMSG_SERVICE_INTERNAL + 10,
    MSG_REG_START,
    MSG_REG_UPDATE,
    MSG_REG_STOP,
    MSG_REG_RECONFIG,
    MSG_REG_RECOVER,
    MSG_IPCAN_CHANGED,
    MSG_PUB_TERMINATED,
    MSG_DESTROY,
    MSG_IMS_EST_TIMER_CONTROL,
    MSG_REG_EXCHANGE,
    MSG_AC_CONFIGURED,
    MSG_PCSCF_RECOVER,
    MSG_SCSCF_RESTORATION,
    MSG_PLMN_BLOCK_WITH_TIMEOUT,
    MSG_RETRY_COUNT_INCREASE,
    MSG_OTHERS
};

enum
{
    CONNECTION_ACTIVATED = 10,
    CONNECTION_DEACTIVATED,
    CONNECTION_UPDATED
};

enum
{
    RETRY_COUNT_REG_NONE = 0,
    RETRY_COUNT_REG_RECOVER
};

enum
{
    REQUEST_NONE = 0,
    REQUEST_STOP,
    REQUEST_DESTROY,
    REQUEST_RECOVER,
    REQUEST_PDN_DISCONNECT
};

enum
{
    ACCESS_NETWORK_TYPE_GERAN = 1,
    ACCESS_NETWORK_TYPE_UTRAN = 2,
    ACCESS_NETWORK_TYPE_EUTRAN = 3,
    ACCESS_NETWORK_TYPE_IWLAN = 5,
    ACCESS_NETWORK_TYPE_NGRAN = 6
};

class TestAosCondition : public AosCondition
{
    inline explicit TestAosCondition(IN IAosAppContext* piAppContext) :
            AosCondition(piAppContext),
            m_bReady(IMS_FALSE)
    {
    }

    friend class AosApplicationTest;

public:
    inline virtual IMS_BOOL IsReady() { return m_bReady; }
    inline void SetIsReady(IN IMS_BOOL bReady) { m_bReady = bReady; }

private:
    IMS_BOOL m_bReady;
};

class TestAosConnector : public AosConnector
{
    inline explicit TestAosConnector(IN IAosAppContext* piAppContext) :
            AosConnector(piAppContext),
            m_bIsPdnDeactivationRequired(IMS_FALSE)
    {
    }

    friend class AosApplicationTest;

public:
    inline IMS_BOOL Start() override { return IMS_TRUE; }
    inline void Stop() override
    {
        if (m_piConnection != IMS_NULL)
        {
            m_piConnection->Deactivate();
        }
    }
    inline void Stop(IN IMS_SINT32 /* nDelayTimeSec */) override {}
    inline void SetPdnDeactivationRequired(IN IMS_BOOL bIsRequired) override
    {
        m_bIsPdnDeactivationRequired = bIsRequired;
    }
    inline IMS_BOOL IsPdnDeactivationRequired() override { return m_bIsPdnDeactivationRequired; }

private:
    IMS_BOOL m_bIsPdnDeactivationRequired;
};

class AppTestAosRegistration : public AosRegistration
{
    inline AppTestAosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosRegistration(piAppContext, strRegId)
    {
    }

    friend class AosApplicationTest;

public:
private:
};

class TestAosApplication : public AosApplication
{
    inline TestAosApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
            AosApplication(piAppContext, strAppId),
            m_pOrigAosCondition(IMS_NULL),
            m_pOrigAosConnector(IMS_NULL),
            m_piOrigAosRegistration(IMS_NULL)
    {
        m_pUtil = AosUtil::GetInstance();
    }

    friend class AosApplicationTest;

    FRIEND_TEST(AosApplicationTest, CreateAndDestroy);
    FRIEND_TEST(AosApplicationTest, GetAndSet);
    FRIEND_TEST(AosApplicationTest, IsPdnDisconnectRequired);
    FRIEND_TEST(AosApplicationTest, Reconfig);
    FRIEND_TEST(AosApplicationTest, OnMessage);
    FRIEND_TEST(AosApplicationTest, ProcessMessage);
    FRIEND_TEST(AosApplicationTest, RegRetryCount);
    FRIEND_TEST(AosApplicationTest, StateMachinePreProcess);
    FRIEND_TEST(AosApplicationTest, StateMachine);
    FRIEND_TEST(AosApplicationTest, Process);
    FRIEND_TEST(AosApplicationTest, RegTerminating);
    FRIEND_TEST(AosApplicationTest, PdnDisconnect);
    FRIEND_TEST(AosApplicationTest, ImsEstablishmentStart);
    FRIEND_TEST(AosApplicationTest, Callback);

public:
    void SetAosCondition(IN AosCondition* pCondition)
    {
        if (pCondition == IMS_NULL)
        {
            m_pCondition = m_pOrigAosCondition;
            m_pOrigAosCondition = IMS_NULL;
        }
        else
        {
            if (m_pCondition != IMS_NULL)
            {
                m_pOrigAosCondition = m_pCondition;
            }

            m_pCondition = pCondition;
        }
    }

    void SetAosConnector(IN AosConnector* pConnector)
    {
        if (pConnector == IMS_NULL)
        {
            m_pConnector = m_pOrigAosConnector;
            m_pOrigAosConnector = IMS_NULL;
        }
        else
        {
            if (m_pConnector != IMS_NULL)
            {
                m_pOrigAosConnector = m_pConnector;
            }

            m_pConnector = pConnector;
        }
    }

    void SetAosRegistration(IN IAosRegistration* piAosRegistration)
    {
        if (piAosRegistration != IMS_NULL)
        {
            m_piOrigAosRegistration = m_piRegistration;
            m_piRegistration = piAosRegistration;
        }
        else
        {
            m_piRegistration = m_piOrigAosRegistration;
        }
    }

    void SetRat(IN IMS_UINT32 nRat) { m_nRat = nRat; }

    void SetLteAttachState(IN IMS_UINT32 nLteAttachState) { m_nLteAttachState = nLteAttachState; }

    void SetLteExtraInfo(IN IMS_UINT32 nLteExtraInfo) { m_nLteExtraInfo = nLteExtraInfo; }

private:
    AosCondition* m_pOrigAosCondition;
    AosConnector* m_pOrigAosConnector;
    IAosRegistration* m_piOrigAosRegistration;
};

class AosApplicationTest : public ::testing::Test
{
public:
    TestAosApplication* m_pTestAosApplication;
    TestAosCondition* m_pTestAosCondition;
    TestAosConnector* m_pTestAosConnector;
    AosStaticProfile* m_pAosStaticProfile;

    IAosCallTracker* m_piAosCallTracker;
    IAosLocationStarter* m_piAosLocationStarter;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosRegStateManager* m_piAosRegStateManager;
    IAosService* m_piAosService;
    IAosRetryRepository* m_piAosRetryRepository;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosCallTracker m_objMockIAosCallTracker;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosLocationStarter m_objMockIAosLocationStarter;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistration m_objMockIAosRegistration;
    MockIAosRegStateManager m_objMockIAosRegStateManager;
    MockIAosService m_objMockIAosService;
    MockIAosRetryRepository m_objMockAosRetryRepository;
    MockIImsAosMonitor m_objMockIImsAosMonitor;

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    ImsMap<AString, IAosHandle*> m_objHandles;
    AStringArray m_objPcscfs;

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
        EXPECT_CALL(m_objMockIAosAppContext, SetSlotId(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_pAosStaticProfile));
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_pAosStaticProfile->GetId()));
        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosBlock));
        EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));
        EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NetworkPolicy::APN_IMS));
        EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        m_objPcscfs.AddElement(AString("192.168.0.100"));
        EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosPcscf));
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPcscfs));
        EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscfIndex())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));
        EXPECT_CALL(m_objMockIAosPcscf, GetChangedType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IAosPcscf::TYPE_CHANGED_SAME));
        EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosNetTracker));
        EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
        EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
        EXPECT_CALL(m_objMockIAosNetTracker, IsRoaming())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));
        EXPECT_CALL(m_objMockIAosNetTracker, SetListener(_)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosRegistration));
        EXPECT_CALL(m_objMockIAosRegistration, SetAppReady(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, Destroy()).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, SetListener(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, Start()).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, Update(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, Stop()).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, Reconfig()).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, GetMode())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IAosRegistration::MODE_NORMAL));
        EXPECT_CALL(m_objMockIAosRegistration, IsRefreshing())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));
        EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
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
        EXPECT_CALL(m_objMockIAosHandle, GetMonitor())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIImsAosMonitor));

        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(
                static_cast<IAosCallTracker*>(&m_objMockIAosCallTracker), SLOT_ID);
        EXPECT_CALL(m_objMockIAosCallTracker, RemoveListener(_)).Times(AnyNumber());

        m_piAosLocationStarter = AosProvider::GetInstance()->GetLocationStarter();
        AosProvider::GetInstance()->SetLocationStarter(
                static_cast<IAosLocationStarter*>(&m_objMockIAosLocationStarter), SLOT_ID);

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration), SLOT_ID);
        EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED));
        EXPECT_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockIAosService), SLOT_ID);

        m_piAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository(SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(
                static_cast<IAosRetryRepository*>(&m_objMockAosRetryRepository), SLOT_ID);

        m_piAosRegStateManager = AosProvider::GetInstance()->GetRegStateManager();
        AosProvider::GetInstance()->SetRegStateManager(
                static_cast<IAosRegStateManager*>(&m_objMockIAosRegStateManager), SLOT_ID);
        EXPECT_CALL(m_objMockIAosRegStateManager, SetImsRegState(_, _)).Times(AnyNumber());

        m_pTestAosApplication =
                new TestAosApplication(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetId());

        m_pTestAosCondition =
                new TestAosCondition(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        m_pTestAosApplication->SetAosCondition(m_pTestAosCondition);

        m_pTestAosConnector =
                new TestAosConnector(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        m_pTestAosApplication->SetAosConnector(m_pTestAosConnector);

        m_pTestAosApplication->SetAosRegistration(
                static_cast<IAosRegistration*>(&m_objMockIAosRegistration));
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetRegStateManager(m_piAosRegStateManager, SLOT_ID);
        AosProvider::GetInstance()->SetRetryRepository(m_piAosRetryRepository, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetLocationStarter(m_piAosLocationStarter, SLOT_ID);
        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker, SLOT_ID);

        if (m_pTestAosConnector)
        {
            delete m_pTestAosConnector;
        }

        if (m_pTestAosCondition)
        {
            delete m_pTestAosCondition;
        }

        if (m_pTestAosApplication)
        {
            delete m_pTestAosApplication;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }
};

TEST_F(AosApplicationTest, CreateAndDestroy)
{
    m_pTestAosApplication->SetAosCondition(IMS_NULL);
    m_pTestAosApplication->SetAosConnector(IMS_NULL);

    // TEST_F : CreateAosCondition
    m_pTestAosApplication->CreateAosCondition();

    // TEST_F : CreateAosConnector
    m_pTestAosApplication->CreateAosConnector();

    // TEST_F : CreateAosLocationStarter
    m_pTestAosApplication->CreateAosLocationStarter();
    EXPECT_CALL(m_objMockIAosLocationStarter, Init(_, _)).Times(0);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL, SLOT_ID);
    m_pTestAosApplication->CreateAosLocationStarter();
    IAosLocationStarter* piLocationStarter = AosProvider::GetInstance()->GetLocationStarter();
    EXPECT_NE(piLocationStarter, nullptr);

    // TEST_F : AddEventListener
    m_pTestAosApplication->AddEventListener();

    // TEST_F : SetNetTrackerListener
    m_pTestAosApplication->SetNetTrackerListener();

    // TEST_F : CleanUp
    m_pTestAosApplication->CleanUp();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    piLocationStarter = AosProvider::GetInstance()->GetLocationStarter();
    EXPECT_EQ(piLocationStarter, nullptr);
}

TEST_F(AosApplicationTest, GetAndSet)
{
    // TEST_F : GetActivityName
    m_pTestAosApplication->GetActivityName();

    // TEST_F : SetAppState, GetAppState
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosApplication->GetAppState(), IAosApplication::STATE_NOTREADY);

    // TEST_F : UpdateRegisteredRat, GetProperty
    IMS_UINT32 nValue;
    AString strValue;

    m_pTestAosApplication->UpdateRegisteredRat(NW_REPORT_RADIO_LTE);
    m_pTestAosApplication->GetProperty(IAosApplication::PROPERTY_REGISTERED_RAT, nValue, strValue);
    EXPECT_EQ(nValue, NW_REPORT_RADIO_LTE);

    // TEST_F : SetOffReason, GetOffReason, ClearOffReason
    m_pTestAosApplication->SetOffReason(AosReason::AosReason::POWER_OFF);
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::POWER_OFF);
    m_pTestAosApplication->ClearOffReason();
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::NONE);

    // TEST_F : IsActivated, SetActivation
    m_pTestAosApplication->SetActivation(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->IsActivated());
    m_pTestAosApplication->SetActivation(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosApplication->IsActivated());

    // TEST_F : SetAppState, IsOn
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->IsOn());
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_TRUE(m_pTestAosApplication->IsOn());
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_FALSE(m_pTestAosApplication->IsOn());

    // TEST_F : NotifyEpsFallbackCallState
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pTestAosApplication->NotifyEpsFallbackCallState(IImsAosInfo::EPSFB_CALL_START);

    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_EPS_FALLBACK_STARTED, IMS_FALSE)).Times(1);
    m_pTestAosApplication->NotifyEpsFallbackCallState(IImsAosInfo::EPSFB_CALL_START);

    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_EPS_FALLBACK_STARTED, IMS_TRUE))
            .Times(1);
    m_pTestAosApplication->NotifyEpsFallbackCallState(IImsAosInfo::EPSFB_CALL_FAILED);

    // TEST_F : NotifyPublishState, IsPublished
    m_pTestAosApplication->NotifyPublishState(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->IsPublished());
    m_pTestAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    m_pTestAosApplication->NotifyPublishState(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosApplication->IsPublished());
    m_pTestAosApplication->ClearTimers();

    // TEST_F : SetRegRecoveryHeld, IsRegRecoveryHeld
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->IsRegRecoveryHeld());
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosApplication->IsRegRecoveryHeld());

    // TEST_F : IsAllDetached
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_ENABLER_DETACHED, _, _))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pTestAosApplication->IsAllDetached());
    EXPECT_FALSE(m_pTestAosApplication->IsAllDetached());

    // TEST_F : ClearTimers, IsTimerRunning
    m_pTestAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    m_pTestAosApplication->StartTimer(TIMER_MSG_CONITION, 1000);
    m_pTestAosApplication->StartTimer(TIMER_REG_BLOCKED, 1000);
    m_pTestAosApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    m_pTestAosApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    m_pTestAosApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    m_pTestAosApplication->ClearTimers();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_STOP));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_MSG_CONITION));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_INVALID));

    // TEST_F : IsRegTypeNormal, IsRegStateUpdatedByNrLteRatChange
    EXPECT_TRUE(m_pTestAosApplication->IsRegTypeNormal());
    EXPECT_TRUE(m_pTestAosApplication->IsRegStateUpdatedByNrLteRatChange());

    // TEST_F : IsNotReady, SetAppState, SetCleanState, IsUpdateAvailable, GetReportState
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_EQ(m_pTestAosApplication->GetReportState(), IAosApplication::APP_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->IsUpdateAvailable());
    m_pTestAosCondition->SetIsReady(IMS_TRUE);
    EXPECT_FALSE(m_pTestAosApplication->IsNotReady());
    m_pTestAosApplication->SetCleanState();
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_READY);
    EXPECT_EQ(m_pTestAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTED);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_EQ(m_pTestAosApplication->GetReportState(), IAosApplication::APP_UPDATING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_EQ(m_pTestAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->IsNotReady());

    // TEST_F : IsStateMessage
    EXPECT_FALSE(m_pTestAosApplication->IsStateMessage(MSG_REG_START));
    EXPECT_TRUE(m_pTestAosApplication->IsStateMessage(MSG_REGISTRATION));

    // TEST_F : IsEmergency, SetAppType
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    EXPECT_TRUE(m_pTestAosApplication->IsEmergency());
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);

    // TEST_F : IsReconfigHandleChanged
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .WillOnce(Return(IAosHandle::DETACH))
            .WillRepeatedly(Return(IAosHandle::ATTACH));
    AosFeatureTagList objBindedList;
    AosFeatureTagList objList;
    objList.AddFeature(ImsAosFeature::MMTEL);
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList()).WillOnce(ReturnRef(objBindedList));
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList()).WillOnce(ReturnRef(objList));
    EXPECT_TRUE(m_pTestAosApplication->IsReconfigHandleChanged());
    EXPECT_TRUE(m_pTestAosApplication->IsReconfigHandleChanged());
    EXPECT_TRUE(m_pTestAosApplication->IsReconfigHandleChanged());

    // TEST_F : IsRequestCmdHeldByCondition, SetImsCall, IsImsCall
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_FALSE(
            m_pTestAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::POWER_OFF));
    EXPECT_TRUE(m_pTestAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::NONE));
    m_pTestAosApplication->ClearPending();
    m_pTestAosApplication->SetImsCall(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::NONE));

    // TEST_F : IsAllHandleDetached
    EXPECT_TRUE(m_pTestAosApplication->IsAllHandleDetached());
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_FALSE(m_pTestAosApplication->IsAllHandleDetached());

    // TEST_F : IsConditionTimerSkippedDueToTimer
    EXPECT_FALSE(m_pTestAosApplication->IsConditionTimerSkippedDueToTimer());

    // TEST_F : IsRegUpdatedByNrLteRatChange
    ImsVector<IMS_SINT32> objRegUpdateRats;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUpdateRegistrationWithRatChange())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegUpdateRats));
    EXPECT_FALSE(m_pTestAosApplication->IsRegUpdatedByNrLteRatChange());
    objRegUpdateRats.Add(ACCESS_NETWORK_TYPE_EUTRAN);
    objRegUpdateRats.Add(ACCESS_NETWORK_TYPE_NGRAN);
    EXPECT_TRUE(m_pTestAosApplication->IsRegUpdatedByNrLteRatChange());
    objRegUpdateRats.Clear();
}

TEST_F(AosApplicationTest, Reconfig)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_SERVICE_CONNECTING, IMS_TRUE)).Times(1);

    // TEST_F : IsEqualOrLessState
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosApplication->Reconfig();
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    m_pTestAosApplication->StopTimer(TIMER_RECONFIG_GUARD);
}

TEST_F(AosApplicationTest, IsPdnDisconnectRequired)
{
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED, _, _))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // blocked with BLOCK_IMS_SERVICE_DISABLED
    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    // reason is IMS_DISABLED
    m_pTestAosApplication->SetOffReason(AosReason::IMS_DISABLED);
    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    // reason is NONE
    m_pTestAosApplication->SetOffReason(AosReason::NONE);
    EXPECT_FALSE(m_pTestAosApplication->IsPdnDisconnectRequired());
}

TEST_F(AosApplicationTest, RequestCmd)
{
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // ImsAosControl::REGISTER_START
    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(1)
            .WillOnce(Return(IAosRegistration::STATE_REGISTERING));

    EXPECT_FALSE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_START));

    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillOnce(Return(IAosRegistration::STATE_OFFLINE));

    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_START));

    // ImsAosControl::REGISTER_START_WITH_WLAN
    EXPECT_FALSE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_START_WITH_WLAN));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillOnce(Return(IAosRegistration::STATE_OFFLINE));
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_START_WITH_WLAN));

    // ImsAosControl::REGISTER_REFRESH
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_REFRESH));

    // ImsAosControl::REGISTER_STOP
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_STOP));

    // ImsAosControl::REGISTER_STOP_BY_ROAMING
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_STOP_BY_ROAMING));

    // ImsAosControl::REGISTER_REINITIATE
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_REINITIATE));

    // ImsAosControl::REGISTER_REINITIATE_BY_CSFB
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_REINITIATE_BY_CSFB));

    // ImsAosControl::PCSCF_NEXT
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::PCSCF_NEXT));

    // ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY));

    // ImsAosControl::IPSEC_DISABLED
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(AnyNumber());
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::IPSEC_DISABLED));

    // ImsAosControl::RETRY_COUNT_INCREASE
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::RETRY_COUNT_INCREASE));

    // ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(
            ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    // ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION
    EXPECT_TRUE(
            m_pTestAosApplication->RequestCmd(ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION));

    // ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT));

    // etc
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION));
    EXPECT_TRUE(
            m_pTestAosApplication->RequestCmd(ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION));

    // unspecified CmdType
    IMS_UINT32 nUnspecifiedCmdType = -1;
    EXPECT_FALSE(m_pTestAosApplication->RequestCmd(nUnspecifiedCmdType));
}

TEST_F(AosApplicationTest, OnMessage)
{
    // non State Message
    ImsMessage objNotHandledMsg(MSG_INIT, 0, 0);
    EXPECT_FALSE(m_pTestAosApplication->OnMessage(objNotHandledMsg));

    // STATE_NOTREADY
    ImsMessage objConditionMsg(MSG_CONDITION, 0, 0);
    ImsMessage objConnectionMsg(MSG_CONNECTION, 0, 0);
    ImsMessage objRegistrationMsg(MSG_REGISTRATION, 0, 0);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConnectionMsg));

    // STATE_READY
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_READY);
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConnectionMsg));

    // STATE_CONNECTING
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objRegistrationMsg));

    // STATE_CONNECTED
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objRegistrationMsg));

    // STATE_UPDATING
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objRegistrationMsg));

    // STATE_DISCONNECTING
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pTestAosApplication->OnMessage(objRegistrationMsg));
}

TEST_F(AosApplicationTest, ProcessMessage)
{
    // MSG_REG_START
    // TEST_F : ProcessRegStart
    // Report_StateChanged - Report_Notify, UpdateRegState, IsRegTypeNormal, IsOn
    ImsMessage objMessage(MSG_REG_START, 0, 0);

    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_CALL(m_objMockIAosBlock, PrintBlockReasons())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_REG_UPDATE
    // TEST_F : ProcessRegUpdate
    objMessage.nMSG = MSG_REG_UPDATE;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_REG_STOP
    // TEST_F : ProcessRegStop - ProcessStateStart
    objMessage.nMSG = MSG_REG_STOP;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillOnce(Return(IAosRegistration::STATE_DEREGISTERING))
            .WillRepeatedly(Return(IAosRegistration::STATE_REGISTERED));
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_STOP));

    m_pTestAosApplication->SetPublishState(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_STOP));
    m_pTestAosApplication->ClearTimers();

    EXPECT_CALL(m_objMockIAosRegistration, Stop()).Times(1);
    m_pTestAosApplication->SetPublishState(IMS_FALSE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_STOP));

    // MSG_REG_RECONFIG
    // TEST_F : ProcessRegReconfig, ResetBlock
    objMessage.nMSG = MSG_REG_RECONFIG;
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_ENABLER_DETACHED, _, _))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    // IsAllDetached return true
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    // IsAllDetached return false
    EXPECT_CALL(m_objMockIAosRegistration, Reconfig()).Times(1);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // MSG_REG_RECOVER
    // TEST_F : ProcessRegRecovery
    objMessage.nMSG = MSG_REG_RECOVER;
    EXPECT_CALL(m_objMockIAosRegistration, Start()).Times(1);
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    objMessage.nWparam = AoSRegRecoveryType::PCSCF_CHANGE;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    objMessage.nWparam = 0;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_IPCAN_CHANGED
    // TEST_F : ProcessIpcanChanged
    objMessage.nMSG = MSG_IPCAN_CHANGED;
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED, _))
            .Times(1);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->StartTimer(TIMER_RECONFIG_GUARD, 1000);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pTestAosApplication->ClearTimers();

    // MSG_PUB_TERMINATED
    // TEST_F : ProcessRegStopTimerExpired
    objMessage.nMSG = MSG_PUB_TERMINATED;
    EXPECT_CALL(m_objMockIAosRegistration, Stop()).Times(1);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->ClearTimers();

    // MSG_DESTROY
    // TEST_F : ProcessDestroy
    objMessage.nMSG = MSG_DESTROY;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_READY);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pTestAosApplication->ClearTimers();

    // MSG_IMS_EST_TIMER_CONTROL
    // TEST_F : ProcessImsEstablishmentControl
    objMessage.nMSG = MSG_IMS_EST_TIMER_CONTROL;
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_REG_EXCHANGE
    // TEST_F : ProcessRegExchange
    objMessage.nMSG = MSG_REG_EXCHANGE;
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_AC_CONFIGURED
    // TEST_F : ProcessAutoConfigurationComplete
    objMessage.nMSG = MSG_AC_CONFIGURED;
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_PCSCF_RECOVER
    // TEST_F : ProcessPcscfRecovery
    objMessage.nMSG = MSG_PCSCF_RECOVER;
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->ClearPending();
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    m_objPcscfs.AddElement(AString("192.168.0.101"));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(1);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_objPcscfs.RemoveElement(AString("192.168.0.101"), IMS_FALSE);

    // MSG_SCSCF_RESTORATION
    // TEST_F : ProcessScscfRestoration
    objMessage.nMSG = MSG_SCSCF_RESTORATION;
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->ClearPending();

    // MSG_PLMN_BLOCK_WITH_TIMEOUT
    // TEST_F : ProcessPlmnBlock
    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosApplication->SetNetTrackerListener();
    objMessage.nMSG = MSG_PLMN_BLOCK_WITH_TIMEOUT;
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(2);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // MSG_RETRY_COUNT_INCREASE
    // TEST_F : ProcessRegRetryCount
    objMessage.nMSG = MSG_RETRY_COUNT_INCREASE;
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrMaxCount()).Times(1).WillOnce(Return(0));
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_OTHERS
    // TEST_F : ProcessOthers
    objMessage.nMSG = MSG_OTHERS;
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, RegRetryCount)
{
    ImsMessage objMessage(MSG_RETRY_COUNT_INCREASE, RETRY_COUNT_REG_NONE, 0);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrMaxCount())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(3));

    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockAosRetryRepository, IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    objMessage.nWparam = RETRY_COUNT_REG_RECOVER;
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(1);

    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // HasNextPcscf: IMS_TRUE
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, StateMachinePreProcess)
{
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    ImsMessage objMessageCnd(MSG_CONDITION, 0, 0);

    // TEST_F : PreprocessStateMessage - PreprocessStateMessage_Connection
    // PreprocessStateMessage_Connection - CONNECTION_ACTIVATED
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UPDATE_IPCAN, _))
            .Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    // not support WFC
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnx));
    // support WFC
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnx));

    // PreprocessStateMessage_Connection - CONNECTION_DEACTIVATED
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT, _))
            .Times(1);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnx));

    // TEST_F : PreprocessStateMessage_Condition
    // STATE_NOTREADY, AosBlock with invalid UICC, Emergency
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetState()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_NOTREADY, AosBlock with invalid UICC, Normal, IMS PDN not requested
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosConnection, GetState()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_NOTREADY, AosBlock with invalid UICC, IMS PDN not connected
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .WillOnce(Return(IAosConnection::STATE_ACTIVATING));
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(1);
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_NOTREADY, AosBlock with invalid UICC, IMS PDN connected
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetState()).WillOnce(Return(IAosConnection::STATE_ACTIVE));
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_READY, AosBlock not blocked with invalid UICC
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_CONNECTED, AosBlock with invalid UICC
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _)).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnd));
}

TEST_F(AosApplicationTest, StateMachine)
{
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    ImsMessage objMessageCnd(MSG_CONDITION, 0, 0);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_SUCCESS, 0);
    m_pTestAosApplication->SetNetTrackerListener();

    // TEST_F : StateNotReady_Condition
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pTestAosCondition->SetIsReady(IMS_TRUE);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    // StateNotReady_Condition - TIMER_MSG_CONITION is running
    m_pTestAosApplication->StartTimer(TIMER_MSG_CONITION, 1000);
    EXPECT_TRUE(m_pTestAosApplication->StateNotReady_Condition(objMessageCnd));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // StateNotReady_Condition - TIMER_MSG_CONITION is not running
    m_pTestAosApplication->ClearTimers();
    EXPECT_TRUE(m_pTestAosApplication->StateNotReady_Condition(objMessageCnd));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_READY);

    // TEST_F : StateNotReady_Connection
    EXPECT_TRUE(m_pTestAosApplication->StateNotReady_Connection(objMessageCnx));

    // TEST_F : StateReady_Condition
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Condition(objMessageCnd));

    // TEST_F : StateReady_Connection
    // StateReady_Connection - CONNECTION_ACTIVATED
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTING);

    // StateReady_Connection - CONNECTION_DEACTIVATED
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));

    // StateReady_Connection - CONNECTION_DEACTIVATED - REASON_PERMANENTLY_FAILED
    objMessageCnx.nLparam = AosConnector::REASON_PERMANENTLY_FAILED;
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_PERMANENT_DATA_FAILED, IMS_TRUE)).Times(1);
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));

    // StateReady_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));

    // TEST_F : StateConnecting_Condition
    objMessageCnd.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Condition(objMessageCnd));

    // TEST_F : StateConnecting_Connection
    // StateConnecting_Connection - CONNECTION_DEACTIVATED - ProcessConnectionDeactivated
    // REASON_PERMANENTLY_FAILED
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = AosConnector::REASON_PERMANENTLY_FAILED;
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_PERMANENT_DATA_FAILED, IMS_FALSE))
            .Times(1);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetAppState(), IAosApplication::STATE_NOTREADY);

    // StateConnecting_Connection - CONNECTION_DEACTIVATED - ProcessConnectionDeactivated
    // not REASON_PERMANENTLY_FAILED
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    objMessageCnx.nLparam = AosConnector::REASON_NONE;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetAppState(), IAosApplication::STATE_NOTREADY);

    // StateConnecting_Connection - CONNECTION_UPDATED (ProcessConnectionUpdated)
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Connection(objMessageCnx));

    // StateConnecting_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Connection(objMessageCnx));

    // TEST_F : StateConnecting_Registration
    // StateConnecting_Registration - RESULT_SUCCESS (ProcessRegSucceeded)
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    m_pTestAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // StateConnecting_Registration - RESULT_TRYING (ProcessRegTrying_StateConnecting)
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));

    // StateConnecting_Registration - won't handled
    objMessageReg.nWparam = IAosRegistration::RESULT_NONE;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));

    // StateConnecting_Registration - RESULT_FAILURE (ProcessRegFailed_StateConnecting)
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    // RESULT_FAILURE - ProcessRegAuthenticationFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_FORBIDDEN;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessRegFailed_Terminated
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    m_pTestAosApplication->ClearTimers();
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    m_pTestAosApplication->ClearTimers();
    // RESULT_FAILURE - ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessPdnBlock
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_BANNDED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessRegFailed_Start
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_GENERAL;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));

    // TEST_F : StateConnected_Condition
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Condition(objMessageReg));

    // TEST_F : StateConnected_Connection
    // StateConnected_Connection - ProcessConnectionDeactivated
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));
    m_pTestAosApplication->ClearTimers();

    // StateConnected_Connection - ProcessConnectionUpdated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    // StateConnected_Connection - ProcessConnectionUpdated - REASON_IP_CHANGED
    objMessageCnx.nLparam = AosConnector::REASON_IP_CHANGED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));

    // StateConnected_Connection - ProcessConnectionUpdated - REASON_PCSCF_CHANGED
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIAosPcscf, GetChangedType())
            .WillOnce(Return(IAosPcscf::TYPE_CHANGED_SAME))
            .WillOnce(Return(IAosPcscf::TYPE_CHANGED_REORDER))
            .WillOnce(Return(IAosPcscf::TYPE_CHANGED_DIFFERENT));
    objMessageCnx.nLparam = AosConnector::REASON_PCSCF_CHANGED;
    // REASON_PCSCF_CHANGED - ProcessConnectionUpdated_Pcscf - TYPE_CHANGED_SAME
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));
    // ProcessConnectionUpdated_Pcscf - TYPE_CHANGED_REORDER
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPcscfUpdatePolicy())
            .WillOnce(Return(CarrierConfig::Assets::REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME));
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));
    // REASON_PCSCF_CHANGED - ProcessConnectionUpdated_Pcscf - TYPE_CHANGED_DIFFERENT
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));

    // StateConnected_Connection - ProcessConnectionUpdated - REASON_IPCAN_CAT_CHANGED
    objMessageCnx.nLparam = AosConnector::REASON_IPCAN_CAT_CHANGED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));

    // StateConnected_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));

    // TEST_F : StateConnected_Registration
    // StateConnected_Registration - ProcessRegTrying_StateConnected
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    // ProcessRegTrying_StateConnected - REASON_TRYING_START
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_START;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    // ProcessRegTrying_StateConnected - REASON_TRYING_UPDATE
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_UPDATE;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_UPDATING);
    // ProcessRegTrying_StateConnected - REASON_TRYING_STOP
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_STOP;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    // ProcessRegTrying_StateConnected - won't handled
    objMessageReg.nLparam = IAosRegistration::REASON_NONE;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // StateConnected_Registration - ProcessRegFailed_StateConnected
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    // ProcessRegFailed_StateConnected - won't handled
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessRegFailed_Terminated
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));

    // StateConnected_Registration - won't handled
    objMessageReg.nWparam = IAosRegistration::RESULT_NONE;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));

    // TEST_F : StateUpdating_Condition
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Condition(objMessageCnd));

    // TEST_F : StateUpdating_Connection
    // StateUpdating_Connection - ProcessConnectionDeactivated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateUpdating_Connection - ProcessConnectionUpdated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = AosConnector::REASON_IP_CHANGED;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateUpdating_Connection - won't handled
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_UPDATING);

    // TEST_F : StateUpdating_Registration
    // StateUpdating_Registration - won't handled
    objMessageReg.nWparam = IAosRegistration::RESULT_NONE;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));

    // StateUpdating_Registration - ProcessRegSucceeded
    objMessageReg.nWparam = IAosRegistration::RESULT_SUCCESS;
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillOnce(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIImsAosMonitor, ImsAosMonitor_Connected(_, _)).Times(1);
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTED);

    // StateUpdating_Registration - ProcessRegTrying_StateUpdating - REASON_TRYING_START
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_START;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    // StateUpdating_Registration - ProcessRegTrying_StateUpdating - REASON_TRYING_STOP
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_STOP;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    // StateUpdating_Registration - ProcessRegTrying_StateUpdating - won't handled
    objMessageReg.nLparam = IAosRegistration::REASON_NONE;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    // StateUpdating_Registration - ProcessRegFailed_StateUpdating
    // ProcessRegFailed_StateUpdating - ProcessRegFailed_Update
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    objMessageReg.nLparam = 0;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    m_pTestAosApplication->SetOffReason(AosReason::NONE);
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::NONE);
    m_pTestAosApplication->SetImsCall(IMS_FALSE);

    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_CALL(m_objMockIAosRegistration, IsRefreshing()).WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessRegAuthenticationFailed
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_AUTHENTICATION_FAILED, IMS_TRUE)).Times(1);
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_FORBIDDEN;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // ProcessRegFailed_StateUpdating - ProcessRegFailed_Terminated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // ProcessRegFailed_StateUpdating - ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::REG_FAILURE);
    // ProcessRegFailed_StateUpdating - ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessPdnBlock
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_BANNDED;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));

    // TEST_F : StateDisconnecting_Condition
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Condition(objMessageCnd));

    // TEST_F : StateDisconnecting_Connection
    // StateDisconnecting_Connection - ProcessConnectionDeactivated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateDisconnecting_Connection - ProcessConnectionUpdated_StateDisconnecting
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    objMessageCnx.nLparam = AosConnector::REASON_IP_CHANGED;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateDisconnecting_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Connection(objMessageCnx));

    // TEST_F : StateDisconnecting_Registration
    // StateDisconnecting_Registration - RESULT_TRYING
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    // StateDisconnecting_Registration - RESULT_SUCCESS
    objMessageReg.nWparam = IAosRegistration::RESULT_SUCCESS;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosApplicationTest, Process)
{
    // TEST_F : ProcessDisconnectingState
    m_pTestAosApplication->ProcessDisconnectingState(AosReason::NONE);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessNetworkEvent
    m_pTestAosApplication->ProcessNetworkEvent(
            IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED, IMS_LTE_INFO_EXTRA_NONE);
    m_pTestAosApplication->ProcessNetworkEvent(IMS_EVENT_VOICE_SERVICE_STATE, 0, 0);

    // TEST_F : ProcessRegControlEvent
    // ProcessRegControlEvent - won't handled
    m_pTestAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_IPCAN, 0);
    // ProcessRegControlEvent - IMS_REG_CONTROL_RECOVER
    m_pTestAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_RECOVER, 0);
    m_pTestAosApplication->ProcessRegControlEvent(
            IMS_REG_CONTROL_RECOVER, IMS_REG_CONTROL_KEEP_DATA_CONNECTION);
    // ProcessRegControlEvent - IMS_REG_CONTROL_UPDATE
    m_pTestAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_UPDATE, 0);
    // ProcessRegControlEvent - IMS_REG_CONTROL_DESTROY
    m_pTestAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_DESTROY, 0);
    // ProcessRegControlEvent - IMS_REG_CONTROL_STOP
    m_pTestAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_STOP, 0);

    // TEST_F : ProcessRegTerminated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->ProcessRegTerminated();
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    m_pTestAosApplication->ClearTimers();

    // TEST_F : ProcessPingCommand
    m_pTestAosApplication->ProcessPingCommand();

    // TEST_F : ProcessAppActivatedTimerExpired
    m_pTestAosApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    m_pTestAosApplication->ProcessAppActivatedTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_MSG_CONITION));
    m_pTestAosApplication->ClearTimers();

    // TEST_F : ProcessAppConnectedTimerExpired
    m_pTestAosApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    m_pTestAosApplication->ProcessAppConnectedTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));

    // TEST_F : ProcessAppTerminatedTimerExpired
    m_pTestAosApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));
    m_pTestAosApplication->ProcessAppTerminatedTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // TEST_F : ProcessReconfigTimerExpired
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_ENABLER_DETACHED, _, _))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosHandle::ATTACH));

    // AllHandleDetached - AllDetached
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pTestAosApplication->ProcessReconfigTimerExpired();
    // AllHandleDetached - not AllDetached - Registered
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pTestAosApplication->ProcessReconfigTimerExpired();
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    // AllHandleDetached - not AllDetached - not Registered
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pTestAosApplication->ProcessReconfigTimerExpired();
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // Not AllHandleDetached - NotReady state
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pTestAosApplication->ProcessReconfigTimerExpired();
    // Not AllHandleDetached - Connecting state - IsReconfigHandleChanged return true
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillOnce(Return(IMS_TRUE));
    m_pTestAosApplication->ProcessReconfigTimerExpired();
    // Not AllHandleDetached - Connecting state - IsReconfigHandleChanged return false
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_FALSE));
    m_pTestAosApplication->ProcessReconfigTimerExpired();

    // TEST_F : ProcessRegBlockedTimerExpired
    m_pTestAosApplication->StartTimer(TIMER_REG_BLOCKED, 1000);
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    m_pTestAosApplication->ProcessRegBlockedTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessRegStopTimerExpired
    m_pTestAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_STOP));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    m_pTestAosApplication->ProcessRegStopTimerExpired();
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessPdnBlockedTimerExpired
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED, _)).Times(1);
    m_pTestAosApplication->StartTimer(TIMER_PDN_BLOCKED, 1000);
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    m_pTestAosApplication->ProcessPdnBlockedTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));

    // TEST_F : ProcessImsEstablishmentTimerExpired
    m_pTestAosApplication->SetNetTrackerListener();
    m_pTestAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    m_pTestAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pTestAosApplication->ProcessImsEstablishmentTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(0);
    m_pTestAosApplication->ProcessImsEstablishmentTimerExpired();
    m_pTestAosApplication->SetImsCall(IMS_FALSE);

    // TEST_F : UpdateRegRecoveryHeld
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_FALSE(m_pTestAosApplication->UpdateRegRecoveryHeld());
    m_pTestAosApplication->SetImsCall(IMS_FALSE);

    // TEST_F : UpdateRegStopHeld()
    EXPECT_FALSE(m_pTestAosApplication->UpdateRegStopHeld());

    // TEST_F : UpdateMonitorNotify()
    EXPECT_CALL(m_objMockIImsAosMonitor, ImsAosMonitor_Notify(0, 0)).Times(1);
    m_pTestAosApplication->UpdateMonitorNotify(0, 0);
}

TEST_F(AosApplicationTest, RegTerminating)
{
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->SetImsCall(IMS_TRUE);

    m_pTestAosApplication->ProcessRegTerminating();
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::REG_TERMINATING);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
}

TEST_F(AosApplicationTest, PdnDisconnect)
{
    m_pTestAosApplication->SetNetTrackerListener();

    // TEST_F : ProcessPdnDisconnect
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _)).Times(0);
    m_pTestAosApplication->ProcessPdnDisconnect();
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::REG_TERMINATING);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    m_pTestAosApplication->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED));
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(1);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_NR);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(1);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_INVALID);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_NOT_SPECIFIED));
    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _)).Times(0);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    m_pTestAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));
    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _)).Times(0);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    m_pTestAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pTestAosApplication->SetLteExtraInfo(IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(1);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_NR);
    m_pTestAosApplication->SetOffReason(AosReason::DATA_PERMANENTLY_FAILED);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_NOT_SPECIFIED));
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK))
            .Times(1);
    m_pTestAosApplication->ProcessPdnDisconnect();
}

TEST_F(AosApplicationTest, ImsEstablishmentStart)
{
    // TEST_F : ProcessImsEstablishmentStart
    m_pTestAosApplication->SetNetTrackerListener();
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTime())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(10));

    // ImsEstablishmentTime is 0
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(0);
    m_pTestAosApplication->ProcessImsEstablishmentStart();

    // Registered
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(0);
    m_pTestAosApplication->ProcessImsEstablishmentStart();

    // IMS call is active
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(0);
    m_pTestAosApplication->ProcessImsEstablishmentStart();
    m_pTestAosApplication->SetImsCall(IMS_FALSE);

    // IsSupportedNetworkTypeForCellular is false
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillOnce(Return(NW_REPORT_RADIO_INVALID))
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).Times(0);
    m_pTestAosApplication->ProcessImsEstablishmentStart();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // IsImsVoiceCallSupported is false
    m_pTestAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).Times(0);
    m_pTestAosApplication->ProcessImsEstablishmentStart();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // Blocked
    m_pTestAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillOnce(Return(IMS_TRUE));
    m_pTestAosApplication->ProcessImsEstablishmentStart();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // start TIMER_IMS_ESTABLISHMENT
    EXPECT_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->ProcessImsEstablishmentStart();
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, Callback)
{
    // TEST_F : Report_Request
    m_pTestAosApplication->Report_Request(0, 0);

    // TEST_F : Condition_Changed
    m_pTestAosApplication->StartTimer(TIMER_MSG_CONITION, 2000);
    m_pTestAosApplication->Condition_Changed(0);

    // TEST_F : Condition_RequestCommand
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    m_pTestAosApplication->Condition_RequestCommand(REQUEST_PDN_DISCONNECT, AosReason::NONE);
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::NONE);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTED);

    m_pTestAosApplication->Condition_RequestCommand(REQUEST_PDN_DISCONNECT, AosReason::POWER_OFF);
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::POWER_OFF);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->Condition_RequestCommand(REQUEST_DESTROY, AosReason::NONE);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    m_pTestAosApplication->Condition_RequestCommand(REQUEST_RECOVER, AosReason::NONE);
    m_pTestAosApplication->Condition_RequestCommand(REQUEST_NONE, AosReason::NONE);
    m_pTestAosApplication->SetImsCall(IMS_FALSE);
    m_pTestAosApplication->SetOffReason(AosReason::NONE);

    // TEST_F : Connector_Activated
    m_pTestAosApplication->Connector_Activated();

    // TEST_F : Connector_Deactivated
    m_pTestAosApplication->Connector_Deactivated(0);

    // TEST_F : Connector_Updated
    m_pTestAosApplication->Connector_Updated(0);

    // TEST_F : Registration_StateChanged
    m_pTestAosApplication->Registration_StateChanged(0, 0);

    // TEST_F : CallTracker_StateChanged
    m_pTestAosApplication->CallTracker_StateChanged(
            IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT, 0))
            .Times(1);
    m_pTestAosApplication->CallTracker_StateChanged(
            IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : NetTracker_StatusChanged
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pTestAosApplication->NetTracker_StatusChanged();

    // mobile network type is LTE
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_EPS_FALLBACK_STARTED, _)).Times(1);
    m_pTestAosApplication->NetTracker_StatusChanged();

    // mobile network type is NR
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(1);
    m_pTestAosApplication->NetTracker_StatusChanged();

    // mobile network type is NR again while m_nLteAttachState is IMS_LTE_INFO_COMBINED_ATTACHED
    m_pTestAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(1);
    m_pTestAosApplication->NetTracker_StatusChanged();

    // mobile network type is NR again while m_nLteAttachState is not IMS_LTE_INFO_COMBINED_ATTACHED
    m_pTestAosApplication->SetLteAttachState(IMS_LTE_INFO_EPS_ONLY_ATTACHED);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(0);
    m_pTestAosApplication->NetTracker_StatusChanged();

    // TEST_F : NConfiguration_NotifyConfigChanged
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillOnce(Return(IMS_TRUE));
    m_pTestAosApplication->NConfiguration_NotifyConfigChanged();

    // TEST_F : Event_NotifyEvent
    m_pTestAosApplication->Event_NotifyEvent(IMS_EVENT_RTT_SETTING, 0, 0);
    m_pTestAosApplication->Event_NotifyEvent(IMS_EVENT_REG_CONTROL, 0, 0);
    m_pTestAosApplication->Event_NotifyEvent(IMS_EVENT_VOICE_SERVICE_STATE, 0, 0);

    // TEST_F : Timer_TimerExpired
    m_pTestAosApplication->Timer_TimerExpired(IMS_NULL);

    // TEST_F : RegistrationControl_ControlRegistration
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED, _, _))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    // eCause is IMS_SERVICE - eType is START
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_IMS_SERVICE_DISABLED, IMS_TRUE))
            .Times(1);
    m_pTestAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START, AosPcscfOrder::CURRENT, AosControlCause::IMS_SERVICE);
    // eCause is IMS_SERVICE - eType is STOP
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_IMS_SERVICE_DISABLED, IMS_FALSE)).Times(1);
    m_pTestAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::IMS_SERVICE);
    // eCause is PDN_CAPABILITY_CHANGED - eType is STOP
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_EQ(m_pTestAosConnector->IsPdnDeactivationRequired(), IMS_FALSE);
    m_pTestAosApplication->RegistrationControl_ControlRegistration(AosRegRequestType::STOP,
            AosPcscfOrder::CURRENT, AosControlCause::PDN_CAPABILITY_CHANGED);
    EXPECT_EQ(m_pTestAosConnector->IsPdnDeactivationRequired(), IMS_TRUE);

    m_pTestAosApplication->SetImsCall(IMS_FALSE);
    m_pTestAosConnector->SetPdnDeactivationRequired(IMS_FALSE);
    EXPECT_EQ(m_pTestAosConnector->IsPdnDeactivationRequired(), IMS_FALSE);
    m_pTestAosApplication->RegistrationControl_ControlRegistration(AosRegRequestType::STOP,
            AosPcscfOrder::CURRENT, AosControlCause::PDN_CAPABILITY_CHANGED);
    EXPECT_EQ(m_pTestAosConnector->IsPdnDeactivationRequired(), IMS_TRUE);
    // eCause is DATA - eType is START
    m_pTestAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START, AosPcscfOrder::CURRENT, AosControlCause::DATA);
    // eCause is DATA - eType is REFRESH
    m_pTestAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::REFRESH, AosPcscfOrder::CURRENT, AosControlCause::DATA);
    // eCause is DATA - eType is STOP
    m_pTestAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::DATA);

    // TEST_F : ServicePhone_LocationInfoChanged
    // IsReregRetryWithChangedCountryOnWifi return false
    m_pTestAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::COUNTRY_CHANGED);
    // IsReregRetryWithChangedCountryOnWifi return true - eState is COUNTRY_CHANGED
    m_pTestAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::COUNTRY_CHANGED);
    // IsReregRetryWithChangedCountryOnWifi return true - eState is AVAILABLE)
    m_pTestAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::AVAILABLE);
}
