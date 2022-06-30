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

class CodecAmrConfigTest : public ::testing::Test {

public :
    CodecAmrConfig* m_pConfig;
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {
        m_pConfig = new CodecAmrConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {
        if (m_pConfig)
        {
            delete m_pConfig;
        }
    }
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
    IMSVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetIntArray(pszKey);
    }
};

TEST_F(CodecAmrConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(m_pConfig->GetOctetAlign(), DEFAULT_OCTET_ALIGN);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);
    EXPECT_EQ(m_pConfig->GetDtx(), IMS_TRUE);
}

// TODO - it is better to change the bundle structure
TEST_F(CodecAmrConfigTest, GetConfigOctetAlign)
{
    m_pConfig->Create(m_piCc);

    EXPECT_GE(m_pConfig->GetOctetAlign(), 0);
    EXPECT_GE(m_pConfig->GetModeSetList(), 0);
    EXPECT_GE(m_pConfig->GetModeSet(), 0);
}

/*TEST_F(CodecAmrConfigTest, GetConfigModeSetList)
{
    IMSVector<IMS_SINT32> objCodecAttributeModeset;
    objCodecAttributeModeset.Push(0);
    objCodecAttributeModeset.Push(1);
    objCodecAttributeModeset.Push(2);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeModeset));

    m_pConfig->Create(pMockICarrierConfig);

    EXPECT_EQ(m_pConfig->GetModeSetList(), 7);
    EXPECT_EQ(m_pConfig->GetModeSet(), 2);
}*/
