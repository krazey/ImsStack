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

#include "MockIOsFactory.h"
#include "MockISystem.h"
#include "PlatformContext.h"
#include "TestConfigService.h"

namespace android
{

class PlatformContextTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(PlatformContextTest, SetOsFactory)
{
    IOsFactory* piDefaultOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    EXPECT_NE(piDefaultOsFactory, nullptr);

    MockIOsFactory* pOsFactory = new MockIOsFactory();
    IOsFactory* piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(pOsFactory);
    EXPECT_EQ(piOldOsFactory, nullptr);

    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    EXPECT_NE(piDefaultOsFactory, piOsFactory);
    EXPECT_EQ(pOsFactory, piOsFactory);

    piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(IMS_NULL);
    EXPECT_EQ(piOsFactory, piOldOsFactory);

    piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
    EXPECT_EQ(piDefaultOsFactory, piOsFactory);
}

TEST_F(PlatformContextTest, SetService)
{
    PlatformService* pDefaultConfigService =
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_CONFIG);
    EXPECT_NE(pDefaultConfigService, nullptr);

    TestConfigService* pTestConfigService = new TestConfigService();
    PlatformService* pOldConfigService = PlatformContext::GetInstance()->SetService(
            PlatformContext::SERVICE_CONFIG, pTestConfigService);
    EXPECT_EQ(pOldConfigService, nullptr);

    PlatformService* pConfigService =
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_CONFIG);
    EXPECT_NE(pDefaultConfigService, pConfigService);
    EXPECT_EQ(pTestConfigService, pConfigService);

    pOldConfigService =
            PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    EXPECT_EQ(pConfigService, pOldConfigService);

    pConfigService = PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_CONFIG);
    EXPECT_EQ(pDefaultConfigService, pConfigService);

    EXPECT_EQ(PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_MAX), nullptr);
    EXPECT_EQ(PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_MAX, IMS_NULL),
            nullptr);
}

TEST_F(PlatformContextTest, SetSystem)
{
    ISystem* piDefaultSystem = PlatformContext::GetInstance()->GetSystem();
    EXPECT_NE(piDefaultSystem, nullptr);

    MockISystem* pSystem = new MockISystem();
    ISystem* piOldSystem = PlatformContext::GetInstance()->SetSystem(pSystem);
    EXPECT_EQ(piOldSystem, nullptr);

    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();
    EXPECT_NE(piDefaultSystem, piSystem);
    EXPECT_EQ(pSystem, piSystem);

    piOldSystem = PlatformContext::GetInstance()->SetSystem(IMS_NULL);
    EXPECT_EQ(piSystem, piOldSystem);

    piSystem = PlatformContext::GetInstance()->GetSystem();
    EXPECT_EQ(piDefaultSystem, piSystem);
}

}  // namespace android
