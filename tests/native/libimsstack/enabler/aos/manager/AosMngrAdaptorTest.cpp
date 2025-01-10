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

#include "ImsTypeDef.h"
#include "manager/AosMngrAdaptor.h"

class AosMngrAdaptorTest : public ::testing::Test
{
public:
    AosMngrAdaptor* m_AosMngrAdaptor;

protected:
    void SetUp() override
    {
        const AString strName("AosManagerAdapter");
        m_AosMngrAdaptor = new AosMngrAdaptor(strName, IMS_SLOT_0);

        ASSERT_TRUE(m_AosMngrAdaptor != nullptr);
    }

    void TearDown() override
    {
        if (m_AosMngrAdaptor)
        {
            delete m_AosMngrAdaptor;
        }
    }
};

TEST_F(AosMngrAdaptorTest, GetImsAos)
{
    EXPECT_NE(m_AosMngrAdaptor->GetImsAos("ims.app.mtc", "ims.service.mtc"), nullptr);
    EXPECT_NE(m_AosMngrAdaptor->GetImsAos("ims.app.mtc", "ims.service.mtc.emergency"), nullptr);
}

TEST_F(AosMngrAdaptorTest, GetImsAosList)
{
    EXPECT_NE(m_AosMngrAdaptor->GetImsAosList("ims.app.mtc", "ims.service.mtc").GetSize(), 0);
    EXPECT_NE(m_AosMngrAdaptor->GetImsAosList("ims.app.mtc", "ims.service.mtc.emergency").GetSize(),
            0);
    EXPECT_NE(m_AosMngrAdaptor->GetImsAosList("ims.app.mtc").GetSize(), 0);
}