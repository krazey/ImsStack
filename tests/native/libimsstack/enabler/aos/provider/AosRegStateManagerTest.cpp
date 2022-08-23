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
    virtual void SetUp() override
    {
        pAosRegStateManager = new AosRegStateManager();
        ASSERT_TRUE(pAosRegStateManager != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosRegStateManager)
        {
            delete pAosRegStateManager;
        }
    }

    IMS_UINT32 GetEImsRegState() { return pAosRegStateManager->m_nERegState; }

    void SetRegServices(IN IMS_UINT32 nRegServices)
    {
        pAosRegStateManager->m_nRegServices = nRegServices;
    }

    void AddRegService(IN IMS_UINT32 nType) { pAosRegStateManager->AddRegService(nType); }

    IMS_BOOL IsRegService(IN IMS_UINT32 nType) { return pAosRegStateManager->IsRegService(nType); }

    IMS_SINT32 GetReportedRegDetailState()
    {
        return pAosRegStateManager->m_nReportedRegDetailState;
    }
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

    EXPECT_FALSE(pAosRegStateManager->IsLimitedMode());
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
    IMS_UINT32 nOrigRegServices = pAosRegStateManager->GetRegServices();

    pAosRegStateManager->SetRegState(ImsAosService::EMERGENCY_MTC, IMS_REG_ON);

    pAosRegStateManager->ClearRegServices();
    EXPECT_FALSE(IsRegService(ImsAosService::MTS));

    AddRegService(ImsAosService::MTS);
    EXPECT_TRUE(IsRegService(ImsAosService::MTS));

    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_OFF);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_OFF);
    EXPECT_FALSE(IsRegService(ImsAosService::MTC));
    EXPECT_FALSE(IsRegService(ImsAosService::MTS));

    pAosRegStateManager->ClearRegServices();
    AddRegService(ImsAosService::MTS);

    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_ON);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_ON);
    EXPECT_TRUE(IsRegService(ImsAosService::MTC));
    EXPECT_TRUE(IsRegService(ImsAosService::MTS));

    SetRegServices(nOrigRegServices);
}

TEST_F(AosRegStateManagerTest, EnforceUpdateRegistration)
{
    // This test will go on again after the modem interface has completed.
    // Currently it is implemented until it is called.

    IMS_UINT32 nOrigRegServices = pAosRegStateManager->GetRegServices();

    pAosRegStateManager->ClearRegServices();
    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_ON);
    pAosRegStateManager->SetDetailState(IMS_REGISTRATION_REGISTERED);

    // EnforceUpdateRegistration()
    pAosRegStateManager->UpdateRegServices(IMS_TRUE);
    EXPECT_EQ(IMS_REGISTRATION_REGISTERED, GetReportedRegDetailState());

    pAosRegStateManager->ClearRegServices();
    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_OFF);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_OFF);
    pAosRegStateManager->SetDetailState(IMS_REGISTRATION_STOP);

    // EnforceUpdateRegistration()
    pAosRegStateManager->UpdateRegServices(IMS_TRUE);
    EXPECT_EQ(IMS_REGISTRATION_OFFLINE, GetReportedRegDetailState());

    SetRegServices(nOrigRegServices);
}

TEST_F(AosRegStateManagerTest, UpdateRegistration)
{
    // This test will go on again after the modem interface has completed.
    // Currently it is implemented until it is called.

    IMS_UINT32 nOrigRegServices = pAosRegStateManager->GetRegServices();

    pAosRegStateManager->ClearRegServices();
    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_ON);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_ON);
    pAosRegStateManager->SetDetailState(IMS_REGISTRATION_REGISTERED);

    // UpdateRegistration()
    pAosRegStateManager->UpdateRegServices(IMS_FALSE);
    EXPECT_EQ(IMS_REGISTRATION_REGISTERED, GetReportedRegDetailState());

    pAosRegStateManager->ClearRegServices();
    pAosRegStateManager->SetRegState(ImsAosService::MTC, IMS_REG_OFF);
    pAosRegStateManager->SetRegState(ImsAosService::MTS, IMS_REG_OFF);
    pAosRegStateManager->SetDetailState(IMS_REGISTRATION_STOP);

    // UpdateRegistration()
    pAosRegStateManager->UpdateRegServices(IMS_FALSE);
    EXPECT_EQ(IMS_REGISTRATION_OFFLINE, GetReportedRegDetailState());

    SetRegServices(nOrigRegServices);
}
