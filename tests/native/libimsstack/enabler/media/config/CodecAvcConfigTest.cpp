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
#include "config/CodecAvcConfig.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_CHANNEL = CodecAvcConfig::DEFAULT_CHANNEL;
static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = CodecAvcConfig::DEFAULT_AVC_RESOLUTION_WIDTH;
static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = CodecAvcConfig::DEFAULT_AVC_RESOLUTION_HEIGHT;
static const IMS_SINT32 DEFAULT_FRAMERATE = CodecAvcConfig::DEFAULT_AVC_FRAMERATE;
static const IMS_SINT32 DEFAULT_BITRATE = CodecAvcConfig::DEFAULT_AVC_BITRATE;
static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = CodecAvcConfig::DEFAULT_PACKETIZATION_MODE;
static const IMS_BOOL DEFAULT_INCLUDE_SPROP = CodecAvcConfig::DEFAULT_INCLUDE_SPROP;
#define DEFAULT_SPROP_PARAMS "Z0LAFukDwKMg,aM4G4g=="
#define DEFAULT_PROFILE_ID "42C00C"
#define DEFAULT_IMAGE_ATTR \
    "send [x=320,y=240] [x=640,y=480] recv [x=320,y=240] [x=640,y=480] [x=1280,y=720]"
#define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

using ::testing::Return;

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class CodecAvcConfigTest : public ::testing::Test {
public:
    CodecAvcConfig* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pVideoBundle;
    MockICarrierConfig* m_pVideoSubBundle;
    IMS_SINT32 m_nAvcPayloadTypeNumber = 105;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new CodecAvcConfig(ImsCodec::VIDEO_AVC, m_nAvcPayloadTypeNumber);
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
        strPayloadTypeNumber.SetNumber(m_nAvcPayloadTypeNumber);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pVideoBundle));
        ON_CALL(*m_pVideoBundle, GetBundle(IsSameKey(strPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pVideoSubBundle));
    }
};

TEST_F(CodecAvcConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetResolutionWidth(), DEFAULT_RESOLUTION_WIDTH);
    EXPECT_EQ(m_pConfig->GetResolutionHeight(), DEFAULT_RESOLUTION_HEIGHT);
    EXPECT_EQ(m_pConfig->GetFramerate(), DEFAULT_FRAMERATE);
    EXPECT_EQ(m_pConfig->GetBitrate(), DEFAULT_BITRATE);
    EXPECT_EQ(m_pConfig->GetPacketizationMode(), DEFAULT_PACKETIZATION_MODE);
    EXPECT_EQ(m_pConfig->GetIncludeSpropParameterSets(), DEFAULT_INCLUDE_SPROP);
    EXPECT_EQ(m_pConfig->GetSpropParameterSets(), DEFAULT_SPROP_PARAMS);
    EXPECT_EQ(m_pConfig->GetProfileLevelId(), DEFAULT_PROFILE_ID);
    EXPECT_EQ(m_pConfig->GetImageAttr(), DEFAULT_IMAGE_ATTR);
    EXPECT_EQ(m_pConfig->GetFrameSize(), DEFAULT_FRAME_SIZE);
}

TEST_F(CodecAvcConfigTest, GetChannel)
{
    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
}

TEST_F(CodecAvcConfigTest, GetVideoResolution)
{
    IMS_SINT32 nVideoWidth = 480;
    IMS_SINT32 nVideoHeight = 640;
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

TEST_F(CodecAvcConfigTest, GetFramerate)
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

TEST_F(CodecAvcConfigTest, GetVideoBitrate)
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

TEST_F(CodecAvcConfigTest, GetPacketizationMode)
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

TEST_F(CodecAvcConfigTest, GetIncludeSpropParameterSets)
{
    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetIncludeSpropParameterSets(), DEFAULT_INCLUDE_SPROP);
}

TEST_F(CodecAvcConfigTest, GetSpropParameterSets)
{
    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetSpropParameterSets(), DEFAULT_SPROP_PARAMS);
}

TEST_F(CodecAvcConfigTest, GetProfileLevelId)
{
    AString strProfileLevelId("42E00C");
    ON_CALL(*m_pVideoSubBundle,
            GetString(CarrierConfig::ImsVt::KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
                    AString::ConstNull()))
            .WillByDefault(Return(strProfileLevelId));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetProfileLevelId(), strProfileLevelId);
}

TEST_F(CodecAvcConfigTest, GetVideoImageAttr)
{
    ImsVector<AString> objImageAttr;
    objImageAttr.Push("send [x=640,y=480] recv [x=640,y=480]");

    ON_CALL(*m_pMockICarrierConfig,
            GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY))
            .WillByDefault(Return(objImageAttr));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetImageAttr(), objImageAttr.GetAt(0));
}
