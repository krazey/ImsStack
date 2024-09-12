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

#include "config/MediaConfiguration.h"

static const IMS_SINT32 DEFAULT_RTP_PORT = MediaConfiguration::DEFAULT_RTP_PORT;
static const IMS_SINT32 DEFAULT_RTP_PORT_END = MediaConfiguration::DEFAULT_RTP_PORT_END;
static const IMS_SINT32 DEFAULT_RTCP_PORT = MediaConfiguration::DEFAULT_RTCP_PORT;
static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_ACTIVE =
        MediaConfiguration::DEFAULT_RTCP_INVERVAL_ACTIVE;
static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_HOLD = MediaConfiguration::DEFAULT_RTCP_INVERVAL_HOLD;
static const IMS_SINT32 DEFAULT_AS = MediaConfiguration::DEFAULT_AS;
static const IMS_SINT32 DEFAULT_RS = MediaConfiguration::DEFAULT_RS;
static const IMS_SINT32 DEFAULT_RR = MediaConfiguration::DEFAULT_RR;
static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = MediaConfiguration::DEFAULT_RTP_INACTIVITY;
static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = MediaConfiguration::DEFAULT_RTCP_INACTIVITY;

class MediaConfigurationTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(MediaConfigurationTest, GetConfigAudio)
{
    MediaConfiguration* m_pConfig = new MediaConfiguration(MEDIA_TYPE_AUDIO);

    EXPECT_EQ(m_pConfig->GetSessionType(), MEDIA_TYPE_AUDIO);
    EXPECT_EQ(m_pConfig->GetPortRtp(), DEFAULT_RTP_PORT);
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), DEFAULT_RTP_PORT_END);
    EXPECT_EQ(m_pConfig->GetPortRtcp(), DEFAULT_RTCP_PORT);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnActive(), DEFAULT_RTCP_INVERVAL_ACTIVE);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnHold(), DEFAULT_RTCP_INVERVAL_HOLD);
    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(), DEFAULT_AS);
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(), DEFAULT_RS);
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(), DEFAULT_RR);
    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), DEFAULT_RTP_INACTIVITY);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), DEFAULT_RTCP_INACTIVITY);
    delete m_pConfig;
}

TEST_F(MediaConfigurationTest, GetConfigVideo)
{
    MediaConfiguration* m_pConfig = new MediaConfiguration(MEDIA_TYPE_VIDEO);

    EXPECT_EQ(m_pConfig->GetSessionType(), MEDIA_TYPE_VIDEO);
    EXPECT_EQ(m_pConfig->GetPortRtp(), DEFAULT_RTP_PORT);
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), DEFAULT_RTP_PORT_END);
    EXPECT_EQ(m_pConfig->GetPortRtcp(), DEFAULT_RTCP_PORT);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnActive(), DEFAULT_RTCP_INVERVAL_ACTIVE);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnHold(), DEFAULT_RTCP_INVERVAL_HOLD);
    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(), DEFAULT_AS);
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(), DEFAULT_RS);
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(), DEFAULT_RR);
    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), DEFAULT_RTP_INACTIVITY);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), DEFAULT_RTCP_INACTIVITY);
    delete m_pConfig;
}

TEST_F(MediaConfigurationTest, GetConfigText)
{
    MediaConfiguration* m_pConfig = new MediaConfiguration(MEDIA_TYPE_TEXT);

    EXPECT_EQ(m_pConfig->GetSessionType(), MEDIA_TYPE_TEXT);
    EXPECT_EQ(m_pConfig->GetPortRtp(), DEFAULT_RTP_PORT);
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), DEFAULT_RTP_PORT_END);
    EXPECT_EQ(m_pConfig->GetPortRtcp(), DEFAULT_RTCP_PORT);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnActive(), DEFAULT_RTCP_INVERVAL_ACTIVE);
    EXPECT_EQ(m_pConfig->GetRtcpIntervalOnHold(), DEFAULT_RTCP_INVERVAL_HOLD);
    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(), DEFAULT_AS);
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(), DEFAULT_RS);
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(), DEFAULT_RR);
    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), DEFAULT_RTP_INACTIVITY);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), DEFAULT_RTCP_INACTIVITY);
    delete m_pConfig;
}
