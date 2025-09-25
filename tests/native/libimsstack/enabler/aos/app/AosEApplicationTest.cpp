/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <TestPhoneInfoService.h>

#include "AString.h"
#include "AStringArray.h"
#include "ImsMap.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "TestTimerService.h"
#include "MockIThread.h"
#include "MockITimer.h"

#include "../../../config/interface/CarrierConfig.h"
#include "../../../config/interface/ImsServiceConfig.h"

#include "../../interface/aos/MockIImsAosMonitor.h"

#include "condition/MockAosCondition.h"
#include "connection/MockAosConnector.h"
#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistration.h"

#include "AosAppRequestType.h"
#include "AosReason.h"
#include "ImsAosParameter.h"
#include "app/AosEApplication.h"
#include "condition/AosCondition.h"
#include "connection/AosConnection.h"
#include "connection/AosConnector.h"
#include "registration/AosRegistration.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base)                                \
    using Base::CallTracker_ECallSessionReleased;          \
    using Base::CallTracker_StateChanged;                  \
    using Base::ClearConnection;                           \
    using Base::ClearTimers;                               \
    using Base::Condition_RequestCommand;                  \
    using Base::GetAppState;                               \
    using Base::GetState;                                  \
    using Base::IsRegWaitingRequired;                      \
    using Base::IsECallConnectedNetworkUnavailable;        \
    using Base::IsImsCall;                                 \
    using Base::IsKeepEPdnWhenNoPcscf;                     \
    using Base::IsRegBlockInCbm;                           \
    using Base::IsReleaseEmergencyPdnUponEmergencyCallEnd; \
    using Base::IsTimerRunning;                            \
    using Base::NetTracker_StatusChanged;                  \
    using Base::ProcessAppActivatedTimerExpired;           \
    using Base::ProcessAppConnectedTimerExpired;           \
    using Base::ProcessAppTerminatedTimerExpired;          \
    using Base::ProcessConnectionUpdated;                  \
    using Base::ProcessECallStarted;                       \
    using Base::ProcessECallTerminated;                    \
    using Base::ProcessMessage;                            \
    using Base::ProcessReconfigTimerExpired;               \
    using Base::ProcessRegBlockedTimerExpired;             \
    using Base::ProcessRegStart;                           \
    using Base::SetAppState;                               \
    using Base::SetAppType;                                \
    using Base::SetImsCall;                                \
    using Base::SetKeepEPdnWhenNoPcscf;                    \
    using Base::SetRegBlockInCbm;                          \
    using Base::StateConnected_Connection;                 \
    using Base::StateConnected_Registration;               \
    using Base::StateConnecting_Connection;                \
    using Base::StateConnecting_Registration;              \
    using Base::StateDisconnecting_Connection;             \
    using Base::StateNotReady_Condition;                   \
    using Base::StateReady_Condition;                      \
    using Base::StateReady_Connection;                     \
    using Base::StateUpdating_Registration;                \
    using Base::StartTimer;                                \
    using Base::StopTimer;                                 \
    using Base::UpdateConnectedServices;

const IMS_SINT32 SLOT_ID = 0;

enum
{
    TIMER_INVALID = -1,
    TIMER_RECONFIG_GUARD = 0,
    TIMER_MSG_CONDITION,
    TIMER_REG_STOP,
    TIMER_REG_BLOCKED,
    TIMER_APP_ACTIVATED,
    TIMER_APP_CONNECTED,
    TIMER_APP_TERMINATED,
    TIMER_PDN_BLOCKED,
    TIMER_IMS_ESTABLISHMENT,
    TIMER_RAT_BLOCK
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
    REQUEST_NONE = 0,
    REQUEST_STOP,
    REQUEST_DESTROY,
    REQUEST_RECOVER,
    REQUEST_PDN_DISCONNECT
};

class TestAosEApplication : public AosEApplication
{
public:
    DECLARE_USING(AosEApplication)

    inline TestAosEApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
            AosEApplication(piAppContext, strAppId)
    {
        m_pUtil = AosUtil::GetInstance();
    }

    inline void SetAosCondition(IN AosCondition* pCondition) { m_pCondition = pCondition; }

    inline void SetAosConnector(IN AosConnector* pConnector) { m_pConnector = pConnector; }

    inline void SetAosRegistration(IN IAosRegistration* piAosRegistration)
    {
        m_piRegistration = piAosRegistration;
    }

    inline void SetAosCallTracker(IN IAosCallTracker* piAosCallTracker)
    {
        m_piCallTracker = piAosCallTracker;
    }

    inline void SetEpdgEnabled(IN IMS_BOOL bEnabled) { m_bEpdgEnabled = bEnabled; }

private:
};

MATCHER_P(IsSameMsg, message, "")
{
    return arg.nMSG == message;
}

class AosEApplicationTest : public ::testing::Test
{
public:
    TestAosEApplication* m_pTestAosEApplication;
    TestThreadService m_objThreadService;
    TestTimerService m_objTimerService;
    TestPhoneInfoService m_objPhoneInfoService;
    AosStaticProfile* m_pAosStaticProfile;
    IAosNConfiguration* m_piAosNConfiguration;

