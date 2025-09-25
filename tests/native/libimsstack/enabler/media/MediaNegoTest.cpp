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

#include "MediaDef.h"
#include "MediaEnvironment.h"
#include "MediaNego.h"
#include "MockISdpReader.h"
#include "MockISession.h"
#include "MockISessionDescriptor.h"
#include "audio/MockAudioNego.h"
#include "core/MockICoreService.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "sdp/SdpMedia.h"
#include "text/MockTextNego.h"
#include "video/MockVideoNego.h"

#include "config/MockMediaSessionConfigFactory.h"
#include "config/MockMediaSessionConfig.h"
#include "config/MockAudioConfiguration.h"
#include "config/MockVideoConfiguration.h"
#include "config/MockTextConfiguration.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const AString REMOTE_IP = "127.0.0.1";
const IMS_UINT32 LOCAL_PORT = 40000;
const IMS_UINT32 REMOTE_PORT = 40000;

class MediaNegoTest : public ::testing::Test
{
public:
    MediaNegoTest() :
            m_pMockAudioNego(IMS_NULL),
            m_pMockVideoNego(IMS_NULL),
            m_pMockTextNego(IMS_NULL),
            m_pMediaEnvironment(IMS_NULL)
    {
    }
    ~MediaNegoTest() {}
    MediaNego m_objMediaNego;
    MockISession m_objIsession;
    IpAddress m_objLocalIpAddress;
    IpAddress m_objRemoteIpAddress;
    MockICoreService m_objICoreService;
    std::shared_ptr<MockAudioNego> m_pMockAudioNego;
    std::shared_ptr<MockVideoNego> m_pMockVideoNego;
    std::shared_ptr<MockTextNego> m_pMockTextNego;
    std::shared_ptr<MediaEnvironment> m_pMediaEnvironment;

    MockMediaSessionConfigFactory m_objConfigFactory;
    MockMediaSessionConfig m_objConfig;
    MockAudioConfiguration m_objAudioConfig;
    MockVideoConfiguration m_objVideoConfig;
    MockTextConfiguration m_objTextConfig;

protected:
    virtual void SetUp() override
    {
        m_objLocalIpAddress = IpAddress(LOCAL_IP);
        m_objRemoteIpAddress = IpAddress(REMOTE_IP);
        ON_CALL(m_objICoreService, GetIpAddress()).WillByDefault(ReturnRef(m_objLocalIpAddress));

        m_pMediaEnvironment = std::make_shared<MediaEnvironment>(
                MEDIA_NETWORK_NONE, MEDIA_SERVICE_DEFAULT, &m_objICoreService);
        m_pMockAudioNego = std::make_shared<MockAudioNego>(DEFAULT_SLOT_ID);
        m_pMockVideoNego = std::make_shared<MockVideoNego>(DEFAULT_SLOT_ID);
        m_pMockTextNego = std::make_shared<MockTextNego>(DEFAULT_SLOT_ID);
        m_objMediaNego.SetAudioNego(m_pMockAudioNego);
        m_objMediaNego.SetVideoNego(m_pMockVideoNego);
        m_objMediaNego.SetTextNego(m_pMockTextNego);

        MediaSessionConfigFactory::ReleaseInstance(MediaSessionConfigFactory::GetInstance());
        MediaSessionConfigFactory::SetInstance(&m_objConfigFactory);

        ON_CALL(m_objConfigFactory, FindMediaSessionConfig(_, _))
                .WillByDefault(Return(&m_objConfig));
        ON_CALL(m_objConfig, GetAudioConfiguration()).WillByDefault(Return(&m_objAudioConfig));
        ON_CALL(m_objConfig, GetVideoConfiguration()).WillByDefault(Return(&m_objVideoConfig));
        ON_CALL(m_objConfig, GetTextConfiguration()).WillByDefault(Return(&m_objTextConfig));
    }

    virtual void TearDown() override { MediaSessionConfigFactory::SetInstance(IMS_NULL); }

