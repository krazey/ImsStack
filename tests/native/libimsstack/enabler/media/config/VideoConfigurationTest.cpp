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

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_VIDEO_DSCP = VideoConfiguration::DEFAULT_VIDEO_DSCP;
static const IMS_SINT32 DEFAULT_CVO_ID = VideoConfiguration::DEFAULT_CVO_ID;
static const IMS_SINT32 DEFAULT_AVPF_ENABLED = VideoConfiguration::DEFAULT_AVPF_ENABLED;
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

class VideoConfigurationTest : public ::testing::Test {

public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {}

    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
    IMS_BOOL GetBoolean(IN const IMS_CHAR* pszKey) { return m_piCc->GetBoolean(pszKey); }
    AString GetString(IN const IMS_CHAR* pszKey) { return m_piCc->GetString(pszKey); }
    IMSVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetIntArray(pszKey);
    }
    IMSVector<AString> GetStringArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetStringArray(pszKey);
    }
};

TEST_F(VideoConfigurationTest, GetConfigDefault)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);

    EXPECT_EQ(m_pConfig->GetVideoDscp(), DEFAULT_VIDEO_DSCP);
    EXPECT_EQ(m_pConfig->GetVideoSendPeriodicSpsPps(), DEFAULT_SEND_PERIODIC_SPS_PPS);
    EXPECT_EQ(m_pConfig->GetCvoId(), DEFAULT_CVO_ID);
    EXPECT_EQ(m_pConfig->IsVideoAvpfEnabled(), DEFAULT_AVPF_ENABLED);
    EXPECT_EQ(m_pConfig->IsVideoAvpfTrrEnabled(), DEFAULT_AVPF_TRR);
    EXPECT_EQ(m_pConfig->IsVideoAvpfNackEnabled(), DEFAULT_AVPF_NACK);
    EXPECT_EQ(m_pConfig->IsVideoAvpfTmmbrEnabled(), DEFAULT_AVPF_TMMBR);
    EXPECT_EQ(m_pConfig->IsVideoAvpfPliEnabled(), DEFAULT_AVPF_PLI);
    EXPECT_EQ(m_pConfig->IsVideoAvpfFirEnabled(), DEFAULT_AVPF_FIR);
    EXPECT_EQ(m_pConfig->GetSdpOfferCapNegoForAvpf(), DEFAULT_AVPF_CAPA_NEGO);
    EXPECT_EQ(m_pConfig->GetVideoIframeIntervalSec(), DEFAULT_I_FRAME_INTERVAL);
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetVideoSamplingRate(), DEFAULT_VIDEO_SAMPLING_RATE);

    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigTest)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);

    EXPECT_TRUE(m_pConfig->Create(m_piCc));

    EXPECT_EQ(m_pConfig->GetVideoSendPeriodicSpsPps(),
            GetInt(CarrierConfig::Assets::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT));
    EXPECT_EQ(m_pConfig->GetCvoId(), GetInt(CarrierConfig::Assets::KEY_VIDEO_CVO_VALUE_INT));
    EXPECT_EQ(m_pConfig->IsVideoAvpfEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_VIDEO_AVPF_ENABLE_BOOL));
    EXPECT_EQ(m_pConfig->GetSdpOfferCapNegoForAvpf(),
            GetInt(CarrierConfig::Assets::KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT));
    EXPECT_EQ(m_pConfig->GetVideoIframeIntervalSec(),
            GetInt(CarrierConfig::Assets::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT));
    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigVideoDscp)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
    IMS_SINT32 nMockVideoDscp = 40;
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();

    ON_CALL(*pMockICarrierConfig, GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT, -1))
            .WillByDefault(Return(nMockVideoDscp));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));
    EXPECT_EQ(m_pConfig->GetVideoDscp(), nMockVideoDscp << 2);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigVideoAvpfFeature)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
    IMS_SINT32 nMockVideoAvpfFeature = 31;  // 0x00011111
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();

    ON_CALL(*pMockICarrierConfig, GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FEATURE_INT, -1))
            .WillByDefault(Return(nMockVideoAvpfFeature));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsVideoAvpfTrrEnabled(), IMS_TRUE);
    EXPECT_EQ(m_pConfig->IsVideoAvpfNackEnabled(), IMS_TRUE);
    EXPECT_EQ(m_pConfig->IsVideoAvpfTmmbrEnabled(), IMS_TRUE);
    EXPECT_EQ(m_pConfig->IsVideoAvpfPliEnabled(), IMS_TRUE);
    EXPECT_EQ(m_pConfig->IsVideoAvpfFirEnabled(), IMS_TRUE);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigVideoPort)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
    IMSVector<IMS_SINT32> objVideoPortRtp;
    objVideoPortRtp.Push(50100);
    objVideoPortRtp.Push(50700);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY))
            .WillByDefault(Return(objVideoPortRtp));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetPortRtp(), objVideoPortRtp.GetAt(0));
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), objVideoPortRtp.GetAt(1));
    EXPECT_EQ(m_pConfig->GetPortRtcp(), objVideoPortRtp.GetAt(0) + 1);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigVideoRtcpInterval)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
    IMSVector<IMS_SINT32> objVideoRtcpInterval;
    objVideoRtcpInterval.Push(3);
    objVideoRtcpInterval.Push(10);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY))
            .WillByDefault(Return(objVideoRtcpInterval));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtcpLiveInterval(), objVideoRtcpInterval.GetAt(0));
    EXPECT_EQ(m_pConfig->GetRtcpInterval(), objVideoRtcpInterval.GetAt(1));

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigVideoBandwidth)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(m_piCc));

    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT));
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT));
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT));
    delete m_pConfig;
}

TEST_F(VideoConfigurationTest, GetConfigVideoInactivityTimer)
{
    VideoConfiguration* m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(m_piCc));

    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT));
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(),
            GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT));
    delete m_pConfig;
}
