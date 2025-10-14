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

#include "AosAppRequestType.h"
#include "AosReason.h"
#include "AString.h"
#include "AStringArray.h"
#include "IImsAosInfo.h"
#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "ImsMap.h"
#include "INetworkWatcher.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "TestTimerService.h"

#include "../../../config/interface/CarrierConfig.h"
#include "../../../config/interface/ImsServiceConfig.h"

#include "../../interface/aos/MockIAosService.h"
#include "../../interface/aos/MockIImsAosMonitor.h"

#include "app/MockAosAppContext.h"
#include "condition/MockAosCondition.h"
#include "connection/MockAosConnector.h"
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

#include "app/AosApplication.h"
#include "condition/AosCondition.h"
#include "connection/AosConnection.h"
#include "connection/AosConnector.h"
#include "handle/AosFeatureTag.h"
#include "interface/AosInternalMsgDef.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistrationControlListener.h"

#include "provider/AosProvider.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosUtil.h"

using ::testing::_;
using ::testing::An;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

const IMS_SINT32 SLOT_ID = 0;
AString PROFILE_ID = AString("test");

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
    RETRY_COUNT_REG_NONE = 0,
    RETRY_COUNT_REG_RECOVER
};

enum
{
    REQUEST_NONE = 0,
    REQUEST_STOP,
    REQUEST_DESTROY,
    REQUEST_RECOVER,
    REQUEST_PDN_DISCONNECT,
    REQUEST_RESET_CONNECTION_RECOVERY
};

enum
{
    ACCESS_NETWORK_TYPE_GERAN = 1,
    ACCESS_NETWORK_TYPE_UTRAN = 2,
    ACCESS_NETWORK_TYPE_EUTRAN = 3,
    ACCESS_NETWORK_TYPE_IWLAN = 5,
    ACCESS_NETWORK_TYPE_NGRAN = 6
};

enum
{
    PENDING_NONE = 0x0,
    PENDING_REG_RECOVERY_HELD = 0x1,
    PENDING_REG_STOP_HELD = 0x2,
    PENDING_APP_DESTROY_HELD = 0x4,
    PENDING_REG_RECONFIG_HELD = 0x8,
    PENDING_REG_AFTER_CSFB_COMPLETE = 0x10,
    PENDING_IPCAN_HELD = 0x20,
    PENDING_REG_UPDATE_HELD = 0x40
};

#define DECLARE_USING(Base)                                     \
    using Base::GetState;                                       \
    using Base::IsCrossSimConnected;                            \
    using Base::ClearOffReason;                                 \
    using Base::ClearPending;                                   \
    using Base::ClearWifiRegBlock;                              \
    using Base::GetNetworkTypeForImsRegState;                   \
    using Base::SetOffReason;                                   \
    using Base::SetImsCall;                                     \
    using Base::SetPublishState;                                \
    using Base::SetRegRecoveryHeld;                             \
    using Base::ResetBlock;                                     \
    using Base::NotifyDeregistered;                             \
    using Base::AddRatBlock;                                    \
    using Base::ClearRatBlocks;                                 \
    using Base::PerformRatBlockActions;                         \
    using Base::IsEmergency;                                    \
    using Base::IsStateMessage;                                 \
    using Base::IsNotReady;                                     \
    using Base::IsEqualOrLessState;                             \
    using Base::IsRegRecoveryHeld;                              \
    using Base::IsImsCall;                                      \
    using Base::IsPublished;                                    \
    using Base::IsAllDetached;                                  \
    using Base::IsTimerRunning;                                 \
    using Base::IsRegTypeNormal;                                \
    using Base::IsRegStateUpdatedByNrLteRatChange;              \
    using Base::IsPdnDisconnectRequired;                        \
    using Base::IsPdnDeactivationRequired;                      \
    using Base::IsPlmnBlockRequired;                            \
    using Base::IsBlockRat;                                     \
    using Base::CreateAosCondition;                             \
    using Base::CreateAosConnector;                             \
    using Base::CreateAosLocationStarter;                       \
    using Base::AddEventListener;                               \
    using Base::RemoveEventListener;                            \
    using Base::SetNetTrackerListener;                          \
    using Base::SetAppType;                                     \
    using Base::SetAppState;                                    \
    using Base::SetCleanState;                                  \
    using Base::IsUpdateAvailable;                              \
    using Base::IsReconfigHandleChanged;                        \
    using Base::IsRegisteredNetwork;                            \
    using Base::IsRequestCmdHeldByCondition;                    \
    using Base::IsAllHandleDetached;                            \
    using Base::IsConditionTimerSkippedDueToTimer;              \
    using Base::IsRegUpdatedByNrLteRatChange;                   \
    using Base::CleanAll;                                       \
    using Base::ClearConnection;                                \
    using Base::ClearConnector;                                 \
    using Base::GetReportState;                                 \
    using Base::OnMessage;                                      \
    using Base::ProcessMessage;                                 \
    using Base::PreprocessStateMessage;                         \
    using Base::PreprocessStateMessage_Connection;              \
    using Base::StateNotReady_Condition;                        \
    using Base::StateReady_Connection;                          \
    using Base::StateNotReady_Connection;                       \
    using Base::StateReady_Condition;                           \
    using Base::StateConnecting_Condition;                      \
    using Base::StateConnecting_Connection;                     \
    using Base::StateConnecting_Registration;                   \
    using Base::StateConnected_Condition;                       \
    using Base::StateConnected_Registration;                    \
    using Base::StateConnected_Connection;                      \
    using Base::StateUpdating_Condition;                        \
    using Base::StateUpdating_Connection;                       \
    using Base::ProcessConnectionUpdated;                       \
    using Base::StateUpdating_Registration;                     \
    using Base::StateDisconnecting_Condition;                   \
    using Base::StateDisconnecting_Registration;                \
    using Base::ProcessConnectionDeactivated;                   \
    using Base::ProcessDisconnectingState;                      \
    using Base::ProcessNetworkEvent;                            \
    using Base::ProcessRegControlEvent;                         \
    using Base::ProcessRegTerminated;                           \
    using Base::ProcessRegFailed_NoNextPcscfOnScscfRestoration; \
    using Base::ProcessRegFailed_Terminated;                    \
    using Base::StateDisconnecting_Connection;                  \
    using Base::ProcessRegInternalFailed;                       \
    using Base::ProcessPingCommand;                             \
    using Base::ProcessPdnDisconnect;                           \
    using Base::ProcessAppActivatedTimerExpired;                \
    using Base::ProcessAppConnectedTimerExpired;                \
    using Base::ProcessAppTerminatedTimerExpired;               \
    using Base::ProcessReconfigTimerExpired;                    \
    using Base::ProcessRoamingState;                            \
    using Base::ProcessRegBlockedTimerExpired;                  \
    using Base::ProcessRegFailed_StateConnected;                \
    using Base::ProcessRegStopTimerExpired;                     \
    using Base::ProcessPdnBlockedTimerExpired;                  \
    using Base::ProcessImsEstablishmentTimerExpired;            \
    using Base::ProcessPdnBlockWithTime;                        \
    using Base::ProcessImsEstablishmentControl;                 \
    using Base::ProcessImsEstablishmentStart;                   \
    using Base::ProcessPlmnBlock;                               \
    using Base::Report_Request;                                 \
    using Base::UpdateRegRecoveryHeld;                          \
    using Base::UpdateRegStopHeld;                              \
    using Base::StartTimer;                                     \
    using Base::StopTimer;                                      \
    using Base::ClearTimers;                                    \
    using Base::UpdateConnectedServices;                        \
    using Base::UpdateRegisteredRat;                            \
    using Base::UpdateMonitorNotify;                            \
    using Base::Init;                                           \
    using Base::CleanUp;                                        \
    using Base::Condition_Changed;                              \
    using Base::Condition_RequestCommand;                       \
    using Base::Connector_Activated;                            \
    using Base::Connector_Deactivated;                          \
    using Base::Connector_Updated;                              \
    using Base::Registration_StateChanged;                      \
    using Base::CallTracker_StateChanged;                       \
    using Base::NetTracker_StatusChanged;                       \
    using Base::NConfiguration_NotifyConfigChanged;             \
    using Base::Event_NotifyEvent;                              \
    using Base::Timer_TimerExpired;                             \
    using Base::RegistrationControl_ControlRegistration;        \
    using Base::ServicePhone_LocationInfoChanged;               \
    using Base::ProcessRegTerminating;                          \
    using Base::ProcessScscfRestoration;                        \
    using Base::GetImsEstablishmentTime;                        \
    using Base::RegistrationControl_UpdateDataFailureReason;    \
    using Base::GetDataFailureReason;                           \
    using Base::SetDataFailureReason;                           \
    using Base::ClearDataFailureReason;

class TestAosApplication : public AosApplication
{
public:
    DECLARE_USING(AosApplication)

    inline TestAosApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
            AosApplication(piAppContext, strAppId),
            m_bRegReconfigAvailable(IMS_TRUE)
    {
        m_pUtil = AosUtil::GetInstance();
    }

    inline IMS_BOOL IsRegReconfigAvailable() const override { return m_bRegReconfigAvailable; }

    inline void SetRegReconfigAvailable(IN IMS_BOOL bIsAvailable)
    {
        m_bRegReconfigAvailable = bIsAvailable;
    }

    inline void AddFeature(IN IMS_UINT32 nAdd) { m_pUtil->AddFeature(nAdd, m_nRegPending); }

    inline void RemoveFeature(IN IMS_UINT32 nRemove)
    {
        m_pUtil->RemoveFeature(nRemove, m_nRegPending);
    }

    inline IMS_BOOL IsFeatureOn(IN IMS_UINT32 nFeature) const
    {
        return m_pUtil->IsFeatureOn(nFeature, m_nRegPending);
    }

    inline ITimer* GetReconfigTimer() const { return m_piReconfigTimer; }
    inline ITimer* GetMsgConditionTimer() const { return m_piMsgConditionTimer; }
    inline ITimer* GetRegStopTimer() const { return m_piRegStopTimer; }
    inline ITimer* GetRegBlockedTimer() const { return m_piRegBlockedTimer; }
    inline ITimer* GetAppActivatedTimer() const { return m_piAppActivatedTimer; }
    inline ITimer* GetAppConnectedTimer() const { return m_piAppConnectedTimer; }
    inline ITimer* GetAppTerminatedTimer() const { return m_piAppTerminatedTimer; }
    inline ITimer* GetPdnBlockedTimer() const { return m_piPdnBlockedTimer; }
    inline ITimer* GetImsEstablishmentTimer() const { return m_piImsEstablishmentTimer; }

    inline void SetImsEstablishmentTimer(IN ITimer* piTimer)
    {
        m_piImsEstablishmentTimer = piTimer;
    }

    inline void SetRecoverReason(IN IMS_UINT32 nReason) { m_nRecoverReason = nReason; }
    inline void SetRat(IN IMS_UINT32 nRat) { m_nRat = nRat; }
    inline void SetLteAttachState(IN IMS_UINT32 nLteAttachState)
    {
        m_nLteAttachState = nLteAttachState;
    }
    inline void SetLteExtraInfo(IN IMS_UINT32 nLteExtraInfo) { m_nLteExtraInfo = nLteExtraInfo; }

    inline AosCondition* GetAosCondition() { return m_pCondition; }
    inline void SetAosCondition(IN AosCondition* pCondition) { m_pCondition = pCondition; }

    inline AosConnector* GetAosConnector() { return m_pConnector; }
    inline void SetAosConnector(IN AosConnector* pConnector) { m_pConnector = pConnector; }

    inline IAosCallTracker* GetCallTracker() { return m_piCallTracker; }
    inline void SetCallTracker(IN IAosCallTracker* piCallTracker)
    {
        m_piCallTracker = piCallTracker;
    }

    inline IAosRegistration* GetAosRegistration() { return m_piRegistration; }
    inline void SetAosRegistration(IN IAosRegistration* piRegistration)
    {
        m_piRegistration = piRegistration;
    }

    inline IAosNetTracker* GetNetTracker() { return m_piNetTracker; }
    inline void SetNetTracker(IN IAosNetTracker* piNetTracker) { m_piNetTracker = piNetTracker; }

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }

    inline void SetAppTypeEmergency() { m_nAppType = TYPE_EMERGENCY; }

    inline void SetEpdgEnabled(IN IMS_BOOL bEpdgEnabled) { m_bEpdgEnabled = bEpdgEnabled; }

    inline void SetPdnDeativationRequired(IN IMS_BOOL bPdnDeativationRequired)
    {
        m_bPdnDeactivationRequired = bPdnDeativationRequired;
    }
    inline void SetReportState(IN IMS_UINT32 nState) { m_nReportState = nState; }

private:
    IMS_BOOL m_bRegReconfigAvailable;
};

class AosApplicationTest : public ::testing::Test
{
public:
    TestAosApplication* m_pAosApplication;

    IAosNConfiguration* m_piAosNConfiguration;
    IAosService* m_piAosService;
    IAosCallTracker* m_piAosCallTracker;
    IAosLocationStarter* m_piAosLocationStarter;
    IAosRegStateManager* m_piAosRegStateManager;
    IAosRetryRepository* m_piAosRetryRepository;

