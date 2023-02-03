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
#include "ImsAosReason.h"
#include "ImsTypeDef.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/MtcEmergencyServiceManager.h"
#include "helper/MockICallStateProxy.h"
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

    inline EmergencyServiceState GetState() const { return m_eState; }
    inline void SetState(IN EmergencyServiceState eState) { m_eState = eState; }
};

class MtcEmergencyServiceManagerTest : public ::testing::Test
{
public:
    inline explicit MtcEmergencyServiceManagerTest() :
            objContext(),
            objService(),
            objAosConnector(),
            objCallStateProxy(),
            objPhoneInfoService(),
            pMockServiceThread(new MockIJniMtcServiceThread()),
            pConfigurationManager(new MockIMtcConfigurationManager()),
            pConfigurationProxy(new MtcConfigurationProxy(pConfigurationManager)),
            pEsm(IMS_NULL)
    {
    }

    inline virtual ~MtcEmergencyServiceManagerTest()
    {
        delete pMockServiceThread;
        delete pEsm;
        delete pConfigurationProxy;
    }

protected:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    MockICallStateProxy objCallStateProxy;
    TestPhoneInfoService objPhoneInfoService;
    MockIJniMtcServiceThread* pMockServiceThread;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

    TestEmergencyServiceManager* pEsm;

    inline void SetUp() override
    {
        ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY))
                .WillByDefault(Return(&objAosConnector));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
                .WillByDefault(Return(0));  // Default: not in roaming

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(IMS_SLOT_0));

        // this must be done before MtcEmergencyServiceManager is created.
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(pMockServiceThread));

        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));

        pEsm = new TestEmergencyServiceManager(objContext);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }
};

TEST_F(MtcEmergencyServiceManagerTest, GetDefaultStateReturnsIdle)
{
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceDoesNotChangeStateifAosConnectorIsNull)
{
    ON_CALL(objContext, GetAosConnector(ServiceType::EMERGENCY)).WillByDefault(Return(nullptr));

    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceInvokesRegisterStart)
{
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_START));
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::OPENING);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceSetsStateOpenedIfCurrentStateIsInCall)
{
    pEsm->SetState(EmergencyServiceState::IN_CALL);
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenServiceSetsStateOpenedIfCurrentStateIsOpened)
{
    pEsm->SetState(EmergencyServiceState::OPENED);
    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosDisconnectedSetsStateByCurrentState)
{
    // TODO: IsRetryEmergencyOnImsPdnBool -> IsRetryEmergencyOnImsPdn
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_FALSE));

    pEsm->SetState(EmergencyServiceState::OPENING);
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::UNAVAILABLE);

    pEsm->SetState(EmergencyServiceState::IDLE);
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosDisconnectedDoesNotRetryOnImsPdnInIdleState)
{
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objService, GetStatus).Times(0);
    pEsm->SetState(EmergencyServiceState::IDLE);
    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, ImsAosReason::DATA_DISCONNECTED);
}

TEST_F(MtcEmergencyServiceManagerTest,
        OnAosDisconnectedRetriesOnImsPdnIfReasonIsDisconnectedInOpeningState)
{
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objService, GetStatus).Times(0);
    pEsm->SetState(EmergencyServiceState::OPENING);
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, ImsAosReason::OUT_OF_SERVICE);

    EXPECT_CALL(objCallStateProxy, AddListener(_));
    EXPECT_CALL(objCallStateProxy, RemoveListener(_));
    pEsm->SetState(EmergencyServiceState::OPENING);
    EXPECT_CALL(objService, GetStatus).WillOnce(Return(ServiceStatus::SERVICE_ACTIVE));
    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)));

    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, ImsAosReason::DATA_DISCONNECTED);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosDisconnectedSetsIdleStateIfRetryOnImsPdn)
{
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));

    pEsm->SetState(EmergencyServiceState::OPENING);

    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    ON_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)))
            .WillByDefault(Return());

    EXPECT_CALL(objCallStateProxy, AddListener(_));
    EXPECT_CALL(objCallStateProxy, RemoveListener(_));
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, ImsAosReason::DATA_DISCONNECTED);

    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosDisconnectedDoesNotRetryOnImsPdnIfRoaming)
{
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetRoamingState())
            .WillByDefault(Return(1));
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));

    pEsm->SetState(EmergencyServiceState::OPENING);
    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::UNAVAILABLE),
                    ImsAosReason::DATA_DISCONNECTED,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, ImsAosReason::DATA_DISCONNECTED);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::UNAVAILABLE);
}

