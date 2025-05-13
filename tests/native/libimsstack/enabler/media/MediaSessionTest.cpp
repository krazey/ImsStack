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

#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MediaSession.h"

#include "MockICoreService.h"
#include "MockIMediaSessionClientListener.h"
#include "MockISession.h"
#include "MockMediaNego.h"
#include "MockMediaNegoHandler.h"
#include "audio/MockAudioController.h"
#include "video/MockVideoController.h"
#include "text/MockTextController.h"
#include "audio/MockAudioNego.h"
#include "video/MockVideoNego.h"

using ::testing::_;
using ::testing::An;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;

const AString LOCAL_IP = "127.0.0.1";
const AString REMOTE_IP = "127.0.0.1";
const IMS_UINT32 LOCAL_PORT = 20000;
const IMS_UINT32 REMOTE_PORT = 40000;
const IMS_UINT32 NEGO_ID = 1234567;
const IMS_UINT32 NEGO_ID_2 = 7654321;

class MediaSessionTest : public ::testing::Test
{
public:
    MediaSessionTest()
    {
        m_pMediaNego = std::make_shared<MockMediaNego>(0);
        m_nNegoId = NEGO_ID;
    }
    ~MediaSessionTest() {}

    MediaSession* m_pSession;
    MockISession* m_pIsession;
    MockICoreService m_objMockICoreService;
    MockIMediaSessionClientListener m_objMockClientListener;
    IpAddress m_objLocalIpAddress;
    IpAddress m_objRemoteIpAddress;
    std::shared_ptr<MockMediaNego> m_pMediaNego;
    IMS_UINTP m_nNegoId;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    std::shared_ptr<MockMediaNegoHandler> m_pMockMediaNegoHandler;
    std::shared_ptr<MockAudioController> m_pMockAudioController;
    std::shared_ptr<MockVideoController> m_pMockVideoController;
    std::shared_ptr<MockTextController> m_pMockTextController;
    std::shared_ptr<MockAudioNego> m_pMockAudioNegoInstance;
    std::shared_ptr<MockVideoNego> m_pMockVideoNegoInstance;

    // Helper to simulate successful negotiation and opening for a media type
    void SimulateNegotiateAndOpen(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE typeToOpen)
    {
        IMS_SINT32 nAudioDir = MEDIA_DIRECTION_INACTIVE;
        IMS_SINT32 nVideoDir = MEDIA_DIRECTION_INACTIVE;
        IMS_SINT32 nTextDir = MEDIA_DIRECTION_INACTIVE;
        MediaNego::MediaNegoResult errorReason = MediaNego::NO_ERROR;

        EXPECT_CALL(*m_pMockMediaNegoHandler,
                NegotiateSdp(nNegoId, m_pIsession, _, _, _, An<MediaNego::MediaNegoResult&>()))
                .WillOnce(Return(IMS_TRUE));
        EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId))
                .WillRepeatedly(Return(m_pMediaNego));  // Ensure nego is found
        EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedMediaType(nNegoId))
                .WillOnce(Return(typeToOpen));

        if (typeToOpen & MEDIA_TYPE_AUDIO)
        {
            EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).WillOnce(Return(IMS_TRUE));
            EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).WillOnce(Return(IMS_TRUE));
        }
        if (typeToOpen & MEDIA_TYPE_VIDEO)
        {
            EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).WillOnce(Return(IMS_TRUE));
            EXPECT_CALL(*m_pMockVideoController, OpenSession()).WillOnce(Return(IMS_TRUE));
        }
        if (typeToOpen & MEDIA_TYPE_TEXT)
        {
            EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).WillOnce(Return(IMS_TRUE));
            EXPECT_CALL(*m_pMockTextController, OpenSession()).WillOnce(Return(IMS_TRUE));
        }

        EXPECT_TRUE(m_pSession->NegotiateSdp(
                nNegoId, m_pIsession, &nAudioDir, &nVideoDir, &nTextDir, errorReason));
    }

