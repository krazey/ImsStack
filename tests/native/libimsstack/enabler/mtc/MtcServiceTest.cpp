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
#include "INetworkWatcher.h"
#include "IPageMessage.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
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
#include "MockIMtcService.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MtcService.h"
#include "PlatformContext.h"
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
#include "helper/MockIMtcNetworkWatcherListener.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/MockISrvccStateListener.h"
#include "helper/MockMtcAosEventHandler.h"
#include "helper/MockMtcNetworkWatcher.h"
#include "helper/MockSrvccStateManager.h"
#include "service/IReasonInfo.h"
#include <gtest/gtest.h>
#include <vector>

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::_;
using ::testing::Ref;
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

    inline void ReplaceNetworkWatcher(IN MtcNetworkWatcher* pNetworkWatcher)
    {
        delete m_pNetworkWatcher;
        m_pNetworkWatcher = pNetworkWatcher;
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
    TestPhoneInfoService objPhoneInfoService;
    MockIMtcImsEventReceiver objEventReceiver;
    TestConnector objConnector;
    MockMtcNetworkWatcher* pNetworkWatcher;
    MockIPassiveTimerHolder objPassiveTimerHolder;

    MtcService* pNormalMtcService;
    MtcService* pEmergencyMtcService;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(SLOT_ID));

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
        ON_CALL(objMockContext, GetPassiveTimerHolder)
                .WillByDefault(ReturnRef(objPassiveTimerHolder));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

        pNormalMtcService = CreateNormalService();
        pEmergencyMtcService = CreateEmergencyService();

        ON_CALL(objMockContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(pNormalMtcService));
        ON_CALL(objMockContext, GetServiceByType(ServiceType::EMERGENCY))
                .WillByDefault(Return(pEmergencyMtcService));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete pMockEmergencyManager;
        ON_CALL(objMockContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(IMS_NULL));
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

        pNetworkWatcher = new MockMtcNetworkWatcher(*pService, objMockContext.GetSlotId());
        pService->ReplaceNetworkWatcher(pNetworkWatcher);
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

    std::vector<SrvccState> objSrvccState{SrvccState::IDLE, SrvccState::STARTED,
            SrvccState::SUCCEEDED, SrvccState::FAILED, SrvccState::CANCELED};
    for (SrvccState eSrvccState : objSrvccState)
    {
        EXPECT_CALL(*pSrvccListener, OnSrvccStateUpdated(eSrvccState));
        pNormalMtcService->UpdateSrvccState(eSrvccState);
    }
}

TEST_F(MtcServiceTest, RemoveSrvccStateListenerThenNotBeingNotified)
{
    MockISrvccStateListener* pSrvccListener = new MockISrvccStateListener();
    pNormalMtcService->AddSrvccStateListener(pSrvccListener);
    pNormalMtcService->RemoveSrvccStateListener(pSrvccListener);

    std::vector<SrvccState> objSrvccState{SrvccState::IDLE, SrvccState::STARTED,
            SrvccState::SUCCEEDED, SrvccState::FAILED, SrvccState::CANCELED};
    for (SrvccState eSrvccState : objSrvccState)
    {
        EXPECT_CALL(*pSrvccListener, OnSrvccStateUpdated(eSrvccState)).Times(0);
        pNormalMtcService->UpdateSrvccState(eSrvccState);
    }
}

TEST_F(MtcServiceTest, UpdateSrvccStateInvokesSameApiForEmergencyType)
{
    MockIMtcService objEmergencyService;
    ON_CALL(objMockContext, GetServiceByType(ServiceType::EMERGENCY))
            .WillByDefault(Return(&objEmergencyService));

    std::vector<SrvccState> objSrvccState{SrvccState::IDLE, SrvccState::STARTED,
            SrvccState::SUCCEEDED, SrvccState::FAILED, SrvccState::CANCELED};
    for (SrvccState eSrvccState : objSrvccState)
    {
        EXPECT_CALL(objEmergencyService, UpdateSrvccState(eSrvccState));
        pNormalMtcService->UpdateSrvccState(eSrvccState);
    }
}

