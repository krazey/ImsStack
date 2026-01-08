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

#include "ImsAosParameter.h"
#include "provider/AosRegStateManager.h"

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SLOT_ID2 = 1;

class AosRegStateManagerTest : public ::testing::Test
{
public:
    AosRegStateManager* pAosRegStateManager;

protected:
    void SetUp() override
    {
        pAosRegStateManager = new AosRegStateManager();
        ASSERT_TRUE(pAosRegStateManager != nullptr);
    }

    void TearDown() override
    {
        if (pAosRegStateManager)
        {
            delete pAosRegStateManager;
        }
    }

    IMS_UINT32 GetEImsRegState() { return pAosRegStateManager->m_nERegState; }

    IMS_UINT32 GetRegServices() { return pAosRegStateManager->m_nRegServices; }

    void SetRegServices(IN IMS_UINT32 nRegServices)
    {
        pAosRegStateManager->m_nRegServices = nRegServices;
    }

    void AddRegService(IN IMS_UINT32 nType) { pAosRegStateManager->AddRegService(nType); }

    IMS_BOOL IsRegService(IN IMS_UINT32 nType) { return pAosRegStateManager->IsRegService(nType); }
};

TEST_F(AosRegStateManagerTest, SlotId)
{
    pAosRegStateManager->SetSlotId(SLOT_ID);
    EXPECT_EQ(SLOT_ID, pAosRegStateManager->GetSlotId());

    pAosRegStateManager->SetSlotId(SLOT_ID2);
    EXPECT_EQ(SLOT_ID2, pAosRegStateManager->GetSlotId());
}

TEST_F(AosRegStateManagerTest, ImsRegState)
{
    pAosRegStateManager->SetImsRegState(IMS_REG_OFF, IMS_FALSE);
    EXPECT_EQ(IMS_REG_OFF, pAosRegStateManager->GetImsRegState());

    pAosRegStateManager->SetImsRegState(IMS_REG_ON, IMS_FALSE);
    EXPECT_EQ(IMS_REG_ON, pAosRegStateManager->GetImsRegState());
}

TEST_F(AosRegStateManagerTest, ImsERegState)
{
    pAosRegStateManager->SetEImsRegState(IMS_REG_OFF);
    EXPECT_EQ(IMS_REG_OFF, GetEImsRegState());

    pAosRegStateManager->SetEImsRegState(IMS_REG_ON);
    EXPECT_EQ(IMS_REG_ON, GetEImsRegState());
}

TEST_F(AosRegStateManagerTest, RegServices)
{
    IMS_UINT32 nOrigRegServices = GetRegServices();

    pAosRegStateManager->SetRegState(ImsAosService::EMERGENCY_MTC, IMS_REG_ON);

    EXPECT_FALSE(IsRegService(ImsAosService::MTS));

    AddRegService(ImsAosService::MTS);
    EXPECT_TRUE(IsRegService(ImsAosService::MTS));

    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_OFF);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_OFF);
    EXPECT_FALSE(IsRegService(ImsAosService::MTC));
    EXPECT_FALSE(IsRegService(ImsAosService::MTS));

    pAosRegStateManager->SetRegState(ImsAosService::NONE, IMS_REG_ON);
    EXPECT_FALSE(IsRegService(ImsAosService::MTC));
    EXPECT_FALSE(IsRegService(ImsAosService::MTS));

    AddRegService(ImsAosService::MTS);

    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_ON);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_ON);
    EXPECT_TRUE(IsRegService(ImsAosService::MTC));
    EXPECT_TRUE(IsRegService(ImsAosService::MTS));

    SetRegServices(nOrigRegServices);
}