    NiceMock<MockAosCondition> m_objMockAosCondition;
    NiceMock<MockAosConnector> m_objMockAosConnector;
    NiceMock<MockIAosAppContext> m_objMockIAosAppContext;
    NiceMock<MockIAosBlock> m_objMockIAosBlock;
    NiceMock<MockIAosCallTracker> m_objMockIAosCallTracker;
    NiceMock<MockIAosConnection> m_objMockIAosConnection;
    NiceMock<MockIAosHandle> m_objMockIAosHandle;
    NiceMock<MockIAosLocationStarter> m_objMockIAosLocationStarter;
    NiceMock<MockIAosNConfiguration> m_objMockIAosNConfiguration;
    NiceMock<MockIAosNetTracker> m_objMockIAosNetTracker;
    NiceMock<MockIAosPcscf> m_objMockIAosPcscf;
    NiceMock<MockIAosRegistration> m_objMockIAosRegistration;
    NiceMock<MockIAosRegStateManager> m_objMockIAosRegStateManager;
    NiceMock<MockIAosService> m_objMockIAosService;
    NiceMock<MockIAosRetryRepository> m_objMockIAosRetryRepository;
    NiceMock<MockIImsAosMonitor> m_objMockIImsAosMonitor;

    AString m_strAppId = AString("ims.app.test");
    AString m_strServiceId = AString("ims.service.test");
    ImsMap<AString, IAosHandle*> m_objHandles;
    AStringArray m_objPcscfs;
    ImsVector<IMS_SINT32> m_objRegTempPlmnBlockRats;

protected:
    void SetUp() override
    {
        ReplaceOriginWithMock();
        SetDefaultValues();

        // MockIAosAppContext
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(Return(&m_objMockIAosBlock));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetPcscf()).WillByDefault(Return(&m_objMockIAosPcscf));
        ON_CALL(m_objMockIAosAppContext, GetNetTracker())
                .WillByDefault(Return(&m_objMockIAosNetTracker));
        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&m_objMockIAosRegistration));
        ON_CALL(m_objMockIAosAppContext, GetHandles()).WillByDefault(ReturnRef(m_objHandles));

        // MockIAosNConfiguration
        ON_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
                .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_REPEATED));
        ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosNConfiguration, GetRegTempPlmnBlockRatsOnAllPcscfsFail())
                .WillByDefault(ReturnRef(m_objRegTempPlmnBlockRats));

        // MockIAosNetTracker
        ON_CALL(m_objMockIAosNetTracker, GetNetworkType())
                .WillByDefault(Return(NW_REPORT_RADIO_LTE));
        ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
                .WillByDefault(Return(NW_REPORT_RADIO_LTE));
        ON_CALL(m_objMockIAosNetTracker, IsRoaming()).WillByDefault(Return(IMS_FALSE));

        // MockIAosRegistration
        ON_CALL(m_objMockIAosRegistration, GetMode())
                .WillByDefault(Return(IAosRegistration::MODE_NORMAL));
        ON_CALL(m_objMockIAosRegistration, IsRefreshing()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_TRUE));

        // MockIAosConnection
        ON_CALL(m_objMockIAosConnection, GetConnectionType())
                .WillByDefault(Return(NetworkPolicy::APN_IMS));
        ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));

        // MockIAosHandle
        ON_CALL(m_objMockIAosHandle, GetAppId()).WillByDefault(ReturnRef(m_strAppId));
        ON_CALL(m_objMockIAosHandle, GetServiceId()).WillByDefault(ReturnRef(m_strServiceId));
        ON_CALL(m_objMockIAosHandle, GetMonitor()).WillByDefault(Return(&m_objMockIImsAosMonitor));

        // MockIAosPcscf
        ON_CALL(m_objMockIAosPcscf, GetPcscfs()).WillByDefault(ReturnRef(m_objPcscfs));
        ON_CALL(m_objMockIAosPcscf, GetNextPcscfIndex()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosPcscf, GetChangedType())
                .WillByDefault(Return(IAosPcscf::TYPE_CHANGED_SAME));

        // MockAosCondition
        ON_CALL(m_objMockAosCondition, IsReady()).WillByDefault(Return(IMS_FALSE));

        // Test subject
        m_pAosApplication = new TestAosApplication(&m_objMockIAosAppContext, PROFILE_ID);

        m_pAosApplication->SetAosCondition(&m_objMockAosCondition);
        m_pAosApplication->SetAosConnector(&m_objMockAosConnector);
        m_pAosApplication->SetAosRegistration(&m_objMockIAosRegistration);
    }

    void TearDown() override
    {
        ClearDefaultValues();
        RestoreOriginInstance();

        if (m_pAosApplication)
        {
            CleanUpAosApplication();
            delete m_pAosApplication;
            m_pAosApplication = IMS_NULL;
        }
    }

    void ReplaceOriginWithMock()
    {
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        m_piAosService = AosProvider::GetInstance()->GetService();
        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        m_piAosLocationStarter = AosProvider::GetInstance()->GetLocationStarter();
        m_piAosRetryRepository = AosProvider::GetInstance()->GetRetryRepository();
        m_piAosRegStateManager = AosProvider::GetInstance()->GetRegStateManager();

        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);
        AosProvider::GetInstance()->SetService(&m_objMockIAosService);
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker);
        AosProvider::GetInstance()->SetLocationStarter(&m_objMockIAosLocationStarter);
        AosProvider::GetInstance()->SetRetryRepository(&m_objMockIAosRetryRepository);
        AosProvider::GetInstance()->SetRegStateManager(&m_objMockIAosRegStateManager);
    }

    void RestoreOriginInstance()
    {
        AosProvider::GetInstance()->SetRegStateManager(m_piAosRegStateManager);
        AosProvider::GetInstance()->SetRetryRepository(m_piAosRetryRepository);
        AosProvider::GetInstance()->SetLocationStarter(m_piAosLocationStarter);
        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker);
        AosProvider::GetInstance()->SetService(m_piAosService);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }

    void SetDefaultValues()
    {
        m_objHandles.Add(m_strServiceId, &m_objMockIAosHandle);
        m_objPcscfs.AddElement(AString("192.168.0.100"));
    }

    void ClearDefaultValues()
    {
        m_objPcscfs.RemoveAllElements();
        m_objHandles.Clear();
    }

    void CleanUpAosApplication()
    {
        AosCondition* pCondition = m_pAosApplication->GetAosCondition();
        if (pCondition && pCondition != &m_objMockAosCondition)
        {
            delete pCondition;
        }

        AosConnector* pConnector = m_pAosApplication->GetAosConnector();
        if (pConnector && pConnector != &m_objMockAosConnector)
        {
            delete pConnector;
        }

        m_pAosApplication->ClearTimers();
        m_pAosApplication->StopTimer(TIMER_RECONFIG_GUARD);
        m_pAosApplication->StopTimer(TIMER_PDN_BLOCKED);
        m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
        m_pAosApplication->StopTimer(TIMER_RAT_BLOCK);
    }
};

TEST_F(AosApplicationTest, SucceedsCreateAosConditionWhenInit)
{
    // GIVEN
    m_pAosApplication->SetAosCondition(IMS_NULL);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(m_pAosApplication->GetAosCondition(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsCreateAosConnectorWhenInit)
{
    // GIVEN
    m_pAosApplication->SetAosConnector(IMS_NULL);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(m_pAosApplication->GetAosConnector(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsGetCallTrackerListenerWhenInit)
{
    // GIVEN
    m_pAosApplication->SetCallTracker(IMS_NULL);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(m_pAosApplication->GetCallTracker(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsGetRegistrationWhenInit)
{
    // GIVEN
    m_pAosApplication->SetAosRegistration(IMS_NULL);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(m_pAosApplication->GetAosRegistration(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsCreateLocationStarterWhenInit)
{
    // GIVEN
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(AosProvider::GetInstance()->GetLocationStarter(), nullptr);
}

TEST_F(AosApplicationTest, ShouldNotCreateLocationStarterWhenInitAndIsNotWfcImsAvailable)
{
    // GIVEN
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_EQ(AosProvider::GetInstance()->GetLocationStarter(), nullptr);
}

TEST_F(AosApplicationTest, ShouldNotCreateLocationStarterWhenInitAndIsNotSupportPidf)
{
    // GIVEN
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    ON_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_EQ(AosProvider::GetInstance()->GetLocationStarter(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsGetNetTrackerListenerWhenInit)
{
    // GIVEN
    m_pAosApplication->SetNetTracker(IMS_NULL);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(m_pAosApplication->GetNetTracker(), nullptr);
}

TEST_F(AosApplicationTest, ShouldNotSetNetTrackerListenerWhenInitAndRegTypeIsNotNormal)
{
    // GIVEN
    m_pAosApplication->SetNetTracker(IMS_NULL);
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_NE(m_pAosApplication->GetNetTracker(), nullptr);
}

TEST_F(AosApplicationTest, FailsCreateAndGetInstanceWhenInitWithoutConfiguration)
{
    // GIVEN
    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL);

    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetCallTracker(IMS_NULL);
    m_pAosApplication->SetAosRegistration(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    // WHEN
    m_pAosApplication->Init();

    // THEN
    EXPECT_EQ(m_pAosApplication->GetAosCondition(), nullptr);
    EXPECT_EQ(m_pAosApplication->GetAosConnector(), nullptr);
    EXPECT_EQ(m_pAosApplication->GetCallTracker(), nullptr);
    EXPECT_EQ(m_pAosApplication->GetAosRegistration(), nullptr);
    EXPECT_EQ(m_pAosApplication->GetNetTracker(), nullptr);
    EXPECT_EQ(AosProvider::GetInstance()->GetLocationStarter(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsSetListenerToConfigWhenInit)
{
    // GIVEN
    // It is also called once in AosCondition:Start()
    EXPECT_CALL(m_objMockIAosNConfiguration, SetListener(_)).Times(2);

    // WHEN
    m_pAosApplication->Init();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsSetListenerToRegistrationWhenInit)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosRegistration, SetListener(_));

    // WHEN
    m_pAosApplication->Init();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsSetListenerToCallTrackerWhenInit)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosCallTracker, SetListener(_)).Times(AtLeast(1));

    // WHEN
    m_pAosApplication->Init();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsAddListenerToAosServiceWhenInit)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosRegistrationControlListener*>()))
            .Times(AtLeast(1));
    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosServicePhoneListener*>()))
            .Times(AtLeast(1));

    // WHEN
    m_pAosApplication->Init();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsCreateAosConditionWhenEmergencyType)
{
    // GIVEN
    m_pAosApplication->SetAppTypeEmergency();
    m_pAosApplication->SetAosCondition(IMS_NULL);

    // WHEN
    m_pAosApplication->CreateAosCondition();

    // THEN
    EXPECT_NE(m_pAosApplication->GetAosCondition(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsStopTimersWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    m_pAosApplication->StartTimer(TIMER_RECONFIG_GUARD, 1000);
    m_pAosApplication->StartTimer(TIMER_PDN_BLOCKED, 1000);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, SucceedsRemoveListenerForNetTrackerWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();

    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    EXPECT_CALL(m_objMockIAosNetTracker, RemoveListener(_));

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsRemoveListenerForAosServiceWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosServicePhoneListener*>()));
    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosRegistrationControlListener*>()));

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ShouldNotRemoveListenerForAosServiceWhenEmergencyTypeCleanUp)
{
    // GIVEN
    m_pAosApplication->SetAppTypeEmergency();

    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosServicePhoneListener*>())).Times(0);
    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosRegistrationControlListener*>()))
            .Times(0);

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsDeleteLocationStarterWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);

    AosProvider::GetInstance()->SetLocationStarter(m_piAosLocationStarter);

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN
    EXPECT_EQ(AosProvider::GetInstance()->GetLocationStarter(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsRemoveListenerForCallTrackerWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetCallTracker(&m_objMockIAosCallTracker);

    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    EXPECT_CALL(m_objMockIAosCallTracker, RemoveListener(_));

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsClearConnectorWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->CreateAosConnector();

    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN
    EXPECT_EQ(m_pAosApplication->GetAosConnector(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsClearConditionWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->CreateAosCondition();

    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN
    EXPECT_EQ(m_pAosApplication->GetAosCondition(), nullptr);
}

TEST_F(AosApplicationTest, SucceedsSetListenerWithNullForRegistrationWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    EXPECT_CALL(m_objMockIAosRegistration, SetListener(IMS_NULL));

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, SucceedsRemoveListenerForConfigurationWhenCleanUp)
{
    // GIVEN
    m_pAosApplication->SetAosCondition(IMS_NULL);
    m_pAosApplication->SetAosConnector(IMS_NULL);
    m_pAosApplication->SetNetTracker(IMS_NULL);
    AosProvider::GetInstance()->SetLocationStarter(IMS_NULL);

    EXPECT_CALL(m_objMockIAosNConfiguration, RemoveListener(_));

    // WHEN
    m_pAosApplication->CleanUp();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ShouldSetServiceConnectingBlockWhenReconfigAndStateIsReady)
{
    // GIVEN
    m_pAosApplication->SetAppState(IAosApplication::STATE_READY);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_SERVICE_CONNECTING, IMS_TRUE));

    // WHEN
    m_pAosApplication->Reconfig();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ShouldSetServiceConnectingBlockWhenReconfigAndStateIsNotReady)
{
    // GIVEN
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_SERVICE_CONNECTING, IMS_TRUE));

    // WHEN
    m_pAosApplication->Reconfig();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ShouldNotSetServiceConnectingBlockWhenReconfigAndStateIsGreaterThanReady)
{
    // GIVEN
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_SERVICE_CONNECTING, IMS_TRUE)).Times(0);

    // WHEN
    m_pAosApplication->Reconfig();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ShouldStartReconfigGuardTimerWhenReconfig)
{
    // GIVEN
    // WHEN
    m_pAosApplication->Reconfig();

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
}

