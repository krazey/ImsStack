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
#include <config/VideoConfiguration.h>

using ::testing::Return;

static const IMS_SINT32 DEFAULT_VIDEO_DSCP = VideoConfiguration::DEFAULT_VIDEO_DSCP;
static const IMS_SINT32 DEFAULT_SEND_PERIODIC_SPS_PPS =
        VideoConfiguration::DEFAULT_SEND_PERIODIC_SPS_PPS;
static const IMS_BOOL DEFAULT_AVPF_TRR = VideoConfiguration::DEFAULT_AVPF_TRR;
static const IMS_BOOL DEFAULT_AVPF_NACK = VideoConfiguration::DEFAULT_AVPF_NACK;
static const IMS_BOOL DEFAULT_AVPF_TMMBR = VideoConfiguration::DEFAULT_AVPF_TMMBR;
static const IMS_BOOL DEFAULT_AVPF_PLI = VideoConfiguration::DEFAULT_AVPF_PLI;
static const IMS_BOOL DEFAULT_AVPF_FIR = VideoConfiguration::DEFAULT_AVPF_FIR;
static const IMS_SINT32 DEFAULT_I_FRAME_INTERVAL = VideoConfiguration::DEFAULT_I_FRAME_INTERVAL;
static const IMS_SINT32 DEFAULT_VIDEO_SAMPLING_RATE =
        VideoConfiguration::DEFAULT_VIDEO_SAMPLING_RATE;

class VideoConfigurationTest : public ::testing::Test {

public:
    VideoConfiguration* pConfig;
protected:
    virtual void SetUp() override {
        pConfig = new VideoConfiguration();
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(VideoConfigurationTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetVideoDscp(), DEFAULT_VIDEO_DSCP);
    EXPECT_EQ(pConfig->GetVideoSendPeriodicSpsPps(), DEFAULT_SEND_PERIODIC_SPS_PPS);
    EXPECT_EQ(pConfig->IsVideoAvpfTrrEnabled(), DEFAULT_AVPF_TRR);
    EXPECT_EQ(pConfig->IsbVideoAvpfNackEnabled(), DEFAULT_AVPF_NACK);
    EXPECT_EQ(pConfig->IsVideoAvpfTmmbrEnabled(), DEFAULT_AVPF_TMMBR);
    EXPECT_EQ(pConfig->IsVideoAvpfPliEnabled(), DEFAULT_AVPF_PLI);
    EXPECT_EQ(pConfig->IsVideoAvpfFirEnabled(), DEFAULT_AVPF_FIR);
    EXPECT_EQ(pConfig->GetVideoIframeIntervalSec(), DEFAULT_I_FRAME_INTERVAL);
    EXPECT_EQ(pConfig->GetVideoSamplingRate(), DEFAULT_VIDEO_SAMPLING_RATE);
}
