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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockICarrierConfig.h"
#include "PlatformContext.h"
#include "TestConfigService.h"

#include "CarrierConfig.h"

using ::testing::_;
using ::testing::ReturnRoundRobin;

namespace android
{

class CarrierConfigTest : public ::testing::Test
{
public:
    inline CarrierConfigTest() :
            m_pConfigService(IMS_NULL)
    {
    }
    inline virtual ~CarrierConfigTest() {}

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        if (m_pConfigService != IMS_NULL)
        {
            delete m_pConfigService;
            m_pConfigService = IMS_NULL;
        }
    }

protected:
    TestConfigService* m_pConfigService;
};

TEST_F(CarrierConfigTest, IsVoLteEnabled)
{
    m_pConfigService->SetCarrierConfig(IMS_NULL);

    EXPECT_FALSE(CarrierConfig::IsVoLteEnabled(IMS_SLOT_0));

    MockICarrierConfig& objCarrierConfig = m_pConfigService->GetMockCarrierConfig();
    m_pConfigService->SetCarrierConfig(&objCarrierConfig);

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::KEY_CARRIER_VOLTE_AVAILABLE_BOOL, _))
            .WillByDefault(ReturnRoundRobin({IMS_FALSE, IMS_TRUE}));

    EXPECT_FALSE(CarrierConfig::IsVoLteEnabled(IMS_SLOT_0));
    EXPECT_TRUE(CarrierConfig::IsVoLteEnabled(IMS_SLOT_0));
}

TEST_F(CarrierConfigTest, IsVtEnabled)
{
    m_pConfigService->SetCarrierConfig(IMS_NULL);

    EXPECT_FALSE(CarrierConfig::IsVtEnabled(IMS_SLOT_0));

    MockICarrierConfig& objCarrierConfig = m_pConfigService->GetMockCarrierConfig();
    m_pConfigService->SetCarrierConfig(&objCarrierConfig);

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::KEY_CARRIER_VT_AVAILABLE_BOOL, _))
            .WillByDefault(ReturnRoundRobin({IMS_FALSE, IMS_TRUE}));

    EXPECT_FALSE(CarrierConfig::IsVtEnabled(IMS_SLOT_0));
    EXPECT_TRUE(CarrierConfig::IsVtEnabled(IMS_SLOT_0));
}

TEST_F(CarrierConfigTest, IsWfcEnabled)
{
    m_pConfigService->SetCarrierConfig(IMS_NULL);

    EXPECT_FALSE(CarrierConfig::IsWfcEnabled(IMS_SLOT_0));

    MockICarrierConfig& objCarrierConfig = m_pConfigService->GetMockCarrierConfig();
    m_pConfigService->SetCarrierConfig(&objCarrierConfig);

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, _))
            .WillByDefault(ReturnRoundRobin({IMS_FALSE, IMS_TRUE}));

    EXPECT_FALSE(CarrierConfig::IsWfcEnabled(IMS_SLOT_0));
    EXPECT_TRUE(CarrierConfig::IsWfcEnabled(IMS_SLOT_0));
}

}  // namespace android