TEST_F(AosApplicationTest, GetAndSet)
{
    // TEST_F : GetActivityName
    m_pAosApplication->GetActivityName();

    // TEST_F : SetAppState, GetAppState
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pAosApplication->GetAppState(), IAosApplication::STATE_NOTREADY);

    // TEST_F : UpdateRegisteredRat, GetProperty
    IMS_UINT32 nValue = NW_REPORT_RADIO_INVALID;
    AString strValue;

    m_pAosApplication->UpdateRegisteredRat(NW_REPORT_RADIO_LTE);
    m_pAosApplication->GetProperty(1, nValue, strValue);
    EXPECT_EQ(nValue, NW_REPORT_RADIO_INVALID);

    m_pAosApplication->GetProperty(IAosApplication::PROPERTY_REGISTERED_RAT, nValue, strValue);
    EXPECT_EQ(nValue, NW_REPORT_RADIO_LTE);

    // TEST_F : SetOffReason, GetOffReason, ClearOffReason
    m_pAosApplication->SetOffReason(AosReason::AosReason::POWER_OFF);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::POWER_OFF);
    m_pAosApplication->ClearOffReason();
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::NONE);

    // TEST_F : IsActivated, SetActivation
    m_pAosApplication->SetActivation(IMS_TRUE);
    EXPECT_TRUE(m_pAosApplication->IsActivated());
    m_pAosApplication->SetActivation(IMS_FALSE);
    EXPECT_FALSE(m_pAosApplication->IsActivated());

    // TEST_F : SetAppState, IsOn
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->IsOn());
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_TRUE(m_pAosApplication->IsOn());
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_FALSE(m_pAosApplication->IsOn());

    // TEST_F : NotifyPublishState, IsPublished
    m_pAosApplication->NotifyPublishState(IMS_TRUE);
    EXPECT_TRUE(m_pAosApplication->IsPublished());
    m_pAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    m_pAosApplication->NotifyPublishState(IMS_FALSE);
    EXPECT_FALSE(m_pAosApplication->IsPublished());
    m_pAosApplication->ClearTimers();

    // TEST_F : SetRegRecoveryHeld, IsRegRecoveryHeld
    m_pAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pAosApplication->IsRegRecoveryHeld());
    m_pAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_FALSE(m_pAosApplication->IsRegRecoveryHeld());

    // TEST_F : IsAllDetached
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_ENABLER_DETACHED))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_TRUE(m_pAosApplication->IsAllDetached());
    EXPECT_FALSE(m_pAosApplication->IsAllDetached());

    // TEST_F : StartTimer, StopTimer
    m_pAosApplication->StartTimer(TIMER_REG_STOP, 0);
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
    m_pAosApplication->StartTimer(TIMER_INVALID, 0);
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_INVALID));
    m_pAosApplication->StopTimer(TIMER_INVALID);

    // TEST_F : ClearTimers, IsTimerRunning
    m_pAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    m_pAosApplication->StartTimer(TIMER_MSG_CONDITION, 1000);
    m_pAosApplication->StartTimer(TIMER_REG_BLOCKED, 1000);
    m_pAosApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    m_pAosApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    m_pAosApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    m_pAosApplication->StartTimer(TIMER_INVALID, 1000);
    m_pAosApplication->ClearTimers();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_MSG_CONDITION));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_INVALID));

    // TEST_F : IsRegTypeNormal, IsRegStateUpdatedByNrLteRatChange
    EXPECT_TRUE(m_pAosApplication->IsRegTypeNormal());
    EXPECT_TRUE(m_pAosApplication->IsRegStateUpdatedByNrLteRatChange());

    // TEST_F : IsNotReady, SetAppState, SetCleanState, IsUpdateAvailable, GetReportState
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_EQ(m_pAosApplication->GetReportState(), IAosApplication::APP_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->IsUpdateAvailable());

    EXPECT_FALSE(m_pAosApplication->IsNotReady());
    m_pAosApplication->SetCleanState();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTED);

    ON_CALL(m_objMockAosCondition, IsReady()).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pAosApplication->IsNotReady());
    m_pAosApplication->SetCleanState();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_READY);
    EXPECT_EQ(m_pAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTED);
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_EQ(m_pAosApplication->GetReportState(), IAosApplication::APP_UPDATING);
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTING);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_EQ(m_pAosApplication->GetReportState(), IAosApplication::APP_DISCONNECTED);
    EXPECT_TRUE(m_pAosApplication->IsNotReady());

    // TEST_F : IsStateMessage
    EXPECT_FALSE(m_pAosApplication->IsStateMessage(MSG_REG_START));
    EXPECT_TRUE(m_pAosApplication->IsStateMessage(MSG_REGISTRATION));

    // TEST_F : IsEmergency, SetAppType
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    EXPECT_TRUE(m_pAosApplication->IsEmergency());
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    // TEST_F : IsReconfigHandleChanged
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded())
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .WillOnce(Return(IAosHandle::DETACH))
            .WillOnce(Return(IAosHandle::DETACH))
            .WillOnce(Return(IAosHandle::DETACH))
            .WillRepeatedly(Return(IAosHandle::ATTACH));
    AosFeatureTagList objBindedList;
    AosFeatureTagList objList;
    objList.AddFeature(ImsAosFeature::MMTEL);
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList()).WillOnce(ReturnRef(objBindedList));
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList()).WillOnce(ReturnRef(objList));
    EXPECT_FALSE(m_pAosApplication->IsReconfigHandleChanged());
    EXPECT_TRUE(m_pAosApplication->IsReconfigHandleChanged());
    EXPECT_TRUE(m_pAosApplication->IsReconfigHandleChanged());
    EXPECT_TRUE(m_pAosApplication->IsReconfigHandleChanged());

    // TEST_F : IsRequestCmdHeldByCondition, SetImsCall, IsImsCall
    m_pAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_FALSE(
            m_pAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::POWER_OFF));
    EXPECT_TRUE(m_pAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::NONE));
    m_pAosApplication->ClearPending();
    m_pAosApplication->SetImsCall(IMS_FALSE);
    EXPECT_FALSE(m_pAosApplication->IsRequestCmdHeldByCondition(REQUEST_STOP, AosReason::NONE));

    // TEST_F : IsAllHandleDetached
    EXPECT_TRUE(m_pAosApplication->IsAllHandleDetached());
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_FALSE(m_pAosApplication->IsAllHandleDetached());

    // TEST_F : IsConditionTimerSkippedDueToTimer
    EXPECT_FALSE(m_pAosApplication->IsConditionTimerSkippedDueToTimer());

    // TEST_F : IsRegUpdatedByNrLteRatChange
    ImsVector<IMS_SINT32> objRegUpdateRats;

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUpdateRegistrationWithRatChange())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegUpdateRats));
    EXPECT_FALSE(m_pAosApplication->IsRegUpdatedByNrLteRatChange());
    objRegUpdateRats.Add(ACCESS_NETWORK_TYPE_EUTRAN);
    objRegUpdateRats.Add(ACCESS_NETWORK_TYPE_NGRAN);
    EXPECT_TRUE(m_pAosApplication->IsRegUpdatedByNrLteRatChange());
    objRegUpdateRats.Clear();

    // TEST_F : GetNetworkTypeForImsRegState
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);
    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::LTE);
    AosUtil::GetInstance()->SetWifiTest(bIsWifiTest);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_WLAN))
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_NR))
            .WillOnce(Return(NW_REPORT_RADIO_WCDMA))
            .WillOnce(Return(NW_REPORT_RADIO_HSPA))
            .WillOnce(Return(NW_REPORT_RADIO_EHRPD));

    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::IWLAN);
    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::LTE);
    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::NR);
    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::UTRAN);
    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::UTRAN);
    EXPECT_EQ(m_pAosApplication->GetNetworkTypeForImsRegState(), AosNetworkType::NONE);

    // TEST_F : IsRegReconfigAvailable
    EXPECT_TRUE(m_pAosApplication->IsRegReconfigAvailable());
}

TEST_F(AosApplicationTest, GetSetClearDataFailureReason)
{
    IMS_SINT32 anyDataFailureReason = 1;
    m_pAosApplication->SetDataFailureReason(anyDataFailureReason);
    EXPECT_EQ(m_pAosApplication->GetDataFailureReason(), anyDataFailureReason);
    m_pAosApplication->ClearDataFailureReason();
    EXPECT_EQ(m_pAosApplication->GetDataFailureReason(), 0);
}

TEST_F(AosApplicationTest, ResetWifiRegForbiddenWhenClearWifiRegBlockCalled)
{
    ON_CALL(m_objMockIAosNConfiguration, GetSubConsecutiveRetryCntForRegForbiddenInWifi())
            .WillByDefault(Return(3));

    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_WIFI_REG_FORBIDDEN, _));

    m_pAosApplication->ClearWifiRegBlock();
}

TEST_F(AosApplicationTest, GetCrossSimStatusFromConnectorWhenIsCrossSimConnectedCalled)
{
    ON_CALL(m_objMockAosConnector, IsCrossSimConnected()).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pAosApplication->IsCrossSimConnected());
}

TEST_F(AosApplicationTest, ShouldNotSetBlockEpsfbStartWhenEmergencyTypeAndStart)
{
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_EPS_FALLBACK_STARTED, IMS_FALSE)).Times(0);

    m_pAosApplication->NotifyEpsFallbackCallState(IImsAosInfo::EPSFB_CALL_START);
}

TEST_F(AosApplicationTest,
        ShouldSetBlockEpsfbStartAndRequestCloseTcpSocketCmdWhenNormalTypeAndStart)
{
    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_EPS_FALLBACK_STARTED, IMS_FALSE)).Times(1);
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_CLOSE_UNSECURE_TCP_SOCKET, 0));

    m_pAosApplication->NotifyEpsFallbackCallState(IImsAosInfo::EPSFB_CALL_START);
}

TEST_F(AosApplicationTest, ShouldResetBlockEpsfbStartWhenNormalTypeAndFail)
{
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_EPS_FALLBACK_STARTED, IMS_TRUE)).Times(1);

    m_pAosApplication->NotifyEpsFallbackCallState(IImsAosInfo::EPSFB_CALL_FAILED);
}

TEST_F(AosApplicationTest, IsPdnDisconnectRequired)
{
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // blocked with BLOCK_IMS_SERVICE_DISABLED
    EXPECT_TRUE(m_pAosApplication->IsPdnDisconnectRequired());

    // reason is IMS_DISABLED
    m_pAosApplication->SetOffReason(AosReason::IMS_DISABLED);
    EXPECT_TRUE(m_pAosApplication->IsPdnDisconnectRequired());

    // reason is NONE
    m_pAosApplication->SetOffReason(AosReason::NONE);
    EXPECT_FALSE(m_pAosApplication->IsPdnDisconnectRequired());
}

TEST_F(AosApplicationTest, IsPdnDisconnectRequiredShouldReturnFalseForEmergencyType)
{
    m_pAosApplication->SetAppTypeEmergency();

    // reason is DATA_PERMANENTLY_FAILED
    m_pAosApplication->SetOffReason(AosReason::DATA_PERMANENTLY_FAILED);
    EXPECT_FALSE(m_pAosApplication->IsPdnDisconnectRequired());
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

    EXPECT_FALSE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_START));

    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillOnce(Return(IAosRegistration::STATE_OFFLINE));

    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_START));

    // ImsAosControl::REGISTER_START_WITH_WLAN
    EXPECT_FALSE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_START_WITH_WLAN));
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosRegistration, GetState())
            .Times(AnyNumber())
            .WillOnce(Return(IAosRegistration::STATE_OFFLINE));
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_START_WITH_WLAN));

    // ImsAosControl::REGISTER_REFRESH
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_REFRESH));

    // ImsAosControl::REGISTER_STOP
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_STOP));

    // ImsAosControl::REGISTER_STOP_BY_ROAMING
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_STOP_BY_ROAMING));

    // ImsAosControl::REGISTER_REINITIATE
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_REINITIATE));

    // ImsAosControl::REGISTER_REINITIATE_BY_CSFB
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::REGISTER_REINITIATE_BY_CSFB));

    // ImsAosControl::PCSCF_NEXT
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::PCSCF_NEXT));

    // ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY));

    // ImsAosControl::IPSEC_DISABLED
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(AnyNumber());
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::IPSEC_DISABLED));

    // ImsAosControl::RETRY_COUNT_INCREASE
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::RETRY_COUNT_INCREASE));

    // ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION
    EXPECT_TRUE(m_pAosApplication->RequestCmd(
            ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    // ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION));

    // ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT));

    // etc
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION));
    EXPECT_TRUE(m_pAosApplication->RequestCmd(ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION));

    // unspecified CmdType
    IMS_UINT32 nUnspecifiedCmdType = -1;
    EXPECT_FALSE(m_pAosApplication->RequestCmd(nUnspecifiedCmdType));
}

TEST_F(AosApplicationTest, OnMessage)
{
    // non State Message
    ImsMessage objNotHandledMsg(MSG_INIT, 0, 0);
    EXPECT_FALSE(m_pAosApplication->OnMessage(objNotHandledMsg));

    // STATE_NOTREADY
    ImsMessage objConditionMsg(MSG_CONDITION, 0, 0);
    ImsMessage objConnectionMsg(MSG_CONNECTION, 0, 0);
    ImsMessage objRegistrationMsg(MSG_REGISTRATION, 0, 0);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConnectionMsg));

    // STATE_READY
    m_pAosApplication->SetAppState(IAosApplication::STATE_READY);
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConnectionMsg));

    // STATE_CONNECTING
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objRegistrationMsg));

    // STATE_CONNECTED
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objRegistrationMsg));

    // STATE_UPDATING
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objRegistrationMsg));

    // STATE_DISCONNECTING
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConditionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objConnectionMsg));
    EXPECT_TRUE(m_pAosApplication->OnMessage(objRegistrationMsg));
}

