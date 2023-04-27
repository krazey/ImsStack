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

#include "CarrierConfig.h"
#include "Configuration.h"
#include "IIpcan.h"
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "ImsServiceConfig.h"
#include "ImsServiceConfigTypeDef.h"
#include "ImsVector.h"
#include "JniEnablerConnector.h"
#include "MockICarrierConfig.h"
#include "MockIFeatureCaps.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcCallController.h"
#include "MockIMtcContext.h"
#include "MtcService.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcCallController.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/IPageMessage.h"
#include "core/MockICapabilities.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockIReference.h"
#include "core/MockISession.h"
#include "emergency/MockIMtcEmergencyServiceManager.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIMtcAosStateListener.h"
#include "helper/MockISrvccStateListener.h"
#include "helper/MockMtcAosEventHandler.h"
#include "helper/MockSrvccStateManager.h"
#include "service/IReasonInfo.h"
#include <gtest/gtest.h>

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

    inline void ReplaceCoreService(IN ICoreService* pCoreService)
    {
        m_piCoreService->Close();
        m_piCoreService = pCoreService;
    }

    inline void ReplaceAosEventHandler(IN MtcAosEventHandler* pAosEventHandler)
    {
        delete m_pAosEventHandler;
        m_pAosEventHandler = pAosEventHandler;
    }

    inline void ReplaceSrvccStateManager(IN SrvccStateManager* pSrvccStateManager)
    {
        delete m_pSrvccStateManager;
        m_pSrvccStateManager = pSrvccStateManager;
    }

    inline void ReplaceAosConnector(IN IMtcAosConnector* pAosConnector)
    {
        delete m_pAosConnector;
        m_pAosConnector = pAosConnector;
    }
};

LOCAL IMS_UINTP FAKE_ADDRESS = 1;

class MtcServiceTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcEmergencyServiceManager* pMockEmergencyManager;
    MockIMtcCallManager objMockCallManager;
    MockIMtcCallController objMockCallController;
    MockIMtcRadioChecker objMockIMtcRadioChecker;
    MockICoreService objMockCoreService;
    MockMtcAosEventHandler* pMockAosEventHandler;
    MockSrvccStateManager* pMockSrvccStateManager;
    MockIMtcAosConnector* pMockAosConnector;
    MockIMtcAosConnector* pMockEmergencyAosConnector;
    MockIJniEnabler objMockJniEnabler;
    MockIJniMtcServiceThread objMockServiceThread;
    JniEnablerConnector* pConnector;
    TestConfigService objConfigService;
    TestPhoneInfoService objPhoneInfoService;

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

        ON_CALL(objMockContext, GetServiceByType(_)).WillByDefault(Return(nullptr));
        pMockEmergencyManager = new MockIMtcEmergencyServiceManager();
        ON_CALL(objMockContext, GetEmergencyServiceManager)
                .WillByDefault(ReturnRef(*pMockEmergencyManager));

        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objMockContext, GetCallController).WillByDefault(ReturnRef(objMockCallController));
        ON_CALL(objMockContext, GetRadioChecker).WillByDefault(ReturnRef(objMockIMtcRadioChecker));

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, &objMockJniEnabler);
        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objMockServiceThread));

        ON_CALL(*pMockConfigurationManager,
                IsUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration)
                .WillByDefault(Return(IMS_TRUE));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

        // to make Connector::Open() return valid IConnector even though MtcApp is not created
        // during the test.
        Configuration::GetInstance()->SetAppConfig(
                ImsServiceConfig::GetAppName(ImsAppId::MTC), SLOT_ID);

        pNormalMtcService = CreateNormalService();
        pEmergencyMtcService = CreateEmergencyService();
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete pMockEmergencyManager;
        delete pNormalMtcService;
        delete pEmergencyMtcService;
        delete pConnector;
        delete pConfigurationProxy;
    }

    MtcService* CreateNormalService()
    {
        TestMtcService* pService = new TestMtcService(objMockContext, ServiceType::NORMAL);

        pService->ReplaceCoreService(&objMockCoreService);

        pMockAosEventHandler = new MockMtcAosEventHandler(*pService, *pConfigurationProxy);
        pService->ReplaceAosEventHandler(pMockAosEventHandler);

        pMockSrvccStateManager = new MockSrvccStateManager();
        pService->ReplaceSrvccStateManager(pMockSrvccStateManager);

        pMockAosConnector = new MockIMtcAosConnector();
        pService->ReplaceAosConnector(pMockAosConnector);
        return pService;
    }

    MtcService* CreateEmergencyService()
    {
        TestMtcService* pService = new TestMtcService(objMockContext, ServiceType::EMERGENCY);

        pMockEmergencyAosConnector = new MockIMtcAosConnector();
        pService->ReplaceAosConnector(pMockEmergencyAosConnector);
        return pService;
    }
};

