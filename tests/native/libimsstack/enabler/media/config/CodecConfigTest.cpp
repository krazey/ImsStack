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

#include "ServiceConfig.h"
#include "config/CodecConfig.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_CODEC = ImsCodec::AUDIO_AMR_WB;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 99;

class CodecConfigTest : public ::testing::Test
{
public:
    CodecConfig* m_pConfig;

protected:
    virtual void SetUp() override
    {
        m_pConfig = new CodecConfig(DEFAULT_CODEC, DEFAULT_PAYLOAD_NUM);
    }
    virtual void TearDown() override
    {
        if (m_pConfig)
        {
            delete m_pConfig;
        }
    }
};

TEST_F(CodecConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetCodec(), DEFAULT_CODEC);
    EXPECT_EQ(m_pConfig->GetPayloadType(), DEFAULT_PAYLOAD_NUM);
}

TEST_F(CodecConfigTest, GetConfigTest)
{
    CodecConfig* m_pConfigNew = new CodecConfig(ImsCodec::AUDIO_AMR, 105);

    EXPECT_EQ(m_pConfigNew->GetCodec(), ImsCodec::AUDIO_AMR);
    EXPECT_EQ(m_pConfigNew->GetPayloadType(), 105);

    delete m_pConfigNew;
}
