/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "AString.h"
#include "INetworkWatcher.h"
#include "ImsAosParameter.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MockICoreService.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "SipAddress.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/IDialogInfoManager.h"
#include "dialogevent/IDialogSubscription.h"
#include "dialogevent/MockIDialogInfoManager.h"
#include "dialogevent/MockIDialogSubscription.h"
#include "dialogevent/MockMultiEndpointFactory.h"
#include "dialogevent/MultiEndpointFactory.h"
#include "dialogevent/MultiEndpointManager.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>
#include <memory>
#include <utility>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Unused;

LOCAL const SipAddress objAnyLocalAddress("sip:anyaddress");

namespace android
{

MATCHER_P(IsSameListSize, size, "")
{
    return arg.GetSize() == size;
}

class MultiEndpointManagerTest : public ::testing::Test
{
public:
    MultiEndpointManagerTest() :
            objContext(),
            objService(),
            objAosConnector(),
            objConfigService(),
            objPhoneInfoService(),
            objConfigurationProxy(),
            objCoreService(),
            objJniMtcServiceThread(),
            piDialogInfoManager(std::make_unique<MockIDialogInfoManager>()),
            piDialogSubscription(std::make_unique<MockIDialogSubscription>()),
            pMultiEndpointFactory(std::make_unique<MockMultiEndpointFactory>()),
            pMultiEndpointManager(IMS_NULL)
    {
    }

protected:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    TestConfigService objConfigService;
    TestPhoneInfoService objPhoneInfoService;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockICoreService objCoreService;
    MockIJniMtcServiceThread objJniMtcServiceThread;

    std::unique_ptr<MockIDialogInfoManager> piDialogInfoManager;
    std::unique_ptr<MockIDialogSubscription> piDialogSubscription;
    std::unique_ptr<MockMultiEndpointFactory> pMultiEndpointFactory;

    MultiEndpointManager* pMultiEndpointManager;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(IMS_SLOT_0));
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objContext, GetAosConnector(_)).WillByDefault(Return(&objAosConnector));

        ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
        ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(&objJniMtcServiceThread));

        ON_CALL(objCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objAnyLocalAddress));
    }

    virtual void TearDown() override
    {
        delete pMultiEndpointManager;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }

    // This must be called after setting the mock operation of piDialogInfoManager and
    // piDialogSubscription.
    void CreateManager(IN IMS_BOOL bConfigOn = IMS_TRUE)
    {
        ON_CALL(objConfigurationProxy, GetBoolean(ConfigVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL))
                .WillByDefault(Return(bConfigOn));

        ON_CALL(*pMultiEndpointFactory, CreateDialogInfoManager)
                .WillByDefault(Invoke(
                        [&]()
                        {
                            return std::move(piDialogInfoManager);
                        }));
        ON_CALL(*pMultiEndpointFactory, CreateDialogSubscription)
                .WillByDefault(Invoke(
                        [&]()
                        {
                            return std::move(piDialogSubscription);
                        }));

        // use std::move() to convert pMultiEndpointFactory to std::unique_ptr<MultiEndpointFactory>
        pMultiEndpointManager =
                new MultiEndpointManager(objContext, std::move(pMultiEndpointFactory));
    }
};

TEST_F(MultiEndpointManagerTest, ConstructorStartsAndDestructorStopsListeningEvents)
{
    EXPECT_CALL(objService, AddAosStateListener(_));
    EXPECT_CALL(objConfigService.GetMockCarrierConfig(), AddListener(_));
    CreateManager(IMS_FALSE);

    EXPECT_CALL(objService, RemoveAosStateListener(pMultiEndpointManager));
    EXPECT_CALL(objConfigService.GetMockCarrierConfig(), RemoveListener(pMultiEndpointManager));
}

TEST_F(MultiEndpointManagerTest, ConstructorChecksCondition)
{
    // return non audio to break the test flow.
    EXPECT_CALL(objAosConnector, GetFeatures).WillOnce(Return(ImsAosFeature::VIDEO));
    CreateManager(IMS_TRUE);
}

TEST_F(MultiEndpointManagerTest, ConstructorStartsIfConditionMet)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));

    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogInfoManager);
    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogSubscription(_, _, _));
    EXPECT_CALL(*(piDialogSubscription.get()), Subscribe).WillOnce(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);
}

TEST_F(MultiEndpointManagerTest, ConstructorDoesNotStartIfServiceIsNull)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));

    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogInfoManager).Times(0);
    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogSubscription(_, _, _)).Times(0);
    EXPECT_CALL(*(piDialogSubscription.get()), Subscribe).Times(0);
    CreateManager(IMS_TRUE);
}