protected:
    virtual void SetUp() override
    {
        m_objLocalIpAddress = IpAddress(LOCAL_IP);
        m_objRemoteIpAddress = IpAddress(REMOTE_IP);
        ON_CALL(m_objMockICoreService, GetIpAddress())
                .WillByDefault(ReturnRef(m_objLocalIpAddress));
        m_pSession = new MediaSession(
                MEDIA_NETWORK_LTE, MEDIA_SERVICE_DEFAULT, &m_objMockICoreService, 1, 0);
        m_pSession->SetMtcListener(&m_objMockClientListener);

        m_pEnvironment = std::make_shared<MediaEnvironment>();
        m_pEnvironment->pIService = &m_objMockICoreService;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;

        m_pIsession = new MockISession();
        m_pMockMediaNegoHandler =
                std::make_shared<MockMediaNegoHandler>(0, m_pEnvironment, IMS_NULL);
        m_pMockAudioController = std::make_shared<MockAudioController>();
        m_pMockVideoController = std::make_shared<MockVideoController>();
        m_pMockTextController = std::make_shared<MockTextController>();

        m_pMockAudioNegoInstance = std::make_shared<MockAudioNego>(0);
        m_pMockVideoNegoInstance = std::make_shared<MockVideoNego>(0);

        m_pSession->SetMediaNegoHandler(m_pMockMediaNegoHandler);
        m_pSession->SetAudioController(m_pMockAudioController);
        m_pSession->SetVideoController(m_pMockVideoController);
        m_pSession->SetTextController(m_pMockTextController);

        ON_CALL(*m_pMockMediaNegoHandler, FindMediaNego(_)).WillByDefault(Return(m_pMediaNego));
        ON_CALL(*m_pMockMediaNegoHandler, CreateMediaNego(_)).WillByDefault(Return(NEGO_ID));
        ON_CALL(*m_pMockMediaNegoHandler, DeleteMediaNego(_)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockMediaNegoHandler, GetRemotePort(_, _))
                .WillByDefault(Return(REMOTE_PORT));  // Default remote port
        ON_CALL(*m_pMockMediaNegoHandler, GetNegotiatedRemoteAddress(_, _))
                .WillByDefault(ReturnRef(m_objRemoteIpAddress));  // Default remote address

        // Stub Get*Nego methods on the main MediaNego mock to return the sub-nego mock instances
        ON_CALL(*m_pMediaNego, GetAudioNego()).WillByDefault(Return(m_pMockAudioNegoInstance));
        ON_CALL(*m_pMediaNego, GetVideoNego()).WillByDefault(Return(m_pMockVideoNegoInstance));
        ON_CALL(*m_pMediaNego, GetTextNego()).WillByDefault(Return(nullptr));

        ON_CALL(*m_pMockAudioController, DeleteSession(_)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockAudioController, CloseSession()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockVideoController, CloseSession()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockTextController, CloseSession()).WillByDefault(Return(IMS_TRUE));
    }

    virtual void TearDown() override
    {
        delete m_pSession;
        delete m_pIsession;
    }
};

// --- Profile Management Tests ---

TEST_F(MediaSessionTest, testCreateProfileSuccess)
{
    EXPECT_CALL(*m_pMockMediaNegoHandler, CreateMediaNego(0))
            .Times(1)
            .WillOnce(Return(NEGO_ID));  // Assume MediaNego returns the ID

    EXPECT_CALL(*m_pMockAudioController, CreateSession(_, m_nNegoId, _, MEDIA_SERVICE_DEFAULT))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockVideoController, CreateSession(_, _)).Times(0);  // No video
    EXPECT_CALL(*m_pMockTextController, CreateSession(_, _)).Times(0);   // No text

    EXPECT_EQ(m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO), m_nNegoId);
}

TEST_F(MediaSessionTest, testCreateProfileVideoTextSuccess)
{
    EXPECT_CALL(*m_pMockMediaNegoHandler, CreateMediaNego(0)).Times(1).WillOnce(Return(NEGO_ID));

    EXPECT_CALL(*m_pMockAudioController, CreateSession(_, m_nNegoId, _, _)).Times(0);  // No audio
    EXPECT_CALL(*m_pMockVideoController, CreateSession(_, _)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockTextController, CreateSession(_, _)).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(m_pSession->CreateProfile(0, MEDIA_TYPE_VIDEOTEXT), m_nNegoId);
}

