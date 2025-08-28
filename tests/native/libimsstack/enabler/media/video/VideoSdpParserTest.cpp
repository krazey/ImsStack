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
#include <memory>

#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"
#include "video/VideoDef.h"
#include "video/VideoProfile.h"
#include "video/VideoSdpParser.h"
#include "SdpMedia.h"
#include "core/media/MockIMediaDescriptor.h"
#include "SdpBandwidth.h"
#include "core/MockISessionDescriptor.h"

using ::testing::_;
using ::testing::An;
using ::testing::Return;
using ::testing::ReturnRef;

class VideoSdpParserTest : public VideoSdpParser, public ::testing::Test
{
protected:
    std::unique_ptr<VideoProfile> m_pProfile;
    MockIMediaDescriptor m_objMockMediaDescriptor;
    std::unique_ptr<VideoSdpParser> mVideoSdpParser;

    void SetUp() override { mVideoSdpParser = std::make_unique<VideoSdpParser>(); }
    void TearDown() override { mVideoSdpParser.reset(); }
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

TEST_F(VideoSdpParserTest, ParsePayloads_InvalidPayloadType)
{
    VideoProfile profile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with a static payload type (8), which should be rejected.
    // Dynamic payload types are in the range [96, 127].
    SdpAvCodec* pStaticCodec = new SdpAvCodec();
    pStaticCodec->SetParameters("8 PCMA/8000", "");
    lstMediaFormats.Append(pStaticCodec);

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    // Call the method under test
    ParsePayloads(&m_objMockMediaDescriptor, &profile);

    // Expect no payloads to be added because the payload type is not dynamic.
    EXPECT_EQ(profile.GetPayloadList().GetSize(), 0);
}

TEST_F(VideoSdpParserTest, testParsePayloadTypeNumber)
{
    MockISessionDescriptor sessionDescriptor;
    MockIMediaDescriptor mediaDescriptor;
    VideoProfile videoProfile;

    SdpMedia sdpMedia;
    AStringArray formats;
    formats.AddElement("96");
    formats.AddElement("97");
    sdpMedia.SetFormats(formats);

    ImsList<SdpMediaFormat*> emptyMediaFormats;

    ON_CALL(mediaDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&sdpMedia));
    ON_CALL(mediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(emptyMediaFormats));
    ON_CALL(mediaDescriptor, GetAttributes(_, _)).WillByDefault(Return(ImsList<AString>()));
    ON_CALL(mediaDescriptor, GetAttributeInt(_, _))
            .WillByDefault(Return(IMediaDescriptor::INVALID_VALUE));
    ON_CALL(mediaDescriptor, GetRemoteAddress()).WillByDefault(Return(IpAddress::NONE));
    ON_CALL(mediaDescriptor, GetRemotePort()).WillByDefault(Return(0));
    ON_CALL(mediaDescriptor, GetBandwidth(An<IMS_SINT32>(), _)).WillByDefault(Return(0));
    ON_CALL(mediaDescriptor, GetDirection()).WillByDefault(Return(MEDIA_DIRECTION_INVALID));
    ON_CALL(sessionDescriptor, GetDirection()).WillByDefault(Return(MEDIA_DIRECTION_INVALID));

    IMS_BOOL result = mVideoSdpParser->Parse(&sessionDescriptor, &mediaDescriptor, &videoProfile);

    ASSERT_TRUE(result);
    ASSERT_EQ(videoProfile.GetPayloadList().GetSize(), 2);

    VideoProfile::Payload* payload1 = videoProfile.GetPayloadAt(0);
    ASSERT_NE(payload1, nullptr);
    EXPECT_EQ(payload1->GetRtpMap().GetPayloadNumber(), 96);
    EXPECT_EQ(payload1->GetFmtp(), nullptr);

    VideoProfile::Payload* payload2 = videoProfile.GetPayloadAt(1);
    ASSERT_NE(payload2, nullptr);
    EXPECT_EQ(payload2->GetRtpMap().GetPayloadNumber(), 97);
    EXPECT_EQ(payload2->GetFmtp(), nullptr);
}
