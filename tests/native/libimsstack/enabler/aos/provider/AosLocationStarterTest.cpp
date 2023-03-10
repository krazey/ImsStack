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

#include "interface/IAosAppContext.h"
#include "provider/AosLocationStarter.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::ReturnNull;

static const IMS_UINT32 DEFAULT_SHORT_UPDATE_INTERVAL = 300;
static const IMS_UINT32 TIMER_STOP_DELAY = 0;

class AosLocationStarterTest : public ::testing::Test
{
public:
    AosLocationStarter* m_pAosLocationStarter;
    MockIAosAppContext m_objMockIAosAppContext;

protected:
    virtual void SetUp() override
    {
        m_pAosLocationStarter = new AosLocationStarter();
        ASSERT_TRUE(m_pAosLocationStarter != nullptr);

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        m_pAosLocationStarter->Init(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
    }

    virtual void TearDown() override
    {
        if (m_pAosLocationStarter)
        {
            delete m_pAosLocationStarter;
        }
    }

    void SetInitialized(IN IMS_BOOL bIsInitialized)
    {
        m_pAosLocationStarter->m_bInitialized = bIsInitialized;
    }

    ImsList<IMS_UINT32> GetVolteBlockReasons()
    {
        return m_pAosLocationStarter->m_objVolteBlockReasons;
    }

    ImsList<IMS_UINT32> GetWfcBlockReasons() { return m_pAosLocationStarter->m_objWfcBlockReasons; }

    void ClearBlockReasons()
    {
        m_pAosLocationStarter->m_objVolteBlockReasons.Clear();
        m_pAosLocationStarter->m_objWfcBlockReasons.Clear();
    }

    ITimer* GetStopDeleyTimer() { return m_pAosLocationStarter->m_piStopDelayTimer; }

    IMS_BOOL GetWfcSetting() { return m_pAosLocationStarter->m_bWfcSetting; }

    IAosBlock* GetBlock() { return m_pAosLocationStarter->m_piBlock; }

    void SetBlock(IN IAosBlock* piBlock) { m_pAosLocationStarter->m_piBlock = piBlock; }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        m_pAosLocationStarter->StartTimer(nType, nDuration);
    }

    void Timer_TimerExpired(IN ITimer* piTimer)
    {
        m_pAosLocationStarter->Timer_TimerExpired(piTimer);
    }

    void Event_NotifyEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
    {
        m_pAosLocationStarter->Event_NotifyEvent(nEvent, nWParam, nLParam);
    }

    void Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
    {
        m_pAosLocationStarter->Block_Changed(nType, nParam);
    }
};

TEST_F(AosLocationStarterTest, SlotId_GetSet)
{
    m_pAosLocationStarter->SetSlotId(0);
    EXPECT_EQ(m_pAosLocationStarter->GetSlotId(), 0);

    m_pAosLocationStarter->SetSlotId(1);
    EXPECT_EQ(m_pAosLocationStarter->GetSlotId(), 1);

    m_pAosLocationStarter->SetSlotId(0);
    EXPECT_EQ(m_pAosLocationStarter->GetSlotId(), 0);
}

TEST_F(AosLocationStarterTest, Init_Initialized)
{
    SetInitialized(IMS_TRUE);
    EXPECT_FALSE(
            m_pAosLocationStarter->Init(static_cast<IAosAppContext*>(&m_objMockIAosAppContext)));
}

TEST_F(AosLocationStarterTest, Init_NotInitialized)
{
    SetInitialized(IMS_FALSE);
    EXPECT_TRUE(
            m_pAosLocationStarter->Init(static_cast<IAosAppContext*>(&m_objMockIAosAppContext)));
}

TEST_F(AosLocationStarterTest, SetPolicy_AddFeatureDisabled)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    EXPECT_FALSE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_TRUE(m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0));
}

TEST_F(AosLocationStarterTest, SetPolicy_AddFeatureNotDisabled)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_TRUE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_FALSE(m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0));
}

TEST_F(AosLocationStarterTest, SetPolicy_RemoveFeatureEnabled)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_TRUE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_TRUE(m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1));
}

TEST_F(AosLocationStarterTest, SetPolicy_RemoveFeatureNotEnabled)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    EXPECT_FALSE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_FALSE(m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1));
}

TEST_F(AosLocationStarterTest, IsPolicyEnabled_Enabled)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_TRUE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, IsPolicyEnabled_Disabled)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    EXPECT_FALSE(m_pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, AddBlockReason_VolteReason)
{
    ClearBlockReasons();
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_AUTHENTICATION_FAILED, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 1);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_AOS_INCOMPLETED, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 2);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 3);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 4);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 5);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 6);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);
}

TEST_F(AosLocationStarterTest, AddBlockReason_WfcReason)
{
    ClearBlockReasons();
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    m_pAosLocationStarter->AddBlockReason(BLOCK_AUTHENTICATION_FAILED, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 1);

    m_pAosLocationStarter->AddBlockReason(BLOCK_AOS_INCOMPLETED, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 2);

    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 3);

    m_pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 4);

    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 5);

    m_pAosLocationStarter->AddBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 6);
}

TEST_F(AosLocationStarterTest, SetUpdateInterval_ChangeIntervalParam)
{
    EXPECT_FALSE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL - 3));
    EXPECT_FALSE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL - 2));
    EXPECT_FALSE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL - 1));

    EXPECT_TRUE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL));
    EXPECT_TRUE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL + 1));
    EXPECT_TRUE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL + 2));
    EXPECT_TRUE(m_pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL + 3));
}

TEST_F(AosLocationStarterTest, StartLocationInfoUpdate_Start)
{
    EXPECT_TRUE(m_pAosLocationStarter->StartLocationInfoUpdate());
}

TEST_F(AosLocationStarterTest, StopLocationInfoUpdate_StopDelayTime)
{
    EXPECT_FALSE(m_pAosLocationStarter->StopLocationInfoUpdate());
}

TEST_F(AosLocationStarterTest, Timer_TimerExpired)
{
    IMS_UINT32 nDelayTime = 10;
    StartTimer(TIMER_STOP_DELAY, nDelayTime * 1000);
    EXPECT_NE(GetStopDeleyTimer(), nullptr);

    Timer_TimerExpired(GetStopDeleyTimer());
    EXPECT_EQ(GetStopDeleyTimer(), nullptr);
}

TEST_F(AosLocationStarterTest, Event_NotifyEvent_WfcOn)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_SETTING, 1);

    Event_NotifyEvent(IMS_EVENT_WFC_SETTING_CHANGED, IMS_WFC_ON, 0);
    EXPECT_TRUE(GetWfcSetting());
}

TEST_F(AosLocationStarterTest, Event_NotifyEvent_WfcOff)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_SETTING, 1);

    Event_NotifyEvent(IMS_EVENT_WFC_SETTING_CHANGED, IMS_WFC_OFF, 0);
    EXPECT_FALSE(GetWfcSetting());
}

TEST_F(AosLocationStarterTest, Block_Changed_PolicyStartAfterCheckingVolteBlockReason)
{
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);
    m_pAosLocationStarter->SetPolicy(IAosLocationStarter::POLICY_START_ON_VOLTE_AVAILABLE, 1);
    m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON, 1);
    m_pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_AFTER_CHECKING_VOLTE_BLOCK_REASON, 0);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);

    IAosBlock* piOriginBlock = GetBlock();
    SetBlock(static_cast<IAosBlock*>(&objMockIAosBlock));
    Block_Changed(0, 0);

    SetBlock(piOriginBlock);
}