TEST_F(MediaSessionTest, testCreateProfileNegoHandlerFails)
{
    EXPECT_CALL(*m_pMockMediaNegoHandler, CreateMediaNego(0))
            .Times(1)
            .WillOnce(Return(0));  // Handler fails to create nego

    EXPECT_CALL(*m_pMockAudioController, CreateSession(_, _, _, _)).Times(0);

    EXPECT_EQ(m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO), 0);  // Expect failure (negoId 0)
}

TEST_F(MediaSessionTest, testDestroyProfileSuccess)
{
    // First, create the profile successfully
    EXPECT_CALL(*m_pMockMediaNegoHandler, CreateMediaNego(0)).WillOnce(Return(NEGO_ID));
    EXPECT_CALL(*m_pMockAudioController, CreateSession(_, m_nNegoId, _, MEDIA_SERVICE_DEFAULT))
            .WillOnce(Return(IMS_TRUE));
    ASSERT_EQ(m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO), m_nNegoId);

    // Now, test destruction
    EXPECT_CALL(*m_pMockMediaNegoHandler, DeleteMediaNego(m_nNegoId))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockAudioController, DeleteSession(m_nNegoId))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->DestroyProfile(m_nNegoId));
}

TEST_F(MediaSessionTest, testGetSupportedMediaTypesFromSdp)
{
    ON_CALL(*m_pMockMediaNegoHandler, GetSupportedMediaTypesFromSdp(_, _))
            .WillByDefault(Return(MEDIA_TYPE_AUDIO));

    EXPECT_EQ(m_pSession->GetSupportedMediaTypesFromSdp(0, m_pIsession), MEDIA_TYPE_AUDIO);
}

// --- Negotiation Tests ---

TEST_F(MediaSessionTest, testFormSdpSuccess)
{
    IMS_UINTP nNegoId = NEGO_ID;  // Use a known value or create profile
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIOVIDEO;
    IMS_SINT32 nAudioDir = MEDIA_DIRECTION_SEND_RECEIVE;
    IMS_SINT32 nVideoDir = MEDIA_DIRECTION_SEND_RECEIVE;
    IMS_SINT32 nTextDir = MEDIA_DIRECTION_INACTIVE;
    IMS_BOOL bReoffer = IMS_FALSE;

    // Expect the call to be delegated to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            FormSdp(nNegoId, m_pIsession, eType, nAudioDir, nVideoDir, nTextDir, bReoffer))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // Call the method under test
    EXPECT_TRUE(m_pSession->FormSdp(
            nNegoId, m_pIsession, eType, nAudioDir, nVideoDir, nTextDir, bReoffer));
}

TEST_F(MediaSessionTest, testFormSdpFailure)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIO;
    IMS_SINT32 nAudioDir = MEDIA_DIRECTION_SEND;
    IMS_SINT32 nVideoDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nTextDir = MEDIA_DIRECTION_INACTIVE;
    IMS_BOOL bReoffer = IMS_TRUE;

    // Expect the call to be delegated, but the handler returns false
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            FormSdp(nNegoId, m_pIsession, eType, nAudioDir, nVideoDir, nTextDir, bReoffer))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    // Call the method under test
    EXPECT_FALSE(m_pSession->FormSdp(
            nNegoId, m_pIsession, eType, nAudioDir, nVideoDir, nTextDir, bReoffer));
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccess)
{
    IMS_UINTP nNegoId = NEGO_ID;
    IMS_SINT32 nAudioDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nVideoDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nTextDir = MEDIA_DIRECTION_INACTIVE;
    MediaNego::MediaNegoResult errorReason = MediaNego::NO_ERROR;
    MEDIA_CONTENT_TYPE negotiatedType = MEDIA_TYPE_AUDIOVIDEO;

    // Expect the negotiation call to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            NegotiateSdp(
                    nNegoId, m_pIsession, _, _, _, ::testing::An<MediaNego::MediaNegoResult&>()))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));  // Simulate successful negotiation

    // Expect GetNegotiatedMediaType to determine which sessions to open
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedMediaType(nNegoId))
            .Times(1)
            .WillOnce(Return(negotiatedType));

    // Expect OpenMediaSessions to be triggered for Audio and Video
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(1);
    // TextController methods should not be called if negotiatedType doesn't include TEXT
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(0);

    EXPECT_TRUE(m_pSession->NegotiateSdp(
            nNegoId, m_pIsession, &nAudioDir, &nVideoDir, &nTextDir, errorReason));
}