TEST_F(AosApplicationTest, ProcessMessage)
{
    // MSG_REG_START
    // TEST_F : ProcessRegStart
    // Report_StateChanged - Report_Notify, UpdateRegState, IsRegTypeNormal, IsOn
    ImsMessage objMessage(MSG_REG_START, 0, 0);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    ON_CALL(m_objMockAosCondition, PrintBlockReasons()).WillByDefault(Return());
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    // MSG_REG_UPDATE
    // TEST_F : ProcessRegUpdate
    objMessage.nMSG = MSG_REG_UPDATE;
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    // MSG_REG_RECONFIG
    // TEST_F : ProcessRegReconfig, ResetBlock
    objMessage.nMSG = MSG_REG_RECONFIG;

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_ENABLER_DETACHED))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    // IsAllDetached return true
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    // IsAllDetached return false
    EXPECT_CALL(m_objMockIAosRegistration, Reconfig()).Times(1);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    // IsRegReconfigAvailable return false
    m_pAosApplication->SetRegReconfigAvailable(IMS_FALSE);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pAosApplication->SetRegReconfigAvailable(IMS_TRUE);

    // MSG_REG_RECOVER
    // TEST_F : ProcessRegRecovery
    objMessage.nMSG = MSG_REG_RECOVER;
    EXPECT_CALL(m_objMockIAosRegistration, Start()).Times(2);
    m_pAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    objMessage.nWparam = AosRegRecoveryType::PCSCF_CHANGE;
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    objMessage.nWparam = AosRegRecoveryType::KEEP_DATA_CONNECTION;
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    objMessage.nWparam = 0;
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    // MSG_IPCAN_CHANGED
    // TEST_F : ProcessIpcanChanged
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(AnyNumber());

    objMessage.nMSG = MSG_IPCAN_CHANGED;
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_IPCAN_CHANGED, _))
            .Times(1);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->StartTimer(TIMER_RECONFIG_GUARD, 1000);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pAosApplication->ClearTimers();

    // MSG_PUB_TERMINATED
    // TEST_F : ProcessRegStopTimerExpired
    objMessage.nMSG = MSG_PUB_TERMINATED;
    EXPECT_CALL(m_objMockIAosRegistration, Stop()).Times(1);
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->ClearTimers();

    // MSG_DESTROY
    // TEST_F : ProcessDestroy
    objMessage.nMSG = MSG_DESTROY;
    m_pAosApplication->SetAppState(IAosApplication::STATE_READY);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    m_pAosApplication->ClearTimers();

    // MSG_REG_EXCHANGE
    // TEST_F : ProcessRegExchange
    objMessage.nMSG = MSG_REG_EXCHANGE;
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    // MSG_AC_CONFIGURED
    // TEST_F : ProcessAutoConfigurationComplete
    objMessage.nMSG = MSG_AC_CONFIGURED;
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    // MSG_PCSCF_RECOVER
    // TEST_F : ProcessPcscfRecovery
    objMessage.nMSG = MSG_PCSCF_RECOVER;
    m_pAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->ClearPending();
    m_pAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    m_objPcscfs.AddElement(AString("192.168.0.101"));
    EXPECT_CALL(m_objMockIAosPcscf, GetNextPcscfIndex()).WillOnce(Return(0)).WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosPcscf, UpdatePcscfs(_, _)).Times(2);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_objPcscfs.RemoveElement(AString("192.168.0.101"), IMS_FALSE);

    // MSG_SCSCF_RESTORATION
    // TEST_F : ProcessScscfRestoration
    objMessage.nMSG = MSG_SCSCF_RESTORATION;
    m_pAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->ClearPending();

    // MSG_PLMN_BLOCK_WITH_TIMEOUT
    // TEST_F : ProcessPlmnBlock
    m_pAosApplication->SetNetTrackerListener();
    objMessage.nMSG = MSG_PLMN_BLOCK_WITH_TIMEOUT;
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _))
            .Times(2);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // MSG_RETRY_COUNT_INCREASE
    // TEST_F : ProcessRegRetryCount
    objMessage.nMSG = MSG_RETRY_COUNT_INCREASE;
    EXPECT_CALL(m_objMockIAosNConfiguration, IsExtraRegErrRetryCntSharedForRegAndSubRequired())
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrMaxCount())
            .WillOnce(Return(0))
            .WillOnce(Return(1));
    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _, _, _)).Times(0);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    // MSG_OTHERS
    // TEST_F : ProcessOthers
    objMessage.nMSG = MSG_OTHERS;
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, IgnoreRegStopMsgIfAlreadyInDeregisteringState)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_DEREGISTERING));

    EXPECT_CALL(m_objMockIAosRegistration, Stop()).Times(0);

    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    m_pAosApplication->ProcessMessage(objMessage);

    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
}

TEST_F(AosApplicationTest, StopRegistrationWhenReceiveRegStopMsg)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));

    EXPECT_CALL(m_objMockIAosRegistration, Stop());

    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    m_pAosApplication->ProcessMessage(objMessage);

    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
}

TEST_F(AosApplicationTest, StartRegStopTimerIfPublishedWhenReceiveRegStopMsg)
{
    m_pAosApplication->SetPublishState(IMS_TRUE);
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));

    EXPECT_CALL(m_objMockIAosRegistration, Stop()).Times(0);

    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    m_pAosApplication->ProcessMessage(objMessage);

    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
}

TEST_F(AosApplicationTest, CleanAndStartIfNotInDisconnectingStateWhenReceiveRegStopMsg)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_READY);

    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    m_pAosApplication->ProcessMessage(objMessage);

    EXPECT_EQ(m_pAosApplication->GetAppState(), IAosApplication::STATE_NOTREADY);
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_MSG_CONDITION));
}

TEST_F(AosApplicationTest, NotifyDeregisteringWhenReceiveRegStopMsg)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistering(_));

    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    m_pAosApplication->ProcessMessage(objMessage);
}

TEST_F(AosApplicationTest, ShouldNotNotifyDeregisteringWhenReceiveRegStopMsgForEmergencyType)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistering(_)).Times(0);

    ImsMessage objMessage(MSG_REG_STOP, 0, 0);
    m_pAosApplication->ProcessMessage(objMessage);
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

    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_CALL(m_objMockIAosRetryRepository, IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL))
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    objMessage.nWparam = RETRY_COUNT_REG_RECOVER;
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    EXPECT_CALL(m_objMockIAosPcscf, HasNextPcscf())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillOnce(Return(CarrierConfig::Ims::ERROR_TYPE_REPEATED));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _))
            .Times(1);

    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));

    // HasNextPcscf: IMS_TRUE
    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, StateMachinePreProcess)
{
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    ImsMessage objMessageCnd(MSG_CONDITION, 0, 0);

    // TEST_F : PreprocessStateMessage - PreprocessStateMessage_Connection
    // PreprocessStateMessage_Connection - CONNECTION_ACTIVATED
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UPDATE_IPCAN, _))
            .Times(2);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    // support WFC
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnx));

    // PreprocessStateMessage_Connection - CONNECTION_UPDATED
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = AosConnector::REASON_IPCAN_CAT_CHANGED;
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnx));
    // support WFC
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnx));
    // not support WFC
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnx));

    // PreprocessStateMessage_Connection - CONNECTION_DEACTIVATED
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT, _))
            .Times(1);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnx));

    // TEST_F : PreprocessStateMessage_Condition
    // STATE_NOTREADY, AosBlock with invalid UICC, Emergency
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetState()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_NOTREADY, AosBlock with invalid UICC, Normal, IMS PDN not requested
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosConnection, GetState()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_NOTREADY, AosBlock with invalid UICC, IMS PDN not connected
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .WillOnce(Return(IAosConnection::STATE_ACTIVATING));
    EXPECT_CALL(m_objMockAosConnector, Stop()).Times(1);
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_NOTREADY, AosBlock with invalid UICC, IMS PDN connected
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetState()).WillOnce(Return(IAosConnection::STATE_ACTIVE));
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_READY, AosBlock not blocked with invalid UICC
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetAppState(IAosApplication::STATE_READY);

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnd));

    // STATE_CONNECTED, AosBlock with invalid UICC
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);

    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED)).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, IsActivationRequested()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(0);
    EXPECT_TRUE(m_pAosApplication->PreprocessStateMessage(objMessageCnd));
}

TEST_F(AosApplicationTest, DoNotClearRetryCountIfConfiguredToKeepWhenReceivedDeactivatedMessage)
{
    ON_CALL(m_objMockIAosNConfiguration, IsKeepRegRetryCntUponPdnReconnect())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_CLEAR_RETRY_COUNT, 0))
            .Times(0);

    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_DEACTIVATED, 0);
    m_pAosApplication->PreprocessStateMessage_Connection(objMessageCnx);
}