    void SetupProfileExpectation()
    {
        EXPECT_CALL(*m_pMockAudioNego, CreateProfiles(_, _)).Times(1);
        EXPECT_CALL(*m_pMockVideoNego, CreateProfiles(_, _)).Times(1);
        EXPECT_CALL(*m_pMockTextNego, CreateProfiles(_, _)).Times(1);
    }

    void SetUpMedia(MockIMedia& objIMedia, MockIMediaDescriptor& objIMediaDescriptor,
            SdpMedia& objSdpMedia, ImsList<IMedia*>& objMediaList)
    {
        ON_CALL(objIMedia, GetMediaDescriptor()).WillByDefault(Return(&objIMediaDescriptor));
        ON_CALL(objIMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_UNCHANGED));
        ON_CALL(objIMediaDescriptor, GetMediaDescriptionEx()).WillByDefault(Return(&objSdpMedia));
        objMediaList.Append(&objIMedia);
        ON_CALL(m_objIsession, GetMedia()).WillByDefault(Return(objMediaList));
        ON_CALL(m_objIsession, CreateMedia(_, _, _)).WillByDefault(Return(IMS_NULL));
    }
};

TEST_F(MediaNegoTest, testIsForking)
{
    EXPECT_FALSE(m_objMediaNego.IsForking());
}

TEST_F(MediaNegoTest, testSetNegoState)
{
    m_objMediaNego.SetNegoState(STATE_NEGOTIATED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_NEGOTIATED);
}

TEST_F(MediaNegoTest, testGetNegoState)
{
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_IDLE);
}

TEST_F(MediaNegoTest, testSetAudioNego)
{
    std::shared_ptr<MockAudioNego> pMockAudioNego =
            std::make_shared<MockAudioNego>(DEFAULT_SLOT_ID);
    m_objMediaNego.SetAudioNego(pMockAudioNego);
    EXPECT_EQ(m_objMediaNego.GetAudioNego(), pMockAudioNego);
}

TEST_F(MediaNegoTest, testGetAudioNego)
{
    EXPECT_EQ(m_objMediaNego.GetAudioNego(), m_pMockAudioNego);
}

TEST_F(MediaNegoTest, testSetVideoNego)
{
    std::shared_ptr<MockVideoNego> pMockVideoNego =
            std::make_shared<MockVideoNego>(DEFAULT_SLOT_ID);
    m_objMediaNego.SetVideoNego(pMockVideoNego);
    EXPECT_EQ(m_objMediaNego.GetVideoNego(), pMockVideoNego);
}

TEST_F(MediaNegoTest, testGetVideoNego)
{
    EXPECT_EQ(m_objMediaNego.GetVideoNego(), m_pMockVideoNego);
}

TEST_F(MediaNegoTest, testSetTextNego)
{
    std::shared_ptr<MockTextNego> pMockTextNego = std::make_shared<MockTextNego>(DEFAULT_SLOT_ID);
    m_objMediaNego.SetTextNego(pMockTextNego);
    EXPECT_EQ(m_objMediaNego.GetTextNego(), pMockTextNego);
}

TEST_F(MediaNegoTest, testGetTextNego)
{
    EXPECT_EQ(m_objMediaNego.GetTextNego(), m_pMockTextNego);
}

TEST_F(MediaNegoTest, testCreateProfileNullNego)
{
    m_objMediaNego.SetAudioNego(IMS_NULL);
    m_objMediaNego.SetVideoNego(IMS_NULL);
    m_objMediaNego.SetTextNego(IMS_NULL);
    EXPECT_FALSE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));
}

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

    // test empty descriptor
    objMediaDescriptors.Clear();
    EXPECT_EQ(m_objMediaNego.GetSupportedMediaTypesFromSdp(&m_objIsession), MEDIA_TYPE_INVALID);

    // test null sdp reader
    ON_CALL(m_objIsession, GetRemoteMediaCapabilities()).WillByDefault(Return(IMS_NULL));
    EXPECT_EQ(m_objMediaNego.GetSupportedMediaTypesFromSdp(&m_objIsession), MEDIA_TYPE_INVALID);

    // test null session
    EXPECT_EQ(m_objMediaNego.GetSupportedMediaTypesFromSdp(IMS_NULL), MEDIA_TYPE_INVALID);
}

