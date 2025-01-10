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

#include "manager/AosMngr.h"

class AosMngrTest : public ::testing::Test
{
public:
    AosMngr* m_pAosMngr;

    const AString m_strAppId = AString("ims.app.test");
    const AString m_strServiceId = AString("ims.service.test");
    const IMS_UINT32 m_nServiceType = -1;

protected:
    void SetUp() override { m_pAosMngr = new AosMngr(IMS_SLOT_0); }

    void TearDown() override
    {
        if (m_pAosMngr != nullptr)
        {
            delete m_pAosMngr;
            m_pAosMngr = nullptr;
        }
    }

    AosStaticConfig* GetStaticConfig() { return m_pAosMngr->m_pStaticConfig; }

    void SetStaticConfig(AosStaticConfig* objStaticConfig)
    {
        m_pAosMngr->m_pStaticConfig = objStaticConfig;
    }

    AosBuildDirector* GetBuildDirector() { return m_pAosMngr->m_pBuildDirector; }
};

TEST_F(AosMngrTest, Constructor_Test)
{
    EXPECT_TRUE(GetStaticConfig() != IMS_NULL);
    EXPECT_TRUE(GetBuildDirector() != IMS_NULL);
}

TEST_F(AosMngrTest, GetAosHandle_Test1)
{
    // Test1: static config is null
    // Expectation: return null

    SetStaticConfig(IMS_NULL);
    EXPECT_EQ(m_pAosMngr->GetAosHandle(m_strAppId, m_strServiceId), nullptr);
}
/*
TEST_F(AosMngrTest, GetAosHandle_Test2)
{
    // Test2: static profile is null
    // Expectation: return null

    // TODO: Need a way that can make pProfile IMS_NULL
}
*/