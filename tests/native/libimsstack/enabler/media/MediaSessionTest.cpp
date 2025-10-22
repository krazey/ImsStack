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

#include "MediaSession.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MediaEnvironment.h"
#include "MockICoreService.h"
#include "MockIMediaSessionClientListener.h"
#include "MockISession.h"
#include "MockMediaNego.h"
#include "MockMediaNegoHandler.h"
#include "audio/MockAudioController.h"
#include "audio/MockAudioNego.h"
#include "text/MockTextController.h"
#include "video/MockVideoController.h"
#include "video/MockVideoNego.h"

using ::testing::_;
using ::testing::An;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;

const AString LOCAL_IP = "127.0.0.1";
const AString REMOTE_IP = "127.0.0.1";
const IMS_UINT32 REMOTE_PORT = 40000;
const IMS_UINT32 NEGO_ID = 1234567;

class MockMediaSession : public MediaSession
{
public:
    MockMediaSession(MEDIA_NETWORK_TYPE eNetwork, MEDIA_SERVICE_TYPE eServiceType,
            IService* pIService, IN IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId) :
            MediaSession(eNetwork, eServiceType, pIService, nCallKey, nSlotId)
    {
    }
    MOCK_METHOD(IMS_BOOL, MediaSession_SendMsgToMediaManager,
            (IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam), (override));
};

class MediaSessionTest : public ::testing::Test
{
public:
    MediaSessionTest() :
            m_pMediaNego(std::make_shared<MockMediaNego>(0)),
            m_nNegoId(NEGO_ID)
    {
    }
    ~MediaSessionTest() override {}

    std::unique_ptr<MockMediaSession> m_pSession;
    std::unique_ptr<MockISession> m_pIsession;
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

protected:
    virtual void SetUp() override
    {
        m_objLocalIpAddress = IpAddress(LOCAL_IP);
        m_objRemoteIpAddress = IpAddress(REMOTE_IP);
        ON_CALL(m_objMockICoreService, GetIpAddress())
                .WillByDefault(ReturnRef(m_objLocalIpAddress));
        m_pSession = std::make_unique<MockMediaSession>(
                MEDIA_NETWORK_LTE, MEDIA_SERVICE_DEFAULT, &m_objMockICoreService, 1, 0);
        m_pSession->SetMtcListener(&m_objMockClientListener);

        m_pEnvironment = std::make_shared<MediaEnvironment>();
        m_pEnvironment->pIService = &m_objMockICoreService;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;

        m_pIsession = std::make_unique<MockISession>();
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
        ON_CALL(*m_pMediaNego, GetTextNego()).WillByDefault(Return(IMS_NULL));

        ON_CALL(*m_pMockAudioController, DeleteSession(_)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockAudioController, CloseSession()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockVideoController, CloseSession()).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockTextController, CloseSession()).WillByDefault(Return(IMS_TRUE));
    }

    virtual void TearDown() override {}
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

    EXPECT_EQ(m_pSession->GetSupportedMediaTypesFromSdp(0, m_pIsession.get()), MEDIA_TYPE_AUDIO);
}

// --- Negotiation Tests ---

TEST_F(MediaSessionTest, testFormSdpSuccess)
{
    IMS_UINTP nNegoId = NEGO_ID;  // Use a known value or create profile
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIOVIDEO;
    MEDIA_DIRECTION eAudioDir = MEDIA_DIRECTION_SEND_RECEIVE;
    MEDIA_DIRECTION eVideoDir = MEDIA_DIRECTION_SEND_RECEIVE;
    MEDIA_DIRECTION eTextDir = MEDIA_DIRECTION_INACTIVE;
    IMS_BOOL bReoffer = IMS_FALSE;

    // Expect the call to be delegated to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            FormSdp(nNegoId, m_pIsession.get(), eType, eAudioDir, eVideoDir, eTextDir, bReoffer))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // Call the method under test
    EXPECT_TRUE(m_pSession->FormSdp(
            nNegoId, m_pIsession.get(), eType, eAudioDir, eVideoDir, eTextDir, bReoffer));
}

