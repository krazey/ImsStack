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
#include <config/MediaConfiguration.h>

using ::testing::Return;

static const IMS_SINT32 DEFAULT_RTP_PORT = MediaConfiguration::DEFAULT_RTP_PORT;
static const IMS_SINT32 DEFAULT_RTP_PORT_END = MediaConfiguration::DEFAULT_RTP_PORT_END;
static const IMS_SINT32 DEFAULT_RTCP_PORT = MediaConfiguration::DEFAULT_RTCP_PORT;
static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_LIVE = MediaConfiguration::DEFAULT_RTCP_INVERVAL_LIVE;
static const IMS_SINT32 DEFAULT_RTCP_INVERVAL = MediaConfiguration::DEFAULT_RTCP_INVERVAL;
static const IMS_SINT32 DEFAULT_AS = MediaConfiguration::DEFAULT_AS;
static const IMS_SINT32 DEFAULT_RS = MediaConfiguration::DEFAULT_RS;
static const IMS_SINT32 DEFAULT_RR = MediaConfiguration::DEFAULT_RR;
static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = MediaConfiguration::DEFAULT_RTP_INACTIVITY;
static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = MediaConfiguration::DEFAULT_RTCP_INACTIVITY;

class MediaConfigurationTest : public ::testing::Test {

public:
    MediaConfiguration* pConfig;
protected:
    virtual void SetUp() override {
        pConfig = new MediaConfiguration();
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(MediaConfigurationTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetPortRtp(), DEFAULT_RTP_PORT);
    EXPECT_EQ(pConfig->GetPortRtpEnd(), DEFAULT_RTP_PORT_END);
    EXPECT_EQ(pConfig->GetPortRtcp(), DEFAULT_RTCP_PORT);
    EXPECT_EQ(pConfig->GetRtcpLiveInterval(), DEFAULT_RTCP_INVERVAL_LIVE);
    EXPECT_EQ(pConfig->GetRtcpInterval(), DEFAULT_RTCP_INVERVAL);
    EXPECT_EQ(pConfig->GetAsBandwidthKbps(), DEFAULT_AS);
    EXPECT_EQ(pConfig->GetRsBandwidthBps(), DEFAULT_RS);
    EXPECT_EQ(pConfig->GetRrBandwidthBps(), DEFAULT_RR);
    EXPECT_EQ(pConfig->GetRtpInactivityTimerMillis(), DEFAULT_RTP_INACTIVITY);
    EXPECT_EQ(pConfig->GetRtcpInactivityTimerMillis(), DEFAULT_RTCP_INACTIVITY);
}
