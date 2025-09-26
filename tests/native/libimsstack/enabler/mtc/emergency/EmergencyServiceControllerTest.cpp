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

#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsTypeDef.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcCallController.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/EmergencyServiceController.h"
#include "emergency/MockIMtcEmergencyServiceManager.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class EmergencyServiceControllerTest : public ::testing::Test
{
protected:
    MockIMtcContext objContext;
    MockIMtcService objNormalService;
    MockIMtcService objEmergencyService;
    MockIMtcCallController objCallController;
    MockICallStateProxy objCallStateProxy;
    MockIPassiveTimerHolder objPassiveTimer;
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread objJniMtcServiceThread;
    MockIMtcEmergencyServiceManager objEsm;
    TestPhoneInfoService objPhoneInfoService;
    MockIMessageUtils objMessageUtils;
    MockIMtcCallManager objMockCallManager;
    MockIMtcCall objMockMtcCall;
    MockIMtcCallContext objMockCallContext;
    MockIMtcSession objMockMtcSession;
    MockISession objMockSession;

    MockMtcConfigurationProxy* pConfigurationProxy;
    EmergencyServiceController* pController;

    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();

        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objNormalService));
        ON_CALL(objContext, GetServiceByType(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objEmergencyService));
        ON_CALL(objContext, GetCallController).WillByDefault(ReturnRef(objCallController));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMockCallManager, GetCallByCallKey(_)).WillByDefault(Return(&objMockMtcCall));
        ON_CALL(objMockMtcCall, GetCallContext).WillByDefault(ReturnRef(objMockCallContext));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMockMtcSession));
        ON_CALL(objMockMtcSession, GetISession).WillByDefault(ReturnRef(objMockSession));

        ON_CALL(objNormalService, GetJniServiceThread)
                .WillByDefault(Return(&objJniMtcServiceThread));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
                .WillByDefault(Return(0));  // Default: not in roaming

        pController = new EmergencyServiceController(objEsm, objContext);
    }

    virtual void TearDown() override
    {
        EXPECT_CALL(objCallStateProxy, RemoveListener(pController)).Times(1);
        EXPECT_CALL(objEmergencyService, RemoveAosStateListener(pController)).Times(1);
        EXPECT_CALL(objPassiveTimer,
                RemoveListener(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, pController));
        EXPECT_CALL(objPassiveTimer,
                RemoveListener(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN,
                        pController));
        EXPECT_CALL(objPassiveTimer,
                RemoveTimer(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN))
                .Times(1);
        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objNormalService));
        EXPECT_CALL(objNormalService, RemoveNetworkWatcherListener(pController)).Times(1);

        delete pController;
        delete pConfigurationProxy;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }

    void StartAndAosDisconnectedOnWlan()
    {
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(objNormalService, IsActive()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_TRUE));
        pController->Start();
        pController->OnAosStateChanged(
                objEmergencyService, MtcAosState::DISCONNECTED, ImsAosReason::DATA_DISCONNECTED, 0);
    }
};

TEST_F(EmergencyServiceControllerTest, NoJniThreadDoesNothing)
{
    ON_CALL(objNormalService, GetJniServiceThread).WillByDefault(Return(nullptr));

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, NoNormalServiceDoesNothing)
{
    ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL)).WillByDefault(Return(nullptr));

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, StartAddsListeners)
{
    EXPECT_CALL(objCallStateProxy, AddListener(pController)).Times(1);
    EXPECT_CALL(objEmergencyService, AddAosStateListener(pController)).Times(1);
    EXPECT_CALL(objPassiveTimer,
            AddListener(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, pController));
    EXPECT_CALL(objNormalService, AddNetworkWatcherListener(pController)).Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, StartStartsRegistration)
{
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_START)).Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, StartNotifiesOpening)
{
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    IuMtcService::EmergencyServiceState::OPENING, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, CloseStopsRegistration)
{
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(1);

    pController->Close();
}

TEST_F(EmergencyServiceControllerTest, AosStateChangeNotifiesNothingInitially)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, nAosReason, 0);
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
}