TEST_F(MediaSessionTest, testFormSdpSuccessVideoOpen)
{
    EXPECT_CALL(*m_pMockMediaNegoHandler, FormSdp(_, _, _, _, _, _, _)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(1);
    m_pSession->FormSdp(NEGO_ID, m_pIsession.get(), MEDIA_TYPE_VIDEO, MEDIA_DIRECTION_SEND_RECEIVE,
            MEDIA_DIRECTION_SEND_RECEIVE, MEDIA_DIRECTION_SEND_RECEIVE, false);
}

TEST_F(MediaSessionTest, testFormSdpClosesVideoSession)
{
    // Arrange: Simulate that the video session is one time opened.
    // This is necessary for CloseMediaSessions to actually attempt to close it.
    EXPECT_CALL(*m_pMockVideoController, IsSessionOpened())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    // Expect CloseSession to be called on the video controller.
    EXPECT_CALL(*m_pMockVideoController, CloseSession()).Times(1).WillOnce(Return(IMS_TRUE));

    // Act: Call FormSdp with a media type that does NOT include MEDIA_TYPE_VIDEO (e.g.,
    // MEDIA_TYPE_AUDIO).
    EXPECT_CALL(*m_pMockMediaNegoHandler, FormSdp(_, _, _, _, _, _, _)).WillOnce(Return(IMS_TRUE));
    m_pSession->FormSdp(NEGO_ID, m_pIsession.get(), MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND_RECEIVE,
            MEDIA_DIRECTION_INACTIVE, MEDIA_DIRECTION_INACTIVE, false);
}

TEST_F(MediaSessionTest, testFormSdpFailure)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIO;
    MEDIA_DIRECTION eAudioDir = MEDIA_DIRECTION_SEND;
    MEDIA_DIRECTION eVideoDir = MEDIA_DIRECTION_INACTIVE;
    MEDIA_DIRECTION eTextDir = MEDIA_DIRECTION_INACTIVE;
    IMS_BOOL bReoffer = IMS_TRUE;

    // Expect the call to be delegated, but the handler returns false
    EXPECT_CALL(*m_pMockMediaNegoHandler,
            FormSdp(nNegoId, m_pIsession.get(), eType, eAudioDir, eVideoDir, eTextDir, bReoffer))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    // Call the method under test
    EXPECT_FALSE(m_pSession->FormSdp(
            nNegoId, m_pIsession.get(), eType, eAudioDir, eVideoDir, eTextDir, bReoffer));
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccessAudioVideo)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIOVIDEO,
            MEDIA_DIRECTION_SEND_RECEIVE, MEDIA_DIRECTION_SEND_RECEIVE, MEDIA_DIRECTION_INACTIVE);

    // Expect the negotiation call to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));  // Simulate successful negotiation

    // Expect OpenMediaSessions to be triggered for Audio and Video
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(1);
    // TextController methods should not be called if eNegotiatedType doesn't include TEXT
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(0);

    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, expectedResult.eNegotiatedType);
    EXPECT_EQ(objResult.eResult, expectedResult.eResult);
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccessAudioText)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIOTEXT,
            MEDIA_DIRECTION_SEND_RECEIVE, MEDIA_DIRECTION_INACTIVE, MEDIA_DIRECTION_SEND_RECEIVE);

    // Expect the negotiation call to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));  // Simulate successful negotiation

    // Expect OpenMediaSessions to be triggered for Audio
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(1);
    // VideoController methods should not be called if eNegotiatedType not included Video
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(0);
    // TextController methods should be called if eNegotiatedType includes TEXT
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(1);

    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, expectedResult.eNegotiatedType);
    EXPECT_EQ(objResult.eResult, expectedResult.eResult);
}

TEST_F(MediaSessionTest, testNegotiateSdpFailure)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);

    // Expect the negotiation call to the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));  // Simulate failed negotiation

    // Controller methods for opening sessions should NOT be called
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(0);
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(0);

    // Call the method under test
    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, MEDIA_TYPE_INVALID);
    EXPECT_EQ(objResult.eResult, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccessButNegoNotFound)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIO);

    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));  // Simulate successful negotiation

    // Simulate FindMediaNego returning NULL after successful negotiation
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId))
            .Times(1)
            .WillOnce(Return(IMS_NULL));

    // Controller methods for opening sessions should NOT be called
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(_)).Times(0);

    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, MEDIA_TYPE_INVALID);
    EXPECT_EQ(objResult.eResult, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
}

