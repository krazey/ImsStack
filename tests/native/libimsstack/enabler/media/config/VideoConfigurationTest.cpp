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

#include "ImsStrLib.h"
#include "ICarrierConfig.h"
#include "CarrierConfig.h"
#include "ServiceConfig.h"
#include "MockICarrierConfig.h"
#include "config/VideoConfiguration.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_VIDEO_DSCP = VideoConfiguration::DEFAULT_VIDEO_DSCP;
static const IMS_SINT32 DEFAULT_CVO_ID = VideoConfiguration::DEFAULT_CVO_ID;
static const IMS_SINT32 DEFAULT_AVPF_ENABLED = VideoConfiguration::DEFAULT_AVPF_ENABLED;
static const IMS_SINT32 DEFAULT_AVPF_FEATURE = VideoConfiguration::DEFAULT_AVPF_FEATURE;
static const IMS_SINT32 DEFAULT_AVPF_CAPA_NEGO = VideoConfiguration::DEFAULT_AVPF_CAPA_NEGO;
static const IMS_SINT32 DEFAULT_CHANNEL = VideoConfiguration::DEFAULT_CHANNEL;
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
static const IMS_BOOL DEFAULT_BW_NEGO_OPTION = VideoConfiguration::DEFAULT_BW_NEGO_OPTION;
static const IMS_BOOL DEFAULT_VIDEO_LOWEST_BITRATE =
        VideoConfiguration::DEFAULT_VIDEO_LOWEST_BITRATE;
static const IMS_SINT32 DEFAULT_AS_VIDEO = VideoConfiguration::DEFAULT_AS_VIDEO;
static const IMS_SINT32 DEFAULT_RS_VIDEO = VideoConfiguration::DEFAULT_RS_VIDEO;
static const IMS_SINT32 DEFAULT_RR_VIDEO = VideoConfiguration::DEFAULT_RR_VIDEO;
static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = MediaConfiguration::DEFAULT_RTP_INACTIVITY;
static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = MediaConfiguration::DEFAULT_RTCP_INACTIVITY;

class VideoConfigurationTest : public ::testing::Test {

public:
    VideoConfiguration* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pVideoBundle;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pVideoBundle = new MockICarrierConfig();
    }

    virtual void TearDown() override
    {
        delete m_pConfig;
        delete m_pMockICarrierConfig;
        delete m_pVideoBundle;

        m_pConfig = IMS_NULL;
        m_pMockICarrierConfig = IMS_NULL;
        m_pVideoBundle = IMS_NULL;
    }

    inline void GetReadyToCreate()
    {
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pVideoBundle));
    }
};

TEST_F(VideoConfigurationTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetVideoDscp(), DEFAULT_VIDEO_DSCP);
    EXPECT_EQ(m_pConfig->GetVideoSendPeriodicSpsPps(), DEFAULT_SEND_PERIODIC_SPS_PPS);
    EXPECT_EQ(m_pConfig->GetCvoId(), DEFAULT_CVO_ID);
    EXPECT_EQ(m_pConfig->IsVideoAvpfEnabled(), DEFAULT_AVPF_ENABLED);
    EXPECT_EQ(m_pConfig->IsVideoAvpfTrrEnabled(), DEFAULT_AVPF_TRR);
    EXPECT_EQ(m_pConfig->IsVideoAvpfNackEnabled(), DEFAULT_AVPF_NACK);
    EXPECT_EQ(m_pConfig->IsVideoAvpfTmmbrEnabled(), DEFAULT_AVPF_TMMBR);
    EXPECT_EQ(m_pConfig->IsVideoAvpfPliEnabled(), DEFAULT_AVPF_PLI);
    EXPECT_EQ(m_pConfig->IsVideoAvpfFirEnabled(), DEFAULT_AVPF_FIR);
    EXPECT_EQ(m_pConfig->IsAvpfCapabilityNegotiationEnabled(), (DEFAULT_AVPF_CAPA_NEGO > 0));
    EXPECT_EQ(m_pConfig->GetSdpOfferCapNegoForAvpf(), DEFAULT_AVPF_CAPA_NEGO);
    EXPECT_EQ(m_pConfig->GetVideoIframeIntervalSec(), DEFAULT_I_FRAME_INTERVAL);
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetVideoSamplingRate(), DEFAULT_VIDEO_SAMPLING_RATE);
    EXPECT_EQ(m_pConfig->GetBandwidthNegoOption(), DEFAULT_BW_NEGO_OPTION);
    EXPECT_EQ(m_pConfig->GetVideoLowestBitrateBps(), DEFAULT_VIDEO_LOWEST_BITRATE);
}