    MockAosCondition m_objMockAosCondition;
    MockAosConnector m_objMockAosConnector;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosCallTracker m_objMockIAosCallTracker;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosPcscf m_objMockIAosPcscf;
    MockIAosRegistration m_objMockIAosRegistration;
    MockIImsAosMonitor m_objMockIImsAosMonitor;
    MockIThread m_objMockThread;
    MockITimer m_objMockITimer;

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    ImsMap<AString, IAosHandle*> m_objHandles;
    AStringArray m_objPcscfs;
    ImsVector<IMS_SINT32> m_objEmptyCauses;

protected:
    void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

        ImsList<ImsServiceName> objServiceName =
                ImsServiceConfig::GetServiceNames(ImsServiceConfig::GetServiceProfile());

        for (IMS_UINT32 i = 0; i < objServiceName.GetSize(); i++)
        {
            ImsServiceName objService = objServiceName.GetAt(i);
            m_pAosStaticProfile->AddService(objService.GetAppId(), objService.GetServiceId());
        }
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(SLOT_ID));

        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_pAosStaticProfile->GetId()));

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosBlock));

        EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NetworkPolicy::APN_EMERGENCY));

        EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosPcscf));

        m_objPcscfs.AddElement(AString("192.168.0.100"));
        EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPcscfs));

        EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscfIndex())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        EXPECT_CALL(m_objMockIAosPcscf, GetChangedType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IAosPcscf::TYPE_CHANGED_SAME));

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosNetTracker));

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

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration), SLOT_ID);

        ON_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosNConfiguration, GetSipTimerT1()).WillByDefault(Return(2000));
        ON_CALL(m_objMockIAosNConfiguration, GetNetworkAttachRejectCausesForCrossStackRedial())
                .WillByDefault(ReturnRef(m_objEmptyCauses));

        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        m_objTimerService.SetTimer(&m_objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);

        m_pTestAosEApplication =
                new TestAosEApplication(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetId());

        m_pTestAosEApplication->SetAosCondition(&m_objMockAosCondition);

        m_pTestAosEApplication->SetAosConnector(&m_objMockAosConnector);
        EXPECT_CALL(m_objMockAosConnector, Stop()).Times(AnyNumber());

        m_pTestAosEApplication->SetAosRegistration(
                static_cast<IAosRegistration*>(&m_objMockIAosRegistration));
        ON_CALL(m_objMockIAosRegistration, IsInCallbackMode()).WillByDefault(Return(IMS_FALSE));
        EXPECT_CALL(m_objMockIAosRegistration, SetAppReady(_)).Times(AnyNumber());
        EXPECT_CALL(m_objMockIAosRegistration, Destroy()).Times(AnyNumber());

        m_pTestAosEApplication->SetAosCallTracker(
                static_cast<IAosCallTracker*>(&m_objMockIAosCallTracker));

        m_pTestAosEApplication->SetAppType(AosRegistrationType::EMERGENCY);
    }

    void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

        if (m_pTestAosEApplication)
        {
            m_pTestAosEApplication->ClearTimers();
            m_pTestAosEApplication->StopTimer(TIMER_RECONFIG_GUARD);
            m_pTestAosEApplication->StopTimer(TIMER_PDN_BLOCKED);
            m_pTestAosEApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
            m_pTestAosEApplication->StopTimer(TIMER_RAT_BLOCK);
            PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
            delete m_pTestAosEApplication;
            m_pTestAosEApplication = IMS_NULL;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }
};

TEST_F(AosEApplicationTest, ReturnsFalseWhenRequestCmdWithStartAndNotReady)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    IMS_BOOL bResult = m_pTestAosEApplication->RequestCmd(ImsAosControl::REGISTER_START, 0);

    EXPECT_FALSE(bResult);
}

TEST_F(AosEApplicationTest, ReturnsTrueWhenRequestCmdWithStartAndReady)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);

    IMS_BOOL bResult = m_pTestAosEApplication->RequestCmd(ImsAosControl::REGISTER_START, 0);

    EXPECT_TRUE(bResult);
}

TEST_F(AosEApplicationTest, InitEmergencyVariableWhenRegisterStartCalled)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->SetKeepEPdnWhenNoPcscf(IMS_TRUE);
    m_pTestAosEApplication->SetRegBlockInCbm(IMS_TRUE);

    EXPECT_TRUE(m_pTestAosEApplication->RequestCmd(ImsAosControl::REGISTER_START, 0));

    EXPECT_FALSE(m_pTestAosEApplication->IsKeepEPdnWhenNoPcscf());
    EXPECT_FALSE(m_pTestAosEApplication->IsRegBlockInCbm());
}

TEST_F(AosEApplicationTest, ReturnsTrueWhenRequestCmdWithStop)
{
    EXPECT_TRUE(m_pTestAosEApplication->RequestCmd(ImsAosControl::REGISTER_STOP, 0));
}

TEST_F(AosEApplicationTest, ShouldRequestFakeModeWithNextPcscfIfPdnActive)
{
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_FAKE_MODE,
                    IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF));

    m_pTestAosEApplication->RequestCmd(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF, 0);
}

