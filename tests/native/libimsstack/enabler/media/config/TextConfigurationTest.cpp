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
#include "config/TextConfiguration.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_PAYLOAD_T140 = TextConfiguration::DEFAULT_PAYLOAD_T140;
static const IMS_SINT32 DEFAULT_PAYLOAD_RED = TextConfiguration::DEFAULT_PAYLOAD_RED;
static const IMS_SINT32 DEFAULT_TEXT_DSCP = TextConfiguration::DEFAULT_TEXT_DSCP;
static const IMS_BOOL DEFAULT_EMPTY_REDUNDANT = TextConfiguration::DEFAULT_EMPTY_REDUNDANT;
static const IMS_SINT32 DEFAULT_AS = MediaConfiguration::DEFAULT_AS;
static const IMS_SINT32 DEFAULT_RS = MediaConfiguration::DEFAULT_RS;
static const IMS_SINT32 DEFAULT_RR = MediaConfiguration::DEFAULT_RR;
static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = MediaConfiguration::DEFAULT_RTP_INACTIVITY;
static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = MediaConfiguration::DEFAULT_RTCP_INACTIVITY;

class TextConfigurationTest : public ::testing::Test
{
public:
    TextConfiguration* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pTextBundle;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pTextBundle = new MockICarrierConfig();
    }

    virtual void TearDown() override
    {
        delete m_pConfig;
        delete m_pMockICarrierConfig;
        delete m_pTextBundle;

        m_pConfig = IMS_NULL;
        m_pMockICarrierConfig = IMS_NULL;
        m_pTextBundle = IMS_NULL;
    }

    inline void GetReadyToCreate()
    {
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pTextBundle));
    }
};

TEST_F(TextConfigurationTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetT140PayloadType(), DEFAULT_PAYLOAD_T140);
    EXPECT_EQ(m_pConfig->GetRedPayloadType(), DEFAULT_PAYLOAD_RED);
    EXPECT_EQ(m_pConfig->GetTextDscp(), DEFAULT_TEXT_DSCP);
    EXPECT_EQ(m_pConfig->IsTextCodecEmptyRedundantEnabled(), DEFAULT_EMPTY_REDUNDANT);
}

TEST_F(TextConfigurationTest, GetConfigTextPort)
{
    IMS_SINT32 nTextRtpStart = 50300;
    IMS_SINT32 nTextRtpEnd = 50900;

    ImsVector<IMS_SINT32> objTextPortRtp;
    objTextPortRtp.Push(nTextRtpStart);
    objTextPortRtp.Push(nTextRtpEnd);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsRtt::KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY))
            .WillByDefault(Return(objTextPortRtp));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetPortRtp(), nTextRtpStart);
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), nTextRtpEnd);
    EXPECT_EQ(m_pConfig->GetPortRtcp(), nTextRtpStart + 1);
}

TEST_F(TextConfigurationTest, GetConfigTextRtcpInterval)
{
    IMS_SINT32 nTextRtcpIntervalOnActive = 0;
    IMS_SINT32 nTextRtcpIntervalOnHold = 3;

    ImsVector<IMS_SINT32> objTextRtcpInterval;
    objTextRtcpInterval.Push(nTextRtcpIntervalOnActive);
    objTextRtcpInterval.Push(nTextRtcpIntervalOnHold);

    ON_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INTERVAL_INT_ARRAY))
            .WillByDefault(Return(objTextRtcpInterval));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnActive(), nTextRtcpIntervalOnActive);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnHold(), nTextRtcpIntervalOnHold);
}

TEST_F(TextConfigurationTest, GetConfigTextBandwidth)
{
    IMS_SINT32 nAsBandwidthKbps = 40;
    IMS_SINT32 nRsBandwidthBps = 100;
    IMS_SINT32 nRrBandwidthBps = 200;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_AS_BANDWIDTH_KBPS_INT, DEFAULT_AS))
            .WillByDefault(Return(nAsBandwidthKbps));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RS_BANDWIDTH_BPS_INT, DEFAULT_RS))
            .WillByDefault(Return(nRsBandwidthBps));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RR_BANDWIDTH_BPS_INT, DEFAULT_RR))
            .WillByDefault(Return(nRrBandwidthBps));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(), nAsBandwidthKbps);
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(), nRsBandwidthBps);
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(), nRrBandwidthBps);
}

TEST_F(TextConfigurationTest, GetConfigTextInactivityTimer)
{
    IMS_UINT32 nTextRtpInactivity = 5000;
    IMS_UINT32 nTextRtcpInactivity = 10000;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RTP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTP_INACTIVITY))
            .WillByDefault(Return(nTextRtpInactivity));
    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTCP_INACTIVITY))
            .WillByDefault(Return(nTextRtcpInactivity));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), nTextRtpInactivity);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), nTextRtcpInactivity);
}

TEST_F(TextConfigurationTest, GetConfigTextDscp)
{
    IMS_UINT32 nTextDscp = 40;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RTP_DSCP_INT, DEFAULT_TEXT_DSCP))
            .WillByDefault(Return(nTextDscp));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetTextDscp(), nTextDscp);
}

TEST_F(TextConfigurationTest, GetConfigTextCodecEmptyRedundantEnabled)
{
    IMS_BOOL bTextEmptyRedundantEnabled = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL,
                    DEFAULT_EMPTY_REDUNDANT))
            .WillByDefault(Return(bTextEmptyRedundantEnabled));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsTextCodecEmptyRedundantEnabled(), bTextEmptyRedundantEnabled);
}
