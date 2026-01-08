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
#include "provider/AosStaticConfig.h"

#define DECLARE_USING(Base) using Base::DestroyStaticConfig;

class TestAosMngr : public AosMngr
{
public:
    DECLARE_USING(AosMngr)

    inline explicit TestAosMngr(IN IMS_UINT32 nSlotId) :
            AosMngr(nSlotId)
    {
    }

    inline AosBuildDirector* GetBuildDirector() { return m_pBuildDirector; }
    inline AosStaticConfig* GetStaticConfig() { return m_pStaticConfig; }

    void SetStaticConfig(AosStaticConfig* objStaticConfig)
    {
        if (m_pStaticConfig != nullptr)
        {
            DestroyStaticConfig();
        }

        m_pStaticConfig = objStaticConfig;
    }
};

class AosMngrTest : public ::testing::Test
{
public:
    TestAosMngr* m_pAosMngr;

    const AString m_strAppId = AString("ims.app.test");
    const AString m_strServiceId = AString("ims.service.test");

protected:
    void SetUp() override { m_pAosMngr = new TestAosMngr(IMS_SLOT_0); }

    void TearDown() override
    {
        if (m_pAosMngr != nullptr)
        {
            delete m_pAosMngr;
            m_pAosMngr = nullptr;
        }
    }
};

TEST_F(AosMngrTest, Constructor_Test)
{
    EXPECT_TRUE(m_pAosMngr->GetStaticConfig() != IMS_NULL);
    EXPECT_TRUE(m_pAosMngr->GetBuildDirector() != IMS_NULL);
}

TEST_F(AosMngrTest, GetAosHandleReturnsNullIfStaticConfigIsNull)
{
    // GIVEN
    m_pAosMngr->SetStaticConfig(IMS_NULL);

    // WHEN & THEN
    EXPECT_EQ(m_pAosMngr->GetAosHandle(m_strAppId, m_strServiceId), nullptr);
}

TEST_F(AosMngrTest, GetAosHandleReturnsNullIfStaticProfileIsNull)
{
    // GIVEN
    AosStaticConfig* pStaticConfig = new AosStaticConfig();
    m_pAosMngr->SetStaticConfig(pStaticConfig);

    // WHEN & THEN
    EXPECT_EQ(m_pAosMngr->GetAosHandle(m_strAppId, m_strServiceId), nullptr);
}

TEST_F(AosMngrTest, StaticConfigShouldProperlyDestroyed)
{
    // WHEN
    m_pAosMngr->DestroyStaticConfig();

    // THEN
    EXPECT_EQ(m_pAosMngr->GetStaticConfig(), nullptr);
    EXPECT_EQ(m_pAosMngr->GetStaticConfig(), IMS_NULL);
}
