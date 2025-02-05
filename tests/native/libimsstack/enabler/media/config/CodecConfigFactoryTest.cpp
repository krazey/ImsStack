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
#include "MockICarrierConfig.h"

#include "config/CodecConfigFactory.h"

using ::testing::Return;
using ::testing::TypedEq;

class CodecConfigFactoryTest : public ::testing::Test
{
public:
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pMediaBundle;
    MockICarrierConfig* m_pMediaSubBundle;
    AString m_strPayloadTypeNumber;

protected:
    virtual void SetUp() override
    {
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pMediaBundle = new MockICarrierConfig();
        m_pMediaSubBundle = new MockICarrierConfig();
    }

    virtual void TearDown() override
    {
        delete m_pMockICarrierConfig;
        delete m_pMediaBundle;
        delete m_pMediaSubBundle;

        m_pMockICarrierConfig = IMS_NULL;
        m_pMediaBundle = IMS_NULL;
        m_pMediaSubBundle = IMS_NULL;
    }
};

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

TEST_F(CodecConfigFactoryTest, CreateAudioPayloadConfigTest)
{
    IMS_SINT32 nPayloadNumber = 99;
    m_strPayloadTypeNumber.SetNumber(nPayloadNumber);

    ON_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillByDefault(Return(m_pMediaBundle));
    ON_CALL(*m_pMediaBundle, GetBundle(IsSameKey(m_strPayloadTypeNumber.GetStr())))
            .WillByDefault(Return(m_pMediaSubBundle));

    CodecConfig* pConfig = CodecConfigFactory::CreateAudioPayloadConfig(
            m_pMockICarrierConfig, ImsCodec::AUDIO_AMR_WB, nPayloadNumber);

    EXPECT_TRUE(pConfig != nullptr);
    EXPECT_THAT(pConfig, TypedEq<CodecConfig*>(pConfig));
}

TEST_F(CodecConfigFactoryTest, CreateVideoPayloadConfigTest)
{
    IMS_SINT32 nPayloadNumber = 100;
    m_strPayloadTypeNumber.SetNumber(nPayloadNumber);

    ON_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
            .WillByDefault(Return(m_pMediaBundle));
    ON_CALL(*m_pMediaBundle, GetBundle(IsSameKey(m_strPayloadTypeNumber.GetStr())))
            .WillByDefault(Return(m_pMediaSubBundle));

    CodecConfig* pConfig = CodecConfigFactory::CreateVideoPayloadConfig(
            m_pMockICarrierConfig, ImsCodec::VIDEO_AVC, nPayloadNumber);

    EXPECT_TRUE(pConfig != nullptr);
    EXPECT_THAT(pConfig, TypedEq<CodecConfig*>(pConfig));
}

TEST_F(CodecConfigFactoryTest, CreateTextPayloadConfigTest)
{
    IMS_SINT32 nPayloadNumber = 110;

    ON_CALL(*m_pMockICarrierConfig,
            GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
            .WillByDefault(Return(m_pMediaBundle));

    CodecConfig* pConfig = CodecConfigFactory::CreateTextPayloadConfig(
            m_pMockICarrierConfig, ImsCodec::TEXT_T140, nPayloadNumber);

    EXPECT_TRUE(pConfig != nullptr);
    EXPECT_THAT(pConfig, TypedEq<CodecConfig*>(pConfig));
}
