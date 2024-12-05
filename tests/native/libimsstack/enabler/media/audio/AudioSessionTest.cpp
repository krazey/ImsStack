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
    AudioSession* m_pAudioMediaSession;

protected:
    virtual void SetUp() override { m_pAudioMediaSession = new AudioSession(); }

    virtual void TearDown() override { delete m_pAudioMediaSession; }
};

TEST_F(AudioSessionTest, testSetGetInactivityTimer)
{
    IMS_UINT32 INACTIVITY_TIME = 12000;

    m_pAudioMediaSession->SetNetworkToneTimer(INACTIVITY_TIME);
    EXPECT_EQ(m_pAudioMediaSession->GetInactivityTimer(NETWORK_TONE_INACTIVITY), INACTIVITY_TIME);
    m_pAudioMediaSession->SetNetworkToneTimer(0);
}

TEST_F(AudioSessionTest, testSetServiceType)
{
    m_pAudioMediaSession->SetServiceType(MEDIA_SERVICE_DEFAULT);
    EXPECT_EQ(m_pAudioMediaSession->GetServiceType(), MEDIA_SERVICE_DEFAULT);

    m_pAudioMediaSession->SetServiceType(MEDIA_SERVICE_EMERGENCY);
    EXPECT_EQ(m_pAudioMediaSession->GetServiceType(), MEDIA_SERVICE_EMERGENCY);
}
