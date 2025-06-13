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

#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "ImsTypeDef.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoLocation.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/MtcEmergencyServiceManager.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/MtcLocationRefresher.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class TestEmergencyServiceManager : public MtcEmergencyServiceManager
{
public:
    inline explicit TestEmergencyServiceManager(IN IMtcContext& objContext) :
            MtcEmergencyServiceManager(objContext)
    {
    }

    inline IEmergencyServiceController* GetController() const { return m_pController.get(); }
};

class MtcEmergencyServiceManagerTest : public ::testing::Test
{
protected:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockICallStateProxy objCallStateProxy;
    MockIPassiveTimerHolder objPassiveTimer;
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread objJniMtcServiceThread;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockILocationInfo objLocationInfo;
    MtcLocationRefresher* pLocationRefresher;

    TestEmergencyServiceManager* pEsm;

    virtual void SetUp() override
    {
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(&objJniMtcServiceThread));
        pLocationRefresher = new MtcLocationRefresher(objLocationInfo);
        ON_CALL(objContext, GetLocationRefresher).WillByDefault(ReturnRef(*pLocationRefresher));

        pEsm = new TestEmergencyServiceManager(objContext);
    }

    virtual void TearDown() override
    {
        delete pEsm;
        delete pLocationRefresher;
    }
};

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsNormalServiceForNormalService)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::NORMAL))
            .Times(1);

    pEsm->StartOpen(ServiceType::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsEmergencyServiceForEmergencyService)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::EMERGENCY))
            .Times(1);

    pEsm->StartOpen(ServiceType::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsEachService)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::EMERGENCY))
            .Times(1);
    pEsm->StartOpen(ServiceType::EMERGENCY);

    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::NORMAL))
            .Times(1);
    pEsm->StartOpen(ServiceType::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, StopOpenDoesNothing)
{
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pEsm->StopOpen(IMS_FALSE);
    pEsm->StopOpen(IMS_TRUE);
}

TEST_F(MtcEmergencyServiceManagerTest, StopOpenStopsRegistrationForEmergencyService)
{
    pEsm->StartOpen(ServiceType::EMERGENCY);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(1);

    pEsm->StopOpen(IMS_TRUE);
}

TEST_F(MtcEmergencyServiceManagerTest, StopOpenDoesNotStopRegistrationForEmergencyService)
{
    pEsm->StartOpen(ServiceType::EMERGENCY);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(0);

    pEsm->StopOpen(IMS_FALSE);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenMakesNewInstanceForServiceType)
{
    IEmergencyServiceController* pPreviousController = nullptr;
    pEsm->StartOpen(ServiceType::EMERGENCY);

    pPreviousController = pEsm->GetController();
    pEsm->StartOpen(ServiceType::EMERGENCY);
    ASSERT_EQ(pPreviousController, pEsm->GetController());

    pPreviousController = pEsm->GetController();
    pEsm->StartOpen(ServiceType::NORMAL);
    ASSERT_NE(pPreviousController, pEsm->GetController());
}

TEST_F(MtcEmergencyServiceManagerTest, GetStateReturnsIdleInitially)
{
    EXPECT_EQ(pEsm->GetState(), IEmergencyServiceController::State::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, GetStateReturnsControllerState)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    pEsm->StartOpen(ServiceType::NORMAL);

    EXPECT_EQ(pEsm->GetState(), IEmergencyServiceController::State::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenDoesNotRequestsLocationUpdateIfTimerIsNotSet)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(0));

    EXPECT_CALL(objLocationInfo, RequestLocationUpdate(_, _)).Times(0);

    pEsm->StartOpen(ServiceType::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenRequestsLocationUpdateIfTimerIsSet)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_CALL(objLocationInfo, RequestLocationUpdate(1000, _));

    pEsm->StartOpen(ServiceType::EMERGENCY);
}
