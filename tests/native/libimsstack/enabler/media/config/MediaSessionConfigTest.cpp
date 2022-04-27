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
#include <config/MediaSessionConfig.h>

using ::testing::Return;

static const MEDIA_SERVICE_TYPE DEFAULT_SERVICE_TYPE = MEDIA_SERVICE_DEFAULT;
static const IMS_BOOL DEFAULT_SESSION_LEVEL_BW = MediaSessionConfig::DEFAULT_SESSION_LEVEL_BW;
static const IMS_BOOL DEFAULT_ANBR_CAPABILITY = MediaSessionConfig::DEFAULT_ANBR_CAPABILITY;
static const IMS_BOOL DEFAULT_SUPPORT_MULTICONFIG = MediaSessionConfig::DEFAULT_SUPPORT_MULTICONFIG;

class MediaSessionConfigTest : public ::testing::Test {

public:
    MediaSessionConfig* pConfig;
protected:
    virtual void SetUp() override {
        pConfig = new MediaSessionConfig();
    }
    virtual void TearDown() override {
        delete pConfig;
    }
};

TEST_F(MediaSessionConfigTest, GET_DEFAULT) {
    EXPECT_EQ(pConfig->GetServiceType(), DEFAULT_SERVICE_TYPE);
    EXPECT_EQ(pConfig->IsSessionLevelBandwidth(), DEFAULT_SESSION_LEVEL_BW);
    EXPECT_EQ(pConfig->IsAnbrSupported(), DEFAULT_ANBR_CAPABILITY);
    EXPECT_EQ(pConfig->IsSupportMultiConfigInEarlySession(), DEFAULT_SUPPORT_MULTICONFIG);
}
