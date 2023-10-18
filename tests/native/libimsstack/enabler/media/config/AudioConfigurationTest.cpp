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
#include "config/AudioConfiguration.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_SUPPORT_EVS = AudioConfiguration::DEFAULT_SUPPORT_EVS;
static const IMS_SINT32 DEFAULT_PTIME = AudioConfiguration::DEFAULT_PTIME;
static const IMS_SINT32 DEFAULT_MAX_PTIME = AudioConfiguration::DEFAULT_MAX_PTIME;
static const IMS_SINT32 DEFAULT_MAX_RED = AudioConfiguration::DEFAULT_MAX_RED;
static const IMS_BOOL DEFAULT_BW_NEGO_OPTION = AudioConfiguration::DEFAULT_BW_NEGO_OPTION;
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
static const IMS_SINT32 DEFAULT_AS = MediaConfiguration::DEFAULT_AS;
static const IMS_SINT32 DEFAULT_RS = MediaConfiguration::DEFAULT_RS;
static const IMS_SINT32 DEFAULT_RR = MediaConfiguration::DEFAULT_RR;
static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = MediaConfiguration::DEFAULT_RTP_INACTIVITY;
static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = MediaConfiguration::DEFAULT_RTCP_INACTIVITY;

class AudioConfigurationTest : public ::testing::Test
{
public:
    AudioConfiguration* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pAudioBundle;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new AudioConfiguration(MEDIA_TYPE_AUDIO);
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pAudioBundle = new MockICarrierConfig();
    }
    virtual void TearDown() override
    {
        delete m_pConfig;
        delete m_pMockICarrierConfig;
        delete m_pAudioBundle;

        m_pConfig = IMS_NULL;
        m_pMockICarrierConfig = IMS_NULL;
        m_pAudioBundle = IMS_NULL;
    }

    inline void GetReadyToCreate()
    {
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pAudioBundle));
    }
};

TEST_F(AudioConfigurationTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->IsEvsSupported(), IMS_FALSE);
    EXPECT_EQ(m_pConfig->GetPtime(), DEFAULT_PTIME);
    EXPECT_EQ(m_pConfig->GetMaxPtime(), DEFAULT_MAX_PTIME);
    EXPECT_EQ(m_pConfig->GetMaxRed(), DEFAULT_MAX_RED);
    EXPECT_EQ(m_pConfig->GetBandwidthNegoOption(), DEFAULT_BW_NEGO_OPTION);
    EXPECT_EQ(m_pConfig->GetRtpDscp(), DEFAULT_AUDIO_DSCP);
    EXPECT_EQ(m_pConfig->GetJitterBufferMinSize(), DEFAULT_JITTER_MIN);
    EXPECT_EQ(m_pConfig->GetJitterBufferMaxSize(), DEFAULT_JITTER_MAX);
    EXPECT_EQ(m_pConfig->GetJitterBufferAdjustTime(), DEFAULT_JITTER_ADJUST);
    EXPECT_EQ(m_pConfig->GetJitterBufferStepSize(), DEFAULT_JITTER_STEP);
    EXPECT_EQ(m_pConfig->IsRtcpXrEnabled(), DEFAULT_RTCPXR);
    EXPECT_EQ(m_pConfig->IsRtcpXrStatisticsEnabled(), DEFAULT_RTCPXR_STATISTICS);
    EXPECT_EQ(m_pConfig->IsRtcpXrVoipEnabled(), DEFAULT_RTCPXR_VOIP_METRICS);
    EXPECT_EQ(m_pConfig->IsRtcpXrPlrEnabled(), DEFAULT_RTCPXR_PACKET_LOSS_RLE);
    EXPECT_EQ(m_pConfig->IsRtcpXrPdrEnabled(), DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE);
    EXPECT_EQ(m_pConfig->GetDTMFDuration(), DEFAULT_DTMF_DURATION);
}

TEST_F(AudioConfigurationTest, CreateNullConfig)
{
    ICarrierConfig* m_nullPicc = IMS_NULL;

    EXPECT_FALSE(m_pConfig->Create(m_nullPicc));
}

TEST_F(AudioConfigurationTest, IsEvsSupported)
{
    IMS_BOOL bEvsSupport = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_EVS_SUPPORT_BOOL, DEFAULT_SUPPORT_EVS))
            .WillByDefault(Return(bEvsSupport));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsEvsSupported(), bEvsSupport);
}