TEST_F(MediaNegoTest, testForking)
{
    EXPECT_FALSE(m_objMediaNego.Forking(IMS_NULL));

    MediaNego* pMediaNego1 = new MediaNego(DEFAULT_SLOT_ID);
    MediaNego* pMediaNego2 = new MediaNego(DEFAULT_SLOT_ID);
    EXPECT_TRUE(pMediaNego2->Forking(pMediaNego1));
    EXPECT_TRUE(pMediaNego2->IsForking());
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

TEST_F(MediaNegoTest, testFormSdpWithoutProfile)
{
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
}

TEST_F(MediaNegoTest, testFormSdpNullAudioNego)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(m_objIsession, GetMedia()).WillByDefault(Return(ImsList<IMedia*>()));
    ON_CALL(m_objIsession, CreateMedia(_, _, _)).WillByDefault(Return(IMS_NULL));

    // null audio nego
    m_objMediaNego.SetAudioNego(IMS_NULL);
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
    m_objMediaNego.SetAudioNego(m_pMockAudioNego);
}

TEST_F(MediaNegoTest, testFormSdpNullVideoNego)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(m_objIsession, GetMedia()).WillByDefault(Return(ImsList<IMedia*>()));
    ON_CALL(m_objIsession, CreateMedia(_, _, _)).WillByDefault(Return(IMS_NULL));

    // null video nego
    m_objMediaNego.SetVideoNego(IMS_NULL);
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_VIDEO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
    m_objMediaNego.SetVideoNego(m_pMockVideoNego);
}

TEST_F(MediaNegoTest, testFormSdpNullTextNego)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(m_objIsession, GetMedia()).WillByDefault(Return(ImsList<IMedia*>()));
    ON_CALL(m_objIsession, CreateMedia(_, _, _)).WillByDefault(Return(IMS_NULL));

    // null text nego
    m_objMediaNego.SetTextNego(IMS_NULL);
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_TEXT, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
    m_objMediaNego.SetTextNego(m_pMockTextNego);
}