TEST_F(EmergencyServiceControllerTest, CallStateChangeDoesNothingInitially)
{
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    // clang-format off
    std::vector<IMtcCall::State> objCallStates{
            IMtcCall::State::IDLE, IMtcCall::State::OUTGOING,
            IMtcCall::State::INCOMING, IMtcCall::State::ALERTING,
            IMtcCall::State::ESTABLISHED, IMtcCall::State::UPDATING,
            IMtcCall::State::TERMINATING};
    // clang-format on
    for (IMtcCall::State eCallState : objCallStates)
    {
        pController->OnCallStateChanged(
                1, eCallState, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    }
}

TEST_F(EmergencyServiceControllerTest, OnCallSessionReleasedDoesNothingInitially)
{
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallSessionReleased(1, IMS_TRUE, IMS_FALSE);
}

TEST_F(EmergencyServiceControllerTest, StartAndStartNotifiesOpening)
{
    pController->Start();

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    IuMtcService::EmergencyServiceState::OPENING, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->Start();
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENING);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedNotifiesUnavailable)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::IDLE);
}

TEST_F(EmergencyServiceControllerTest,
        StartAndAosDisconnectedOtherThanDataDisconnectedNotifiesUnavailable)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::POWER_OFF;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::IDLE);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedDoesNothingIfNormalServiceIsNull)
{
    ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL)).WillByDefault(Return(nullptr));

    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(0);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::IDLE);
}

TEST_F(EmergencyServiceControllerTest,
        StartAndAosDisconnectedNotifiesUnavailableIfNormalServiceIsNotActive)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsActive()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::IDLE);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedOnWlanStartsTimer)
{
    const IMS_SINT32 nTimer = 10;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT))
            .WillByDefault(Return(nTimer));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, nTimer, IMS_FALSE, IMS_FALSE))
            .Times(1);
    EXPECT_CALL(objPassiveTimer,
            AddListener(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, pController))
            .Times(1);
    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN, nTimer,
                    IMS_FALSE, IMS_TRUE))
            .Times(1);
    EXPECT_CALL(objPassiveTimer,
            AddListener(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN,
                    pController))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    StartAndAosDisconnectedOnWlan();
}

TEST_F(EmergencyServiceControllerTest, HandoverTimerExpiresNotifiesUnavailable)
{
    StartAndAosDisconnectedOnWlan();

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(1);
    // EXPECT_CALL(objEsm, StopOpen(IMS_FALSE)).Times(1); - By AsyncRunner

    pController->OnPassiveTimerExpired(
            IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN);
}

TEST_F(EmergencyServiceControllerTest, RatChangedToWwanRetriesOnImsPdn)
{
    StartAndAosDisconnectedOnWlan();

    ON_CALL(objPassiveTimer,
            IsActive(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(0));

    EXPECT_CALL(objPassiveTimer,
            RemoveTimer(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN))
            .Times(1);
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(0);
    // EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(1); - By AsyncRunner

    pController->OnRatChanged(ServiceType::NORMAL, INetworkWatcher::RADIOTECH_TYPE_IWLAN,
            INetworkWatcher::RADIOTECH_TYPE_LTE);
}

TEST_F(EmergencyServiceControllerTest, RatChangedToWwanInRoamingNotifiesUnavailable)
{
    StartAndAosDisconnectedOnWlan();

    ON_CALL(objPassiveTimer,
            IsActive(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(1));

    EXPECT_CALL(objPassiveTimer,
            RemoveTimer(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN))
            .Times(1);
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X))
            .Times(1);

    pController->OnRatChanged(ServiceType::NORMAL, INetworkWatcher::RADIOTECH_TYPE_IWLAN,
            INetworkWatcher::RADIOTECH_TYPE_LTE);
}

