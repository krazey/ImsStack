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

#include "ServiceConfig.h"
#include "config/CodecConfig.h"

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_SINT32 DEFAULT_CODEC = ImsCodec::AUDIO_AMR_WB;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 99;

class CodecConfigTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(CodecConfigTest, GetConfigDefault)
{
    CodecConfig* m_pConfig = new CodecConfig(DEFAULT_CODEC, DEFAULT_PAYLOAD_NUM);
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(piCc));
    EXPECT_EQ(m_pConfig->GetCodec(), DEFAULT_CODEC);
    EXPECT_EQ(m_pConfig->GetPayloadType(), DEFAULT_PAYLOAD_NUM);

    delete m_pConfig;
}

TEST_F(CodecConfigTest, GetConfigTest)
{
    CodecConfig* m_pConfigNew = new CodecConfig(ImsCodec::AUDIO_AMR, 105);
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfigNew->Create(piCc));
    EXPECT_EQ(m_pConfigNew->GetCodec(), ImsCodec::AUDIO_AMR);
    EXPECT_EQ(m_pConfigNew->GetPayloadType(), 105);

    delete m_pConfigNew;
}