TEST_F(MediaNegoTest, testFormSdpOfferSentState)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(m_objIsession, GetMedia()).WillByDefault(Return(ImsList<IMedia*>()));
    ON_CALL(m_objIsession, CreateMedia(_, _, _)).WillByDefault(Return(IMS_NULL));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testFormSdpSuccessAudio)
{
    m_objMediaNego.SetNegoState(STATE_IDLE);
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(*m_pMockAudioNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    MockIMedia objIMediaAudio;
    MockIMediaDescriptor objIMediaDescriptorAudio;
    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);
    objSdpMediaAudio.SetPort(LOCAL_PORT);
    ImsList<IMedia*> objMediaListAudio;
    SetUpMedia(objIMediaAudio, objIMediaDescriptorAudio, objSdpMediaAudio, objMediaListAudio);
    EXPECT_TRUE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_INVALID, MEDIA_DIRECTION_INVALID, IMS_FALSE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testFormSdpSuccessVideo)
{
    m_objMediaNego.SetNegoState(STATE_IDLE);
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(*m_pMockVideoNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    MockIMedia objIMediaVideo;
    MockIMediaDescriptor objIMediaDescriptorVideo;
    SdpMedia objSdpMediaVideo;
    objSdpMediaVideo.SetType(SdpMedia::TYPE_VIDEO);
    ImsList<IMedia*> objMediaListVideo;
    SetUpMedia(objIMediaVideo, objIMediaDescriptorVideo, objSdpMediaVideo, objMediaListVideo);
    EXPECT_TRUE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_VIDEO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testFormSdpSuccessText)
{
    m_objMediaNego.SetNegoState(STATE_IDLE);
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(*m_pMockTextNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    MockIMedia objIMediaText;
    MockIMediaDescriptor objIMediaDescriptorText;
    SdpMedia objSdpMediaText;
    objSdpMediaText.SetType(SdpMedia::TYPE_TEXT);
    ImsList<IMedia*> objMediaListText;
    SetUpMedia(objIMediaText, objIMediaDescriptorText, objSdpMediaText, objMediaListText);
    EXPECT_TRUE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_TEXT, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_FALSE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testFormSdpSuccessAudioVideo)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(*m_pMockAudioNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockVideoNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    // success case 4: audio and video
    MockIMedia objIMediaAudio;
    MockIMediaDescriptor objIMediaDescriptorAudio;
    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);
    MockIMedia objIMediaVideo;
    MockIMediaDescriptor objIMediaDescriptorVideo;
    SdpMedia objSdpMediaVideo;
    objSdpMediaVideo.SetType(SdpMedia::TYPE_VIDEO);

    ImsList<IMedia*> objMediaListAudioVideo;
    SetUpMedia(objIMediaAudio, objIMediaDescriptorAudio, objSdpMediaAudio, objMediaListAudioVideo);
    SetUpMedia(objIMediaVideo, objIMediaDescriptorVideo, objSdpMediaVideo, objMediaListAudioVideo);
    EXPECT_TRUE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIOVIDEO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testFormSdpSuccessAudioVideoText)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    ON_CALL(*m_pMockAudioNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockVideoNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockTextNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    // success case 5: audio, video and text
    MockIMedia objIMediaAudio;
    MockIMediaDescriptor objIMediaDescriptorAudio;
    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);
    MockIMedia objIMediaVideo;
    MockIMediaDescriptor objIMediaDescriptorVideo;
    SdpMedia objSdpMediaVideo;
    objSdpMediaVideo.SetType(SdpMedia::TYPE_VIDEO);
    MockIMedia objIMediaText;
    MockIMediaDescriptor objIMediaDescriptorText;
    SdpMedia objSdpMediaText;
    objSdpMediaText.SetType(SdpMedia::TYPE_TEXT);

    ImsList<IMedia*> objMediaListAudioVideoText;
    SetUpMedia(
            objIMediaAudio, objIMediaDescriptorAudio, objSdpMediaAudio, objMediaListAudioVideoText);
    SetUpMedia(
            objIMediaVideo, objIMediaDescriptorVideo, objSdpMediaVideo, objMediaListAudioVideoText);
    SetUpMedia(objIMediaText, objIMediaDescriptorText, objSdpMediaText, objMediaListAudioVideoText);

    EXPECT_TRUE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIOVIDEOTEXT,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testFormSdpFormSdpFailedAudio)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));
    MockIMedia objIMediaAudio;
    MockIMediaDescriptor objIMediaDescriptorAudio;
    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);
    ImsList<IMedia*> objMediaListAudio;
    SetUpMedia(objIMediaAudio, objIMediaDescriptorAudio, objSdpMediaAudio, objMediaListAudio);

    ON_CALL(*m_pMockAudioNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
    ON_CALL(*m_pMockAudioNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_IDLE);
}

TEST_F(MediaNegoTest, testFormSdpFormSdpFailedVideo)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));

    MockIMedia objIMediaVideo;
    MockIMediaDescriptor objIMediaDescriptorVideo;
    SdpMedia objSdpMediaVideo;
    objSdpMediaVideo.SetType(SdpMedia::TYPE_VIDEO);
    ImsList<IMedia*> objMediaListVideo;
    SetUpMedia(objIMediaVideo, objIMediaDescriptorVideo, objSdpMediaVideo, objMediaListVideo);
    ON_CALL(*m_pMockVideoNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_VIDEO, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
    ON_CALL(*m_pMockVideoNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_IDLE);
}