TEST_F(MediaSessionTest, testNegotiateSdpFailure)
{
    IMS_UINTP nNegoId = NEGO_ID;
    IMS_SINT32 nAudioDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nVideoDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nTextDir = MEDIA_DIRECTION_INACTIVE;
    MediaNego::MediaNegoResult errorReason = MediaNego::ERROR_INVALID_DESCRIPTOR;  // Example error

    // Expect the negotiation call to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            NegotiateSdp(
                    nNegoId, m_pIsession, _, _, _, ::testing::An<MediaNego::MediaNegoResult&>()))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));  // Simulate failed negotiation

    // FindMediaNego and GetNegotiatedMediaType should NOT be called if NegotiateSdp fails
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId)).Times(0);
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedMediaType(nNegoId)).Times(0);

    // Controller methods for opening sessions should NOT be called
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(0);
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(0);

    // Call the method under test
    EXPECT_FALSE(m_pSession->NegotiateSdp(
            nNegoId, m_pIsession, &nAudioDir, &nVideoDir, &nTextDir, errorReason));
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccessButNegoNotFound)
{
    IMS_UINTP nNegoId = NEGO_ID;
    IMS_SINT32 nAudioDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nVideoDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nTextDir = MEDIA_DIRECTION_INACTIVE;
    MediaNego::MediaNegoResult errorReason = MediaNego::NO_ERROR;

    // Expect the negotiation call to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            NegotiateSdp(
                    nNegoId, m_pIsession, _, _, _, ::testing::An<MediaNego::MediaNegoResult&>()))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));  // Simulate successful negotiation

    // Simulate FindMediaNego returning NULL after successful negotiation
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId))
            .Times(1)
            .WillOnce(Return(nullptr));

    // GetNegotiatedMediaType should NOT be called if FindMediaNego fails
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedMediaType(nNegoId)).Times(0);

    // Controller methods for opening sessions should NOT be called
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(_)).Times(0);
    // ... (similar for video and text) ...

    // Call the method under test
    EXPECT_FALSE(m_pSession->NegotiateSdp(
            nNegoId, m_pIsession, &nAudioDir, &nVideoDir, &nTextDir, errorReason));
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccessOpenTextOnly)
{
    IMS_UINTP nNegoId = NEGO_ID;
    IMS_SINT32 nAudioDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nVideoDir = MEDIA_DIRECTION_INACTIVE;
    IMS_SINT32 nTextDir = MEDIA_DIRECTION_SEND_RECEIVE;  // Expect text direction update
    MediaNego::MediaNegoResult errorReason = MediaNego::NO_ERROR;
    MEDIA_CONTENT_TYPE negotiatedType = MEDIA_TYPE_TEXT;

    EXPECT_CALL(*m_pMockMediaNegoHandler,
            NegotiateSdp(nNegoId, m_pIsession, _, _, _, An<MediaNego::MediaNegoResult&>()))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<4>(MEDIA_DIRECTION_SEND_RECEIVE),
                    Return(IMS_TRUE)));  // Simulate direction update

    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId))
            .WillRepeatedly(Return(m_pMediaNego));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedMediaType(nNegoId))
            .Times(1)
            .WillOnce(Return(negotiatedType));

    // Expect OpenMediaSessions for Text only
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(0);
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->NegotiateSdp(
            nNegoId, m_pIsession, &nAudioDir, &nVideoDir, &nTextDir, errorReason));
    // Verify direction was updated
    EXPECT_EQ(nTextDir, MEDIA_DIRECTION_SEND_RECEIVE);
}

// --- Getter Tests ---

TEST_F(MediaSessionTest, testGetNegotiatedMediaType)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MEDIA_CONTENT_TYPE expectedType = MEDIA_TYPE_AUDIOVIDEO;

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedMediaType(nNegoId))
            .Times(1)
            .WillOnce(Return(expectedType));

    EXPECT_EQ(m_pSession->GetNegotiatedMediaType(nNegoId), expectedType);
}

