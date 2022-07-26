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

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::VIDEO_AVC;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;
static const IMS_SINT32 DEFAULT_CHANNEL = CodecAvcConfig::DEFAULT_CHANNEL;
static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = CodecAvcConfig::DEFAULT_RESOLUTION_WIDTH;
static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = CodecAvcConfig::DEFAULT_RESOLUTION_HEIGHT;
static const IMS_SINT32 DEFAULT_FRAMERATE = CodecAvcConfig::DEFAULT_FRAMERATE;
static const IMS_SINT32 DEFAULT_BITRATE = CodecAvcConfig::DEFAULT_BITRATE;
static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = CodecAvcConfig::DEFAULT_PACKETIZATION_MODE;
static const IMS_BOOL DEFAULT_INCLUDE_SPROP = CodecAvcConfig::DEFAULT_INCLUDE_SPROP;
#define DEFAULT_SPROP_PARAMS "Z0LAFukDwKMg,aM4G4g=="
#define DEFAULT_PROFILE_ID "42C00C"
#define DEFAULT_IMAGE_ATTR \
    "send [x=320,y=240] [x=640,y=480] recv [x=320,y=240] [x=640,y=480] [x=1280,y=720]"
#define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

class CodecAvcConfigTest : public ::testing::Test {
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
    AString GetString(IN const IMS_CHAR* pszKey) { return m_piCc->GetString(pszKey); }
    IMSVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetIntArray(pszKey);
    }
    IMSVector<AString> GetStringArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetStringArray(pszKey);
    }
};

TEST_F(CodecAvcConfigTest, GetConfigDefault)
{
    CodecAvcConfig* m_pConfig = new CodecAvcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
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
    delete m_pConfig;
}

TEST_F(CodecAvcConfigTest, GetConfigTest)
{
    CodecAvcConfig* m_pConfig = new CodecAvcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(m_piCc, 0));
    EXPECT_EQ(m_pConfig->GetFramerate(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT));
    EXPECT_EQ(m_pConfig->GetPacketizationMode(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT));
    EXPECT_EQ(m_pConfig->GetProfileLevelId(),
            GetString(
                    CarrierConfig::ImsVt::KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING));
    // EXPECT_EQ(m_pConfig->GetFrameSize(), DEFAULT_FRAME_SIZE);   // TODO - need to check later
    delete m_pConfig;
}

TEST_F(CodecAvcConfigTest, GetConfigVideoResolution)
{
    CodecAvcConfig* m_pConfig = new CodecAvcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    IMSVector<IMS_SINT32> objVideoCodecResolution;
    objVideoCodecResolution.Push(480);
    objVideoCodecResolution.Push(640);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY))
            .WillByDefault(Return(objVideoCodecResolution));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetResolutionWidth(), 480);
    EXPECT_EQ(m_pConfig->GetResolutionHeight(), 640);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(CodecAvcConfigTest, GetConfigVideoBitrate)
{
    CodecAvcConfig* m_pConfig = new CodecAvcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    IMSVector<IMS_SINT32> objVideoBitrate;
    objVideoBitrate.Push(512);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY))
            .WillByDefault(Return(objVideoBitrate));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetBitrate(), 512);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(CodecAvcConfigTest, GetConfigVideoImageAttr)
{
    CodecAvcConfig* m_pConfig = new CodecAvcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    IMSVector<AString> objImageAttr;
    objImageAttr.Push("send [x=640,y=480] recv [x=640,y=480]");

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY))
            .WillByDefault(Return(objImageAttr));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig->GetImageAttr(), "send [x=640,y=480] recv [x=640,y=480]");

    delete pMockICarrierConfig;
    delete m_pConfig;
}