TEST_F(AosApplicationTest, StateMachine)
{
    ImsMessage objMessageCnx(MSG_CONNECTION, CONNECTION_ACTIVATED, 0);
    ImsMessage objMessageCnd(MSG_CONDITION, 0, 0);
    ImsMessage objMessageReg(MSG_REGISTRATION, IAosRegistration::RESULT_SUCCESS, 0);
    m_pAosApplication->SetNetTrackerListener();

    // TEST_F : StateNotReady_Condition
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    ON_CALL(m_objMockAosCondition, IsReady()).WillByDefault(Return(IMS_TRUE));

    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    // StateNotReady_Condition - TIMER_MSG_CONDITION is running
    m_pAosApplication->StartTimer(TIMER_MSG_CONDITION, 1000);
    EXPECT_TRUE(m_pAosApplication->StateNotReady_Condition(objMessageCnd));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // StateNotReady_Condition - TIMER_MSG_CONDITION is not running
    m_pAosApplication->ClearTimers();
    EXPECT_TRUE(m_pAosApplication->StateNotReady_Condition(objMessageCnd));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_READY);

    // TEST_F : StateNotReady_Connection
    EXPECT_TRUE(m_pAosApplication->StateNotReady_Connection(objMessageCnx));

    // TEST_F : StateReady_Condition
    ON_CALL(m_objMockAosCondition, IsReady()).WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(m_pAosApplication->StateReady_Condition(objMessageCnd));

    // TEST_F : StateReady_Connection
    // StateReady_Connection - CONNECTION_ACTIVATED
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    EXPECT_TRUE(m_pAosApplication->StateReady_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTING);

    // StateReady_Connection - CONNECTION_DEACTIVATED
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pAosApplication->StateReady_Connection(objMessageCnx));

    // StateReady_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    EXPECT_TRUE(m_pAosApplication->StateReady_Connection(objMessageCnx));

    // TEST_F : StateConnecting_Condition
    objMessageCnd.nWparam = CONNECTION_DEACTIVATED;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Condition(objMessageCnd));

    // StateConnecting_Connection - CONNECTION_DEACTIVATED - ProcessConnectionDeactivated
    // not REASON_PERMANENTLY_FAILED
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = AosConnector::REASON_NONE;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetAppState(), IAosApplication::STATE_NOTREADY);

    // StateConnecting_Connection - CONNECTION_UPDATED (ProcessConnectionUpdated)
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Connection(objMessageCnx));

    // StateConnecting_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Connection(objMessageCnx));

    // TEST_F : StateConnecting_Registration
    // StateConnecting_Registration - RESULT_SUCCESS (ProcessRegSucceeded)
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // StateConnecting_Registration - RESULT_TRYING (ProcessRegTrying_StateConnecting)
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));

    // StateConnecting_Registration - won't handled
    objMessageReg.nWparam = IAosRegistration::RESULT_NONE;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));

    // StateConnecting_Registration - RESULT_FAILURE (ProcessRegFailed_StateConnecting)
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    // RESULT_FAILURE - ProcessRegAuthenticationFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_FORBIDDEN;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessRegTerminated
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    m_pAosApplication->ClearTimers();
    // RESULT_FAILURE - ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessPdnBlock
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_BANNDED;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessPlmnBlock
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PCO_LIMITED_SERVICE;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessPlmnBlock & ProcessRegFailed_Start
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PLMN_BLOCK_WITH_TIMEOUT;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));
    // RESULT_FAILURE - ProcessRegFailed_Start
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_GENERAL;
    EXPECT_TRUE(m_pAosApplication->StateConnecting_Registration(objMessageReg));

    // TEST_F : StateConnected_Condition
    EXPECT_TRUE(m_pAosApplication->StateConnected_Condition(objMessageReg));

    // TEST_F : StateConnected_Connection
    // StateConnected_Connection - ProcessConnectionDeactivated
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));
    m_pAosApplication->ClearTimers();

    // StateConnected_Connection - ProcessConnectionUpdated
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    // StateConnected_Connection - ProcessConnectionUpdated - REASON_IP_CHANGED
    objMessageCnx.nLparam = AosConnector::REASON_IP_CHANGED;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));

    // StateConnected_Connection - ProcessConnectionUpdated - REASON_PCSCF_CHANGED
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIAosPcscf, GetChangedType())
            .WillOnce(Return(IAosPcscf::TYPE_CHANGED_SAME))
            .WillOnce(Return(IAosPcscf::TYPE_CHANGED_REORDER))
            .WillOnce(Return(IAosPcscf::TYPE_CHANGED_DIFFERENT))
            .WillOnce(Return(-1));
    objMessageCnx.nLparam = AosConnector::REASON_PCSCF_CHANGED;
    // REASON_PCSCF_CHANGED - ProcessConnectionUpdated_Pcscf - TYPE_CHANGED_SAME
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));
    // ProcessConnectionUpdated_Pcscf - TYPE_CHANGED_REORDER
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegistrationPcscfUpdatePolicy())
            .WillOnce(Return(CarrierConfig::Ims::REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME));
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));
    // REASON_PCSCF_CHANGED - ProcessConnectionUpdated_Pcscf - TYPE_CHANGED_DIFFERENT
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));
    // REASON_PCSCF_CHANGED - ProcessConnectionUpdated_Pcscf - invalid type
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));

    // StateConnected_Connection - ProcessConnectionUpdated - REASON_IPCAN_CAT_CHANGED
    objMessageCnx.nLparam = AosConnector::REASON_IPCAN_CAT_CHANGED;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));

    // StateConnected_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Connection(objMessageCnx));

    // TEST_F : StateConnected_Registration
    // StateConnected_Registration - ProcessRegTrying_StateConnected
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    // ProcessRegTrying_StateConnected - REASON_TRYING_START
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_START;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    // ProcessRegTrying_StateConnected - REASON_TRYING_UPDATE
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_UPDATE;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_UPDATING);
    // ProcessRegTrying_StateConnected - REASON_TRYING_STOP
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_STOP;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    // ProcessRegTrying_StateConnected - won't handled
    objMessageReg.nLparam = IAosRegistration::REASON_NONE;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // StateConnected_Registration - ProcessRegFailed_StateConnected
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    // ProcessRegFailed_StateConnected - won't handled
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessRegFailed_Terminated
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));
    // ProcessRegFailed_StateConnected - ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));

    // StateConnected_Registration - won't handled
    objMessageReg.nWparam = IAosRegistration::RESULT_NONE;
    EXPECT_TRUE(m_pAosApplication->StateConnected_Registration(objMessageReg));

    // TEST_F : StateUpdating_Condition
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Condition(objMessageCnd));

    // TEST_F : StateUpdating_Connection
    // StateUpdating_Connection - ProcessConnectionDeactivated
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateUpdating_Connection - ProcessConnectionUpdated
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = AosConnector::REASON_IP_CHANGED;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateUpdating_Connection - won't handled
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_UPDATING);

    // ProcessConnectionUpdated - invalid state
    m_pAosApplication->SetAppState(IAosApplication::STATE_READY);
    m_pAosApplication->ProcessConnectionUpdated(AosConnector::REASON_OTHERS);

    // TEST_F : StateUpdating_Registration
    // StateUpdating_Registration - won't handled
    objMessageReg.nWparam = IAosRegistration::RESULT_NONE;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));

    // StateUpdating_Registration - ProcessRegSucceeded
    objMessageReg.nWparam = IAosRegistration::RESULT_SUCCESS;
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType()).WillOnce(Return(IAosHandle::ATTACH));
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIImsAosMonitor, ImsAosMonitor_Connected(_, _)).Times(1);
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTED);

    // StateUpdating_Registration - ProcessRegTrying_StateUpdating - REASON_TRYING_START
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_START;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    // StateUpdating_Registration - ProcessRegTrying_StateUpdating - REASON_TRYING_STOP
    objMessageReg.nLparam = IAosRegistration::REASON_TRYING_STOP;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    // StateUpdating_Registration - ProcessRegTrying_StateUpdating - won't handled
    objMessageReg.nLparam = IAosRegistration::REASON_NONE;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    // StateUpdating_Registration - ProcessRegFailed_StateUpdating
    // ProcessRegFailed_StateUpdating - ProcessRegFailed_Update
    objMessageReg.nWparam = IAosRegistration::RESULT_FAILURE;
    objMessageReg.nLparam = 0;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pAosApplication->SetImsCall(IMS_TRUE);
    m_pAosApplication->SetOffReason(AosReason::NONE);
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::NONE);
    m_pAosApplication->SetImsCall(IMS_FALSE);

    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_CALL(m_objMockIAosRegistration, IsRefreshing()).WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessRegAuthenticationFailed
    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_AUTHENTICATION_FAILED, IMS_TRUE)).Times(1);
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_FORBIDDEN;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // ProcessRegFailed_StateUpdating - ProcessRegFailed_Terminated
    m_pAosApplication->SetAppState(IAosApplication::STATE_UPDATING);
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_TERMINATED;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // ProcessRegFailed_StateUpdating - ProcessRegInternalFailed
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_INTERNAL;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::REG_FAILURE);
    // ProcessRegFailed_StateUpdating - ProcessRegTerminating
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_REG_TERMINATING;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessPdnDisconnect
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessPdnBlockWithTime
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_PDN_RECONNECT_WITH_AWT;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));
    // ProcessRegFailed_StateUpdating - ProcessPdnBlock
    objMessageReg.nLparam = IAosRegistration::REASON_FAILURE_BANNDED;
    EXPECT_TRUE(m_pAosApplication->StateUpdating_Registration(objMessageReg));

    // TEST_F : StateDisconnecting_Condition
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Condition(objMessageCnd));

    // TEST_F : StateDisconnecting_Connection
    // StateDisconnecting_Connection - ProcessConnectionDeactivated
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    m_pAosApplication->SetOffReason(AosReason::REG_FAILURE);
    objMessageCnx.nWparam = CONNECTION_DEACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateDisconnecting_Connection - ProcessConnectionUpdated_StateDisconnecting
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    objMessageCnx.nWparam = CONNECTION_UPDATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    objMessageCnx.nLparam = AosConnector::REASON_IP_CHANGED;
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Connection(objMessageCnx));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // StateDisconnecting_Connection - won't handled
    objMessageCnx.nWparam = CONNECTION_ACTIVATED;
    objMessageCnx.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Connection(objMessageCnx));

    // TEST_F : StateDisconnecting_Registration
    // StateDisconnecting_Registration - RESULT_TRYING
    m_pAosApplication->SetAppState(IAosApplication::STATE_DISCONNECTING);
    objMessageReg.nWparam = IAosRegistration::RESULT_TRYING;
    objMessageReg.nLparam = 0;
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    // StateDisconnecting_Registration - RESULT_SUCCESS
    objMessageReg.nWparam = IAosRegistration::RESULT_SUCCESS;
    EXPECT_TRUE(m_pAosApplication->StateDisconnecting_Registration(objMessageReg));
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosApplicationTest, SetBlockPermanentDataFailedWhenStateReadyConnection)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_PERMANENT_DATA_FAILED, IMS_TRUE));

    ImsMessage objMessage(
            MSG_CONNECTION, CONNECTION_DEACTIVATED, AosConnector::REASON_PERMANENTLY_FAILED);

    // WHEN
    m_pAosApplication->StateReady_Connection(objMessage);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosApplicationTest, SetBlockPermanentDataFailedWhenStateConnectingConnection)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_PERMANENT_DATA_FAILED, IMS_FALSE));

    ImsMessage objMessage(
            MSG_CONNECTION, CONNECTION_DEACTIVATED, AosConnector::REASON_PERMANENTLY_FAILED);

    // WHEN
    m_pAosApplication->StateConnecting_Connection(objMessage);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosApplicationTest, SetBlockInvalidPcscfWhenStateReadyConnection)
{
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_INVALID_CONNECTION, IMS_TRUE));
    EXPECT_CALL(m_objMockAosConnector, Stop());

    ImsMessage objMessage(
            MSG_CONNECTION, CONNECTION_DEACTIVATED, AosConnector::REASON_PCSCF_DISCOVERY_FAILED);
    m_pAosApplication->StateReady_Connection(objMessage);
}

TEST_F(AosApplicationTest, SetBlockWifiRegForbiddenWhenStateConnectedWithRegFailureForbiddenInWifi)
{
    ON_CALL(m_objMockIAosNConfiguration, GetSubConsecutiveRetryCntForRegForbiddenInWifi())
            .WillByDefault(Return(3));

    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_WIFI_REG_FORBIDDEN, IMS_TRUE));

    m_pAosApplication->ProcessRegFailed_StateConnected(
            IAosRegistration::REASON_FAILURE_FORBIDDEN_IN_WIFI);
}

TEST_F(AosApplicationTest, SetIpChangedReasonWhenProcessConnectionUpdated)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->ProcessConnectionUpdated(AosConnector::REASON_IP_CHANGED);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::IP_CHANGED);
}

TEST_F(AosApplicationTest,
        SetWifiOffReasonWhenProcessConnectionDeactivatedAfterWifiOffWhenIsImsCall)
{
    m_pAosApplication->SetEpdgEnabled(IMS_TRUE);
    m_pAosApplication->SetImsCall(IMS_TRUE);

    m_pAosApplication->Condition_RequestCommand(
            AosCondition::REQUEST_REASON_UPDATE, AosReason::WIFI_OFF);
    m_pAosApplication->ProcessConnectionDeactivated(AosConnector::REASON_NONE);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::WIFI_OFF);
}

TEST_F(AosApplicationTest, ShouldNotSetWifiOffReasonWhenProcessConnectionDeactivatedAfterWifiOff)
{
    m_pAosApplication->SetEpdgEnabled(IMS_TRUE);
    m_pAosApplication->SetImsCall(IMS_FALSE);

    m_pAosApplication->Condition_RequestCommand(
            AosCondition::REQUEST_REASON_UPDATE, AosReason::WIFI_OFF);
    m_pAosApplication->ProcessConnectionDeactivated(AosConnector::REASON_NONE);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
}

TEST_F(AosApplicationTest,
        ShouldNotSetWifiOffReasonWhenProcessConnectionDeactivatedAfterWifiOffInCellular)
{
    m_pAosApplication->SetEpdgEnabled(IMS_FALSE);

    m_pAosApplication->Condition_RequestCommand(
            AosCondition::REQUEST_REASON_UPDATE, AosReason::WIFI_OFF);
    m_pAosApplication->ProcessConnectionDeactivated(AosConnector::REASON_NONE);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
}

TEST_F(AosApplicationTest, SetIpChangedReasonWhenProcessConnectionDeactivated)
{
    m_pAosApplication->ProcessConnectionDeactivated(AosConnector::REASON_IP_CHANGED);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::IP_CHANGED);
}

TEST_F(AosApplicationTest, SetDataDisconnectedReasonWhenProcessConnectionDeactivated)
{
    m_pAosApplication->ProcessConnectionDeactivated(AosConnector::REASON_NONE);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
}

TEST_F(AosApplicationTest, ResetBlockInvalidPcscfWhenNetStatusChanged)
{
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetRat(NW_REPORT_RADIO_INVALID);
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_CELLULAR_RAT_BLOCK))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_INVALID_CONNECTION))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_INVALID_CONNECTION, _));

    m_pAosApplication->NetTracker_StatusChanged();
}