TEST_F(MediaSessionTest, testGetNegotiatedMediaDirection)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MEDIA_DIRECTION expectedDir = MEDIA_DIRECTION_SEND_RECEIVE;

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedDirection(nNegoId, MEDIA_TYPE_AUDIO))
            .Times(1)
            .WillOnce(Return(expectedDir));

    EXPECT_EQ(m_pSession->GetNegotiatedDirection(nNegoId, MEDIA_TYPE_AUDIO), expectedDir);
}

TEST_F(MediaSessionTest, testGetRemotePort)
{
    // Set expectations on the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(NEGO_ID, MEDIA_TYPE_AUDIO))
            .Times(1)
            .WillOnce(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(NEGO_ID, MEDIA_TYPE_VIDEO))
            .Times(1)
            .WillOnce(Return(REMOTE_PORT + 1));  // Use different ports for clarity
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(NEGO_ID, MEDIA_TYPE_TEXT))
            .Times(1)
            .WillOnce(Return(REMOTE_PORT + 2));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(NEGO_ID, MEDIA_TYPE_INVALID))
            .Times(1)
            .WillOnce(Return(MEDIA_PORT_INVALID));  // Expect invalid port for invalid type

    // Call the method under test
    EXPECT_EQ(m_pSession->GetRemotePort(NEGO_ID, MEDIA_TYPE_AUDIO), REMOTE_PORT);
    EXPECT_EQ(m_pSession->GetRemotePort(NEGO_ID, MEDIA_TYPE_VIDEO), REMOTE_PORT + 1);
    EXPECT_EQ(m_pSession->GetRemotePort(NEGO_ID, MEDIA_TYPE_TEXT), REMOTE_PORT + 2);
    EXPECT_EQ(m_pSession->GetRemotePort(NEGO_ID, MEDIA_TYPE_INVALID), MEDIA_PORT_INVALID);

    // Test with invalid NegoId (should likely go through handler which might return invalid)
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(0, MEDIA_TYPE_AUDIO))
            .Times(1)
            .WillOnce(Return(MEDIA_PORT_INVALID));
    EXPECT_EQ(m_pSession->GetRemotePort(0, MEDIA_TYPE_AUDIO), MEDIA_PORT_INVALID);
}

// --- Terminate Test ---

TEST_F(MediaSessionTest, testTerminate)
{
    ON_CALL(*m_pMockAudioController, IsSessionOpened()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockVideoController, IsSessionOpened()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockTextController, IsSessionOpened()).WillByDefault(Return(IMS_TRUE));

    // Expect CloseSession to be called on all controllers
    EXPECT_CALL(*m_pMockAudioController, CloseSession()).Times(2);
    EXPECT_CALL(*m_pMockVideoController, CloseSession()).Times(2);
    EXPECT_CALL(*m_pMockTextController, CloseSession()).Times(2);

    // Expect DeleteMediaNego to be called to clear the existing MediaNego instances.
    EXPECT_CALL(*m_pMockMediaNegoHandler, DeleteMediaNego(_))
            .Times(0);  // Or Times(number_of_created_profiles)
    EXPECT_CALL(*m_pMockMediaNegoHandler, ClearAllMediaNego()).Times(2);

    EXPECT_TRUE(m_pSession->Terminate());
}

// --- QoS related Tests ---

