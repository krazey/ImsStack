/**
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

#include "ImsStrLib.h"
#include "ICarrierConfig.h"
#include "CarrierConfig.h"
#include "ServiceConfig.h"
#include "MockICarrierConfig.h"
#include "config/CodecHevcConfig.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_CHANNEL = CodecHevcConfig::DEFAULT_CHANNEL;
static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = CodecHevcConfig::DEFAULT_HEVC_RESOLUTION_WIDTH;
static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = CodecHevcConfig::DEFAULT_HEVC_RESOLUTION_HEIGHT;
static const IMS_SINT32 DEFAULT_FRAMERATE = CodecHevcConfig::DEFAULT_HEVC_FRAMERATE;
static const IMS_SINT32 DEFAULT_BITRATE = CodecHevcConfig::DEFAULT_HEVC_BITRATE;
static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = CodecHevcConfig::DEFAULT_PACKETIZATION_MODE;
static const IMS_SINT32 DEFAULT_PROFILE = CodecHevcConfig::DEFAULT_HEVC_PROFILE;
static const IMS_SINT32 DEFAULT_LEVEL = CodecHevcConfig::DEFAULT_HEVC_LEVEL;
#define DEFAULT_SPROP_PARAMS                                \
    "AAAAAUABDAH//wFgAAADALAAAAMAAAMAWqxZ,"                 \
    "AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==," \
    "AAAAAUQBwPPAAhA="
#define DEFAULT_IMAGE_ATTR "send [x=480,y=640] [x=720,y=1280] recv [x=480,y=640] [x=720,y=1280]"
#define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

using ::testing::Return;

class CodecHevcConfigTest : public ::testing::Test {
public:
    CodecHevcConfig* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pVideoBundle;
    MockICarrierConfig* m_pVideoSubBundle;
    IMS_SINT32 m_nHevcPayloadTypeNumber = 120;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new CodecHevcConfig(ImsCodec::VIDEO_HEVC, m_nHevcPayloadTypeNumber);
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pVideoBundle = new MockICarrierConfig();
        m_pVideoSubBundle = new MockICarrierConfig();
    }

    virtual void TearDown() override
    {
        delete m_pConfig;
        delete m_pMockICarrierConfig;
        delete m_pVideoBundle;
        delete m_pVideoSubBundle;

        m_pConfig = IMS_NULL;
        m_pMockICarrierConfig = IMS_NULL;
        m_pVideoBundle = IMS_NULL;
        m_pVideoSubBundle = IMS_NULL;
    }

    inline void GetReadyToCreate()
    {
        AString strPayloadTypeNumber;
        strPayloadTypeNumber.SetNumber(m_nHevcPayloadTypeNumber);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::Assets::KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pVideoBundle));
        ON_CALL(*m_pVideoBundle, GetBundle(strPayloadTypeNumber.GetStr()))
                .WillByDefault(Return(m_pVideoSubBundle));
    }
};

TEST_F(CodecHevcConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetResolutionWidth(), DEFAULT_RESOLUTION_WIDTH);
    EXPECT_EQ(m_pConfig->GetResolutionHeight(), DEFAULT_RESOLUTION_HEIGHT);
    EXPECT_EQ(m_pConfig->GetFramerate(), DEFAULT_FRAMERATE);
    EXPECT_EQ(m_pConfig->GetBitrate(), DEFAULT_BITRATE);
    EXPECT_EQ(m_pConfig->GetPacketizationMode(), DEFAULT_PACKETIZATION_MODE);
    EXPECT_EQ(m_pConfig->GetSpropParameterSets(), DEFAULT_SPROP_PARAMS);
    EXPECT_EQ(m_pConfig->GetHevcProfile(), DEFAULT_PROFILE);
    EXPECT_EQ(m_pConfig->GetHevcLevel(), DEFAULT_LEVEL);
    EXPECT_EQ(m_pConfig->GetImageAttr(), DEFAULT_IMAGE_ATTR);
    EXPECT_EQ(m_pConfig->GetFrameSize(), DEFAULT_FRAME_SIZE);
}

TEST_F(CodecHevcConfigTest, GetChannel)
{
    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
}

TEST_F(CodecHevcConfigTest, GetVideoResolution)
{
    IMS_SINT32 nVideoWidth = 720;
    IMS_SINT32 nVideoHeight = 1280;
    ImsVector<IMS_SINT32> objVideoResolution;
    objVideoResolution.Push(nVideoWidth);
    objVideoResolution.Push(nVideoHeight);

    ON_CALL(*m_pVideoSubBundle,
            GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY))
            .WillByDefault(Return(objVideoResolution));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetResolutionWidth(), nVideoWidth);
    EXPECT_EQ(m_pConfig->GetResolutionHeight(), nVideoHeight);
}

TEST_F(CodecHevcConfigTest, GetFramerate)
{
    IMS_SINT32 nFramerate = 1;

    ON_CALL(*m_pVideoSubBundle,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT,
                    DEFAULT_FRAMERATE))
            .WillByDefault(Return(nFramerate));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetFramerate(), nFramerate);
}

TEST_F(CodecHevcConfigTest, GetVideoBitrate)
{
    IMS_SINT32 nVideoBitrate = 512;
    ImsVector<IMS_SINT32> objVideoBitrate;
    objVideoBitrate.Push(nVideoBitrate);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY))
            .WillByDefault(Return(objVideoBitrate));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetBitrate(), nVideoBitrate);
}

TEST_F(CodecHevcConfigTest, GetPacketizationMode)
{
    IMS_SINT32 nPacketizationMode = 1;

    ON_CALL(*m_pVideoSubBundle,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
                    DEFAULT_PACKETIZATION_MODE))
            .WillByDefault(Return(nPacketizationMode));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetPacketizationMode(), nPacketizationMode);
}

TEST_F(CodecHevcConfigTest, GetSpropParameterSets)
{
    AString strSpropParameterSets(
            "AAAAAUABDAH//"
            "wFgAAADALAAAAMAAAMAWqxZ,AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==,"
            "AAAAAUQBwPPAAhA=");
    ON_CALL(*m_pVideoSubBundle,
            GetString(CarrierConfig::Assets::KEY_HEVC_SPROP_PARAMETER_SETS_STRING,
                    AString::ConstNull()))
            .WillByDefault(Return(strSpropParameterSets));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetSpropParameterSets(), strSpropParameterSets);
}

TEST_F(CodecHevcConfigTest, GetHevcProfile)
{
    IMS_SINT32 nHevcProfile = 3;
    ON_CALL(*m_pVideoSubBundle,
            GetInt(CarrierConfig::Assets::KEY_HEVC_PROFILE_INT, DEFAULT_PROFILE))
            .WillByDefault(Return(nHevcProfile));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetHevcProfile(), nHevcProfile);
}

TEST_F(CodecHevcConfigTest, GetHevcLevel)
{
    IMS_SINT32 nHevcLevel = 3;
    ON_CALL(*m_pVideoSubBundle, GetInt(CarrierConfig::Assets::KEY_HEVC_LEVEL_INT, DEFAULT_LEVEL))
            .WillByDefault(Return(nHevcLevel));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetHevcLevel(), nHevcLevel);
}

TEST_F(CodecHevcConfigTest, GetImageAttr)
{
    ImsVector<AString> objImageAttr;
    objImageAttr.Push("send  [x=720,y=1280] recv [x=720,y=1280]");

    ON_CALL(*m_pMockICarrierConfig,
            GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY))
            .WillByDefault(Return(objImageAttr));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetImageAttr(), objImageAttr.GetAt(0));
}
