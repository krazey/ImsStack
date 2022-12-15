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
#include <ServiceConfig.h>

#include "config/ImsCodec.h"

static const IMS_SINT32 CODEC_AMR = ImsCodec::AUDIO_AMR;
static const IMS_SINT32 CODEC_AMR_WB = ImsCodec::AUDIO_AMR_WB;
static const IMS_CHAR* RETURN_AMR_STRING = "AMR";
static const IMS_CHAR* RETURN_AMR_WB_STRING = "AMR-WB";

class ImsCodecTest : public ::testing::Test
{
};

TEST_F(ImsCodecTest, GetConfigDefault)
{
    EXPECT_STREQ(ImsCodec::CodecToString(CODEC_AMR), RETURN_AMR_STRING);
    EXPECT_STREQ(ImsCodec::CodecToString(CODEC_AMR_WB), RETURN_AMR_WB_STRING);
}
