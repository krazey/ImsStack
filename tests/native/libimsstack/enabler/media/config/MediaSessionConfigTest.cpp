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

#include "ICarrierConfig.h"
#include "CarrierConfig.h"
#include "ServiceConfig.h"
#include "config/MediaSessionConfig.h"

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const MEDIA_SERVICE_TYPE DEFAULT_SERVICE_TYPE = MEDIA_SERVICE_DEFAULT;
static const MEDIA_SERVICE_TYPE EMERGENCY_SERVICE_TYPE = MEDIA_SERVICE_EMERGENCY;
static const IMS_BOOL DEFAULT_SESSION_LEVEL_BW = MediaSessionConfig::DEFAULT_SESSION_LEVEL_BW;
static const IMS_BOOL DEFAULT_ANBR_CAPABILITY = MediaSessionConfig::DEFAULT_ANBR_CAPABILITY;
static const IMS_BOOL DEFAULT_SUPPORT_MULTICONFIG = MediaSessionConfig::DEFAULT_SUPPORT_MULTICONFIG;

class MediaSessionConfigTest : public ::testing::Test
{
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override
    {
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {}

    IMS_BOOL GetBoolean(IN const IMS_CHAR* pszKey) { return m_piCc->GetBoolean(pszKey); }
};

TEST_F(MediaSessionConfigTest, GetConfigDefault)
{
    MediaSessionConfig* m_pConfig = new MediaSessionConfig(DEFAULT_SLOT_ID, MEDIA_SERVICE_DEFAULT);
    EXPECT_EQ(m_pConfig->GetServiceType(), DEFAULT_SERVICE_TYPE);
    EXPECT_EQ(m_pConfig->IsSessionLevelBandwidth(), DEFAULT_SESSION_LEVEL_BW);
    EXPECT_EQ(m_pConfig->IsAnbrSupported(), DEFAULT_ANBR_CAPABILITY);
    EXPECT_EQ(m_pConfig->IsSupportMultiConfigInEarlySession(), DEFAULT_SUPPORT_MULTICONFIG);
    EXPECT_EQ(m_pConfig->IsSdpReofferFullCapability(), IMS_TRUE);

    m_pConfig->SetServiceType(EMERGENCY_SERVICE_TYPE);
    EXPECT_EQ(m_pConfig->GetServiceType(), EMERGENCY_SERVICE_TYPE);

    m_pConfig->SetServiceType(DEFAULT_SERVICE_TYPE);
    EXPECT_EQ(m_pConfig->GetServiceType(), DEFAULT_SERVICE_TYPE);
    delete m_pConfig;
}

TEST_F(MediaSessionConfigTest, GetConfigTest)
{
    MediaSessionConfig* m_pConfig = new MediaSessionConfig(DEFAULT_SLOT_ID, MEDIA_SERVICE_DEFAULT);
    EXPECT_TRUE(m_pConfig->Create(DEFAULT_SLOT_ID));
    EXPECT_EQ(m_pConfig->IsSessionLevelBandwidth(),
            GetBoolean(CarrierConfig::ImsVoice::KEY_MEDIA_SESSION_LEVEL_BANDWIDTH_BOOL));
    EXPECT_EQ(m_pConfig->IsAnbrSupported(),
            GetBoolean(CarrierConfig::ImsVoice::KEY_MEDIA_ANBR_CAPABILITY_IN_MODEM_BOOL));
    EXPECT_EQ(m_pConfig->IsSupportMultiConfigInEarlySession(),
            GetBoolean(CarrierConfig::ImsVoice::KEY_SUPPORT_MULTI_CONFIG_IN_EARLY_SESSION_BOOL));
    EXPECT_EQ(m_pConfig->IsSdpReofferFullCapability(),
            GetBoolean(CarrierConfig::ImsVoice::KEY_SDP_REOFFER_FULL_CAPABILITY_BOOL));
    delete m_pConfig;
}