TEST_F(VideoConfigurationTest, GetVideoDscp)
{
    IMS_SINT32 nVideoDscp = 40;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT, DEFAULT_VIDEO_DSCP))
            .WillByDefault(Return(nVideoDscp));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetVideoDscp(), nVideoDscp);
}

TEST_F(VideoConfigurationTest, GetVideoSendPeriodicSpsPps)
{
    IMS_SINT32 nVideoSendPeriodicSpsPps = 2;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT,
                    DEFAULT_SEND_PERIODIC_SPS_PPS))
            .WillByDefault(Return(nVideoSendPeriodicSpsPps));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetVideoSendPeriodicSpsPps(), nVideoSendPeriodicSpsPps);
}

TEST_F(VideoConfigurationTest, GetCvoId)
{
    IMS_SINT32 nCvoId = 1;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_VIDEO_CVO_VALUE_INT, DEFAULT_CVO_ID))
            .WillByDefault(Return(nCvoId));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetCvoId(), nCvoId);
}

TEST_F(VideoConfigurationTest, GetConfigVideoAvpfFeature)
{
    IMS_BOOL bVideoAvpfEnabled = IMS_TRUE;
    IMS_BOOL bVideoAvpfTrrEnabled = IMS_FALSE;
    IMS_BOOL bVideoAvpfNackEnabled = IMS_TRUE;
    IMS_BOOL bVideoAvpfTmmbrEnabled = IMS_FALSE;
    IMS_BOOL bVideoAvpfPliEnabled = IMS_FALSE;
    IMS_BOOL bVideoAvpfFirEnabled = IMS_TRUE;

    IMS_SINT32 nVideoAvpfFeature = 0;
    nVideoAvpfFeature |= bVideoAvpfTrrEnabled;
    nVideoAvpfFeature |= bVideoAvpfNackEnabled << 1;
    nVideoAvpfFeature |= bVideoAvpfTmmbrEnabled << 2;
    nVideoAvpfFeature |= bVideoAvpfPliEnabled << 3;
    nVideoAvpfFeature |= bVideoAvpfFirEnabled << 4;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_VIDEO_AVPF_ENABLE_BOOL, DEFAULT_AVPF_ENABLED))
            .WillByDefault(Return(bVideoAvpfEnabled));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FEATURE_INT, DEFAULT_AVPF_FEATURE))
            .WillByDefault(Return(nVideoAvpfFeature));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsVideoAvpfEnabled(), bVideoAvpfEnabled);

    EXPECT_EQ(m_pConfig->IsVideoAvpfTrrEnabled(), bVideoAvpfTrrEnabled);
    EXPECT_EQ(m_pConfig->IsVideoAvpfNackEnabled(), bVideoAvpfNackEnabled);
    EXPECT_EQ(m_pConfig->IsVideoAvpfTmmbrEnabled(), bVideoAvpfTmmbrEnabled);
    EXPECT_EQ(m_pConfig->IsVideoAvpfPliEnabled(), bVideoAvpfPliEnabled);
    EXPECT_EQ(m_pConfig->IsVideoAvpfFirEnabled(), bVideoAvpfFirEnabled);
}

TEST_F(VideoConfigurationTest, GetSdpOfferCapNegoForAvpf)
{
    IMS_SINT32 nSdpOfferCapNegoForAvpf = 1;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT,
                    DEFAULT_AVPF_CAPA_NEGO))
            .WillByDefault(Return(nSdpOfferCapNegoForAvpf));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsAvpfCapabilityNegotiationEnabled(), (nSdpOfferCapNegoForAvpf > 0));
    EXPECT_EQ(m_pConfig->GetSdpOfferCapNegoForAvpf(), nSdpOfferCapNegoForAvpf);
}