TEST_F(MultiEndpointManagerTest, ConstructorDoesNotStartIfCoreServiceIsNull)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(objService, GetICoreService).WillByDefault(Return(nullptr));

    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogInfoManager).Times(0);
    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogSubscription(_, _, _)).Times(0);
    EXPECT_CALL(*(piDialogSubscription.get()), Subscribe).Times(0);
    CreateManager(IMS_TRUE);
}

TEST_F(MultiEndpointManagerTest, ConstructorStartsAndStopsIfSubscriptionFailed)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));

    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogInfoManager);
    EXPECT_CALL(*(pMultiEndpointFactory.get()), CreateDialogSubscription(_, _, _));
    EXPECT_CALL(*(piDialogSubscription.get()), Subscribe).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(*(piDialogSubscription.get()), Unsubscribe);
    CreateManager(IMS_TRUE);
}

TEST_F(MultiEndpointManagerTest, OnAosStateChangedStopsIfDisconnected)
{
    IMS_UINT32 nAnyReason = 0;
    MtcAosState eAnyState = MtcAosState::DISCONNECTED;
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    EXPECT_CALL(*(piDialogSubscription.get()), Unsubscribe);
    CreateManager(IMS_TRUE);
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::VIDEO));
    pMultiEndpointManager->OnAosStateChanged(objService, eAnyState, nAnyReason);
}

TEST_F(MultiEndpointManagerTest, IsRequiredReturnsConfigurationValue)
{
    ON_CALL(objConfigurationProxy, GetBoolean(ConfigVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(MultiEndpointManager::IsRequired(objConfigurationProxy));

    ON_CALL(objConfigurationProxy, GetBoolean(ConfigVoice::KEY_MULTIENDPOINT_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(MultiEndpointManager::IsRequired(objConfigurationProxy));
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsDefaultInfoIfNotRunning)
{
    CreateManager(IMS_FALSE);
    IMultiEndpointManager::PullingDialogInfo objInfo = pMultiEndpointManager->GetDialogInfo(0);
    EXPECT_STREQ(objInfo.strCallId.GetStr(), "");
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsDefaultInfoIfDialogListIsEmpty)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    ImsList<Dialog*> objDialogs;
    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));

    CreateManager(IMS_TRUE);
    IMultiEndpointManager::PullingDialogInfo objInfo = pMultiEndpointManager->GetDialogInfo(0);
    EXPECT_STREQ(objInfo.strCallId.GetStr(), "");
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsDefaultInfoIfNoMatchedInfoExists)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();
    const AString strOtherId("otherId");
    const AString strCallId("callId");

    MockDialog objDialog;
    objDialog.SetDialogInfo(strOtherId, strCallId);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialog);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_STREQ(objInfo.strCallId.GetStr(), "");
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsMatchedInfoIfMatchedInfoExists)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();
    const AString strOtherId("otherId");
    const AString strCallId("callId");

    MockDialog objDialogMatched;
    objDialogMatched.SetDialogInfo(strSomeId, strCallId);

    MockDialog objDialogUnMatched;
    objDialogUnMatched.SetDialogInfo(strOtherId, strCallId);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogUnMatched);
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_STREQ(objInfo.strCallId.GetStr(), strCallId.GetStr());
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsHeldDialogIfMatched)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();
    const AString strLocalUri("localUri");

    MockDialog objDialogMatched;
    ImsMap<AString, AString> objParamMap;
    objParamMap.Add("+sip.rendering", "no");
    MockTarget objTarget(objParamMap, strLocalUri);

    objDialogMatched.SetDialogInfo(strSomeId, "anyCallid");
    objDialogMatched.SetLocalUri(strLocalUri, objTarget);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_TRUE(objInfo.bHeld);

    // If it's held, it's not pullable.
    EXPECT_FALSE(objInfo.bPullable);
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsUnHeldDialogIfMatched)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();
    const AString strLocalUri("localUri");

    MockDialog objDialogMatched;
    ImsMap<AString, AString> objParamMap;
    objParamMap.Add("+sip.rendering", "yes");
    MockTarget objTarget(objParamMap, strLocalUri);

    objDialogMatched.SetDialogInfo(strSomeId, "anyCallid");
    objDialogMatched.SetLocalUri(strLocalUri, objTarget);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_FALSE(objInfo.bHeld);
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsPullableDialogIfMatched)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();

    MockDialog objDialogMatched;
    objDialogMatched.SetDialogInfo(strSomeId, "anyCallid");

    MockState objState(0, 0, Dialog::State::STATE_CONFIRMED);

    const AString strExclusive("false");
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioQuality = AUDIO_QUALITY_AMR_WB;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objMediaInfo.eVideoQuality = VIDEO_QUALITY_QVGA_PR;
    objMediaInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;
    MockExtraInfo objExtraInfo(strExclusive, objMediaInfo);
    objDialogMatched.SetState(objState);
    objDialogMatched.SetExtraInfo(objExtraInfo);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_TRUE(objInfo.bPullable);
}