TEST_F(AosApplicationTest, Process)
{
    // TEST_F : ProcessDisconnectingState
    m_pAosApplication->ProcessDisconnectingState(AosReason::NONE);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessNetworkEvent
    m_pAosApplication->ProcessNetworkEvent(
            IMS_EVENT_LTE_INFO, IMS_LTE_INFO_EPS_ONLY_ATTACHED, IMS_LTE_INFO_EXTRA_NONE);
    m_pAosApplication->ProcessNetworkEvent(
            IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED, IMS_LTE_INFO_EXTRA_NONE);
    m_pAosApplication->ProcessNetworkEvent(IMS_EVENT_VOICE_SERVICE_STATE, 0, 0);

    // TEST_F : ProcessRegControlEvent
    // ProcessRegControlEvent - won't handled
    m_pAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_IPCAN, 0);
    // ProcessRegControlEvent - IMS_REG_CONTROL_RECOVER
    m_pAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_RECOVER, 0);
    m_pAosApplication->ProcessRegControlEvent(
            IMS_REG_CONTROL_RECOVER, IMS_REG_CONTROL_KEEP_DATA_CONNECTION);
    // ProcessRegControlEvent - IMS_REG_CONTROL_UPDATE
    m_pAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_UPDATE, 0);
    // ProcessRegControlEvent - IMS_REG_CONTROL_DESTROY
    m_pAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_DESTROY, 0);
    // ProcessRegControlEvent - IMS_REG_CONTROL_STOP
    m_pAosApplication->ProcessRegControlEvent(IMS_REG_CONTROL_STOP, 0);

    // TEST_F : ProcessRegTerminated
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->ProcessRegTerminated();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    m_pAosApplication->ClearTimers();

    // TEST_F : ProcessRegFailed_Terminated
    m_pAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    m_pAosApplication->ProcessRegFailed_Terminated();
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::REG_TERMINATED);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
    m_pAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    m_pAosApplication->ProcessRegFailed_Terminated();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessRegInternalFailed
    m_pAosApplication->ProcessRegInternalFailed();
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::REG_FAILURE);
    m_pAosApplication->ProcessRegInternalFailed(AosReason::REG_TERMINATED);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::REG_TERMINATED);
    m_pAosApplication->ClearTimers();

    // TEST_F : ProcessPingCommand
    m_pAosApplication->ProcessPingCommand();

    // TEST_F : ProcessAppActivatedTimerExpired
    m_pAosApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    m_pAosApplication->ProcessAppActivatedTimerExpired();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_MSG_CONDITION));
    m_pAosApplication->ClearTimers();

    // TEST_F : ProcessAppConnectedTimerExpired
    m_pAosApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    m_pAosApplication->ProcessAppConnectedTimerExpired();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));

    // TEST_F : ProcessAppTerminatedTimerExpired
    m_pAosApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));
    m_pAosApplication->ProcessAppTerminatedTimerExpired();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));

    // TEST_F : ProcessReconfigTimerExpired
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_ENABLER_DETACHED))
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosHandle, GetRequestType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosHandle::ATTACH));

    // AllHandleDetached - AllDetached
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pAosApplication->ProcessReconfigTimerExpired();
    // AllHandleDetached - not AllDetached - Registered
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pAosApplication->ProcessReconfigTimerExpired();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    // AllHandleDetached - not AllDetached - not Registered
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pAosApplication->ProcessReconfigTimerExpired();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
    // Not AllHandleDetached - NotReady state
    EXPECT_CALL(m_objMockIAosHandle, IsRegFeatureTagRequired()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_SERVICE_CONNECTING, _)).Times(1);
    m_pAosApplication->ProcessReconfigTimerExpired();
    // Not AllHandleDetached - Connecting state - PENDING_REG_RECOVERY_HELD feature on
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    m_pAosApplication->AddFeature(PENDING_REG_RECOVERY_HELD);
    m_pAosApplication->ProcessReconfigTimerExpired();
    m_pAosApplication->RemoveFeature(PENDING_REG_RECOVERY_HELD);
    // Not AllHandleDetached - Connecting state - IsReconfigHandleChanged return true
    EXPECT_CALL(m_objMockIAosHandle, IsRegBinded())
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));
    m_pAosApplication->ProcessReconfigTimerExpired();
    // Not AllHandleDetached - Connecting state - IsReconfigHandleChanged return false
    // IsOn return false - No feature on
    AosFeatureTagList objBindedList;
    AosFeatureTagList objList;
    objBindedList.AddFeature(ImsAosFeature::MMTEL);
    objList.AddFeature(ImsAosFeature::MMTEL);
    EXPECT_CALL(m_objMockIAosHandle, GetBindedFeatureTagList())
            .WillRepeatedly(ReturnRef(objBindedList));
    EXPECT_CALL(m_objMockIAosHandle, GetFeatureTagList()).WillRepeatedly(ReturnRef(objList));
    m_pAosApplication->ProcessReconfigTimerExpired();
    // IsOn return true - PENDING_IPCAN_HELD, PENDING_IPCAN_HELD on
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->AddFeature(PENDING_IPCAN_HELD);
    m_pAosApplication->AddFeature(PENDING_REG_UPDATE_HELD);
    m_pAosApplication->ProcessReconfigTimerExpired();
    m_pAosApplication->AddFeature(PENDING_IPCAN_HELD);
    m_pAosApplication->RemoveFeature(PENDING_REG_UPDATE_HELD);

    // TEST_F : ProcessRoamingState
    m_pAosApplication->ProcessRoamingState(IMS_FALSE);  // No change
    m_pAosApplication->ProcessRoamingState(IMS_TRUE);   // No timer
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillOnce(Return(0))
            .WillOnce(Return(120));
    TestTimerService objTestTimerService;
    PlatformService* pTimerService =
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_TIMER);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 123456);
    ITimer* piTimer = m_pAosApplication->GetImsEstablishmentTimer();
    PlatformContext::GetInstance()->SetService(
            PlatformContext::SERVICE_TIMER, &objTestTimerService);
    EXPECT_CALL(objTestTimerService.GetMockTimer(), SetTimer(120000, m_pAosApplication)).Times(1);
    m_pAosApplication->ProcessRoamingState(IMS_FALSE);
    m_pAosApplication->ProcessRoamingState(IMS_TRUE);
    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
    m_pAosApplication->SetImsEstablishmentTimer(piTimer);
    m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);

    // TEST_F : ProcessRegBlockedTimerExpired
    m_pAosApplication->StartTimer(TIMER_REG_BLOCKED, 1000);
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    m_pAosApplication->ProcessRegBlockedTimerExpired();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessRegStopTimerExpired
    m_pAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTING);
    m_pAosApplication->ProcessRegStopTimerExpired();
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);

    // TEST_F : ProcessPdnBlockedTimerExpired
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED, _)).Times(1);
    m_pAosApplication->StartTimer(TIMER_PDN_BLOCKED, 1000);
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    m_pAosApplication->ProcessPdnBlockedTimerExpired();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));

    // TEST_F : ProcessImsEstablishmentTimerExpired
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
    m_pAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(0);
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
    m_pAosApplication->SetImsCall(IMS_FALSE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pAosApplication->SetLteExtraInfo(IMS_LTE_INFO_EXTRA_NONE);
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::NR,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .WillOnce(Return(IAosConnection::STATE_IDLE))
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
    EXPECT_CALL(m_objMockIAosRegistration, IsRegistered())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
    EXPECT_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_TRAFFIC_PRIORITY_BLOCK, _, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(AosProperty::AOS_FALSE), Return(0)));
    EXPECT_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_REG_FAILURE_COUNT, _, _))
            .WillOnce(DoAll(SetArgReferee<1>(60), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(0), Return(0)));
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();

    // TEST_F : ProcessPdnBlockWithTime
    EXPECT_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_PDN_REACIVATE_WAIT_TIME, _, _))
            .WillOnce(DoAll(SetArgReferee<1>(0), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(60), Return(0)));
    m_pAosApplication->ProcessPdnBlockWithTime();
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    m_pAosApplication->ProcessPdnBlockWithTime();
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    m_pAosApplication->StopTimer(TIMER_PDN_BLOCKED);

    // TEST_F : ProcessPlmnBlock
    // reg type not normal
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pAosApplication->ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    // epdn enabled
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));
    m_pAosApplication->ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
    // IsPlmnBlockRequired false
    m_pAosApplication->SetNetTrackerListener();
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pAosApplication->SetLteExtraInfo(IMS_LTE_INFO_EXTRA_NONE);
    m_pAosApplication->ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
    // est timer running
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::NR,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _))
            .Times(1);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    m_pAosApplication->ProcessPlmnBlock(AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT);
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // TEST_F : UpdateRegRecoveryHeld
    m_pAosApplication->SetImsCall(IMS_TRUE);
    EXPECT_FALSE(m_pAosApplication->UpdateRegRecoveryHeld());
    EXPECT_TRUE(m_pAosApplication->IsRegRecoveryHeld());
    // pending feature on, held
    m_pAosApplication->AddFeature(PENDING_REG_RECOVERY_HELD);
    m_pAosApplication->SetRegRecoveryHeld(IMS_FALSE);
    EXPECT_FALSE(m_pAosApplication->UpdateRegRecoveryHeld());
    EXPECT_TRUE(m_pAosApplication->IsRegRecoveryHeld());
    // pending feature on, not held - recover reason PCSCF_CHANGE
    m_pAosApplication->SetImsCall(IMS_FALSE);
    m_pAosApplication->SetRecoverReason(AosRegRecoveryType::PCSCF_CHANGE);
    EXPECT_TRUE(m_pAosApplication->UpdateRegRecoveryHeld());
    EXPECT_FALSE(m_pAosApplication->IsRegRecoveryHeld());
    EXPECT_FALSE(m_pAosApplication->IsFeatureOn(PENDING_REG_RECOVERY_HELD));
    // pending feature on, not held - recover reason other
    m_pAosApplication->SetRegRecoveryHeld(IMS_TRUE);
    m_pAosApplication->AddFeature(PENDING_REG_RECOVERY_HELD);
    m_pAosApplication->SetRecoverReason(AosRegRecoveryType::UNKNOWN);
    EXPECT_TRUE(m_pAosApplication->UpdateRegRecoveryHeld());
    EXPECT_FALSE(m_pAosApplication->IsRegRecoveryHeld());
    EXPECT_FALSE(m_pAosApplication->IsFeatureOn(PENDING_REG_RECOVERY_HELD));

    // TEST_F : UpdateRegStopHeld()
    EXPECT_FALSE(m_pAosApplication->UpdateRegStopHeld());

    // TEST_F : UpdateMonitorNotify()
    EXPECT_CALL(m_objMockIImsAosMonitor, ImsAosMonitor_Notify(0, 0)).Times(1);
    m_pAosApplication->UpdateMonitorNotify(0, 0);
}

TEST_F(AosApplicationTest, ProcessImsEstTimerExpiredShouldNotInvokeNotifyDeregisteredWhenIsBlock)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetImsCall(IMS_FALSE);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _, _, _)).Times(0);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest,
        ProcessImsEstTimerExpiredShouldNotInvokeNotifyDeregisteredWhenIsTrafficPriorityBlock)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetImsCall(IMS_FALSE);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_TRAFFIC_PRIORITY_BLOCK, _, _))
            .WillOnce(DoAll(SetArgReferee<1>(AosProperty::AOS_TRUE), Return(0)));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _, _, _)).Times(0);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentTimerExpired();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, NotifyAppWithRegAllPcscfFailedReasonIfNoNextPcscfOnScscfRestoration)
{
    // GIVEN
    m_pAosApplication->SetReportState(IAosApplication::APP_CONNECTED);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosHandle,
            App_StateChanged(IAosApplication::APP_DISCONNECTED, AosReason::REG_ALL_PCSCF_FAILED));
    EXPECT_CALL(m_objMockIAosHandle, App_Notify());

    // WHEN
    m_pAosApplication->ProcessRegFailed_NoNextPcscfOnScscfRestoration();

    // THEN: The GIVEN and below conditions should be met.
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
}

TEST_F(AosApplicationTest, RequestUpdateStopRetryTimerWhenImsEstTimerIsExpired)
{
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetImsCall(IMS_FALSE);
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(_)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_TRAFFIC_PRIORITY_BLOCK, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(AosProperty::AOS_FALSE), Return(0)));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_NR));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_REG_FAILURE_COUNT, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(2), Return(0)));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsUpdateOngoingRegRetryTimerOnImsEstTimerExpiry())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::NR,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_STOP_RETRY_TIMER_WITH_DEFAULT, _));

    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
}

TEST_F(AosApplicationTest,
        NotifyTempPlmnBlockAndDisconnectPdnWithDelayWhenProcessImsEstTimerExpired)
{
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    ON_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_TRAFFIC_PRIORITY_BLOCK, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(AosProperty::AOS_FALSE), Return(0)));
    ON_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_REG_FAILURE_COUNT, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(0), Return(0)));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetReleasePdnDelaySecAfterTempPlmnBlock())
            .WillByDefault(Return(5));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));
    EXPECT_CALL(m_objMockAosConnector, Stop(5));

    m_pAosApplication->ProcessImsEstablishmentTimerExpired();
}

TEST_F(AosApplicationTest, RegTerminating)
{
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->SetImsCall(IMS_TRUE);

    m_pAosApplication->ProcessRegTerminating();
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::REG_TERMINATING);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTING);
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotBlockPlmnWhenImsServiceDisabled)
{
    // GIVEN
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED))
            .WillOnce(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated()).Times(0);
    EXPECT_CALL(m_objMockAosConnector, Stop());

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotBlockPlmnWhenPowerOff)
{
    // GIVEN
    m_pAosApplication->SetOffReason(AosReason::POWER_OFF);

    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated()).Times(0);
    EXPECT_CALL(m_objMockAosConnector, Stop());

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldSetStateWithNotReadyWhenIsImsCall)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosApplication->SetImsCall(IMS_TRUE);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_NOTREADY);
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldSetOffReasonWithRegTerminationWhenIsImsCall)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosApplication->SetImsCall(IMS_TRUE);

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::REG_TERMINATING);
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotSetOffReasonWhenIsNotImsCall)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsCallEndAndPdnReactivationByRegTerminated())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosApplication->SetImsCall(IMS_FALSE);

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN
    EXPECT_NE(m_pAosApplication->GetOffReason(), AosReason::REG_TERMINATING);
}

TEST_F(AosApplicationTest,
        ProcessPdnDisconnectShouldStopConnectorWhenOffReasonIsDataPermanentlyFailed)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetOffReason(AosReason::DATA_PERMANENTLY_FAILED);

    EXPECT_CALL(m_objMockAosConnector,
            Stop(TestAosApplication::PLMN_BLOCK_PDN_STOP_WAITING_TIME_SECONDS));

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldStartRatBlockTimerWhenTypeRatBlock)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK));

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_RAT_BLOCK));
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldAddRatBlockWhenTypeRatBlock)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK));

    m_pAosApplication->SetRat(NW_REPORT_RADIO_LTE);

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsBlockRat(NW_REPORT_RADIO_LTE));
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotifyDeregisteredWhenTypeRatBlock)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, _, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest,
        NotifyTempPlmnBlockAndDisconnectPdnWithDelayWhenProcessPdnDisconnectWithNr)
{
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetRat(NW_REPORT_RADIO_NR);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(
                    Return(CarrierConfig::Ims::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));
    ON_CALL(m_objMockIAosNConfiguration, GetReleasePdnDelaySecAfterTempPlmnBlock())
            .WillByDefault(Return(5));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));
    EXPECT_CALL(m_objMockAosConnector, Stop(5));

    m_pAosApplication->ProcessPdnDisconnect();
}

