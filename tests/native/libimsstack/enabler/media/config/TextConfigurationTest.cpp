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
#include <config/TextConfiguration.h>

using ::testing::Return;

static const IMS_SINT32 DEFAULT_PAYLOAD_T140 = TextConfiguration::DEFAULT_PAYLOAD_T140;
static const IMS_SINT32 DEFAULT_PAYLOAD_RED = TextConfiguration::DEFAULT_PAYLOAD_RED;
static const IMS_BOOL DEFAULT_EMPTY_REDUNDANT = TextConfiguration::DEFAULT_EMPTY_REDUNDANT;

class TextConfigurationTest : public ::testing::Test {

public:
    TextConfiguration* pConfig;
protected:
    virtual void SetUp() override {
        pConfig = new TextConfiguration();
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(TextConfigurationTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetT140PayloadType(), DEFAULT_PAYLOAD_T140);
    EXPECT_EQ(pConfig->GetRedPayloadType(), DEFAULT_PAYLOAD_RED);
    EXPECT_EQ(pConfig->IsTextCodecEmptyRedundantEnabled(), DEFAULT_EMPTY_REDUNDANT);
}
