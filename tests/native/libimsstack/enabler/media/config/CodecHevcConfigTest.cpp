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
#include <config/CodecHevcConfig.h>
using ::testing::Return;

static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::VIDEO_HEVC;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;

static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = CodecHevcConfig::DEFAULT_RESOLUTION_WIDTH;
static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = CodecHevcConfig::DEFAULT_RESOLUTION_HEIGHT;
static const IMS_SINT32 DEFAULT_FRAMERATE = CodecHevcConfig::DEFAULT_FRAMERATE;
static const IMS_SINT32 DEFAULT_BITRATE = CodecHevcConfig::DEFAULT_BITRATE;
static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = CodecHevcConfig::DEFAULT_PACKETIZATION_MODE;
static const IMS_BOOL DEFAULT_INCLUDE_SPROP = CodecHevcConfig::DEFAULT_INCLUDE_SPROP;
static const IMS_SINT32 DEFAULT_HEVC_PROFILE = CodecHevcConfig::DEFAULT_HEVC_PROFILE;
static const IMS_SINT32 DEFAULT_HEVC_LEVEL = CodecHevcConfig::DEFAULT_HEVC_LEVEL;
#define DEFAULT_IMAGE_ATTR "NEED_TO_CHECK"
#define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

class CodecHevcConfigTest : public ::testing::Test {

public :
    CodecHevcConfig* pConfig;

protected:
    virtual void SetUp() override {
        pConfig = new CodecHevcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(CodecHevcConfigTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetResolutionWidth(), DEFAULT_RESOLUTION_WIDTH);
    EXPECT_EQ(pConfig->GetResolutionHeight(), DEFAULT_RESOLUTION_HEIGHT);
    EXPECT_EQ(pConfig->GetFramerate(), DEFAULT_FRAMERATE);
    EXPECT_EQ(pConfig->GetBitrate(), DEFAULT_BITRATE);
    EXPECT_EQ(pConfig->GetPacketizationMode(), DEFAULT_PACKETIZATION_MODE);
    EXPECT_EQ(pConfig->GetIncludeSpropParameterSets(), DEFAULT_INCLUDE_SPROP);
    EXPECT_EQ(pConfig->GetHevcProfile(), DEFAULT_HEVC_PROFILE);
    EXPECT_EQ(pConfig->GetHevcLevel(), DEFAULT_HEVC_LEVEL);
    EXPECT_EQ(pConfig->GetImageAttr(), DEFAULT_IMAGE_ATTR);
    EXPECT_EQ(pConfig->GetFrameSize(), DEFAULT_FRAME_SIZE);
}
