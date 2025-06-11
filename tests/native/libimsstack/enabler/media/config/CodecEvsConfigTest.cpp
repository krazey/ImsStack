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
#include "config/CodecAudioConfig.h"
#include "config/CodecEvsConfig.h"

using ::testing::_;
using ::testing::Return;

static const IMS_SINT32 DEFAULT_CHANNEL = CodecEvsConfig::DEFAULT_CHANNEL;
static const IMS_BOOL DEFAULT_DTX = CodecAudioConfig::DEFAULT_DTX;
static const IMS_BOOL DEFAULT_DTX_RECV = CodecEvsConfig::DEFAULT_DTX_RECV;
static const IMS_SINT32 DEFAULT_HF_ONLY = CodecEvsConfig::DEFAULT_HF_ONLY;
static const IMS_SINT32 DEFAULT_EVS_MODESWITCH = CodecEvsConfig::DEFAULT_EVS_MODESWITCH;
static const IMS_SINT32 DEFAULT_BR_LIST = CodecEvsConfig::DEFAULT_BR_LIST;
static const IMS_SINT32 DEFAULT_BW_LIST = CodecEvsConfig::DEFAULT_BW_LIST;
static const IMS_SINT32 DEFAULT_CMR = CodecEvsConfig::DEFAULT_CMR;
static const IMS_SINT32 DEFAULT_CH_AW_RECV = CodecEvsConfig::DEFAULT_CH_AW_RECV;
static const IMS_SINT32 DEFAULT_MODESET_AMR_WB = CodecAudioConfig::DEFAULT_MODESET_AMR_WB;
static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY =
        CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY;
