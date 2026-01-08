/*
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

#include <MediaResourceManager.h>

#include <config/MockMediaConfiguration.h>

const IMS_UINT32 RTP_PORT_START = 10000;
const IMS_UINT32 RTP_PORT_END = 20000;
const AString LOCAL_IP = "127.0.0.1";

using ::testing::Return;

class MediaResourceManagerTest : public ::testing::Test
{
public:
    std::shared_ptr<MediaResourceManager> m_pManager;
    std::shared_ptr<MockMediaConfiguration> m_pMediaConfiguration;

protected:
    virtual void SetUp() override
    {
        m_pManager = std::make_shared<MediaResourceManager>();
        m_pMediaConfiguration = std::make_shared<MockMediaConfiguration>(MEDIA_TYPE_AUDIO);
    }

    virtual void TearDown() override {}
};

TEST_F(MediaResourceManagerTest, testAcquireReleasePort)
{
    ON_CALL(*m_pMediaConfiguration, GetPortRtp()).WillByDefault(Return(RTP_PORT_START));
    ON_CALL(*m_pMediaConfiguration, GetPortRtpEnd()).WillByDefault(Return(RTP_PORT_END));

    IMS_UINT32 nPort = m_pManager->AcquireRtpPort(m_pMediaConfiguration.get());
    EXPECT_TRUE((nPort >= RTP_PORT_START) && (nPort <= RTP_PORT_END));

    EXPECT_EQ(m_pManager->ReleaseRtpPort(nPort), IMS_TRUE);
    EXPECT_EQ(m_pManager->ReleaseRtpPort(30000), IMS_FALSE);
}
