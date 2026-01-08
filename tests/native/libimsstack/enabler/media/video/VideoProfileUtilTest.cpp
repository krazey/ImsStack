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
#include "video/VideoProfileUtil.h"

class VideoProfileUtilTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test cases for GetWidthHeightFromResolution
TEST_F(VideoProfileUtilTest, GetWidthHeightFromResolutionVgaLandscape)
{
    IMS_UINT32 nWidth = 0, nHeight = 0;
    VideoProfileUtil::GetWidthHeightFromResolution(VIDEO_RESOLUTION_VGA_LS, &nWidth, &nHeight);
    EXPECT_EQ(nWidth, 640);
    EXPECT_EQ(nHeight, 480);
}

TEST_F(VideoProfileUtilTest, GetWidthHeightFromResolutionHdPortrait)
{
    IMS_UINT32 nWidth = 0, nHeight = 0;
    VideoProfileUtil::GetWidthHeightFromResolution(VIDEO_RESOLUTION_HD_PR, &nWidth, &nHeight);
    EXPECT_EQ(nWidth, 720);
    EXPECT_EQ(nHeight, 1280);
}

TEST_F(VideoProfileUtilTest, GetWidthHeightFromResolutionInvalid)
{
    IMS_UINT32 nWidth = 1, nHeight = 1;  // Non-zero initial
    VideoProfileUtil::GetWidthHeightFromResolution(VIDEO_RESOLUTION_INVALID, &nWidth, &nHeight);
    EXPECT_EQ(nWidth, 0);
    EXPECT_EQ(nHeight, 0);
}

// Test cases for GetResolutionFromWidthHeight
TEST_F(VideoProfileUtilTest, GetResolutionFromWidthHeightQcifLandscape)
{
    EXPECT_EQ(VideoProfileUtil::GetResolutionFromWidthHeight(176, 144), VIDEO_RESOLUTION_QCIF_LS);
}

TEST_F(VideoProfileUtilTest, GetResolutionFromWidthHeightQvgaPortrait)
{
    EXPECT_EQ(VideoProfileUtil::GetResolutionFromWidthHeight(240, 320), VIDEO_RESOLUTION_QVGA_PR);
}

TEST_F(VideoProfileUtilTest, GetResolutionFromWidthHeightUnknown)
{
    EXPECT_EQ(VideoProfileUtil::GetResolutionFromWidthHeight(100, 100), VIDEO_RESOLUTION_INVALID);
}

// Test cases for GetAvcProfileFromProfileLevelId
TEST_F(VideoProfileUtilTest, GetAvcProfileFromProfileLevelIdBaseline)
{
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId("42e01f"), AVC_PROFILE_CB);
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId("42000C"), AVC_PROFILE_B);
}

TEST_F(VideoProfileUtilTest, GetAvcProfileFromProfileLevelIdMain)
{
    // "4D401f" -> profile-iop is '4' (0b0100). (nProfileIop & 0x08) is false, so it's Main.
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId("4D401f"), AVC_PROFILE_M);
    // "4DC01f" -> profile-iop is 'C' (0b1100). (nProfileIop & 0x08) is true, so it's Constrained
    // Baseline.
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId("4DC01f"), AVC_PROFILE_CB);
}

TEST_F(VideoProfileUtilTest, GetAvcProfileFromProfileLevelIdHigh)
{
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId("640028"), AVC_PROFILE_H);
}

TEST_F(VideoProfileUtilTest, GetAvcProfileFromProfileLevelIdInvalid)
{
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId(""), AVC_PROFILE_NOT_USED);
    EXPECT_EQ(VideoProfileUtil::GetAvcProfileFromProfileLevelId("123"), AVC_PROFILE_NOT_USED);
}

// Test cases for GetAvcLevelFromProfileLevelId
TEST_F(VideoProfileUtilTest, GetAvcLevelFromProfileLevelIdValid)
{
    EXPECT_EQ(VideoProfileUtil::GetAvcLevelFromProfileLevelId("42e01f"), 31);  // 1f hex = 31 dec
    EXPECT_EQ(VideoProfileUtil::GetAvcLevelFromProfileLevelId("4D400C"), 12);  // 0C hex = 12 dec
    EXPECT_EQ(VideoProfileUtil::GetAvcLevelFromProfileLevelId("64002A"), 42);  // 2A hex = 42 dec
}

TEST_F(VideoProfileUtilTest, GetAvcLevelFromProfileLevelIdInvalid)
{
    EXPECT_EQ(VideoProfileUtil::GetAvcLevelFromProfileLevelId("42e0"), 12);  // default
}

// Test cases for GetWidthHeightFromSpropParam
TEST_F(VideoProfileUtilTest, GetWidthHeightFromSpropParamAvcVgaPortrait)
{
    IMS_UINT32 nWidth = 0, nHeight = 0;
    // SPS for 480x640 (VGA Portrait)
    AString sprop = "Z0IAHuoPAo01AgICB4QCHA==,aMqPIA==";
    EXPECT_TRUE(VideoProfileUtil::GetWidthHeightFromSpropParam(
            VIDEO_CODEC_AVC, sprop, &nWidth, &nHeight));
    EXPECT_EQ(nWidth, 480);
    EXPECT_EQ(nHeight, 640);
}

TEST_F(VideoProfileUtilTest, GetWidthHeightFromSpropParamAvcQvgaPortrait)
{
    IMS_UINT32 nWidth = 0, nHeight = 0;
    // SPS for 240x320 (QVGA Portrait)
    AString sprop = "Z0LADdoPCmmoEBAQPFCqgA==,aM4NiA==";
    EXPECT_TRUE(VideoProfileUtil::GetWidthHeightFromSpropParam(
            VIDEO_CODEC_AVC, sprop, &nWidth, &nHeight));
    EXPECT_EQ(nWidth, 240);
    EXPECT_EQ(nHeight, 320);
}

TEST_F(VideoProfileUtilTest, GetWidthHeightFromSpropParamAvcCifLandscape)
{
    IMS_UINT32 nWidth = 0, nHeight = 0;
    // SPS for 352x288 (CIF Landscape)
    AString sprop = "J0LgDZWWFglk,KM4Ecg==";
    EXPECT_TRUE(VideoProfileUtil::GetWidthHeightFromSpropParam(
            VIDEO_CODEC_AVC, sprop, &nWidth, &nHeight));
    EXPECT_EQ(nWidth, 352);
    EXPECT_EQ(nHeight, 288);
}

TEST_F(VideoProfileUtilTest, GetWidthHeightFromSpropParamInvalid)
{
    IMS_UINT32 nWidth = 0, nHeight = 0;
    EXPECT_FALSE(
            VideoProfileUtil::GetWidthHeightFromSpropParam(VIDEO_CODEC_AVC, "", &nWidth, &nHeight));
    EXPECT_FALSE(VideoProfileUtil::GetWidthHeightFromSpropParam(
            VIDEO_CODEC_AVC, "invalid,base64", &nWidth, &nHeight));
    // HEVC not supported yet
    EXPECT_FALSE(VideoProfileUtil::GetWidthHeightFromSpropParam(
            VIDEO_CODEC_HEVC, "some,sprop", &nWidth, &nHeight));
}
