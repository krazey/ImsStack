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

static const IMS_SINT32 DEFAULT_CHANNEL = CodecAmrConfig::DEFAULT_CHANNEL;
static const IMS_SINT32 DEFAULT_OCTET_ALIGN = CodecAmrConfig::DEFAULT_OCTET_ALIGN;
static const IMS_SINT32 DEFAULT_MODESET_AMR_WB = CodecAmrConfig::DEFAULT_MODESET_AMR_WB;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMRWB = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMRWB;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMR = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMR;
static const IMS_BOOL DEFAULT_AMR_DTX = CodecAmrConfig::DEFAULT_AMR_DTX;
static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY =
        CodecAmrConfig::DEFAULT_MODECHANGE_CAPABILITY;
static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD = CodecAmrConfig::DEFAULT_MODECHANGE_PERIOD;
static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR = CodecAmrConfig::DEFAULT_MODECHANGE_NEIGHBOR;

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class CodecAmrConfigTest : public ::testing::Test {
public:
    ICarrierConfig* m_piCc;
    CodecAmrConfig* m_pConfig_AmrWb;
    CodecAmrConfig* m_pConfig_AmrNb;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pAudioBundle;
    MockICarrierConfig* m_pAudioSubBundle;
    IMS_SINT32 m_nAmrWbPayloadTypeNumber = 101;
    IMS_SINT32 m_nAmrNbPayloadTypeNumber = 99;

protected:
    virtual void SetUp() override
    {
        m_pConfig_AmrWb = new CodecAmrConfig(ImsCodec::AUDIO_AMR_WB, m_nAmrWbPayloadTypeNumber);
        m_pConfig_AmrNb = new CodecAmrConfig(ImsCodec::AUDIO_AMR, m_nAmrNbPayloadTypeNumber);
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pAudioBundle = new MockICarrierConfig();
        m_pAudioSubBundle = new MockICarrierConfig();
    }

    virtual void TearDown() override
    {
        delete m_pConfig_AmrWb;
        delete m_pConfig_AmrNb;
        delete m_pMockICarrierConfig;
        delete m_pAudioBundle;
        delete m_pAudioSubBundle;

        m_pConfig_AmrWb = IMS_NULL;
        m_pConfig_AmrNb = IMS_NULL;
        m_pMockICarrierConfig = IMS_NULL;
        m_pAudioBundle = IMS_NULL;
        m_pAudioSubBundle = IMS_NULL;
    }

    inline void GetReadyToCreateAmrWb()
    {
        AString strPayloadTypeNumber;
        strPayloadTypeNumber.SetNumber(m_nAmrWbPayloadTypeNumber);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAudioBundle));
        ON_CALL(*m_pAudioBundle, GetBundle(IsSameKey(strPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAudioSubBundle));
    }

    inline void GetReadyToCreateAmrNb()
    {
        AString strPayloadTypeNumber;
        strPayloadTypeNumber.SetNumber(m_nAmrNbPayloadTypeNumber);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAudioBundle));
        ON_CALL(*m_pAudioBundle, GetBundle(IsSameKey(strPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAudioSubBundle));
    }
};

TEST_F(CodecAmrConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig_AmrWb->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig_AmrWb->GetModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(m_pConfig_AmrWb->GetDefaultModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(m_pConfig_AmrWb->GetShowModeSet(), IMS_FALSE);
    EXPECT_EQ(m_pConfig_AmrWb->GetOctetAlign(), DEFAULT_OCTET_ALIGN);
    EXPECT_EQ(m_pConfig_AmrWb->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);
    EXPECT_EQ(m_pConfig_AmrWb->GetDtx(), IMS_TRUE);
    EXPECT_EQ(m_pConfig_AmrWb->GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(m_pConfig_AmrWb->GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(m_pConfig_AmrWb->GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);
}

TEST_F(CodecAmrConfigTest, WB_GetConfigOctetAlign)
{
    IMS_SINT32 mOctetAlign = 0;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    DEFAULT_OCTET_ALIGN))
            .WillByDefault(Return(mOctetAlign));
    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetOctetAlign(), mOctetAlign);
}

