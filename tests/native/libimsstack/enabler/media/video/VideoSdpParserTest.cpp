/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "video/VideoDef.h"
#include "video/VideoSdpParser.h"
#include "video/VideoProfile.h"

class VideoSdpParserTest : public VideoSdpParser, public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test case for parsing resolution from 'imageattr'
TEST_F(VideoSdpParserTest, testGetResolutionFromImageAttr)
{
    AString imageAttr = "96 send [x=640,y=480] recv [x=640,y=480]";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_LS);
}

// Test case for parsing resolution from 'framesize' when 'imageattr' is absent
TEST_F(VideoSdpParserTest, testGetResolutionFromFrameSize)
{
    AString imageAttr = "";
    AString frameSize = "96 352-288";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_LS);
}

// Test case for parsing resolution from 'sprop-parameter-sets'
TEST_F(VideoSdpParserTest, testGetResolutionFromSpropParam)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    // SPS for 480x640 (VGA Portrait)
    pFmtp->SetSpropParam("Z0IAHuoPAo01AgICB4QCHA==,aMqPIA==");

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_PR);
}

// Test case for deriving resolution from AVC level
TEST_F(VideoSdpParserTest, testGetResolutionFromLevelHD)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");  // No sprop
    pFmtp->SetLevel(31);       // Level 3.1 for HD

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_HD_PR);
}

// Test case for deriving resolution from AVC level
TEST_F(VideoSdpParserTest, testGetResolutionFromLevelVGA)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");  // No sprop
    pFmtp->SetLevel(22);       // Level 2.2 for VGA

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_PR);
}

// Test case for deriving resolution from AVC level
TEST_F(VideoSdpParserTest, testGetResolutionFromLevelCIF)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");  // No sprop
    pFmtp->SetLevel(11);       // Level 1.1 for CIF

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_PR);
}

// Test case for when no resolution attributes are present
TEST_F(VideoSdpParserTest, testGetResolutionNoAttributes)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");
    pFmtp->SetLevel(0);  // Invalid level to ensure it doesn't resolve

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    // With level 0, it should default to VGA_PR
    EXPECT_EQ(result, VIDEO_RESOLUTION_NOT_USED);

    // Test with null fmtp
    result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, nullptr);
    EXPECT_EQ(result, VIDEO_RESOLUTION_NOT_USED);
}

// Test with invalid imageattr format
TEST_F(VideoSdpParserTest, testGetResolutionFromInvalidImageAttr)
{
    AString imageAttr = "96 send [x=640,y=480";  // Missing closing bracket
    AString frameSize = "96 352-288";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    // Should fall back to framesize
    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_LS);
}

// Test with invalid framesize format
TEST_F(VideoSdpParserTest, testGetResolutionFromInvalidFrameSize)
{
    AString imageAttr = "";
    AString frameSize = "96 352_288";  // Invalid separator
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetLevel(21);  // CIF

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    // Should fall back to level
    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_PR);
}

// Test with invalid sprop-parameter-sets
TEST_F(VideoSdpParserTest, testGetResolutionFromInvalidSprop)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");
    pFmtp->SetLevel(30);  // VGA

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_AVC, imageAttr, frameSize, pFmtp);

    // Should fall back to level
    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_PR);
}

// Test case for HEVC (currently not supported, should return NOT_USED)
TEST_F(VideoSdpParserTest, testGetResolutionFromHevc)
{
    AString imageAttr = "";
    AString frameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::HevcFmtp>();
    pFmtp->SetSpropParam("some_sprop");
    pFmtp->SetLevel(120);  // HEVC Main Tier, Level 4.0

    VIDEO_RESOLUTION result = GetResolutionFromSdp(VIDEO_CODEC_HEVC, imageAttr, frameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_NOT_USED);
}
