/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "Engine.h"
#include "RegistrationContext.h"

#include "MockIRegistrationContext.h"
#include "TestEngine.h"

using ::testing::Return;

namespace android
{

class EngineTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

protected:
    TestEngine m_objEngine;
};

TEST_F(EngineTest, GetConfiguration)
{
    MockIConfiguration& objConfiguration = m_objEngine.GetMockConfiguration();
    EXPECT_EQ(Engine::GetConfiguration(), &objConfiguration);
}

TEST_F(EngineTest, GetRegistrationManager)
{
    MockIRegistrationContext objRegistrationContext;
    RegistrationContext::GetInstance()->SetRegistrationContext(&objRegistrationContext);
    EXPECT_CALL(objRegistrationContext, GetRegistrationManager())
            .Times(1)
            .WillOnce(Return(nullptr));

    EXPECT_EQ(Engine::GetRegistrationManager(), nullptr);

    RegistrationContext::GetInstance()->SetRegistrationContext(IMS_NULL);
}

}  // namespace android
