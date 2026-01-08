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
#include "PlatformContext.h"
#include "TestUtilService.h"

using ::testing::Eq;
using ::testing::Return;

const IMS_SINT32 SLOT_ID = 0;

class AosStaticProfileTest : public ::testing::Test
{
public:
    AosStaticProfile* m_pProfile;

    TestUtilService m_objUtilService;

protected:
    void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);

        m_pProfile = new AosStaticProfile();
        ASSERT_TRUE(m_pProfile != nullptr);
    }

    void TearDown() override
    {
        if (m_pProfile)
        {
            delete m_pProfile;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
    }
};

TEST_F(AosStaticProfileTest, SucceedsSetProfileTypeWhenNormal)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(AosStaticProfile::Type::NORMAL, m_pProfile->GetProfileType());
}

TEST_F(AosStaticProfileTest, SucceedsSetIdWhenNormal)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(AString("aos_normal"), m_pProfile->GetId());
}

TEST_F(AosStaticProfileTest, SucceedsSetConnectionTypeWhenNormal)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(NetworkPolicy::APN_IMS, m_pProfile->GetConnectionType());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationIdWhenNormal)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(AString("aos_normal_reg"), m_pProfile->GetRegistrationId());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationTypeWhenNormal)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(AosRegistrationType::NORMAL, m_pProfile->GetRegistrationType());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationFlowIdWhenNormal)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(static_cast<IMS_UINT32>(AosRegistrationFlowId::NORMAL),
            m_pProfile->GetRegistrationFlowId());
}

TEST_F(AosStaticProfileTest, SucceedsSetProfileTypeWhenEmergency)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(AosStaticProfile::Type::EMERGENCY, m_pProfile->GetProfileType());
}

TEST_F(AosStaticProfileTest, SucceedsSetIdWhenEmergency)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(AString("aos_emergency"), m_pProfile->GetId());
}

TEST_F(AosStaticProfileTest, SucceedsSetConnectionTypeWhenEmergency)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(NetworkPolicy::APN_EMERGENCY, m_pProfile->GetConnectionType());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationIdWhenEmergency)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(AString("aos_emergency_reg"), m_pProfile->GetRegistrationId());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationTypeWhenEmergency)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(AosRegistrationType::EMERGENCY, m_pProfile->GetRegistrationType());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationFlowIdWhenEmergency)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(static_cast<IMS_UINT32>(AosRegistrationFlowId::EMERGENCY),
            m_pProfile->GetRegistrationFlowId());
}

TEST_F(AosStaticProfileTest, SucceedsSetProfileTypeWhenRcs)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::RCS);

    // THEN
    EXPECT_EQ(AosStaticProfile::Type::RCS, m_pProfile->GetProfileType());
}

TEST_F(AosStaticProfileTest, SucceedsSetIdWhenRcs)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::RCS);

    // THEN
    EXPECT_EQ(AString("aos_rcs"), m_pProfile->GetId());
}

TEST_F(AosStaticProfileTest, SucceedsSetConnectionTypeWhenRcs)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::RCS);

    // THEN
    EXPECT_EQ(NetworkPolicy::APN_INTERNET, m_pProfile->GetConnectionType());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationIdWhenRcs)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::RCS);

    // THEN
    EXPECT_EQ(AString("aos_rcs_reg"), m_pProfile->GetRegistrationId());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationTypeWhenRcs)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::RCS);

    // THEN
    EXPECT_EQ(AosRegistrationType::RCS, m_pProfile->GetRegistrationType());
}

TEST_F(AosStaticProfileTest, SucceedsSetRegistrationFlowIdWhenRcs)
{
    // GIVEN
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::RCS);

    // THEN
    EXPECT_EQ(static_cast<IMS_UINT32>(AosRegistrationFlowId::RCS),
            m_pProfile->GetRegistrationFlowId());
}

TEST_F(AosStaticProfileTest, SucceedsSetConnectionTypeWhenNormalAndWifiTest)
{
    // GIVEN
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(Eq(ImsPrivateProperties::Persistent::KEY_WIFI_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

    // THEN
    EXPECT_EQ(NetworkPolicy::APN_WIFI, m_pProfile->GetConnectionType());
}

TEST_F(AosStaticProfileTest, SucceedsSetConnectionTypeWhenEmergencyAndWifiTest)
{
    // GIVEN
    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistentInt(Eq(ImsPrivateProperties::Persistent::KEY_WIFI_TEST), Eq(SLOT_ID)))
            .WillByDefault(Return(1));
    // WHEN
    m_pProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    // THEN
    EXPECT_EQ(NetworkPolicy::APN_WIFI, m_pProfile->GetConnectionType());
}

TEST_F(AosStaticProfileTest, SucceedsAddService)
{
    // GIVEN
    IMS_UINT32 nSize = m_pProfile->GetServiceProfiles().GetSize();

    // WHEN
    m_pProfile->AddService(AString("TestAppId1"), AString("TestServiceId1"));
    m_pProfile->AddService(AString("TestAppId2"), AString("TestServiceId2"));
    m_pProfile->AddService(AString("TestAppId3"), AString("TestServiceId3"));

    // THEN
    EXPECT_EQ(nSize + 3, m_pProfile->GetServiceProfiles().GetSize());
}
