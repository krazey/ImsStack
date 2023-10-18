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
#include "emergency/MtcEmergencyServiceManager.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIMtcAosConnector.h"
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
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread objJniMtcServiceThread;

    TestEmergencyServiceManager* pEsm;

    virtual void SetUp() override
    {
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));

        ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(&objJniMtcServiceThread));

        pEsm = new TestEmergencyServiceManager(objContext);
    }

    virtual void TearDown() override { delete pEsm; }
};

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsNormalServiceForNormalPdn)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::NORMAL))
            .Times(1);

    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsEmergencyServiceForEmergencyPdn)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::EMERGENCY))
            .Times(1);

    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsEmergencyServiceForUnknownPdn)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::EMERGENCY))
            .Times(1);

    pEsm->StartOpen(EmergencyCallRoutingPdn::UNKNOWN);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenNotifiesAsEachService)
{
    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::EMERGENCY))
            .Times(1);
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);

    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::EMERGENCY))
            .Times(1);
    pEsm->StartOpen(EmergencyCallRoutingPdn::UNKNOWN);

    EXPECT_CALL(objJniMtcServiceThread, OnEmergencyServiceChanged(_, _, ServiceType::NORMAL))
            .Times(1);
    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, StopOpenDoesNothing)
{
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);

    pEsm->StopOpen(IMS_FALSE);
    pEsm->StopOpen(IMS_TRUE);
}

TEST_F(MtcEmergencyServiceManagerTest, StopOpenStopsRegistrationForEmergencyService)
{
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(1);

    pEsm->StopOpen(IMS_TRUE);
}

TEST_F(MtcEmergencyServiceManagerTest, StopOpenDoesNotStopRegistrationForEmergencyService)
{
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_STOP)).Times(0);

    pEsm->StopOpen(IMS_FALSE);
}

TEST_F(MtcEmergencyServiceManagerTest, StartOpenMakesNewInstanceForPdnType)
{
    IEmergencyServiceController* pPreviousController = nullptr;
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);

    pPreviousController = pEsm->GetController();
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
    ASSERT_EQ(pPreviousController, pEsm->GetController());

    pPreviousController = pEsm->GetController();
    pEsm->StartOpen(EmergencyCallRoutingPdn::UNKNOWN);
    ASSERT_EQ(pPreviousController, pEsm->GetController());

    pPreviousController = pEsm->GetController();
    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
    ASSERT_NE(pPreviousController, pEsm->GetController());
}