TEST_F(AosApplicationTest,
        NotifyTempPlmnBlockAndDisconnectPdnWithDelayWhenProcessPdnDisconnectWithEpsOnly)
{
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_EPS_ONLY_ATTACHED);
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(
                    Return(CarrierConfig::Ims::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));
    ON_CALL(m_objMockIAosNConfiguration, GetReleasePdnDelaySecAfterTempPlmnBlock())
            .WillByDefault(Return(0));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));
    EXPECT_CALL(m_objMockAosConnector, Stop(0));

    m_pAosApplication->ProcessPdnDisconnect();
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldStopConnectorWhenCombinedAttach)
{
    // GIVEN
    m_pAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);

    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(
                    Return(CarrierConfig::Ims::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));

    EXPECT_CALL(m_objMockAosConnector, Stop());

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotifyDeregisteredWhenLteInfoExtraIsNotNone)
{
    // GIVEN
    m_pAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pAosApplication->SetLteExtraInfo(IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED);

    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(
                    Return(CarrierConfig::Ims::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotStopWhenTestModeEnabledWithoutDeactivation)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_CRITICAL));

    ON_CALL(m_objMockIAosNConfiguration, IsTestModeEnabled(_)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockAosConnector, Stop()).Times(0);

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotNotifyDeregisterWhenIsNotPlmnBlockConfig)
{
    // GIVEN
    m_pAosApplication->SetRat(NW_REPORT_RADIO_INVALID);

    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED));

    EXPECT_CALL(m_objMockIAosService, NotifyDeregistered(_, _, _, _)).Times(0);

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest, ProcessPdnDisconnectShouldNotifyDeregisterWhenNrBlockCondition)
{
    // GIVEN
    m_objRegTempPlmnBlockRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
    m_pAosApplication->SetRat(NW_REPORT_RADIO_NR);

    ON_CALL(m_objMockIAosNConfiguration, GetExtraRegErrFinalType())
            .WillByDefault(Return(CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED));

    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(_, _, AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT, _));

    // WHEN
    m_pAosApplication->ProcessPdnDisconnect();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerWhenImsEstablishmentStartIfRegTypeIsEmergency)
{
    // GIVEN
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenProcessImsEstablishmentStartIfEstTimeIsZero)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte()).WillByDefault(Return(0));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfEpdgEnabled)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfImsRegistered)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfImsCallActive)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    m_pAosApplication->SetImsCall(IMS_TRUE);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfDataOos)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_FALSE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfNotSupportedRat)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_WCDMA));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfLteCombinedAttach)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pAosApplication->SetLteExtraInfo(IMS_LTE_INFO_EXTRA_NONE);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfVopsNotSupported)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfAcIncompleted)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AC_INCOMPLETED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfAuthenticationFailed)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfUsimAuthenticationFailed)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_USIM_AUTHENTICATION_FAILED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfAosIncompleted)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AOS_INCOMPLETED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfPermanentDataFailed)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfEnablerDetached)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_ENABLER_DETACHED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfImsDisabled)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_DISABLED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfPermanentRegFailed)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfSubscriberIncompleted)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfImsServiceDisabled)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        StopRunningImsEstablishmentTimerWhenImsEstablishmentStartIfInvalidConnection)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_INVALID_CONNECTION))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        KeepRunningImsEstablishmentTimerWhenImsEstablishmentStartIfRatIsNotChanged)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetRat(NW_REPORT_RADIO_LTE);
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));

    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // Clean Up
    m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
}

TEST_F(AosApplicationTest,
        KeepRunningImsEstablishmentTimerWhenImsEstablishmentStartIfOldRatIsNotSupportedType)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetRat(NW_REPORT_RADIO_HSPA);
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // Clean Up
    m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
}

TEST_F(AosApplicationTest, StartImsEstablishmentTimerWhenImsEstablishmentStartIfNoBlockConditions)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetRat(NW_REPORT_RADIO_HSPA);
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosRegistration, IsRegistered()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsDataIn()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentStart();

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // Clean Up
    m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
}

TEST_F(AosApplicationTest, Callback)
{
    // TEST_F : Report_Request
    m_pAosApplication->Report_Request(0, 0);

    // TEST_F : Condition_Changed
    m_pAosApplication->StartTimer(TIMER_MSG_CONDITION, 2000);
    m_pAosApplication->Condition_Changed(0);

    // TEST_F : Condition_RequestCommand
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->SetImsCall(IMS_TRUE);
    m_pAosApplication->Condition_RequestCommand(REQUEST_PDN_DISCONNECT, AosReason::NONE);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::NONE);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_CONNECTED);

    m_pAosApplication->Condition_RequestCommand(REQUEST_PDN_DISCONNECT, AosReason::POWER_OFF);
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::POWER_OFF);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->Condition_RequestCommand(REQUEST_DESTROY, AosReason::NONE);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);

    m_pAosApplication->Condition_RequestCommand(REQUEST_RECOVER, AosReason::NONE);
    m_pAosApplication->Condition_RequestCommand(REQUEST_NONE, AosReason::NONE);
    m_pAosApplication->SetImsCall(IMS_FALSE);
    m_pAosApplication->SetOffReason(AosReason::NONE);

    // TEST_F : Connector_Activated
    m_pAosApplication->Connector_Activated();

    // TEST_F : Connector_Deactivated
    m_pAosApplication->Connector_Deactivated(0);

    // TEST_F : Connector_Updated
    m_pAosApplication->Connector_Updated(0);

    // TEST_F : Registration_StateChanged
    m_pAosApplication->Registration_StateChanged(0, 0);

    // TEST_F : CallTracker_StateChanged
    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(AnyNumber());

    m_pAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT, 0))
            .Times(1);
    m_pAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    m_pAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
    m_pAosApplication->SetAppState(IAosApplication::STATE_NOTREADY);

    // TEST_F : NetTracker_StatusChanged
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->NetTracker_StatusChanged();

    // mobile network type is LTE while m_nLteAttachState is IMS_LTE_INFO_COMBINED_ATTACHED
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_EPS_FALLBACK_STARTED, _)).Times(1);
    m_pAosApplication->NetTracker_StatusChanged();

    // mobile network type is LTE while m_nLteAttachState is not IMS_LTE_INFO_COMBINED_ATTACHED
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_EPS_ONLY_ATTACHED);
    m_pAosApplication->NetTracker_StatusChanged();

    // mobile network type is neither LTE nor NR
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_HSPA));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(
                    IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_DISABLE))
            .Times(1);
    m_pAosApplication->NetTracker_StatusChanged();

    // mobile network type is NR
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(1);
    m_pAosApplication->NetTracker_StatusChanged();

    // mobile network type is NR again while m_nLteAttachState is IMS_LTE_INFO_COMBINED_ATTACHED
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(1);
    m_pAosApplication->NetTracker_StatusChanged();

    // mobile network type is NR again while m_nLteAttachState is not IMS_LTE_INFO_COMBINED_ATTACHED
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_EPS_ONLY_ATTACHED);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(0);
    m_pAosApplication->NetTracker_StatusChanged();

    // IsOn true, IsRegUpdatedByNrLteRatChange true, TIMER_RECONFIG_GUARD running
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosRegistration, GetImsRegNetwork())
            .WillByDefault(Return(AosNetworkType::NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(1);
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_EPS_FALLBACK_STARTED, _)).Times(1);
    ImsVector<IMS_SINT32> objRegUpdateRats;
    objRegUpdateRats.Add(ACCESS_NETWORK_TYPE_EUTRAN);
    objRegUpdateRats.Add(ACCESS_NETWORK_TYPE_NGRAN);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetUpdateRegistrationWithRatChange())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegUpdateRats));
    m_pAosApplication->StartTimer(TIMER_RECONFIG_GUARD, 1000);
    m_pAosApplication->AddFeature(PENDING_IPCAN_HELD);
    m_pAosApplication->RemoveFeature(PENDING_REG_UPDATE_HELD);
    m_pAosApplication->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosApplication->IsFeatureOn(PENDING_IPCAN_HELD));
    EXPECT_TRUE(m_pAosApplication->IsFeatureOn(PENDING_REG_UPDATE_HELD));
    m_pAosApplication->RemoveFeature(PENDING_REG_UPDATE_HELD);
    m_pAosApplication->StopTimer(TIMER_RECONFIG_GUARD);

    // IsOn true, IsRegUpdatedByNrLteRatChange true, TIMER_RECONFIG_GUARD not running
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));
    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_SET_EPS_5GS_ONLY, IAosRegistration::REASON_SET_ENABLE))
            .Times(1);
    m_pAosApplication->NetTracker_StatusChanged();

    // TEST_F : NConfiguration_NotifyConfigChanged
    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL);
    m_pAosApplication->NConfiguration_NotifyConfigChanged();
    AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsGeolocationPidfSupported(_))
            .WillOnce(Return(IMS_TRUE));
    m_pAosApplication->NConfiguration_NotifyConfigChanged();

    // TEST_F : Event_NotifyEvent
    m_pAosApplication->Event_NotifyEvent(IMS_EVENT_RTT_SETTING, 0, 0);
    m_pAosApplication->Event_NotifyEvent(IMS_EVENT_REG_CONTROL, 0, 0);
    m_pAosApplication->Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, 0, 0);
    m_pAosApplication->Event_NotifyEvent(IMS_EVENT_VOICE_SERVICE_STATE, 0, 0);

    // TEST_F : Timer_TimerExpired
    m_pAosApplication->Timer_TimerExpired(IMS_NULL);
    m_pAosApplication->StartTimer(TIMER_RECONFIG_GUARD, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetReconfigTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_RECONFIG_GUARD));
    m_pAosApplication->StartTimer(TIMER_MSG_CONDITION, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetMsgConditionTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_MSG_CONDITION));
    m_pAosApplication->StartTimer(TIMER_REG_STOP, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetRegStopTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_STOP));
    m_pAosApplication->StartTimer(TIMER_REG_BLOCKED, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetRegBlockedTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_REG_BLOCKED));
    m_pAosApplication->StartTimer(TIMER_APP_ACTIVATED, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetAppActivatedTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_ACTIVATED));
    m_pAosApplication->StartTimer(TIMER_APP_CONNECTED, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetAppConnectedTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_CONNECTED));
    m_pAosApplication->StartTimer(TIMER_APP_TERMINATED, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetAppTerminatedTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_APP_TERMINATED));
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED, _)).Times(1);
    m_pAosApplication->StartTimer(TIMER_PDN_BLOCKED, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetPdnBlockedTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_PDN_BLOCKED));
    m_pAosApplication->SetImsCall(IMS_TRUE);
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 1000);
    m_pAosApplication->Timer_TimerExpired(m_pAosApplication->GetImsEstablishmentTimer());
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
    m_pAosApplication->SetImsCall(IMS_FALSE);
    m_pAosApplication->ClearTimers();

    // TEST_F : RegistrationControl_ControlRegistration
    EXPECT_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    // eCause is IMS_SERVICE - eType is START
    EXPECT_CALL(m_objMockAosCondition, ResetBlock(BLOCK_IMS_SERVICE_DISABLED, IMS_TRUE)).Times(1);
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START, AosPcscfOrder::CURRENT, AosControlCause::IMS_SERVICE);
    // eCause is IMS_SERVICE - eType is STOP
    EXPECT_CALL(m_objMockAosCondition, SetBlock(BLOCK_IMS_SERVICE_DISABLED, IMS_FALSE)).Times(1);
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::IMS_SERVICE);
    // eCause is DATA - eType is START
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START, AosPcscfOrder::CURRENT, AosControlCause::DATA);
    // eCause is DATA - eType is REFRESH
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::REFRESH, AosPcscfOrder::CURRENT, AosControlCause::DATA);
    // eCause is DATA - eType is STOP
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::DATA);
    // eCause is DATA - eType is START_IMS_EST_TIMER, Establish time 0
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte()).WillOnce(Return(0));
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START_IMS_EST_TIMER, AosPcscfOrder::CURRENT, AosControlCause::DATA);
    // eCause is DATA - eType is START_IMS_EST_TIMER, Establish time 120
    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillRepeatedly(Return(120));
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::START_IMS_EST_TIMER, AosPcscfOrder::CURRENT, AosControlCause::DATA);

    // TEST_F : ServicePhone_LocationInfoChanged
    // IsReregRetryWithChangedCountryOnWifi return false
    m_pAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::COUNTRY_CHANGED);
    // IsReregRetryWithChangedCountryOnWifi return true - eState is COUNTRY_CHANGED, epdg not
    // enabled
    EXPECT_CALL(m_objMockIAosNConfiguration, IsReregRetryWithChangedCountryOnWifi())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::COUNTRY_CHANGED);
    // IsReregRetryWithChangedCountryOnWifi return true - eState is COUNTRY_CHANGED, epdg enabled
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_TRUE));
    m_pAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::COUNTRY_CHANGED);
    // IsReregRetryWithChangedCountryOnWifi return true - eState is AVAILABLE
    m_pAosApplication->ServicePhone_LocationInfoChanged(LocationInfo::AVAILABLE);
}

TEST_F(AosApplicationTest, SetDisconnectingStateWhenControlRegistrationCalledWithStop)
{
    m_pAosApplication->RegistrationControl_ControlRegistration(AosRegRequestType::STOP,
            AosPcscfOrder::CURRENT, AosControlCause::RADIO_ALLOWED_NETWORK_TYPES_CHANGED);

    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
}

TEST_F(AosApplicationTest,
        SetAirplaneModeReasonWhenControlRegistrationCalledWithStopIfAirplaneModeReasonIsPreviouslySet)
{
    m_pAosApplication->SetOffReason(AosReason::AIRPLANE_MODE);

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::DATA);

    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::AIRPLANE_MODE);
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
}

TEST_F(AosApplicationTest,
        SetPdnDeactivationRequiredFalseWhenControlRegistrationCalledWithStopIfSimRemovedForEmergency)
{
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::RADIO_SIM_REMOVED);

    EXPECT_FALSE(m_pAosApplication->IsPdnDeactivationRequired());
}

TEST_F(AosApplicationTest,
        SetPdnDeactivationRequiredTrueWhenControlRegistrationCalledWithStopIfSimRemovedForNormal)
{
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::RADIO_SIM_REMOVED);

    EXPECT_TRUE(m_pAosApplication->IsPdnDeactivationRequired());
    EXPECT_EQ(m_pAosApplication->GetState(), IAosApplication::STATE_DISCONNECTING);
}

