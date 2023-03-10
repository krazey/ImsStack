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

#include "AosReason.h"
#include "ImsAosParameter.h"
#include "app/AosApplication.h"
#include "condition/AosCondition.h"
#include "connection/AosConnection.h"
#include "connection/AosConnector.h"
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
    TIMER_RECONFIG_GUARD = 0,
    TIMER_MSG_CONITION,
    TIMER_REG_STOP,
    TIMER_REG_BLOCKED,
    TIMER_APP_ACTIVATED,
    TIMER_APP_CONNECTED,
    TIMER_APP_TERMINATED,
    TIMER_PDN_BLOCKED
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
    MSG_SERVICE_CONTROL,
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
            AosConnector(piAppContext)
    {
    }

    friend class AosApplicationTest;

public:
    inline IMS_BOOL Start() override { return IMS_TRUE; }
    inline void Stop() override {}
    inline void Stop(IN IMS_SINT32 /* nDelayTimeSec */) override {}

private:
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
    FRIEND_TEST(AosApplicationTest, ProcessMessage);
    FRIEND_TEST(AosApplicationTest, RegRetryCount);
    FRIEND_TEST(AosApplicationTest, StateMachine);
    FRIEND_TEST(AosApplicationTest, Process);
    FRIEND_TEST(AosApplicationTest, RegTerminating);
    FRIEND_TEST(AosApplicationTest, PdnDisconnect);
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
    AppTestAosRegistration* m_pTestAosRegistration;
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

        EXPECT_CALL(m_objMockIAosAppContext, GetHandles())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objHandles));

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

        m_pTestAosRegistration =
                new AppTestAosRegistration(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetRegistrationId());
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

        if (m_pTestAosRegistration)
        {
            delete m_pTestAosRegistration;
        }

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

    // TEST_F : CreateAosLocationStarter
    m_pTestAosApplication->AddEventListener();

    // TEST_F : SetNetTrackerListener
    m_pTestAosApplication->SetNetTrackerListener();

    // CleanUp
    /* TEST_F :
            CleanAll : SetAppState, ClearPending, SetOffReason, ClearTimers, ClearConnection
                , IsPdnDisconnectRequired - IsAllDetached
    */
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // TEST_F : StopTimer

    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL, SLOT_ID);

    // TEST_F : RemoveEventListener
    m_pTestAosApplication->CleanUp();

    AosProvider::GetInstance()->SetLocationStarter(
            static_cast<IAosLocationStarter*>(&m_objMockIAosLocationStarter), SLOT_ID);

    m_pTestAosApplication->SetAosConnector(m_pTestAosConnector);
    m_pTestAosApplication->SetAosCondition(m_pTestAosCondition);
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
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_TRUE(m_pTestAosApplication->IsOn());
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_FALSE(m_pTestAosApplication->IsOn());

    // TEST_F : NotifyPublishState, IsPublished
    m_pTestAosApplication->NotifyPublishState(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->IsPublished());
    m_pTestAosApplication->NotifyPublishState(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosApplication->IsPublished());

    // TEST_F : SetRegRecoveryHeld, IsRegRecoveryHeld
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->IsRegRecoveryHeld());
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosApplication->IsRegRecoveryHeld());

    // TEST_F : IsAllDetached
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTestAosApplication->IsAllDetached());
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_FALSE(m_pTestAosApplication->IsAllDetached());

    // TEST_F : IsTimerRunning
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_STOP));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_MSG_CONITION));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));

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

    // TEST_F : IsRequestCmdHeldByCondition, SetImsCall, IsImsCall
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_FALSE(
            m_pTestAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::POWER_OFF));
    EXPECT_TRUE(m_pTestAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::NONE));
    m_pTestAosApplication->ClearPending();
    m_pTestAosApplication->SetImsCall(IMS_FALSE);

    // TEST_F : IsAllHandleDetached
    EXPECT_TRUE(m_pTestAosApplication->IsAllHandleDetached());

    AString serviceId("ims.service.mts");

    m_objHandles.Add(serviceId, &m_objMockIAosHandle);
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_FALSE(m_pTestAosApplication->IsAllHandleDetached());
    m_objHandles.Clear();

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
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // TEST_F : IsEqualOrLessState
    m_pTestAosApplication->Reconfig();

    m_pTestAosApplication->StopTimer(TIMER_RECONFIG_GUARD);
}

TEST_F(AosApplicationTest, IsPdnDisconnectRequired)
{
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosApplication->SetOffReason(AosReason::IMS_DISABLED);
    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    m_pTestAosApplication->SetOffReason(AosReason::NONE);
    EXPECT_FALSE(m_pTestAosApplication->IsPdnDisconnectRequired());
}

