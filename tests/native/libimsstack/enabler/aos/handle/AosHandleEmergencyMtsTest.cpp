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

#include "handle/AosHandleEmergencyMts.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosNConfiguration.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base)               \
    using Base::InitializeServiceBlock;   \
    using Base::InitializeServiceFeature; \
    using Base::IsBlocked;

class TestAosHandleEmergencyMts : public AosHandleEmergencyMts
{
public:
    DECLARE_USING(AosHandleEmergencyMts)

    inline TestAosHandleEmergencyMts(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
            AosHandleEmergencyMts(piAppContext, strAppId, strServiceId, nServiceType)
    {
    }
};

class AosHandleEmergencyMtsTest : public ::testing::Test
{
public:
    TestAosHandleEmergencyMts* m_pAosHandleEmergencyMts;

    MockIAosAppContext m_objMockIAosAppContext;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    const AString m_strAppId = AString("ims.app.mts.emergency.test");
    const AString m_strServiceId = AString("ims.service.mts.emergency.test");
    const IMS_UINT32 m_nServiceType = -1;

protected:
    void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(0));

        const AString strValue = AString("test");
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(strValue));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);

        m_pAosHandleEmergencyMts = new TestAosHandleEmergencyMts(
                &m_objMockIAosAppContext, m_strAppId, m_strServiceId, m_nServiceType);

        ASSERT_TRUE(m_pAosHandleEmergencyMts != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosHandleEmergencyMts != nullptr)
        {
            delete m_pAosHandleEmergencyMts;
            m_pAosHandleEmergencyMts = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }
};

TEST_F(AosHandleEmergencyMtsTest, InitializeServiceBlock_Test)
{
    // Expectation: Blocked if EmergencySmsOverIms is not supported

    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmergencySmsOverImsSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandleEmergencyMts->InitializeServiceBlock();
    EXPECT_FALSE(m_pAosHandleEmergencyMts->IsBlocked());

    m_pAosHandleEmergencyMts->InitializeServiceBlock();
    EXPECT_TRUE(m_pAosHandleEmergencyMts->IsBlocked());
}

TEST_F(AosHandleEmergencyMtsTest, InitializeServiceFeature_Test)
{
    // Expectation: SMSIP feature is added if EmergencySmsOverIms supported

    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmergencySmsOverImsSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandleEmergencyMts->InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleEmergencyMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));

    m_pAosHandleEmergencyMts->InitializeServiceFeature();
    EXPECT_FALSE(m_pAosHandleEmergencyMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}
