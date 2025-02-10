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
#include "MockICarrierConfig.h"
#include "config/CodecT140Config.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::AUDIO_AMR;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;
static const IMS_SINT32 DEFAULT_RED_LEVEL_NONE = CodecT140Config::DEFAULT_RED_LEVEL_NONE;
static const IMS_SINT32 DEFAULT_RED_LEVEL = CodecT140Config::DEFAULT_RED_LEVEL;
static const IMS_SINT32 DEFAULT_TEXT_SAMPLING_RATE = CodecT140Config::DEFAULT_TEXT_SAMPLING_RATE;

class CodecT140ConfigTest : public ::testing::Test {
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override
    {
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {}
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
};

TEST_F(CodecT140ConfigTest, GetConfigDefault)
{
    CodecT140Config* m_pConfig = new CodecT140Config(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    EXPECT_EQ(m_pConfig->GetRedLevel(), DEFAULT_RED_LEVEL);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_TEXT_SAMPLING_RATE);
    delete m_pConfig;
}

TEST_F(CodecT140ConfigTest, GetConfigTest)
{
    CodecT140Config* m_pConfig_redlevel = new CodecT140Config(ImsCodec::TEXT_RED, 120);

    IMS_UINT32 nTextRedLevel = 5;
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_REDUNDANCY_LEVEL_INT, -1))
            .WillByDefault(Return(nTextRedLevel));

    EXPECT_TRUE(m_pConfig_redlevel->Create(pMockICarrierConfig));
    EXPECT_EQ(m_pConfig_redlevel->GetRedLevel(), 5);
    EXPECT_EQ(m_pConfig_redlevel->GetSamplingRate(), DEFAULT_TEXT_SAMPLING_RATE);

    delete pMockICarrierConfig;
    delete m_pConfig_redlevel;
}

TEST_F(CodecT140ConfigTest, GetConfigTestT140)
{
    CodecT140Config* m_pConfig_redlevel = new CodecT140Config(ImsCodec::TEXT_T140, 121);
    IMS_UINT32 nTextRedLevel = 5;
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_REDUNDANCY_LEVEL_INT, -1))
            .WillByDefault(Return(nTextRedLevel));

    EXPECT_TRUE(m_pConfig_redlevel->Create(pMockICarrierConfig));
    EXPECT_EQ(m_pConfig_redlevel->GetRedLevel(), DEFAULT_RED_LEVEL_NONE);
    EXPECT_EQ(m_pConfig_redlevel->GetSamplingRate(), DEFAULT_TEXT_SAMPLING_RATE);

    delete pMockICarrierConfig;
    delete m_pConfig_redlevel;
}