TEST_F(MultiEndpointManagerTest, VideoInactiveDialogIsNotPullable)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();

    MockDialog objDialogMatched;
    objDialogMatched.SetDialogInfo(strSomeId, "anyCallid");

    MockState objState(0, 0, Dialog::State::STATE_CONFIRMED);

    const AString strExclusive("false");
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioQuality = AUDIO_QUALITY_AMR_WB;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objMediaInfo.eVideoQuality = VIDEO_QUALITY_QVGA_PR;
    objMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    MockExtraInfo objExtraInfo(strExclusive, objMediaInfo);
    objDialogMatched.SetState(objState);
    objDialogMatched.SetExtraInfo(objExtraInfo);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_FALSE(objInfo.bPullable);
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsVoipCallTypeIfMatched)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();

    MockDialog objDialogMatched;
    objDialogMatched.SetDialogInfo(strSomeId, "anyCallid");

    const AString strExclusive("true");  // to cover IsPullable
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioQuality = AUDIO_QUALITY_AMR_WB;
    objMediaInfo.eVideoQuality = VIDEO_QUALITY_NONE;
    MockExtraInfo objExtraInfo(strExclusive, objMediaInfo);
    objDialogMatched.SetExtraInfo(objExtraInfo);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_TRUE(objInfo.eCallType == CallType::VOIP);
}

TEST_F(MultiEndpointManagerTest, GetDialogInfoReturnsVtCallTypeIfMatched)
{
    const AString strSomeId("someId");
    const IMS_UINT32 nSomeId = strSomeId.GetHashCode();

    MockDialog objDialogMatched;
    objDialogMatched.SetDialogInfo(strSomeId, "anyCallid");

    const AString strExclusive("true");
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioQuality = AUDIO_QUALITY_AMR_WB;
    objMediaInfo.eVideoQuality = VIDEO_QUALITY_QVGA_PR;
    MockExtraInfo objExtraInfo(strExclusive, objMediaInfo);
    objDialogMatched.SetExtraInfo(objExtraInfo);

    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialogMatched);

    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    CreateManager(IMS_TRUE);

    IMultiEndpointManager::PullingDialogInfo objInfo =
            pMultiEndpointManager->GetDialogInfo(nSomeId);
    EXPECT_TRUE(objInfo.eCallType == CallType::VT);
}

TEST_F(MultiEndpointManagerTest, OnAosStateChangedChecksCondition)
{
    CreateManager(IMS_TRUE);
    EXPECT_CALL(objAosConnector, GetFeatures).WillOnce(Return(ImsAosFeature::VIDEO));

    IMS_UINT32 nAnyReason = 0;
    pMultiEndpointManager->OnAosStateChanged(objService, MtcAosState::CONNECTED, nAnyReason);
}

TEST_F(MultiEndpointManagerTest, OnRatChangedChecksCondition)
{
    CreateManager(IMS_TRUE);
    EXPECT_CALL(objAosConnector, GetFeatures).WillOnce(Return(ImsAosFeature::VIDEO));

    pMultiEndpointManager->OnRatChanged(objService.GetServiceType(),
            INetworkWatcher::RADIOTECH_TYPE_IWLAN, INetworkWatcher::RADIOTECH_TYPE_IWLAN);
}

