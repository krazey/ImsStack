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

#include "IMtcCallController.h"
#include "IMtcImsEventReceiver.h"
#include "IMtcService.h"
#include "MockICarrierConfig.h"
#include "MtcApp.h"
#include "PlatformContext.h"
#include "ServiceUtil.h"
#include "TestConfigService.h"
#include "call/IMtcCallManager.h"
#include "call/radio/IMtcRadioChecker.h"
#include "conferencecall/IConferenceManager.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "ect/IEctManager.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include "helper/ICallStateProxy.h"
#include "helper/ILastComeFirstServedHelper.h"
#include "helper/OperationAsyncRunner.h"
#include "utility/IMessageUtils.h"
#include <gtest/gtest.h>

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::_;
using ::testing::Return;

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

    IMS_SINT32 GetServiceCount() const { return m_lstServices.GetSize(); }
};

class MtcAppTest : public ::testing::Test
{
protected:
    MtcApp* pMtcApp;

    PlatformService* m_pOldConfigService;
    TestConfigService* m_pConfigService;
    MockICarrierConfig m_objMockICarrierConfig;

    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();
        m_pConfigService->SetCarrierConfig(&m_objMockICarrierConfig);
        m_pOldConfigService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);
        pMtcApp = new MtcApp(SLOT_ID);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pOldConfigService);
        delete m_pConfigService;

        delete pMtcApp;
    }
};

TEST_F(MtcAppTest, Constructor)
{
    ASSERT_NE(pMtcApp, nullptr);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, GetSlotId)
{
    EXPECT_EQ(pMtcApp->GetSlotId(), SLOT_ID);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, GetSubscriberConfig)
{
    EXPECT_EQ(pMtcApp->GetSubscriberConfig(), nullptr);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, CreateNormalServiceAfterStart)
{
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::NORMAL);
    ASSERT_EQ(piService, nullptr);

    pMtcApp->Start();
    piService = pMtcApp->GetServiceByType(ServiceType::NORMAL);
    ASSERT_NE(piService, nullptr);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, CreateEmergencyServiceAfterStart)
{
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::EMERGENCY);
    ASSERT_EQ(piService, nullptr);

    pMtcApp->Start();
    piService = pMtcApp->GetServiceByType(ServiceType::EMERGENCY);
    ASSERT_NE(piService, nullptr);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, ReturnNullForGetServiceForUnknownType)
{
    pMtcApp->Start();
    IMtcService* piService = pMtcApp->GetServiceByType(ServiceType::UNKNOWN);
    ASSERT_EQ(piService, nullptr);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, StartDoesNotCreateMultiEndPointManagerIfNotSupported)
{
    ON_CALL(m_objMockICarrierConfig, GetBoolean(ConfigVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    pMtcApp->Start();
    EXPECT_EQ(pMtcApp->GetMultiEndpointManager(), nullptr);
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, StartCreatesMultiEndpointManagerIfSupported)
{
    ON_CALL(m_objMockICarrierConfig, GetBoolean(ConfigVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));

    pMtcApp->Start();
    ASSERT_NE(pMtcApp->GetMultiEndpointManager(), nullptr);
    pMtcApp->Stop();
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
    pMtcApp->Stop();
}

TEST_F(MtcAppTest, CreateEctManagerOnlyOnceWhenFirstGetterIsCalled)
{
    IEctManager* pFirstManager = &pMtcApp->GetEctManager();
    IEctManager* pSecondManager = &pMtcApp->GetEctManager();
    EXPECT_EQ(pFirstManager, pSecondManager);
}

TEST_F(MtcAppTest, CreateEmergencyManagerOnlyOnceWhenFirstGetterIsCalled)
{
    pMtcApp->Start();  // to have MtcService
    IMtcEmergencyServiceManager* pFirstManager = &pMtcApp->GetEmergencyServiceManager();
    IMtcEmergencyServiceManager* pSecondManager = &pMtcApp->GetEmergencyServiceManager();
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

TEST_F(MtcAppTest, GetRadioCheckerAfterConstructor)
{
    IMtcRadioChecker* piTrafficChecker = &pMtcApp->GetRadioChecker();
    ASSERT_NE(piTrafficChecker, nullptr);
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

TEST_F(MtcAppTest, RunAsyncOperationAfterConstructor)
{
    // null case
    pMtcApp->RunAsyncOperation(IMS_NULL, nullptr);
    pMtcApp->RunAsyncOperation(IMS_NULL,
            []()
            {
            });

    // non-null case
    pMtcApp->RunAsyncOperation(this,
            []()
            {
            });

    // Here, BaseThread is null so OperationAsyncRunner is deleted immediately
    // so pRunner is Dangling pointer.
    // Let's test the normal case within OperationAsyncRunner with some SetUps
}

TEST_F(MtcAppTest, GetMessageUtilsAfterConstructor)
{
    IMessageUtils* piMessageUtils = &pMtcApp->GetMessageUtils();
    ASSERT_NE(piMessageUtils, nullptr);
}

TEST_F(MtcAppTest, CreateLastComeFirstServedHelperOnlyOnceWhenFirstGetterIsCalled)
{
    ILastComeFirstServedHelper* pFirstHelper = &pMtcApp->GetLastComeFirstServedHelper();
    ILastComeFirstServedHelper* pSecondHelper = &pMtcApp->GetLastComeFirstServedHelper();
    EXPECT_EQ(pFirstHelper, pSecondHelper);
}

TEST_F(MtcAppTest, IsWifiTestModeReturnsSameValueOfUtilService)
{
    IMS_BOOL bWifiTestMode = UtilService::GetUtilService()->GetPrivateProperty()->GetPersistentInt(
                                     ImsPrivateProperties::Persistent::KEY_WIFI_TEST, 0) == 1;
    EXPECT_EQ(bWifiTestMode, pMtcApp->IsWifiTestMode());
}

TEST_F(MtcAppTest, GetAosConnectorReturnsNullIfNotStarted)
{
    EXPECT_EQ(pMtcApp->GetAosConnector(ServiceType::NORMAL), nullptr);
}

TEST_F(MtcAppTest, GetAosConnectorReturnsNullIfStarted)
{
    // It's always null in the test since the AoS module(IImsAos) is not ready.
    pMtcApp->Start();
    EXPECT_EQ(pMtcApp->GetAosConnector(ServiceType::NORMAL), nullptr);
    EXPECT_EQ(pMtcApp->GetAosConnector(ServiceType::EMERGENCY), nullptr);
    pMtcApp->Stop();
}

}  // namespace android
