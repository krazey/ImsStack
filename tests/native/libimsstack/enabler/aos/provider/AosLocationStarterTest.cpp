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
#include <gmock/gmock.h>

#include "ImsEventDef.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"

#include "interface/IAosAppContext.h"
#include "provider/AosLocationStarter.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgPointee;

#define DECLARE_USING(Base)         \
    using Base::Timer_TimerExpired; \
    using Base::Event_NotifyEvent;  \
    using Base::Block_Changed;      \
    using Base::Start;              \
    using Base::Stop;               \
    using Base::StartDelayTimer;    \
    using Base::StopDelayTimer;

class TestAosLocationStarter : public AosLocationStarter
{
public:
    DECLARE_USING(AosLocationStarter)

    inline explicit TestAosLocationStarter() :
            AosLocationStarter()
    {
    }

    inline void SetAppContext(IN IAosAppContext* piAosAppContext)
    {
        m_piAppContext = piAosAppContext;
    }

    inline void SetInitialized(IN IMS_BOOL bInitialized) { m_bInitialized = bInitialized; }

    inline IMS_BOOL GetWfcSetting() { return m_bWfcSetting; }

    inline ImsList<IMS_UINT32> GetVolteBlockReasons() { return m_objVolteBlockReasons; }

    inline ImsList<IMS_UINT32> GetWfcBlockReasons() { return m_objWfcBlockReasons; }

    inline ITimer* GetStopDelayTimer() { return m_piStopDelayTimer; }

    inline void SetStopDelayTimer(IN ITimer* piTimer) { m_piStopDelayTimer = piTimer; }
};

class AosLocationStarterTest : public ::testing::Test
{
public:
    TestAosLocationStarter* m_pAosLocationStarter;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockILocationInfo m_objMockILocationInfo;
    TestPhoneInfoService m_objPhoneInfoService;

protected:
    static const IMS_SINT32 OPERATION_ADD = 0;
    static const IMS_SINT32 OPERATION_REMOVE = 1;

protected:
    virtual void SetUp() override
    {
        m_objPhoneInfoService.SetLocationInfo(&m_objMockILocationInfo);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        m_pAosLocationStarter = new TestAosLocationStarter();
        ASSERT_TRUE(m_pAosLocationStarter != nullptr);

        ON_CALL(m_objMockIAosBlock, RemoveListener(_)).WillByDefault(Return());

        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(Return(&m_objMockIAosBlock));

        m_pAosLocationStarter->SetAppContext(&m_objMockIAosAppContext);
    }

    virtual void TearDown() override
    {
        if (m_pAosLocationStarter)
        {
            delete m_pAosLocationStarter;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }
};

TEST_F(AosLocationStarterTest, SucceedsSetSlotId)
{
    // GIVEN
    IMS_SINT32 nTestSlotId = 1;
    m_pAosLocationStarter->SetSlotId(nTestSlotId);

    // WHEN
    IMS_SINT32 nSlotId = m_pAosLocationStarter->GetSlotId();

    // THEN
    EXPECT_EQ(nTestSlotId, nSlotId);
}

TEST_F(AosLocationStarterTest, FailedInitializeWhenAleadyInitialized)
{
    // GIVEN
    m_pAosLocationStarter->SetInitialized(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosBlock, SetListener(_)).Times(0);

    // WHEN
    m_pAosLocationStarter->Init(&m_objMockIAosAppContext);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, SucceedsInitialize)
{
    // GIVEN
    m_pAosLocationStarter->SetInitialized(IMS_FALSE);
    m_pAosLocationStarter->SetAppContext(IMS_NULL);

    EXPECT_CALL(m_objMockIAosBlock, SetListener(_));

    // WHEN
    m_pAosLocationStarter->Init(&m_objMockIAosAppContext);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, SucceedsEnableFeature)
{
    // GIVEN
    m_pAosLocationStarter->DisableFeature(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY);
    IMS_SINT32 nOperation = OPERATION_ADD;

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, nOperation);

    // THEN
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, FailedEnableFeatureWhenAlreadyEnabled)
{
    // GIVEN
    m_pAosLocationStarter->EnableFeature(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY);
    IMS_SINT32 nOperation = OPERATION_ADD;

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, nOperation);

    // THEN
    EXPECT_FALSE(bResult);
    EXPECT_TRUE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, SucceedsDisableFeature)
{
    // GIVEN
    m_pAosLocationStarter->EnableFeature(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY);
    IMS_SINT32 nOperation = OPERATION_REMOVE;

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, nOperation);

    // THEN
    EXPECT_TRUE(bResult);
    EXPECT_FALSE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, FailedDisableFeatureWhenAlreadyDisabled)
{
    // GIVEN
    m_pAosLocationStarter->DisableFeature(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY);
    IMS_SINT32 nOperation = OPERATION_REMOVE;

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, nOperation);

    // THEN
    EXPECT_FALSE(bResult);
    EXPECT_FALSE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, ReturnsTrueWhenPolicyEnabled)
{
    // GIVEN
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosLocationStarterTest, ReturnsFalseWhenPolicyNotEnabled)
{
    // GIVEN
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosLocationStarterTest, SucceedsAddBlockReasonForVolte)
{
    // GIVEN
    m_pAosLocationStarter->GetVolteBlockReasons().Clear();
    IMS_SINT32 nType = 0;  // VoLTE

    // WHEN
    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, nType);
    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, nType);
    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE, nType);

    // THEN
    EXPECT_EQ(m_pAosLocationStarter->GetVolteBlockReasons().GetSize(), 3);
}

