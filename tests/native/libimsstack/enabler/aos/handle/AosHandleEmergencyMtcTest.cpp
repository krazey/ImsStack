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

#include "handle/AosHandleEmergencyMtc.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosNConfiguration.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleEmergencyMtcTest : public ::testing::Test
{
public:
    AosHandleEmergencyMtc* m_pAosHandleEmergencyMtc;

    MockIAosAppContext m_objMockIAosAppContext;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    const AString m_strAppId = AString("ims.app.mtc.emergency.test");
    const AString m_strServiceId = AString("ims.service.mtc.emergency.test");
    const IMS_UINT32 m_nServiceType = -1;

protected:
    virtual void SetUp() override
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

        m_pAosHandleEmergencyMtc =
                new AosHandleEmergencyMtc(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_strAppId, m_strServiceId, m_nServiceType);

        ASSERT_TRUE(m_pAosHandleEmergencyMtc != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosHandleEmergencyMtc != nullptr)
        {
            delete m_pAosHandleEmergencyMtc;
            m_pAosHandleEmergencyMtc = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }

    void InitializeServiceFeature() { m_pAosHandleEmergencyMtc->InitializeServiceFeature(); }
};

TEST_F(AosHandleEmergencyMtcTest, Constructor_Test) {}

TEST_F(AosHandleEmergencyMtcTest, Destructor_Test) {}

TEST_F(AosHandleEmergencyMtcTest, InitializeServiceFeature_Test)
{
    // Expectation: MMTEL feature is always added. TEXT feature is added if RTT is supported.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoSupportedForEmergencyReg())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));

    InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
}
