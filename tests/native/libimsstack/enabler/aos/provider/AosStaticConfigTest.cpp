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

TEST(AosStaticConfigTest, NEW)
{
    AosStaticConfig* pConfig = new AosStaticConfig();

    EXPECT_NE(pConfig, nullptr);

    delete pConfig;
    pConfig = nullptr;
    EXPECT_EQ(pConfig, nullptr);
}

TEST(AosStaticConfigTest, Create)
{
    AosStaticConfig* pConfig = new AosStaticConfig();
    EXPECT_TRUE(pConfig->Create());

    delete pConfig;
    pConfig = nullptr;
}

TEST(AosStaticConfigTest, GetProfile_ReturnNull)
{
    AosStaticConfig* pConfig = new AosStaticConfig();

    EXPECT_EQ(pConfig->GetProfile(AString("ims.app.test"), AString("ims.service.test")), nullptr);

    delete pConfig;
    pConfig = nullptr;
}

TEST(AosStaticConfigTest, GetProfile_ReturnProfile)
{
    AosStaticConfig* pConfig = new AosStaticConfig();

    EXPECT_EQ(pConfig->GetProfile(AString("ims.app.mts"), AString("ims.service.mts")), nullptr);

    delete pConfig;
    pConfig = nullptr;
}

TEST(AosStaticConfigTest, GetProfiles)
{
    AosStaticConfig* pConfig = new AosStaticConfig();

    EXPECT_EQ(pConfig->GetProfiles().GetSize(), 0);

    delete pConfig;
    pConfig = nullptr;
}