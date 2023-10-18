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
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

class AosEConditionTest : public ::testing::Test {
public:
    AosECondition* m_pAosECondition;
    AosBlock* m_pAosBlock;

protected:
    virtual void SetUp() override {
        MockIAosAppContext objMockIAosAppContext;

        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(objMockIAosAppContext, GetConnection())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        m_pAosECondition = new AosECondition(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(m_pAosECondition != nullptr);

        m_pAosBlock = new AosBlock(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(m_pAosBlock != nullptr);

        IAosBlock* piAosBlock = static_cast<IAosBlock*>(m_pAosBlock);
        SetAosBlock(piAosBlock);
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

    void SetAosBlock(IN IAosBlock* piBlock) { m_pAosECondition->m_piBlock = piBlock; }

    IMS_BOOL AddAosServiceListener() { return m_pAosECondition->AddAosServiceListener(); }

    IMS_BOOL RemoveAosServiceListener() { return m_pAosECondition->RemoveAosServiceListener(); }

    void Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
    {
        m_pAosECondition->Block_Changed(nType, nParam);
    }

    void ServicePhone_AosStart() { m_pAosECondition->ServicePhone_AosStart(); }
};

TEST_F(AosEConditionTest, IsReady) {
    EXPECT_TRUE(m_pAosECondition->IsReady());

    m_pAosECondition->SetBlock(BLOCK_AC_INCOMPLETED);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    EXPECT_FALSE(m_pAosECondition->IsReady());

    m_pAosECondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    EXPECT_FALSE(m_pAosECondition->IsReady());

    m_pAosECondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    EXPECT_FALSE(m_pAosECondition->IsReady());

    m_pAosECondition->ResetBlock(BLOCK_AC_INCOMPLETED);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->ResetBlock(BLOCK_AUTHENTICATION_FAILED);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->ResetBlock(BLOCK_AOS_INCOMPLETED);
    EXPECT_FALSE(m_pAosECondition->IsReady());

    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_NO_NETWORK);
    EXPECT_FALSE(m_pAosECondition->IsReady());
    m_pAosECondition->ResetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    EXPECT_TRUE(m_pAosECondition->IsReady());

    ImsList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 3);

    EXPECT_TRUE(m_pAosECondition->IsReady());
}

TEST_F(AosEConditionTest, AddAosServiceListener_ServiceNull)
{
    EXPECT_FALSE(AddAosServiceListener());
}

TEST_F(AosEConditionTest, RemoveAosServiceListener_ServiceNull)
{
    EXPECT_FALSE(RemoveAosServiceListener());
}

TEST_F(AosEConditionTest, Block_Changed)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_Changed(_)).Times(1);

    m_pAosECondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));
    Block_Changed(1, 1);
}

TEST_F(AosEConditionTest, ServicePhone_AosStart)
{
    m_pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    EXPECT_TRUE(m_pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));

    ServicePhone_AosStart();
    EXPECT_FALSE(m_pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
}