static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD = CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD;
static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR = CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR;

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class CodecEvsConfigTest : public ::testing::Test
{
protected:
    std::unique_ptr<CodecEvsConfig> m_pCodecEvsConfig;
    std::unique_ptr<MockICarrierConfig> m_pMockICarrierConfig;
    std::unique_ptr<MockICarrierConfig> m_pMockEvsBundle;
    std::unique_ptr<MockICarrierConfig> m_pMockEvsSubBundle;

    IMS_SINT32 m_nDefaultPayloadType = 115;
    AString m_strDefaultPayloadTypeNumber;

    void SetUp() override
    {
        m_pMockICarrierConfig = std::make_unique<MockICarrierConfig>();
        m_pMockEvsBundle = std::make_unique<MockICarrierConfig>();
        m_pMockEvsSubBundle = std::make_unique<MockICarrierConfig>();
        m_strDefaultPayloadTypeNumber.SetNumber(m_nDefaultPayloadType);

        EXPECT_CALL(*m_pMockEvsBundle, ReleaseBundle()).Times(::testing::AnyNumber());
        EXPECT_CALL(*m_pMockEvsSubBundle, ReleaseBundle()).Times(::testing::AnyNumber());
    }

    void TearDown() override {}

    void BuildDefaultEvsCodecValues(const CodecEvsConfig& config)
    {
        // Values from CodecAudioConfig constructor
        EXPECT_EQ(config.GetChannel(), DEFAULT_CHANNEL);
        EXPECT_EQ(config.GetDtx(), DEFAULT_DTX);
        EXPECT_EQ(config.GetAmrModeSetList(), DEFAULT_MODESET_AMR_WB);
        EXPECT_EQ(config.GetDefaultAmrModeSetList(), DEFAULT_MODESET_AMR_WB);
        EXPECT_EQ(config.GetShowAmrModeSet(), IMS_FALSE);
        EXPECT_EQ(config.GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
        EXPECT_EQ(config.GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
        EXPECT_EQ(config.GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);

        // Values from CodecEvsConfig constructor
        EXPECT_EQ(config.GetShowDtx(), IMS_FALSE);
        EXPECT_EQ(config.GetDtxRecv(), DEFAULT_DTX_RECV);
        EXPECT_EQ(config.GetHfOnly(), DEFAULT_HF_ONLY);
        EXPECT_EQ(config.GetEvsModeSwitch(), DEFAULT_EVS_MODESWITCH);
        EXPECT_EQ(config.GetBrList(), DEFAULT_BR_LIST);
        EXPECT_EQ(config.GetBwList(), DEFAULT_BW_LIST);
        EXPECT_EQ(config.GetCmr(), DEFAULT_CMR);
        EXPECT_EQ(config.GetChAwareRecv(), DEFAULT_CH_AW_RECV);
    }
};

TEST_F(CodecEvsConfigTest, Constructor_InitializesWithDefaultValues)
{
    m_pCodecEvsConfig =
            std::make_unique<CodecEvsConfig>(ImsCodec::AUDIO_EVS, m_nDefaultPayloadType);
    BuildDefaultEvsCodecValues(*m_pCodecEvsConfig);
}

TEST_F(CodecEvsConfigTest, Create_NullCarrierConfig)
{
    m_pCodecEvsConfig =
            std::make_unique<CodecEvsConfig>(ImsCodec::AUDIO_EVS, m_nDefaultPayloadType);
    IMS_BOOL bResult = m_pCodecEvsConfig->Create(nullptr);
    ASSERT_EQ(bResult, IMS_FALSE);
}

TEST_F(CodecEvsConfigTest, Create_NullEvsBundle)
{
    m_pCodecEvsConfig =
            std::make_unique<CodecEvsConfig>(ImsCodec::AUDIO_EVS, m_nDefaultPayloadType);
    ON_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pCodecEvsConfig->Create(m_pMockICarrierConfig.get());
    ASSERT_EQ(bResult, IMS_FALSE);
}

TEST_F(CodecEvsConfigTest, Create_NullEvsSubBundle)
{
    m_pCodecEvsConfig =
            std::make_unique<CodecEvsConfig>(ImsCodec::AUDIO_EVS, m_nDefaultPayloadType);

    ON_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillByDefault(Return(m_pMockEvsBundle.get()));
    ON_CALL(*m_pMockEvsBundle,
            GetBundle(AString(std::to_string(m_nDefaultPayloadType).c_str()).GetStr()))
            .WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pCodecEvsConfig->Create(m_pMockICarrierConfig.get());
    ASSERT_EQ(bResult, IMS_FALSE);
}

TEST_F(CodecEvsConfigTest, Create_Successful_ReadsAllValuesFromCarrierConfig)
{
    // Define specific test values
    IMS_SINT32 nChannel = 2;
    IMS_BOOL bShowDtx = IMS_TRUE;
    IMS_BOOL bDtx = IMS_FALSE;
    IMS_BOOL bDtxRecv = IMS_FALSE;
    IMS_SINT32 nHfOnly = 1;
    IMS_SINT32 nEvsModeSwitch = 1;
    ImsVector<IMS_SINT32> objBitrateListArray;  // {5.9kbps, 7.2kbps}
    objBitrateListArray.Push(CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_5_9_KBPS);  // 0
    objBitrateListArray.Push(CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_7_2_KBPS);  // 1
    IMS_UINT32 nExpectedBrList = (1 << 0) | (1 << 1);             // Bitmask for 5.9 and 7.2
    IMS_SINT32 nBwList = CodecEvsConfig::EVS_ENCODED_BW_TYPE_WB;  // 1 (WB only)
    IMS_UINT32 nExpectedBwList = (1 << CodecEvsConfig::EVS_BANDWIDTH_WB);  // (1 << 1) = 2
    IMS_SINT32 nCmr = 1;
    IMS_SINT32 nChAwRecv = 2;
    IMS_BOOL bShowAmrModeSet = IMS_TRUE;
    IMS_UINT32 nAmrModeSetList = 5;  // Example mode for AMR-WB IO
    IMS_UINT32 nDefaultAmrModeSetList = 6;
    IMS_SINT32 nModeChangeCapability = 1;
    IMS_SINT32 nModeChangePeriod = 0;
    IMS_SINT32 nModeChangeNeighbor = 1;

    m_pCodecEvsConfig =
            std::make_unique<CodecEvsConfig>(ImsCodec::AUDIO_EVS, m_nDefaultPayloadType);

    // Setup mock calls
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillOnce(::testing::Return(m_pMockEvsBundle.get()));
    EXPECT_CALL(*m_pMockEvsBundle,
            GetBundle(AString(std::to_string(m_nDefaultPayloadType).c_str()).GetStr()))
            .WillOnce(::testing::Return(m_pMockEvsSubBundle.get()));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT, DEFAULT_CHANNEL))
            .WillOnce(Return(nChannel));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_DTX_BOOL, IMS_FALSE))
            .WillOnce(Return(bShowDtx));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL, DEFAULT_DTX))
            .WillOnce(Return(bDtx));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL,
                    DEFAULT_DTX_RECV))
            .WillOnce(Return(bDtxRecv));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT, DEFAULT_HF_ONLY))
            .WillOnce(Return(nHfOnly));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT,
                    DEFAULT_EVS_MODESWITCH))
            .WillOnce(Return(nEvsModeSwitch));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY, _))
            .WillOnce(Return(objBitrateListArray));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT, DEFAULT_BW_LIST))
            .WillOnce(Return(nBwList));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CMR_INT, DEFAULT_CMR))
            .WillOnce(Return(nCmr));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT,
                    DEFAULT_CH_AW_RECV))
            .WillOnce(Return(nChAwRecv));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL,
                    IMS_FALSE))
            .WillOnce(Return(bShowAmrModeSet));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT,
                    CodecAudioConfig::DEFAULT_MODESET_AMR_WB))
            .WillOnce(Return(nAmrModeSetList));
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::
                            KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY,
                    CodecAudioConfig::DEFAULT_MODESET_AMR_WB))
            .WillOnce(Return(nDefaultAmrModeSetList));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY))
            .WillOnce(Return(nModeChangeCapability));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD))
            .WillOnce(Return(nModeChangePeriod));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT,
                    CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR))
            .WillOnce(Return(nModeChangeNeighbor));

    // Call Create
    IMS_BOOL bResult = m_pCodecEvsConfig->Create(m_pMockICarrierConfig.get());
    ASSERT_EQ(bResult, IMS_TRUE);

    // Verify values
    EXPECT_EQ(m_pCodecEvsConfig->GetChannel(), nChannel);
    EXPECT_EQ(m_pCodecEvsConfig->GetShowDtx(), bShowDtx);
    EXPECT_EQ(m_pCodecEvsConfig->GetDtx(), bDtx);
    EXPECT_EQ(m_pCodecEvsConfig->GetDtxRecv(), bDtxRecv);
    EXPECT_EQ(m_pCodecEvsConfig->GetHfOnly(), nHfOnly);
    EXPECT_EQ(m_pCodecEvsConfig->GetEvsModeSwitch(), nEvsModeSwitch);
    EXPECT_EQ(m_pCodecEvsConfig->GetBrList(), nExpectedBrList);
    EXPECT_EQ(m_pCodecEvsConfig->GetBwList(), nExpectedBwList);
    EXPECT_EQ(m_pCodecEvsConfig->GetCmr(), nCmr);
    EXPECT_EQ(m_pCodecEvsConfig->GetChAwareRecv(), nChAwRecv);
    EXPECT_EQ(m_pCodecEvsConfig->GetShowAmrModeSet(), bShowAmrModeSet);
    EXPECT_EQ(m_pCodecEvsConfig->GetAmrModeSetList(), nAmrModeSetList);
    EXPECT_EQ(m_pCodecEvsConfig->GetDefaultAmrModeSetList(), nDefaultAmrModeSetList);
    EXPECT_EQ(m_pCodecEvsConfig->GetModeChangeCapability(), nModeChangeCapability);
    EXPECT_EQ(m_pCodecEvsConfig->GetModeChangePeriod(), nModeChangePeriod);
    EXPECT_EQ(m_pCodecEvsConfig->GetModeChangeNeighbor(), nModeChangeNeighbor);
}

