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

#include "../../interface/aos/MockIAosService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConditionListener.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "condition/AosBlock.h"
#include "condition/AosECondition.h"
#include "provider/AosProvider.h"

using ::testing::_;
using ::testing::An;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");

#define DECLARE_USING(Base)               \
    using Base::AddAosServiceListener;    \
    using Base::RemoveAosServiceListener; \
    using Base::Block_Changed;            \
    using Base::ServicePhone_AosStart;

class TestAosECondition : public AosECondition
{
public:
    DECLARE_USING(AosECondition)

    inline explicit TestAosECondition(IN IAosAppContext* piAppContext) :
            AosECondition(piAppContext)
    {
    }

    inline void SetAosBlock(IN IAosBlock* piBlock) { m_piBlock = piBlock; }
};

class AosEConditionTest : public ::testing::Test {
public:
    TestAosECondition* m_pAosECondition;
    AosBlock* m_pAosBlock;

    IAosService* m_piAosService;

    NiceMock<MockIAosAppContext> m_objMockIAosAppContext;
    NiceMock<MockIAosService> m_objMockIAosService;

protected:
    void SetUp() override
    {
        ReplaceOriginWithMock();

        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetConnection()).WillByDefault(ReturnNull());

        m_pAosECondition = new TestAosECondition(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosECondition != nullptr);

        m_pAosBlock = new AosBlock(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosBlock != nullptr);

        m_pAosECondition->SetAosBlock(m_pAosBlock);
    }

    void TearDown() override
    {
        RestoreOriginInstance();

        if (m_pAosBlock)
        {
            delete m_pAosBlock;
        }
        if (m_pAosECondition)
        {
            delete m_pAosECondition;
        }
    }

    void ReplaceOriginWithMock()
    {
        m_piAosService = AosProvider::GetInstance()->GetService();

        AosProvider::GetInstance()->SetService(&m_objMockIAosService);
    }

    void RestoreOriginInstance() { AosProvider::GetInstance()->SetService(m_piAosService); }
};

TEST_F(AosEConditionTest, ReturnsFalseWhenIsNotReady)
{
    // GIVEN
    m_pAosECondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosECondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosECondition->SetBlock(BLOCK_AOS_INCOMPLETED);

    m_pAosECondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosECondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);

    m_pAosECondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosECondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosECondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);

    // WHEN
    IMS_BOOL bResult = m_pAosECondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosEConditionTest, ReturnsTrueWhenIsReady)
{
    // GIVEN
    m_pAosBlock->ClearAllBlockReasons();

    // WHEN
    IMS_BOOL bResult = m_pAosECondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosEConditionTest, SucceedsAddAosServiceListener)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosServicePhoneListener*>()));

    // WHEN
    m_pAosECondition->AddAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosEConditionTest, FailsAddAosServiceListenerWhenAosServiceIsNull)
{
    // GIVEN
    AosProvider::GetInstance()->SetService(IMS_NULL);

    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosServicePhoneListener*>())).Times(0);

    // WHEN
    m_pAosECondition->AddAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosEConditionTest, SucceedsRemoveAosServiceListener)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosServicePhoneListener*>()));

    // WHEN
    m_pAosECondition->RemoveAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosEConditionTest, FailsRemoveAosServiceListenerWhenAosServiceIsNull)
{
    // GIVEN
    AosProvider::GetInstance()->SetService(IMS_NULL);

    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosServicePhoneListener*>())).Times(0);

    // WHEN
    m_pAosECondition->RemoveAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosEConditionTest, InvokeConditionChangedWhenBlockChanged)
{
    // GIVEN
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_Changed(_));

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