TEST_F(CodecAmrConfigTest, NB_GetConfigOctetAlign)
{
    IMS_SINT32 mOctetAlign = 1;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    DEFAULT_OCTET_ALIGN))
            .WillByDefault(Return(mOctetAlign));
    GetReadyToCreateAmrNb();
    EXPECT_TRUE(m_pConfig_AmrNb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrNb->GetOctetAlign(), mOctetAlign);
}

TEST_F(CodecAmrConfigTest, WB_GetChannel)
{
    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetChannel(), DEFAULT_CHANNEL);
}

TEST_F(CodecAmrConfigTest, NB_GetChannel)
{
    GetReadyToCreateAmrNb();
    EXPECT_TRUE(m_pConfig_AmrNb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrNb->GetChannel(), DEFAULT_CHANNEL);
}

TEST_F(CodecAmrConfigTest, GetConfigShowModeSetList)
{
    IMS_BOOL bMockShowModeSet = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL, IMS_FALSE))
            .WillByDefault(Return(bMockShowModeSet));

    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetShowModeSet(), bMockShowModeSet);
}

TEST_F(CodecAmrConfigTest, WB_GetConfigModeSetList)
{
    ImsVector<IMS_SINT32> objCodecAttributeModesetWb;
    objCodecAttributeModesetWb.Push(0);
    objCodecAttributeModesetWb.Push(1);
    objCodecAttributeModesetWb.Push(2);

    ON_CALL(*m_pAudioSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeModesetWb));

    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetModeSetList(), 7);
    EXPECT_EQ(m_pConfig_AmrWb->GetModeSet(), 2);
}

TEST_F(CodecAmrConfigTest, NB_GetConfigModeSetList)
{
    ImsVector<IMS_SINT32> objCodecAttributeModesetNb;
    objCodecAttributeModesetNb.Push(0);
    objCodecAttributeModesetNb.Push(2);
    objCodecAttributeModesetNb.Push(4);
    objCodecAttributeModesetNb.Push(7);

    ON_CALL(*m_pAudioSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY))
            .WillByDefault(Return(objCodecAttributeModesetNb));
    GetReadyToCreateAmrNb();
    EXPECT_TRUE(m_pConfig_AmrNb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrNb->GetModeSetList(), 149);
    EXPECT_EQ(m_pConfig_AmrNb->GetModeSet(), 7);
}

TEST_F(CodecAmrConfigTest, WB_GetConfigSamplingRate)
{
    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);
}

TEST_F(CodecAmrConfigTest, NB_GetConfigSamplingRate)
{
    GetReadyToCreateAmrNb();
    EXPECT_TRUE(m_pConfig_AmrNb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrNb->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMR);
}

TEST_F(CodecAmrConfigTest, GetDtx)
{
    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetDtx(), DEFAULT_AMR_DTX);
}

TEST_F(CodecAmrConfigTest, GetModeChangeCapability)
{
    IMS_SINT32 nModeChangeCapability = 10;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT, -1))
            .WillByDefault(Return(nModeChangeCapability));

    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetModeChangeCapability(), nModeChangeCapability);
}

TEST_F(CodecAmrConfigTest, GetModeChangePeriod)
{
    IMS_SINT32 nModeChangePeriod = 20;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT, -1))
            .WillByDefault(Return(nModeChangePeriod));

    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetModeChangePeriod(), nModeChangePeriod);
}

TEST_F(CodecAmrConfigTest, GetModeChangeNeighbor)
{
    IMS_SINT32 nModeChangeNeighbor = 30;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT, -1))
            .WillByDefault(Return(nModeChangeNeighbor));

    GetReadyToCreateAmrWb();
    EXPECT_TRUE(m_pConfig_AmrWb->Create(m_pMockICarrierConfig, 0));

    EXPECT_EQ(m_pConfig_AmrWb->GetModeChangeNeighbor(), nModeChangeNeighbor);
}