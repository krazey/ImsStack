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
#include "interface/IAosBlock.h"
#include "interface/IAosNConfiguration.h"
#include "condition/AosServiceAvailableCellular.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosBlock.h"
#include "interface/MockIAosNConfiguration.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

#define DECLARE_USING(Base)                \
    using Base::SetBlock;                  \
    using Base::HandleRoamingChanged;      \
    using Base::HandleAirplaneModeChanged; \
    using Base::HandleVopsChanged;

class TestAosServiceAvailableCellular : public AosServiceAvailableCellular
{
public:
    DECLARE_USING(AosServiceAvailableCellular)

    inline explicit TestAosServiceAvailableCellular() :
            AosServiceAvailableCellular()
    {
    }
};

class AosServiceAvailableCellularTest : public ::testing::Test
{
public:
    TestAosServiceAvailableCellular* m_pServiceAvailableCellular;
    IAosNConfiguration* m_piOriginConfiguration;

    NiceMock<MockIAosNConfiguration> m_objMockIAosNConfiguration;
    NiceMock<MockIAosBlock> m_objMockIAosBlock;

protected:
    void SetUp() override
    {
        m_pServiceAvailableCellular = new TestAosServiceAvailableCellular();
        ASSERT_TRUE(m_pServiceAvailableCellular != nullptr);

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();

        ON_CALL(m_objMockIAosNConfiguration, IsVoLteRoamingAvailable())
                .WillByDefault(Return(IMS_FALSE));
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, 0);

        ON_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).WillByDefault(Return(IMS_FALSE));
        m_pServiceAvailableCellular->SetBlock(&m_objMockIAosBlock);
    }

    void TearDown() override
    {
        if (m_pServiceAvailableCellular)
        {
            delete m_pServiceAvailableCellular;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);
    }
};

TEST_F(AosServiceAvailableCellularTest, ShouldNotInvokeAosBlockWhenVoLteRoamingAvailable)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteRoamingAvailable())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pServiceAvailableCellular->HandleRoamingChanged(0);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldNotInvokeAosBlockWhenConfigIsNull)
{
    // GIVEN
    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL, 0);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteRoamingAvailable()).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pServiceAvailableCellular->HandleRoamingChanged(0);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldSetBlockReasonWhenRoamingStateOn)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pServiceAvailableCellular->HandleRoamingChanged(1);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldResetBlockReasonWhenRoamingStateOff)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    // WHEN
    m_pServiceAvailableCellular->HandleRoamingChanged(0);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldSetBlockReasonWhenAirplaneModeOn)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pServiceAvailableCellular->HandleAirplaneModeChanged(1);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldResetBlockReasonWhenAirplaneModeOff)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    // WHEN
    m_pServiceAvailableCellular->HandleAirplaneModeChanged(0);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldResetBlockReasonWhenVopsOn)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    // WHEN
    m_pServiceAvailableCellular->HandleVopsChanged(1);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableCellularTest, ShouldSetBlockReasonWhenVopsOff)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pServiceAvailableCellular->HandleVopsChanged(0);

    // THEN: The GIVEN condition should be met.
}
