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
#include "IConfiguration.h"
#include "IIpcan.h"
#include "IMtcService.h"
#include "IPageMessage.h"
#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "ImsVector.h"
#include "JniEnablerConnector.h"
#include "MockICapabilities.h"
#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockIFeatureCaps.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMessage.h"
#include "MockIMtcCallController.h"
#include "MockIMtcContext.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MtcService.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "TestConnector.h"
#include "TestPhoneInfoService.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcCallController.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
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
    MockMtcConfigurationProxy* pConfigurationProxy;
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
    MockIMtcImsEventReceiver objEventReceiver;
    TestConnector objConnector;

    MtcService* pNormalMtcService;
    MtcService* pEmergencyMtcService;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
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

        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::
                                KEY_USE_CARRIER_SPECIFIC_REJECT_PHRASE_FOR_INCOMING_CALL_DURING_NO_REGISTRATION_BOOL))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(objMockContext, GetImsEventReceiver).WillByDefault(ReturnRef(objEventReceiver));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

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

    void SetIsCsfbAvailable(IN IMS_BOOL bIfEpsOnlyAttachBlocked, IN IMS_BOOL bEpsOnlydAttach,
            IN IMS_BOOL bInNrBlocked, IN IMS_BOOL bNr, IN IMS_BOOL bInWifiBlocked,
            IN IMS_BOOL bWlanIpCanType, IN IMS_BOOL bInRoamingBlocked, IN IMS_BOOL bRoaming,
            IN IMS_BOOL bInHomeBlocked)
    {
        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IF_EPS_ONLY_ATTACH))
                .WillByDefault(Return(bIfEpsOnlyAttachBlocked));

        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_NR))
                .WillByDefault(Return(bInNrBlocked));

        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_WIFI))
                .WillByDefault(Return(bInWifiBlocked));

        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_ROAMING))
                .WillByDefault(Return(bInRoamingBlocked));

        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_HOME))
                .WillByDefault(Return(bInHomeBlocked));

        ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
                .WillByDefault(Return(bEpsOnlydAttach ? IMS_LTE_INFO_EPS_ONLY_ATTACHED
                                                      : IMS_LTE_INFO_COMBINED_ATTACHED));

        ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
                .WillByDefault(Return(bNr ? NW_REPORT_RADIO_NR : NW_REPORT_RADIO_LTE));

        ON_CALL(*pMockAosConnector, GetIpcanType)
                .WillByDefault(
                        Return(bWlanIpCanType ? IIpcan::CATEGORY_WLAN : IIpcan::CATEGORY_MOBILE));

        ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_ROAMING_STATE))
                .WillByDefault(Return(bRoaming ? IMS_ROAMING_STATE_ON : IMS_ROAMING_STATE_OFF));
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

TEST_F(MtcServiceTest, IsNrChecksWifiFirst)
{
    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_FALSE(pNormalMtcService->IsNr());
}

TEST_F(MtcServiceTest, IsNrChecksRatType)
{
    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(pNormalMtcService->IsNr());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(pNormalMtcService->IsNr());
}

TEST_F(MtcServiceTest, IsEpsCombinedAttachChecksRatType)
{
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));
    EXPECT_FALSE(pNormalMtcService->IsEpsCombinedAttach());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));
    EXPECT_TRUE(pNormalMtcService->IsEpsCombinedAttach());
}

TEST_F(MtcServiceTest, IsEpsCombinedAttachChecksLteInfo)
{
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));
    EXPECT_FALSE(pNormalMtcService->IsEpsCombinedAttach());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_UNKNOWN));
    EXPECT_FALSE(pNormalMtcService->IsEpsCombinedAttach());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));
    EXPECT_TRUE(pNormalMtcService->IsEpsCombinedAttach());
}

