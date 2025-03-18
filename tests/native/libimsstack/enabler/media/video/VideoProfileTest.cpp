/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <video/VideoProfile.h>

const AString AVC_PAYLOAD_TYPE = "H264";
const AString HEVC_PAYLOAD_TYPE = "H265";
const VIDEO_RESOLUTION VIDEO_FMTP_RESOLUTION = VIDEO_RESOLUTION_QVGA_PR;
const IMS_SINT32 VIDEO_FMTP_BITRATE = 1;
const IMS_SINT32 VIDEO_FMTP_FRAMERATE = 33;
const IMS_SINT32 VIDEO_FMTP_AS = 560;
const IMS_UINT32 VIDEO_FMTP_PROFILE = 1;
const IMS_UINT32 VIDEO_FMTP_LEVEL = 93;
const IMS_SINT32 VIDEO_FMTP_PACKETIZATION_MODE = 0;
const AString VIDEO_FMTP_SPROP = "Z0LADNoPCmQ=,aM4G8g==";
const IMS_BOOL VIDEO_FMTP_SHOW_PACKETIZATION_MODE = IMS_TRUE;
const IMS_BOOL VIDEO_FMTP_SHOW_SPROP = IMS_TRUE;
const AString AVC_FMTP_PROFILE_LEVEL_ID = "42C00C";
const IMS_BOOL AVC_FMTP_SHOW_PROFILE_LEVEL_ID = IMS_TRUE;
const IMS_BOOL HEVC_FMTP_SHOW_PROFILE = IMS_TRUE;
const IMS_BOOL HEVC_FMTP_SHOW_LEVEL = IMS_TRUE;
const IMS_BOOL RTCP_FB_TRR_SUPPORTED = IMS_TRUE;
const IMS_SINT32 RTCP_FB_TRR_INT = 1;
const IMS_BOOL RTCP_FB_NACK_SUPPORTED = IMS_TRUE;
const IMS_BOOL RTCP_FB_TMMBR_SUPPORTED = IMS_TRUE;
const IMS_SINT32 RTCP_FB_TMMBR_SMAX = 1;
const IMS_BOOL RTCP_FB_PLI_SUPPORTED = IMS_TRUE;
const IMS_BOOL RTCP_FB_FIR_SUPPORTED = IMS_TRUE;
const IMS_BOOL VIDEO_PAYLOAD_INCLUDE_IMAGE_ATTR = IMS_TRUE;
const IMS_BOOL VIDEO_PAYLOAD_INCLUDE_FRAME_SIZE = IMS_TRUE;
const AString VIDEO_PAYLOAD_IMAGE_ATTR = "send [x=480,y=640] recv [x=480,y=640]";
const IMS_SINT32 VIDEO_PROFILE_FRAMERATE = 33;
const IMS_BOOL VIDEO_PROFILE_SUPPORT_AVPF = IMS_TRUE;
const IMS_SINT32 VIDEO_PROFILE_CVO_ID = 1;
const IMS_BOOL VIDEO_PROFILE_SUPPORT_CAPA_NEGO = IMS_TRUE;

class VideoProfileTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(VideoProfileTest, testVideoFmtpResolution)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetResolution(), VIDEO_RESOLUTION_INVALID);

    pFmtp->SetResolution(VIDEO_FMTP_RESOLUTION);
    EXPECT_EQ(pFmtp->GetResolution(), VIDEO_FMTP_RESOLUTION);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpBitrate)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetBitrate(), 0);

    pFmtp->SetBitrate(VIDEO_FMTP_BITRATE);
    EXPECT_EQ(pFmtp->GetBitrate(), VIDEO_FMTP_BITRATE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpFrameRate)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetFramerate(), 0);

    pFmtp->SetFramerate(VIDEO_FMTP_FRAMERATE);
    EXPECT_EQ(pFmtp->GetFramerate(), VIDEO_FMTP_FRAMERATE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpAs)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetAs(), 0);

    pFmtp->SetAs(VIDEO_FMTP_AS);
    EXPECT_EQ(pFmtp->GetAs(), VIDEO_FMTP_AS);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpProfile)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetProfile(), 0);

    pFmtp->SetProfile(VIDEO_FMTP_PROFILE);
    EXPECT_EQ(pFmtp->GetProfile(), VIDEO_FMTP_PROFILE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpLevel)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetLevel(), 0);

    pFmtp->SetLevel(VIDEO_FMTP_LEVEL);
    EXPECT_EQ(pFmtp->GetLevel(), VIDEO_FMTP_LEVEL);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpSprop)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->GetSpropParam(), AString::ConstNull());

    pFmtp->SetSpropParam(VIDEO_FMTP_SPROP);
    EXPECT_EQ(pFmtp->GetSpropParam(), VIDEO_FMTP_SPROP);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpShowPacketizationMode)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->IsPacketizationModeVisible(), IMS_FALSE);

    pFmtp->SetShowPacketizationMode(VIDEO_FMTP_SHOW_PACKETIZATION_MODE);
    EXPECT_EQ(pFmtp->IsPacketizationModeVisible(), VIDEO_FMTP_SHOW_PACKETIZATION_MODE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpShowSpropParam)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();
    EXPECT_EQ(pFmtp->IsSpropParamVisible(), IMS_FALSE);

    pFmtp->SetShowSpropParam(VIDEO_FMTP_SHOW_SPROP);
    EXPECT_EQ(pFmtp->IsSpropParamVisible(), VIDEO_FMTP_SHOW_SPROP);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpCreationDefault)
{
    VideoProfile::VideoFmtp* pFmtp = new VideoProfile::VideoFmtp();

    EXPECT_EQ(pFmtp->GetResolution(), VIDEO_RESOLUTION_INVALID);
    EXPECT_EQ(pFmtp->GetBitrate(), 0);
    EXPECT_EQ(pFmtp->GetFramerate(), 0);
    EXPECT_EQ(pFmtp->GetAs(), 0);
    EXPECT_EQ(pFmtp->GetProfile(), 0);
    EXPECT_EQ(pFmtp->GetLevel(), 0);
    EXPECT_EQ(pFmtp->GetSpropParam(), AString::ConstNull());
    EXPECT_EQ(pFmtp->IsPacketizationModeVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsSpropParamVisible(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testVideoFmtpCreation)
{
    VideoProfile::VideoFmtp* pFmtp1 = new VideoProfile::VideoFmtp(VIDEO_FMTP_RESOLUTION,
            VIDEO_FMTP_BITRATE, VIDEO_FMTP_FRAMERATE, VIDEO_FMTP_AS, VIDEO_FMTP_PROFILE,
            VIDEO_FMTP_LEVEL, VIDEO_FMTP_PACKETIZATION_MODE, VIDEO_FMTP_SPROP);

    EXPECT_EQ(pFmtp1->GetResolution(), VIDEO_FMTP_RESOLUTION);
    EXPECT_EQ(pFmtp1->GetBitrate(), VIDEO_FMTP_BITRATE);
    EXPECT_EQ(pFmtp1->GetFramerate(), VIDEO_FMTP_FRAMERATE);
    EXPECT_EQ(pFmtp1->GetAs(), VIDEO_FMTP_AS);
    EXPECT_EQ(pFmtp1->GetProfile(), VIDEO_FMTP_PROFILE);
    EXPECT_EQ(pFmtp1->GetLevel(), VIDEO_FMTP_LEVEL);
    EXPECT_EQ(pFmtp1->GetSpropParam(), VIDEO_FMTP_SPROP);
    EXPECT_EQ(pFmtp1->IsPacketizationModeVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp1->IsSpropParamVisible(), IMS_FALSE);

    VideoProfile::VideoFmtp* pFmtp2 = new VideoProfile::VideoFmtp(pFmtp1);

    EXPECT_EQ(pFmtp2->GetResolution(), VIDEO_FMTP_RESOLUTION);
    EXPECT_EQ(pFmtp2->GetBitrate(), VIDEO_FMTP_BITRATE);
    EXPECT_EQ(pFmtp2->GetFramerate(), VIDEO_FMTP_FRAMERATE);
    EXPECT_EQ(pFmtp2->GetAs(), VIDEO_FMTP_AS);
    EXPECT_EQ(pFmtp2->GetProfile(), VIDEO_FMTP_PROFILE);
    EXPECT_EQ(pFmtp2->GetLevel(), VIDEO_FMTP_LEVEL);
    EXPECT_EQ(pFmtp2->GetSpropParam(), VIDEO_FMTP_SPROP);
    EXPECT_EQ(pFmtp2->IsPacketizationModeVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp2->IsSpropParamVisible(), IMS_FALSE);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(VideoProfileTest, testAvcFmtpProfileLevelId)
{
    VideoProfile::AvcFmtp* pFmtp = new VideoProfile::AvcFmtp();
    EXPECT_EQ(pFmtp->GetProfileLevelId(), AString::ConstNull());

    pFmtp->SetProfileLevelId(AVC_FMTP_PROFILE_LEVEL_ID);
    EXPECT_EQ(pFmtp->GetProfileLevelId(), AVC_FMTP_PROFILE_LEVEL_ID);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testAvcFmtpShowProfileLevelId)
{
    VideoProfile::AvcFmtp* pFmtp = new VideoProfile::AvcFmtp();
    EXPECT_EQ(pFmtp->IsProfileLevelIdVisible(), IMS_FALSE);

    pFmtp->SetShowProfileLevelId(AVC_FMTP_SHOW_PROFILE_LEVEL_ID);
    EXPECT_EQ(pFmtp->IsProfileLevelIdVisible(), AVC_FMTP_SHOW_PROFILE_LEVEL_ID);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testAvcFmtpCreationDefault)
{
    VideoProfile::AvcFmtp* pFmtp = new VideoProfile::AvcFmtp();

    EXPECT_EQ(pFmtp->GetProfileLevelId(), AString::ConstNull());
    EXPECT_EQ(pFmtp->IsProfileLevelIdVisible(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testAvcFmtpCreation)
{
    VideoProfile::AvcFmtp* pFmtp1 =
            new VideoProfile::AvcFmtp(VIDEO_FMTP_RESOLUTION, VIDEO_FMTP_BITRATE,
                    VIDEO_FMTP_FRAMERATE, VIDEO_FMTP_AS, VIDEO_FMTP_PROFILE, VIDEO_FMTP_LEVEL,
                    AVC_FMTP_PROFILE_LEVEL_ID, VIDEO_FMTP_PACKETIZATION_MODE, VIDEO_FMTP_SPROP);

    EXPECT_EQ(pFmtp1->GetProfileLevelId(), AVC_FMTP_PROFILE_LEVEL_ID);
    EXPECT_EQ(pFmtp1->IsProfileLevelIdVisible(), IMS_FALSE);

    pFmtp1->SetShowProfileLevelId(AVC_FMTP_SHOW_PROFILE_LEVEL_ID);

    VideoProfile::AvcFmtp* pFmtp2 = new VideoProfile::AvcFmtp(pFmtp1);

    EXPECT_EQ(pFmtp2->GetProfileLevelId(), AVC_FMTP_PROFILE_LEVEL_ID);
    EXPECT_EQ(pFmtp2->IsProfileLevelIdVisible(), AVC_FMTP_SHOW_PROFILE_LEVEL_ID);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(VideoProfileTest, testHevcFmtpShowProfile)
{
    VideoProfile::HevcFmtp* pFmtp = new VideoProfile::HevcFmtp();
    EXPECT_EQ(pFmtp->IsProfileVisible(), IMS_FALSE);

    pFmtp->SetShowProfile(HEVC_FMTP_SHOW_PROFILE);
    EXPECT_EQ(pFmtp->IsProfileVisible(), HEVC_FMTP_SHOW_PROFILE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testHevcFmtpShowLevel)
{
    VideoProfile::HevcFmtp* pFmtp = new VideoProfile::HevcFmtp();
    EXPECT_EQ(pFmtp->IsLevelVisible(), IMS_FALSE);

    pFmtp->SetShowLevel(HEVC_FMTP_SHOW_LEVEL);
    EXPECT_EQ(pFmtp->IsLevelVisible(), HEVC_FMTP_SHOW_LEVEL);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testHevcFmtpCreationDefault)
{
    VideoProfile::HevcFmtp* pFmtp = new VideoProfile::HevcFmtp();
    EXPECT_EQ(pFmtp->IsProfileVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsLevelVisible(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(VideoProfileTest, testHevcFmtpCreation)
{
    VideoProfile::HevcFmtp* pFmtp1 = new VideoProfile::HevcFmtp(VIDEO_FMTP_RESOLUTION,
            VIDEO_FMTP_BITRATE, VIDEO_FMTP_FRAMERATE, VIDEO_FMTP_AS, VIDEO_FMTP_PROFILE,
            VIDEO_FMTP_LEVEL, VIDEO_FMTP_PACKETIZATION_MODE, VIDEO_FMTP_SPROP);

    EXPECT_EQ(pFmtp1->IsProfileVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp1->IsLevelVisible(), IMS_FALSE);

    pFmtp1->SetShowProfile(HEVC_FMTP_SHOW_PROFILE);
    pFmtp1->SetShowLevel(HEVC_FMTP_SHOW_LEVEL);

    VideoProfile::HevcFmtp* pFmtp2 = new VideoProfile::HevcFmtp(pFmtp1);

    EXPECT_EQ(pFmtp2->IsProfileVisible(), HEVC_FMTP_SHOW_PROFILE);
    EXPECT_EQ(pFmtp2->IsLevelVisible(), HEVC_FMTP_SHOW_LEVEL);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesTrrSupported)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->IsTrrSupported(), IMS_FALSE);

    pRtcpFb->SetTrrSupported(RTCP_FB_TRR_SUPPORTED);
    EXPECT_EQ(pRtcpFb->IsTrrSupported(), RTCP_FB_TRR_SUPPORTED);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesTrrInt)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->GetTrrInt(), 0);

    pRtcpFb->SetTrrInt(RTCP_FB_TRR_INT);
    EXPECT_EQ(pRtcpFb->GetTrrInt(), RTCP_FB_TRR_INT);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesNackSupported)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->IsNackSupported(), IMS_FALSE);

    pRtcpFb->SetNackSupported(RTCP_FB_NACK_SUPPORTED);
    EXPECT_EQ(pRtcpFb->IsNackSupported(), RTCP_FB_NACK_SUPPORTED);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesTmmbrSupported)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->IsTmmbrSupported(), IMS_FALSE);

    pRtcpFb->SetTmmbrSupported(RTCP_FB_TMMBR_SUPPORTED);
    EXPECT_EQ(pRtcpFb->IsTmmbrSupported(), RTCP_FB_TMMBR_SUPPORTED);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesTmmbrSmaxPr)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->GetTmmbrSmaxPr(), -1);

    pRtcpFb->SetTmmbrSmaxPr(RTCP_FB_TMMBR_SMAX);
    EXPECT_EQ(pRtcpFb->GetTmmbrSmaxPr(), RTCP_FB_TMMBR_SMAX);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesPliSupported)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->IsPliSupported(), IMS_FALSE);

    pRtcpFb->SetPliSupported(RTCP_FB_PLI_SUPPORTED);
    EXPECT_EQ(pRtcpFb->IsPliSupported(), RTCP_FB_PLI_SUPPORTED);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesFirSupported)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb = new VideoProfile::RtcpFbAttributes();
    EXPECT_EQ(pRtcpFb->IsFirSupported(), IMS_FALSE);

    pRtcpFb->SetFirSupported(RTCP_FB_FIR_SUPPORTED);
    EXPECT_EQ(pRtcpFb->IsFirSupported(), RTCP_FB_FIR_SUPPORTED);

    delete pRtcpFb;
}

