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
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "call/IMtcCall.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/EmergencyServiceController.h"
#include "emergency/MockIMtcEmergencyServiceManager.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class EmergencyServiceControllerTest : public ::testing::Test
{
protected:
    MockIMtcContext objContext;
    MockIMtcService objNormalService;
    MockIMtcService objEmergencyService;
    MockICallStateProxy objCallStateProxy;
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread objJniMtcServiceThread;
    MockIMtcEmergencyServiceManager objEsm;
    TestPhoneInfoService objPhoneInfoService;

    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    EmergencyServiceController* pController;

    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);

        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objNormalService));
        ON_CALL(objContext, GetServiceByType(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objEmergencyService));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

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

        delete pController;
        delete pConfigurationProxy;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
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
            OnEmergencyServiceChanged(EmergencyServiceState::OPENING, _, ServiceType::EMERGENCY))
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
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, nAosReason);
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason);
}

TEST_F(EmergencyServiceControllerTest, CallStateChangeDoesNothingInitially)
{
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, StartAndStartNotifiesOpening)
{
    pController->Start();

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(EmergencyServiceState::OPENING, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedNotifiesUnavailable)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    EmergencyServiceState::UNAVAILABLE, nAosReason, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason);
}

TEST_F(EmergencyServiceControllerTest,
        StartAndAosDisconnectedOtherThanDataDisconnectedNotifiesUnavailable)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::POWER_OFF;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    EmergencyServiceState::UNAVAILABLE, nAosReason, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedInRoamingNotifiesUnavailable)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(1));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    EmergencyServiceState::UNAVAILABLE, nAosReason, ServiceType::EMERGENCY))
            .Times(1);
    EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosDisconnectedRetriesOverImsPdnWhenConfigSet)
{
    pController->Start();

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);
    // EXPECT_CALL(objEsm, StartOpen(ServiceType::NORMAL)).Times(1); - By AsyncRunner

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason);
}

TEST_F(EmergencyServiceControllerTest, StartAndAosConnectedNotifiesOpened)
{
    pController->Start();

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(EmergencyServiceState::OPENED, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndStartNotifiesOpened)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(EmergencyServiceState::OPENED, _, ServiceType::EMERGENCY))
            .Times(1);

    pController->Start();
}

TEST_F(EmergencyServiceControllerTest, OpenedAndAosConnectedNotifiesNothing)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndAosDisconnectedNotifiesIdle)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    const IMS_UINT32 nAosReason = ImsAosReason::DATA_DISCONNECTED;
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(
                    EmergencyServiceState::IDLE, nAosReason, ServiceType::EMERGENCY))
            .Times(1);

    pController->OnAosStateChanged(objEmergencyService, MtcAosState::DISCONNECTED, nAosReason);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallNormallyEndsDoesNothing)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::ESTABLISHED, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallSetupFailDoesNothing)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndCallSetupFailStopsRegistrationWhenConfigSet)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    ON_CALL(*pConfigurationManager, IsReleaseEmergencyPdnWithEmergencyCallFail)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(1);

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndNormalCallSetupFailDoesNothingWhenConfigSet)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    ON_CALL(*pConfigurationManager, IsReleaseEmergencyPdnWithEmergencyCallFail)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pController->OnCallStateChanged(
            1, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_FALSE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_FALSE, 0);
    pController->OnCallStateChanged(
            1, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_FALSE, 0);
}

TEST_F(EmergencyServiceControllerTest, OpenedAndOtherEmergencyCallBlockDoesNothingWhenConfigSet)
{
    pController->Start();
    pController->OnAosStateChanged(objEmergencyService, MtcAosState::CONNECTED, ImsAosReason::NONE);

    ON_CALL(*pConfigurationManager, IsReleaseEmergencyPdnWithEmergencyCallFail)
            .WillByDefault(Return(IMS_TRUE));

    const CallKey nFirstCall = 1;
    pController->OnCallStateChanged(
            nFirstCall, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(
            nFirstCall, IMtcCall::State::OUTGOING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(nFirstCall, IMtcCall::State::ESTABLISHED,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);

    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    // Blocked because call cannot be added while an emergency call
    const CallKey nSecondCall = 2;
    pController->OnCallStateChanged(
            nSecondCall, IMtcCall::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pController->OnCallStateChanged(nSecondCall, IMtcCall::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyServiceControllerTest, OnIpcanChangedDoesNothing)
{
    const IMS_UINT32 ANY_VALUE = 0;
    pController->OnIpcanChanged(objNormalService, ANY_VALUE);
    pController->OnIpcanChanged(objEmergencyService, ANY_VALUE);
    // Nothing to be checked.
}

TEST_F(EmergencyServiceControllerTest, OnTotalCallStateChangedDoesNothing)
{
    const IMtcCall::State ANY_STATE = IMtcCall::State::IDLE;
    pController->OnTotalCallStateChanged(ANY_STATE);
    // Nothing to be checked.
}
