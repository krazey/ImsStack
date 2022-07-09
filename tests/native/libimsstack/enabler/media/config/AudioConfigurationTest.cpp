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

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_PTIME = AudioConfiguration::DEFAULT_PTIME;
static const IMS_SINT32 DEFAULT_MAX_PTIME = AudioConfiguration::DEFAULT_MAX_PTIME;
static const IMS_SINT32 DEFAULT_MAX_RED = AudioConfiguration::DEFAULT_MAX_RED;
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
static const IMS_CHAR* DEFAULT_CANDIDATE_ATTRIBUTE =
        "1, UDP, 1119400811, 10.3.210.77, 7010, typ, host";

class AudioConfigurationTest : public ::testing::Test {

public:
    AudioConfiguration* m_pConfig;
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {
        m_pConfig = new AudioConfiguration(MEDIA_TYPE_AUDIO);
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {
        if (m_pConfig)
        {
            delete m_pConfig;
        }
    }
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

TEST_F(AudioConfigurationTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetPtime(), DEFAULT_PTIME);
    EXPECT_EQ(m_pConfig->GetMaxPtime(), DEFAULT_MAX_PTIME);
    EXPECT_EQ(m_pConfig->GetMaxRed(), DEFAULT_MAX_RED);
    EXPECT_EQ(m_pConfig->GetBandwidthNegoOption(), DEFAULT_BW_NEGO_OPERION);
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
    EXPECT_EQ(m_pConfig->GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(m_pConfig->GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(m_pConfig->GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);

    IMSVector<AString> objCandidateAttr = m_pConfig->GetAudioCandidateAttribute();
    EXPECT_EQ(objCandidateAttr.GetAt(0), DEFAULT_CANDIDATE_ATTRIBUTE);
}

TEST_F(AudioConfigurationTest, GetConfigTest)
{
    m_pConfig->Create(m_piCc);

    EXPECT_EQ(m_pConfig->GetPtime(), DEFAULT_PTIME);
    EXPECT_EQ(
            m_pConfig->GetMaxPtime(), GetInt(CarrierConfig::Assets::KEY_AUDIO_MAXPTIME_MILLIS_INT));
    EXPECT_EQ(m_pConfig->GetMaxRed(), GetInt(CarrierConfig::Assets::KEY_AUDIO_MAXRED_INT));
    EXPECT_EQ(m_pConfig->GetBandwidthNegoOption(),
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_BW_NEGO_OPTION_BOOL));
    EXPECT_EQ(m_pConfig->GetRtpDscp(), DEFAULT_AUDIO_DSCP);
    EXPECT_EQ(m_pConfig->IsRtcpXrEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_ENABLE_BOOL));
    EXPECT_EQ(m_pConfig->IsRtcpXrStatisticsEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_STATISTICS_BOOL));
    EXPECT_EQ(m_pConfig->IsRtcpXrVoipEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL));
    EXPECT_EQ(m_pConfig->IsRtcpXrPlrEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL));
    EXPECT_EQ(m_pConfig->IsRtcpXrPdrEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL));
    EXPECT_EQ(m_pConfig->GetDTMFDuration(),
            GetInt(CarrierConfig::Assets::KEY_AUDIO_TELEPHONE_EVENT_DURATION_MILLIS_INT));
    EXPECT_EQ(m_pConfig->GetModeChangeCapability(),
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT));
    EXPECT_EQ(m_pConfig->GetModeChangePeriod(),
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT));
    EXPECT_EQ(m_pConfig->GetModeChangeNeighbor(),
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT));
}

TEST_F(AudioConfigurationTest, GetConfigAudioPort)
{
    IMSVector<IMS_SINT32> objAudioPortRtp;
    objAudioPortRtp.Push(50000);
    objAudioPortRtp.Push(50500);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY))
            .WillByDefault(Return(objAudioPortRtp));

    m_pConfig->Create(pMockICarrierConfig);

    EXPECT_EQ(m_pConfig->GetPortRtp(), objAudioPortRtp.GetAt(0));
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), objAudioPortRtp.GetAt(1));
    EXPECT_EQ(m_pConfig->GetPortRtcp(), objAudioPortRtp.GetAt(0) + 1);

    delete pMockICarrierConfig;
}

TEST_F(AudioConfigurationTest, GetConfigAudioRtcpInterval)
{
    IMSVector<IMS_SINT32> objAudioRtcpInterval;
    objAudioRtcpInterval.Push(3);
    objAudioRtcpInterval.Push(10);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY))
            .WillByDefault(Return(objAudioRtcpInterval));

    m_pConfig->Create(pMockICarrierConfig);

    EXPECT_EQ(m_pConfig->GetRtcpLiveInterval(), objAudioRtcpInterval.GetAt(0));
    EXPECT_EQ(m_pConfig->GetRtcpInterval(), objAudioRtcpInterval.GetAt(1));

    delete pMockICarrierConfig;
}

TEST_F(AudioConfigurationTest, GetConfigAudioBandwidth)
{
    m_pConfig->Create(m_piCc);

    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(),
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_AS_BANDWIDTH_KBPS_INT));
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(),
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RS_BANDWIDTH_BPS_INT));
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(),
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RR_BANDWIDTH_BPS_INT));
}

TEST_F(AudioConfigurationTest, GetConfigAudioInactivityTimer)
{
    m_pConfig->Create(m_piCc);

    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(),
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT));
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(),
            GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT));
}

TEST_F(AudioConfigurationTest, GetConfigJitterBuffer)
{
    IMSVector<IMS_SINT32> objJitterArray;
    objJitterArray.Push(5);
    objJitterArray.Push(10);
    objJitterArray.Push(5);
    objJitterArray.Push(5);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY))
            .WillByDefault(Return(objJitterArray));

    m_pConfig->Create(pMockICarrierConfig);

    EXPECT_EQ(m_pConfig->GetJitterBufferMinSize(), objJitterArray.GetAt(0));
    EXPECT_EQ(m_pConfig->GetJitterBufferMaxSize(), objJitterArray.GetAt(1));
    EXPECT_EQ(m_pConfig->GetJitterBufferAdjustTime(), objJitterArray.GetAt(2));
    EXPECT_EQ(m_pConfig->GetJitterBufferStepSize(), objJitterArray.GetAt(3));

    delete pMockICarrierConfig;
}

TEST_F(AudioConfigurationTest, GetConfigCandidateAttr)
{
    IMSVector<AString> objAudioCandidateAttribute;
    objAudioCandidateAttribute.Push("2, UDP, 1119400811, 10.3.210.77, 7010, typ, host");

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetStringArray(CarrierConfig::Assets::KEY_AUDIO_CANDIDATE_ATTRIBUTE_STRING_ARRAY))
            .WillByDefault(Return(objAudioCandidateAttribute));

    m_pConfig->Create(pMockICarrierConfig);

    IMSVector<AString> objCandidateAttr = m_pConfig->GetAudioCandidateAttribute();
    EXPECT_EQ(objCandidateAttr.GetAt(0), "2, UDP, 1119400811, 10.3.210.77, 7010, typ, host");

    delete pMockICarrierConfig;
}