TEST_F(VideoProfileTest, testRtcpFbAttributesAssign)
{
    VideoProfile::RtcpFbAttributes* pRtcpFb1 = new VideoProfile::RtcpFbAttributes();
    pRtcpFb1->SetTrrSupported(RTCP_FB_TRR_SUPPORTED);
    pRtcpFb1->SetTrrInt(RTCP_FB_TRR_INT);
    pRtcpFb1->SetNackSupported(RTCP_FB_NACK_SUPPORTED);
    pRtcpFb1->SetTmmbrSupported(RTCP_FB_TMMBR_SUPPORTED);
    pRtcpFb1->SetTmmbrSmaxPr(RTCP_FB_TMMBR_SMAX);
    pRtcpFb1->SetPliSupported(RTCP_FB_PLI_SUPPORTED);
    pRtcpFb1->SetFirSupported(RTCP_FB_FIR_SUPPORTED);

    VideoProfile::RtcpFbAttributes* pRtcpFb2 = new VideoProfile::RtcpFbAttributes();
    *pRtcpFb2 = *pRtcpFb1;

    EXPECT_EQ(pRtcpFb2->IsTrrSupported(), RTCP_FB_TRR_SUPPORTED);
    EXPECT_EQ(pRtcpFb2->GetTrrInt(), RTCP_FB_TRR_INT);
    EXPECT_EQ(pRtcpFb2->IsNackSupported(), RTCP_FB_NACK_SUPPORTED);
    EXPECT_EQ(pRtcpFb2->IsTmmbrSupported(), RTCP_FB_TMMBR_SUPPORTED);
    EXPECT_EQ(pRtcpFb2->GetTmmbrSmaxPr(), RTCP_FB_TMMBR_SMAX);
    EXPECT_EQ(pRtcpFb2->IsPliSupported(), RTCP_FB_PLI_SUPPORTED);
    EXPECT_EQ(pRtcpFb2->IsFirSupported(), RTCP_FB_FIR_SUPPORTED);

    delete pRtcpFb1;
    delete pRtcpFb2;
}

