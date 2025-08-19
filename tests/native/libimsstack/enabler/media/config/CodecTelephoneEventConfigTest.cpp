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

#include "ICarrierConfig.h"
#include "CarrierConfig.h"
#include "ServiceConfig.h"
#include "config/CodecTelephoneEventConfig.h"

static const IMS_SINT32 DEFAULT_SLOT_ID = 0;
static const IMS_CHAR* DEFAULT_EVENT = "0-15";
static const IMS_SINT32 DEFAULT_REDUNDANT_COUNT_TEST = 3;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_TEST = 8000;
static const IMS_SINT32 DEFAULT_SAMPLING_RATE_WB_TEST = 16000;

class CodecTelephoneEventConfigTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(CodecTelephoneEventConfigTest, GetConfigAMRWB)
{
    CodecTelephoneEventConfig* m_pConfig =
            new CodecTelephoneEventConfig(ImsCodec::AUDIO_TELEPHONE_EVENT_WB, 99);
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(piCc));

    EXPECT_EQ(m_pConfig->GetEvents(), DEFAULT_EVENT);
    EXPECT_EQ(m_pConfig->GetRedundancyCount(), DEFAULT_REDUNDANT_COUNT_TEST);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_WB_TEST);

    delete m_pConfig;
}

TEST_F(CodecTelephoneEventConfigTest, GetConfigAMR)
{
    CodecTelephoneEventConfig* m_pConfig =
            new CodecTelephoneEventConfig(ImsCodec::AUDIO_TELEPHONE_EVENT, 100);
    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID);

    EXPECT_TRUE(m_pConfig->Create(piCc));
    EXPECT_EQ(m_pConfig->GetEvents(), DEFAULT_EVENT);
    EXPECT_EQ(m_pConfig->GetRedundancyCount(), DEFAULT_REDUNDANT_COUNT_TEST);
    EXPECT_EQ(m_pConfig->GetSamplingRate(), DEFAULT_SAMPLING_RATE_TEST);

    delete m_pConfig;
}
