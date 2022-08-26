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
#include "IMtcService.h"
#include "MtcService.h"
#include "MockIMtcContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockISrvccStateListener.h"
#include "helper/MockMtcAosEventHandler.h"
#include "helper/MockSrvccEventHandler.h"
#include "ImsAosParameter.h"
#include "IIpcan.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcCallController.h"
#include "MockMtcEmergencyServiceManager.h"
#include "core/MockICoreService.h"
#include "core/MockIReference.h"
#include "core/MockISession.h"
#include "core/MockIMessage.h"
#include "core/IPageMessage.h"
#include "service/IReasonInfo.h"
#include "MockIMtcCallController.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class TestMtcService : public MtcService
{
public:
    inline TestMtcService(IN IMtcContext& objContext, IN ServiceType eType) :
            MtcService(objContext, eType)
    {
    }

    inline void ReplaceAosConnector(IN MtcAosConnector* pAosConnector)
    {
        delete m_pAosConnector;
        m_pAosConnector = pAosConnector;
    }

    inline void ReplaceAosEventHandler(IN MtcAosEventHandler* pAosEventHandler)
    {
        delete m_pAosEventHandler;
        m_pAosEventHandler = pAosEventHandler;
    }

    inline void ReplaceSrvccEventHandler(IN SrvccEventHandler* pSrvccEventHandler)
    {
        delete m_pSrvccEventHandler;
        m_pSrvccEventHandler = pSrvccEventHandler;
    }
};

LOCAL IMS_UINTP FAKE_ADDRESS = 1;

class MtcServiceTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockMtcEmergencyServiceManager* pMockEmergencyManager;
    MockIMtcCallManager objMockCallManager;
    MockIMtcCallController objMockCallController;
    MockICoreService objMockCoreService;
    MockMtcAosEventHandler* pMockAosEventHandler;
    MockSrvccEventHandler* pMockSrvccEventHandler;

    MtcService* pNormalMtcService;
    MtcService* pEmergencyMtcService;

protected:
    virtual void SetUp() override
    {
        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(SLOT_ID));

        pMockEmergencyManager = new MockMtcEmergencyServiceManager(objMockContext);
        ON_CALL(objMockContext, GetEmergencyServiceManager)
                .WillByDefault(Return(pMockEmergencyManager));

        ON_CALL(objMockContext, GetCallManager)
                .WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objMockContext, GetCallController)
                .WillByDefault(ReturnRef(objMockCallController));

        pNormalMtcService = CreateMtcService(ServiceType::NORMAL);
        pEmergencyMtcService = new MtcService(objMockContext, ServiceType::EMERGENCY);
    }

    virtual void TearDown() override
    {
        delete pMockEmergencyManager;
        delete pNormalMtcService;
        delete pEmergencyMtcService;
        delete pConfigurationProxy;
    }

    MtcService* CreateMtcService(IN ServiceType eType)
    {
        TestMtcService* pService = new TestMtcService(objMockContext, eType);

        //pService->ReplaceAosConnector();

        pMockAosEventHandler = new MockMtcAosEventHandler(*pService, *pConfigurationProxy);
        pService->ReplaceAosEventHandler(pMockAosEventHandler);

        pMockSrvccEventHandler = new MockSrvccEventHandler(objMockContext);
        pService->ReplaceSrvccEventHandler(pMockSrvccEventHandler);
        return pService;
    }
};

TEST_F(MtcServiceTest, GetServiceTypeReturnsNormal)
{
    EXPECT_EQ(pNormalMtcService->GetServiceType(), ServiceType::NORMAL);
}