TEST_F(VideoProfileTest, testVideoPayloadIncludeImageAttr)
{
    VideoProfile::Payload* pPayload = new VideoProfile::Payload();
    EXPECT_EQ(pPayload->IsImageAttrIncluded(), IMS_FALSE);

    pPayload->SetIncludeImageAttr(VIDEO_PAYLOAD_INCLUDE_IMAGE_ATTR);
    EXPECT_EQ(pPayload->IsImageAttrIncluded(), VIDEO_PAYLOAD_INCLUDE_IMAGE_ATTR);

    delete pPayload;
}

TEST_F(VideoProfileTest, testVideoPayloadIncludeFrameSize)
{
    VideoProfile::Payload* pPayload = new VideoProfile::Payload();
    EXPECT_EQ(pPayload->IsFrameSizeIncluded(), IMS_FALSE);

    pPayload->SetIncludeFrameSize(VIDEO_PAYLOAD_INCLUDE_FRAME_SIZE);
    EXPECT_EQ(pPayload->IsFrameSizeIncluded(), VIDEO_PAYLOAD_INCLUDE_FRAME_SIZE);

    delete pPayload;
}

TEST_F(VideoProfileTest, testVideoPayloadImageAttr)
{
    VideoProfile::Payload* pPayload = new VideoProfile::Payload();
    EXPECT_EQ(pPayload->GetImageAttr(), AString::ConstNull());

    pPayload->SetImageAttr(VIDEO_PAYLOAD_IMAGE_ATTR);
    EXPECT_EQ(pPayload->GetImageAttr(), VIDEO_PAYLOAD_IMAGE_ATTR);

    delete pPayload;
}