TEST_F(AosLocationStarterTest, SucceedsAddBlockReasonForWfc)
{
    // GIVEN
    m_pAosLocationStarter->GetWfcBlockReasons().Clear();
    IMS_SINT32 nType = 1;  // WFC

    // WHEN
    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, nType);
    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, nType);
    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON, nType);

    // THEN
    EXPECT_EQ(m_pAosLocationStarter->GetWfcBlockReasons().GetSize(), 3);
}

TEST_F(AosLocationStarterTest, ReturnsTrueWhenIntervalIsGreaterThanDefault)
{
    // GIVEN
    IMS_UINT32 TEMP_SECONDS = 100;
    IMS_UINT32 nInterval = TestAosLocationStarter::DEFAULT_SHORT_UPDATE_INTERVAL + TEMP_SECONDS;

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->SetUpdateInterval(nInterval);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosLocationStarterTest, ReturnsFalseWhenIntervalIsLessThanDefault)
{
    // GIVEN
    IMS_UINT32 TEMP_SECONDS = 100;
    IMS_UINT32 nInterval = TestAosLocationStarter::DEFAULT_SHORT_UPDATE_INTERVAL - TEMP_SECONDS;

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->SetUpdateInterval(nInterval);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosLocationStarterTest, SucceedsStartListeningForLocation)
{
    // GIVEN
    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->StartLocationInfoUpdate();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, FailedStartListeningWhenLocationIsNull)
{
    // GIVEN
    m_objPhoneInfoService.SetLocationInfo(IMS_NULL);
    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_)).Times(0);

    // WHEN
    m_pAosLocationStarter->StartLocationInfoUpdate();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, StartDelayTimerWhenStopLocationInfoUpdate)
{
    // GIVEN
    // StopListeningForLocation() should only be called once in the destructor.
    // Should not be called inside StopLocationInfoUpdate().
    EXPECT_CALL(m_objMockILocationInfo, StopListeningForLocation());

    // WHEN
    m_pAosLocationStarter->StopLocationInfoUpdate();

    // THEN
    EXPECT_NE(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
}

TEST_F(AosLocationStarterTest, SucceedsStopListeningForLocation)
{
    // GIVEN
    // StopListeningForLocation() is always called once by the destructor.
    // Therefore, this test should be called twice in total.
    EXPECT_CALL(m_objMockILocationInfo, StopListeningForLocation()).Times(2);

    // WHEN
    m_pAosLocationStarter->Stop(0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, FailedStopListeningWhenLocationIsNull)
{
    // GIVEN
    m_objPhoneInfoService.SetLocationInfo(IMS_NULL);

    EXPECT_CALL(m_objMockILocationInfo, StopListeningForLocation()).Times(0);

    // WHEN
    m_pAosLocationStarter->Stop(0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, SucceedsStopListeningWhenTimerIsExpired)
{
    // GIVEN
    // StopListeningForLocation() is always called once by the destructor.
    // Therefore, this test should be called twice in total.
    EXPECT_CALL(m_objMockILocationInfo, StopListeningForLocation()).Times(2);

    m_pAosLocationStarter->StartDelayTimer(10000);

    // WHEN
    m_pAosLocationStarter->Timer_TimerExpired(m_pAosLocationStarter->GetStopDelayTimer());

    // THEN
    EXPECT_EQ(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
}

TEST_F(AosLocationStarterTest, FailedStopListeningWhenTimerIsInvalid)
{
    // GIVEN
    // StopListeningForLocation() should only be called once in the destructor.
    // Should not be called inside StopLocationInfoUpdate().
    EXPECT_CALL(m_objMockILocationInfo, StopListeningForLocation());

    m_pAosLocationStarter->StartDelayTimer(10000);

    // WHEN
    m_pAosLocationStarter->Timer_TimerExpired(IMS_NULL);

    // THEN
    EXPECT_NE(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
}

TEST_F(AosLocationStarterTest, StartListeningWhenWfcOnWithPolicy)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_SETTING, 0);

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->Event_NotifyEvent(IMS_EVENT_WFC_SETTING_CHANGED, IMS_WFC_ON, 0);

    // THEN
    EXPECT_TRUE(m_pAosLocationStarter->GetWfcSetting());
}

TEST_F(AosLocationStarterTest, StartDelayTimerWhenWfcOffWithPolicy)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_SETTING, 0);

    // StopListeningForLocation() should only be called once in the destructor.
    EXPECT_CALL(m_objMockILocationInfo, StopListeningForLocation());

    // WHEN
    m_pAosLocationStarter->Event_NotifyEvent(IMS_EVENT_WFC_SETTING_CHANGED, IMS_WFC_OFF, 0);

    // THEN
    EXPECT_FALSE(m_pAosLocationStarter->GetWfcSetting());
    EXPECT_NE(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
}

TEST_F(AosLocationStarterTest, UpdateWfcSettingWhenWfcOnWithoutPolicy)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);

    // WHEN
    m_pAosLocationStarter->Event_NotifyEvent(IMS_EVENT_WFC_SETTING_CHANGED, IMS_WFC_ON, 0);

    // THEN
    EXPECT_TRUE(m_pAosLocationStarter->GetWfcSetting());
}