TEST_F(MtcServiceTest, AddSrvccStateListenerThenBeingNotified)
{
    MockISrvccStateListener* pSrvccListener = new MockISrvccStateListener();
    pNormalMtcService->AddSrvccStateListener(pSrvccListener);

    EXPECT_CALL(*pSrvccListener, OnStateUpdated(SrvccState::STARTED))
            .Times(1);
    EXPECT_CALL(*pSrvccListener, OnStateUpdated(SrvccState::SUCCEEDED))
            .Times(1);
    EXPECT_CALL(*pMockAosEventHandler, SetOnSrvcc(IMS_TRUE))
            .Times(1);
    EXPECT_CALL(*pMockAosEventHandler, SetOnSrvcc(IMS_FALSE))
            .Times(1);

    pNormalMtcService->UpdateSrvccState(SrvccState::STARTED);
    pNormalMtcService->UpdateSrvccState(SrvccState::SUCCEEDED);
}

TEST_F(MtcServiceTest, RemoveSrvccStateListenerThenNotBeingNotified)
{
    MockISrvccStateListener* pSrvccListener = new MockISrvccStateListener();
    pNormalMtcService->AddSrvccStateListener(pSrvccListener);
    pNormalMtcService->RemoveSrvccStateListener(pSrvccListener);

    EXPECT_CALL(*pSrvccListener, OnStateUpdated(SrvccState::STARTED)).Times(0);

    pNormalMtcService->UpdateSrvccState(SrvccState::STARTED);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseBeforeAosConnected)
{
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseAfterAosConnecting)
{
    pNormalMtcService->ImsAos_Connecting();
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsTrueAfterAosConnected)
{
    IMS_UINT32 nFeature = ImsAosFeature::MMTEL;
    IMS_UINT32 nIpcan = IIpcan::CATEGORY_MOBILE;
    EXPECT_CALL(*pMockAosEventHandler, OnConnected(
            nFeature, nIpcan, IMS_NULL, pMockEmergencyManager, _))
            .Times(1);

    pNormalMtcService->ImsAos_Connected(nFeature, nIpcan);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsActiveReturnsTrueAfterAosDisconnecting)
{
    IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnecting(nReason, _))
            .Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnecting(nReason);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseAfterAosDisconnected)
{
    IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnected(nReason, _, IMS_NULL, pMockEmergencyManager))
            .Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(nReason);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseAfterAosSuspended)
{
    IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnSuspended(nReason, _))
            .Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Suspended(nReason);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsTrueAfterAosResumed)
{
    EXPECT_CALL(*pMockAosEventHandler, OnResumed)
            .Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Suspended(ImsAosReason::NONE);
    pNormalMtcService->ImsAos_Resumed();
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsEmergencyReturnsFalse)
{
    EXPECT_EQ(pNormalMtcService->IsEmergency(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsEmergencyReturnsTrue)
{
    EXPECT_EQ(pEmergencyMtcService->IsEmergency(), IMS_TRUE);
}

// TODO: Mocking MtcAosConnector
/*
TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsTrue)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(pEmergencyMtcService->IsWlanIpCanType(), IMS_TRUE);
}
*/

TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsFalse)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_FALSE);
}

TEST_F(MtcServiceTest, GetServiceStatusReturnsActiveAfterAosConnected)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), ServiceStatus::SERVICE_ACTIVE);
}

TEST_F(MtcServiceTest, GetServiceStatusReturnsSuspendedfterAosSuspended)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Suspended(ImsAosReason::NONE);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), ServiceStatus::SERVICE_SUSPENDED);
}

TEST_F(MtcServiceTest, GetServiceStatusReturnsIdleAfterAosDisconnected)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::NONE);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), ServiceStatus::SERVICE_IDLE);
}

TEST_F(MtcServiceTest, GetICoreServiceReturnsNotNull)
{
    EXPECT_NE(pNormalMtcService->GetICoreService(), nullptr);
    EXPECT_NE(pEmergencyMtcService->GetICoreService(), nullptr);
    EXPECT_NE(pNormalMtcService->GetICoreService(), pEmergencyMtcService->GetICoreService());
}

TEST_F(MtcServiceTest, GetAosConnectorReturnsNull)
{
    // TODO: mocking IImsAos
    EXPECT_EQ(pNormalMtcService->GetAosConnector(), nullptr);
}