TEST_F(VideoConfigurationTest, GetVideoIframeIntervalSec)
{
    IMS_SINT32 nVideoIframeIntervalSec = 2;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT,
                    DEFAULT_I_FRAME_INTERVAL))
            .WillByDefault(Return(nVideoIframeIntervalSec));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetVideoIframeIntervalSec(), nVideoIframeIntervalSec);
}

TEST_F(VideoConfigurationTest, GetBandwidthNegoOption)
{
    IMS_BOOL bVideoBwNegoOptionEnabled = 2;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_VIDEO_BW_NEGO_OPTION_BOOL, DEFAULT_BW_NEGO_OPTION))
            .WillByDefault(Return(bVideoBwNegoOptionEnabled));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetBandwidthNegoOption(), bVideoBwNegoOptionEnabled);
}

TEST_F(VideoConfigurationTest, GetChannel)
{
    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
}

TEST_F(VideoConfigurationTest, GetVideoSamplingRate)
{
    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetVideoSamplingRate(), DEFAULT_VIDEO_SAMPLING_RATE);
}

TEST_F(VideoConfigurationTest, GetVideoPort)
{
    IMS_SINT32 nVideoRtpStart = 50100;
    IMS_SINT32 nVideoRtpEnd = 50700;
    ImsVector<IMS_SINT32> objVideoPortRtp;
    objVideoPortRtp.Push(nVideoRtpStart);
    objVideoPortRtp.Push(nVideoRtpEnd);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY))
            .WillByDefault(Return(objVideoPortRtp));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetPortRtp(), nVideoRtpStart);
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), nVideoRtpEnd);
    EXPECT_EQ(m_pConfig->GetPortRtcp(), nVideoRtpStart + 1);
}

TEST_F(VideoConfigurationTest, GetVideoRtcpInterval)
{
    IMS_SINT32 nVideoRtcpLiveInterval = 30;
    IMS_SINT32 nVideoRtcpInterval = 15;

    ImsVector<IMS_SINT32> objVideoRtcpInterval;
    objVideoRtcpInterval.Push(nVideoRtcpLiveInterval);
    objVideoRtcpInterval.Push(nVideoRtcpInterval);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY))
            .WillByDefault(Return(objVideoRtcpInterval));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtcpLiveInterval(), nVideoRtcpLiveInterval);
    EXPECT_EQ(m_pConfig->GetRtcpInterval(), nVideoRtcpInterval);
}

TEST_F(VideoConfigurationTest, GetConfigVideoBandwidth)
{
    IMS_SINT32 nAsBandwidthKbps = 30;
    IMS_SINT32 nRsBandwidthBps = 200;
    IMS_SINT32 nRrBandwidthBps = 300;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT, DEFAULT_AS_VIDEO))
            .WillByDefault(Return(nAsBandwidthKbps));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT, DEFAULT_RS_VIDEO))
            .WillByDefault(Return(nRsBandwidthBps));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT, DEFAULT_RR_VIDEO))
            .WillByDefault(Return(nRrBandwidthBps));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(), nAsBandwidthKbps);
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(), nRsBandwidthBps);
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(), nRrBandwidthBps);
}

TEST_F(VideoConfigurationTest, GetVideoInactivityTimer)
{
    IMS_SINT32 nRtpInactivityTimerMillis = 30;
    IMS_SINT32 nRtcpInactivityTimerMillis = 30;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTP_INACTIVITY))
            .WillByDefault(Return(nRtpInactivityTimerMillis));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTCP_INACTIVITY))
            .WillByDefault(Return(nRtcpInactivityTimerMillis));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), nRtpInactivityTimerMillis);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), nRtcpInactivityTimerMillis);
}

TEST_F(VideoConfigurationTest, GetVideoLowestBitrateBps)
{
    IMS_SINT32 nVideoLowestBitrateBps = 90000;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_VIDEO_LOWEST_BITRATE_BPS_INT,
                    DEFAULT_VIDEO_LOWEST_BITRATE))
            .WillByDefault(Return(nVideoLowestBitrateBps));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetVideoLowestBitrateBps(), nVideoLowestBitrateBps);
}
