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
#include "IIpcan.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "MtsDef.h"
#include "core/MockICoreService.h"
#include "core/MockIReference.h"
#include "core/IPageMessage.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"
#include "../include/mts/MockIMtsServiceListener.h"
#include "../../interface/aos/MockIImsAos.h"

using ::testing::Return;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_UINTP FAKE_ADDRESS = 1;

class MtsServiceTest : public ::testing::Test
{
public:
    MockICoreService objMockCoreService;
    MtsMessageController* pMtsMessageController;
    MtsDynamicLoader* pMtsDynamicLoader;
    MtsService* pMtsService;
    MockIImsAos* pMockIImsAos;
    MockIImsAos* pMockIImsEmergencyAos;

protected:
    virtual void SetUp() override
    {
        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsService = new MtsService(SLOT_ID);
        pMtsMessageController = new MtsMessageController(SLOT_ID, pMtsService, pMtsDynamicLoader);
        pMockIImsAos = new MockIImsAos();
        pMockIImsEmergencyAos = new MockIImsAos();

        pMtsService->SetIImsAos(pMockIImsAos);
        pMtsService->SetIImsEmergencyAos(pMockIImsEmergencyAos);
    }

    virtual void TearDown() override
    {
        delete pMtsMessageController;
        delete pMtsDynamicLoader;
        delete pMtsService;
    }
};

TEST_F(MtsServiceTest, GetICoreServiceReturnsNotNull)
{
    EXPECT_NE(pMtsService->GetICoreService(IMS_FALSE), nullptr);
}

TEST_F(MtsServiceTest, CoreServicePageMessageReceived)
{
    MockIMtsServiceListener* piMtsServiceListener = new MockIMtsServiceListener();
    pMtsService->SetListener(piMtsServiceListener);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(*piMtsServiceListener, NotifyMtSms(piMessage)).Times(1);

    pMtsService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
}

TEST_F(MtsServiceTest, CoreServiceReferenceReceivedDoesNothing)
{
    IReference* piReference = reinterpret_cast<IReference*>(FAKE_ADDRESS);
    pMtsService->CoreService_ReferenceReceived(&objMockCoreService, piReference);
}

TEST_F(MtsServiceTest, CoreServiceServiceClosedDoesNothing)
{
    IReasonInfo* piReasonInfo = reinterpret_cast<IReasonInfo*>(FAKE_ADDRESS);
    pMtsService->CoreService_ServiceClosed(&objMockCoreService, piReasonInfo);
}

TEST_F(MtsServiceTest, CoreServiceSessionInvitationReceivedDoesNothing)
{
    ISession* piSession = reinterpret_cast<ISession*>(FAKE_ADDRESS);
    pMtsService->CoreService_SessionInvitationReceived(&objMockCoreService, piSession);
}

TEST_F(MtsServiceTest, CoreServiceUnsolicitedNotifyReceivedDoesNothing)
{
    IMessage* piNotify = reinterpret_cast<IMessage*>(FAKE_ADDRESS);
    pMtsService->CoreService_UnsolicitedNotifyReceived(&objMockCoreService, piNotify);
}

TEST_F(MtsServiceTest, CoreServiceCapabilityQueryReceivedDoesNothing)
{
    ICapabilities* piCapabilities = reinterpret_cast<ICapabilities*>(FAKE_ADDRESS);
    pMtsService->CoreService_CapabilityQueryReceived(&objMockCoreService, piCapabilities);
}

TEST_F(MtsServiceTest, ImsAosMonitorConnected)
{
    pMtsService->ImsAosMonitor_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    EXPECT_TRUE(pMtsService->GetIMtsServiceState()->IsServiceConnected(ImsAosFeature::SMSIP));
}

TEST_F(MtsServiceTest, GetServiceStateReturnsReadyAfterAosConnected)
{
    ON_CALL(*pMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::TEXT, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_READY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsNotreadyAfterAosConnecting)
{
    pMtsService->SetIImsEmergencyAos(pMockIImsEmergencyAos);
    ON_CALL(*pMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::TEXT, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnected(ImsAosReason::NONE);
    pMtsService->ImsAos_Connecting();
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_NOTREADY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsNotreadyAfterAosDisconnected)
{
    pMtsService->SetIImsEmergencyAos(pMockIImsEmergencyAos);
    ON_CALL(*pMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::TEXT, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnected(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_NOTREADY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsReadyAfterAosDisconnecting)
{
    pMtsService->SetIImsEmergencyAos(pMockIImsEmergencyAos);
    ON_CALL(*pMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::TEXT, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnecting(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_READY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsLimitedAfterAosSuspended)
{
    pMtsService->SetIImsEmergencyAos(pMockIImsEmergencyAos);
    ON_CALL(*pMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::TEXT, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Suspended(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_LIMITED);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsReadyAfterAosResumed)
{
    pMtsService->SetIImsEmergencyAos(pMockIImsEmergencyAos);
    ON_CALL(*pMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::TEXT, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Suspended(ImsAosReason::NONE);
    pMtsService->ImsAos_Resumed();
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_READY);
}

}  // namespace android
