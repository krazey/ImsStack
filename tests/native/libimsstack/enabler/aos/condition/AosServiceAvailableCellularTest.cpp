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
using ::testing::Return;

class TestAosServiceAvailableCellular : public AosServiceAvailableCellular
{
public:
    inline explicit TestAosServiceAvailableCellular() :
            AosServiceAvailableCellular()
    {
    }

    FRIEND_TEST(AosServiceAvailableCellularTest, HandleRoamingChanged_ReturnByConfig);
    FRIEND_TEST(AosServiceAvailableCellularTest, HandleRoamingChanged_RoamingStateTrue);
    FRIEND_TEST(AosServiceAvailableCellularTest, HandleRoamingChanged_RoamingStateFalse);
    FRIEND_TEST(AosServiceAvailableCellularTest, HandleAirplaneModeChanged_AirplaneModeTrue);
    FRIEND_TEST(AosServiceAvailableCellularTest, HandleAirplaneModeChanged_AirplaneModeFalse);
    FRIEND_TEST(AosServiceAvailableCellularTest, HandleVopsChange_VopsSupport);
    FRIEND_TEST(AosServiceAvailableCellularTest, HandleVopsChange_VopsNotSupport);
};

class AosServiceAvailableCellularTest : public ::testing::Test
{
public:
    TestAosServiceAvailableCellular* m_pAosServiceAvailableCellular;
    IAosNConfiguration* m_piOriginConfiguration;

protected:
    virtual void SetUp() override
    {
        m_pAosServiceAvailableCellular = new TestAosServiceAvailableCellular();
        ASSERT_TRUE(m_pAosServiceAvailableCellular != nullptr);

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    }

    virtual void TearDown() override
    {
        if (m_pAosServiceAvailableCellular)
        {
            delete m_pAosServiceAvailableCellular;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);
    }
};

TEST_F(AosServiceAvailableCellularTest, HandleRoamingChanged_ReturnByConfig)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(&objMockIAosNConfiguration, 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteRoamingAvailable())
            .WillRepeatedly(Return(IMS_TRUE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableCellular->HandleRoamingChanged(0);
    m_pAosServiceAvailableCellular->HandleRoamingChanged(1);
}

TEST_F(AosServiceAvailableCellularTest, HandleRoamingChanged_RoamingStateTrue)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(&objMockIAosNConfiguration, 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteRoamingAvailable())
            .WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableCellular->HandleRoamingChanged(1);
}

TEST_F(AosServiceAvailableCellularTest, HandleRoamingChanged_RoamingStateFalse)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(&objMockIAosNConfiguration, 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteRoamingAvailable())
            .WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableCellular->HandleRoamingChanged(0);
}

TEST_F(AosServiceAvailableCellularTest, HandleAirplaneModeChanged_AirplaneModeTrue)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(&objMockIAosNConfiguration, 0);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableCellular->HandleAirplaneModeChanged(1);
}

TEST_F(AosServiceAvailableCellularTest, HandleAirplaneModeChanged_AirplaneModeFalse)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(&objMockIAosNConfiguration, 0);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;

    m_pAosServiceAvailableCellular->HandleAirplaneModeChanged(0);
}

TEST_F(AosServiceAvailableCellularTest, HandleVopsChange_VopsSupport)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;
    m_pAosServiceAvailableCellular->HandleVopsChanged(1);
}

TEST_F(AosServiceAvailableCellularTest, HandleVopsChange_VopsNotSupport)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosServiceAvailableCellular->m_piBlock = &objMockIAosBlock;
    m_pAosServiceAvailableCellular->HandleVopsChanged(0);
}
