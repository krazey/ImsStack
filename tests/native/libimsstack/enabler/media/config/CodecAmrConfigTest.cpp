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
#include "config/MediaCarrierConfigBundle.h"
#include "config/CodecAmrConfig.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::AUDIO_AMR_WB;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;
static const IMS_SINT32 DEFAULT_CHANNEL = CodecAmrConfig::DEFAULT_CHANNEL;
static const IMS_SINT32 DEFAULT_OCTET_ALIGN = CodecAmrConfig::DEFAULT_OCTET_ALIGN;
static const IMS_SINT32 DEFAULT_MODESET_AMR_WB = CodecAmrConfig::DEFAULT_MODESET_AMR_WB;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMRWB = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMRWB;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMR = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMR;

class CodecAmrConfigTest : public ::testing::Test {
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override
    {
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {}
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
    ImsVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetIntArray(pszKey);
    }
};

TEST_F(CodecAmrConfigTest, GetConfigDefault)
{
    CodecAmrConfig* m_pConfig = new CodecAmrConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);

    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(m_pConfig->GetDefaultModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(m_pConfig->GetShowModeSet(), IMS_FALSE);
    EXPECT_EQ(m_pConfig->GetOctetAlign(), DEFAULT_OCTET_ALIGN);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);
    EXPECT_EQ(m_pConfig->GetDtx(), IMS_TRUE);

    delete m_pConfig;
}

TEST_F(CodecAmrConfigTest, GetConfigOctetAlignAsset)
{
    CodecAmrConfig* m_pConfig_amrwb;
    CodecAmrConfig* m_pConfig_amrnb;

    ImsVector<IMS_SINT32> objOctetAlign;
    objOctetAlign.Push(1);
    objOctetAlign.Push(0);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_ASSET_AMRWB_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT_ARRAY))
            .WillByDefault(Return(objOctetAlign));

    m_pConfig_amrwb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, 99);

    EXPECT_TRUE(m_pConfig_amrwb->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(m_pConfig_amrwb->GetOctetAlign(), 1);
    delete m_pConfig_amrwb;

    m_pConfig_amrwb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, 100);

    EXPECT_TRUE(m_pConfig_amrwb->Create(pMockICarrierConfig, 1));
    EXPECT_EQ(m_pConfig_amrwb->GetOctetAlign(), 0);
    delete m_pConfig_amrwb;

    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_ASSET_AMRNB_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT_ARRAY))
            .WillByDefault(Return(objOctetAlign));

    m_pConfig_amrnb = new CodecAmrConfig(ImsCodec::AUDIO_AMR, 96);

    EXPECT_TRUE(m_pConfig_amrnb->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(m_pConfig_amrnb->GetOctetAlign(), 1);
    delete m_pConfig_amrnb;

    m_pConfig_amrnb = new CodecAmrConfig(ImsCodec::AUDIO_AMR, 97);

    EXPECT_TRUE(m_pConfig_amrnb->Create(pMockICarrierConfig, 1));
    EXPECT_EQ(m_pConfig_amrnb->GetOctetAlign(), 0);
    delete m_pConfig_amrnb;

    delete pMockICarrierConfig;
}

TEST_F(CodecAmrConfigTest, GetConfigShowModeSetListAsset)
{
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    CodecAmrConfig* m_pConfig_amrwb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, 99);
    IMS_BOOL bMockShowModeSet = IMS_TRUE;

    ON_CALL(*pMockICarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL, IMS_FALSE))
            .WillByDefault(Return(bMockShowModeSet));

    EXPECT_TRUE(m_pConfig_amrwb->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(m_pConfig_amrwb->GetShowModeSet(), IMS_TRUE);

    delete m_pConfig_amrwb;
    delete pMockICarrierConfig;
}

TEST_F(CodecAmrConfigTest, GetConfigModeSetListAsset)
{
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    CodecAmrConfig* m_pConfig_amrwb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, 99);
    ImsVector<IMS_SINT32> objCodecAttributeModesetWb;
    objCodecAttributeModesetWb.Push(0);
    objCodecAttributeModesetWb.Push(1);
    objCodecAttributeModesetWb.Push(2);

    ON_CALL(*pMockICarrierConfig,
            GetIntArray(
                    CarrierConfig::Assets::KEY_ASSET_AMR_AMRWB_CODEC_ATTRIBUTE_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeModesetWb));

    EXPECT_TRUE(m_pConfig_amrwb->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_amrwb->GetModeSetList(), 7);
    EXPECT_EQ(m_pConfig_amrwb->GetModeSet(), 2);
    delete m_pConfig_amrwb;

    CodecAmrConfig* m_pConfig_amrnb = new CodecAmrConfig(ImsCodec::AUDIO_AMR, 100);
    ImsVector<IMS_SINT32> objCodecAttributeModesetNb;
    objCodecAttributeModesetNb.Push(0);
    objCodecAttributeModesetNb.Push(2);
    objCodecAttributeModesetNb.Push(4);
    objCodecAttributeModesetNb.Push(7);

    ON_CALL(*pMockICarrierConfig,
            GetIntArray(
                    CarrierConfig::Assets::KEY_ASSET_AMR_AMRNB_CODEC_ATTRIBUTE_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeModesetNb));

    EXPECT_TRUE(m_pConfig_amrnb->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_amrnb->GetModeSetList(), 149);
    EXPECT_EQ(m_pConfig_amrnb->GetModeSet(), 7);

    delete m_pConfig_amrnb;
    delete pMockICarrierConfig;
}

TEST_F(CodecAmrConfigTest, GetConfigDefaultModeSetListAsset)
{
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    CodecAmrConfig* m_pConfig_amrwb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, 99);
    ImsVector<IMS_SINT32> objCodecAttributeDefaultModesetWb;
    objCodecAttributeDefaultModesetWb.Push(0);
    objCodecAttributeDefaultModesetWb.Push(1);
    objCodecAttributeDefaultModesetWb.Push(2);

    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeDefaultModesetWb));

    EXPECT_TRUE(m_pConfig_amrwb->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_amrwb->GetDefaultModeSetList(), 7);
    delete m_pConfig_amrwb;

    CodecAmrConfig* m_pConfig_amrnb = new CodecAmrConfig(ImsCodec::AUDIO_AMR, 100);
    ImsVector<IMS_SINT32> objCodecAttributeDefaultModesetNb;
    objCodecAttributeDefaultModesetNb.Push(0);
    objCodecAttributeDefaultModesetNb.Push(2);
    objCodecAttributeDefaultModesetNb.Push(4);
    objCodecAttributeDefaultModesetNb.Push(7);

    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::
                            KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeDefaultModesetNb));

    EXPECT_TRUE(m_pConfig_amrnb->Create(pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_amrnb->GetDefaultModeSetList(), 149);

    delete m_pConfig_amrnb;
    delete pMockICarrierConfig;
}

TEST_F(CodecAmrConfigTest, GetConfigSamplingRate)
{
    CodecAmrConfig* m_pConfig_amrwb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, 99);

    EXPECT_TRUE(m_pConfig_amrwb->Create(m_piCc, 0));
    EXPECT_EQ(m_pConfig_amrwb->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);

    CodecAmrConfig* m_pConfig_amrnb = new CodecAmrConfig(ImsCodec::AUDIO_AMR, 100);

    EXPECT_TRUE(m_pConfig_amrnb->Create(m_piCc, 0));
    EXPECT_EQ(m_pConfig_amrnb->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMR);

    delete m_pConfig_amrwb;
    delete m_pConfig_amrnb;
}