TEST_F(VideoProfileTest, testVideoPayloadRtcpFbAttr)
{
    VideoProfile::Payload* pPayload = new VideoProfile::Payload();
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsTrrSupported(), IMS_FALSE);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().GetTrrInt(), 0);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsNackSupported(), IMS_FALSE);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsTmmbrSupported(), IMS_FALSE);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().GetTmmbrSmaxPr(), -1);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsPliSupported(), IMS_FALSE);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsFirSupported(), IMS_FALSE);

    VideoProfile::RtcpFbAttributes objRtcpFb;
    objRtcpFb.SetTrrSupported(RTCP_FB_TRR_SUPPORTED);
    objRtcpFb.SetTrrInt(RTCP_FB_TRR_INT);
    objRtcpFb.SetNackSupported(RTCP_FB_NACK_SUPPORTED);
    objRtcpFb.SetTmmbrSupported(RTCP_FB_TMMBR_SUPPORTED);
    objRtcpFb.SetTmmbrSmaxPr(RTCP_FB_TMMBR_SMAX);
    objRtcpFb.SetPliSupported(RTCP_FB_PLI_SUPPORTED);
    objRtcpFb.SetFirSupported(RTCP_FB_FIR_SUPPORTED);

    pPayload->SetRtcpFbAttr(objRtcpFb);

    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsTrrSupported(), RTCP_FB_TRR_SUPPORTED);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().GetTrrInt(), RTCP_FB_TRR_INT);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsNackSupported(), RTCP_FB_NACK_SUPPORTED);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsTmmbrSupported(), RTCP_FB_TMMBR_SUPPORTED);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().GetTmmbrSmaxPr(), RTCP_FB_TMMBR_SMAX);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsPliSupported(), RTCP_FB_PLI_SUPPORTED);
    EXPECT_EQ(pPayload->GetRtcpFbAttr().IsFirSupported(), RTCP_FB_FIR_SUPPORTED);

    delete pPayload;
}