TEST_F(AosEApplicationTest, ShouldNotRequestFakeModeWithNextPcscfIfPdnNotActive)
{
    ON_CALL(m_objMockIAosConnection, GetState()).WillByDefault(Return(IAosConnection::STATE_IDLE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(
                    IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_NEXT_PCSCF))
            .Times(0);

    m_pTestAosEApplication->RequestCmd(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF, 0);
}

TEST_F(AosEApplicationTest, ReturnsFalseWhenRequestCmdWithNotCoveredCommands)
{
    IMS_UINT32 nCmdNotCovered = 999;
    EXPECT_FALSE(m_pTestAosEApplication->RequestCmd(nCmdNotCovered, 0));
}

TEST_F(AosEApplicationTest, ShouldRequestFakeModeWithSamePcscfIfPdnActive)
{
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_FAKE_MODE,
                    IAosRegistration::REASON_FAKE_MODE_SAME_PCSCF));

    m_pTestAosEApplication->RequestCmd(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF, 0);
}

TEST_F(AosEApplicationTest, ShouldNotRequestFakeModeWithSamePcscfIfPdnNotActive)
{
    ON_CALL(m_objMockIAosConnection, GetState()).WillByDefault(Return(IAosConnection::STATE_IDLE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(
                    IAosRegistration::CMD_FAKE_MODE, IAosRegistration::REASON_FAKE_MODE_SAME_PCSCF))
            .Times(0);

    m_pTestAosEApplication->RequestCmd(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF, 0);
}

TEST_F(AosEApplicationTest, RegisterStartCmdShouldRegStopWhenNwAttachRejected)
{
    IMS_SINT32 nRejectCause = 3;
    ImsVector<IMS_SINT32> objCauses;
    objCauses.Add(nRejectCause);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetNetworkAttachRejectCausesForCrossStackRedial())
            .WillOnce(ReturnRef(objCauses));
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkRegistrationRejectCause())
            .WillOnce(Return(nRejectCause));
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);

    EXPECT_TRUE(m_pTestAosEApplication->RequestCmd(ImsAosControl::REGISTER_START, 0));
}

TEST_F(AosEApplicationTest, ResetRegBlockInCbmWhenECallIsInitiated)
{
    m_pTestAosEApplication->SetRegBlockInCbm(IMS_TRUE);

    m_pTestAosEApplication->RequestCmd(IAosApplication::CMD_ECALL_INIT);

    EXPECT_FALSE(m_pTestAosEApplication->IsRegBlockInCbm());
}

TEST_F(AosEApplicationTest, ResetRegBlockInCbmWhenESmsIsInitiated)
{
    m_pTestAosEApplication->SetRegBlockInCbm(IMS_TRUE);

    m_pTestAosEApplication->RequestCmd(IAosApplication::CMD_ESMS_INIT);

    EXPECT_FALSE(m_pTestAosEApplication->IsRegBlockInCbm());
}

TEST_F(AosEApplicationTest, GetProperty)
{
    IMS_UINT32 nValue;
    AString strValue;
    m_pTestAosEApplication->GetProperty(IAosApplication::PROPERTY_REGISTERED_RAT, nValue, strValue);
    EXPECT_EQ(strValue, AString::ConstNull());
}

TEST_F(AosEApplicationTest, ClearConnection)
{
    m_pTestAosEApplication->ClearConnection();
}

TEST_F(AosEApplicationTest, ProcessMessage)
{
    // MSG_DESTROY
    ImsMessage objMessage(MSG_DESTROY, 0, 0);
    EXPECT_TRUE(m_pTestAosEApplication->ProcessMessage(objMessage));

    // MSG_OTHERS
    objMessage.nMSG = MSG_OTHERS;
    EXPECT_FALSE(m_pTestAosEApplication->ProcessMessage(objMessage));
}

TEST_F(AosEApplicationTest, ProcessRegStart)
{
    ImsMessage objMessage(MSG_REG_START, AosRegType::TYPE_IPCAN_WLAN, 0);
    // STATE_CONNECTED
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis()).Times(0);
    EXPECT_CALL(m_objMockIAosHandle, App_StateChanged(_, _)).Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosHandle, App_Notify()).Times(AnyNumber());
    EXPECT_TRUE(m_pTestAosEApplication->ProcessMessage(objMessage));

    // STATE_CONNECTING - Connector::IsReady() return true
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1000));
    EXPECT_CALL(m_objMockAosConnector, IsReady()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistration, Start()).Times(1);
    EXPECT_TRUE(m_pTestAosEApplication->ProcessMessage(objMessage));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    m_pTestAosEApplication->StopTimer(TIMER_APP_CONNECTED);

    // STATE_CONNECTING - Connector::IsReady() return false
    EXPECT_CALL(m_objMockAosConnector, IsReady()).WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosConnector, Start()).Times(AnyNumber());
    EXPECT_TRUE(m_pTestAosEApplication->ProcessMessage(objMessage));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_READY);
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    m_pTestAosEApplication->StopTimer(TIMER_APP_CONNECTED);

    // STATE_CONNECTING - Connector::IsReady() return false, TYPE_IPCAN_MOBILE
    objMessage.nWparam = AosRegType::TYPE_IPCAN_MOBILE;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegTimerForEmcCall()).WillOnce(Return(0));
    EXPECT_TRUE(m_pTestAosEApplication->ProcessMessage(objMessage));
}