TEST_F(MtcServiceTest, SetAndCheckTerminalBasedCallWaiting)
{
    // TODO: configuration
    EXPECT_TRUE(pNormalMtcService->IsTerminalBasedCallWaitingEnabled());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_FALSE, IMS_FALSE);
    EXPECT_TRUE(pNormalMtcService->IsTerminalBasedCallWaitingEnabled());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_FALSE, IMS_TRUE);
    EXPECT_TRUE(pNormalMtcService->IsTerminalBasedCallWaitingEnabled());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_TRUE, IMS_FALSE);
    EXPECT_FALSE(pNormalMtcService->IsTerminalBasedCallWaitingEnabled());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_TRUE, IMS_TRUE);
    EXPECT_TRUE(pNormalMtcService->IsTerminalBasedCallWaitingEnabled());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_FALSE, IMS_FALSE);
    EXPECT_TRUE(pNormalMtcService->IsTerminalBasedCallWaitingEnabled());
}

TEST_F(MtcServiceTest, OpenEmergencyServiceCallsEmergencyServiceManager)
{
    EXPECT_CALL(*pMockEmergencyManager, OpenEmergencyService).Times(1);
    pNormalMtcService->OpenEmergencyService();
}

TEST_F(MtcServiceTest, CoreServicePageMessageReceivedDoesNothing)
{
    ServiceStatus eOldStatus = pNormalMtcService->GetServiceStatus();
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);
    pNormalMtcService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), eOldStatus);
}

TEST_F(MtcServiceTest, CoreServiceReferenceReceivedDoesNothing)
{
    ServiceStatus eOldStatus = pNormalMtcService->GetServiceStatus();
    MockIReference objMockReference;
    pNormalMtcService->CoreService_ReferenceReceived(&objMockCoreService, &objMockReference);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), eOldStatus);
}

TEST_F(MtcServiceTest, CoreServiceServiceClosedDoesNothing)
{
    ServiceStatus eOldStatus = pNormalMtcService->GetServiceStatus();
    IReasonInfo* piReasonInfo = reinterpret_cast<IReasonInfo*>(FAKE_ADDRESS);
    pNormalMtcService->CoreService_ServiceClosed(&objMockCoreService, piReasonInfo);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), eOldStatus);
}

TEST_F(MtcServiceTest, CoreServiceCapabilityQueryReceivedCallsQueryHandler)
{
    // TODO: mock MtcCapabilityQueryHandler.
    // pNormalMtcService->CoreService_CapabilityQueryReceived(&objMockCoreService, );
}

TEST_F(MtcServiceTest, ImsAosMonitorConnectedInvokesEventHandler)
{
    IMS_UINT32 nFeature = ImsAosFeature::MMTEL;
    IMS_UINT32 nIpcan = IIpcan::CATEGORY_MOBILE;
    EXPECT_CALL(*pMockAosEventHandler, OnServiceConnected(nFeature, nIpcan))
            .Times(1);

    ServiceStatus eOldStatus = pNormalMtcService->GetServiceStatus();
    pNormalMtcService->ImsAosMonitor_Connected(nFeature, nIpcan);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), eOldStatus);
}

TEST_F(MtcServiceTest, ImsAosMonitorNotifyInvokesEventHandler)
{
    IMS_UINT32 nType = IImsAosMonitor::TYPE_IPCAN;
    IMS_UINT32 nState = IIpcan::CATEGORY_WLAN;
    EXPECT_CALL(*pMockAosEventHandler, OnEventNotify(nType, nState))
            .Times(1);

    ServiceStatus eOldStatus = pNormalMtcService->GetServiceStatus();
    pNormalMtcService->ImsAosMonitor_Notify(nType, nState);
    EXPECT_EQ(pNormalMtcService->GetServiceStatus(), eOldStatus);
}

}  // namespace android
