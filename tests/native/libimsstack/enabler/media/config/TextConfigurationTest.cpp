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

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_PAYLOAD_T140 = TextConfiguration::DEFAULT_PAYLOAD_T140;
static const IMS_SINT32 DEFAULT_PAYLOAD_RED = TextConfiguration::DEFAULT_PAYLOAD_RED;
static const IMS_SINT32 DEFAULT_TEXT_DSCP = TextConfiguration::DEFAULT_TEXT_DSCP;
static const IMS_BOOL DEFAULT_EMPTY_REDUNDANT = TextConfiguration::DEFAULT_EMPTY_REDUNDANT;

class TextConfigurationTest : public ::testing::Test
{
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}

    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
    IMS_BOOL GetBoolean(IN const IMS_CHAR* pszKey) { return m_piCc->GetBoolean(pszKey); }
    AString GetString(IN const IMS_CHAR* pszKey) { return m_piCc->GetString(pszKey); }
    ImsVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetIntArray(pszKey);
    }
    ImsVector<AString> GetStringArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetStringArray(pszKey);
    }
};

TEST_F(TextConfigurationTest, GetConfigDefault)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_EQ(m_pConfig->GetT140PayloadType(), DEFAULT_PAYLOAD_T140);
    EXPECT_EQ(m_pConfig->GetRedPayloadType(), DEFAULT_PAYLOAD_RED);
    EXPECT_EQ(m_pConfig->GetTextDscp(), DEFAULT_TEXT_DSCP);
    EXPECT_EQ(m_pConfig->IsTextCodecEmptyRedundantEnabled(), DEFAULT_EMPTY_REDUNDANT);

    delete m_pConfig;
}

TEST_F(TextConfigurationTest, GetConfigTest)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(m_piCc));

    EXPECT_EQ(m_pConfig->GetT140PayloadType(),
            GetInt(CarrierConfig::Assets::KEY_ASSET_T140_PAYLOAD_TYPE_INT));
    EXPECT_EQ(m_pConfig->GetRedPayloadType(),
            GetInt(CarrierConfig::Assets::KEY_ASSET_RED_PAYLOAD_TYPE_INT));
    EXPECT_EQ(m_pConfig->IsTextCodecEmptyRedundantEnabled(),
            GetBoolean(CarrierConfig::Assets::KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL));

    delete m_pConfig;
}

TEST_F(TextConfigurationTest, GetConfigTextPort)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);

    ImsVector<IMS_SINT32> objTextPortRtp;
    objTextPortRtp.Push(50100);
    objTextPortRtp.Push(50500);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY))
            .WillByDefault(Return(objTextPortRtp));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetPortRtp(), objTextPortRtp.GetAt(0));
    EXPECT_EQ(m_pConfig->GetPortRtpEnd(), objTextPortRtp.GetAt(1));
    EXPECT_EQ(m_pConfig->GetPortRtcp(), objTextPortRtp.GetAt(0) + 1);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(TextConfigurationTest, GetConfigTextRtcpInterval)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);

    ImsVector<IMS_SINT32> objTextRtcpInterval;
    objTextRtcpInterval.Push(3);
    objTextRtcpInterval.Push(10);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INTERVAL_INT_ARRAY))
            .WillByDefault(Return(objTextRtcpInterval));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetRtcpLiveInterval(), objTextRtcpInterval.GetAt(0));
    EXPECT_EQ(m_pConfig->GetRtcpInterval(), objTextRtcpInterval.GetAt(1));

    delete pMockICarrierConfig;
    delete m_pConfig;
}

// TODO: need to check later - due to access fail
TEST_F(TextConfigurationTest, GetConfigTextBandwidth)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(m_piCc));

    EXPECT_EQ(m_pConfig->GetAsBandwidthKbps(),
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_AS_BANDWIDTH_KBPS_INT));
    EXPECT_EQ(m_pConfig->GetRsBandwidthBps(),
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RR_BANDWIDTH_BPS_INT));
    EXPECT_EQ(m_pConfig->GetRrBandwidthBps(),
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RS_BANDWIDTH_BPS_INT));

    delete m_pConfig;
}

TEST_F(TextConfigurationTest, GetConfigTextInactivityTimer)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
    IMS_UINT32 nMockTextRtpInactivity = 5000;
    IMS_UINT32 nMockTextRtcpInactivity = 10000;

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_TEXT_RTP_INACTIVITY_TIMER_MILLIS_INT, -1))
            .WillByDefault(Return(nMockTextRtpInactivity));
    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_TEXT_RTCP_INACTIVITY_TIMER_MILLIS_INT, -1))
            .WillByDefault(Return(nMockTextRtcpInactivity));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));
    EXPECT_EQ(m_pConfig->GetRtpInactivityTimerMillis(), nMockTextRtpInactivity);
    EXPECT_EQ(m_pConfig->GetRtcpInactivityTimerMillis(), nMockTextRtcpInactivity);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(TextConfigurationTest, GetConfigTextDscp)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
    IMS_UINT32 nMockTextDscp = 40;

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig, GetInt(CarrierConfig::Assets::KEY_TEXT_RTP_DSCP_INT, -1))
            .WillByDefault(Return(nMockTextDscp));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetTextDscp(), nMockTextDscp << 2);

    delete pMockICarrierConfig;
    delete m_pConfig;
}

TEST_F(TextConfigurationTest, GetConfigTextCodecEmptyRedundantEnabled)
{
    TextConfiguration* m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);
    IMS_BOOL bTextEmptyRedundantEnabled = IMS_TRUE;

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL, IMS_FALSE))
            .WillByDefault(Return(bTextEmptyRedundantEnabled));

    EXPECT_TRUE(m_pConfig->Create(pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->IsTextCodecEmptyRedundantEnabled(), bTextEmptyRedundantEnabled);

    delete pMockICarrierConfig;
    delete m_pConfig;
}