TEST_F(MultiEndpointManagerTest, NotifyConfigChangedChecksCondition)
{
    CreateManager(IMS_TRUE);
    EXPECT_CALL(objAosConnector, GetFeatures).WillOnce(Return(ImsAosFeature::VIDEO));

    IMS_SINT32 nSameSlotId = IMS_SLOT_0;
    IMS_SINT32 nDifferentSlotId = IMS_SLOT_1;
    pMultiEndpointManager->CarrierConfig_NotifyConfigChanged(nSameSlotId);

    EXPECT_CALL(objAosConnector, GetFeatures).Times(0);
    pMultiEndpointManager->CarrierConfig_NotifyConfigChanged(nDifferentSlotId);
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionStartedDoesNothing)
{
    CreateManager(IMS_TRUE);
    EXPECT_FALSE(pMultiEndpointManager->IsRunning());
    pMultiEndpointManager->OnSubscriptionStarted();
    EXPECT_FALSE(pMultiEndpointManager->IsRunning());

    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));
    pMultiEndpointManager->OnAosStateChanged(objService, MtcAosState::CONNECTED, 0);

    EXPECT_TRUE(pMultiEndpointManager->IsRunning());
    pMultiEndpointManager->OnSubscriptionStarted();
    EXPECT_TRUE(pMultiEndpointManager->IsRunning());
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionStartFailedStops)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    EXPECT_CALL(*(piDialogSubscription.get()), Unsubscribe);
    CreateManager(IMS_TRUE);

    pMultiEndpointManager->OnSubscriptionStartFailed();
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionTerminatedStopsAndChecksConditionAgain)
{
    EXPECT_CALL(objAosConnector, GetFeatures)
            .Times(2)
            .WillOnce(Return(ImsAosFeature::MMTEL))
            .WillOnce(Return(ImsAosFeature::VIDEO));
    EXPECT_CALL(*(piDialogSubscription.get()), Unsubscribe);
    CreateManager(IMS_TRUE);

    pMultiEndpointManager->OnSubscriptionTerminated();
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionNotifiedNotifiesExteralCallsToJniIfUpdateSucceeded)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    ImsList<Dialog*> objDialogs;
    EXPECT_CALL(*(piDialogInfoManager.get()), GetDialogs).WillOnce(ReturnRef(objDialogs));
    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(IsSameListSize(0)));

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionNotifiedNotifiesExteralCallsToJniIfConfirmedState)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    MockState objState(0, 0, Dialog::State::STATE_CONFIRMED);
    MockDialog objDialog;
    objDialog.SetState(objState);
    const AString strNonLocalUri("someUri");
    objDialog.SetLocalUri(strNonLocalUri);
    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialog);

    EXPECT_CALL(*(piDialogInfoManager.get()), GetDialogs).WillRepeatedly(ReturnRef(objDialogs));
    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(IsSameListSize(1)));

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest,
        OnSubscriptionNotifiedNotifiesExteralCallsToJniExcludingTryingState)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    MockState objState(0, 0, Dialog::State::STATE_TRYING);
    MockDialog objDialog;
    objDialog.SetState(objState);
    const AString strNonLocalUri("someUri");
    objDialog.SetLocalUri(strNonLocalUri);
    ImsList<Dialog*> objDialogs;
    objDialogs.Append(&objDialog);

    EXPECT_CALL(*(piDialogInfoManager.get()), GetDialogs).WillRepeatedly(ReturnRef(objDialogs));
    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(IsSameListSize(0)));

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionNotifiedDoesNotNotifyIfServiceIsNull)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(_)).Times(0);  // meaningless

    CreateManager(IMS_TRUE);
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionNotifiedDoesNotNotifyIfJniIsNull)
{
    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(_)).Times(0);  // meaningless

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest, OnSubscriptionNotifiedDoesNotNotifyIfUpdateFailed)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(*(piDialogInfoManager.get()), GetDialogs).Times(0);
    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(_)).Times(0);

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest, JniExternalCallsExcludesOwnDialog)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    const AString strSomeImei("12345678901234");  // 12345678-901234-0
    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [strSomeImei](Unused, OUT AString& strImei)
                    {
                        strImei = strSomeImei;
                        return IMS_TRUE;
                    }));

    const AString strLocalOwnUri("sip:local;gr=urn:gsma:imei:12345678-901234-0");
    ImsList<Dialog*> objDialogs;
    MockDialog objDialog;
    objDialog.SetLocalUri(strLocalOwnUri);
    objDialogs.Append(&objDialog);
    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));

    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(IsSameListSize(0)));

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

TEST_F(MultiEndpointManagerTest, JniExternalCallsDoesNotExcludeNonOwnDialog)
{
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::MMTEL));
    ON_CALL(*(piDialogSubscription.get()), Subscribe).WillByDefault(Return(IMS_SUCCESS));

    const AString strAnyXml;
    ON_CALL(*(piDialogInfoManager.get()), Update(strAnyXml)).WillByDefault(Return(IMS_SUCCESS));

    const AString strSomeImei("12345678901234");  // 12345678-901234-0
    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [strSomeImei](Unused, OUT AString& strImei)
                    {
                        strImei = strSomeImei;
                        return IMS_TRUE;
                    }));

    const AString strLocalOwnUri("sip:local;gr=urn:gsma:imei:98765432-109876-0");
    ImsList<Dialog*> objDialogs;
    MockDialog objDialog;
    MockState objState(0, 0, Dialog::State::STATE_CONFIRMED);
    objDialog.SetState(objState);
    objDialog.SetLocalUri(strLocalOwnUri);
    objDialogs.Append(&objDialog);
    ON_CALL(*(piDialogInfoManager.get()), GetDialogs).WillByDefault(ReturnRef(objDialogs));

    EXPECT_CALL(objJniMtcServiceThread, OnExternalCallsChanged(IsSameListSize(1)));

    CreateManager(IMS_TRUE);
    pMultiEndpointManager->OnSubscriptionNotified(strAnyXml);
}

}  // namespace android
