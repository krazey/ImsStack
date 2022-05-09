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

#include "interface/IAosAppContext.h"
#include "provider/AosLocationStarter.h"

#include "interface/MockIAosAppContext.h"

using ::testing::AnyNumber;
using ::testing::ReturnNull;

static const IMS_UINT32 DEFAULT_SHORT_UPDATE_INTERVAL = 300;

class AosLocationStarterTest : public ::testing::Test {
public:
    AosLocationStarter* pAosLocationStarter;
    MockIAosAppContext objMockIAosAppContext;

protected:
    virtual void SetUp() override {
        pAosLocationStarter = new AosLocationStarter();
        ASSERT_TRUE(pAosLocationStarter != nullptr);

        EXPECT_CALL(objMockIAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        pAosLocationStarter->Init(static_cast<IAosAppContext*>(&objMockIAosAppContext));
    }

    virtual void TearDown() override {
        if (pAosLocationStarter) {
            delete pAosLocationStarter;
        }
    }

    void SetInitialized(IN IMS_BOOL bIsInitialized) {
        pAosLocationStarter->m_bInitialized = bIsInitialized;
    }

    IMSList<IMS_UINT32> GetVolteBlockReasons() {
        return pAosLocationStarter->m_objVolteBlockReasons;
    }

    IMSList<IMS_UINT32> GetWfcBlockReasons() {
        return pAosLocationStarter->m_objWfcBlockReasons;
    }

    void ClearBlockReasons() {
        pAosLocationStarter->m_objVolteBlockReasons.Clear();
        pAosLocationStarter->m_objWfcBlockReasons.Clear();
    }
};

TEST_F(AosLocationStarterTest, Init_Initialized) {
    SetInitialized(IMS_TRUE);
    EXPECT_FALSE(pAosLocationStarter->Init(static_cast<IAosAppContext*>(&objMockIAosAppContext)));
}

TEST_F(AosLocationStarterTest, Init_NotInitialized) {
    SetInitialized(IMS_FALSE);
    EXPECT_TRUE(pAosLocationStarter->Init(static_cast<IAosAppContext*>(&objMockIAosAppContext)));
}

TEST_F(AosLocationStarterTest, SetPolicy_AddFeatureDisabled) {
    pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    EXPECT_FALSE(pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_TRUE(pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0));
}

TEST_F(AosLocationStarterTest, SetPolicy_AddFeatureNotDisabled) {
    pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_TRUE(pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_FALSE(pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0));
}

TEST_F(AosLocationStarterTest, SetPolicy_RemoveFeatureEnabled) {
    pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_TRUE(pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_TRUE(pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1));
}

TEST_F(AosLocationStarterTest, SetPolicy_RemoveFeatureNotEnabled) {
    pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    EXPECT_FALSE(pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
    EXPECT_FALSE(pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1));
}

TEST_F(AosLocationStarterTest, IsPolicyEnabled_Enabled) {
    pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 0);

    EXPECT_TRUE(pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, IsPolicyEnabled_Disabled) {
    pAosLocationStarter->SetPolicy(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY, 1);

    EXPECT_FALSE(pAosLocationStarter->IsPolicyEnabled(
            IAosLocationStarter::POLICY_START_ON_WFC_AVAILABILITY));
}

TEST_F(AosLocationStarterTest, AddBlockReason_VolteReason) {
    ClearBlockReasons();
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_AUTHENTICATION_FAILED, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 1);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_AOS_INCOMPLETED, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 2);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 3);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 4);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 5);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, 0);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 6);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);
}

TEST_F(AosLocationStarterTest, AddBlockReason_WfcReason) {
    ClearBlockReasons();
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 0);

    pAosLocationStarter->AddBlockReason(BLOCK_AUTHENTICATION_FAILED, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 1);

    pAosLocationStarter->AddBlockReason(BLOCK_AOS_INCOMPLETED, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 2);

    pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 3);

    pAosLocationStarter->AddBlockReason(BLOCK_CELLULAR_NO_NETWORK, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 4);

    pAosLocationStarter->AddBlockReason(BLOCK_WIFI_BAD_CONNECTION, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 5);

    pAosLocationStarter->AddBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, 1);
    EXPECT_EQ(GetVolteBlockReasons().GetSize(), 0);
    EXPECT_EQ(GetWfcBlockReasons().GetSize(), 6);
}

TEST_F(AosLocationStarterTest, SetUpdateInterval_ChangeIntervalParam) {
    EXPECT_FALSE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL - 3));
    EXPECT_FALSE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL - 2));
    EXPECT_FALSE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL - 1));

    EXPECT_TRUE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL));
    EXPECT_TRUE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL + 1));
    EXPECT_TRUE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL + 2));
    EXPECT_TRUE(pAosLocationStarter->SetUpdateInterval(DEFAULT_SHORT_UPDATE_INTERVAL + 3));
}

TEST_F(AosLocationStarterTest, StartLocationInfoUpdate_Start) {
    EXPECT_TRUE(pAosLocationStarter->StartLocationInfoUpdate());
}

TEST_F(AosLocationStarterTest, StopLocationInfoUpdate_StopDelayTime) {
    EXPECT_FALSE(pAosLocationStarter->StopLocationInfoUpdate());
}