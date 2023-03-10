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
#include "config/CodecHevcConfig.h"

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::VIDEO_HEVC;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;
static const IMS_SINT32 DEFAULT_RESOLUTION_WIDTH = CodecHevcConfig::DEFAULT_HEVC_RESOLUTION_WIDTH;
static const IMS_SINT32 DEFAULT_RESOLUTION_HEIGHT = CodecHevcConfig::DEFAULT_HEVC_RESOLUTION_HEIGHT;
static const IMS_SINT32 DEFAULT_FRAMERATE = CodecHevcConfig::DEFAULT_HEVC_FRAMERATE;
static const IMS_SINT32 DEFAULT_BITRATE = CodecHevcConfig::DEFAULT_HEVC_BITRATE;
static const IMS_SINT32 DEFAULT_PACKETIZATION_MODE = CodecHevcConfig::DEFAULT_PACKETIZATION_MODE;
static const IMS_BOOL DEFAULT_INCLUDE_SPROP = CodecHevcConfig::DEFAULT_INCLUDE_SPROP;
static const IMS_SINT32 DEFAULT_HEVC_PROFILE = CodecHevcConfig::DEFAULT_HEVC_PROFILE;
static const IMS_SINT32 DEFAULT_HEVC_LEVEL = CodecHevcConfig::DEFAULT_HEVC_LEVEL;
#define DEFAULT_IMAGE_ATTR "NEED_TO_CHECK"
#define DEFAULT_FRAME_SIZE "NEED_TO_CHECK"

class CodecHevcConfigTest : public ::testing::Test {
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override
    {
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
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

TEST_F(CodecHevcConfigTest, GetConfigDefault)
{
    CodecHevcConfig* m_pConfig = new CodecHevcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    EXPECT_EQ(m_pConfig->GetResolutionWidth(), DEFAULT_RESOLUTION_WIDTH);
    EXPECT_EQ(m_pConfig->GetResolutionHeight(), DEFAULT_RESOLUTION_HEIGHT);
    EXPECT_EQ(m_pConfig->GetFramerate(), DEFAULT_FRAMERATE);
    EXPECT_EQ(m_pConfig->GetBitrate(), DEFAULT_BITRATE);
    EXPECT_EQ(m_pConfig->GetPacketizationMode(), DEFAULT_PACKETIZATION_MODE);
    EXPECT_EQ(m_pConfig->GetIncludeSpropParameterSets(), DEFAULT_INCLUDE_SPROP);
    EXPECT_EQ(m_pConfig->GetHevcProfile(), DEFAULT_HEVC_PROFILE);
    EXPECT_EQ(m_pConfig->GetHevcLevel(), DEFAULT_HEVC_LEVEL);
    EXPECT_EQ(m_pConfig->GetImageAttr(), DEFAULT_IMAGE_ATTR);
    EXPECT_EQ(m_pConfig->GetFrameSize(), DEFAULT_FRAME_SIZE);
    delete m_pConfig;
}

TEST_F(CodecHevcConfigTest, GetConfigTest)
{
    CodecHevcConfig* m_pConfig = new CodecHevcConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    EXPECT_FALSE(m_pConfig->Create(m_piCc, 0));  // piCcBundle is null
    delete m_pConfig;
}