TEST_F(AosApplicationTest, RequestCmd)
{
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));

    // ImsAosControl::REGISTER_START
    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(1)
            .WillOnce(Return(IAosRegistration::STATE_REGISTERING));

    EXPECT_FALSE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_START));

    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillOnce(Return(IAosRegistration::STATE_OFFLINE));

    // ImsAosControl::REGISTER_START_WITH_WLAN
    EXPECT_FALSE(m_pTestAosApplication->RequestCmd(ImsAosControl::REGISTER_START_WITH_WLAN));

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

    // ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT));

    // etc
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::RETRY_COUNT_INCREASE));
    EXPECT_TRUE(m_pTestAosApplication->RequestCmd(ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION));
    EXPECT_TRUE(
            m_pTestAosApplication->RequestCmd(ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION));
    EXPECT_TRUE(
            m_pTestAosApplication->RequestCmd(ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION));
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
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosRegistration::STATE_DEREGISTERING));
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetPublishState(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->ClearTimers();

    // MSG_REG_RECONFIG
    // TEST_F : ProcessRegReconfig, ResetBlock
    objMessage.nMSG = MSG_REG_RECONFIG;
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // MSG_REG_RECOVER
    // TEST_F : ProcessRegRecovery
    objMessage.nMSG = MSG_REG_RECOVER;
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // MSG_IPCAN_CHANGED
    // TEST_F : ProcessIpcanChanged
    objMessage.nMSG = MSG_IPCAN_CHANGED;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // MSG_PUB_TERMINATED
    // TEST_F : ProcessRegStopTimerExpired
    objMessage.nMSG = MSG_PUB_TERMINATED;
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

    // MSG_SERVICE_CONTROL
    // TEST_F : ProcessServiceControl
    objMessage.nMSG = MSG_SERVICE_CONTROL;
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
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // MSG_SCSCF_RESTORATION
    // TEST_F : ProcessScscfRestoration
    objMessage.nMSG = MSG_SCSCF_RESTORATION;
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
    m_pTestAosApplication->ClearPending();

    // MSG_PLMN_BLOCK_WITH_TIMEOUT
    // TEST_F : ProcessPlmnBlockWithTimeout
    objMessage.nMSG = MSG_PLMN_BLOCK_WITH_TIMEOUT;
    m_pTestAosApplication->SetNetTrackerListener();
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

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
            .Times(1)
            .WillRepeatedly(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));

    // HasNextPcscf: IMS_TRUE
    EXPECT_TRUE(m_pTestAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, StateMachine)
{
    // TEST_F : PreprocessStateMessage - PreprocessStateMessage_Connection
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosApplication->PreprocessStateMessage(objMessageCnx));

    // TEST_F : StateNotReady_Condition
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pTestAosCondition->SetIsReady(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosApplication->StateNotReady_Condition(objMessageCnx));
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);

    // TEST_F : StateReady_Condition
    m_pTestAosCondition->SetIsReady(IMS_FALSE);
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Condition(objMessageCnx));

    // TEST_F : StateReady_Connection
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    objMessageCnx.nLparam = AosConnector::REASON_PERMANENTLY_FAILED;

    EXPECT_TRUE(m_pTestAosApplication->StateReady_Connection(objMessageCnx));

    // TEST_F : StateConnecting_Condition
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Condition(objMessageCnx));

    // TEST_F : StateConnecting_Connection - ProcessConnectionDeactivated
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Connection(objMessageCnx));
    m_pTestAosApplication->ClearTimers();
    objMessageCnx.nLparam = AosConnector::REASON_PERMANENTLY_FAILED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Connection(objMessageCnx));
    m_pTestAosApplication->ClearTimers();

    // TEST_F : StateConnecting_Registration
    // ProcessRegSucceeded, UpdateConnectedServices
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_SUCCESS, 0);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // ProcessRegTrying_StateConnecting
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // ProcessRegFailed_StateConnecting
    // ProcessRegAuthenticationFailed
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_FORBIDDEN;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // ProcessRegFailed_Terminated
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    m_pTestAosApplication->ClearTimers();
    // ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    m_pTestAosApplication->ClearTimers();
    // ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_CALL(m_objMockIAosRegistration, GetProperty(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(5));
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // ProcessPdnBlock
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_BANNDED;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));
    // ProcessRegFailed_Start
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_GENERAL;
    EXPECT_TRUE(m_pTestAosApplication->StateConnecting_Registration(objMessageReg));

    // TEST_F : StateConnected_Condition
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Condition(objMessageReg));

    // TEST_F : StateConnected_Connection
    // ProcessConnectionDeactivated
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));
    m_pTestAosApplication->ClearTimers();

    // ProcessConnectionUpdated - ProcessConnectionUpdated_Pcscf
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = AosConnector::REASON_PCSCF_CHANGED;
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Connection(objMessageCnx));

    // TEST_F : StateConnected_Registration
    // ProcessRegTrying_StateConnected
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_START;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_UPDATE;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_UPDATING);
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_STOP;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    // ProcessRegFailed_StateConnected
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateConnected_Registration(objMessageReg));

    // TEST_F : StateUpdating_Condition
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Condition(objMessageReg));

    // TEST_F : StateUpdating_Connection
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Connection(objMessageCnx));

    // TEST_F : StateUpdating_Registration
    // ProcessRegTrying_StateUpdating
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_START;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_STOP;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    // TEST_F : ProcessRegFailed_StateUpdating - ProcessRegFailed_Update
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateUpdating_Registration(objMessageReg));

    // TEST_F : StateDisconnecting_Condition
    ImsMessage objMessageCondition(MSG_CONDITION, 0, 0);
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Condition(objMessageCondition));

    // TEST_F : StateDisconnecting_Connection - ProcessConnectionUpdated_StateDisconnecting
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Connection(objMessageCnx));

    // TEST_F : StateDisconnecting_Registration
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pTestAosApplication->StateDisconnecting_Registration(objMessageReg));
}

