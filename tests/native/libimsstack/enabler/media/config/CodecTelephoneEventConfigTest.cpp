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

#include "config/CodecTelephoneEventConfig.h"

using ::testing::Return;

static const IMS_SINT32 DEFAULT_TYPE = ImsCodec::AUDIO_TELEPHONE_EVENT_WB;
static const IMS_SINT32 DEFAULT_PAYLOAD_NUM = 3;
static const IMS_CHAR* DEFAULT_EVENT = "0-15";
static const IMS_SINT32 DEFAULT_REDUNDANT_COUNT_TEST =
        CodecTelephoneEventConfig::DEFAULT_REDUNDANT_COUNT;
// static const IMS_SINT32 DEFAULT_SAMPLING_RATE_TEST =
// CodecTelephoneEventConfig::DEFAULT_SAMPLING_RATE;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_WB_TEST =
        CodecTelephoneEventConfig::DEFAULT_SAMPLING_RATE_WB;

class CodecTelephoneEventConfigTest : public ::testing::Test {

public :
    CodecTelephoneEventConfig* m_pConfig;

protected:
    virtual void SetUp() override {
        m_pConfig = new CodecTelephoneEventConfig(DEFAULT_TYPE, DEFAULT_PAYLOAD_NUM);
    }
    virtual void TearDown() override {
        if (m_pConfig)
        {
            delete m_pConfig;
        }
    }
};

TEST_F(CodecTelephoneEventConfigTest, GetConfigDefault)
{
    EXPECT_EQ(m_pConfig->GetEvents(), DEFAULT_EVENT);
    EXPECT_EQ(m_pConfig->GetRedundancyCount(), DEFAULT_REDUNDANT_COUNT_TEST);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_WB_TEST);
}

/*TEST_F(CodecTelephoneEventConfigTest, GetConfigAMR)
{
    m_pConfig = new CodecTelephoneEventConfig(ImsCodec::AUDIO_TELEPHONE_EVENT, DEFAULT_PAYLOAD_NUM);

    EXPECT_EQ(m_pConfig->GetEvents(), DEFAULT_EVENT);
    EXPECT_EQ(m_pConfig->GetRedundancyCount(), DEFAULT_REDUNDANT_COUNT_TEST);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_TEST);

    delete m_pConfig;
}*/