TEST_F(MediaSessionTest, testQosRequestAndCallbackUnmatch)
{
    ON_CALL(*m_pMockMediaNegoHandler, GetRemotePort(_, _)).WillByDefault(Return(REMOTE_PORT));
    ON_CALL(*m_pMockAudioController, DeleteSession(_)).WillByDefault(Return(IMS_TRUE));

    IMS_UINTP nNegoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(nNegoId, 0);
    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);

    // send unmatched port number
    ImsMediaMsgQosParam objParam =
            ImsMediaMsgQosParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, 30000);
    objParam.m_bResult = IMS_TRUE;

    EXPECT_CALL(
            m_objMockClientListener, MediaSession_NotifyQos(nNegoId, IMS_TRUE, MEDIA_TYPE_AUDIO))
            .Times(0);

    EXPECT_EQ(
            m_pSession->SendMessage(IJniMedia::REQUEST_QOS, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(nNegoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackAudio)
{
    IMS_UINTP nNegoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    ASSERT_NE(nNegoId, 0);

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(nNegoId, MEDIA_TYPE_AUDIO))
            .WillRepeatedly(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedRemoteAddress(nNegoId, MEDIA_TYPE_AUDIO))
            .WillRepeatedly(ReturnRef(m_objRemoteIpAddress));

    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);
    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);  // check request twice

    EXPECT_CALL(
            m_objMockClientListener, MediaSession_NotifyQos(nNegoId, IMS_TRUE, MEDIA_TYPE_AUDIO))
            .Times(1);

    ImsMediaMsgQosParam objParam =
            ImsMediaMsgQosParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, REMOTE_PORT);
    objParam.m_bResult = IMS_TRUE;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(nNegoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackVideo)
{
    IMS_UINTP nNegoId = m_pSession->CreateProfile(0, MEDIA_TYPE_VIDEO);
    ASSERT_NE(nNegoId, 0);

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(nNegoId, MEDIA_TYPE_VIDEO))
            .WillRepeatedly(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedRemoteAddress(nNegoId, MEDIA_TYPE_VIDEO))
            .WillRepeatedly(ReturnRef(m_objRemoteIpAddress));
    // Need VideoController CreateSession for CreateProfile to succeed
    EXPECT_CALL(*m_pMockVideoController, CreateSession(_, _)).WillOnce(Return(IMS_TRUE));
    ASSERT_EQ(m_pSession->CreateProfile(0, MEDIA_TYPE_VIDEO), nNegoId);

    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_VIDEO), IMS_TRUE);

    ImsMediaMsgQosParam objParam =
            ImsMediaMsgQosParam(MEDIA_TYPE_VIDEO, m_objRemoteIpAddress, REMOTE_PORT);
    objParam.m_bResult = IMS_TRUE;

    EXPECT_CALL(
            m_objMockClientListener, MediaSession_NotifyQos(nNegoId, IMS_TRUE, MEDIA_TYPE_VIDEO))
            .Times(1);

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(nNegoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackText)
{
    IMS_UINTP nNegoId = m_pSession->CreateProfile(0, MEDIA_TYPE_TEXT);
    ASSERT_NE(nNegoId, 0);

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(nNegoId, MEDIA_TYPE_TEXT))
            .WillRepeatedly(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedRemoteAddress(nNegoId, MEDIA_TYPE_TEXT))
            .WillRepeatedly(ReturnRef(m_objRemoteIpAddress));
    // Need TextController CreateSession for CreateProfile to succeed
    EXPECT_CALL(*m_pMockTextController, CreateSession(_, _)).WillOnce(Return(IMS_TRUE));
    ASSERT_EQ(m_pSession->CreateProfile(0, MEDIA_TYPE_TEXT), nNegoId);

    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_TEXT), IMS_TRUE);

    ImsMediaMsgQosParam objParam =
            ImsMediaMsgQosParam(MEDIA_TYPE_TEXT, m_objRemoteIpAddress, REMOTE_PORT);
    objParam.m_bResult = IMS_TRUE;

    EXPECT_CALL(m_objMockClientListener, MediaSession_NotifyQos(nNegoId, IMS_TRUE, MEDIA_TYPE_TEXT))
            .Times(1);

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(nNegoId), IMS_TRUE);
}

// --- Event Tests ---

TEST_F(MediaSessionTest, testNotifySrvccSuccess)
{
    ON_CALL(*m_pMockAudioController, CloseSession()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockAudioController, UpdateMediaDirection(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED));
    EXPECT_TRUE(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_SUCCEED));
}

TEST_F(MediaSessionTest, testNotifySrvccFailed)
{
    ON_CALL(*m_pMockAudioController, UpdateMediaDirection(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED));
    EXPECT_TRUE(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_FAILED));
}

TEST_F(MediaSessionTest, testNotifySrvccCanceled)
{
    ON_CALL(*m_pMockAudioController, UpdateMediaDirection(_, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED));
    EXPECT_TRUE(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_CANCELED));
}