TEST_F(AosEApplicationTest,
        ShouldStopAppConnectedTimerIfEpdgEnabledAndKeepERegOnWlanIsRequiredWhenEPdnIsAlreadyConnected)
{
    // GIVEN
    ImsMessage objMessage(MSG_REG_START, AosRegType::TYPE_IPCAN_WLAN, 0);
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration, GetEmergencyRegistrationTimerMillis())
            .WillByDefault(Return(10000));
    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConnector, IsReady()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pTestAosEApplication->ProcessRegStart(objMessage);

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
}

TEST_F(AosEApplicationTest, ProcessRegStop)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    EXPECT_TRUE(m_pTestAosEApplication->ProcessMessage(objMessage));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosEApplicationTest, StateNotReady_Condition)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    ImsMessage objMessageCnd(MSG_CONDITION, 0, 0);
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AOS_INCOMPLETED))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    // EmergencyBlocked
    m_pTestAosEApplication->StateNotReady_Condition(objMessageCnd);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // not EmergencyBlocked
    m_pTestAosEApplication->StateNotReady_Condition(objMessageCnd);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_READY);
}

TEST_F(AosEApplicationTest, StateReady_Connection)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);

    // CONNECTION_ACTIVATED
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTING);

    // CONNECTION_DEACTIVATED
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosEApplicationTest,
        ShouldStopAppConnectedTimerIfEpdgEnabledAndKeepERegOnWlanIsRequiredWhenIpcanIsChanged)
{
    // GIVEN
    ImsMessage objMessageCnx(MSG_IPCAN_CHANGED, 0, 0);
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 10000);

    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pTestAosEApplication->ProcessMessage(objMessageCnx);

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
}

TEST_F(AosEApplicationTest,
        ShouldStopAppConnectedTimerIfEpdgEnabledAndKeepERegOnWlanIsRequiredWhenEPdnIsActivated)
{
    // GIVEN
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 10000);

    ON_CALL(m_objMockIAosNConfiguration, IsKeepERegRetryOnWlanRequired())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConnector, IsReady()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
}

TEST_F(AosEApplicationTest,
        ShouldStopAppConnectedTimerIfIsStopERegTimerOnEpdnConnectedAndRegRetryTimerEnabledWhenEPdnIsActivated)
{
    // GIVEN
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 10000);

    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryTimerMillis()).WillByDefault(Return(10000));
    ON_CALL(m_objMockIAosNConfiguration, IsStopERegTimerOnEpdnConnected())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConnector, IsReady()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
}

TEST_F(AosEApplicationTest,
        ShouldNotStopAppConnectedTimerIfIsStopERegTimerOnEpdnConnectedAndRegRetryTimerDisabledWhenEPdnIsActivated)
{
    // GIVEN
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 10000);

    ON_CALL(m_objMockIAosNConfiguration, GetEmcRegRetryTimerMillis()).WillByDefault(Return(0));
    ON_CALL(m_objMockIAosNConfiguration, IsStopERegTimerOnEpdnConnected())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosConnector, IsReady()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);

    // THEN
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
}

TEST_F(AosEApplicationTest, SetRegBlockInCbmWhenConnectionActivatedInReadyState)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    ON_CALL(m_objMockIAosRegistration, IsInCallbackMode()).WillByDefault(Return(IMS_TRUE));

    // CONNECTION_ACTIVATED
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetAppState(), IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pTestAosEApplication->IsRegBlockInCbm());
}

TEST_F(AosEApplicationTest,
        SetNotReadyStateWhenConnectionActivatedInReadyStateAndRegblockInCbmIsTrue)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->SetRegBlockInCbm(IMS_TRUE);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);

    // CONNECTION_ACTIVATED
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetAppState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosEApplicationTest, ConnectorStartWhenConnectionDeactivatedInReadyStateDuringCbm)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_DEACTIVATED, 0);
    ON_CALL(m_objMockIAosRegistration, IsInCallbackMode()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConnector, Start()).WillRepeatedly(Return(IMS_TRUE));

    // CONNECTION_DEACTIVATED
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);
}

TEST_F(AosEApplicationTest, SetReasonDisconnectedWhenConnectionDeactivatedInReadyState)
{
    // GIVEN
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    ImsMessage objMessageCnx(
            MSG_CONNECTION, CONNECTION_DEACTIVATED, AosConnector::REASON_DISCONNECTED);

    // WHEN
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);

    // THEN
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
}

TEST_F(AosEApplicationTest,
        SetReasonDataPermanentlyFailedWhenConnectionDeactivatedByPermanentlyFailedInReadyState)
{
    // GIVEN
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    ImsMessage objMessageCnx(
            MSG_CONNECTION, CONNECTION_DEACTIVATED, AosConnector::REASON_PERMANENTLY_FAILED);

    // WHEN
    m_pTestAosEApplication->StateReady_Connection(objMessageCnx);

    // THEN
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::DATA_PERMANENTLY_FAILED);
}

TEST_F(AosEApplicationTest, StateReady_Condition)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    ImsMessage objMessageCnd(MSG_CONDITION, 0, 0);
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AOS_INCOMPLETED))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    // not EmergencyBlocked
    m_pTestAosEApplication->StateReady_Condition(objMessageCnd);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_READY);

    // EmergencyBlocked
    m_pTestAosEApplication->StateReady_Condition(objMessageCnd);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosEApplicationTest, ProcessRegFailed_StateUpdating)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_UPDATING);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_FAILURE, 0);
    m_pTestAosEApplication->StateUpdating_Registration(objMessageReg);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::REG_FAILURE);
}