TEST_F(CodecEvsConfigTest, BitrateConversion)
{
    m_pCodecEvsConfig =
            std::make_unique<CodecEvsConfig>(ImsCodec::AUDIO_EVS, m_nDefaultPayloadType);
    ImsVector<IMS_SINT32> objBitrateRange;
    objBitrateRange.Push(CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_5_9_KBPS);   // 0
    objBitrateRange.Push(CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_24_4_KBPS);  // 6

    // Setup mock calls
    EXPECT_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillOnce(::testing::Return(m_pMockEvsBundle.get()));
    EXPECT_CALL(*m_pMockEvsBundle,
            GetBundle(AString(std::to_string(m_nDefaultPayloadType).c_str()).GetStr()))
            .WillOnce(::testing::Return(m_pMockEvsSubBundle.get()));
    EXPECT_CALL(*m_pMockEvsSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY, _))
            .WillOnce(Return(objBitrateRange));

    m_pCodecEvsConfig->Create(m_pMockICarrierConfig.get());

    IMS_UINT32 nExpectedBrList = 0;
    for (int i = CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_5_9_KBPS;
            i <= CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_24_4_KBPS; ++i)
    {
        nExpectedBrList |= (1 << i);
    }
    EXPECT_EQ(m_pCodecEvsConfig->GetBrList(), nExpectedBrList);
    EXPECT_EQ(m_pCodecEvsConfig->GetBr(), CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_24_4_KBPS);
}
