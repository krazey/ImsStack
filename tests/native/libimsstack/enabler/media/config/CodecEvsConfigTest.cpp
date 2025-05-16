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
#include "config/CodecAudioConfig.h"
#include "config/CodecEvsConfig.h"

using ::testing::_;
using ::testing::Return;

static const IMS_SINT32 DEFAULT_CHANNEL = CodecEvsConfig::DEFAULT_CHANNEL;
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
static const IMS_BOOL DEFAULT_DTX = CodecAudioConfig::DEFAULT_DTX;

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class CodecEvsConfigTest : public ::testing::Test {
public:
    CodecEvsConfig* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pAudioBundle;
    MockICarrierConfig* m_pAudioSubBundle;
    IMS_SINT32 m_nEvsPayloadTypeNumber = 125;
    AString m_strPayloadTypeNumber;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, m_nEvsPayloadTypeNumber);
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pAudioBundle = new MockICarrierConfig();
        m_pAudioSubBundle = new MockICarrierConfig();
    }

    virtual void TearDown() override
    {
        delete m_pConfig;
        delete m_pMockICarrierConfig;
        delete m_pAudioBundle;
        delete m_pAudioSubBundle;

        m_pConfig = IMS_NULL;
        m_pMockICarrierConfig = IMS_NULL;
        m_pAudioBundle = IMS_NULL;
        m_pAudioSubBundle = IMS_NULL;
    }

    inline void GetReadyToCreate()
    {
        m_strPayloadTypeNumber.SetNumber(m_nEvsPayloadTypeNumber);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAudioBundle));
        ON_CALL(*m_pAudioBundle, GetBundle(IsSameKey(m_strPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAudioSubBundle));
    }
};

TEST_F(CodecEvsConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetShowDtx(), IMS_FALSE);
    EXPECT_EQ(m_pConfig->GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(m_pConfig->GetDtxRecv(), DEFAULT_DTX_RECV);
    EXPECT_EQ(m_pConfig->GetHfOnly(), DEFAULT_HF_ONLY);
    EXPECT_EQ(m_pConfig->GetEvsModeSwitch(), DEFAULT_EVS_MODESWITCH);
    EXPECT_EQ(m_pConfig->GetBrList(), DEFAULT_BR_LIST);
    EXPECT_EQ(m_pConfig->GetBwList(), DEFAULT_BW_LIST);
    EXPECT_EQ(m_pConfig->GetCmr(), DEFAULT_CMR);
    EXPECT_EQ(m_pConfig->GetChAwareRecv(), DEFAULT_CH_AW_RECV);
    EXPECT_EQ(m_pConfig->GetShowAmrModeSet(), IMS_FALSE);
    EXPECT_EQ(m_pConfig->GetAmrModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(m_pConfig->GetModeChangeCapability(), DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(m_pConfig->GetModeChangePeriod(), DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(m_pConfig->GetModeChangeNeighbor(), DEFAULT_MODECHANGE_NEIGHBOR);
}

TEST_F(CodecEvsConfigTest, GetEvsChannelId)
{
    IMS_SINT32 nEvsChannelId = 2;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT, DEFAULT_CHANNEL))
            .WillByDefault(Return(nEvsChannelId));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetChannel(), nEvsChannelId);
}

TEST_F(CodecEvsConfigTest, GetShowEvsDtx)
{
    IMS_BOOL bShowEvsDtx = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_DTX_BOOL, IMS_FALSE))
            .WillByDefault(Return(bShowEvsDtx));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetShowDtx(), bShowEvsDtx);
}

TEST_F(CodecEvsConfigTest, GetEvsDtx)
{
    IMS_BOOL bEvsDtx = IMS_TRUE;

    ON_CALL(*m_pAudioSubBundle,
            GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL, DEFAULT_DTX))
            .WillByDefault(Return(bEvsDtx));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetDtx(), bEvsDtx);
}

TEST_F(CodecEvsConfigTest, GetEvsDtxRecv)
{
    IMS_BOOL bEvsDtxRecv = IMS_TRUE;

    ON_CALL(*m_pAudioSubBundle,
            GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL,
                    DEFAULT_DTX_RECV))
            .WillByDefault(Return(bEvsDtxRecv));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetDtxRecv(), bEvsDtxRecv);
}

TEST_F(CodecEvsConfigTest, GetEvsHfOnly)
{
    IMS_SINT32 nEvsHFOnly = 1;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT, -1))
            .WillByDefault(Return(nEvsHFOnly));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetHfOnly(), nEvsHFOnly);
}

TEST_F(CodecEvsConfigTest, GetEvsModeSwitch)
{
    IMS_SINT32 nEvsModeSwitch = 1;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT, -1))
            .WillByDefault(Return(nEvsModeSwitch));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetEvsModeSwitch(), nEvsModeSwitch);
}

TEST_F(CodecEvsConfigTest, GetBwList)
{
    IMS_SINT32 nEvsBWList = 4;
    IMS_SINT32 nEvsBWListConverted = 3;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT, DEFAULT_BW_LIST))
            .WillByDefault(Return(nEvsBWList));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetBwList(), nEvsBWListConverted);
}

TEST_F(CodecEvsConfigTest, GetCmr)
{
    IMS_SINT32 nEvsCmr = 5;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CMR_INT, DEFAULT_CMR))
            .WillByDefault(Return(nEvsCmr));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetCmr(), nEvsCmr);
}

TEST_F(CodecEvsConfigTest, GetChAwareRecv)
{
    IMS_SINT32 nEvsChAwareRecv = 5;

    ON_CALL(*m_pAudioSubBundle,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT,
                    DEFAULT_CH_AW_RECV))
            .WillByDefault(Return(nEvsChAwareRecv));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetChAwareRecv(), nEvsChAwareRecv);
}

TEST_F(CodecEvsConfigTest, GetAmrModeSetList)
{
    IMS_SINT32 nEvsAmrIoMoseSetList = 7;

    ON_CALL(*m_pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT, DEFAULT_MODESET_AMR_WB))
            .WillByDefault(Return(nEvsAmrIoMoseSetList));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetAmrModeSetList(), nEvsAmrIoMoseSetList);
    EXPECT_EQ(m_pConfig->GetAmrModeSet(), 2);
}

TEST_F(CodecEvsConfigTest, GetConfigEvsBitrateList)
{
    ImsVector<IMS_SINT32> objBitrateList;
    objBitrateList.Clear();
    objBitrateList.Push(0);
    objBitrateList.Push(4);

    ON_CALL(*m_pAudioSubBundle,
            GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY, _))
            .WillByDefault(Return(objBitrateList));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetBrList(), 31);
    EXPECT_EQ(m_pConfig->GetBr(), 4);
}

TEST_F(CodecEvsConfigTest, GetShowEvsAmrWbIoModeSet)
{
    IMS_BOOL bShowEvsAmrWbIoModeSet = IMS_TRUE;

    ON_CALL(*m_pMockICarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL,
                    IMS_FALSE))
            .WillByDefault(Return(bShowEvsAmrWbIoModeSet));

    GetReadyToCreate();
    EXPECT_TRUE(m_pConfig->Create(m_pMockICarrierConfig));

    EXPECT_EQ(m_pConfig->GetShowAmrModeSet(), bShowEvsAmrWbIoModeSet);
}