TEST_F(AudioConfigurationTest, GetPtime)
{
    IMS_SINT32 nAudioPtime = 20;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_AUDIO_PTIME_MILLIS_INT, DEFAULT_PTIME))
            .WillByDefault(Return(nAudioPtime));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetPtime(), nAudioPtime);
}

TEST_F(AudioConfigurationTest, GetMaxPtime)
{
    IMS_SINT32 nAudioMaxPtime = 20;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_AUDIO_MAXPTIME_MILLIS_INT, DEFAULT_MAX_PTIME))
            .WillByDefault(Return(nAudioMaxPtime));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetMaxPtime(), nAudioMaxPtime);
}

TEST_F(AudioConfigurationTest, GetMaxRed)
{
    IMS_SINT32 nAudioMaxRed = 220;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_AUDIO_MAXRED_INT, DEFAULT_MAX_RED))
            .WillByDefault(Return(nAudioMaxRed));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetMaxRed(), nAudioMaxRed);
}

TEST_F(AudioConfigurationTest, GetBandwidthNegoOption)
{
    IMS_BOOL bAudioBwNegoOptionEnabled = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_AUDIO_BW_NEGO_OPTION_BOOL, DEFAULT_BW_NEGO_OPTION))
            .WillByDefault(Return(bAudioBwNegoOptionEnabled));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetBandwidthNegoOption(), bAudioBwNegoOptionEnabled);
}

TEST_F(AudioConfigurationTest, GetRtpDscp)
{
    IMS_SINT32 nAudioRtpDscp = 46;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_AUDIO_RTP_DSCP_INT, DEFAULT_AUDIO_DSCP))
            .WillByDefault(Return(nAudioRtpDscp));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtpDscp(), nAudioRtpDscp);
}

TEST_F(AudioConfigurationTest, GetJitterBufferSize)
{
    IMS_SINT32 nJitterBufferMinSize = 4;
    IMS_SINT32 nJitterBufferMaxSize = 10;
    IMS_SINT32 nJitterBufferAdjustTime = 5;
    IMS_SINT32 nJitterBufferStepSize = 6;

    ImsVector<IMS_SINT32> objJitterArray;
    objJitterArray.Push(nJitterBufferMinSize);
    objJitterArray.Push(nJitterBufferMaxSize);
    objJitterArray.Push(nJitterBufferAdjustTime);
    objJitterArray.Push(nJitterBufferStepSize);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY))
            .WillByDefault(Return(objJitterArray));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetJitterBufferMinSize(), nJitterBufferMinSize);
    EXPECT_EQ(m_pConfig->GetJitterBufferMaxSize(), nJitterBufferMaxSize);
    EXPECT_EQ(m_pConfig->GetJitterBufferAdjustTime(), nJitterBufferAdjustTime);
    EXPECT_EQ(m_pConfig->GetJitterBufferStepSize(), nJitterBufferStepSize);
}

TEST_F(AudioConfigurationTest, IsRtcpXrEnabled)
{
    IMS_BOOL m_bAudioRtcpxrEnabled = IMS_TRUE;
    IMS_BOOL m_bAudioRtcpxrStatisticsEnabled = IMS_FALSE;
    IMS_BOOL m_bAudioRtcpxrVoipMetricsEnabled = IMS_TRUE;
    IMS_BOOL m_bAudioRtcpxrPacketLossRleEnabled = IMS_FALSE;
    IMS_BOOL m_bAudioRtcpxrPacketDuplicateRleEnabled = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_ENABLE_BOOL, DEFAULT_RTCPXR))
            .WillByDefault(Return(m_bAudioRtcpxrEnabled));
    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_STATISTICS_BOOL,
                    DEFAULT_RTCPXR_STATISTICS))
            .WillByDefault(Return(m_bAudioRtcpxrStatisticsEnabled));
    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL,
                    DEFAULT_RTCPXR_VOIP_METRICS))
            .WillByDefault(Return(m_bAudioRtcpxrVoipMetricsEnabled));
    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL,
                    DEFAULT_RTCPXR_PACKET_LOSS_RLE))
            .WillByDefault(Return(m_bAudioRtcpxrPacketLossRleEnabled));
    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL,
                    DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE))
            .WillByDefault(Return(m_bAudioRtcpxrPacketDuplicateRleEnabled));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsRtcpXrEnabled(), m_bAudioRtcpxrEnabled);
    EXPECT_EQ(m_pConfig->IsRtcpXrStatisticsEnabled(), m_bAudioRtcpxrStatisticsEnabled);
    EXPECT_EQ(m_pConfig->IsRtcpXrVoipEnabled(), m_bAudioRtcpxrVoipMetricsEnabled);
    EXPECT_EQ(m_pConfig->IsRtcpXrPlrEnabled(), m_bAudioRtcpxrPacketLossRleEnabled);
    EXPECT_EQ(m_pConfig->IsRtcpXrPdrEnabled(), m_bAudioRtcpxrPacketDuplicateRleEnabled);
}

