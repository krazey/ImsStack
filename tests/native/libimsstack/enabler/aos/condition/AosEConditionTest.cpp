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
#include "condition/AosECondition.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosBlock.h"

using ::testing::_;
using ::testing::An;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");

#define DECLARE_USING(Base)                           \
    using Base::AddAosServiceListener;                \
    using Base::RemoveAosServiceListener;             \
    using Base::Block_Changed;                        \
    using Base::ServicePhone_AosStart;                \
    using Base::ServicePhone_LocationInfoChanged;     \
    using Base::ServicePhone_PhoneNumberStateChanged; \
    using Base::ServicePhone_PlmnChanged;             \
    using Base::ServicePhone_PowerOff;                \
    using Base::ServicePhone_AllowedNetworkTypesChanged;

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
    AosEConditionTest() :
            m_pAosECondition(IMS_NULL),
            m_piAosService(IMS_NULL)
    {
    }

    TestAosECondition* m_pAosECondition;

    IAosService* m_piAosService;

    NiceMock<MockIAosAppContext> m_objMockIAosAppContext;
    NiceMock<MockIAosService> m_objMockIAosService;
    NiceMock<MockIAosBlock> m_objMockIAosBlock;

protected:
    void SetUp() override
    {
        ReplaceOriginWithMock();

        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetConnection()).WillByDefault(ReturnNull());

        m_pAosECondition = new TestAosECondition(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosECondition != nullptr);

        m_pAosECondition->SetAosBlock(&m_objMockIAosBlock);
    }

    void TearDown() override
    {
        RestoreOriginInstance();

        delete m_pAosECondition;
    }

    void ReplaceOriginWithMock()
    {
        m_piAosService = AosProvider::GetInstance()->GetService();

        AosProvider::GetInstance()->SetService(&m_objMockIAosService);
    }

    void RestoreOriginInstance() const { AosProvider::GetInstance()->SetService(m_piAosService); }
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
    ON_CALL(m_objMockIAosBlock, IsCleared(_)).WillByDefault(Return(IMS_TRUE));

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
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_AOS_INCOMPLETED, _));

    // WHEN
    m_pAosECondition->ServicePhone_AosStart();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosEConditionTest, ShouldNotDoAnythingWhenLocationInfoChanged)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosECondition->ServicePhone_LocationInfoChanged(LocationInfo::CHANGED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosEConditionTest, ShouldNotDoAnythingWhenPhoneNumberStateChanged)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosECondition->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::RETRY_SUCCESS);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosEConditionTest, ShouldNotDoAnythingWhenPlmnChanged)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosECondition->ServicePhone_PlmnChanged(AString("TEST_PLMN"));

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosEConditionTest, ShouldNotDoAnythingWhenPowerOff)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosECondition->ServicePhone_PowerOff();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosEConditionTest, ShouldNotDoAnythingWhenAllowedNetworkTypesChanged)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosECondition->ServicePhone_AllowedNetworkTypesChanged(0);

    // THEN: The GIVEN condition should be met.
}
