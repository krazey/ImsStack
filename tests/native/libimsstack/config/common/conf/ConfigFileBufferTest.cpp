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

#include "MockIFile.h"
#include "PlatformContext.h"
#include "TestFileService.h"
#include "conf/ConfigFileBuffer.h"

using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{
// clang-format off
static const IMS_CHAR CONFIG_DATA[] = {
    ";first data\n"
    "\n"
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
    ";second data\n"
    "[test.service]\n"
    ";audio config\n"
    "stream_audio_count=3\n"
    "stream_audio_0=m=audio 0 RTP/AVP 96 100\n"
    "stream_audio_1=a=rtpmap:96 AMR/8000/1\n"
    "stream_audio_2=a=rtpmap:100 AMR-WB/16000/1\n"
    ";video config\n"
    "stream_video_count=3\n"
    "stream_video_0=m=video 0 RTP/AVP 102 34\n"
    "stream_video_1=a=rtpmap:102 H264/90000\n"
    "stream_video_2=a=fmtp:102 profile-level-id=42800D\n"
    "\n"
    ";Config done\n"
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

    // Reading value before Capture Section
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);
    EXPECT_EQ(piBuffer->ReadValue(pszKey), AString::ConstNull());
    EXPECT_EQ(piBuffer->ReadValue(pszKey, 0), AString::ConstNull());
    EXPECT_FALSE(piBuffer->ReadValueBoolean("test_data"));
    EXPECT_EQ(piBuffer->ReadValueInt("stream_video_2"), -1);

    const IMS_CHAR* pszSection = "test.capabilities";
    EXPECT_TRUE(piBuffer->CaptureSection(pszSection));

    nExpectedKeyCount = 3;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    AString strValue("m=audio 0 RTP/AVP 96 100");
    EXPECT_EQ(piBuffer->ReadValue(pszKey, 0), strValue);

    pszKey = "framed";
    nExpectedKeyCount = 0;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    pszKey = "stream_video";
    nExpectedKeyCount = -1;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    pszSection = "test.service";
    EXPECT_TRUE(piBuffer->CaptureSection(pszSection));

    nExpectedKeyCount = 3;
    EXPECT_EQ(piBuffer->ReadKeyCount(pszKey), nExpectedKeyCount);

    strValue = "m=video 0 RTP/AVP 102 34";
    EXPECT_TRUE((piBuffer->ReadValue(pszKey)).IsNull());
    pszKey = "stream_video_0";
    EXPECT_EQ((piBuffer->ReadValue(pszKey)), strValue);
    EXPECT_EQ(piBuffer->ReadValue(IMS_NULL), AString::ConstNull());
    EXPECT_EQ(piBuffer->ReadValueInt("stream_video_2"), -1);

    // Reading wrong Section
    EXPECT_FALSE(piBuffer->CaptureSection(pszSection, 1));
    pszSection = "test.service2";
    EXPECT_FALSE(piBuffer->CaptureSection(pszSection));

    piBuffer->Destroy();
}