TEST_F(VideoProfileTest, testVideoPayloadCreation)
{
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();

    VideoProfile::RtcpFbAttributes objRtcpFb;
    objRtcpFb.SetTrrSupported(RTCP_FB_TRR_SUPPORTED);
    objRtcpFb.SetTrrInt(RTCP_FB_TRR_INT);
    objRtcpFb.SetNackSupported(RTCP_FB_NACK_SUPPORTED);
    objRtcpFb.SetTmmbrSupported(RTCP_FB_TMMBR_SUPPORTED);
    objRtcpFb.SetTmmbrSmaxPr(RTCP_FB_TMMBR_SMAX);
    objRtcpFb.SetPliSupported(RTCP_FB_PLI_SUPPORTED);
    objRtcpFb.SetFirSupported(RTCP_FB_FIR_SUPPORTED);

    pPayload1->SetIncludeImageAttr(VIDEO_PAYLOAD_INCLUDE_IMAGE_ATTR);
    pPayload1->SetIncludeFrameSize(VIDEO_PAYLOAD_INCLUDE_FRAME_SIZE);
    pPayload1->SetImageAttr(VIDEO_PAYLOAD_IMAGE_ATTR);
    pPayload1->SetRtcpFbAttr(objRtcpFb);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload(*pPayload1);

    EXPECT_EQ(pPayload2->IsImageAttrIncluded(), VIDEO_PAYLOAD_INCLUDE_IMAGE_ATTR);
    EXPECT_EQ(pPayload2->IsFrameSizeIncluded(), VIDEO_PAYLOAD_INCLUDE_FRAME_SIZE);
    EXPECT_EQ(pPayload2->GetImageAttr(), VIDEO_PAYLOAD_IMAGE_ATTR);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().IsTrrSupported(), RTCP_FB_TRR_SUPPORTED);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().GetTrrInt(), RTCP_FB_TRR_INT);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().IsNackSupported(), RTCP_FB_NACK_SUPPORTED);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().IsTmmbrSupported(), RTCP_FB_TMMBR_SUPPORTED);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().GetTmmbrSmaxPr(), RTCP_FB_TMMBR_SMAX);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().IsPliSupported(), RTCP_FB_PLI_SUPPORTED);
    EXPECT_EQ(pPayload2->GetRtcpFbAttr().IsFirSupported(), RTCP_FB_FIR_SUPPORTED);

    delete pPayload1;
    delete pPayload2;
}