TEST_F(AosEApplicationTest, ProcessRegFailed_StateConnecting)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_FAILURE, 0);
    m_pTestAosEApplication->StateConnecting_Registration(objMessageReg);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::REG_FAILURE);
}

TEST_F(AosEApplicationTest, ProcessRegFailed_StateConnected)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_FAILURE, 0);
    m_pTestAosEApplication->StateConnected_Registration(objMessageReg);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::REG_FAILURE);
}

TEST_F(AosEApplicationTest,
        KeepEPdnWhenProcessRegFailed_StateConnectedIfSettingKeepPdnUntilEModeEnd)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);

    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_FAILURE,
            IAosRegistration::REASON_FAILURE_NO_PCSCF_AVAILABLE);
    m_pTestAosEApplication->StateConnected_Registration(objMessageReg);

    EXPECT_TRUE(m_pTestAosEApplication->IsKeepEPdnWhenNoPcscf());
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::DATA_CONNECTION_MAINTAIN);
}

TEST_F(AosEApplicationTest, ProcessConnectionUpdated_StateDisconnecting)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_UPDATED, 0);
    m_pTestAosEApplication->StateDisconnecting_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::REG_FAILURE);
}

TEST_F(AosEApplicationTest, StateReadyWhenProcessConnectionDeactivatedDuringCbm)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_DEACTIVATED, 0);
    ON_CALL(m_objMockIAosRegistration, IsInCallbackMode()).WillByDefault(Return(IMS_TRUE));

    m_pTestAosEApplication->StateConnected_Connection(objMessageCnx);

    EXPECT_EQ(m_pTestAosEApplication->GetAppState(), IAosApplication::STATE_READY);
}

TEST_F(AosEApplicationTest, ProcessConnectionDeactivated)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_DEACTIVATED, 0);
    m_pTestAosEApplication->StateConnected_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
}

TEST_F(AosEApplicationTest, ProcessConnectionUpdated)
{
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_UPDATED, AosConnector::REASON_IP_CHANGED);

    // STATE_CONNECTED or STATE_UPDATING -  REASON_IP_CHANGED
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosEApplication->StateConnected_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::IP_CHANGED);

    // STATE_CONNECTED or STATE_UPDATING -  REASON_OTHERS
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    objMessageCnx.nLparam = AosConnector::REASON_OTHERS;
    m_pTestAosEApplication->StateConnected_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTED);

    // STATE_CONNECTING
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    objMessageCnx.nLparam = AosConnector::REASON_NONE;
    m_pTestAosEApplication->StateConnecting_Connection(objMessageCnx);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTING);
}

TEST_F(AosEApplicationTest, ProcessRegSucceeded)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    m_pTestAosEApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_SUCCESS, 0);

    m_pTestAosEApplication->StateConnecting_Registration(objMessageReg);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTED);
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosEApplicationTest, ProcessRegFailed_Start)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_FAILURE,
            IAosRegistration::REASON_FAILURE_GENERAL);

    m_pTestAosEApplication->StateConnecting_Registration(objMessageReg);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::REG_FAILURE);
}

TEST_F(AosEApplicationTest, ProcessRegFailed_Update)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_FAILURE,
            IAosRegistration::REASON_FAILURE_GENERAL);

    m_pTestAosEApplication->StateUpdating_Registration(objMessageReg);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::REG_FAILURE);
}

TEST_F(AosEApplicationTest, ProcessAppActivatedTimerExpired)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_READY);
    m_pTestAosEApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    EXPECT_CALL(m_objMockIAosCallTracker, GetCallState(IAosCallTracker::TYPE_EMERGENCY))
            .WillOnce(Return(CallState::IDLE));

    m_pTestAosEApplication->ProcessAppActivatedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::NONE);
}

TEST_F(AosEApplicationTest, ProcessAppConnectedTimerExpired)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTING);

    // PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillOnce(
                    Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK));
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_FAKE_MODE, _)).Times(1);
    m_pTestAosEApplication->ProcessAppConnectedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTING);

    // PREFERRED_EMERGENCY_REGISTRATION_NORMAL
    m_pTestAosEApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredEmergencyRegistration())
            .WillOnce(Return(CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL));
    m_pTestAosEApplication->ProcessAppConnectedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pTestAosEApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
}

TEST_F(AosEApplicationTest, ProcessAppTerminatedTimerExpired)
{
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);

    // CallState is not IDLE
    EXPECT_CALL(m_objMockIAosCallTracker, GetCallState(IAosCallTracker::TYPE_EMERGENCY))
            .WillOnce(Return(CallState::OFFHOOK));
    m_pTestAosEApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    m_pTestAosEApplication->ProcessAppTerminatedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    // CallState is IDLE
    EXPECT_CALL(m_objMockIAosCallTracker, GetCallState(IAosCallTracker::TYPE_EMERGENCY))
            .WillOnce(Return(CallState::IDLE));
    m_pTestAosEApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    m_pTestAosEApplication->ProcessAppTerminatedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosEApplicationTest, ProcessReconfigTimerExpired)
{
    m_pTestAosEApplication->StartTimer(TIMER_RECONFIG_GUARD, 1000);

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SERVICE_CONNECTING))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_SERVICE_CONNECTING, _)).Times(1);

    m_pTestAosEApplication->ProcessReconfigTimerExpired();

    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
}

