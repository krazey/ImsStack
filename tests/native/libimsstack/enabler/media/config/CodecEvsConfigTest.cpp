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
#include <config/CodecEvsConfig.h>
using ::testing::Return;

static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::AUDIO_EVS;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 10;

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

public :
    CodecEvsConfig* pConfig;

protected:
    virtual void SetUp() override {
        pConfig = new CodecEvsConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(CodecEvsConfigTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetChannel(), DEFAULT_CHANNEL);
    EXPECT_EQ(pConfig->GetDtx(), DEFAULT_DTX);
    EXPECT_EQ(pConfig->GetDtxRecv(), DEFAULT_DTX_RECV);
    EXPECT_EQ(pConfig->GetHfOnly(), DEFAULT_HF_ONLY);
    EXPECT_EQ(pConfig->GetEvsModeSwitch(), DEFAULT_EVS_MODESWITCH);
    EXPECT_EQ(pConfig->GetBrList(), DEFAULT_BR_LIST);
    EXPECT_EQ(pConfig->GetBr(), 6);
    EXPECT_EQ(pConfig->GetBwList(), DEFAULT_BW_LIST);
    EXPECT_EQ(pConfig->GetBw(), 2);
    EXPECT_EQ(pConfig->GetCmr(), DEFAULT_CMR);
    EXPECT_EQ(pConfig->GetChAwareRecv(), DEFAULT_CH_AW_RECV);
    EXPECT_EQ(pConfig->GetModeSetList(),  DEFAULT_AMRWB_IO_MODESET);
}