TEST_F(MtcServiceTest, GetServiceTypeReturnsNormal)
{
    EXPECT_EQ(pNormalMtcService->GetServiceType(), ServiceType::NORMAL);
}

TEST_F(MtcServiceTest, AddAosStateListenerInvokesMtcAosEventHandler)
{
    MockIMtcAosStateListener objListener;
    EXPECT_CALL(*pMockAosEventHandler, AddListener(&objListener)).Times(1);

    pNormalMtcService->AddAosStateListener(&objListener);
}

TEST_F(MtcServiceTest, RemoveAosStateListenerInvokesMtcAosEventHandler)
{
    MockIMtcAosStateListener objListener;
    EXPECT_CALL(*pMockAosEventHandler, RemoveListener(&objListener)).Times(1);

    pNormalMtcService->RemoveAosStateListener(&objListener);
}

TEST_F(MtcServiceTest, AddSrvccStateListenerThenBeingNotified)
{
    MockISrvccStateListener* pSrvccListener = new MockISrvccStateListener();
    pNormalMtcService->AddSrvccStateListener(pSrvccListener);

    EXPECT_CALL(*pSrvccListener, OnSrvccStateUpdated(SrvccState::STARTED)).Times(1);
    EXPECT_CALL(*pSrvccListener, OnSrvccStateUpdated(SrvccState::SUCCEEDED)).Times(1);

    pNormalMtcService->UpdateSrvccState(SrvccState::STARTED);
    pNormalMtcService->UpdateSrvccState(SrvccState::SUCCEEDED);
}

TEST_F(MtcServiceTest, RemoveSrvccStateListenerThenNotBeingNotified)
{
    MockISrvccStateListener* pSrvccListener = new MockISrvccStateListener();
    pNormalMtcService->AddSrvccStateListener(pSrvccListener);
    pNormalMtcService->RemoveSrvccStateListener(pSrvccListener);

    EXPECT_CALL(*pSrvccListener, OnSrvccStateUpdated(SrvccState::STARTED)).Times(0);

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
    EXPECT_CALL(*pMockAosEventHandler, OnConnected(nFeature, nIpcan)).Times(1);

    pNormalMtcService->ImsAos_Connected(nFeature, nIpcan);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsActiveReturnsTrueAfterAosDisconnecting)
{
    IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnecting(nReason)).Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnecting(nReason);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseAfterAosDisconnected)
{
    ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(nullptr));
    IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnected(nReason)).Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(nReason);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseAfterAosSuspended)
{
    IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnSuspended(nReason)).Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Suspended(nReason);
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsTrueAfterAosResumed)
{
    EXPECT_CALL(*pMockAosEventHandler, OnResumed).Times(1);

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

TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsTrue)
{
    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_TRUE);

    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsNrChecksWifiFirst)
{
    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_FALSE(pNormalMtcService->IsNr());
}

TEST_F(MtcServiceTest, IsNrChecksRadioInfo)
{
    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(pNormalMtcService->IsNr());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(pNormalMtcService->IsNr());
}

TEST_F(MtcServiceTest, ImsAosConnectedNormalServiceInvokesSetReady)
{
    EXPECT_CALL(*pMockAosConnector, SetReady(IMS_TRUE, ImsAosService::MTC)).Times(1);
    // by destructor
    EXPECT_CALL(*pMockAosConnector, SetReady(IMS_FALSE, ImsAosService::MTC)).Times(1);
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcServiceTest, ImsAosConnectedEmergencyServiceInvokesSetReady)
{
    EXPECT_CALL(*pMockEmergencyAosConnector, SetReady(IMS_TRUE, ImsAosService::EMERGENCY_MTC))
            .Times(1);
    // by destructor
    EXPECT_CALL(*pMockEmergencyAosConnector, SetReady(IMS_FALSE, ImsAosService::EMERGENCY_MTC))
            .Times(1);
    pEmergencyMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtcServiceTest, ImsAosConnectedWithCallComposerFeatureAddsFeature)
{
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objMockCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    EXPECT_CALL(objFeatureCaps,
            AddFeature(AString("+g.gsma.callcomposer"), AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_ANY))
            .Times(1);
    const IMS_UINT32 nFeatures = ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    pNormalMtcService->ImsAos_Connected(nFeatures, IIpcan::CATEGORY_ANY);
    pNormalMtcService->ImsAos_Connected(nFeatures, IIpcan::CATEGORY_ANY);
}