TEST_F(ConfigFileBufferTest, WriteKeyCount)
{
    IConfigBuffer* piBuffer = ConfigFileBuffer::CreateFileBuffer(m_strConfigData);

    ASSERT_NE(piBuffer, nullptr);

    // Writing value before section capture
    EXPECT_FALSE(piBuffer->WriteKeyCount("stream_video", 4));
    EXPECT_FALSE(piBuffer->WriteValue("stream_video_0", AString("8000")));
    EXPECT_FALSE(piBuffer->WriteValue("stream_video", 1, AString("4000")));
    EXPECT_FALSE(piBuffer->WriteValueInt("stream_video_2", 8000));
    EXPECT_FALSE(piBuffer->WriteValueBoolean("stream_video_1", true));

    const IMS_CHAR* pszSection = "test.service";
    EXPECT_TRUE(piBuffer->CaptureSection(pszSection));

    // Writing value after section capture
    EXPECT_TRUE(piBuffer->WriteKeyCount("stream_video", 4));
    EXPECT_TRUE(piBuffer->WriteValue("stream_video_0", AString("8000")));
    EXPECT_TRUE(piBuffer->WriteValue("stream_video", 1, AString("4000")));
    EXPECT_FALSE(piBuffer->WriteValue("stream_video_3", AString("100")));

    EXPECT_EQ(piBuffer->ReadValue("stream_video_0"), AString("8000"));

    EXPECT_FALSE(piBuffer->WriteValueInt("stream_video_2", -1));
    EXPECT_TRUE(piBuffer->WriteValueInt("stream_video_2", 8000));
    EXPECT_EQ(piBuffer->ReadValueInt("stream_video_2"), 8000);

    EXPECT_TRUE(piBuffer->WriteValueBoolean("stream_video_1", true));
    EXPECT_TRUE(piBuffer->ReadValueBoolean("stream_video_1"));
    EXPECT_FALSE(piBuffer->ReadValueBoolean("stream_video_2"));

    piBuffer->Destroy();
}

TEST_F(ConfigFileBufferTest, Create)
{
    TestFileService objFileService;
    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_FILE, &objFileService);
    MockIFile& objFile = objFileService.GetMockFile();

    ConfigFileBuffer objConfigFileBuffer("com.test.ut", "testconfig");
    ConfigBuffer* pConfigBuffer = static_cast<ConfigBuffer*>(&objConfigFileBuffer);

    EXPECT_CALL(objFile, Open)
            .Times(1)
            .WillOnce(Invoke(
                    [&](AString strName, Unused)
                    {
                        EXPECT_STREQ(strName.GetStr(), "com.test.uttestconfig.conf");
                        return IMS_FALSE;
                    }));
    EXPECT_FALSE(pConfigBuffer->Create(0));

    EXPECT_CALL(objFile, Open).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objFile, GetSize).Times(1).WillOnce(Return(0));
    EXPECT_FALSE(pConfigBuffer->Create(0));

    IMS_UINT32 nSize = m_strConfigData.GetLength();
    EXPECT_CALL(objFile, GetSize).Times(AnyNumber()).WillRepeatedly(Return(nSize));
    EXPECT_CALL(objFile, Read).Times(1).WillOnce(Return(0));

    EXPECT_FALSE(pConfigBuffer->Create(0));

    EXPECT_CALL(objFile, Read)
            .Times(1)
            .WillOnce(Invoke(
                    [&](void* pBuffer, Unused)
                    {
                        IMS_MEM_Memcpy(pBuffer, m_strConfigData.GetStr(), nSize);
                        return nSize;
                    }));
    EXPECT_TRUE(pConfigBuffer->Create(0));

    IConfigBuffer* piBuffer = static_cast<IConfigBuffer*>(&objConfigFileBuffer);
    ASSERT_TRUE(piBuffer->CaptureSection("test.capabilities"));

    EXPECT_CALL(objFileService.GetMockFileUtil(), Exist)
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objFile, Open).Times(1).WillOnce(Return(IMS_FALSE));

    EXPECT_FALSE(piBuffer->WriteToMedium());

    EXPECT_CALL(objFile, Open).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objFile, Write).Times(1).WillOnce(Return(0));
    EXPECT_FALSE(piBuffer->WriteToMedium());

    EXPECT_CALL(objFile, Write).Times(1).WillOnce(Return(nSize));
    EXPECT_TRUE(piBuffer->WriteToMedium());

    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_FILE, IMS_NULL);
}

TEST_F(ConfigFileBufferTest, ConfigDataError)
{
    AString strConfigData("[test.service]\n[audio config]\n");
    IConfigBuffer* piBuffer = ConfigFileBuffer::CreateFileBuffer(strConfigData);

    ASSERT_EQ(piBuffer, nullptr);
}

}  // namespace android
