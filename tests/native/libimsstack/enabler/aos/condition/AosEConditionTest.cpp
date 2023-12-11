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

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConditionListener.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosConditionListener.h"
#include "condition/AosBlock.h"
#include "condition/AosECondition.h"
#include "provider/AosStaticProfile.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

class TestAosECondition : public AosECondition
{
public:
    inline explicit TestAosECondition(IN IAosAppContext* piAppContext) :
            AosECondition(piAppContext)
    {
    }

    // TEST : IsReady
    FRIEND_TEST(AosEConditionTest, ReturnFalseWhenIsReady);
    FRIEND_TEST(AosEConditionTest, ReturnTrueWhenIsReady);
    // TEST : AddAosServiceListener
    FRIEND_TEST(AosEConditionTest, ReturnFalseAddAosServiceListenerWhenServiceNull);
    // TEST : RemoveAosServiceListener
    FRIEND_TEST(AosEConditionTest, ReturnFalseRemoveAosServiceListenerWhenServiceNull);
    // TEST : Block_Changed
    FRIEND_TEST(AosEConditionTest, InvokeConditionChangedWhenBlockChanged);
    // TEST : ServicePhone_AosStart
    FRIEND_TEST(AosEConditionTest, ResetBlockWhenServicePhoneAosStart);

public:
    void SetAosBlock(IN IAosBlock* piBlock) { m_piBlock = piBlock; }
};

class AosEConditionTest : public ::testing::Test {
public:
    TestAosECondition* m_pAosECondition;
    AosBlock* m_pAosBlock;

protected:
    virtual void SetUp() override {
        MockIAosAppContext objMockIAosAppContext;

        ON_CALL(objMockIAosAppContext, GetSlotId()).WillByDefault(Return(0));

        const AString strValue = AString("test");
        ON_CALL(objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(strValue));

        ON_CALL(objMockIAosAppContext, GetConnection()).WillByDefault(ReturnNull());

        m_pAosECondition = new TestAosECondition(&objMockIAosAppContext);
        ASSERT_TRUE(m_pAosECondition != nullptr);

        m_pAosBlock = new AosBlock(&objMockIAosAppContext);
        ASSERT_TRUE(m_pAosBlock != nullptr);

        m_pAosECondition->SetAosBlock(m_pAosBlock);
    }

    virtual void TearDown() override {
        if (m_pAosBlock)
        {
            delete m_pAosBlock;
        }
        if (m_pAosECondition)
        {
            delete m_pAosECondition;
        }
    }
};

TEST_F(AosEConditionTest, ReturnFalseWhenIsReady)
{
    // GIVEN
    EXPECT_TRUE(m_pAosECondition->IsReady());

    m_pAosECondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosECondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosECondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosECondition->SetBlock(BLOCK_CSCALL_STARTED);
    m_pAosECondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosECondition->SetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosECondition->SetBlock(BLOCK_IMS_DISABLED);
    m_pAosECondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosECondition->SetBlock(BLOCK_POWER_OFF);
    m_pAosECondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosECondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosECondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosECondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosECondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosECondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    // WHEN
    // THEN
    EXPECT_FALSE(m_pAosECondition->IsReady());
}

TEST_F(AosEConditionTest, ReturnTrueWhenIsReady)
{
    // GIVEN
    EXPECT_TRUE(m_pAosECondition->IsReady());

    m_pAosECondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosECondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosECondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosECondition->SetBlock(BLOCK_CSCALL_STARTED);
    m_pAosECondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosECondition->SetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosECondition->SetBlock(BLOCK_IMS_DISABLED);
    m_pAosECondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosECondition->SetBlock(BLOCK_POWER_OFF);
    m_pAosECondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosECondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosECondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosECondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosECondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosECondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    EXPECT_FALSE(m_pAosECondition->IsReady());

    m_pAosECondition->ResetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosECondition->ResetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosECondition->ResetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosECondition->ResetBlock(BLOCK_CSCALL_STARTED);
    m_pAosECondition->ResetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosECondition->ResetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosECondition->ResetBlock(BLOCK_IMS_DISABLED);
    m_pAosECondition->ResetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosECondition->ResetBlock(BLOCK_POWER_OFF);
    m_pAosECondition->ResetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosECondition->ResetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosECondition->ResetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosECondition->ResetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosECondition->ResetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosECondition->ResetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    // WHEN
    // THEN
    EXPECT_TRUE(m_pAosECondition->IsReady());
}

TEST_F(AosEConditionTest, ReturnFalseAddAosServiceListenerWhenServiceNull)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pAosECondition->AddAosServiceListener();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosEConditionTest, ReturnFalseRemoveAosServiceListenerWhenServiceNull)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pAosECondition->RemoveAosServiceListener();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosEConditionTest, InvokeConditionChangedWhenBlockChanged)
{
    // GIVEN
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_Changed(_)).Times(1);

    m_pAosECondition->SetListener(&objMockIAosConditionListener);

    // WHEN
    m_pAosECondition->Block_Changed(1, 1);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosEConditionTest, ResetBlockWhenServicePhoneAosStart)
{
    // GIVEN
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));

    // WHEN
    m_pAosECondition->ServicePhone_AosStart();

    // THEN
    EXPECT_FALSE(m_pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
}