TEST_F(MediaNegoTest, testFormSdpFormSdpFailedText)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MockISessionDescriptor objSessionDescriptor;
    ON_CALL(m_objIsession, GetSessionDescriptor()).WillByDefault(Return(&objSessionDescriptor));

    MockIMedia objIMediaText;
    MockIMediaDescriptor objIMediaDescriptorText;
    SdpMedia objSdpMediaText;
    objSdpMediaText.SetType(SdpMedia::TYPE_TEXT);
    ImsList<IMedia*> objMediaListText;
    SetUpMedia(objIMediaText, objIMediaDescriptorText, objSdpMediaText, objMediaListText);
    ON_CALL(*m_pMockTextNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(m_objMediaNego.FormSdp(&m_objIsession, MEDIA_TYPE_TEXT, MEDIA_DIRECTION_SEND,
            MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND, IMS_TRUE));
    ON_CALL(*m_pMockTextNego, FormSdp(_, _, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_IDLE);
}

TEST_F(MediaNegoTest, testNegotiateSdpWithoutMediaEnvironment)
{
    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
}

TEST_F(MediaNegoTest, testNegotiateSdpNullNegoAudio)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    m_objMediaNego.SetAudioNego(IMS_NULL);
    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
    EXPECT_EQ(nAudioDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testNegotiateSdpNullNegoVideo)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    // success case 5: audio, video and text
    MockIMedia objIMediaAudio;
    MockIMediaDescriptor objIMediaDescriptorAudio;
    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);
    MockIMedia objIMediaVideo;
    MockIMediaDescriptor objIMediaDescriptorVideo;
    SdpMedia objSdpMediaVideo;
    objSdpMediaVideo.SetType(SdpMedia::TYPE_VIDEO);

    ImsList<IMedia*> objMediaListAudioVideo;
    SetUpMedia(objIMediaAudio, objIMediaDescriptorAudio, objSdpMediaAudio, objMediaListAudioVideo);
    SetUpMedia(objIMediaVideo, objIMediaDescriptorVideo, objSdpMediaVideo, objMediaListAudioVideo);

    m_objMediaNego.SetVideoNego(IMS_NULL);
    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
    EXPECT_EQ(nVideoDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testNegotiateSdpNullNegoText)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    // success case 5: audio, video and text
    MockIMedia objIMediaAudio;
    MockIMediaDescriptor objIMediaDescriptorAudio;
    SdpMedia objSdpMediaAudio;
    objSdpMediaAudio.SetType(SdpMedia::TYPE_AUDIO);
    MockIMedia objIMediaText;
    MockIMediaDescriptor objIMediaDescriptorText;
    SdpMedia objSdpMediaText;
    objSdpMediaText.SetType(SdpMedia::TYPE_TEXT);

    ImsList<IMedia*> objMediaListAudioText;
    SetUpMedia(objIMediaAudio, objIMediaDescriptorAudio, objSdpMediaAudio, objMediaListAudioText);
    SetUpMedia(objIMediaText, objIMediaDescriptorText, objSdpMediaText, objMediaListAudioText);

    m_objMediaNego.SetTextNego(IMS_NULL);
    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
    EXPECT_EQ(nTextDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testNegotiateSdpFailedAudio)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockAudioNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_INVALID), Return()));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testNegotiateSdpSuccessAudio)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    // success case
    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioNego, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioNego, GetLocalPort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioNego, GetNegotiatedCodec()).WillByDefault(Return(AUDIO_CODEC_AMR));
    EXPECT_CALL(*m_pMockAudioNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_SEND), Return()));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_TRUE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::NO_ERROR);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_NEGOTIATED);
}

TEST_F(MediaNegoTest, testNegotiateSdpFailedVideo)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_VIDEO);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockVideoNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_INVALID), Return()));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testNegotiateSdpSuccessVideo)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    // success case
    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_VIDEO);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockVideoNego, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockVideoNego, GetNegotiatedResolution())
            .WillByDefault(Return(VIDEO_RESOLUTION_VGA_PR));
    ON_CALL(*m_pMockVideoNego, GetNegotiatedRtpPort()).WillByDefault(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockVideoNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_SEND), Return()));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_TRUE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::NO_ERROR);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_NEGOTIATED);
}

TEST_F(MediaNegoTest, testNegotiateSdpFailedText)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_TEXT);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockTextNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_INVALID), Return()));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_FALSE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_OFFER_SENT);
}