TEST_F(MtcEmergencyServiceManagerTest, OnNormalRoutingClosedResetsRoutingHelper)
{
    ON_CALL(*pConfigurationManager, IsRetryEmergencyOnImsPdnBool).WillByDefault(Return(IMS_TRUE));

    pEsm->SetState(EmergencyServiceState::OPENING);

    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    ON_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)))
            .WillByDefault(Return());

    EXPECT_CALL(objCallStateProxy, AddListener(_));
    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, ImsAosReason::DATA_DISCONNECTED);
    EXPECT_CALL(objCallStateProxy, RemoveListener(_));
    pEsm->OnNormalRoutingClosed();
    EXPECT_CALL(objCallStateProxy, RemoveListener(_)).Times(0);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosConnectedSetsOpenedStateIfOpening)
{
    pEsm->SetState(EmergencyServiceState::OPENING);
    pEsm->OnAosStateChanged(objService, MtcAosState::CONNECTED, 0);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::OPENED);
}

TEST_F(MtcEmergencyServiceManagerTest, OnAosSuspendedDoesNothing)
{
    //clang-format off
    std::vector<EmergencyServiceState> objStates{EmergencyServiceState::IDLE,
            EmergencyServiceState::OPENING, EmergencyServiceState::OPENED,
            EmergencyServiceState::UNAVAILABLE, EmergencyServiceState::IN_CALL};
    //clang-format on

    for (EmergencyServiceState eState : objStates)
    {
        pEsm->SetState(eState);
        pEsm->OnAosStateChanged(objService, MtcAosState::SUSPENDED, 0);
        EXPECT_EQ(pEsm->GetState(), eState);
    }
}

TEST_F(MtcEmergencyServiceManagerTest, OnIpcanChangedDoesNothing)
{
    pEsm->OnIpcanChanged(objService, 0);
    EXPECT_EQ(pEsm->GetState(), EmergencyServiceState::IDLE);
}

TEST_F(MtcEmergencyServiceManagerTest, SetSameStateDoesNotNotify)
{
    pEsm->SetState(EmergencyServiceState::IDLE);

    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->OnAosStateChanged(objService, MtcAosState::DISCONNECTED, 0);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenNotifiesOpenedIfStateIsOpened)
{
    pEsm->SetState(EmergencyServiceState::OPENED);

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenNotifiesOpenedIfStateIsInCall)
{
    pEsm->SetState(EmergencyServiceState::IN_CALL);

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::EMERGENCY)));

    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, NoJniThreadDoesNotNotify)
{
    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));
    pEsm->SetState(EmergencyServiceState::OPENED);
    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, NoNormalServiceDoesNotNotify)
{
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));
    pEsm->SetState(EmergencyServiceState::OPENED);
    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->StartOpen(EmergencyCallRoutingPdn::EMERGENCY);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenWithImsPdnNotifiesOpendIfActiveState)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)));

    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, OpenWithImsPdnNotifiesUnavailableIfIdleState)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_IDLE));

    EXPECT_CALL(*pMockServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::UNAVAILABLE),
                    -1, static_cast<IMS_SINT32>(ServiceType::NORMAL)));

    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, NoNormalServiceDoesNotNotifyForImsPdn)
{
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));
    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
}

TEST_F(MtcEmergencyServiceManagerTest, NoJniThreadDoesNotNotifyForImsPdn)
{
    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));
    EXPECT_CALL(*pMockServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pEsm->StartOpen(EmergencyCallRoutingPdn::NORMAL);
}

}  // namespace android