TEST_F(MtcServiceTest, ImsAosConnectedWithoutCallComposerFeatureDoesNotRemovesFeatureIfNotAdded)
{
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objMockCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    EXPECT_CALL(objFeatureCaps, AddFeature(AString("+g.gsma.callcomposer"), _, _, _)).Times(0);
    const IMS_UINT32 nEmptyFeatures = 0;
    pNormalMtcService->ImsAos_Connected(nEmptyFeatures, IIpcan::CATEGORY_ANY);
}

TEST_F(MtcServiceTest, ImsAosConnectedWithoutCallComposerFeatureRemovesFeatureIfAddedBefore)
{
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objMockCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    const IMS_UINT32 nFeatures = ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    pNormalMtcService->ImsAos_Connected(nFeatures, IIpcan::CATEGORY_ANY);

    EXPECT_CALL(objFeatureCaps,
            RemoveFeature(AString("+g.gsma.callcomposer"), AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_ANY))
            .Times(1);
    const IMS_UINT32 nEmptyFeatures = 0;
    pNormalMtcService->ImsAos_Connected(nEmptyFeatures, IIpcan::CATEGORY_ANY);
}

TEST_F(MtcServiceTest, ImsAosConnectedWithCallComposerFeatureDoesNothingForEmergencyService)
{
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objMockCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    EXPECT_CALL(objFeatureCaps, AddFeature(AString("+g.gsma.callcomposer"), _, _, _)).Times(0);
    const IMS_UINT32 nFeatures = ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    pEmergencyMtcService->ImsAos_Connected(nFeatures, IIpcan::CATEGORY_ANY);
}

TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsFalse)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_FALSE);
}

TEST_F(MtcServiceTest, GetOldStatusReturnsOldStatus)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::NONE);
    EXPECT_EQ(pNormalMtcService->GetOldStatus(), ServiceStatus::SERVICE_ACTIVE);
}

TEST_F(MtcServiceTest, GetStatusReturnsActiveAfterAosConnected)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pNormalMtcService->GetStatus(), ServiceStatus::SERVICE_ACTIVE);
}

TEST_F(MtcServiceTest, GetStatusReturnsSuspendedAfterAosSuspended)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Suspended(ImsAosReason::NONE);
    EXPECT_EQ(pNormalMtcService->GetStatus(), ServiceStatus::SERVICE_SUSPENDED);
}

TEST_F(MtcServiceTest, GetStatusReturnsIdleAfterAosDisconnected)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::NONE);
    EXPECT_EQ(pNormalMtcService->GetStatus(), ServiceStatus::SERVICE_IDLE);
}

TEST_F(MtcServiceTest, GetICoreServiceReturnsNotNull)
{
    EXPECT_NE(pNormalMtcService->GetICoreService(), nullptr);
    EXPECT_NE(pEmergencyMtcService->GetICoreService(), nullptr);
    EXPECT_NE(pNormalMtcService->GetICoreService(), pEmergencyMtcService->GetICoreService());
}

TEST_F(MtcServiceTest, GetAosConnectorReturnsNull)
{
    EXPECT_EQ(pNormalMtcService->GetAosConnector(), pMockAosConnector);
}

TEST_F(MtcServiceTest, GetJniServiceThreadReturnsThread)
{
    EXPECT_EQ(pNormalMtcService->GetJniServiceThread(), &objMockServiceThread);
}

TEST_F(MtcServiceTest, GetSrvccStateReturnsValueFromSrvccStateManager)
{
    ON_CALL(*pMockSrvccStateManager, GetState).WillByDefault(Return(SrvccState::STARTED));

    EXPECT_EQ(pNormalMtcService->GetSrvccState(), SrvccState::STARTED);
}