TEST_F(MediaNegoTest, testNegotiateSdpSuccessText)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    // success case
    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_TEXT);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockTextNego, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockTextNego, GetNegotiatedCodec()).WillByDefault(Return(TEXT_CODEC_T140));
    ON_CALL(*m_pMockTextNego, GetNegotiatedRtpPort()).WillByDefault(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockTextNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_SEND), Return()));

    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_TRUE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::NO_ERROR);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_NEGOTIATED);
}

TEST_F(MediaNegoTest, testFinalizeSdpWithNullSession)
{
    // Act & Assert: Call with a null session, which should be handled gracefully without crashing.
    // No mocks are needed as the function should return early.
    m_objMediaNego.FinalizeSdp(IMS_NULL);
}

TEST_F(MediaNegoTest, testFinalizeSdpRemovesDeletedMedia)
{
    // Arrange: Create mock media objects, one to be deleted and one to be kept.
    MockIMedia objMediaToDelete;
    MockIMedia objMediaToKeep;
    ImsList<IMedia*> mediaList;
    mediaList.Append(&objMediaToDelete);
    mediaList.Append(&objMediaToKeep);

    // Setup the states for the media objects.
    ON_CALL(objMediaToDelete, GetState()).WillByDefault(Return(IMedia::STATE_DELETED));
    ON_CALL(objMediaToKeep, GetState()).WillByDefault(Return(IMedia::STATE_ACTIVE));

    // Configure the mock session to return the prepared list of media.
    ON_CALL(m_objIsession, GetMedia()).WillByDefault(Return(mediaList));

    // Expect that RemoveMedia is called only for the media object marked as STATE_DELETED.
    EXPECT_CALL(m_objIsession, RemoveMedia(&objMediaToDelete)).Times(1);
    EXPECT_CALL(m_objIsession, RemoveMedia(&objMediaToKeep)).Times(0);

    // Act: Call the method under test.
    m_objMediaNego.FinalizeSdp(&m_objIsession);
}

TEST_F(MediaNegoTest, testConfirmSessionAudioNegotiatedSetsStateNegotiated)
{
    // Arrange
    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_CALL(*m_pMockAudioNego, ConfirmSession()).Times(1);
    EXPECT_CALL(*m_pMockVideoNego, ConfirmSession()).Times(1);
    EXPECT_CALL(*m_pMockTextNego, ConfirmSession()).Times(1);

    ON_CALL(*m_pMockAudioNego, GetNegotiatedCodec()).WillByDefault(Return(AUDIO_CODEC_AMR));
    ON_CALL(*m_pMockVideoNego, GetNegotiatedResolution())
            .WillByDefault(Return(VIDEO_RESOLUTION_INVALID));
    ON_CALL(*m_pMockTextNego, GetNegotiatedCodec()).WillByDefault(Return(TEXT_CODEC_NONE));

    // Act
    m_objMediaNego.ConfirmSession();

    // Assert
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_NEGOTIATED);
}

TEST_F(MediaNegoTest, testConfirmSessionAudioVideoNegotiatedSetsStateIdle)
{
    // Arrange
    m_objMediaNego.SetNegoState(STATE_OFFER_SENT);
    EXPECT_CALL(*m_pMockAudioNego, ConfirmSession()).Times(1);
    EXPECT_CALL(*m_pMockVideoNego, ConfirmSession()).Times(1);
    EXPECT_CALL(*m_pMockTextNego, ConfirmSession()).Times(1);

    ON_CALL(*m_pMockAudioNego, GetNegotiatedCodec()).WillByDefault(Return(AUDIO_CODEC_NONE));
    ON_CALL(*m_pMockVideoNego, GetNegotiatedResolution())
            .WillByDefault(Return(VIDEO_RESOLUTION_INVALID));
    ON_CALL(*m_pMockTextNego, GetNegotiatedCodec()).WillByDefault(Return(TEXT_CODEC_NONE));

    // Act
    m_objMediaNego.ConfirmSession();

    // Assert
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_IDLE);
}

