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
#include <IJniMedia.h>
#include <audio/AudioSession.h>

class AudioSessionTest : public ::testing::Test
{
public:
    std::unique_ptr<AudioSession> m_pSession;

protected:
    virtual void SetUp() override
    {
        m_pSession = std::unique_ptr<AudioSession>(new AudioSession());
    }

    virtual void TearDown() override {}
};

TEST_F(AudioSessionTest, testSetGetInactivityTimer)
{
    IMS_UINT32 INACTIVITY_TIME = 12000;

    m_pSession->SetNetworkToneTimer(INACTIVITY_TIME);
    EXPECT_EQ(m_pSession->GetInactivityTimer(NETWORK_TONE_INACTIVITY), INACTIVITY_TIME);
    m_pSession->SetNetworkToneTimer(0);
}

TEST_F(AudioSessionTest, testSetDirection)
{
    m_pSession->SetDirection(MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_INVALID);
    m_pSession->SetDirection(MEDIA_DIRECTION_SEND);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_SEND);
    m_pSession->SetDirection(MEDIA_DIRECTION_RECEIVE);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_RECEIVE);
    m_pSession->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    m_pSession->SetDirection(MEDIA_DIRECTION_INACTIVE);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(AudioSessionTest, testSetNegoId)
{
    const IMS_UINTP nNegoId = 0x1234;
    m_pSession->SetNegoId(nNegoId);
    EXPECT_TRUE(m_pSession->IsSameNegoId(nNegoId));
    EXPECT_FALSE(m_pSession->IsSameNegoId(0x3456));
}

TEST_F(AudioSessionTest, testAccessNetwork)
{
    const IMS_UINTP nNetwork = 0x11;
    EXPECT_TRUE(m_pSession->SetAccessNetwork(nNetwork));
    ASSERT_TRUE(m_pSession->GetRtpConfig() != IMS_NULL);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAccessNetwork(), nNetwork);
}

TEST_F(AudioSessionTest, testSetAnbrMode)
{
    AnbrMode objAnbrMode;
    m_pSession->SetAnbrMode(objAnbrMode);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAnbrMode(), objAnbrMode);
}
