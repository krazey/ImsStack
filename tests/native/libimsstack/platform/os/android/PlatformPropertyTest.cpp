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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>

#include "PlatformProperty.h"
#include "PlatformContext.h"
#include "network/OsSocketDef.h"
#include "network/OsSocketService.h"

namespace android
{

class PlatformPropertyTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(PlatformPropertyTest, Initialize)
{
    PlatformContext* pDefaultPlaformContext = PlatformContext::GetInstance();
    EXPECT_NE(pDefaultPlaformContext, nullptr);

    EXPECT_EQ(PlatformProperty::Initialize(), IMS_TRUE);
    EXPECT_EQ(PlatformContext::GetInstance(), pDefaultPlaformContext);

    PlatformProperty::Start();
    EXPECT_EQ(OsSocketService::GetInstance()->IsStarted(), IMS_TRUE);
    PlatformProperty::Stop();
    EXPECT_EQ(OsSocketService::GetInstance()->IsStarted(), IMS_FALSE);
}

}  // namespace android
