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
#include "provider/AosStaticConfig.h"

TEST(AosStaticConfigTest, GetInstance)
{
    static AosStaticConfig* s_pConfig = IMS_NULL;
    s_pConfig = AosStaticConfig::GetInstance();
    EXPECT_NE(s_pConfig, nullptr);

    s_pConfig->~AosStaticConfig();
    s_pConfig = IMS_NULL;
    EXPECT_EQ(s_pConfig, nullptr);

    s_pConfig = AosStaticConfig::GetInstance();
    EXPECT_NE(s_pConfig, nullptr);
}

TEST(AosStaticConfigTest, Create)
{
    EXPECT_TRUE(AosStaticConfig::GetInstance()->Create());
}

TEST(AosStaticConfigTest, GetProfile_ReturnNull)
{
    EXPECT_EQ(AosStaticConfig::GetInstance()->GetProfile(
                      AString("ims.app.test"), AString("ims.service.test")),
            nullptr);
}

TEST(AosStaticConfigTest, GetProfile_ReturnProfile)
{
    EXPECT_NE(AosStaticConfig::GetInstance()->GetProfile(
                      AString("ims.app.mts"), AString("ims.service.mts")),
            nullptr);
}

TEST(AosStaticConfigTest, GetProfiles)
{
    EXPECT_GT(AosStaticConfig::GetInstance()->GetProfiles().GetSize(), 0);
}