TEST_F(MediaNegoTest, testConfirmSessionMediaNegotiatedButInitialStateIdleStaysIdle)
{
    // Arrange
    m_objMediaNego.SetNegoState(STATE_IDLE);
    EXPECT_CALL(*m_pMockAudioNego, ConfirmSession()).Times(1);
    EXPECT_CALL(*m_pMockVideoNego, ConfirmSession()).Times(1);
    EXPECT_CALL(*m_pMockTextNego, ConfirmSession()).Times(1);

    ON_CALL(*m_pMockAudioNego, GetNegotiatedCodec()).WillByDefault(Return(AUDIO_CODEC_AMR));

    // Act
    m_objMediaNego.ConfirmSession();

    // Assert
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_IDLE);
}

TEST_F(MediaNegoTest, testGetNegotiatedAudioQualitySuccess)
{
    ON_CALL(*m_pMockAudioNego, GetRemotePort()).WillByDefault(Return(10000));
    ON_CALL(*m_pMockVideoNego, GetLocalPort()).WillByDefault(Return(10000));
    ON_CALL(*m_pMockAudioNego, GetNegotiatedCodec()).WillByDefault(Return(AUDIO_CODEC_AMR));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedAudioQuality(), AUDIO_CODEC_AMR);
}

TEST_F(MediaNegoTest, testGetNegotiatedAudioQualityRemotePortNegative)
{
    ON_CALL(*m_pMockAudioNego, GetRemotePort()).WillByDefault(Return(-1));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedAudioQuality(), AUDIO_CODEC_NOT_USED);
}

TEST_F(MediaNegoTest, testGetNegotiatedAudioQualityNullAudioNego)
{
    m_objMediaNego.SetAudioNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedAudioQuality(), AUDIO_CODEC_NOT_USED);
}

TEST_F(MediaNegoTest, testGetNegotiatedVideoQuality)
{
    EXPECT_CALL(*m_pMockVideoNego, GetNegotiatedRtpPort()).WillOnce(Return(10000));
    EXPECT_CALL(*m_pMockVideoNego, GetNegotiatedResolution())
            .WillOnce(Return(VIDEO_RESOLUTION_VGA_PR));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedVideoQuality(), VIDEO_RESOLUTION_VGA_PR);

    EXPECT_CALL(*m_pMockVideoNego, GetNegotiatedRtpPort()).WillOnce(Return(0));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedVideoQuality(), VIDEO_RESOLUTION_NOT_USED);

    m_objMediaNego.SetVideoNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedVideoQuality(), VIDEO_RESOLUTION_NOT_USED);
}

TEST_F(MediaNegoTest, testGetNegotiatedTextQuality)
{
    EXPECT_CALL(*m_pMockTextNego, GetNegotiatedRtpPort()).WillOnce(Return(10000));
    EXPECT_CALL(*m_pMockTextNego, GetNegotiatedCodec()).WillOnce(Return(TEXT_CODEC_T140));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextQuality(), TEXT_CODEC_T140);

    EXPECT_CALL(*m_pMockTextNego, GetNegotiatedRtpPort()).WillOnce(Return(0));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextQuality(), TEXT_CODEC_NOT_USED);

    m_objMediaNego.SetTextNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextQuality(), TEXT_CODEC_NOT_USED);
}

TEST_F(MediaNegoTest, testGetNegotiatedTextQualityRemotePortZero)
{
    ON_CALL(*m_pMockTextNego, GetRemotePort()).WillByDefault(Return(0));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextQuality(), TEXT_CODEC_NOT_USED);
}

TEST_F(MediaNegoTest, testGetNegotiatedTextQualityNullTextNego)
{
    m_objMediaNego.SetTextNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextQuality(), TEXT_CODEC_NOT_USED);
}

