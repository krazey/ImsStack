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
#include "interface/IAosBlock.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosNConfiguration.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base) using Base::InitializeServiceFeature;

class TestAosHandleEmergencyMtc : public AosHandleEmergencyMtc
{
public:
    DECLARE_USING(AosHandleEmergencyMtc)

    inline explicit TestAosHandleEmergencyMtc(IN IAosAppContext* piAosAppContext,
            IN const AString& strAppId, IN const AString& strServiceId,
            IN const IMS_SINT32 nServiceType) :
            AosHandleEmergencyMtc(piAosAppContext, strAppId, strServiceId, nServiceType)
    {
    }
};

class AosHandleEmergencyMtcTest : public ::testing::Test
{
public:
    TestAosHandleEmergencyMtc* m_pAosHandleEmergencyMtc;

    IAosNConfiguration* m_piAosNConfiguration;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    const AString m_strAppId = AString("ims.app.mtc.emergency.test");
    const AString m_strServiceId = AString("ims.service.mtc.emergency.test");
    const IMS_UINT32 m_nServiceType = ImsAosService::EMERGENCY_MTC;

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
        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(Return(&m_objMockIAosBlock));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        m_pAosHandleEmergencyMtc = new TestAosHandleEmergencyMtc(
                &m_objMockIAosAppContext, m_strAppId, m_strServiceId, m_nServiceType);

        ASSERT_TRUE(m_pAosHandleEmergencyMtc != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosHandleEmergencyMtc != nullptr)
        {
            delete m_pAosHandleEmergencyMtc;
            m_pAosHandleEmergencyMtc = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }
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

    m_pAosHandleEmergencyMtc->InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));

    m_pAosHandleEmergencyMtc->InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
}

TEST_F(AosHandleEmergencyMtcTest, VideoFeatureIsSupportedIfSubscriberIncompleted)
{
    // GIVEN
    ON_CALL(m_objMockIAosBlock,
            IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, IMS_FALSE, SERVICE_TYPE::SERVICE_WHOLE))
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleEmergencyMtc->InitializeServiceFeature();

    // THEN
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleEmergencyMtcTest,
        VideoFeatureIsSupportedIfSubscriberCompletedAndConfiguredToSupport)
{
    // GIVEN
    ON_CALL(m_objMockIAosBlock,
            IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, IMS_FALSE, SERVICE_TYPE::SERVICE_WHOLE))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoSupportedForEmergencyReg())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleEmergencyMtc->InitializeServiceFeature();

    // THEN
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleEmergencyMtcTest,
        VideoFeatureIsNotSupportedIfSubscriberCompletedAndConfiguredToNotSupport)
{
    // GIVEN
    ON_CALL(m_objMockIAosBlock,
            IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, IMS_FALSE, SERVICE_TYPE::SERVICE_WHOLE))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoSupportedForEmergencyReg())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleEmergencyMtc->InitializeServiceFeature();

    // THEN
    EXPECT_FALSE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleEmergencyMtcTest, ShouldInitializeServiceFeatureIfEmergencyRegisterStartRequested)
{
    // GIVEN
    MockIAosApplication objMockIAosApplication;
    ON_CALL(m_objMockIAosAppContext, GetApp()).WillByDefault(Return(&objMockIAosApplication));
    ON_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosBlock,
            IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, IMS_FALSE, SERVICE_TYPE::SERVICE_WHOLE))
            .WillByDefault(Return(IMS_TRUE));
    m_pAosHandleEmergencyMtc->GetFeatureTagList().Clear();

    // WHEN
    m_pAosHandleEmergencyMtc->Control(ImsAosControl::REGISTER_START);

    // THEN
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_pAosHandleEmergencyMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
}
