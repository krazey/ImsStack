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
#include <ServiceConfig.h>
#include <config/CodecAmrConfig.h>
using ::testing::Return;

static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::AUDIO_AMR;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;

static const IMS_SINT32 DEFAULT_CHANNEL = CodecAmrConfig::DEFAULT_CHANNEL;
static const IMS_SINT32 DEFAULT_OCTET_ALIGN = CodecAmrConfig::DEFAULT_OCTET_ALIGN;
static const IMS_SINT32 DEFAULT_MODESET_AMR_WB = CodecAmrConfig::DEFAULT_MODESET_AMR_WB;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMRWB = CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMRWB;

//static const IMS_SINT32 DEFAULT_SLOT_ID = 0;

class CodecAmrConfigTest : public ::testing::Test {

public :
    CodecAmrConfig* pConfig;

protected:
    virtual void SetUp() override {
        pConfig = new CodecAmrConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(CodecAmrConfigTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetChannel(), DEFAULT_CHANNEL);
    //EXPECT_EQ(pConfig->GetModeSet(), );
    EXPECT_EQ(pConfig->GetModeSetList(), DEFAULT_MODESET_AMR_WB);
    EXPECT_EQ(pConfig->GetOctetAlign(), DEFAULT_OCTET_ALIGN);
    EXPECT_EQ(pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_AMRWB);
}