TEST_F(MtcServiceTest, AddNetworkWatcherListenerInvokesMtcNetworkWatcher)
{
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;
    EXPECT_CALL(*pNetworkWatcher, AddListener(Ref(objNetworkWatcherListener))).Times(1);

    pNormalMtcService->AddNetworkWatcherListener(&objNetworkWatcherListener);
}

TEST_F(MtcServiceTest, RemoveNetworkWatcherListenerInvokesMtcNetworkWatcher)
{
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;
    EXPECT_CALL(*pNetworkWatcher, RemoveListener(Ref(objNetworkWatcherListener))).Times(1);

    pNormalMtcService->RemoveNetworkWatcherListener(&objNetworkWatcherListener);
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseBeforeAosConnected)
{
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, GetRatTypeReturnsCorrectValue)
{
    EXPECT_CALL(*pNetworkWatcher, GetRatType()).Times(2).WillOnce(Return(0)).WillOnce(Return(1));

    EXPECT_EQ(0, pNormalMtcService->GetRatType());
    EXPECT_EQ(1, pNormalMtcService->GetRatType());
}

TEST_F(MtcServiceTest, GetMobileRatTypeReturnsCorrectValue)
{
    EXPECT_CALL(*pNetworkWatcher, GetMobileRatType())
            .Times(2)
            .WillOnce(Return(1))
            .WillOnce(Return(0));

    EXPECT_EQ(1, pNormalMtcService->GetMobileRatType());
    EXPECT_EQ(0, pNormalMtcService->GetMobileRatType());
}

TEST_F(MtcServiceTest, GetLastConnectedRatTypeReturnsCorrectValue)
{
    EXPECT_CALL(*pNetworkWatcher, GetLastConnectedRatType())
            .Times(2)
            .WillOnce(Return(0))
            .WillOnce(Return(1));

    EXPECT_EQ(0, pNormalMtcService->GetLastConnectedRatType());
    EXPECT_EQ(1, pNormalMtcService->GetLastConnectedRatType());
}

TEST_F(MtcServiceTest, IsActiveReturnsFalseAfterAosConnecting)
{
    pNormalMtcService->ImsAos_Connecting();
    EXPECT_EQ(pNormalMtcService->IsActive(), IMS_FALSE);
}

TEST_F(MtcServiceTest, IsActiveReturnsTrueAfterAosConnected)
{
    IMS_UINT32 nFeature = ImsAosFeature::MMTEL;
    EXPECT_CALL(*pMockAosEventHandler, OnConnected(nFeature)).Times(1);

    pNormalMtcService->ImsAos_Connected(nFeature, IIpcan::CATEGORY_MOBILE);
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
    IMS_SINT32 nDataFailureReason = 0;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnected(nReason, nDataFailureReason)).Times(1);

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(nReason, nDataFailureReason);
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

TEST_F(MtcServiceTest, IsEpsOnlyAttachChecksRatType)
{
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));
    EXPECT_FALSE(pNormalMtcService->IsEpsOnlyAttach());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));
    EXPECT_TRUE(pNormalMtcService->IsEpsOnlyAttach());
}

TEST_F(MtcServiceTest, IsEpsOnlyAttachChecksLteInfo)
{
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));
    EXPECT_FALSE(pNormalMtcService->IsEpsOnlyAttach());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_UNKNOWN));
    EXPECT_FALSE(pNormalMtcService->IsEpsOnlyAttach());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));
    EXPECT_TRUE(pNormalMtcService->IsEpsOnlyAttach());
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

TEST_F(MtcServiceTest, ImsAosConnectedUpdatesCrossSimConnected)
{
    ON_CALL(*pMockAosConnector, IsCrossSimConnected).WillByDefault(Return(IMS_TRUE));
    IMS_SINT32 nDataFailureReason = 0;

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::NONE, nDataFailureReason);

    EXPECT_TRUE(pNormalMtcService->IsCrossSimConnected());
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

