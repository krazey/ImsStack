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
#include "ServiceContext.h"

#include "MockIServiceContext.h"

using ::testing::Return;

namespace android
{

class EngineTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EngineTest, GetConfiguration)
{
    MockIServiceContext objServiceContext;
    ServiceContext::GetInstance()->SetServiceContext(&objServiceContext);
    EXPECT_CALL(objServiceContext, GetConfiguration()).Times(1).WillOnce(Return(nullptr));

    EXPECT_EQ(Engine::GetConfiguration(), nullptr);

    ServiceContext::GetInstance()->SetServiceContext(IMS_NULL);
}

}  // namespace android
