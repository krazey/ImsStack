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
#include "IuMtcService.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MtcEmergencyServiceManager.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class TestEmergencyServiceManager : public MtcEmergencyServiceManager
{
public:
    inline explicit TestEmergencyServiceManager(IN IMtcContext& objContext) :
            MtcEmergencyServiceManager(objContext)
    {
    }

    inline IuMtcService::EmergencyServiceState GetState() const { return m_eState; }
    inline void SetState(IN IuMtcService::EmergencyServiceState eState) { m_eState = eState; }
};

class MtcEmergencyServiceManagerTest : public ::testing::Test
{
public:
    inline explicit MtcEmergencyServiceManagerTest() :
            objContext(),
            objAosConnector(),
            pMockServiceThread(new MockIJniMtcServiceThread()),
            objEsm(objContext)
    {
    }

    inline virtual ~MtcEmergencyServiceManagerTest() { delete pMockServiceThread; }

protected:
    MockIMtcContext objContext;
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread* pMockServiceThread;

    TestEmergencyServiceManager objEsm;

    inline void SetUp() override
    {
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));
    }
};

TEST_F(MtcEmergencyServiceManagerTest, GetDefaultStateReturnsIdle)
{
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceDoesNotChangeStateifAosConnectorIsNull)
{
    ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY)).WillByDefault(Return(nullptr));

    objEsm.OpenEmergencyService(pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceInvokesRegisterStart)
{
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_START));
    objEsm.OpenEmergencyService(pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::OPENING);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceSetsStateOpenedIfCurrentStateIsInCall)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::IN_CALL);
    objEsm.OpenEmergencyService(pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceSetsStateOpenedIfCurrentStateIsOpened)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::OPENED);
    objEsm.OpenEmergencyService(pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, HandleServiceIdleStatusSetsStateByCurrentState)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::OPENING);
    objEsm.HandleServiceStatus(ServiceStatus::SERVICE_IDLE, pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::UNAVAILABLE);

    objEsm.SetState(IuMtcService::EmergencyServiceState::IDLE);
    objEsm.HandleServiceStatus(ServiceStatus::SERVICE_IDLE, pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, HandleServiceActiveStatusSetsOpenedStateIfOpening)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::OPENING);
    objEsm.HandleServiceStatus(ServiceStatus::SERVICE_ACTIVE, pMockServiceThread);
    EXPECT_EQ(objEsm.GetState(), IuMtcService::EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, HandleServiceSuspendedStatusDoesNothing)
{
    //clang-format off
    std::vector<IuMtcService::EmergencyServiceState> objStates{
            IuMtcService::EmergencyServiceState::IDLE, IuMtcService::EmergencyServiceState::OPENING,
            IuMtcService::EmergencyServiceState::OPENED,
            IuMtcService::EmergencyServiceState::UNAVAILABLE,
            IuMtcService::EmergencyServiceState::IN_CALL};
    //clang-format on

    for (IuMtcService::EmergencyServiceState eState : objStates)
    {
        objEsm.SetState(eState);
        objEsm.HandleServiceStatus(ServiceStatus::SERVICE_SUSPENDED, pMockServiceThread);
        EXPECT_EQ(objEsm.GetState(), eState);
    }
}

TEST_F(MtcEmergencyServiceManagerTest, SetSameStateDoesNotNotify)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::IDLE);

    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    objEsm.HandleServiceStatus(ServiceStatus::SERVICE_IDLE, pMockServiceThread);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenEmergencyServiceNotifiesOpenedIfStateIsOpened)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::OPENED);

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(
                    static_cast<IMS_SINT32>(IuMtcService::EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    objEsm.OpenEmergencyService(pMockServiceThread);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenEmergencyServiceNotifiesOpenedIfStateIsInCall)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::IN_CALL);

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(
                    static_cast<IMS_SINT32>(IuMtcService::EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    objEsm.OpenEmergencyService(pMockServiceThread);
}

TEST_F(MtcEmergencyServiceManagerTest, NoJniThreadDoesNotNotify)
{
    objEsm.SetState(IuMtcService::EmergencyServiceState::OPENED);

    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    objEsm.OpenEmergencyService(IMS_NULL);
}

}  // namespace android
