/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "private/AppConfig.h"
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"

namespace android
{

class ConfigurationManagerTest : public ::testing::Test
{
public:
    inline ConfigurationManagerTest() :
            m_pConfigurationManager(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pConfigurationManager = ConfigurationManager::GetInstance();
        ASSERT_TRUE(m_pConfigurationManager != IMS_NULL);

        m_pConfigurationManager->Initialize();
        m_pConfigurationManager->InitConfigs(IMS_SLOT_0);

        const ImsList<SubscriberConfig*>& objSubscriberConfigs =
                m_pConfigurationManager->GetSubscriberConfigs(IMS_SLOT_0);

        EXPECT_FALSE(objSubscriberConfigs.IsEmpty());

        EXPECT_TRUE(objSubscriberConfigs.GetAt(0)->GetId() != IMS_NULL);

        EXPECT_TRUE(m_pConfigurationManager->GetEngineConfig(IMS_SLOT_0) != IMS_NULL);
        EXPECT_TRUE(m_pConfigurationManager->GetSipConfig(IMS_SLOT_0) != IMS_NULL);
        EXPECT_TRUE(m_pConfigurationManager->GetMediaConfig(IMS_SLOT_0) != IMS_NULL);
    }

    virtual void TearDown() override { m_pConfigurationManager->DestroyConfigs(); }

public:
    ConfigurationManager* m_pConfigurationManager;
};

TEST_F(ConfigurationManagerTest, StoreAppConfig)
{
    const AString strAppId("TestAppId");

    EXPECT_FALSE(m_pConfigurationManager->IsAppConfigured(strAppId, IMS_SLOT_0));

    AppConfig* pAppConfig = new AppConfig(strAppId);

    EXPECT_EQ(
            m_pConfigurationManager->StoreAppConfig(pAppConfig, strAppId, IMS_SLOT_0), IMS_SUCCESS);

    EXPECT_TRUE(m_pConfigurationManager->IsAppConfigured(strAppId, IMS_SLOT_0));

    AStringArray objAppIds = m_pConfigurationManager->GetAppIds(IMS_SLOT_0);

    EXPECT_EQ(objAppIds.GetCount(), 1);

    EXPECT_EQ(objAppIds.GetFirstElement(), strAppId);

    // Refresh should not clear AppConfigs
    m_pConfigurationManager->RefreshConfigs(IMS_SLOT_0);

    objAppIds = m_pConfigurationManager->GetAppIds(IMS_SLOT_0);

    EXPECT_EQ(objAppIds.GetCount(), 1);

    EXPECT_EQ(objAppIds.GetFirstElement(), strAppId);

    m_pConfigurationManager->RemoveAppConfig(strAppId, IMS_SLOT_0);

    objAppIds = m_pConfigurationManager->GetAppIds(IMS_SLOT_0);

    EXPECT_EQ(objAppIds.GetCount(), 0);
}

}  // namespace android