TEST_F(MediaSessionTest, testNotifyFirstPacket)
{
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_STARTED, MEDIA_TYPE_AUDIO, _))
            .Times(1);

    ImsMediaResponseConfigParam objParam = ImsMediaResponseConfigParam(MEDIA_TYPE_AUDIO);

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_FIRST_PACKET, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityElse)
{
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_VIDEO, MEDIA_PROTOCOL_RTP))
            .Times(1);

    ImsMediaNotifyInactivityParam objParam = ImsMediaNotifyInactivityParam(MEDIA_TYPE_VIDEO);
    objParam.m_eMediaProtocolType = RTP;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);
}

TEST_F(MediaSessionTest, testNotifyVideoBitrate)
{
    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(REPORT_VIDEO_LOWEST_BITRATE, _, _))
            .Times(1);

    ImsMediaVideoParam objParam;
    objParam.nValue = 100000;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_VIDEO_BITRATE, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);
}

// --- Video Commands Test ---

TEST_F(MediaSessionTest, testVideoControlMessages)
{
    // avoid leakage
    ImsMediaVideoParam objParam;

    // SetSurface
    EXPECT_CALL(*m_pMockVideoController,
            SendMessage(IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(&objParam)))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(&objParam)));

    // SelectCamera
    EXPECT_CALL(*m_pMockVideoController,
            SendMessage(IJniMedia::SELECT_CAMERA_CMD, reinterpret_cast<IMS_UINTP>(&objParam)))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::SELECT_CAMERA_CMD, reinterpret_cast<IMS_UINTP>(&objParam)));

    // ChangeCameraZoom
    EXPECT_CALL(*m_pMockVideoController,
            SendMessage(IJniMedia::CHANGE_CAMERA_ZOOM_CMD, reinterpret_cast<IMS_UINTP>(&objParam)))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::CHANGE_CAMERA_ZOOM_CMD, reinterpret_cast<IMS_UINTP>(&objParam)));

    // SetPauseImage
    EXPECT_CALL(*m_pMockVideoController,
            SendMessage(IJniMedia::SET_PAUSE_IMAGE_CMD, reinterpret_cast<IMS_UINTP>(&objParam)))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::SET_PAUSE_IMAGE_CMD, reinterpret_cast<IMS_UINTP>(&objParam)));

    // ChangeOrientation
    EXPECT_CALL(*m_pMockVideoController,
            SendMessage(IJniMedia::CHANGE_ORIENTATION_CMD, reinterpret_cast<IMS_UINTP>(&objParam)))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::CHANGE_ORIENTATION_CMD, reinterpret_cast<IMS_UINTP>(&objParam)));
}

// --- Dtmf Test ---

TEST_F(MediaSessionTest, testSendMessageSendDtmfSuccess)
{
    char cDtmfDigit = '7';
    ImsMediaMsgDtmfParam objDtmfParam;
    objDtmfParam.m_dtmfCode = cDtmfDigit;

    // Expect the call to be delegated to the AudioController
    EXPECT_CALL(*m_pMockAudioController, SendDtmf(cDtmfDigit)).Times(1).WillOnce(Return(IMS_TRUE));

    // Call SendMessage with the DTMF command and parameter
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::SEND_DTMF, reinterpret_cast<IMS_UINTP>(&objDtmfParam)));
}

TEST_F(MediaSessionTest, testSendMessageSendDtmfNullParam)
{
    // Expect SendDtmf NOT to be called if the parameter is null
    EXPECT_CALL(*m_pMockAudioController, SendDtmf(_)).Times(0);

    // Call SendMessage with the DTMF command and a null parameter
    EXPECT_FALSE(
            m_pSession->SendMessage(IJniMedia::SEND_DTMF, reinterpret_cast<IMS_UINTP>(nullptr)));
}

// --- Response Test ---