TEST_F(EmergencyServiceControllerTest, RatChangedToWlanDoesNothing)
{
    StartAndAosDisconnectedOnWlan();

    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnRatChanged(ServiceType::NORMAL, INetworkWatcher::RADIOTECH_TYPE_IWLAN,
            INetworkWatcher::RADIOTECH_TYPE_IWLAN);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedInRoamingNotifiesUnavailable)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsActive()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(1));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NONE, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::IDLE);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedWithDataPermanentlyFailed)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_PERMANENTLY_FAILED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsActive()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(1));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::DATA_PERMANENTLY_FAILED,
                    ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedWithNetworkAttachRejected)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::NETWORK_ATTACH_REJECTED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsActive()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(1));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::UNAVAILABLE,
                    EmergencyServiceUnavailableReason::NETWORK_ATTACH_REJECTED,
                    ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedRetriesOverImsPdnWhenConfigSet)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigEmergency::KEY_RETRY_EMERGENCY_ON_IMS_PDN_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsActive()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objNormalService, IsWlanIpCanType()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);
    // EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(1); - By AsyncRunner

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosConnectedNotifiesOpened)
{
    pController->Start();

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    IuMtcService::EmergencyServiceState::OPENED, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndStartNotifiesOpened)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    IuMtcService::EmergencyServiceState::OPENED, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->Start();
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndAosConnectedNotifiesNothing)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);
    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndAosDisconnectedNotifiesIdle)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(IuMtcService::EmergencyServiceState::IDLE,
                    EmergencyServiceUnavailableReason::UNKNOWN, ServiceType::EMERGENCY))
            .Times(1);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason, 0);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallNormallyEndsDoesNothing)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallSessionReleased(1, IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallSetupFailDoesNothingByConfig)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigEmergency::KEY_RELEASE_EMERGENCY_PDN_ON_FAILURE_AFTER_100_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallSessionReleased(1, IMS_TRUE, IMS_FALSE);

    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallSetupFailWithout100DoesNothing)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigEmergency::KEY_RELEASE_EMERGENCY_PDN_ON_FAILURE_AFTER_100_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsResponseExist(_, _)).WillByDefault(Return(IMS_FALSE));

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallSessionReleased(1, IMS_TRUE, IMS_FALSE);

    EXPECT_EQ(pController->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallSetupFailRegisterStop)
{
    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(1);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigEmergency::KEY_RELEASE_EMERGENCY_PDN_ON_FAILURE_AFTER_100_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsResponseExist(_, _)).WillByDefault(Return(IMS_TRUE));

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallSessionReleased(1, IMS_TRUE, IMS_FALSE);
}

TEST_F(EmergencyServiceControllerTest, StartStartsRegTo18xTimer)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, nTimer, IMS_FALSE, IMS_FALSE))
            .Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, StartStartsRegTo18xTimerForWifi)
{
    const IMS_BOOL bWifi = IMS_TRUE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 1))
            .WillByDefault(Return(nTimer));

    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, nTimer, IMS_FALSE, IMS_FALSE))
            .Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, CallStateChangedToTerminatingStopsRegTo18xTimer)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);

    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X));

    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, CloseStopsRegTo18xTimer)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    pController->Start();

    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X));
    EXPECT_CALL(objPassiveTimer,
            RemoveTimer(IPassiveTimerHolder::Type::WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN));

    pController->Close();
}

TEST_F(EmergencyServiceControllerTest, AosDisconnectedStopsRegTo18xTimer)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    pController->Start();

    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X));

    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::DISCONNECTED, ImsAosReason::NONE, 0);
}

TEST_F(EmergencyServiceControllerTest, RegTo18xTimerExpiresBeforeCallSetupStopsAos)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    pController->Start();

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP));

    pController->OnPassiveTimerExpired(IPassiveTimerHolder::Type::REGISTRATION_TO_18X);
}

TEST_F(EmergencyServiceControllerTest, RegTo18xTimerExpiresAfterCallSetupTerminatesCall)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);

    const IMS_SINT32 nCallKey = 1;
    pController->OnCallStateChanged(
            nCallKey, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);

    EXPECT_CALL(objCallController,
            Terminate(nCallKey,
                    CallReasonInfo(
                            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY)));

    pController->OnPassiveTimerExpired(IPassiveTimerHolder::Type::REGISTRATION_TO_18X);
}

TEST_F(EmergencyServiceControllerTest,
        CallStateChangedToIdleStartsRegTo18xTimerIfServiceAlreadyOpened)
{
    const IMS_BOOL bWifi = IMS_FALSE;
    const IMS_SINT32 nTimer = 10;
    ON_CALL(objEmergencyService, IsWlanIpCanType).WillByDefault(Return(bWifi));
    ON_CALL(*pConfigurationProxy,
            GetIntFromArray(ConfigVoice::KEY_REGISTRATION_TO_18X_TIMER_MILLIS_INT_ARRAY, 0))
            .WillByDefault(Return(nTimer));

    pController->Start();
    pController->OnAosStateChanged(
            objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);

    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X, nTimer, IMS_FALSE, IMS_FALSE));

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, OnTotalCallStateChangedDoesNothing)
{
    const IMtcCall::State ANY_STATE = IMtcCall::State::IDLE;
    pController->OnTotalCallStateChanged(ANY_STATE);
    // Nothing to be checked.
}