TEST_F(AosApplicationTest, Process)
{
    // TEST_F : ProcessDisconnectingState
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));

    m_pTestAosApplication->ProcessDisconnectingState(0);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessNetworkEvent
    m_pTestAosApplication->ProcessNetworkEvent(IMS_EVENT_LTE_INFO, 0);
    m_pTestAosApplication->ProcessNetworkEvent(IMS_EVENT_VOICE_SERVICE_STATE, 0);

    // TEST_F : ProcessRegControlEvent
    m_pTestAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_IPCAN, 0);

    // TEST_F : ProcessRegTerminated
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosApplication->ProcessRegTerminated();
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    m_pTestAosApplication->ClearTimers();

    // TEST_F : ProcessPingCommand
    m_pTestAosApplication->ProcessPingCommand();

    // TEST_F : ProcessAppActivatedTimerExpired
    m_pTestAosApplication->ProcessAppActivatedTimerExpired();
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
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosApplication->StartTimer(TIMER_PDN_BLOCKED, 1000);
    EXPECT_TRUE(m_pTestAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    m_pTestAosApplication->ProcessPdnBlockedTimerExpired();
    EXPECT_FALSE(m_pTestAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));

    // TEST_F : UpdateRegRecoveryHeld
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_FALSE(m_pTestAosApplication->UpdateRegRecoveryHeld());
    m_pTestAosApplication->SetImsCall(IMS_FALSE);

    // TEST_F : UpdateRegStopHeld()
    EXPECT_FALSE(m_pTestAosApplication->UpdateRegStopHeld());

    // TEST_F : UpdateMonitorNotify()
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
    // TEST_F : ProcessPdnDisconnect
    m_pTestAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .Times(AnyNumber())
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_NOT_SPECIFIED))
            .WillOnce(Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED))
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    m_pTestAosApplication->ProcessPdnDisconnect();
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::REG_TERMINATING);
    EXPECT_EQ(m_pTestAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    m_pTestAosApplication->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_NR);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_INVALID);
    m_pTestAosApplication->ProcessPdnDisconnect();

    m_pTestAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    m_pTestAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pTestAosApplication->ProcessPdnDisconnect();
}

TEST_F(AosApplicationTest, Callback)
{
    // TEST_F : Report_Request
    m_pTestAosApplication->Report_Request(0, 0);

    // TEST_F : Condition_Changed
    m_pTestAosApplication->StartTimer(TIMER_MSG_CONITION, 2000);
    m_pTestAosApplication->Condition_Changed(0);

    // TEST_F : Condition_RequestCommand
    m_pTestAosApplication->SetImsCall(IMS_TRUE);
    m_pTestAosApplication->Condition_RequestCommand(REQUEST_PDN_DISCONNECT, AosReason::POWER_OFF);
    EXPECT_EQ(m_pTestAosApplication->GetOffReason(), AosReason::POWER_OFF);
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

    // TEST_F : NetTracker_StatusChanged
    m_pTestAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pTestAosApplication->NetTracker_StatusChanged();
    m_pTestAosApplication->SetAppType(AosRegistrationType::NORMAL);

    // TEST_F : NConfiguration_NotifyConfigChanged
    m_pTestAosApplication->NConfiguration_NotifyConfigChanged();

    // TEST_F : Event_NotifyEvent
    m_pTestAosApplication->Event_NotifyEvent(IMS_EVENT_RTT_SETTING, 0, 0);

    // TEST_F : Timer_TimerExpired
    m_pTestAosApplication->Timer_TimerExpired(IMS_NULL);

    // TEST_F : RegistrationControl_ControlRegistration
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START, AosPcscfOrder::CURRENT, AosControlCause::IMS_SERVICE);
}