TEST_F(AosLocationStarterTest, UpdateWfcSettingWhenWfcOffWithoutPolicy)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);

    // WHEN
    m_pAosLocationStarter->Event_NotifyEvent(IMS_EVENT_WFC_SETTING_CHANGED, IMS_WFC_OFF, 0);

    // THEN
    EXPECT_FALSE(m_pAosLocationStarter->GetWfcSetting());
}

TEST_F(AosLocationStarterTest, StartWhenEnabledWfcAvailabilityPolicyAndBlockCleared)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, StartWhenEnabledWfcAvailabilityPolicyAndReasonOnlyBlocked)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, DoesNotStartWhenEnabledWfcAvailabilityPolicyAndBlocked)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_)).Times(0);

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, StartWhenEnabledVolteAvailabilityPolicyAndBlockCleared)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_VOLTE_AVAILABLE, 0);

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, DoesNotStartWhenEnabledVolteAvailabilityPolicyAndBlocked)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_VOLTE_AVAILABLE, 0);

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_)).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_)).Times(0);

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, StartWhenEnabledWfcBlockPolicyAndBlockChanged)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON, 0);

    m_pAosLocationStarter->GetWfcBlockReasons().Clear();
    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, 1);  // 1: WFC Type

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, DoesNotStartWhenEnabledWfcBlockPolicyAndSameBlocks)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON, 0);

    m_pAosLocationStarter->GetWfcBlockReasons().Clear();
    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, 1);  // 1: WFC Type

    ImsList<IMS_UINT32> objBlocks;
    objBlocks.Append(BLOCK_WIFI_BAD_CONNECTION);

    ON_CALL(m_objMockIAosBlock, GetBlockReasonsInternal(_, _))
            .WillByDefault(SetArgPointee<0>(objBlocks));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_)).Times(0);

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, StartWhenEnabledVolteBlockPolicyAndBlockChanged)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_AFTER_CHECKING_VOLTE_BLOCK_REASON, 0);

    m_pAosLocationStarter->GetWfcBlockReasons().Clear();
    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, 0);  // 0: VoLTE Type

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_));

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, DoesNotStartWhenEnabledVolteBlockPolicyAndSameBlocks)
{
    // GIVEN
    m_pAosLocationStarter->InitFeatures(TestAosLocationStarter::FEATURE_NONE);
    m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_AFTER_CHECKING_VOLTE_BLOCK_REASON, 0);

    m_pAosLocationStarter->GetWfcBlockReasons().Clear();
    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, 0);  // 0: VoLTE Type

    ImsList<IMS_UINT32> objBlocks;
    objBlocks.Append(BLOCK_CELLULAR_NO_NETWORK);

    ON_CALL(m_objMockIAosBlock, GetBlockReasonsInternal(_, _))
            .WillByDefault(SetArgPointee<0>(objBlocks));

    EXPECT_CALL(m_objMockILocationInfo, StartListeningForLocation(_)).Times(0);

    // WHEN
    m_pAosLocationStarter->Block_Changed(0, 0);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosLocationStarterTest, SucceedsStartDelayTimer)
{
    // GIVEN
    m_pAosLocationStarter->SetStopDelayTimer(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->StartDelayTimer(1000);

    // THEN
    EXPECT_NE(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
    EXPECT_TRUE(bResult);
}

TEST_F(AosLocationStarterTest, FailedStartDelayTimerWhenTimerIsExist)
{
    // GIVEN
    m_pAosLocationStarter->StartDelayTimer(1000);

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->StartDelayTimer(1000);

    // THEN
    EXPECT_NE(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
    EXPECT_FALSE(bResult);
}

TEST_F(AosLocationStarterTest, SucceedsStopDelayTimer)
{
    // GIVEN
    m_pAosLocationStarter->StartDelayTimer(1000);

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->StopDelayTimer();

    // THEN
    EXPECT_EQ(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
    EXPECT_TRUE(bResult);
}

TEST_F(AosLocationStarterTest, FailedStopDelayTimerWhenNoTimer)
{
    // GIVEN
    m_pAosLocationStarter->SetStopDelayTimer(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pAosLocationStarter->StopDelayTimer();

    // THEN
    EXPECT_EQ(nullptr, m_pAosLocationStarter->GetStopDelayTimer());
    EXPECT_FALSE(bResult);
}
