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

#include "Feature.h"
#include "MockISipConfigV.h"
#include "private/CoreServiceConfig.h"
#include "private/TestAppConfig.h"
#include "util/CallerCapability.h"

using ::testing::Return;

namespace android
{

class CallerCapabilityTest : public ::testing::Test
{
public:
    inline CallerCapabilityTest() :
            m_pSipConfigV(IMS_NULL),
            m_objAudioFeature("audio"),
            m_objVideoFeature("video"),
            m_objTextFeature("text")
    {
    }

protected:
    virtual void SetUp() override { m_pSipConfigV = new MockISipConfigV(); }

    virtual void TearDown() override
    {
        if (m_pSipConfigV != IMS_NULL)
        {
            delete m_pSipConfigV;
            m_pSipConfigV = IMS_NULL;
        }
    }

protected:
    MockISipConfigV* m_pSipConfigV;
    Feature m_objAudioFeature;
    Feature m_objVideoFeature;
    Feature m_objTextFeature;
};

TEST_F(CallerCapabilityTest, CreateWithAppConfigNull)
{
    CallerCapability objCc(0);

    ASSERT_FALSE(objCc.Create(IMS_NULL, IMS_NULL, IMS_NULL));
}

TEST_F(CallerCapabilityTest, CreateWithStreamAll)
{
    CallerCapability objCc(0);
    AppConfig objAppConfig =
            TestAppConfig::Create(TestAppConfig::TEST_APP_NAME, TestAppConfig::TEST_CONFIG);

    ON_CALL(*m_pSipConfigV, GetFeatureTagOptions())
            .WillByDefault(Return(ISipConfigV::FEATURE_TAG_MMTEL_DEFAULT |
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT));

    ASSERT_TRUE(objCc.Create(&objAppConfig,
            objAppConfig.GetCoreServiceConfigEx(TestAppConfig::TEST_SERVICE_NAME_1),
            m_pSipConfigV));

    EXPECT_FALSE(objCc.IsEmpty());
    EXPECT_TRUE(objCc.HasFeature(&m_objAudioFeature));
    EXPECT_TRUE(objCc.HasFeature(&m_objVideoFeature));
    EXPECT_TRUE(objCc.HasFeature(&m_objTextFeature));
}

TEST_F(CallerCapabilityTest, CreateWithStreamAudioVideo)
{
    CallerCapability objCc(0);
    AppConfig objAppConfig = TestAppConfig::Create(TestAppConfig::TEST_APP_NAME,
            TestAppConfig::TEST_CONFIG, TestAppConfig::FLAG_STREAM_AUDIO_VIDEO);

    ON_CALL(*m_pSipConfigV, GetFeatureTagOptions())
            .WillByDefault(Return(ISipConfigV::FEATURE_TAG_MMTEL_DEFAULT |
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT));

    ASSERT_TRUE(objCc.Create(&objAppConfig,
            objAppConfig.GetCoreServiceConfigEx(TestAppConfig::TEST_SERVICE_NAME_1),
            m_pSipConfigV));

    EXPECT_FALSE(objCc.IsEmpty());
    EXPECT_TRUE(objCc.HasFeature(&m_objAudioFeature));
    EXPECT_TRUE(objCc.HasFeature(&m_objVideoFeature));
    EXPECT_FALSE(objCc.HasFeature(&m_objTextFeature));
}

TEST_F(CallerCapabilityTest, CreateWithStreamAudio)
{
    CallerCapability objCc(0);
    AppConfig objAppConfig = TestAppConfig::Create(TestAppConfig::TEST_APP_NAME,
            TestAppConfig::TEST_CONFIG, TestAppConfig::FLAG_STREAM_AUDIO);

    ON_CALL(*m_pSipConfigV, GetFeatureTagOptions())
            .WillByDefault(Return(ISipConfigV::FEATURE_TAG_MMTEL_DEFAULT |
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT));

    ASSERT_TRUE(objCc.Create(&objAppConfig,
            objAppConfig.GetCoreServiceConfigEx(TestAppConfig::TEST_SERVICE_NAME_1),
            m_pSipConfigV));

    EXPECT_FALSE(objCc.IsEmpty());
    EXPECT_TRUE(objCc.HasFeature(&m_objAudioFeature));
    EXPECT_FALSE(objCc.HasFeature(&m_objVideoFeature));
    EXPECT_FALSE(objCc.HasFeature(&m_objTextFeature));
}

TEST_F(CallerCapabilityTest, CreateWithStreamEmpty)
{
    CallerCapability objCc(0);
    AppConfig objAppConfig = TestAppConfig::Create(TestAppConfig::TEST_APP_NAME,
            TestAppConfig::TEST_CONFIG, TestAppConfig::FLAG_STREAM_EMPTY);

    ON_CALL(*m_pSipConfigV, GetFeatureTagOptions())
            .WillByDefault(Return(ISipConfigV::FEATURE_TAG_MMTEL_DEFAULT |
                    ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT));

    ASSERT_TRUE(objCc.Create(&objAppConfig,
            objAppConfig.GetCoreServiceConfigEx(TestAppConfig::TEST_SERVICE_NAME_1),
            m_pSipConfigV));

    EXPECT_FALSE(objCc.IsEmpty());
    EXPECT_TRUE(objCc.HasFeature(&m_objAudioFeature));
    EXPECT_TRUE(objCc.HasFeature(&m_objVideoFeature));
    EXPECT_TRUE(objCc.HasFeature(&m_objTextFeature));
}

TEST_F(CallerCapabilityTest, CreateWithStreamEmptyFeatureTagMmTelDefault)
{
    CallerCapability objCc(0);
    AppConfig objAppConfig = TestAppConfig::Create(TestAppConfig::TEST_APP_NAME,
            TestAppConfig::TEST_CONFIG, TestAppConfig::FLAG_STREAM_EMPTY);

    ON_CALL(*m_pSipConfigV, GetFeatureTagOptions())
            .WillByDefault(Return(ISipConfigV::FEATURE_TAG_MMTEL_DEFAULT));

    ASSERT_TRUE(objCc.Create(&objAppConfig,
            objAppConfig.GetCoreServiceConfigEx(TestAppConfig::TEST_SERVICE_NAME_1),
            m_pSipConfigV));

    EXPECT_FALSE(objCc.IsEmpty());
    EXPECT_TRUE(objCc.HasFeature(&m_objAudioFeature));
    EXPECT_TRUE(objCc.HasFeature(&m_objVideoFeature));
    EXPECT_FALSE(objCc.HasFeature(&m_objTextFeature));
}

}  // namespace android