TEST_F(MtcServiceTest, ImsAosConnectedNotifiesNetworkWatcher)
{
    EXPECT_CALL(*pNetworkWatcher, OnConnected(IIpcan::CATEGORY_WLAN));
    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcServiceTest, ImsAosDisconnectedNotifiesNetworkWatcher)
{
    IMS_SINT32 nDataFailureReason = 0;

    EXPECT_CALL(*pNetworkWatcher, OnDisconnected);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::POWER_OFF, nDataFailureReason);
}

TEST_F(MtcServiceTest, GetOldStatusReturnsOldStatus)
{
    IMS_SINT32 nDataFailureReason = 0;

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::NONE, nDataFailureReason);
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
    IMS_SINT32 nDataFailureReason = 0;

    pNormalMtcService->ImsAos_Connected(ImsAosFeature::MMTEL, IIpcan::CATEGORY_MOBILE);
    pNormalMtcService->ImsAos_Disconnected(ImsAosReason::NONE, nDataFailureReason);
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
    EXPECT_CALL(*pMockAosEventHandler, OnConnected(nFeature)).Times(1);
    pNormalMtcService->ProcessTestCommand(
            0 /* TestCommand::AOS_CONNECTED */, nFeature, IIpcan::CATEGORY_MOBILE);

    const IMS_UINT32 nReason = ImsAosReason::NONE;
    const IMS_SINT32 nDataFailureReason = 1;
    EXPECT_CALL(*pMockAosEventHandler, OnDisconnected(nReason, nDataFailureReason)).Times(1);
    pNormalMtcService->ProcessTestCommand(
            1 /* TestCommand::AOS_DISCONNECTED */, nReason, nDataFailureReason);
}

TEST_F(MtcServiceTest, ProcessTestCommandChangesRatType)
{
    const IMS_SINT32 eRatTypeNr = INetworkWatcher::RADIOTECH_TYPE_NR;
    const IMS_SINT32 eRatTypeLte = INetworkWatcher::RADIOTECH_TYPE_LTE;
    EXPECT_CALL(*pNetworkWatcher, UpdateMobileRat(eRatTypeNr)).Times(1);
    pNormalMtcService->ProcessTestCommand(2 /* TestCommand::RAT_CHANGED */, eRatTypeNr, 0);

    EXPECT_CALL(*pNetworkWatcher, UpdateMobileRat(eRatTypeLte)).Times(1);
    pNormalMtcService->ProcessTestCommand(2 /* TestCommand::RAT_CHANGED */, eRatTypeLte, 0);

    // To cover the default case.
    pNormalMtcService->ProcessTestCommand(3 /* Not defined */, 0, 0);
}

TEST_F(MtcServiceTest, SetAndCheckTerminalBasedCallWaiting)
{
    EXPECT_EQ(SuppStatus::UNPROVISIONED, pNormalMtcService->GetTbcwStatus());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_TRUE);
    EXPECT_EQ(SuppStatus::PROVISIONED_ENABLED, pNormalMtcService->GetTbcwStatus());

    pNormalMtcService->SetTerminalBasedCallWaiting(IMS_FALSE);
    EXPECT_EQ(SuppStatus::PROVISIONED_DISABLED, pNormalMtcService->GetTbcwStatus());
}

TEST_F(MtcServiceTest, SetAndCheckTerminalBasedTir)
{
    EXPECT_EQ(SuppStatus::UNPROVISIONED, pNormalMtcService->GetTirStatus());

    pNormalMtcService->SetTerminalBasedTir(IMS_TRUE);
    EXPECT_EQ(SuppStatus::PROVISIONED_ENABLED, pNormalMtcService->GetTirStatus());

    pNormalMtcService->SetTerminalBasedTir(IMS_FALSE);
    EXPECT_EQ(SuppStatus::PROVISIONED_DISABLED, pNormalMtcService->GetTirStatus());
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
