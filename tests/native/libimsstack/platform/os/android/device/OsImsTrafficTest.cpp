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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsTypeDef.h"
#include "device/OsImsTraffic.h"

class OsImsTrafficTest : public ::testing::Test
{
public:
    OsImsTraffic* m_pOsImsTraffic;

protected:
    virtual void SetUp() override
    {
        m_pOsImsTraffic = new OsImsTraffic();
        ASSERT_TRUE(m_pOsImsTraffic != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pOsImsTraffic)
        {
            delete m_pOsImsTraffic;
            m_pOsImsTraffic = nullptr;
        }
    }
};

TEST_F(OsImsTrafficTest, TrafficIsAllowedIfSimultaneousCallingIsSupported)
{
    // GIVEN
    m_pOsImsTraffic->SetSimultaneousCallingSupported(IMS_SLOT_0, IMS_TRUE);

    // WHEN & THEN
    EXPECT_TRUE(
            m_pOsImsTraffic->IsAllowed(IMS_SLOT_0, OsImsTraffic::TRAFFIC_PRIORITY_REGISTRATION));
}