TEST_F(AosApplicationTest,
        SetPdnDeactivationRequiredTrueWhenControlRegistrationCalledWithStopIfSimRefreshForNormal)
{
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::RADIO_SIM_REFRESH);

    EXPECT_TRUE(m_pAosApplication->IsPdnDeactivationRequired());
}

TEST_F(AosApplicationTest, InvokeResetReadyRecoveryWhenReceiveResetPcscfRecoveryRequest)
{
    EXPECT_CALL(m_objMockAosConnector, ResetReadyRecovery());

    m_pAosApplication->Condition_RequestCommand(REQUEST_RESET_CONNECTION_RECOVERY, AosReason::NONE);
}

TEST_F(AosApplicationTest, SetAirplaneModeReasonWhenReceiveRequestReasonUpdateWithAirplaneMode)
{
    m_pAosApplication->Condition_RequestCommand(
            AosCondition::REQUEST_REASON_UPDATE, AosReason::AIRPLANE_MODE);

    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::AIRPLANE_MODE);
}

TEST_F(AosApplicationTest, UpdateConnectedServices)
{
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_TRUE));
    m_pAosApplication->UpdateConnectedServices(IMS_FALSE);
}

TEST_F(AosApplicationTest, ShouldSetOffReasonWithInitialRegRequestedInScscfRestoration)
{
    // GIVEN
    m_pAosApplication->SetOffReason(AosReason::NONE);
    ImsMessage objMessage(MSG_SCSCF_RESTORATION, 0, 30);

    // WHEN
    m_pAosApplication->ProcessScscfRestoration(objMessage);

    // THEN
    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::INITIAL_REG_REQUESTED);
}

TEST_F(AosApplicationTest, ShouldRequestScscfRestorationToRegistration)
{
    // GIVEN
    m_pAosApplication->SetOffReason(AosReason::NONE);
    ImsMessage objMessage(MSG_SCSCF_RESTORATION, 0, 30);

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_SCSCF_RESTORATION, 30));

    // WHEN
    m_pAosApplication->ProcessScscfRestoration(objMessage);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfRegTypeIsEmergency)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetAppType(AosRegistrationType::EMERGENCY);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfEstTimeIsEqualToOrLessThanZero)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte()).WillByDefault(Return(0));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfImsRegisteredState)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    m_pAosApplication->SetAppState(IAosApplication::STATE_CONNECTED);
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, KeepImsEstablishmentTimerIfAlreadyStartedWhenImsEstablishmentControl)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    m_pAosApplication->StartTimer(TIMER_IMS_ESTABLISHMENT, 12345);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // Clean Up
    m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfLteCombinedAttach)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    m_pAosApplication->SetLteAttachState(IMS_LTE_INFO_COMBINED_ATTACHED);
    m_pAosApplication->SetLteExtraInfo(IMS_LTE_INFO_EXTRA_NONE);

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, NotStartImsEstablishmentTimerByImsEstablishmentControlIfVopsNotSupported)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, NotStartImsEstablishmentTimerByImsEstablishmentControlIfAcIncompleted)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AC_INCOMPLETED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfAuthenticationFailed)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfUsimAuthenticationFailed)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_USIM_AUTHENTICATION_FAILED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, NotStartImsEstablishmentTimerByImsEstablishmentControlIfAosIncompleted)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_AOS_INCOMPLETED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfPermanentDataFailed)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, NotStartImsEstablishmentTimerByImsEstablishmentControlIfEnablerDetached)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_ENABLER_DETACHED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, NotStartImsEstablishmentTimerByImsEstablishmentControlIfImsDisabled)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_DISABLED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfPermanentRegFailed)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfSubscriberIncompleted)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfImsServiceDisabled)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_IMS_SERVICE_DISABLED))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest,
        NotStartImsEstablishmentTimerByImsEstablishmentControlIfInvalidConnection)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockAosCondition, IsReasonBlocked(BLOCK_INVALID_CONNECTION))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));
}

TEST_F(AosApplicationTest, StartImsEstablishmentTimerByImsEstablishmentControlIfNoBlockConditions)
{
    // GIVEN
    ImsMessage objMessage(MSG_IMS_EST_TIMER_CONTROL, 0, 0);
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillByDefault(Return(120));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->ProcessImsEstablishmentControl(objMessage);

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsTimerRunning(TIMER_IMS_ESTABLISHMENT));

    // Clean Up
    m_pAosApplication->StopTimer(TIMER_IMS_ESTABLISHMENT);
}

TEST_F(AosApplicationTest, ReturnImsEstablishmentTimeForLteIfRatIsLte)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();

    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForLte())
            .WillRepeatedly(Return(120));

    // WHEN & THEN
    EXPECT_EQ(m_pAosApplication->GetImsEstablishmentTime(), 120);
}

TEST_F(AosApplicationTest, ReturnImsEstablishmentTimeForNrIfRatIsNr)
{
    // GIVEN
    m_pAosApplication->SetNetTrackerListener();
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetImsEstablishmentTimeForNr())
            .WillRepeatedly(Return(160));

    // WHEN & THEN
    EXPECT_EQ(m_pAosApplication->GetImsEstablishmentTime(), 160);
}

TEST_F(AosApplicationTest,
        ProcessCleanAllShouldStopConnectorWhenPdnDeactivationRequiredIsTrue)
{
    // GIVEN
    m_pAosApplication->SetPdnDeativationRequired(IMS_TRUE);

    EXPECT_CALL(m_objMockAosConnector, Stop());

    // WHEN
    m_pAosApplication->CleanAll();

    // THEN : GIVEN conditions should be met.
    EXPECT_FALSE(m_pAosApplication->IsPdnDeactivationRequired());
}

TEST_F(AosApplicationTest, KeepRegistrationIfStopRetryTimerIsRunningWhenCleanAllWithServicePolicy)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsKeepRegRetryTimerOnAllEnablersDetached())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGSTOP));
    ON_CALL(m_objMockIAosRegistration, IsRetryTimer()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistration, Destroy()).Times(0);

    // WHEN
    m_pAosApplication->CleanAll(AosReason::SERVICE_POLICY);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosApplicationTest, DestroyRegistrationIfNotRegStopStateWhenCleanAllWithServicePolicy)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsKeepRegRetryTimerOnAllEnablersDetached())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGISTERED));

    EXPECT_CALL(m_objMockIAosRegistration, Destroy());

    // WHEN
    m_pAosApplication->CleanAll(AosReason::SERVICE_POLICY);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosApplicationTest,
        DestroyRegistrationIfStopRetryTimerIsNotRunningWhenCleanAllWithServicePolicy)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsKeepRegRetryTimerOnAllEnablersDetached())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration, GetState())
            .WillByDefault(Return(IAosRegistration::STATE_REGSTOP));
    ON_CALL(m_objMockIAosRegistration, IsRetryTimer()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosRegistration, Destroy());

    // WHEN
    m_pAosApplication->CleanAll(AosReason::SERVICE_POLICY);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosApplicationTest, SetDataFailureReasonWhenUpdateDataFailureReason)
{
    IMS_SINT32 anyDataFailureReason = 1;
    m_pAosApplication->RegistrationControl_UpdateDataFailureReason(anyDataFailureReason);
    EXPECT_EQ(m_pAosApplication->GetDataFailureReason(), anyDataFailureReason);
}

TEST_F(AosApplicationTest, ReportDataFailureReasonWhenControlRegistrationCalledWithStop)
{
    IMS_SINT32 anyDataFailureReason = 1;
    m_pAosApplication->RegistrationControl_UpdateDataFailureReason(anyDataFailureReason);
    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::DATA);

    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
    EXPECT_EQ(m_pAosApplication->GetDataFailureReason(), anyDataFailureReason);
}

TEST_F(AosApplicationTest, ReportDataFailureReasonWhenConnectionDeactiviated)
{
    IMS_SINT32 anyDataFailureReason = 1;
    m_pAosApplication->RegistrationControl_UpdateDataFailureReason(anyDataFailureReason);
    m_pAosApplication->ProcessConnectionDeactivated(AosConnector::REASON_DISCONNECTED);

    EXPECT_EQ(m_pAosApplication->GetOffReason(), AosReason::DATA_DISCONNECTED);
    EXPECT_EQ(m_pAosApplication->GetDataFailureReason(), anyDataFailureReason);
}

TEST_F(AosApplicationTest, NotifyDeregisteredWhenPlmnBlockDueToVopsNotSupported)
{
    ImsMessage objMessage(MSG_PLMN_BLOCK_WITH_TIMEOUT, AosReason::VOPS_NOT_SUPPORTED, 0);
    m_pAosApplication->SetNetTrackerListener();
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT_BY_VOPS_NOT_SUPPORTED, _))
            .Times(1);

    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, NotifyDeregisteredWhenPlmnBlockDueToSsacBarred)
{
    ImsMessage objMessage(MSG_PLMN_BLOCK_WITH_TIMEOUT, AosReason::SSAC_BARRED, 0);
    m_pAosApplication->SetNetTrackerListener();
    EXPECT_CALL(m_objMockIAosService,
            NotifyDeregistered(IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE,
                    AosReasonCode::PLMN_BLOCK_WITH_TIMEOUT_BY_SSAC_BARRED, _))
            .Times(1);

    EXPECT_TRUE(m_pAosApplication->ProcessMessage(objMessage));
}

TEST_F(AosApplicationTest, SetNormalRegistrationWhenRegistrationStopDueToSimRemoved)
{
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    EXPECT_CALL(m_objMockIAosRegistration, SetReasonCode(AosReasonCode::NORMAL_DEREGISTRATION));

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::RADIO_SIM_REMOVED);
}

TEST_F(AosApplicationTest, SetNormalRegistrationWhenRegistrationStopDueToSimRefresh)
{
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);

    EXPECT_CALL(m_objMockIAosRegistration, SetReasonCode(AosReasonCode::NORMAL_DEREGISTRATION));

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::RADIO_SIM_REFRESH);
}

TEST_F(AosApplicationTest, SetNormalRegistrationWhenRegistrationStopDueToAirplaneMode)
{
    m_pAosApplication->SetAppType(AosRegistrationType::NORMAL);
    m_pAosApplication->SetOffReason(AosReason::AIRPLANE_MODE);

    EXPECT_CALL(m_objMockIAosRegistration, SetReasonCode(AosReasonCode::NORMAL_DEREGISTRATION));

    m_pAosApplication->RegistrationControl_ControlRegistration(
            AosRegRequestType::STOP, AosPcscfOrder::CURRENT, AosControlCause::DATA);
}

TEST_F(AosApplicationTest, ReturnTrueIfRegisteredOnTheGivenNetworkType)
{
    // GIVEN
    ON_CALL(m_objMockIAosRegistration, GetImsRegNetwork())
            .WillByDefault(Return(AosNetworkType::LTE));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosApplication->IsRegisteredNetwork(NW_REPORT_RADIO_LTE));
}

TEST_F(AosApplicationTest, ReturnFalseIfRegisteredOnOtherThanTheGivenNetworkType)
{
    // GIVEN
    ON_CALL(m_objMockIAosRegistration, GetImsRegNetwork())
            .WillByDefault(Return(AosNetworkType::LTE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosApplication->IsRegisteredNetwork(NW_REPORT_RADIO_NR));
}

TEST_F(AosApplicationTest, ReturnFalseIfRegisteredNetworkIsNone)
{
    // GIVEN
    ON_CALL(m_objMockIAosRegistration, GetImsRegNetwork())
            .WillByDefault(Return(AosNetworkType::NONE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosApplication->IsRegisteredNetwork(NW_REPORT_RADIO_INVALID));
}

TEST_F(AosApplicationTest, RemovePendingRegFeatureIfProcessPendingPcscfChangeIsTrueWhenCallStateChangeToIdle)
{
    // GIVEN
    m_pAosApplication->AddFeature(PENDING_REG_RECOVERY_HELD);
    m_pAosApplication->AddFeature(PENDING_REG_STOP_HELD);
    ON_CALL(m_objMockAosConnector, ProcessPendingPcscfChange()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN
    EXPECT_FALSE(m_pAosApplication->IsFeatureOn(PENDING_REG_RECOVERY_HELD));
    EXPECT_FALSE(m_pAosApplication->IsFeatureOn(PENDING_REG_STOP_HELD));
    EXPECT_FALSE(m_pAosApplication->IsImsCall());
    EXPECT_FALSE(m_pAosApplication->IsRegRecoveryHeld());
}

TEST_F(AosApplicationTest, KeepPendingRegFeatureIfProcessPendingPcscfChangeIsFalseWhenCallStateChangeToIdle)
{
    // GIVEN
    m_pAosApplication->AddFeature(PENDING_REG_RECOVERY_HELD);
    m_pAosApplication->AddFeature(PENDING_REG_STOP_HELD);
    ON_CALL(m_objMockAosConnector, ProcessPendingPcscfChange()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsFeatureOn(PENDING_REG_RECOVERY_HELD));
    EXPECT_TRUE(m_pAosApplication->IsFeatureOn(PENDING_REG_STOP_HELD));
}

TEST_F(AosApplicationTest, KeepPendingRegFeatureIfProcessPendingPcscfChangeIsTrueWhenCallStateChangeToOffhook)
{
    // GIVEN
    m_pAosApplication->AddFeature(PENDING_REG_RECOVERY_HELD);
    m_pAosApplication->AddFeature(PENDING_REG_STOP_HELD);
    ON_CALL(m_objMockAosConnector, ProcessPendingPcscfChange()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosApplication->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);

    // THEN
    EXPECT_TRUE(m_pAosApplication->IsFeatureOn(PENDING_REG_RECOVERY_HELD));
    EXPECT_TRUE(m_pAosApplication->IsFeatureOn(PENDING_REG_STOP_HELD));
}