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

#include <memory>

#include <gtest/gtest.h>

#include "offeranswer/SdpAvCodec.h"
#include "video/VideoDef.h"
#include "video/VideoProfile.h"
#include "video/VideoSdpParser.h"
#include "SdpAttribute.h"
#include "SdpMedia.h"
#include "core/media/MockIMediaDescriptor.h"
#include "core/MockISessionDescriptor.h"

using ::testing::_;
using ::testing::An;
using ::testing::Return;
using ::testing::ReturnRef;

class VideoSdpParserTest : public ::testing::Test
{
protected:
    MockIMediaDescriptor m_objMockMediaDescriptor;
};

// A testable version of VideoSdpParser to expose protected methods for testing.
class TestableVideoSdpParser : public VideoSdpParser
{
public:
    using MediaSdpParser::ParsePayloadTypeNumber;
    using VideoSdpParser::GetResolutionFromSdp;
    using VideoSdpParser::ParseCvo;
    using VideoSdpParser::ParsePayloads;
};

// Test case for parsing resolution from 'imageattr'
TEST_F(VideoSdpParserTest, GetResolutionFromImageAttr)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "96 send [x=640,y=480] recv [x=640,y=480]";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_LS);
}

// Test case for parsing resolution from 'framesize' when 'imageattr' is absent
TEST_F(VideoSdpParserTest, GetResolutionFromFrameSize)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "96 352-288";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_LS);
}

// Test case for parsing resolution from 'sprop-parameter-sets'
TEST_F(VideoSdpParserTest, GetResolutionFromSpropParam)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    // SPS for 480x640 (VGA Portrait)
    pFmtp->SetSpropParam("Z0IAHuoPAo01AgICB4QCHA==,aMqPIA==");

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_PR);
}

// Test case for deriving resolution from AVC level
TEST_F(VideoSdpParserTest, GetResolutionFromLevelHD)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");  // No sprop
    pFmtp->SetLevel(31);       // Level 3.1 for HD

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_HD_PR);
}

// Test case for deriving resolution from AVC level
TEST_F(VideoSdpParserTest, GetResolutionFromLevelVGA)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");  // No sprop
    pFmtp->SetLevel(22);       // Level 2.2 for VGA

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_PR);
}

// Test case for deriving resolution from AVC level
TEST_F(VideoSdpParserTest, GetResolutionFromLevelCIF)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");  // No sprop
    pFmtp->SetLevel(11);       // Level 1.1 for CIF

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_PR);
}

// Test case for when no resolution attributes are present
TEST_F(VideoSdpParserTest, GetResolutionNoAttributes)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");
    pFmtp->SetLevel(0);  // Invalid level to ensure it doesn't resolve

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    // With level 0, it should default to VGA_PR
    EXPECT_EQ(result, VIDEO_RESOLUTION_NOT_USED);

    // Test with null fmtp
    result = objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, nullptr);
    EXPECT_EQ(result, VIDEO_RESOLUTION_NOT_USED);
}

// Test with invalid imageattr format
TEST_F(VideoSdpParserTest, GetResolutionFromInvalidImageAttr)
{
    AString strImageAttr = "96 send [x=640,y=480";  // Missing closing bracket
    TestableVideoSdpParser objParser;
    AString strFrameSize = "96 352-288";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    // Should fall back to framesize
    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_LS);
}

// Test with invalid framesize format
TEST_F(VideoSdpParserTest, GetResolutionFromInvalidFrameSize)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "96 352_288";  // Invalid separator
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetLevel(21);  // CIF

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    // Should fall back to level
    EXPECT_EQ(result, VIDEO_RESOLUTION_CIF_PR);
}

// Test with invalid sprop-parameter-sets
TEST_F(VideoSdpParserTest, GetResolutionFromInvalidSprop)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pFmtp->SetSpropParam("");
    pFmtp->SetLevel(30);  // VGA

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_AVC, strImageAttr, strFrameSize, pFmtp);

    // Should fall back to level
    EXPECT_EQ(result, VIDEO_RESOLUTION_VGA_PR);
}

// Test case for HEVC (currently not supported, should return NOT_USED)
TEST_F(VideoSdpParserTest, GetResolutionFromHevc)
{
    TestableVideoSdpParser objParser;
    AString strImageAttr = "";
    AString strFrameSize = "";
    auto pFmtp = std::make_shared<VideoProfile::HevcFmtp>();
    pFmtp->SetSpropParam("some_sprop");
    pFmtp->SetLevel(120);  // HEVC Main Tier, Level 4.0

    VIDEO_RESOLUTION result =
            objParser.GetResolutionFromSdp(VIDEO_CODEC_HEVC, strImageAttr, strFrameSize, pFmtp);

    EXPECT_EQ(result, VIDEO_RESOLUTION_NOT_USED);
}