TEST_F(AosEApplicationTest, ProcessRegBlockedTimerExpired)
{
    // STATE_CONNECTING - Connector::IsReady() return IMS_TRUE
    m_pTestAosEApplication->StartTimer(TIMER_REG_BLOCKED, 8000);
    EXPECT_CALL(m_objMockAosConnector, IsReady()).WillOnce(Return(IMS_TRUE));
    m_pTestAosEApplication->ProcessRegBlockedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_REG_BLOCKED));

    // STATE_CONNECTING - Connector::IsReady() return false
    m_pTestAosEApplication->StartTimer(TIMER_REG_BLOCKED, 8000);
    EXPECT_CALL(m_objMockAosConnector, IsReady()).WillOnce(Return(IMS_FALSE));
    m_pTestAosEApplication->ProcessRegBlockedTimerExpired();
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_REG_BLOCKED));
}

TEST_F(AosEApplicationTest, IsRegWaitingRequiredShouldReturnFalseWhenRegTimerIsNotSet)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegTimerForEmcCall()).WillByDefault(Return(0));

    EXPECT_FALSE(m_pTestAosEApplication->IsRegWaitingRequired());
}

TEST_F(AosEApplicationTest, IsRegWaitingRequiredShouldReturnTrueWhenWifiIsConnected)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegTimerForEmcCall()).WillByDefault(Return(1000));
    ON_CALL(m_objMockIAosNConfiguration, IsRegTimerForECallWithRatCheckEnabled())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockWifiWatcher(), GetState())
            .WillByDefault(Return(IWifiWatcher::STATE_CONNECTED));

    EXPECT_TRUE(m_pTestAosEApplication->IsRegWaitingRequired());
}

TEST_F(AosEApplicationTest, IsRegWaitingRequiredShouldReturnTrueWhenRatIsSupported)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegTimerForEmcCall()).WillByDefault(Return(1000));
    ON_CALL(m_objMockIAosNConfiguration, IsRegTimerForECallWithRatCheckEnabled())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    EXPECT_TRUE(m_pTestAosEApplication->IsRegWaitingRequired());
}

TEST_F(AosEApplicationTest, IsRegWaitingRequiredShouldReturnTrueWhenRatCheckIsDisable)
{
    ON_CALL(m_objMockIAosNConfiguration, GetRegTimerForEmcCall()).WillByDefault(Return(1000));
    ON_CALL(m_objMockIAosNConfiguration, IsRegTimerForECallWithRatCheckEnabled())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_TRUE(m_pTestAosEApplication->IsRegWaitingRequired());
}

TEST_F(AosEApplicationTest, IsECallConnectedNetworkUnavailableShouldReturnTrueIfIwlanIsNotAvailable)
{
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objPhoneInfoService.GetMockWifiWatcher(), GetState())
            .WillByDefault(Return(IWifiWatcher::STATE_DISCONNECTED));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnOfUnavailableNetwork())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosEApplication->IsECallConnectedNetworkUnavailable());
}

TEST_F(AosEApplicationTest, IsECallConnectedNetworkUnavailableShouldReturnTrueIfWwanIsNotAvailable)
{
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NOSRV));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnOfUnavailableNetwork())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosEApplication->IsECallConnectedNetworkUnavailable());
}

TEST_F(AosEApplicationTest, IsECallConnectedNetworkUnavailableShouldReturnFalseIfAvailable)
{
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objPhoneInfoService.GetMockWifiWatcher(), GetState())
            .WillByDefault(Return(IWifiWatcher::STATE_CONNECTED));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnOfUnavailableNetwork())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(m_pTestAosEApplication->IsECallConnectedNetworkUnavailable());
}

TEST_F(AosEApplicationTest, ProcessECallStarted)
{
    m_pTestAosEApplication->SetImsCall(IMS_FALSE);
    m_pTestAosEApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    m_pTestAosEApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    m_pTestAosEApplication->ProcessECallStarted();
    EXPECT_TRUE(m_pTestAosEApplication->IsImsCall());
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest, ResetRegBlockInCbmWhenECallStarted)
{
    m_pTestAosEApplication->SetRegBlockInCbm(IMS_TRUE);
    m_pTestAosEApplication->ProcessECallStarted();

    EXPECT_FALSE(m_pTestAosEApplication->IsRegBlockInCbm());
}

TEST_F(AosEApplicationTest, DoNotStartAppTerminatedTimerAgainIfAlreadyRunningWhenECallTerminated)
{
    m_pTestAosEApplication->SetImsCall(IMS_TRUE);
    m_pTestAosEApplication->StartTimer(TIMER_APP_TERMINATED, 1000);

    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);
    m_pTestAosEApplication->ProcessECallTerminated();

    EXPECT_FALSE(m_pTestAosEApplication->IsImsCall());
    m_pTestAosEApplication->StopTimer(TIMER_APP_TERMINATED);
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedIfRegistrationIsTerminated)
{
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objPhoneInfoService.GetMockWifiWatcher(), GetState())
            .WillByDefault(Return(IWifiWatcher::STATE_CONNECTED));
    ON_CALL(m_objMockIAosRegistration, IsTerminated()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConnector, Stop());

    m_pTestAosEApplication->ProcessECallTerminated();
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedIfNetworkIsUnavailable)
{
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosRegistration, IsTerminated()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_WLAN));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnOfUnavailableNetwork())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NOSRV));

    EXPECT_CALL(m_objMockAosConnector, Stop());

    m_pTestAosEApplication->ProcessECallTerminated();
}

