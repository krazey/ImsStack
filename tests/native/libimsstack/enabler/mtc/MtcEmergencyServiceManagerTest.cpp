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
#include "MockIMtcService.h"
#include "MtcEmergencyServiceManager.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::Return;

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
            objService(),
            objAosConnector(),
            pMockServiceThread(new MockIJniMtcServiceThread()),
            pEsm(IMS_NULL)
    {
    }

    inline virtual ~MtcEmergencyServiceManagerTest()
    {
        delete pMockServiceThread;
        delete pEsm;
    }

protected:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    MockIJniMtcServiceThread* pMockServiceThread;

    TestEmergencyServiceManager* pEsm;

    inline void SetUp() override
    {
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));

        // this must be done before MtcEmergencyServiceManager is created.
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(pMockServiceThread));

        pEsm = new TestEmergencyServiceManager(objContext);
    }
};

TEST_F(MtcEmergencyServiceManagerTest, GetDefaultStateReturnsIdle)
{
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceDoesNotChangeStateifAosConnectorIsNull)
{
    ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY)).WillByDefault(Return(nullptr));

    pEsm->OpenEmergencyService();
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceInvokesRegisterStart)
{
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_START));
    pEsm->OpenEmergencyService();
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::OPENING);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceSetsStateOpenedIfCurrentStateIsInCall)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::IN_CALL);
    pEsm->OpenEmergencyService();
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceSetsStateOpenedIfCurrentStateIsOpened)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::OPENED);
    pEsm->OpenEmergencyService();
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosDisconnectedSetsStateByCurrentState)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::OPENING);
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::UNAVAILABLE);

    pEsm->SetState(IuMtcService::EmergencyServiceState::IDLE);
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosConnectedSetsOpenedStateIfOpening)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::OPENING);
    pEsm->OnAosStateChanged(objService, MtcAosState::CONNECTED, 0);
    EXPECT_EQ(pEsm->GetState(), IuMtcService::EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosSuspendedDoesNothing)
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
        pEsm->SetState(eState);
        pEsm->OnAosStateChanged(objService, MtcAosState::SUSPENDED, 0);
        EXPECT_EQ(pEsm->GetState(), eState);
    }
}

TEST_F(MtcEmergencyServiceManagerTest, SetSameStateDoesNotNotify)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::IDLE);

    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenEmergencyServiceNotifiesOpenedIfStateIsOpened)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::OPENED);

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(
                    static_cast<IMS_SINT32>(IuMtcService::EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    pEsm->OpenEmergencyService();
}

TEST_F(MtcEmergencyServiceManagerTest, OpenEmergencyServiceNotifiesOpenedIfStateIsInCall)
{
    pEsm->SetState(IuMtcService::EmergencyServiceState::IN_CALL);

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(
                    static_cast<IMS_SINT32>(IuMtcService::EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    pEsm->OpenEmergencyService();
}

TEST_F(MtcEmergencyServiceManagerTest, NoJniThreadDoesNotNotify)
{
    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));

    pEsm->SetState(IuMtcService::EmergencyServiceState::OPENED);

    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->OpenEmergencyService();
}

}  // namespace android