TEST_F(MediaSessionTest, testNegotiateSdpSuccessOpenTextOnly)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_TEXT,
            MEDIA_DIRECTION_INACTIVE, MEDIA_DIRECTION_INACTIVE, MEDIA_DIRECTION_SEND_RECEIVE);

    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));

    // Expect OpenMediaSessions for Text only
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, UpdateLocalAddress(_)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, OpenSession()).Times(0);
    EXPECT_CALL(*m_pMockTextController, UpdateLocalAddress(_)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockTextController, OpenSession()).Times(1).WillOnce(Return(IMS_TRUE));

    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, expectedResult.eNegotiatedType);
    EXPECT_EQ(objResult.eTextDirection, MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(MediaSessionTest, testNegotiateSdpDowngradeToAudioOnlyFromAudioVideo)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIO);

    // Simulate successful negotiation
    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));

    // Expect OpenMediaSessions to be called for audio
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(1);

    // Video was active, so it should be closed.
    // IsSessionOpened() is called twice: once in NegotiateSdp (returns true)
    // and once in the destructor (returns false, as it's now closed).
    EXPECT_CALL(*m_pMockVideoController, IsSessionOpened())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*m_pMockVideoController, CloseSession()).Times(1).WillOnce(Return(IMS_TRUE));

    ON_CALL(*m_pMockTextController, IsSessionOpened()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(*m_pMockTextController, CloseSession()).Times(0);

    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, MEDIA_TYPE_AUDIO);
}

TEST_F(MediaSessionTest, testNegotiateSdpDowngradeToAudioOnlyFromAudioText)
{
    IMS_UINTP nNegoId = NEGO_ID;
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIO);

    // Simulate successful negotiation
    EXPECT_CALL(*m_pMockMediaNegoHandler, NegotiateSdp(nNegoId, m_pIsession.get()))
            .Times(1)
            .WillOnce(Return(expectedResult));

    // Expect OpenMediaSessions to be called for audio
    EXPECT_CALL(*m_pMockAudioController, UpdateLocalAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockAudioController, OpenSession(nNegoId)).Times(1);

    ON_CALL(*m_pMockVideoController, IsSessionOpened()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(*m_pMockVideoController, CloseSession()).Times(0);

    // Text was active, so it should be closed.
    EXPECT_CALL(*m_pMockTextController, IsSessionOpened())
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*m_pMockTextController, CloseSession()).Times(1).WillOnce(Return(IMS_TRUE));

    SdpNegotiationResult objResult = m_pSession->NegotiateSdp(nNegoId, m_pIsession.get());
    EXPECT_EQ(objResult.eNegotiatedType, MEDIA_TYPE_AUDIO);
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

