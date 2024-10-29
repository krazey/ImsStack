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

#include "ImsTypeDef.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "emergency/MockIMtcEmergencyServiceManager.h"
#include "emergency/NormalServiceController.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class NormalServiceControllerTest : public ::testing::Test
{
protected:
    MockIMtcContext objContext;
    MockIMtcService objNormalService;
    MockICallStateProxy objCallStateProxy;
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread objJniMtcServiceThread;
    MockIMtcEmergencyServiceManager objEsm;

    NormalServiceController* pController;

    virtual void SetUp() override
    {
        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objNormalService));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetAosConnector(_)).WillByDefault(Return(&objAosConnector));

        ON_CALL(objNormalService, GetJniServiceThread)
                .WillByDefault(Return(&objJniMtcServiceThread));

        pController = new NormalServiceController(objEsm, objContext);
    }

    virtual void TearDown() override
    {
        EXPECT_CALL(objCallStateProxy, RemoveListener(pController)).Times(1);

        delete pController;
    }
};

TEST_F(NormalServiceControllerTest, NoJniThreadDoesNothing)
{
    ON_CALL(objNormalService, GetJniServiceThread).WillByDefault(Return(nullptr));

    pController->Start();
}

TEST_F(NormalServiceControllerTest, NoNormalServiceDoesNothing)
{
    ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL)).WillByDefault(Return(nullptr));

    pController->Start();
}

TEST_F(NormalServiceControllerTest, StartAddsListenersIfNormalServiceActive)
{
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    EXPECT_CALL(objCallStateProxy, AddListener(pController)).Times(1);

    pController->Start();
}

TEST_F(NormalServiceControllerTest, StartNotifiesOpenedIfNormalServiceActive)
{
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(EmergencyServiceState::OPENED, _, ServiceType::NORMAL))
            .Times(1);

    pController->Start();
}

TEST_F(NormalServiceControllerTest, CloseDoesNothing)
{
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pController->Close();
}

TEST_F(NormalServiceControllerTest, StartNotifiesUnavailableIfNormalServiceNotActive)
{
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_IDLE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(EmergencyServiceState::UNAVAILABLE, _, ServiceType::NORMAL))
            .Times(1);

    pController->Start();
}

TEST_F(NormalServiceControllerTest, CallTerminatesDoesNothing)
{
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(EmergencyServiceState::IDLE, _, ServiceType::NORMAL))
            .Times(1);

    pController->OnCallStateChanged(
            0, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(NormalServiceControllerTest, NormalCallTerminatesDoesNothing)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pController->OnCallStateChanged(
            0, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_FALSE, 0);
}

TEST_F(NormalServiceControllerTest, GetServiceTypeReturnsNormal)
{
    EXPECT_EQ(pController->GetServiceType(), ServiceType::NORMAL);
}

TEST_F(NormalServiceControllerTest, OnTotalCallStateChangedDoesNothing)
{
    const IMtcCall::State ANY_STATE = IMtcCall::State::IDLE;
    pController->OnTotalCallStateChanged(ANY_STATE);
    // Nothing to be checked.
}
