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

#include "conf/ConfigFileBuffer.h"

namespace android
{
// clang-format off
static const IMS_CHAR CONFIG_DATA[] = {
    "[test.capabilities]\n"
    "stream_audio_count=3\n"
    "stream_audio_0=m=audio 0 RTP/AVP 96 100\n"
    "stream_audio_1=a=rtpmap:96 AMR/8000/1\n"
    "stream_audio_2=a=rtpmap:100 AMR-WB/16000/1\n"
    "stream_video_count=3a\n"
    "stream_video_0=m=video 0 RTP/AVP 102\n"
    "stream_video_1=a=rtpmap:102 H264/90000\n"
    "stream_video_2=a=fmtp:102 profile-level-id=42C016\n"
    "\n"
    "[test.service]\n"
    "stream_audio_count=3\n"
    "stream_audio_0=m=audio 0 RTP/AVP 96 100\n"
    "stream_audio_1=a=rtpmap:96 AMR/8000/1\n"
    "stream_audio_2=a=rtpmap:100 AMR-WB/16000/1\n"
    "stream_video_count=3\n"
    "stream_video_0=m=video 0 RTP/AVP 102 34\n"
    "stream_video_1=a=rtpmap:102 H264/90000\n"
    "stream_video_2=a=fmtp:102 profile-level-id=42800D\n"
    "\n"
};
// clang-format on

class ConfigFileBufferTest : public ::testing::Test
{
public:
    inline ConfigFileBufferTest() :
            m_strConfigData(CONFIG_DATA)
    {
    }

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}

protected:
    AString m_strConfigData;
};

TEST_F(ConfigFileBufferTest, ReadKeyCount)
{
    IConfigBuffer* piBuffer = ConfigFileBuffer::CreateFileBuffer(m_strConfigData);

    ASSERT_NE(piBuffer, nullptr);

    const IMS_CHAR* pszKey = "stream_audio";
    IMS_SINT32 nExpectedKeyCount = -1;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    const IMS_CHAR* pszSection = "test.capabilities";
    ASSERT_TRUE(piBuffer->CaptureSection(pszSection));

    nExpectedKeyCount = 3;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    pszKey = "framed";
    nExpectedKeyCount = 0;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    pszKey = "stream_video";
    nExpectedKeyCount = -1;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    piBuffer->Destroy();
}

}  // namespace android