TEST_F(MediaSessionTest, testRequestQosWhenAlreadyAcquiredNotifiesAsync)
{
    IMS_UINTP nNegoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    ASSERT_NE(nNegoId, 0);

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(nNegoId, MEDIA_TYPE_AUDIO))
            .WillRepeatedly(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedRemoteAddress(nNegoId, MEDIA_TYPE_AUDIO))
            .WillRepeatedly(ReturnRef(m_objRemoteIpAddress));

    EXPECT_CALL(*m_pSession, MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_QOS, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);

    // Simulate receiving the QoS success notification.
    EXPECT_CALL(
            m_objMockClientListener, MediaSession_NotifyQos(nNegoId, IMS_TRUE, MEDIA_TYPE_AUDIO))
            .Times(1);

    ImsMediaMsgQosParam objParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, REMOTE_PORT);
    objParam.m_bResult = IMS_TRUE;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(&objParam)),
            IMS_TRUE);

    // Act: Call RequestQos again. This should enter the 'if (pQosParams->m_bResult)' block.
    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(nNegoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testRequestQosWhenPendingResendsRequest)
{
    IMS_UINTP nNegoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    ASSERT_NE(nNegoId, 0);

    EXPECT_CALL(*m_pMockMediaNegoHandler, GetRemotePort(nNegoId, MEDIA_TYPE_AUDIO))
            .WillRepeatedly(Return(REMOTE_PORT));
    EXPECT_CALL(*m_pMockMediaNegoHandler, GetNegotiatedRemoteAddress(nNegoId, MEDIA_TYPE_AUDIO))
            .WillRepeatedly(ReturnRef(m_objRemoteIpAddress));

    // Expect two calls to send the REQUEST_QOS message to the manager.
    EXPECT_CALL(*m_pSession, MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_QOS, _))
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));

    // Act: First call adds the request. Second call finds the pending request and re-sends.
    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);
    EXPECT_EQ(m_pSession->RequestQos(nNegoId, MEDIA_TYPE_AUDIO), IMS_TRUE);
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

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_NetworkToneTimeout)
{
    const IMS_SINT32 kNetworkToneTimer = 1000;
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = kNetworkToneTimer;

    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kNetworkToneTimer));  // Corrected enum value

    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));

    EXPECT_CALL(*m_pMockAudioController, SetNetworkToneTimer(UNDEFINED_NEGO_ID, 0)).Times(1);
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(
                    REPORT_NW_TONE_RTP_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTP))
            .Times(1);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_RtpTimeout)
{
    const IMS_SINT32 kRtpTimer = 2000;
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = kRtpTimer;

    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));  // No network tone timer
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtpTimer));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));  // No rtcp timer

    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTP))
            .Times(1);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_RtcpTimeout)
{
    const IMS_SINT32 kRtcpTimer = 3000;
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtcpInactivityTimerMillis = kRtcpTimer;

    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));  // No network tone timer
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));  // No rtp timer
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtcpTimer));

    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTCP))
            .Times(1);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_NoTimeout)
{
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = 500;
    objParam.m_nRtcpInactivityTimerMillis = 500;

    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(1000));  // Timer not expired

    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));

    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(_, _, _)).Times(0);

    EXPECT_FALSE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_NoTimersSet)
{
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = 1000;
    objParam.m_nRtcpInactivityTimerMillis = 1000;

    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));

    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(_, _, _)).Times(0);

    EXPECT_FALSE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_RtpAndRtcpTimeout)
{
    const IMS_SINT32 kRtpTimer = 2000;
    const IMS_SINT32 kRtcpTimer = 3000;
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = kRtpTimer;
    objParam.m_nRtcpInactivityTimerMillis = kRtcpTimer;

    // The direction is NOT send-receive
    EXPECT_CALL(*m_pMockAudioController, GetMediaDirection())
            .WillOnce(Return(MEDIA_DIRECTION_SEND));
    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));  // No network tone timer
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtpTimer));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtcpTimer));

    // Only RTP timeout should be reported as it's checked first
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTP))
            .Times(1);
    // RTCP timeout should not be reported
    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTCP))
            .Times(0);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_ActiveDirection_RtpTimeout)
{
    const IMS_SINT32 kRtpTimer = 2000;
    const IMS_SINT32 kRtcpTimer = 3000;
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = kRtpTimer;
    objParam.m_nRtcpInactivityTimerMillis = kRtcpTimer;

    EXPECT_CALL(*m_pMockAudioController, GetMediaDirection())
            .WillOnce(Return(MEDIA_DIRECTION_SEND_RECEIVE));
    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));  // No network tone timer
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtpTimer));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtcpTimer));

    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTP))
            .Times(1);

    EXPECT_CALL(m_objMockClientListener,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTCP))
            .Times(0);

    EXPECT_TRUE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
}