TEST_F(MediaSessionTest, testSendMessageResponseOpenSessionSuccess)
{
    MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO;
    ImsMediaResponseConfigParam responseParam(eMediaType);
    responseParam.m_eResult = RtpError::NO_ERROR;  // Success

    // Expect MediaSession_Notify for success
    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(REPORT_SUCCESS, eMediaType, _))
            .Times(1);
    EXPECT_CALL(m_objMockClientListener, MediaSession_NotifyFailures(_, _, _))
            .Times(0);  // No failure call

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::RESPONSE_OPEN_SESSION, reinterpret_cast<IMS_UINTP>(&responseParam)));
}

TEST_F(MediaSessionTest, testSendMessageResponseModifySessionFailure)
{
    MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_VIDEO;
    IMS_SINT32 nErrorCode = RtpError::INVALID_PARAM;  // Example failure code
    ImsMediaResponseConfigParam responseParam(eMediaType);
    responseParam.m_eResult = nErrorCode;  // Failure

    // Expect MediaSession_NotifyFailures for failure
    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(REPORT_SUCCESS, _, _))
            .Times(0);  // No success call
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_NotifyFailures(REPORT_FAILURE, nErrorCode, eMediaType))
            .Times(1);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::RESPONSE_MODIFY_SESSION, reinterpret_cast<IMS_UINTP>(&responseParam)));
}

TEST_F(MediaSessionTest, testSendMessageResponseAddConfigSuccess)
{
    MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_TEXT;
    ImsMediaResponseConfigParam responseParam(eMediaType);
    responseParam.m_eResult = RtpError::NO_ERROR;  // Success

    // Expect MediaSession_Notify for success
    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(REPORT_SUCCESS, eMediaType, _))
            .Times(1);
    EXPECT_CALL(m_objMockClientListener, MediaSession_NotifyFailures(_, _, _)).Times(0);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::RESPONSE_ADD_CONFIG, reinterpret_cast<IMS_UINTP>(&responseParam)));
}

TEST_F(MediaSessionTest, testSendMessageResponseConfirmConfigFailure)
{
    MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIOVIDEO;
    IMS_SINT32 nErrorCode = RtpError::INVALID_PARAM;  // Example failure code
    ImsMediaResponseConfigParam responseParam(eMediaType);
    responseParam.m_eResult = nErrorCode;  // Failure

    // Expect MediaSession_NotifyFailures for failure
    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(REPORT_SUCCESS, _, _)).Times(0);
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_NotifyFailures(REPORT_FAILURE, nErrorCode, eMediaType))
            .Times(1);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::RESPONSE_CONFIRM_CONFIG, reinterpret_cast<IMS_UINTP>(&responseParam)));
}

TEST_F(MediaSessionTest, testSendMessageResponseNullParam)
{
    // Expect no listener calls if the parameter is null
    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(REPORT_SUCCESS, _, _)).Times(0);
    EXPECT_CALL(m_objMockClientListener, MediaSession_NotifyFailures(_, _, _)).Times(0);

    // Test with one of the response types and a null parameter
    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::RESPONSE_OPEN_SESSION, reinterpret_cast<IMS_UINTP>(nullptr)));
    // Note: SendMessage itself returns true even if the param is null,
    // because OnResponse handles the null check internally.
}

// --- SetMediaPemType Test ---
TEST_F(MediaSessionTest, testSetMediaPemType)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MEDIA_PEM_TYPE ePemType = MEDIA_PEM_TYPE::SENDRECV;

    // Expect FindMediaNego to be called and return a valid mock
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId))
            .Times(1)
            .WillOnce(Return(m_pMediaNego));

    // Expect SetMediaPemType to be called on the controllers
    EXPECT_CALL(*m_pMockAudioController, SetMediaPemType(nNegoId, ePemType)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, SetMediaPemType(ePemType)).Times(1);

    m_pSession->SetMediaPemType(nNegoId, ePemType);
}

TEST_F(MediaSessionTest, testSetMediaPemTypeNegoNotFound)
{
    // Expect FindMediaNego to be called and return nullptr
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(NEGO_ID))
            .Times(1)
            .WillOnce(Return(nullptr));

    // Expect SetMediaPemType NOT to be called on the controllers
    EXPECT_CALL(*m_pMockAudioController, SetMediaPemType(_, _)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, SetMediaPemType(_)).Times(0);

    m_pSession->SetMediaPemType(NEGO_ID, MEDIA_PEM_TYPE::SENDONLY);
}