TEST_F(VideoSdpParserTest, ParsePayloadsInvalidPayloadType)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with a static payload type (8), which should be rejected.
    // Dynamic payload types are in the range [96, 127].
    auto pStaticCodec = std::make_unique<SdpAvCodec>();
    pStaticCodec->SetParameters("8 PCMA/8000", "");
    lstMediaFormats.Append(pStaticCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    // Call the method under test
    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added because the payload type is not dynamic.
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(VideoSdpParserTest, ParsePayloadsInvalidCodecName)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with an unsupported name.
    auto pInvalidCodec = std::make_unique<SdpAvCodec>();
    pInvalidCodec->SetParameters("98 unsupported/90000", "");
    lstMediaFormats.Append(pInvalidCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    // Call the method under test
    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added because the codec name is not supported.
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(VideoSdpParserTest, ParseCvoValidCvoAttribute)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;
    ImsList<AString> lstExtmapAttr;
    lstExtmapAttr.Append("3 urn:3gpp:video-orientation");

    ON_CALL(m_objMockMediaDescriptor,
            GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, AString("extmap")))
            .WillByDefault(Return(lstExtmapAttr));

    objParser.ParseCvo(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetCvoId(), 3);
}

TEST_F(VideoSdpParserTest, ParseCvoValidCvoAttributeWithDirection)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;
    ImsList<AString> lstExtmapAttr;
    lstExtmapAttr.Append("4/send urn:3gpp:video-orientation");

    ON_CALL(m_objMockMediaDescriptor,
            GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, AString("extmap")))
            .WillByDefault(Return(lstExtmapAttr));

    objParser.ParseCvo(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetCvoId(), 4);
}

TEST_F(VideoSdpParserTest, ParseCvoNoCvoAttribute)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;
    ImsList<AString> lstExtmapAttr;  // Empty list

    ON_CALL(m_objMockMediaDescriptor,
            GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, AString("extmap")))
            .WillByDefault(Return(lstExtmapAttr));

    objParser.ParseCvo(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetCvoId(), -1);  // Default value
}

TEST_F(VideoSdpParserTest, ParseCvoAttributeWithoutCvoUri)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;
    ImsList<AString> lstExtmapAttr;
    lstExtmapAttr.Append("5 some-other-uri");

    ON_CALL(m_objMockMediaDescriptor,
            GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, AString("extmap")))
            .WillByDefault(Return(lstExtmapAttr));

    objParser.ParseCvo(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetCvoId(), -1);  // Should not be parsed
}

TEST_F(VideoSdpParserTest, ParsePayloadTypeNumber)
{
    TestableVideoSdpParser objParser;
    VideoProfile objProfile;

    SdpMedia sdpMedia;
    AStringArray formats;
    formats.AddElement("96");
    formats.AddElement("97");
    sdpMedia.SetFormats(formats);
    // Set a transport protocol to avoid issues with default values.
    sdpMedia.SetTransportProtocol(SdpMedia::TRANSPORT_RTP_AVP, "RTP/AVP");

    ImsList<SdpMediaFormat*> emptyMediaFormats;

    ON_CALL(m_objMockMediaDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&sdpMedia));
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(emptyMediaFormats));
    ON_CALL(m_objMockMediaDescriptor, GetAttributes(_, _))
            .WillByDefault(Return(ImsList<AString>()));
    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(_, _))
            .WillByDefault(Return(IMediaDescriptor::INVALID_VALUE));
    ON_CALL(m_objMockMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(IpAddress::NONE));
    ON_CALL(m_objMockMediaDescriptor, GetRemotePort()).WillByDefault(Return(0));
    ON_CALL(m_objMockMediaDescriptor, GetBandwidth(An<IMS_SINT32>(), _)).WillByDefault(Return(0));
    ON_CALL(m_objMockMediaDescriptor, GetDirection())
            .WillByDefault(Return(MEDIA_DIRECTION_INVALID));

    // The main Parse method calls ParsePayloadTypeNumber internally when GetMediaFormats is empty.
    MockISessionDescriptor sessionDescriptor;
    ON_CALL(sessionDescriptor, GetDirection()).WillByDefault(Return(MEDIA_DIRECTION_INVALID));
    IMS_BOOL result = objParser.Parse(&sessionDescriptor, &m_objMockMediaDescriptor, &objProfile);

    ASSERT_TRUE(result);
    ASSERT_EQ(objProfile.GetPayloadList().GetSize(), 2);

    VideoProfile::Payload* payload1 = objProfile.GetPayloadAt(0);
    ASSERT_NE(payload1, nullptr);
    EXPECT_EQ(payload1->GetRtpMap().GetPayloadNumber(), 96);
    EXPECT_EQ(payload1->GetFmtp(), nullptr);

    VideoProfile::Payload* payload2 = objProfile.GetPayloadAt(1);
    ASSERT_NE(payload2, nullptr);
    EXPECT_EQ(payload2->GetRtpMap().GetPayloadNumber(), 97);
    EXPECT_EQ(payload2->GetFmtp(), nullptr);
}