TEST_F(MediaNegoTest, testGetNegotiatedAudioDirectionNullAudioNego)
{
    m_objMediaNego.SetAudioNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedAudioDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testGetNegotiatedVideoDirectionNullVideoNego)
{
    m_objMediaNego.SetVideoNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedVideoDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testGetNegotiatedTextDirectionNullTextNego)
{
    m_objMediaNego.SetTextNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testGetNegotiatedAudioDirection)
{
    EXPECT_CALL(*m_pMockAudioNego, GetNegotiatedDirection()).WillOnce(Return(MEDIA_DIRECTION_SEND));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedAudioDirection(), MEDIA_DIRECTION_SEND);

    m_objMediaNego.SetAudioNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedAudioDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testGetNegotiatedVideoDirection)
{
    EXPECT_CALL(*m_pMockVideoNego, GetNegotiatedDirection()).WillOnce(Return(MEDIA_DIRECTION_SEND));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedVideoDirection(), MEDIA_DIRECTION_SEND);

    m_objMediaNego.SetVideoNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedVideoDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testGetNegotiatedTextDirection)
{
    EXPECT_CALL(*m_pMockTextNego, GetNegotiatedDirection()).WillOnce(Return(MEDIA_DIRECTION_SEND));
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextDirection(), MEDIA_DIRECTION_SEND);

    m_objMediaNego.SetTextNego(IMS_NULL);
    EXPECT_EQ(m_objMediaNego.GetNegotiatedTextDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoTest, testGetMediaDescriptor)
{
    MockIMedia objIMedia;
    MockIMedia objIMediaProposal;
    MockIMediaDescriptor objIMediaDescriptor;
    MockIMediaDescriptor objIMediaDescriptorProposal;

    ON_CALL(objIMedia, GetMediaDescriptor()).WillByDefault(Return(&objIMediaDescriptor));
    EXPECT_EQ(m_objMediaNego.GetMediaDescriptor(&objIMedia), &objIMediaDescriptor);

    ON_CALL(objIMedia, GetUpdateState()).WillByDefault(Return(IMedia::UPDATE_MODIFIED));
    ON_CALL(objIMedia, GetProposal()).WillByDefault(Return(&objIMediaProposal));
    ON_CALL(objIMediaProposal, GetMediaDescriptor())
            .WillByDefault(Return(&objIMediaDescriptorProposal));
    EXPECT_EQ(m_objMediaNego.GetMediaDescriptor(&objIMedia), &objIMediaDescriptorProposal);

    ON_CALL(objIMedia, GetProposal()).WillByDefault(Return(IMS_NULL));
    EXPECT_EQ(m_objMediaNego.GetMediaDescriptor(&objIMedia), IMS_NULL);

    EXPECT_EQ(m_objMediaNego.GetMediaDescriptor(IMS_NULL), IMS_NULL);
}

TEST_F(MediaNegoTest, testNegotiateSdpWhenPreviewModeAndNegotiated)
{
    SetupProfileExpectation();
    EXPECT_TRUE(m_objMediaNego.CreateProfile(m_pMediaEnvironment));

    MediaNego::MediaNegoResult errorReason;
    IMS_SINT32 nAudioDirection;
    IMS_SINT32 nVideoDirection;
    IMS_SINT32 nTextDirection;

    // success case
    MockIMedia objIMedia;
    MockIMediaDescriptor objIMediaDescriptor;
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    ImsList<IMedia*> objMediaList;
    SetUpMedia(objIMedia, objIMediaDescriptor, objSdpMedia, objMediaList);

    ON_CALL(objIMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(m_objRemoteIpAddress));
    ON_CALL(objIMediaDescriptor, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioNego, GetRemotePort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioNego, GetLocalPort()).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioNego, GetNegotiatedCodec()).WillByDefault(Return(AUDIO_CODEC_AMR));
    EXPECT_CALL(*m_pMockAudioNego, NegotiateSdp(_, _, _, _))
            .WillOnce(DoAll(SetArgReferee<3>(MEDIA_DIRECTION_SEND), Return()));

    m_objMediaNego.SetNegoState(STATE_NEGOTIATED);
    m_objMediaNego.SetPreviewMode(IMS_TRUE);
    ON_CALL(m_objIsession, IsSdpOaInPreviewMode()).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_objMediaNego.NegotiateSdp(
            &m_objIsession, nAudioDirection, nVideoDirection, nTextDirection, errorReason));
    EXPECT_EQ(errorReason, MediaNego::NO_ERROR);
    EXPECT_EQ(m_objMediaNego.GetNegoState(), STATE_NEGOTIATED);
    EXPECT_TRUE(m_objMediaNego.IsPreviewMode());
}