TEST_F(AosEApplicationTest, AddNetTrackerListenerWhenECallTerminatedIfNetworkIsAvailable)
{
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosRegistration, IsTerminated()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_WLAN));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnOfUnavailableNetwork())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, SetListener(_));

    m_pTestAosEApplication->ProcessECallTerminated();
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallTerminatedIfSettingKeepPdnUntilEModeEnd)
{
    m_pTestAosEApplication->SetKeepEPdnWhenNoPcscf(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsKeepEPdnUponPcscfUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    m_pTestAosEApplication->ProcessECallTerminated();

    EXPECT_FALSE(m_pTestAosEApplication->IsKeepEPdnWhenNoPcscf());
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest,
        ReleaseEPdnWhenECallTerminatedInFakeModeBySubscriberIncompletedAndAttachedToPlmnRequiresEPdnRelease)
{
    // GIVEN
    ImsVector<AString> objPlmns;
    objPlmns.Add("50501");
    objPlmns.Add("50502");
    ON_CALL(m_objMockIAosNConfiguration, IsKeepEPdnUponPcscfUnavailable())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_FAKE));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnUponECallEndInFakeMode())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetPlmnsReleaseEPdnUponECallEndInFakeMode())
            .WillByDefault(ReturnRef(objPlmns));
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkPlmn())
            .WillByDefault(Return(AString("50502")));
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillOnce(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL result = m_pTestAosEApplication->IsReleaseEmergencyPdnUponEmergencyCallEnd();

    // THEN
    EXPECT_TRUE(result);
}

TEST_F(AosEApplicationTest,
        ReleaseEPdnWhenECallTerminatedInFakeModeBySubscriberIncompletedAndAttachedToPlmnNotRequiresEPdnRelease)
{
    // GIVEN
    ImsVector<AString> objPlmns;
    objPlmns.Add("50501");
    objPlmns.Add("50502");
    ON_CALL(m_objMockIAosNConfiguration, IsKeepEPdnUponPcscfUnavailable())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_FAKE));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnUponECallEndInFakeMode())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetPlmnsReleaseEPdnUponECallEndInFakeMode())
            .WillByDefault(ReturnRef(objPlmns));
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkPlmn())
            .WillByDefault(Return(AString("50503")));  // Different PLMN
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .WillOnce(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL result = m_pTestAosEApplication->IsReleaseEmergencyPdnUponEmergencyCallEnd();

    // THEN
    EXPECT_FALSE(result);
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedInFakeModeIfConfigured)
{
    // GIVEN
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_FAKE));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnUponECallEndInFakeMode())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // Clean Up
    m_pTestAosEApplication->StopTimer(TIMER_APP_TERMINATED);
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallTerminatedInFakeModeIfNotConfigured)
{
    // GIVEN
    ON_CALL(m_objMockIAosRegistration, GetMode())
            .WillByDefault(Return(IAosRegistration::MODE_FAKE));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnUponECallEndInFakeMode())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedOnWlanIfWlanConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_WLAN));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // Clean Up
    m_pTestAosEApplication->StopTimer(TIMER_APP_TERMINATED);
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedOnWlanIfAllIpcanConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_ALL));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // Clean Up
    m_pTestAosEApplication->StopTimer(TIMER_APP_TERMINATED);
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallTerminatedOnWlanIfCellularConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_CELLULAR));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallTerminatedOnWlanIfNoIpanConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_NONE));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedOnCellularIfCellularConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_CELLULAR));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // Clean Up
    m_pTestAosEApplication->StopTimer(TIMER_APP_TERMINATED);
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallTerminatedOnCellularIfAllIpcanConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_ALL));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_TRUE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // Clean Up
    m_pTestAosEApplication->StopTimer(TIMER_APP_TERMINATED);
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallTerminatedOnCellularIfWlanConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_WLAN));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallTerminatedOnCellularIfNoIpcanConfiguredToRelease)
{
    // GIVEN
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_NONE));

    // WHEN
    m_pTestAosEApplication->ProcessECallTerminated();

    // THEN
    EXPECT_FALSE(m_pTestAosEApplication->IsTimerRunning(TIMER_APP_TERMINATED));
}

TEST_F(AosEApplicationTest, ShouldNotStartAppTerminatedTimerWhenECallTerminatedIfWaitTimeIsZero)
{
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_NONE));
    ON_CALL(m_objMockIAosNConfiguration, GetWaitTimeMillisForReleaseEPdnAfterECallEnd())
            .WillByDefault(Return(0));

    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    m_pTestAosEApplication->ProcessECallTerminated();
}

TEST_F(AosEApplicationTest, StartAppTerminatedTimerWithConfiguredValueWhenECallTerminated)
{
    IMS_SINT32 nTime = 240000;
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_NONE));
    ON_CALL(m_objMockIAosNConfiguration, GetWaitTimeMillisForReleaseEPdnAfterECallEnd())
            .WillByDefault(Return(nTime));

    EXPECT_CALL(m_objMockITimer, SetTimer(nTime, _));

    m_pTestAosEApplication->ProcessECallTerminated();
}