TEST_F(MtcServiceTest, IsRoamingReturnsTrue)
{
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_ROAMING_STATE))
            .WillByDefault(Return(IMS_ROAMING_STATE_ON));
    EXPECT_EQ(pNormalMtcService->IsRoaming(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsRoamingReturnsFalse)
{
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_ROAMING_STATE))
            .WillByDefault(Return(IMS_ROAMING_STATE_OFF));
    EXPECT_EQ(pNormalMtcService->IsRoaming(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsTrue)
{
    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_TRUE);

    ON_CALL(*pMockAosConnector, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsFalseIfAosConnectorIsNull)
{
    MtcService objRealService(objMockContext, ServiceType::NORMAL);
    EXPECT_FALSE(objRealService.IsWlanIpCanType());
}

TEST_F(MtcServiceTest, IsWlanIpCanTypeReturnsFalse)
{
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pNormalMtcService->IsWlanIpCanType(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsFalseIfEpsOnlyAttachBlocked)
{
    SetIsCsfbAvailable(IMS_TRUE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsFalseIfInNrBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsFalseIfInWifiBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_TRUE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsFalseIfInRoamingBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE,
            IMS_TRUE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsFalseIfInHomeBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsTrueIfNoBlockConditions)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsTrueIfEpsOnlyAttachNotBlocked)
{
    SetIsCsfbAvailable(IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsTrueIfInNrNotBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsTrueIfInWifiNotBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsTrueIfInRoamingNotBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_TRUE, IMS_FALSE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcServiceTest, IsCsfbAvailableReturnsTrueIfInHomeNotBlocked)
{
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_TRUE, IMS_TRUE);
    EXPECT_EQ(pNormalMtcService->IsCsfbAvailable(), IMS_TRUE);
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

TEST_F(MtcServiceTest, ImsAosConnectedDoesNothingForCallComposerIfCoreServiceIsNull)
{
    reinterpret_cast<TestMtcService*>(pNormalMtcService)->ReplaceCoreService(IMS_NULL);

    // Meaningless codes.
    MockIFeatureCaps objFeatureCaps;
    ON_CALL(objMockCoreService, GetFeatureCaps).WillByDefault(Return(&objFeatureCaps));

    EXPECT_CALL(objFeatureCaps,
            AddFeature(AString("+g.gsma.callcomposer"), AString::ConstEmpty(), SipMethod::INVITE,
                    ISipMessage::TYPE_ANY))
            .Times(0);

    const IMS_UINT32 nFeatures = ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    pNormalMtcService->ImsAos_Connected(nFeatures, IIpcan::CATEGORY_ANY);
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

TEST_F(MtcServiceTest, GetJniServiceThreadReturnsNullIfJniEnablerIsNull)
{
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, IMS_NULL);
    EXPECT_EQ(pNormalMtcService->GetJniServiceThread(), nullptr);
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
    EXPECT_CALL(*pMockEmergencyManager, StartOpen(ServiceType::EMERGENCY)).Times(1);
    pNormalMtcService->OpenEmergencyService(ServiceType::EMERGENCY);
}

TEST_F(MtcServiceTest, StopEmergencyServiceInvokesStopOpenService)
{
    EXPECT_CALL(*pMockEmergencyManager, StopOpen(IMS_TRUE)).Times(1);
    pNormalMtcService->StopEmergencyService();
}

TEST_F(MtcServiceTest, ProcessTestCommandChangesInternalAosState)
{
    const IMS_UINT32 nFeature = ImsAosFeature::MMTEL;
    const IMS_UINT32 nIpCanType = IIpcan::CATEGORY_MOBILE;
    EXPECT_CALL(*pMockAosEventHandler, OnConnected(nFeature, nIpCanType)).Times(1);
    pNormalMtcService->ProcessTestCommand(0 /* TEST_COMMAND_AOS_CONNECTED */, nFeature, nIpCanType);

    const IMS_UINT32 nReason = ImsAosReason::NONE;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnected(nReason)).Times(1);
    pNormalMtcService->ProcessTestCommand(1 /* TEST_COMMAND_AOS_DISCONNECTED */, nReason, 0);

    pNormalMtcService->ProcessTestCommand(2 /* Not defined */, 0, 0);
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