TEST_F(AudioConfigurationTest, GetDTMFDuration)
{
    IMS_SINT32 m_nDtmfDuration = 100;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_AUDIO_TELEPHONE_EVENT_DURATION_MILLIS_INT,
                    DEFAULT_DTMF_DURATION))
            .WillByDefault(Return(m_nDtmfDuration));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetDTMFDuration(), m_nDtmfDuration);
}

TEST_F(AudioConfigurationTest, GetConfigCandidateAttr)
{
    ImsVector<AString> objAudioCandidateAttribute;
    objAudioCandidateAttribute.Push("2, UDP, 1119400811, 10.3.210.77, 7010, typ, host");

    ON_CALL(*m_pMockICarrierConfig,
            GetStringArray(CarrierConfig::Assets::KEY_AUDIO_CANDIDATE_ATTRIBUTE_STRING_ARRAY))
            .WillByDefault(Return(objAudioCandidateAttribute));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    ImsVector<AString> objCandidateAttr = m_pConfig->GetAudioCandidateAttribute();
    EXPECT_EQ(objCandidateAttr.GetAt(0), objAudioCandidateAttribute.GetAt(0).GetStr());
}

TEST_F(AudioConfigurationTest, GetAudioPort)
{
    IMS_SINT32 nAudioRtpStart = 50000;
    IMS_SINT32 nAudioRtpEnd = 50500;
    ImsVector<IMS_SINT32> objAudioPortRtp;
    objAudioPortRtp.Push(nAudioRtpStart);
    objAudioPortRtp.Push(nAudioRtpEnd);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY))
            .WillByDefault(Return(objAudioPortRtp));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetPortRtp(), nAudioRtpStart);
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), nAudioRtpEnd);
    EXPECT_EQ(m_pConfig->GetPortRtcp(), nAudioRtpStart + 1);
}

TEST_F(AudioConfigurationTest, GetAudioRtcpInterval)
{
    IMS_SINT32 nAudioRtcpLiveInterval = 7;
    IMS_SINT32 nAudioRtcpInterval = 8;

    ImsVector<IMS_SINT32> objAudioRtcpInterval;
    objAudioRtcpInterval.Push(nAudioRtcpLiveInterval);
    objAudioRtcpInterval.Push(nAudioRtcpInterval);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY))
            .WillByDefault(Return(objAudioRtcpInterval));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtcpLiveInterval(), nAudioRtcpLiveInterval);
    EXPECT_EQ(m_pConfig->GetRtcpInterval(), nAudioRtcpInterval);
}

TEST_F(AudioConfigurationTest, GetAudioBandwidth)
{
    IMS_SINT32 nAsBandwidthKbps = 40;
    IMS_SINT32 nRsBandwidthBps = 100;
    IMS_SINT32 nRrBandwidthBps = 200;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_AS_BANDWIDTH_KBPS_INT, DEFAULT_AS))
            .WillByDefault(Return(nAsBandwidthKbps));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RS_BANDWIDTH_BPS_INT, DEFAULT_RS))
            .WillByDefault(Return(nRsBandwidthBps));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RR_BANDWIDTH_BPS_INT, DEFAULT_RR))
            .WillByDefault(Return(nRrBandwidthBps));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(), nAsBandwidthKbps);
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(), nRsBandwidthBps);
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(), nRrBandwidthBps);
}

TEST_F(AudioConfigurationTest, GetAudioInactivityTimer)
{
    IMS_SINT32 nRtpInactivityTimerMillis = 25;
    IMS_SINT32 nRtcpInactivityTimerMillis = 15;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTP_INACTIVITY))
            .WillByDefault(Return(nRtpInactivityTimerMillis));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTCP_INACTIVITY))
            .WillByDefault(Return(nRtcpInactivityTimerMillis));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), nRtpInactivityTimerMillis);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), nRtcpInactivityTimerMillis);
}
