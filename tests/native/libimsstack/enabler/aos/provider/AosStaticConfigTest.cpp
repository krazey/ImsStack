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

class AosStaticConfigTest : public ::testing::Test
{
public:
    AosStaticConfig* m_pConfig;

protected:
    void SetUp() override
    {
        m_pConfig = new AosStaticConfig();
        ASSERT_TRUE(m_pConfig != nullptr);

        m_pConfig->Create();
    }

    void TearDown() override
    {
        if (m_pConfig)
        {
            delete m_pConfig;
        }
    }
};

TEST_F(AosStaticConfigTest, FailsGetProfileWithInvalidService)
{
    // GIVEN
    m_pConfig->Create();

    AString strInvalidApp = AString("ims.app.invalid");
    AString strInvalidService = AString("ims.service.invalid");

    // WHEN
    const AosStaticProfile* pProfile = m_pConfig->GetProfile(strInvalidApp, strInvalidService);

    EXPECT_EQ(nullptr, pProfile);
}

TEST_F(AosStaticConfigTest, SucceedsGetProfileWithValidService)
{
    // GIVEN
    m_pConfig->Create();

    AString strValidApp = AString("ims.app.mts");
    AString strValidService = AString("ims.service.mts");

    // WHEN
    const AosStaticProfile* pProfile = m_pConfig->GetProfile(strValidApp, strValidService);

    // THEN
    EXPECT_NE(nullptr, pProfile);
}

TEST_F(AosStaticConfigTest, SucceedsGetProfiles)
{
    // GIVEN
    m_pConfig->Create();

    // WHEN
    IMS_UINT32 nSize = m_pConfig->GetProfiles().GetSize();

    // THEN
    EXPECT_NE(0, nSize);
}
