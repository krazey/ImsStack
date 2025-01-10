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

class AosHandleEmergencyMtsTest : public ::testing::Test
{
public:
    AosHandleEmergencyMts* m_pAosHandleEmergencyMts;

    MockIAosAppContext m_objMockIAosAppContext;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    const AString m_strAppId = AString("ims.app.mts.emergency.test");
    const AString m_strServiceId = AString("ims.service.mts.emergency.test");
    const IMS_UINT32 m_nServiceType = -1;

protected:
    void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        m_pAosHandleEmergencyMts =
                new AosHandleEmergencyMts(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_strAppId, m_strServiceId, m_nServiceType);

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

    void InitializeServiceBlock() { m_pAosHandleEmergencyMts->InitializeServiceBlock(); }
    void InitializeServiceFeature() { m_pAosHandleEmergencyMts->InitializeServiceFeature(); }
    IMS_BOOL IsBlocked() { return m_pAosHandleEmergencyMts->AosHandle::IsBlocked(); }
};

TEST_F(AosHandleEmergencyMtsTest, InitializeServiceBlock_Test)
{
    // Expectation: Blocked if EmergencySmsOverIms is not supported

    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmergencySmsOverImsSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    InitializeServiceBlock();
    EXPECT_FALSE(IsBlocked());

    InitializeServiceBlock();
    EXPECT_TRUE(IsBlocked());
}

TEST_F(AosHandleEmergencyMtsTest, InitializeServiceFeature_Test)
{
    // Expectation: SMSIP feature is added if EmergencySmsOverIms supported

    EXPECT_CALL(m_objMockIAosNConfiguration, IsEmergencySmsOverImsSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleEmergencyMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));

    InitializeServiceFeature();
    EXPECT_FALSE(m_pAosHandleEmergencyMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}