TEST_F(MediaSessionTest, testNotifyMediaInactivityAudio_ActiveDirection_NoTimeout)
{
    const IMS_SINT32 kRtpTimer = 2000;
    ImsMediaNotifyQualityStatusParam objParam(MEDIA_TYPE_AUDIO);
    objParam.m_nRtpInactivityTimerMillis = kRtpTimer - 1;  // Timer not expired

    EXPECT_CALL(*m_pMockAudioController, GetMediaDirection())
            .WillOnce(Return(MEDIA_DIRECTION_SEND_RECEIVE));
    EXPECT_CALL(
            *m_pMockAudioController, GetInactivityTimer(NETWORK_TONE_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(0));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(kRtpTimer));
    EXPECT_CALL(*m_pMockAudioController, GetInactivityTimer(RTCP_INACTIVITY, UNDEFINED_NEGO_ID))
            .WillOnce(Return(3000));

    EXPECT_CALL(m_objMockClientListener, MediaSession_Notify(_, _, _)).Times(0);

    EXPECT_FALSE(m_pSession->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(&objParam)));
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
            m_pSession->SendMessage(IJniMedia::SEND_DTMF, reinterpret_cast<IMS_UINTP>(IMS_NULL)));
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
            IJniMedia::RESPONSE_OPEN_SESSION, reinterpret_cast<IMS_UINTP>(IMS_NULL)));
    // Note: SendMessage itself returns true even if the param is null,
    // because OnResponse handles the null check internally.
}

// --- SetOptions Tests ---

TEST_F(MediaSessionTest, testSetOptionsSetRtpPort)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO;
    IMS_SINT32 nPort = 12345;

    EXPECT_CALL(*m_pMockMediaNegoHandler, SetRtpPort(nNegoId, eMediaType, nPort))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    m_pSession->SetOptions(nNegoId, MediaSession::SET_RTP_PORT, eMediaType, nPort);
}

TEST_F(MediaSessionTest, testSetOptionsSetConfirmedSession)
{
    IMS_UINTP nNegoId = NEGO_ID;
    IMS_SINT32 nConfirmed = 1;

    // Expect FindMediaNego to be called and return a valid mock
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(nNegoId))
            .Times(1)
            .WillOnce(Return(m_pMediaNego));

    // Expect SetPreviewMode to be called on the MediaNego mock
    EXPECT_CALL(*m_pMediaNego, SetPreviewMode(IMS_FALSE)).Times(1);

    // Expect SetCallSessionState to be called on the controllers
    EXPECT_CALL(*m_pMockAudioController, SetCallSessionState(nConfirmed)).Times(1);
    EXPECT_CALL(*m_pMockVideoController, SetCallSessionState(nConfirmed)).Times(1);

    m_pSession->SetOptions(nNegoId, MediaSession::SET_CONFIRMED_SESSION, nConfirmed, 0);
}

TEST_F(MediaSessionTest, testSetOptionsUnhandled)
{
    // No expectations, just ensure it doesn't crash for unhandled options
    m_pSession->SetOptions(NEGO_ID, MediaSession::SET_DIRECTION, 0, 0);
    m_pSession->SetOptions(NEGO_ID, MediaSession::SET_CONFERENCE_ENABLE, 0, 0);
    m_pSession->SetOptions(NEGO_ID, MediaSession::SEND_FAST_VIDEO_UPDATE, 0, 0);
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
    // Expect FindMediaNego to be called and return IMS_NULL
    EXPECT_CALL(*m_pMockMediaNegoHandler, FindMediaNego(NEGO_ID))
            .Times(1)
            .WillOnce(Return(IMS_NULL));

    // Expect SetMediaPemType NOT to be called on the controllers
    EXPECT_CALL(*m_pMockAudioController, SetMediaPemType(_, _)).Times(0);
    EXPECT_CALL(*m_pMockVideoController, SetMediaPemType(_)).Times(0);

    m_pSession->SetMediaPemType(NEGO_ID, MEDIA_PEM_TYPE::SENDONLY);
}

TEST_F(MediaSessionTest, testFinalizeSdpDelegatesToHandler)
{
    IMS_UINTP nNegoId = NEGO_ID;
    MockISession mockSession;

    // Expect FinalizeSdp to be called on the handler
    EXPECT_CALL(*m_pMockMediaNegoHandler, FinalizeSdp(nNegoId, &mockSession)).Times(1);

    // Expect FinalizeNegotiation to be called on the handler as well
    EXPECT_CALL(*m_pMockMediaNegoHandler, FinalizeNegotiation(nNegoId)).Times(1);

    // Call the method under test
    m_pSession->FinalizeSdp(nNegoId, &mockSession);
}
