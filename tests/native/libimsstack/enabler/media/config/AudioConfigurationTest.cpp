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
#include <config/AudioConfiguration.h>

using ::testing::Return;

static const IMS_SINT32 DEFAULT_PTIME = AudioConfiguration::DEFAULT_PTIME;
static const IMS_SINT32 DEFAULT_MAX_PTIME = AudioConfiguration::DEFAULT_MAX_PTIME;
static const IMS_BOOL DEFAULT_BW_NEGO_OPERION = AudioConfiguration::DEFAULT_BW_NEGO_OPERION;
static const IMS_SINT32 DEFAULT_AUDIO_DSCP = AudioConfiguration::DEFAULT_AUDIO_DSCP;
static const IMS_SINT32 DEFAULT_JITTER_MIN = AudioConfiguration::DEFAULT_JITTER_MIN;
static const IMS_SINT32 DEFAULT_JITTER_MAX = AudioConfiguration::DEFAULT_JITTER_MAX;
static const IMS_SINT32 DEFAULT_JITTER_ADJUST = AudioConfiguration::DEFAULT_JITTER_ADJUST;
static const IMS_SINT32 DEFAULT_JITTER_STEP = AudioConfiguration::DEFAULT_JITTER_STEP;
static const IMS_BOOL DEFAULT_RTCPXR = AudioConfiguration::DEFAULT_RTCPXR;
static const IMS_BOOL DEFAULT_RTCPXR_STATISTICS = AudioConfiguration::DEFAULT_RTCPXR_STATISTICS;
static const IMS_BOOL DEFAULT_RTCPXR_VOIP_METRICS = AudioConfiguration::DEFAULT_RTCPXR_VOIP_METRICS;
static const IMS_BOOL DEFAULT_RTCPXR_PACKET_LOSS_RLE =
        AudioConfiguration::DEFAULT_RTCPXR_PACKET_LOSS_RLE;
static const IMS_BOOL DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE =
        AudioConfiguration::DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE;
static const IMS_SINT32 DEFAULT_DTMF_DURATION = AudioConfiguration::DEFAULT_DTMF_DURATION;
static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY =
        AudioConfiguration::DEFAULT_MODECHANGE_CAPABILITY;
static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD = AudioConfiguration::DEFAULT_MODECHANGE_PERIOD;
static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR =
        AudioConfiguration::DEFAULT_MODECHANGE_NEIGHBOR;
#define DEFAULT_CANDIDATE_ATTRIBUTE "1, UDP, 1119400811, 10.3.210.77, 7010, typ, host"


class AudioConfigurationTest : public ::testing::Test {

public:
    AudioConfiguration* pConfig;

protected:
    virtual void SetUp() override {
        pConfig = new AudioConfiguration();
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(AudioConfigurationTest, Getter) {
    EXPECT_EQ(pConfig->GetPtime(), DEFAULT_PTIME);
    EXPECT_EQ(pConfig->GetMaxPtime(), DEFAULT_MAX_PTIME);
    EXPECT_EQ(pConfig->GetBandwidthNegoOption(), DEFAULT_BW_NEGO_OPERION);
    EXPECT_EQ(pConfig->GetRtpDscp(), DEFAULT_AUDIO_DSCP);
    EXPECT_EQ(pConfig->GetJitterBufferMinSize(), DEFAULT_JITTER_MIN);
    EXPECT_EQ(pConfig->GetJitterBufferMaxSize(), DEFAULT_JITTER_MAX);
    EXPECT_EQ(pConfig->GetJitterBufferAdjustTime(), DEFAULT_JITTER_ADJUST);
    EXPECT_EQ(pConfig->GetJitterBufferStepSize(), DEFAULT_JITTER_STEP);
    EXPECT_EQ(pConfig->IsRtcpXrEnabled(), DEFAULT_RTCPXR);
    EXPECT_EQ(pConfig->IsRtcpXrStatisticsEnabled(), DEFAULT_RTCPXR_STATISTICS);
    EXPECT_EQ(pConfig->IsRtcpXrVoipEnabled(), DEFAULT_RTCPXR_VOIP_METRICS);
    EXPECT_EQ(pConfig->IsRtcpXrPlrEnabled(), DEFAULT_RTCPXR_PACKET_LOSS_RLE);
    EXPECT_EQ(pConfig->IsRtcpXrPdrEnabled(), DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE);
    EXPECT_EQ(pConfig->GetDTMFDuration(), DEFAULT_DTMF_DURATION);
    EXPECT_EQ(pConfig->GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(pConfig->GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(pConfig->GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);

    IMSVector<AString> objCandidateAttr = pConfig->GetAudioCandidateAttribute();
    EXPECT_EQ(objCandidateAttr[0], DEFAULT_CANDIDATE_ATTRIBUTE);
}
