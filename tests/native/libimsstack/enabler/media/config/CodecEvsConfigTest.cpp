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
#include "config/CodecEvsConfig.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_CHANNEL = CodecEvsConfig::DEFAULT_CHANNEL;
static const IMS_BOOL DEFAULT_DTX = CodecEvsConfig::DEFAULT_DTX;
static const IMS_BOOL DEFAULT_DTX_RECV = CodecEvsConfig::DEFAULT_DTX_RECV;
static const IMS_SINT32 DEFAULT_HF_ONLY = CodecEvsConfig::DEFAULT_HF_ONLY;
static const IMS_SINT32 DEFAULT_EVS_MODESWITCH = CodecEvsConfig::DEFAULT_EVS_MODESWITCH;
static const IMS_SINT32 DEFAULT_BR_LIST = CodecEvsConfig::DEFAULT_BR_LIST;
static const IMS_SINT32 DEFAULT_BW_LIST = CodecEvsConfig::DEFAULT_BW_LIST;
static const IMS_SINT32 DEFAULT_CMR = CodecEvsConfig::DEFAULT_CMR;
static const IMS_SINT32 DEFAULT_CH_AW_RECV = CodecEvsConfig::DEFAULT_CH_AW_RECV;
static const IMS_SINT32 DEFAULT_AMRWB_IO_MODESET = CodecEvsConfig::DEFAULT_AMRWB_IO_MODESET;

class CodecEvsConfigTest : public ::testing::Test {
public:
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
        return piCc->GetInt(pszKey);
    }
    IMS_BOOL GetBool(IN const IMS_CHAR* pszKey)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
        return piCc->GetBoolean(pszKey, IMS_TRUE);
    }
};

TEST_F(CodecEvsConfigTest, GetConfigDefault)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);

    EXPECT_EQ(pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(pConfig->GetShowDtx(), IMS_FALSE);
    EXPECT_EQ(pConfig->GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(pConfig->GetDtxRecv(), DEFAULT_DTX_RECV);
    EXPECT_EQ(pConfig->GetHfOnly(), DEFAULT_HF_ONLY);
    EXPECT_EQ(pConfig->GetEvsModeSwitch(), DEFAULT_EVS_MODESWITCH);
    EXPECT_EQ(pConfig->GetBrList(), DEFAULT_BR_LIST);
    EXPECT_EQ(pConfig->GetBwList(), DEFAULT_BW_LIST);
    EXPECT_EQ(pConfig->GetCmr(), DEFAULT_CMR);
    EXPECT_EQ(pConfig->GetChAwareRecv(), DEFAULT_CH_AW_RECV);
    EXPECT_EQ(pConfig->GetShowAmrwbIoModeSet(), IMS_FALSE);
    EXPECT_EQ(pConfig->GetAmrWbIoModeSetList(), DEFAULT_AMRWB_IO_MODESET);
    delete pConfig;
}

// TODO - need to check Bundel configuration later
TEST_F(CodecEvsConfigTest, GetEvsChannelId)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsChannelId = 2;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_CHANNELS_INT, -1))
            .WillByDefault(Return(nEvsChannelId));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetChannel(), nEvsChannelId);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetShowEvsDtx)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_BOOL bShowEvsDtx = IMS_TRUE;

    ON_CALL(*pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_DTX_BOOL, IMS_FALSE))
            .WillByDefault(Return(bShowEvsDtx));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetShowDtx(), bShowEvsDtx);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetEvsDtx)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_BOOL bEvsDtx = IMS_TRUE;

    ON_CALL(*pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_DTX_BOOL, IMS_FALSE))
            .WillByDefault(Return(bEvsDtx));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetDtx(), bEvsDtx);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetEvsDtxRecv)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_BOOL bEvsDtxRecv = IMS_TRUE;

    ON_CALL(*pMockICarrierConfig,
            GetBoolean(
                    CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL, IMS_FALSE))
            .WillByDefault(Return(bEvsDtxRecv));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetDtxRecv(), bEvsDtxRecv);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetEvsHfOnly)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsHFOnly = 1;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT, -1))
            .WillByDefault(Return(nEvsHFOnly));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetHfOnly(), nEvsHFOnly);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetEvsModeSwitch)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsModeSwitch = 1;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT, -1))
            .WillByDefault(Return(nEvsModeSwitch));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetEvsModeSwitch(), nEvsModeSwitch);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetBwList)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsBWList = 4;
    IMS_SINT32 nEvsBWListConverted = 3;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT, -1))
            .WillByDefault(Return(nEvsBWList));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetBwList(), nEvsBWListConverted);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetCmr)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsCmr = 5;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_CMR_INT, -1))
            .WillByDefault(Return(nEvsCmr));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetCmr(), nEvsCmr);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetChAwareRecv)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsChAwareRecv = 5;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT, -1))
            .WillByDefault(Return(nEvsChAwareRecv));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetChAwareRecv(), nEvsChAwareRecv);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetAmrWbIoModeSetList)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_SINT32 nEvsAmrIoMoseSetList = 5;

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_AMRWB_IO_MODE_SET_INT, -1))
            .WillByDefault(Return(nEvsAmrIoMoseSetList));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetAmrWbIoModeSetList(), nEvsAmrIoMoseSetList);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetConfigEvsBitrateList)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    ImsVector<IMS_SINT32> objBitrateList;
    objBitrateList.Push(0);
    objBitrateList.Push(4);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::Assets::KEY_ASSET_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY))
            .WillByDefault(Return(objBitrateList));

    pConfig->Create(pMockICarrierConfig, 0);

    EXPECT_EQ(pConfig->GetBrList(), 31);
    EXPECT_EQ(pConfig->GetBr(), 4);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetShowEvsAmrWbIoModeSet)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    IMS_BOOL bShowEvsAmrWbIoModeSet = IMS_TRUE;

    ON_CALL(*pMockICarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL,
                    IMS_FALSE))
            .WillByDefault(Return(bShowEvsAmrWbIoModeSet));

    EXPECT_TRUE(pConfig->Create(pMockICarrierConfig, 0));
    EXPECT_EQ(pConfig->GetShowAmrwbIoModeSet(), bShowEvsAmrWbIoModeSet);

    delete pMockICarrierConfig;
    delete pConfig;
}

TEST_F(CodecEvsConfigTest, GetConfigEvsAmrWbIoModeSet)
{
    CodecEvsConfig* pConfig = new CodecEvsConfig(ImsCodec::AUDIO_EVS, 125);
    IMS_UINT32 nMockAmrWbIoModeSetList = 7;
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::Assets::KEY_ASSET_EVS_AMRWB_IO_MODE_SET_INT, -1))
            .WillByDefault(Return(nMockAmrWbIoModeSetList));

    pConfig->Create(pMockICarrierConfig, 0);

    EXPECT_EQ(pConfig->GetAmrWbIoModeSetList(), 7);
    EXPECT_EQ(pConfig->GetAmrWbIoModeSet(), 2);

    delete pMockICarrierConfig;
    delete pConfig;
}
