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
#include "MtcApp.h"
#include "IMtcService.h"
#include "ect/IEctManager.h"
#include "MtcEmergencyServiceManager.h"
#include "IMtcImsEventReceiver.h"
#include "call/IMtcCallManager.h"
#include "helper/ICallStateProxy.h"
#include "helper/OperationAsyncRunner.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "IMtcCallController.h"
#include "conferencecall/IConferenceManager.h"
#include "configuration/MtcConfigurationManager.h"
#include "ServiceUtil.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

namespace android
{

class TestMtcApp : public MtcApp
{
public:
    TestMtcApp() :
            MtcApp(SLOT_ID)
    {
    }
    virtual ~TestMtcApp() {}

    IMS_SINT32 GetServiceCount() { return m_lstServices.GetSize(); }
};

class MtcAppTest : public ::testing::Test
{
public:
    MtcApp* pMtcApp;

protected:
    virtual void SetUp() override { pMtcApp = new MtcApp(SLOT_ID); }

    virtual void TearDown() override { delete pMtcApp; }
};

TEST_F(MtcAppTest, Constructor)
{
    ASSERT_NE(pMtcApp, nullptr);
}

TEST_F(MtcAppTest, GetSlotId)
{
    EXPECT_EQ(pMtcApp->GetSlotId(), SLOT_ID);
}

TEST_F(MtcAppTest, CreateNormalServiceAfterStart)
{
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::NORMAL);
    ASSERT_EQ(piService, nullptr);

    pMtcApp->Start();
    piService = pMtcApp->GetServiceByType(ServiceType::NORMAL);
    ASSERT_NE(piService, nullptr);
}

TEST_F(MtcAppTest, CreateEmergencyServiceAfterStart)
{
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::EMERGENCY);
    ASSERT_EQ(piService, nullptr);

    pMtcApp->Start();
    piService = pMtcApp->GetServiceByType(ServiceType::EMERGENCY);
    ASSERT_NE(piService, nullptr);
}

TEST_F(MtcAppTest, ReturnNullForGetServiceForUnknownType)
{
    pMtcApp->Start();
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::UNKNOWN);
    ASSERT_EQ(piService, nullptr);
}

TEST_F(MtcAppTest, StopWithoutStartNoCrash)
{
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, GetNormalServiceAfterStop)
{
    TestMtcApp* pMtcApp = new TestMtcApp();
    pMtcApp->Start();
    pMtcApp->Stop();
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::NORMAL);
    ASSERT_EQ(piService, nullptr);
}

TEST_F(MtcAppTest, GetEmergencyServiceAfterStop)
{
    TestMtcApp* pMtcApp = new TestMtcApp();
    pMtcApp->Start();
    pMtcApp->Stop();
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::EMERGENCY);
    ASSERT_EQ(piService, nullptr);
}

TEST_F(MtcAppTest, StartTwiceWithoutStopNoDuplicatedServiceCreation)
{
    TestMtcApp* pMtcApp = new TestMtcApp();
    pMtcApp->Start();
    IMS_SINT32 nFirstServiceCount = pMtcApp->GetServiceCount();
    pMtcApp->Start();
    IMS_SINT32 nSecondServiceCount = pMtcApp->GetServiceCount();
    EXPECT_EQ(nFirstServiceCount, nSecondServiceCount);
}

TEST_F(MtcAppTest, CreateEctManagerOnlyOnceWhenFirstGetterIsCalled)
{
    IEctManager* pFirstManager = pMtcApp->GetEctManager();
    IEctManager* pSecondManager = pMtcApp->GetEctManager();
    EXPECT_EQ(pFirstManager, pSecondManager);
}

TEST_F(MtcAppTest, CreateEmergencyManagerOnlyOnceWhenFirstGetterIsCalled)
{
    MtcEmergencyServiceManager* pFirstManager = pMtcApp->GetEmergencyServiceManager();
    MtcEmergencyServiceManager* pSecondManager = pMtcApp->GetEmergencyServiceManager();
    EXPECT_EQ(pFirstManager, pSecondManager);
}

TEST_F(MtcAppTest, GetDialingPlanAfterConstructor)
{
    IMtcDialingPlan* piDialingPlan = &pMtcApp->GetDialingPlan();
    ASSERT_NE(piDialingPlan, nullptr);
}

TEST_F(MtcAppTest, GetCallControllerAfterConstructor)
{
    IMtcCallController* piCallController = &pMtcApp->GetCallController();
    ASSERT_NE(piCallController, nullptr);
}

TEST_F(MtcAppTest, GetCallManagerAfterConstructor)
{
    IMtcCallManager* piCallManager = &pMtcApp->GetCallManager();
    ASSERT_NE(piCallManager, nullptr);
}

TEST_F(MtcAppTest, GetConfigurationProxyAfterConstructor)
{
    MtcConfigurationProxy* piConfigProxy = &pMtcApp->GetConfigurationProxy();
    ASSERT_NE(piConfigProxy, nullptr);
}

TEST_F(MtcAppTest, GetCallStateProxyAfterConstructor)
{
    ICallStateProxy* piCallStateProxy = &pMtcApp->GetCallStateProxy();
    ASSERT_NE(piCallStateProxy, nullptr);
}

TEST_F(MtcAppTest, GetEventReceiverAfterConstructor)
{
    IMtcImsEventReceiver* piEventReceiver = &pMtcApp->GetImsEventReceiver();
    ASSERT_NE(piEventReceiver, nullptr);
}

TEST_F(MtcAppTest, GetSipInterfaceFactoryAfterConstructor)
{
    IMtcSipInterfaceFactory* piSipInterfaceFactory = &pMtcApp->GetSipInterfaceFactory();
    ASSERT_NE(piSipInterfaceFactory, nullptr);
}

TEST_F(MtcAppTest, GetConferenceManagerAfterConstructor)
{
    IConferenceManager* piConferenceManager = &pMtcApp->GetConferenceManager();
    ASSERT_NE(piConferenceManager, nullptr);
}

TEST_F(MtcAppTest, GetAsyncRunnerAfterConstructor)
{

    OperationAsyncRunner* pRunner = pMtcApp->GetAsyncRunner([&]()
            {
                // do nothing
            });
    ASSERT_NE(pRunner, nullptr);
    ImsMessage objMessage(0, 0, 0);
    pRunner->OnMessage(objMessage); // to delete pRunner
}

TEST_F(MtcAppTest, IsWifiTestModeReturnsSameValueOfUtilService)
{
    IMS_BOOL bWifiTestMode = UtilService::GetUtilService()->GetPrivateProperty()->GetPersistentInt(
            ImsPrivateProperties::Persistent::KEY_WIFI_TEST, 0) == 1;
    EXPECT_EQ(bWifiTestMode, pMtcApp->IsWifiTestMode());
}

}  // namespace android
