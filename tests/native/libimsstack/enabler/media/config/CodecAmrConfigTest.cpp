/*
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
#include <config/MockMediaConfiguration.h>
#include "config/MediaCarrierConfigBundle.h"
#include "config/CodecAudioConfig.h"
#include "config/CodecAmrConfig.h"

using ::testing::_;
using ::testing::Return;

static const IMS_SINT32 DEFAULT_CHANNEL = CodecAmrConfig::DEFAULT_CHANNEL;
static const IMS_BOOL DEFAULT_DTX = CodecAudioConfig::DEFAULT_DTX;
static const IMS_SINT32 DEFAULT_PAYLOAD_FORMAT = CodecAmrConfig::DEFAULT_PAYLOAD_FORMAT;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMRWB = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMRWB;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMR = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMR;
static const IMS_SINT32 DEFAULT_MODESET_AMRNB = CodecAudioConfig::DEFAULT_MODESET_AMRNB;
static const IMS_SINT32 DEFAULT_MODESET_AMRWB = CodecAudioConfig::DEFAULT_MODESET_AMRWB;
static const IMS_SINT32 FULL_MODESET_AMRNB = CodecAudioConfig::FULL_MODESET_AMRNB;
static const IMS_SINT32 FULL_MODESET_AMRWB = CodecAudioConfig::FULL_MODESET_AMRWB;
static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY =
        CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY;
static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD = CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD;
static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR = CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR;

class CodecAmrConfigTest : public ::testing::Test
{
protected:
    std::unique_ptr<MockICarrierConfig> m_pMockICarrierConfig;
    std::unique_ptr<MockICarrierConfig> m_pMockBundle;
    std::unique_ptr<MockICarrierConfig> m_pMockSubBundle;

    void SetUp() override
    {
        m_pMockICarrierConfig = std::make_unique<MockICarrierConfig>();
        m_pMockBundle = std::make_unique<MockICarrierConfig>();
        m_pMockSubBundle = std::make_unique<MockICarrierConfig>();

        EXPECT_CALL(*m_pMockBundle, ReleaseBundle()).Times(::testing::AnyNumber());
        EXPECT_CALL(*m_pMockSubBundle, ReleaseBundle()).Times(::testing::AnyNumber());
    }

    void TearDown() override {}
};

// Test case 1: piCc is IMS_NULL
TEST_F(CodecAmrConfigTest, Create_NullCarrierConfig)
{
    CodecAmrConfig codecConfig(ImsCodec::AUDIO_AMR, 100);

    IMS_BOOL bResult = codecConfig.Create(IMS_NULL);

    ASSERT_EQ(bResult, IMS_FALSE);
}

// Test case 2: Codec is AUDIO_AMR and carrier config is valid
TEST_F(CodecAmrConfigTest, Create_AmrCodec)
{
    IMS_SINT32 nCodecType = ImsCodec::AUDIO_AMR;
    IMS_SINT32 nPayloadType = 100;
    IMS_SINT32 nOctetAlign = 1;
    ImsVector<IMS_SINT32> objModesetArray;
    objModesetArray.Push(0);
    objModesetArray.Push(1);
    objModesetArray.Push(4);
    objModesetArray.Push(7);
    IMS_SINT32 nModeSetList = (1 << 0) | (1 << 1) | (1 << 4) | (1 << 7);
    IMS_SINT32 nModeChangeCapability = 1;
    IMS_SINT32 nModeChangePeriod = 2;
    IMS_SINT32 nModeChangeNeighbor = 3;

    CodecAmrConfig codecConfig(nCodecType, nPayloadType);

    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));  // Default behavior, can be adjusted if needed
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL, DEFAULT_DTX))
            .WillOnce(Return(DEFAULT_DTX));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillOnce(Return(m_pMockBundle.get()));

    EXPECT_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::
                                KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY,
                    _))
            .WillOnce(Return(ImsVector<IMS_SINT32>()));  // Assuming empty or default list

    EXPECT_CALL(*m_pMockBundle, GetBundle(AString(std::to_string(nPayloadType).c_str()).GetStr()))
            .WillOnce(Return(m_pMockSubBundle.get()));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    CodecAmrConfig::DEFAULT_PAYLOAD_FORMAT))
            .WillOnce(Return(nOctetAlign));
    EXPECT_CALL(*m_pMockSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY, _))
            .WillOnce(Return(ImsVector<IMS_SINT32>(objModesetArray)));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    CodecAmrConfig::NOT_DEFINED))
            .WillOnce(Return(nModeChangeCapability));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::
                               KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_CAPABILITY_BOOL,
                    IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    CodecAmrConfig::NOT_DEFINED))
            .WillOnce(Return(nModeChangePeriod));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_PERIOD_BOOL,
                    IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT,
                    CodecAmrConfig::NOT_DEFINED))
            .WillOnce(Return(nModeChangeNeighbor));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(
                    CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_NEIGHBOR_BOOL,
                    IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    IMS_BOOL bResult = codecConfig.Create(m_pMockICarrierConfig.get());

    ASSERT_EQ(bResult, IMS_TRUE);
    ASSERT_EQ(codecConfig.GetCodec(), nCodecType);
    ASSERT_EQ(codecConfig.GetPayloadType(), nPayloadType);
    ASSERT_EQ(codecConfig.GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMR);
    ASSERT_EQ(codecConfig.GetDefaultModeSetList(), CodecAudioConfig::FULL_MODESET_AMRNB);
    ASSERT_EQ(codecConfig.GetOctetAlign(), nOctetAlign);
    ASSERT_EQ(codecConfig.GetModeSetList(), nModeSetList);
    ASSERT_EQ(codecConfig.GetModeChangeCapability(), nModeChangeCapability);
    ASSERT_EQ(codecConfig.GetModeChangePeriod(), nModeChangePeriod);
    ASSERT_EQ(codecConfig.GetModeChangeNeighbor(), nModeChangeNeighbor);
    EXPECT_EQ(codecConfig.GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeCapability(), IMS_TRUE);
    EXPECT_EQ(codecConfig.GetVisibleModeChangePeriod(), IMS_TRUE);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeNeighbor(), IMS_TRUE);
}

// Test case 3: Codec is AUDIO_AMR_WB and carrier config is valid
TEST_F(CodecAmrConfigTest, Create_AmrWbCodec)
{
    IMS_SINT32 nCodecType = ImsCodec::AUDIO_AMR_WB;
    IMS_SINT32 nPayloadType = 101;
    IMS_SINT32 nOctetAlign = 0;
    ImsVector<IMS_SINT32> objModesetArray;
    objModesetArray.Push(6);
    objModesetArray.Push(8);
    objModesetArray.Push(9);

    IMS_UINT32 nModeSetList = (1 << 6) | (1 << 8) | (1 << 9);
    IMS_SINT32 nModeChangeCapability = CodecAmrConfig::NOT_DEFINED;
    IMS_SINT32 nModeChangePeriod = CodecAmrConfig::NOT_DEFINED;
    IMS_SINT32 nModeChangeNeighbor = CodecAmrConfig::NOT_DEFINED;

    CodecAmrConfig codecConfig(nCodecType, nPayloadType);

    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));  // Default behavior, can be adjusted if needed
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL, DEFAULT_DTX))
            .WillOnce(Return(DEFAULT_DTX));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillOnce(Return(m_pMockBundle.get()));

    EXPECT_CALL(*m_pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::
                                KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY,
                    _))
            .WillOnce(Return(ImsVector<IMS_SINT32>()));  // Assuming empty or default list
    EXPECT_CALL(*m_pMockBundle, GetBundle(AString(std::to_string(nPayloadType).c_str()).GetStr()))
            .WillOnce(Return(m_pMockSubBundle.get()));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    CodecAmrConfig::DEFAULT_PAYLOAD_FORMAT))
            .WillOnce(Return(nOctetAlign));
    EXPECT_CALL(*m_pMockSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY, _))
            .WillOnce(Return(ImsVector<IMS_SINT32>(objModesetArray)));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    CodecAmrConfig::NOT_DEFINED))
            .WillOnce(Return(nModeChangeCapability));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    CodecAmrConfig::NOT_DEFINED))
            .WillOnce(Return(nModeChangePeriod));
    EXPECT_CALL(*m_pMockSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT,
                    CodecAmrConfig::NOT_DEFINED))
            .WillOnce(Return(nModeChangeNeighbor));

    IMS_BOOL bResult = codecConfig.Create(m_pMockICarrierConfig.get());

    ASSERT_EQ(bResult, IMS_TRUE);
    ASSERT_EQ(codecConfig.GetCodec(), nCodecType);
    ASSERT_EQ(codecConfig.GetPayloadType(), nPayloadType);
    ASSERT_EQ(codecConfig.GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);
    ASSERT_EQ(codecConfig.GetDefaultModeSetList(), FULL_MODESET_AMRWB);
    ASSERT_EQ(codecConfig.GetOctetAlign(), nOctetAlign);
    ASSERT_EQ(codecConfig.GetModeSetList(), nModeSetList);
    ASSERT_EQ(codecConfig.GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    // Values should be defaults from CodecAudioConfig as they were not in sub-bundle
    ASSERT_EQ(codecConfig.GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    ASSERT_EQ(codecConfig.GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);
    EXPECT_EQ(codecConfig.GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeCapability(), IMS_TRUE);
    // Visibility should be false as they were not in sub-bundle
    EXPECT_EQ(codecConfig.GetVisibleModeChangePeriod(), IMS_FALSE);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeNeighbor(), IMS_FALSE);
}

// Test case 4: GetBundle for the main bundle returns IMS_NULL
TEST_F(CodecAmrConfigTest, Create_NullBundle)
{
    IMS_SINT32 nCodecType = ImsCodec::AUDIO_AMR;
    IMS_SINT32 nPayloadType = 100;

    CodecAmrConfig codecConfig(nCodecType, nPayloadType);

    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));  // Default behavior, can be adjusted if needed
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillOnce(Return(IMS_NULL));

    IMS_BOOL bResult = codecConfig.Create(m_pMockICarrierConfig.get());
    ASSERT_EQ(bResult, IMS_TRUE);

    ASSERT_EQ(codecConfig.GetCodec(), nCodecType);
    ASSERT_EQ(codecConfig.GetPayloadType(), nPayloadType);
    ASSERT_EQ(codecConfig.GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMR);
    ASSERT_EQ(codecConfig.GetDefaultModeSetList(), FULL_MODESET_AMRNB);
    ASSERT_EQ(codecConfig.GetOctetAlign(), DEFAULT_PAYLOAD_FORMAT);
    ASSERT_EQ(codecConfig.GetModeSetList(), FULL_MODESET_AMRNB);
    ASSERT_EQ(codecConfig.GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    // Values should be defaults from CodecAudioConfig as they were not in sub-bundle
    ASSERT_EQ(codecConfig.GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    ASSERT_EQ(codecConfig.GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);
    EXPECT_EQ(codecConfig.GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeCapability(), IMS_TRUE);
    // Visibility should be false as they were not in sub-bundle
    EXPECT_EQ(codecConfig.GetVisibleModeChangePeriod(), IMS_FALSE);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeNeighbor(), IMS_FALSE);
}

// Test case 5: GetBundle for the sub-bundle returns IMS_NULL
TEST_F(CodecAmrConfigTest, Create_NullSubBundle)
{
    IMS_SINT32 nCodecType = ImsCodec::AUDIO_AMR;
    IMS_SINT32 nPayloadType = 100;

    CodecAmrConfig codecConfig(nCodecType, nPayloadType);

    // Set expectations: main bundle is returned, but sub-bundle is IMS_NULL
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillOnce(Return(m_pMockBundle.get()));
    EXPECT_CALL(*m_pMockBundle, GetBundle(AString(std::to_string(nPayloadType).c_str()).GetStr()))
            .WillOnce(Return(IMS_NULL));

    IMS_BOOL bResult = codecConfig.Create(m_pMockICarrierConfig.get());

    ASSERT_EQ(bResult, IMS_TRUE);
    ASSERT_EQ(codecConfig.GetCodec(), nCodecType);
    ASSERT_EQ(codecConfig.GetPayloadType(), nPayloadType);
    ASSERT_EQ(codecConfig.GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMR);
    ASSERT_EQ(codecConfig.GetDefaultModeSetList(), FULL_MODESET_AMRNB);
    ASSERT_EQ(codecConfig.GetOctetAlign(), DEFAULT_PAYLOAD_FORMAT);
    ASSERT_EQ(codecConfig.GetModeSetList(), FULL_MODESET_AMRNB);
    ASSERT_EQ(codecConfig.GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    // Values should be defaults from CodecAudioConfig as they were not in sub-bundle
    ASSERT_EQ(codecConfig.GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    ASSERT_EQ(codecConfig.GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);
    EXPECT_EQ(codecConfig.GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeCapability(), IMS_TRUE);
    // Visibility should be false as they were not in sub-bundle
    EXPECT_EQ(codecConfig.GetVisibleModeChangePeriod(), IMS_FALSE);
    EXPECT_EQ(codecConfig.GetVisibleModeChangeNeighbor(), IMS_FALSE);
}

// Test case 6: Modeset array processing with various values
TEST_F(CodecAmrConfigTest, Create_ModesetArrayProcessing)
{
    IMS_SINT32 nCodecType = ImsCodec::AUDIO_AMR;
    IMS_SINT32 nPayloadType = 100;

    {
        CodecAmrConfig codecConfig(nCodecType, nPayloadType);
        ImsVector<IMS_SINT32> objEmptyModesetArray;

        EXPECT_CALL(*m_pMockICarrierConfig,
                GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL,
                        IMS_FALSE))
                .WillOnce(Return(IMS_FALSE));
        EXPECT_CALL(*m_pMockICarrierConfig,
                GetBoolean(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL, DEFAULT_DTX))
                .WillOnce(Return(DEFAULT_DTX));
        EXPECT_CALL(*m_pMockICarrierConfig, GetBundle(_)).WillOnce(Return(m_pMockBundle.get()));
        EXPECT_CALL(*m_pMockBundle, GetBundle(_)).WillOnce(Return(m_pMockSubBundle.get()));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT, _))
                .WillOnce(Return(DEFAULT_PAYLOAD_FORMAT));
        EXPECT_CALL(*m_pMockSubBundle,
                GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY, _))
                .WillOnce(Return(ImsVector<IMS_SINT32>(objEmptyModesetArray)));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT, _))
                .WillOnce(Return(CodecAmrConfig::NOT_DEFINED));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT, _))
                .WillOnce(Return(CodecAmrConfig::NOT_DEFINED));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT, _))
                .WillOnce(Return(CodecAmrConfig::NOT_DEFINED));

        IMS_BOOL bResult = codecConfig.Create(m_pMockICarrierConfig.get());
        ASSERT_EQ(bResult, IMS_TRUE);
        ASSERT_EQ(codecConfig.GetModeSetList(), CodecAudioConfig::FULL_MODESET_AMRNB);
    }

    {
        CodecAmrConfig codecConfig(nCodecType, nPayloadType);
        ImsVector<IMS_SINT32> objModesetArrayWithNegative;
        objModesetArrayWithNegative.Push(0);
        objModesetArrayWithNegative.Push(1);
        objModesetArrayWithNegative.Push(-2);
        objModesetArrayWithNegative.Push(7);

        EXPECT_CALL(*m_pMockICarrierConfig,
                GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL,
                        IMS_FALSE))
                .WillOnce(Return(IMS_FALSE));
        EXPECT_CALL(*m_pMockICarrierConfig,
                GetBoolean(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL, DEFAULT_DTX))
                .WillOnce(Return(DEFAULT_DTX));
        EXPECT_CALL(*m_pMockICarrierConfig, GetBundle(_)).WillOnce(Return(m_pMockBundle.get()));
        EXPECT_CALL(*m_pMockBundle, GetBundle(_)).WillOnce(Return(m_pMockSubBundle.get()));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT, _))
                .WillOnce(Return(DEFAULT_PAYLOAD_FORMAT));
        EXPECT_CALL(*m_pMockSubBundle,
                GetIntArray(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY, _))
                .WillOnce(Return(ImsVector<IMS_SINT32>(objModesetArrayWithNegative)));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT, _))
                .WillOnce(Return(CodecAmrConfig::NOT_DEFINED));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT, _))
                .WillOnce(Return(CodecAmrConfig::NOT_DEFINED));
        EXPECT_CALL(*m_pMockSubBundle,
                GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT, _))
                .WillOnce(Return(CodecAmrConfig::NOT_DEFINED));

        IMS_BOOL bResult = codecConfig.Create(m_pMockICarrierConfig.get());
        ASSERT_EQ(bResult, IMS_TRUE);
        ASSERT_EQ(codecConfig.GetModeSetList(), (1 << 0) | (1 << 1));
    }
}
