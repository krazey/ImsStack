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
#include <config/CodecAvcConfig.h>
using ::testing::Return;

static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::VIDEO_AVC;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;

static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = CodecAvcConfig::DEFAULT_RESOLUTION_WIDTH;
static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = CodecAvcConfig::DEFAULT_RESOLUTION_HEIGHT;
static const IMS_SINT32 DEFAULT_FRAMERATE = CodecAvcConfig::DEFAULT_FRAMERATE;
static const IMS_SINT32 DEFAULT_BITRATE = CodecAvcConfig::DEFAULT_BITRATE;
static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = CodecAvcConfig::DEFAULT_PACKETIZATION_MODE;
static const IMS_BOOL DEFAULT_INCLUDE_SPROP = CodecAvcConfig::DEFAULT_INCLUDE_SPROP;
#define DEFAULT_PROFILE_ID "42C00C"
#define DEFAULT_IMAGE_ATTR "NEED_TO_CHECK"
#define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

class CodecAvcConfigTest : public ::testing::Test {

public :
    CodecAvcConfig* pConfig;

protected:
    virtual void SetUp() override {
        pConfig = new CodecAvcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(CodecAvcConfigTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetResolutionWidth(), DEFAULT_RESOLUTION_WIDTH);
    EXPECT_EQ(pConfig->GetResolutionHeight(), DEFAULT_RESOLUTION_HEIGHT);
    EXPECT_EQ(pConfig->GetFramerate(), DEFAULT_FRAMERATE);
    EXPECT_EQ(pConfig->GetBitrate(), DEFAULT_BITRATE);
    EXPECT_EQ(pConfig->GetPacketizationMode(), DEFAULT_PACKETIZATION_MODE);
    EXPECT_EQ(pConfig->GetIncludeSpropParameterSets(), DEFAULT_INCLUDE_SPROP);
    EXPECT_EQ(pConfig->GetProfileLevelId(), DEFAULT_PROFILE_ID);
    EXPECT_EQ(pConfig->GetImageAttr(), DEFAULT_IMAGE_ATTR);
    EXPECT_EQ(pConfig->GetFrameSize(), DEFAULT_FRAME_SIZE);
}
