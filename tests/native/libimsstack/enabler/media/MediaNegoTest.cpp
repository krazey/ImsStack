/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "MediaNego.h"
#include "MockISdpReader.h"
#include "MockISession.h"
#include "MockISessionDescriptor.h"
#include "audio/MockAudioNego.h"
#include "media/MockIMediaDescriptor.h"
#include "sdp/SdpMedia.h"
#include "text/MockTextNego.h"
#include "video/MockVideoNego.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;

class FakeMediaNego : public MediaNego
{
public:
    explicit FakeMediaNego(IN IMS_UINT32 nSlotId) :
            MediaNego(nSlotId)
    {
        m_pAudioNego = new MockAudioNego(DEFAULT_SLOT_ID);
        m_pVideoNego = new MockVideoNego(DEFAULT_SLOT_ID);
        m_pTextNego = new MockTextNego(DEFAULT_SLOT_ID);
    }
    virtual ~FakeMediaNego() {}

    MockAudioNego* GetAudioNego() { return reinterpret_cast<MockAudioNego*>(m_pAudioNego); }
    MockVideoNego* GetVideoNego() { return reinterpret_cast<MockVideoNego*>(m_pVideoNego); }
    MockTextNego* GetTextNego() { return reinterpret_cast<MockTextNego*>(m_pTextNego); }
};

class MediaNegoTest : public ::testing::Test
{
public:
    FakeMediaNego* m_pMediaNego;
    MockISession* m_pIsession;

protected:
    virtual void SetUp() override
    {
        m_pMediaNego = new FakeMediaNego(DEFAULT_SLOT_ID);
        m_pIsession = new MockISession();
    }

    virtual void TearDown() override
    {
        delete m_pMediaNego;
        delete m_pIsession;
    }
};

TEST_F(MediaNegoTest, testGetSupportedMediaTypesFromSdp)
{
    MockISdpReader objISdpReader;
    ImsList<IMediaDescriptor*> objMediaDescriptors;

    MockIMediaDescriptor objAudioDescriptor;
    MockIMediaDescriptor objVideoDescriptor;
    MockIMediaDescriptor objTextDescriptor;

    MockISessionDescriptor objSessionDescriptor;

    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);

    SdpMedia objSdpMediaVideo;
    objSdpMediaVideo.SetType(SdpMedia::TYPE_VIDEO);

    SdpMedia objSdpMediaText;
    objSdpMediaText.SetType(SdpMedia::TYPE_TEXT);

    objMediaDescriptors.Append(&objAudioDescriptor);
    objMediaDescriptors.Append(&objVideoDescriptor);
    objMediaDescriptors.Append(&objTextDescriptor);

    ON_CALL(*m_pIsession, GetRemoteMediaCapabilities()).WillByDefault(Return(&objISdpReader));
    ON_CALL(objISdpReader, GetMediaDescriptors()).WillByDefault(ReturnRef(objMediaDescriptors));
    ON_CALL(objISdpReader, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));

    ON_CALL(objAudioDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMediaAudio));
    ON_CALL(objVideoDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMediaVideo));
    ON_CALL(objTextDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMediaText));

    ON_CALL(*m_pMediaNego->GetAudioNego(),
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objAudioDescriptor))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_EQ(m_pMediaNego->GetSupportedMediaTypesFromSdp(m_pIsession), MEDIA_TYPE_AUDIO);

    ON_CALL(*m_pMediaNego->GetVideoNego(),
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objVideoDescriptor))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(m_pMediaNego->GetSupportedMediaTypesFromSdp(m_pIsession), MEDIA_TYPE_AUDIOVIDEO);

    ON_CALL(*m_pMediaNego->GetVideoNego(),
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objVideoDescriptor))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*m_pMediaNego->GetTextNego(),
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objTextDescriptor))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(m_pMediaNego->GetSupportedMediaTypesFromSdp(m_pIsession), MEDIA_TYPE_AUDIOTEXT);
}