TEST_F(AosEApplicationTest, UpdateConnectedServices)
{
    EXPECT_EQ(m_pTestAosEApplication->UpdateConnectedServices(IMS_FALSE), 0);
}

TEST_F(AosEApplicationTest, Condition_RequestCommand)
{
    // other than REQUEST_DESTROY
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pTestAosEApplication->Condition_RequestCommand(REQUEST_STOP);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_CONNECTED);

    // REQUEST_DESTROY
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered()).WillOnce(Return(IMS_TRUE));
    m_pTestAosEApplication->Condition_RequestCommand(REQUEST_DESTROY);
    EXPECT_EQ(m_pTestAosEApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
}

TEST_F(AosEApplicationTest, CallTracker_StateChanged)
{
    // not TYPE_EMERGENCY
    m_pTestAosEApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // ProcessECallStarted
    m_pTestAosEApplication->SetImsCall(IMS_FALSE);
    m_pTestAosEApplication->CallTracker_StateChanged(
            IAosCallTracker::TYPE_EMERGENCY, CallState::NEW);
    EXPECT_TRUE(m_pTestAosEApplication->IsImsCall());

    // ProcessECallTerminated
    m_pTestAosEApplication->SetImsCall(IMS_TRUE);
    m_pTestAosEApplication->CallTracker_StateChanged(
            IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_FALSE(m_pTestAosEApplication->IsImsCall());
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallSessionReleasedIfRegistrationWasTerminated)
{
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_NONE));
    ON_CALL(m_objMockIAosRegistration, IsTerminated()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConnector, Stop());

    m_pTestAosEApplication->CallTracker_ECallSessionReleased(IMS_TRUE);
}

TEST_F(AosEApplicationTest, ReleaseEPdnWhenECallSessionReleasedIfConfiguredToRelease)
{
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_ALL));
    ON_CALL(m_objMockIAosRegistration, IsTerminated()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConnector, Stop());

    m_pTestAosEApplication->CallTracker_ECallSessionReleased(IMS_TRUE);
}

TEST_F(AosEApplicationTest, KeepEPdnWhenECallSessionReleasedIfNotConfiguredToRelease)
{
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_NONE));
    ON_CALL(m_objMockIAosRegistration, IsTerminated()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockAosConnector, Stop()).Times(0);

    m_pTestAosEApplication->CallTracker_ECallSessionReleased(IMS_TRUE);
}

TEST_F(AosEApplicationTest,
        KeepEPdnWhenECallSessionReleasedWithoutEstablishmentIfConfiguredToKeepPdnUntilEModeEnd)
{
    m_pTestAosEApplication->SetImsCall(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsKeepEPdnUponPcscfUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    m_pTestAosEApplication->CallTracker_ECallSessionReleased(IMS_FALSE);

    EXPECT_TRUE(m_pTestAosEApplication->IsImsCall());
}

TEST_F(AosEApplicationTest, ShouldClearConnectionIfCallIsNotExistWhenNetworkChangedToNotAvailable)
{
    m_pTestAosEApplication->SetImsCall(IMS_FALSE);
    m_pTestAosEApplication->SetEpdgEnabled(IMS_FALSE);
    ON_CALL(m_objMockIAosNConfiguration, GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd())
            .WillByDefault(Return(CarrierConfig::ImsEmergency::IPCAN_WLAN));
    ON_CALL(m_objMockIAosNConfiguration, IsReleaseEPdnOfUnavailableNetwork())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NOSRV));
    ON_CALL(m_objMockAosConnector, IsReady()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConnector, Stop());

    m_pTestAosEApplication->NetTracker_StatusChanged();
}

TEST_F(AosEApplicationTest, ShouldNotifyRegistrationIfIpcanIsChangedWhileTheConfigIsTrue)
{
    // GIVEN
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyReregSupportedOnIpcanChange())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED, _));

    // WHEN
    ImsMessage objMessage(MSG_IPCAN_CHANGED, 0, 0);
    m_pTestAosEApplication->ProcessMessage(objMessage);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosEApplicationTest, ShouldNotNotifyRegistrationIfIpcanIsChangedWhileTheConfigIsFalse)
{
    // GIVEN
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ON_CALL(m_objMockIAosNConfiguration, IsEmergencyReregSupportedOnIpcanChange())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED, _))
            .Times(0);

    // WHEN
    ImsMessage objMessage(MSG_IPCAN_CHANGED, 0, 0);
    m_pTestAosEApplication->ProcessMessage(objMessage);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosEApplicationTest, ShouldPostMessageForIpcanChangeIfConnectionNotifiesIpcanChanged)
{
    // GIVEN
    m_pTestAosEApplication->SetAppState(IAosApplication::STATE_CONNECTED);

    EXPECT_CALL(m_objMockThread, PostMessageI(IsSameMsg(MSG_IPCAN_CHANGED)));

    // WHEN
    m_pTestAosEApplication->ProcessConnectionUpdated(AosConnector::REASON_IPCAN_CAT_CHANGED);

    // THEN: The GIVEN condition should be met.
}