TEST_F(MtcServiceTest, SetAndCheckTerminalBasedCallWaiting)
{
    EXPECT_EQ(TbcwStatus::UNPROVISIONED, pNormalMtcService->GetTbcwStatus());

    ImsVector<IMS_SINT32> objTbcw;
    objTbcw.Add(0);
    ImsVector<IMS_SINT32> objNoTbcw;

    EXPECT_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSs::KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
            .WillOnce(Return(objNoTbcw))
            .WillOnce(Return(objTbcw))
            .WillOnce(Return(objNoTbcw))
            .WillOnce(Return(objTbcw));

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_FALSE);
    EXPECT_EQ(TbcwStatus::UNPROVISIONED, pNormalMtcService->GetTbcwStatus());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_FALSE);
    EXPECT_EQ(TbcwStatus::PROVISIONED_DISABLED, pNormalMtcService->GetTbcwStatus());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_TRUE);
    EXPECT_EQ(TbcwStatus::PROVISIONED_DISABLED, pNormalMtcService->GetTbcwStatus());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_TRUE);
    EXPECT_EQ(TbcwStatus::PROVISIONED_ENABLED, pNormalMtcService->GetTbcwStatus());
}

TEST_F(MtcServiceTest, OpenEmergencyServiceCallsEmergencyServiceManager)
{
    EXPECT_CALL(*pMockEmergencyManager, StartOpen(EmergencyCallRoutingPdn::EMERGENCY)).Times(1);
    pNormalMtcService->OpenEmergencyService(EmergencyCallRoutingPdn::EMERGENCY);
}

TEST_F(MtcServiceTest, NotifyJniEnablerSetDoesNothing)
{
    pNormalMtcService->NotifyJniEnablerSet();
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

TEST_F(MtcServiceTest, CoreServicePageMessageReceivedDoesNothing)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);
    pNormalMtcService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

TEST_F(MtcServiceTest, CoreServiceReferenceReceivedDoesNothing)
{
    MockIReference objMockReference;
    pNormalMtcService->CoreService_ReferenceReceived(&objMockCoreService, &objMockReference);
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

TEST_F(MtcServiceTest, CoreServiceServiceClosedDoesNothing)
{
    IReasonInfo* piReasonInfo = reinterpret_cast<IReasonInfo*>(FAKE_ADDRESS);
    pNormalMtcService->CoreService_ServiceClosed(&objMockCoreService, piReasonInfo);
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

TEST_F(MtcServiceTest, CoreServiceUnsolicitedNotifyReceivedDoesNothing)
{
    MockIMessage objMessage;
    pNormalMtcService->CoreService_UnsolicitedNotifyReceived(&objMockCoreService, &objMessage);
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

TEST_F(MtcServiceTest, CoreServiceSessionInvitationReceivedInvokesHandleIncoming)
{
    MockISession objISession;
    MockICoreService objICoreService;

    EXPECT_CALL(objMockCallController, HandleIncoming(pNormalMtcService, &objISession));

    pNormalMtcService->CoreService_SessionInvitationReceived(&objICoreService, &objISession);
}

TEST_F(MtcServiceTest, CoreServiceCapabilityQueryReceivedInvokesHandleIncomingCapabilityQuery)
{
    MockICapabilities objICapa;
    MockICoreService objICoreService;

    // TODO: cannot check
    pNormalMtcService->CoreService_CapabilityQueryReceived(&objICoreService, &objICapa);
}

TEST_F(MtcServiceTest, ImsAosMonitorConnectedInvokesEventHandler)
{
    IMS_UINT32 nFeature = ImsAosFeature::MMTEL;
    IMS_UINT32 nIpcan = IIpcan::CATEGORY_MOBILE;
    EXPECT_CALL(*pMockAosEventHandler, OnServiceConnected(nFeature, nIpcan)).Times(1);

    pNormalMtcService->ImsAosMonitor_Connected(nFeature, nIpcan);
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

TEST_F(MtcServiceTest, ImsAosMonitorNotifyInvokesEventHandler)
{
    IMS_UINT32 nType = IImsAosMonitor::TYPE_IPCAN;
    IMS_UINT32 nState = IIpcan::CATEGORY_WLAN;
    EXPECT_CALL(*pMockAosEventHandler, OnEventNotify(nType, nState)).Times(1);

    pNormalMtcService->ImsAosMonitor_Notify(nType, nState);
    EXPECT_EQ(pNormalMtcService->GetStatus(), pNormalMtcService->GetOldStatus());
}

}  // namespace android
