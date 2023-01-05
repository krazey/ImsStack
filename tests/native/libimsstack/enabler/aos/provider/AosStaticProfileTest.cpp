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

#include "ServiceNetworkPolicy.h"
#include "provider/AosStaticProfile.h"

class AosStaticProfileTest : public ::testing::Test
{
public:
    AosStaticProfile* pAosStaticProfile;

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        ASSERT_TRUE(pAosStaticProfile != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }
    }
};

TEST_F(AosStaticProfileTest, SetProflieType_Normal)
{
    pAosStaticProfile->SetProflieType(AosStaticProfile::Type::NORMAL);

    EXPECT_EQ(pAosStaticProfile->GetProfileType(), AosStaticProfile::Type::NORMAL);
    EXPECT_EQ(pAosStaticProfile->GetId(), AString("aos_normal"));
    EXPECT_EQ(pAosStaticProfile->GetConnectionType(), NetworkPolicy::APN_IMS);
    EXPECT_EQ(pAosStaticProfile->GetRegistrationId(), AString("aos_normal_reg"));
    EXPECT_EQ(pAosStaticProfile->GetRegistrationType(), AosRegistrationType::NORMAL);
    EXPECT_EQ(pAosStaticProfile->GetRegistrationFlowId(),
            static_cast<IMS_UINT32>(AosRegistrationFlowId::NORMAL));
}

TEST_F(AosStaticProfileTest, SetProflieType_Emergency)
{
    pAosStaticProfile->SetProflieType(AosStaticProfile::Type::EMERGENCY);

    EXPECT_EQ(pAosStaticProfile->GetProfileType(), AosStaticProfile::Type::EMERGENCY);
    EXPECT_EQ(pAosStaticProfile->GetId(), AString("aos_emergency"));
    EXPECT_EQ(pAosStaticProfile->GetConnectionType(), NetworkPolicy::APN_EMERGENCY);
    EXPECT_EQ(pAosStaticProfile->GetRegistrationId(), AString("aos_emergency_reg"));
    EXPECT_EQ(pAosStaticProfile->GetRegistrationType(), AosRegistrationType::EMERGENCY);
    EXPECT_EQ(pAosStaticProfile->GetRegistrationFlowId(),
            static_cast<IMS_UINT32>(AosRegistrationFlowId::EMERGENCY));
}

TEST_F(AosStaticProfileTest, SetProflieType_Rcs)
{
    pAosStaticProfile->SetProflieType(AosStaticProfile::Type::RCS);

    EXPECT_EQ(pAosStaticProfile->GetProfileType(), AosStaticProfile::Type::RCS);
    EXPECT_EQ(pAosStaticProfile->GetId(), AString("aos_rcs"));
    EXPECT_EQ(pAosStaticProfile->GetConnectionType(), NetworkPolicy::APN_INTERNET);
    EXPECT_EQ(pAosStaticProfile->GetRegistrationId(), AString("aos_rcs_reg"));
    EXPECT_EQ(pAosStaticProfile->GetRegistrationType(), AosRegistrationType::RCS);
    EXPECT_EQ(pAosStaticProfile->GetRegistrationFlowId(),
            static_cast<IMS_UINT32>(AosRegistrationFlowId::RCS));
}

TEST_F(AosStaticProfileTest, AddService)
{
    IMS_UINT32 nSize = pAosStaticProfile->GetServiceProfiles().GetSize();

    pAosStaticProfile->AddService(AString("TestAppId1"), AString("TestServiceId1"));
    EXPECT_EQ(pAosStaticProfile->GetServiceProfiles().GetSize(), nSize + 1);

    pAosStaticProfile->AddService(AString("TestAppId2"), AString("TestServiceId2"));
    EXPECT_EQ(pAosStaticProfile->GetServiceProfiles().GetSize(), nSize + 2);

    pAosStaticProfile->AddService(AString("TestAppId3"), AString("TestServiceId3"));
    EXPECT_EQ(pAosStaticProfile->GetServiceProfiles().GetSize(), nSize + 3);
}