TEST_F(VideoProfileTest, testVideoPayloadCreationForAvcFmtp)
{
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AVC_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    VideoProfile::AvcFmtp* pAvcFmtp = new VideoProfile::AvcFmtp();
    pPayload1->SetFmtp(pAvcFmtp);

    VideoProfile::Payload* pPayload3 = new VideoProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<VideoProfile::AvcFmtp*>(pPayload3->GetFmtp())
            ->SetShowProfileLevelId(AVC_FMTP_SHOW_PROFILE_LEVEL_ID);
    EXPECT_EQ(static_cast<VideoProfile::AvcFmtp*>(pPayload3->GetFmtp())->IsProfileLevelIdVisible(),
            AVC_FMTP_SHOW_PROFILE_LEVEL_ID);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(VideoProfileTest, testVideoPayloadCreationForHevcFmtp)
{
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(HEVC_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    VideoProfile::HevcFmtp* pHevcFmtp = new VideoProfile::HevcFmtp();
    pPayload1->SetFmtp(pHevcFmtp);

    VideoProfile::Payload* pPayload3 = new VideoProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<VideoProfile::HevcFmtp*>(pPayload3->GetFmtp())->SetShowLevel(HEVC_FMTP_SHOW_LEVEL);
    EXPECT_EQ(static_cast<VideoProfile::HevcFmtp*>(pPayload3->GetFmtp())->IsLevelVisible(),
            HEVC_FMTP_SHOW_LEVEL);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(VideoProfileTest, testVideoPayloadAssign)
{
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();

    VideoProfile::RtcpFbAttributes objRtcpFb;
    objRtcpFb.SetTrrSupported(RTCP_FB_TRR_SUPPORTED);
    objRtcpFb.SetTrrInt(RTCP_FB_TRR_INT);
    objRtcpFb.SetNackSupported(RTCP_FB_NACK_SUPPORTED);
    objRtcpFb.SetTmmbrSupported(RTCP_FB_TMMBR_SUPPORTED);
    objRtcpFb.SetTmmbrSmaxPr(RTCP_FB_TMMBR_SMAX);
    objRtcpFb.SetPliSupported(RTCP_FB_PLI_SUPPORTED);
    objRtcpFb.SetFirSupported(RTCP_FB_FIR_SUPPORTED);

    pPayload1->SetIncludeImageAttr(VIDEO_PAYLOAD_INCLUDE_IMAGE_ATTR);
    pPayload1->SetIncludeFrameSize(VIDEO_PAYLOAD_INCLUDE_FRAME_SIZE);
    pPayload1->SetImageAttr(VIDEO_PAYLOAD_IMAGE_ATTR);
    pPayload1->SetRtcpFbAttr(objRtcpFb);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload();
    *pPayload2 = *pPayload1;

    EXPECT_EQ(*pPayload1, *pPayload2);

    delete pPayload1;
    delete pPayload2;
}

TEST_F(VideoProfileTest, testVideoPayloadAssignForAvcFmtp)
{
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AVC_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    VideoProfile::AvcFmtp* pAvcFmtp = new VideoProfile::AvcFmtp();
    pPayload1->SetFmtp(pAvcFmtp);

    VideoProfile::Payload* pPayload3 = new VideoProfile::Payload();
    *pPayload3 = *pPayload1;
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<VideoProfile::AvcFmtp*>(pPayload3->GetFmtp())
            ->SetShowProfileLevelId(AVC_FMTP_SHOW_PROFILE_LEVEL_ID);
    EXPECT_EQ(static_cast<VideoProfile::AvcFmtp*>(pPayload3->GetFmtp())->IsProfileLevelIdVisible(),
            AVC_FMTP_SHOW_PROFILE_LEVEL_ID);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(VideoProfileTest, testVideoPayloadAssignForHevcFmtp)
{
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(HEVC_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    VideoProfile::HevcFmtp* pHevcFmtp = new VideoProfile::HevcFmtp();
    pPayload1->SetFmtp(pHevcFmtp);

    VideoProfile::Payload* pPayload3 = new VideoProfile::Payload();
    *pPayload3 = *pPayload1;
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<VideoProfile::HevcFmtp*>(pPayload3->GetFmtp())->SetShowLevel(HEVC_FMTP_SHOW_LEVEL);
    EXPECT_EQ(static_cast<VideoProfile::HevcFmtp*>(pPayload3->GetFmtp())->IsLevelVisible(),
            HEVC_FMTP_SHOW_LEVEL);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(VideoProfileTest, testVideoProfileFrameRate)
{
    VideoProfile* pProfile = new VideoProfile();
    EXPECT_EQ(pProfile->GetFrameRate(), 0);

    pProfile->SetFrameRate(VIDEO_PROFILE_FRAMERATE);
    EXPECT_EQ(pProfile->GetFrameRate(), VIDEO_PROFILE_FRAMERATE);

    delete pProfile;
}

TEST_F(VideoProfileTest, testVideoProfileSupportAvpf)
{
    VideoProfile* pProfile = new VideoProfile();
    EXPECT_EQ(pProfile->IsAvpfSupported(), IMS_FALSE);

    pProfile->SetSupportAvpf(VIDEO_PROFILE_SUPPORT_AVPF);
    EXPECT_EQ(pProfile->IsAvpfSupported(), VIDEO_PROFILE_SUPPORT_AVPF);

    delete pProfile;
}

TEST_F(VideoProfileTest, testVideoProfileCvoId)
{
    VideoProfile* pProfile = new VideoProfile();
    EXPECT_EQ(pProfile->GetCvoId(), -1);

    pProfile->SetCvoId(VIDEO_PROFILE_SUPPORT_AVPF);
    EXPECT_EQ(pProfile->GetCvoId(), VIDEO_PROFILE_SUPPORT_AVPF);

    delete pProfile;
}

TEST_F(VideoProfileTest, testVideoProfileSupportCapaNegoForAvpf)
{
    VideoProfile* pProfile = new VideoProfile();
    EXPECT_EQ(pProfile->IsCapaNegoForAvpfSupported(), IMS_FALSE);

    pProfile->SetSupportCapaNegoForAvpf(VIDEO_PROFILE_SUPPORT_CAPA_NEGO);
    EXPECT_EQ(pProfile->IsCapaNegoForAvpfSupported(), VIDEO_PROFILE_SUPPORT_CAPA_NEGO);

    delete pProfile;
}

TEST_F(VideoProfileTest, testVideoProfileGetPayloadAt)
{
    const IMS_SINT32 nPayload1 = 115;
    const IMS_SINT32 nPayload2 = 116;

    VideoProfile* pProfile = new VideoProfile();
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(HEVC_PAYLOAD_TYPE);
    pPayload1->GetRtpMap().SetPayloadNumber(nPayload1);
    pProfile->GetPayloadList().Append(pPayload1);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload();
    pPayload2->GetRtpMap().SetPayloadType(AVC_PAYLOAD_TYPE);
    pPayload2->GetRtpMap().SetPayloadNumber(nPayload2);
    pProfile->GetPayloadList().Append(pPayload2);

    EXPECT_EQ(pProfile->GetPayloadList().GetSize(), 2);

    EXPECT_EQ(pProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), nPayload1);
    EXPECT_EQ(pProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadNumber(), nPayload2);

    delete pProfile;
}

TEST_F(VideoProfileTest, testVideoProfileCreationDefault)
{
    VideoProfile* pProfile = new VideoProfile();
    EXPECT_EQ(pProfile->GetFrameRate(), 0);
    EXPECT_EQ(pProfile->IsAvpfSupported(), IMS_FALSE);
    EXPECT_EQ(pProfile->GetCvoId(), -1);
    EXPECT_EQ(pProfile->IsCapaNegoForAvpfSupported(), IMS_FALSE);

    delete pProfile;
}

TEST_F(VideoProfileTest, testVideoProfileCopyConstructor)
{
    VideoProfile* pProfile1 = new VideoProfile();

    pProfile1->SetFrameRate(VIDEO_PROFILE_FRAMERATE);
    pProfile1->SetSupportAvpf(VIDEO_PROFILE_SUPPORT_AVPF);
    pProfile1->SetCvoId(VIDEO_PROFILE_SUPPORT_AVPF);
    pProfile1->SetSupportCapaNegoForAvpf(VIDEO_PROFILE_SUPPORT_CAPA_NEGO);

    VideoProfile* pProfile2 = new VideoProfile(*pProfile1);

    EXPECT_EQ(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(VideoProfileTest, testVideoProfileAssign)
{
    VideoProfile* pProfile1 = new VideoProfile();

    pProfile1->SetFrameRate(VIDEO_PROFILE_FRAMERATE);
    pProfile1->SetSupportAvpf(VIDEO_PROFILE_SUPPORT_AVPF);
    pProfile1->SetCvoId(VIDEO_PROFILE_SUPPORT_AVPF);
    pProfile1->SetSupportCapaNegoForAvpf(VIDEO_PROFILE_SUPPORT_CAPA_NEGO);

    VideoProfile* pProfile2 = new VideoProfile();
    *pProfile2 = *pProfile1;

    EXPECT_EQ(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(VideoProfileTest, testVideoProfileEqualNotEqual)
{
    VideoProfile* pProfile1 = new VideoProfile();
    VideoProfile* pProfile2 = new VideoProfile();

    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetFrameRate(VIDEO_PROFILE_FRAMERATE);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetFrameRate(VIDEO_PROFILE_FRAMERATE);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetSupportAvpf(VIDEO_PROFILE_SUPPORT_AVPF);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetSupportAvpf(VIDEO_PROFILE_SUPPORT_AVPF);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetCvoId(VIDEO_PROFILE_CVO_ID);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetCvoId(VIDEO_PROFILE_CVO_ID);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetSupportCapaNegoForAvpf(VIDEO_PROFILE_SUPPORT_CAPA_NEGO);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetSupportCapaNegoForAvpf(VIDEO_PROFILE_SUPPORT_CAPA_NEGO);
    EXPECT_EQ(*pProfile1, *pProfile2);

    // Add Payload to test
    VideoProfile::Payload* pPayload1 = new VideoProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AVC_PAYLOAD_TYPE);
    pProfile1->GetPayloadList().Append(pPayload1);
    EXPECT_NE(*pProfile1, *pProfile2);

    VideoProfile::Payload* pPayload2 = new VideoProfile::Payload();
    pPayload2->GetRtpMap().SetPayloadType(AVC_PAYLOAD_TYPE);
    pProfile2->GetPayloadList().Append(pPayload2);
    EXPECT_EQ(*pProfile1, *pProfile2);

    // Change Payload
    pPayload1->GetRtpMap().SetPayloadType(HEVC_PAYLOAD_TYPE);
    EXPECT_NE(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}
