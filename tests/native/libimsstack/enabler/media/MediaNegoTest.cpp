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
#include <memory>

#include "MediaNego.h"
#include "MockISdpReader.h"
#include "MockISession.h"
#include "MockISessionDescriptor.h"
#include "audio/MockAudioNego.h"
#include "media/MockIMediaDescriptor.h"
#include "sdp/SdpMedia.h"
#include "text/MockTextNego.h"
#include "video/MockVideoNego.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;

class MediaNegoTest : public ::testing::Test
{
public:
    MediaNego m_objMediaNego;
    MockISession m_objIsession;
    std::shared_ptr<MockAudioNego> m_pMockAudioNego;
    std::shared_ptr<MockVideoNego> m_pMockVideoNego;
    std::shared_ptr<MockTextNego> m_pMockTextNego;
    std::shared_ptr<MediaEnvironment> m_pMediaEnvironment;

protected:
    virtual void SetUp() override
    {
        m_pMediaEnvironment = std::make_shared<MediaEnvironment>();
        m_pMockAudioNego = std::make_shared<MockAudioNego>(DEFAULT_SLOT_ID);
        m_pMockVideoNego = std::make_shared<MockVideoNego>(DEFAULT_SLOT_ID);
        m_pMockTextNego = std::make_shared<MockTextNego>(DEFAULT_SLOT_ID);
        m_objMediaNego.SetAudioNego(m_pMockAudioNego);
        m_objMediaNego.SetVideoNego(m_pMockVideoNego);
        m_objMediaNego.SetTextNego(m_pMockTextNego);
    }

    virtual void TearDown() override {}
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

    ON_CALL(m_objIsession, GetRemoteMediaCapabilities()).WillByDefault(Return(&objISdpReader));
    ON_CALL(objISdpReader, GetMediaDescriptors()).WillByDefault(ReturnRef(objMediaDescriptors));
    ON_CALL(objISdpReader, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));

    ON_CALL(objAudioDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMediaAudio));
    ON_CALL(objVideoDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMediaVideo));
    ON_CALL(objTextDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMediaText));

    ON_CALL(*m_pMockAudioNego,
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objAudioDescriptor))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_EQ(m_objMediaNego.GetSupportedMediaTypesFromSdp(&m_objIsession), MEDIA_TYPE_AUDIO);

    ON_CALL(*m_pMockVideoNego,
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objVideoDescriptor))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(m_objMediaNego.GetSupportedMediaTypesFromSdp(&m_objIsession), MEDIA_TYPE_AUDIOVIDEO);

    ON_CALL(*m_pMockVideoNego,
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objVideoDescriptor))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*m_pMockTextNego,
            IsMediaCodecFromSdpSupported(objISdpReader.GetSessionDescriptor(), &objTextDescriptor))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(m_objMediaNego.GetSupportedMediaTypesFromSdp(&m_objIsession), MEDIA_TYPE_AUDIOTEXT);
}

TEST_F(MediaNegoTest, testCreateProfile)
{
    EXPECT_CALL(*m_pMockAudioNego, CreateProfiles(m_pMediaEnvironment, _)).Times(1);
    EXPECT_CALL(*m_pMockVideoNego, CreateProfiles(m_pMediaEnvironment, _)).Times(1);
    EXPECT_CALL(*m_pMockTextNego, CreateProfiles(m_pMediaEnvironment, _)).Times(1);
    m_objMediaNego.CreateProfile(m_pMediaEnvironment);
}

TEST_F(MediaNegoTest, testForking)
{
    EXPECT_FALSE(m_objMediaNego.Forking(IMS_NULL));

    MediaNego* pMediaNego1 = new MediaNego(DEFAULT_SLOT_ID);
    MediaNego* pMediaNego2 = new MediaNego(DEFAULT_SLOT_ID);
    EXPECT_TRUE(pMediaNego2->Forking(pMediaNego1));
    EXPECT_EQ(pMediaNego2->GetNegoState(), STATE_OFFER_SENT);

    EXPECT_NE(pMediaNego2->GetAudioNego(), IMS_NULL);
    EXPECT_NE(pMediaNego2->GetVideoNego(), IMS_NULL);
    EXPECT_NE(pMediaNego2->GetTextNego(), IMS_NULL);

    delete pMediaNego1;

    EXPECT_NE(pMediaNego2->GetAudioNego(), IMS_NULL);
    EXPECT_NE(pMediaNego2->GetVideoNego(), IMS_NULL);
    EXPECT_NE(pMediaNego2->GetTextNego(), IMS_NULL);

    delete pMediaNego2;
}

TEST_F(MediaNegoTest, testFormSdp)
{
    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
    m_objMediaNego.CreateProfile(m_pMediaEnvironment);
    EXPECT_TRUE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
}

TEST_F(MediaNegoTest, testNegotiateSdp)
{
    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    m_objMediaNego.CreateProfile(m_pMediaEnvironment);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
}

TEST_F(MediaNegoTest, testFinalizeSdp)
{
    EXPECT_CALL(*m_pMockAudioNego, FinalizeSdp(_, _)).Times(1);
    EXPECT_CALL(*m_pMockVideoNego, FinalizeSdp(_, _)).Times(1);
    EXPECT_CALL(*m_pMockTextNego, FinalizeSdp(_, _)).Times(1);
    m_objMediaNego.FinalizeSdp(&m_objIsession);
}
