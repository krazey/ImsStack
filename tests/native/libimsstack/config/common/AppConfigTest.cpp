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

#include "private/ImsRegistryLoader.h"
#include "private/TestAppConfig.h"

namespace android
{

class AppConfigTest : public ::testing::Test
{
public:
    inline AppConfigTest() :
            m_strAppId(TestAppConfig::TEST_APP_NAME),
            m_strConfig(TestAppConfig::TEST_CONFIG)
    {
    }

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}

protected:
    AString m_strAppId;
    AString m_strConfig;
};

TEST_F(AppConfigTest, IsStreamMediaTextSupported)
{
    AppConfig objAppConfig(m_strAppId);

    EXPECT_FALSE(objAppConfig.IsStreamMediaTextSupported());

    ImsRegistry objRegistry;
    ASSERT_TRUE(ImsRegistryLoader::GetRegistryFromContent(m_strAppId, m_strConfig, objRegistry));
    ASSERT_TRUE(objAppConfig.Create(objRegistry, IMS_SLOT_0));

    EXPECT_TRUE(objAppConfig.IsStreamMediaTextSupported());
}

TEST_F(AppConfigTest, AssignmentOperator)
{
    AppConfig objAppConfig(m_strAppId);

    ImsRegistry objRegistry;
    ASSERT_TRUE(ImsRegistryLoader::GetRegistryFromContent(m_strAppId, m_strConfig, objRegistry));
    ASSERT_TRUE(objAppConfig.Create(objRegistry, IMS_SLOT_0));

    AppConfig objAssignedAppConfig(AString("TestAssignApp"));

    objAssignedAppConfig = objAppConfig;

    ImsRegistry* pRegistry = objAssignedAppConfig.ToRegistry();

    ASSERT_TRUE(pRegistry != IMS_NULL);

    EXPECT_EQ(pRegistry->GetCount(), objRegistry.GetCount());

    EXPECT_TRUE(objAssignedAppConfig.Equals(m_strAppId));

    EXPECT_EQ(4096, objAssignedAppConfig.GetFramedMediaMaxSize());
    EXPECT_TRUE(objAssignedAppConfig.IsEventPackageSupported(AString("conference")));
    EXPECT_TRUE(objAssignedAppConfig.IsHeaderReadable(AString("Privacy")));
    EXPECT_TRUE(objAssignedAppConfig.IsHeaderWritable(AString("Geolocation")));

    const ImsList<CoreServiceConfig*>& objCoreServiceConfigs =
            objAssignedAppConfig.GetCoreServiceConfigs();

    EXPECT_TRUE(objCoreServiceConfigs.GetSize() == 2);
}

}  // namespace android
