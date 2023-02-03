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
#include "emergency/NormalRoutingEmergencyServiceController.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class NormalRoutingEmergencyServiceControllerTest : public ::testing::Test
{
protected:
    MockIMtcContext objContext;
    MockIMtcService objNormalService;
    MockICallStateProxy objCallStateProxy;
    MockIJniMtcServiceThread objJniMtcServiceThread;
    MockIMtcEmergencyServiceManager objEsm;

    NormalRoutingEmergencyServiceController* pController;

    virtual void SetUp() override
    {
        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objNormalService));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));

        ON_CALL(objNormalService, GetJniServiceThread)
                .WillByDefault(Return(&objJniMtcServiceThread));

        pController = new NormalRoutingEmergencyServiceController(objEsm, objContext);
    }

    virtual void TearDown() override
    {
        EXPECT_CALL(objCallStateProxy, RemoveListener(pController)).Times(1);

        delete pController;
    }
};

TEST_F(NormalRoutingEmergencyServiceControllerTest, NoJniThreadDoesNothing)
{
    ON_CALL(objNormalService, GetJniServiceThread).WillByDefault(Return(nullptr));

    pController->Start();
}

TEST_F(NormalRoutingEmergencyServiceControllerTest, NoNormalServiceDoesNothing)
{
    ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL)).WillByDefault(Return(nullptr));

    pController->Start();
}

TEST_F(NormalRoutingEmergencyServiceControllerTest, StartAddsListenersIfNormalServiceActive)
{
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    EXPECT_CALL(objCallStateProxy, AddListener(pController)).Times(1);

    pController->Start();
}

TEST_F(NormalRoutingEmergencyServiceControllerTest, StartNotifiesOpenedIfNormalServiceActive)
{
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), _,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)))
            .Times(1);

    pController->Start();
}

TEST_F(NormalRoutingEmergencyServiceControllerTest,
        StartNotifiesUnavailableIfNormalServiceNotActive)
{
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_IDLE));
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::UNAVAILABLE),
                    _, static_cast<IMS_SINT32>(ServiceType::NORMAL)))
            .Times(1);

    pController->Start();
}

TEST_F(NormalRoutingEmergencyServiceControllerTest, CallTerminatesDoesNothing)
{
    EXPECT_CALL(objJniMtcServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::IDLE), _,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)))
            .Times(1);

    pController->OnCallStateChanged(
            0, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(NormalRoutingEmergencyServiceControllerTest, NormalCallTerminatesDoesNothing)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pController->OnCallStateChanged(
            0, IMtcCall::State::TERMINATING, IMtcCallStateListener::Type::VOIP, IMS_FALSE, 0);
}
