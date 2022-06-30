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
static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::AUDIO_EVS;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 10;
static const IMS_SINT32 DEFAULT_CHANNEL = CodecEvsConfig::DEFAULT_CHANNEL;
static const IMS_BOOL DEFAULT_DTX = CodecEvsConfig::DEFAULT_DTX;
static const IMS_BOOL DEFAULT_DTX_RECV = CodecEvsConfig::DEFAULT_DTX_RECV;
static const IMS_SINT32 DEFAULT_HF_ONLY = CodecEvsConfig::DEFAULT_HF_ONLY;
static const IMS_SINT32 DEFAULT_EVS_MODESWITCH = CodecEvsConfig::DEFAULT_EVS_MODESWITCH;
static const IMS_SINT32 DEFAULT_BR = CodecEvsConfig::DEFAULT_BR;
static const IMS_SINT32 DEFAULT_BR_LIST = CodecEvsConfig::DEFAULT_BR_LIST;
static const IMS_SINT32 DEFAULT_BW_LIST = CodecEvsConfig::DEFAULT_BW_LIST;
static const IMS_SINT32 DEFAULT_CMR = CodecEvsConfig::DEFAULT_CMR;
static const IMS_SINT32 DEFAULT_CH_AW_RECV = CodecEvsConfig::DEFAULT_CH_AW_RECV;
static const IMS_SINT32 DEFAULT_AMRWB_IO_MODESET = CodecEvsConfig::DEFAULT_AMRWB_IO_MODESET;

class CodecEvsConfigTest : public ::testing::Test {

public :
    CodecEvsConfig* m_pConfig;
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {
        m_pConfig = new CodecEvsConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
        m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);
    }
    virtual void TearDown() override {
        if (m_pConfig)
        {
            delete m_pConfig;
        }
    }
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) { return m_piCc->GetInt(pszKey); }
    IMS_BOOL GetBoolean(IN const IMS_CHAR* pszKey) { return m_piCc->GetBoolean(pszKey); }
    IMSVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey)
    {
        return m_piCc->GetIntArray(pszKey);
    }
};

TEST_F(CodecEvsConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(m_pConfig->GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(m_pConfig->GetDtxRecv(), DEFAULT_DTX_RECV);
    EXPECT_EQ(m_pConfig->GetHfOnly(), DEFAULT_HF_ONLY);
    EXPECT_EQ(m_pConfig->GetEvsModeSwitch(), DEFAULT_EVS_MODESWITCH);
    EXPECT_EQ(m_pConfig->GetBrList(), DEFAULT_BR_LIST);
    EXPECT_EQ(m_pConfig->GetBr(), DEFAULT_BR);
    EXPECT_EQ(m_pConfig->GetBwList(), DEFAULT_BW_LIST);
    EXPECT_EQ(m_pConfig->GetBw(), 2);
    EXPECT_EQ(m_pConfig->GetCmr(), DEFAULT_CMR);
    EXPECT_EQ(m_pConfig->GetChAwareRecv(), DEFAULT_CH_AW_RECV);
    EXPECT_EQ(m_pConfig->GetAmrWbIoModeSetList(), DEFAULT_AMRWB_IO_MODESET);
}

// TODO - need to check Bundel configuration later
/*TEST_F(CodecEvsConfigTest, GetConfigTest)
{
    m_pConfig->Create(m_piCc);

    EXPECT_EQ(m_pConfig->GetDtx(),
            GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL));
    EXPECT_EQ(m_pConfig->GetDtxRecv(),
            GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL));
    EXPECT_EQ(m_pConfig->GetHfOnly(),
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT));
    EXPECT_EQ(m_pConfig->GetEvsModeSwitch(),
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT));
    EXPECT_EQ(m_pConfig->GetBwList(), GetInt(
            CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT));
    // EXPECT_EQ(m_pConfig->GetBw(), CodecEvsConfig::EVS_BANDWIDTH_WB);
    // TODO - need to check later
    EXPECT_EQ(m_pConfig->GetCmr(),
GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CMR_INT));
    EXPECT_EQ(m_pConfig->GetChAwareRecv(),
GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT));
    EXPECT_EQ(m_pConfig->GetAmrWbIoModeSetList(),
GetInt(CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT));
}

TEST_F(CodecEvsConfigTest, GetConfigEvsBitrateList)
{
    IMSVector<IMS_SINT32> objBitrateList;
    objBitrateList.Push(0);
    objBitrateList.Push(4);

    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();
    ON_CALL(*pMockICarrierConfig,
            GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY))
            .WillByDefault(Return(objBitrateList));

    m_pConfig->Create(pMockICarrierConfig);

    EXPECT_EQ(m_pConfig->GetBrList(), 31);
    EXPECT_EQ(m_pConfig->GetBr(), 4);
}

TEST_F(CodecEvsConfigTest, GetConfigEvsAmrWbIoModeSet)
{
    IMS_UINT32 nMockAmrWbIoModeSetList = 7;
    MockICarrierConfig* pMockICarrierConfig = new MockICarrierConfig();

    ON_CALL(*pMockICarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT, -1))
            .WillByDefault(Return(nMockAmrWbIoModeSetList));

    m_pConfig->Create(pMockICarrierConfig);

    EXPECT_EQ(m_pConfig->GetAmrWbIoModeSetList(), 7);
    EXPECT_EQ(m_pConfig->GetAmrWbIoModeSet(), 2);
}
*/