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

#include "config/CodecConfigFactory.h"

using ::testing::TypedEq;

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;

class CodecConfigFactoryTest : public ::testing::Test
{
public:
    ICarrierConfig* m_piCc;

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(CodecConfigFactoryTest, CreateAudioPayloadConfigTest)
{
    m_piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    CodecConfig* m_pConfig =
            CodecConfigFactory::CreateAudioPayloadConfig(m_piCc, ImsCodec::AUDIO_AMR_WB, 99, 0);

    EXPECT_TRUE(m_pConfig != nullptr);
    EXPECT_THAT(m_pConfig, TypedEq<CodecConfig*>(m_pConfig));

    m_pConfig = CodecConfigFactory::CreateVideoPayloadConfig(m_piCc, ImsCodec::VIDEO_AVC, 100, 0);

    EXPECT_TRUE(m_pConfig != nullptr);
    EXPECT_THAT(m_pConfig, TypedEq<CodecConfig*>(m_pConfig));

    m_pConfig = CodecConfigFactory::CreateTextPayloadConfig(m_piCc, ImsCodec::TEXT_T140, 105, 0);

    EXPECT_TRUE(m_pConfig != nullptr);
    EXPECT_THAT(m_pConfig, TypedEq<CodecConfig*>(m_pConfig));
}
