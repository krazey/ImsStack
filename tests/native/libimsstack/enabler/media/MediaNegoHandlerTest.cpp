/*
 * Copyright (C) 2025 The Android Open Source Project
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

// Header for the class under test
#include "MediaNegoHandler.h"

#include "MediaEnvironment.h"
#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "MediaDef.h"
#include "MediaNego.h"

// Mock includes
#include "MockICoreService.h"
#include "MockISession.h"
#include "MockMediaNego.h"
#include "MockIMediaNegoFactory.h"
#include "audio/MockAudioNego.h"
#include "text/MockTextNego.h"
#include "video/MockVideoNego.h"

// Define constants/macros if not globally available from includes
#ifndef UNDEFINED_NEGO_ID
#define UNDEFINED_NEGO_ID (reinterpret_cast<IMS_UINTP>(IMS_NULL))  // Common definition
#endif

// Define MEDIA_PORT_INVALID if not available
#ifndef MEDIA_PORT_INVALID
#define MEDIA_PORT_INVALID (-1)
#endif

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;
using ::testing::StrictMock;

class MediaNegoHandlerTest : public ::testing::Test
{
protected:
    static constexpr IMS_UINT32 kSlotId = 1;

    // Class under test and its dependencies
    MockICoreService m_objMockICoreService;
    std::shared_ptr<MockMediaNegoFactory> m_pMockMediaNegoFactory;
    IpAddress m_objLocalIpAddress;
    std::shared_ptr<MediaEnvironment> m_pTestEnvironment;
    std::unique_ptr<MediaNegoHandler> m_pHandler;

    // Using StrictMock to catch uneExpected calls during delegation tests.
    std::shared_ptr<MockMediaNego> m_pMockMediaNego1 = IMS_NULL;
    std::shared_ptr<MockMediaNego> m_pMockMediaNego2 = IMS_NULL;
    // Use shared_ptr for sub-negotiator mocks for easier management with Return()
    std::shared_ptr<StrictMock<MockAudioNego>> m_objMockAudioNego;
    std::shared_ptr<StrictMock<MockVideoNego>> m_objMockVideoNego;
    std::shared_ptr<StrictMock<MockTextNego>> m_objMockTextNego;

    // Use NiceMock for ISession as many methods might be unused per test
    NiceMock<MockISession> m_objMockSession;

    void SetUp() override
    {
        m_objLocalIpAddress = IpAddress("127.0.0.1");
        ON_CALL(m_objMockICoreService, GetIpAddress())
                .WillByDefault(ReturnRef(m_objLocalIpAddress));

        m_pTestEnvironment = std::make_shared<MediaEnvironment>(
                MEDIA_NETWORK_NONE, MEDIA_SERVICE_DEFAULT, &m_objMockICoreService);
        m_pMockMediaNegoFactory = std::make_shared<MockMediaNegoFactory>();
        m_pHandler = std::make_unique<MediaNegoHandler>(
                kSlotId, m_pTestEnvironment, m_pMockMediaNegoFactory);

        // Create mocks owned by the fixture.
        m_pMockMediaNego1 = std::make_shared<MockMediaNego>(kSlotId);
        m_pMockMediaNego2 = std::make_shared<MockMediaNego>(kSlotId);
        m_objMockAudioNego = std::make_shared<StrictMock<MockAudioNego>>(kSlotId);
        m_objMockVideoNego = std::make_shared<StrictMock<MockVideoNego>>(kSlotId);
        m_objMockTextNego = std::make_shared<StrictMock<MockTextNego>>(kSlotId);

        // Set default return values for Get*Nego methods on the main mocks
        // to avoid null checks in every test unless specifically testing null cases.
        // Return shared_ptr versions for convenience.
        ON_CALL(*m_pMockMediaNego1, GetAudioNego()).WillByDefault(Return(IMS_NULL));
        ON_CALL(*m_pMockMediaNego1, GetVideoNego()).WillByDefault(Return(IMS_NULL));
        ON_CALL(*m_pMockMediaNego1, GetTextNego()).WillByDefault(Return(IMS_NULL));
        ON_CALL(*m_pMockMediaNego2, GetAudioNego()).WillByDefault(Return(IMS_NULL));
        ON_CALL(*m_pMockMediaNego2, GetVideoNego()).WillByDefault(Return(IMS_NULL));
        ON_CALL(*m_pMockMediaNego2, GetTextNego()).WillByDefault(Return(IMS_NULL));

        // Default return for remote address to avoid issues with const&
        ON_CALL(*m_objMockAudioNego, GetNegotiatedRemoteAddress())
                .WillByDefault(ReturnRef(IpAddress::NONE));
        ON_CALL(*m_objMockVideoNego, GetNegotiatedRemoteAddress())
                .WillByDefault(ReturnRef(IpAddress::NONE));
        ON_CALL(*m_objMockTextNego, GetNegotiatedRemoteAddress())
                .WillByDefault(ReturnRef(IpAddress::NONE));
    }

    void TearDown() override
    {
        m_pHandler->ClearAllMediaNego();
        m_pHandler.reset();
    }

    // Helper function to set up a mock nego via the factory for delegation tests
    IMS_UINTP SetupMockNegoForTest(std::shared_ptr<MockMediaNego> pMockNego)
    {
        EXPECT_CALL(*m_pMockMediaNegoFactory, CreateMediaNego(kSlotId)).WillOnce(Return(pMockNego));
        ON_CALL(*pMockNego, CreateProfile(_)).WillByDefault(Return(IMS_TRUE));
        IMS_UINTP nNegoId = m_pHandler->CreateMediaNego();
        EXPECT_NE(nNegoId, 0);  // Ensure ID is not zero
        return nNegoId;
    }
};

TEST_F(MediaNegoHandlerTest, testCreateMediaNegoNewInstance)
{
    IMS_UINTP nNegoId = SetupMockNegoForTest(m_pMockMediaNego1);

    EXPECT_EQ(m_pMockMediaNego1, m_pHandler->FindMediaNego(nNegoId));
}

TEST_F(MediaNegoHandlerTest, CreateMediaNegoForkInstance)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);

    std::shared_ptr<MediaNego> pNego1 = m_pHandler->FindMediaNego(nNegoId1);
    ASSERT_NE(IMS_NULL, pNego1);

    // Expect the factory to be called for forking
    EXPECT_CALL(*m_pMockMediaNegoFactory, CreateForkedMediaNego(kSlotId, pNego1, _))
            .WillOnce(Return(m_pMockMediaNego2));

    IMS_UINTP nNegoId2 = m_pHandler->CreateMediaNego(nNegoId1);  // Fork from pNego1
    EXPECT_NE(nNegoId1, nNegoId2);                               // Should be a different nego id

    EXPECT_EQ(m_pMockMediaNego2, m_pHandler->FindMediaNego(nNegoId2));

    EXPECT_EQ(IMS_TRUE, m_pHandler->DeleteMediaNego(nNegoId1));
    EXPECT_EQ(IMS_FALSE, m_pHandler->DeleteMediaNego(nNegoId2));

    EXPECT_EQ(IMS_NULL, m_pHandler->FindMediaNego(nNegoId1));
    EXPECT_NE(IMS_NULL, m_pHandler->FindMediaNego(nNegoId2));
}

TEST_F(MediaNegoHandlerTest, CreateMediaNegoForkInvalidId)
{
    IMS_UINTP nNegoId = m_pHandler->CreateMediaNego(9999);  // Try to fork from invalid ID
    EXPECT_EQ(nNegoId, 0);
}

TEST_F(MediaNegoHandlerTest, FindMediaNegoNotFound)
{
    // Create one to ensure map isn't empty, but search for a different one
    IMS_UINTP nValidNegoId = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_EQ(IMS_NULL, m_pHandler->FindMediaNego(nValidNegoId + 1));
}

TEST_F(MediaNegoHandlerTest, DeleteMediaNegoFailed)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);

    EXPECT_EQ(IMS_FALSE, m_pHandler->DeleteMediaNego(nNegoId1));
    EXPECT_NE(IMS_NULL, m_pHandler->FindMediaNego(nNegoId1));
}

TEST_F(MediaNegoHandlerTest, DeleteMediaNegoNotFound)
{
    IMS_UINTP nValidNegoId = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_EQ(IMS_FALSE, m_pHandler->DeleteMediaNego(nValidNegoId + 1));
}

TEST_F(MediaNegoHandlerTest, DeleteMediaNegoUndefinedId)
{
    EXPECT_EQ(IMS_FALSE, m_pHandler->DeleteMediaNego(UNDEFINED_NEGO_ID));
}

TEST_F(MediaNegoHandlerTest, ClearAllMediaNego)
{
    // Expect factory to create mocks
    EXPECT_CALL(*m_pMockMediaNegoFactory, CreateMediaNego(kSlotId))
            .WillOnce(Return(m_pMockMediaNego1))
            .WillOnce(Return(m_pMockMediaNego2));
    ON_CALL(*m_pMockMediaNego1, CreateProfile(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockMediaNego2, CreateProfile(_)).WillByDefault(Return(IMS_TRUE));

    IMS_UINTP nNegoId1 = m_pHandler->CreateMediaNego();
    IMS_UINTP nNegoId2 = m_pHandler->CreateMediaNego();
    ASSERT_NE(0, nNegoId1);
    ASSERT_NE(0, nNegoId2);

    m_pHandler->ClearAllMediaNego();
    EXPECT_EQ(IMS_NULL, m_pHandler->FindMediaNego(nNegoId1));  // Verify it's gone
    EXPECT_EQ(IMS_NULL, m_pHandler->FindMediaNego(nNegoId2));  // Verify it's gone
}

// == SDP Negotiation Operations ==

TEST_F(MediaNegoHandlerTest, FormSdpSuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIOVIDEO;
    MEDIA_DIRECTION eAudioDir = MEDIA_DIRECTION_SEND_RECEIVE,
                    eVideoDir = MEDIA_DIRECTION_SEND_RECEIVE, eTextDir = MEDIA_DIRECTION_INACTIVE;
    IMS_BOOL bEnforce = IMS_FALSE;

    EXPECT_CALL(*m_pMockMediaNego1,
            FormSdp(&m_objMockSession, eType, eAudioDir, eVideoDir, eTextDir, bEnforce))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE,
            m_pHandler->FormSdp(
                    nNegoId1, &m_objMockSession, eType, eAudioDir, eVideoDir, eTextDir, bEnforce));
}

TEST_F(MediaNegoHandlerTest, FormSdpNegoNotFound)
{
    EXPECT_EQ(IMS_FALSE,
            m_pHandler->FormSdp(9999, &m_objMockSession, MEDIA_TYPE_AUDIO,
                    MEDIA_DIRECTION_SEND_RECEIVE, MEDIA_DIRECTION_INACTIVE,
                    MEDIA_DIRECTION_INACTIVE, IMS_FALSE));
}

TEST_F(MediaNegoHandlerTest, GetSupportedMediaTypesFromSdpSuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    MEDIA_CONTENT_TYPE eExpectedType = MEDIA_TYPE_AUDIO;

    EXPECT_CALL(*m_pMockMediaNego1, GetSupportedMediaTypesFromSdp(&m_objMockSession))
            .WillOnce(Return(eExpectedType));

    EXPECT_EQ(
            eExpectedType, m_pHandler->GetSupportedMediaTypesFromSdp(nNegoId1, &m_objMockSession));
}

TEST_F(MediaNegoHandlerTest, GetSupportedMediaTypesFromSdpNegoNotFound)
{
    EXPECT_EQ(
            MEDIA_TYPE_INVALID, m_pHandler->GetSupportedMediaTypesFromSdp(9999, &m_objMockSession));
}

TEST_F(MediaNegoHandlerTest, NegotiateSdpSuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    SdpNegotiationResult expectedResult(MEDIA_NEGO_NO_ERROR,
            (MEDIA_CONTENT_TYPE)(MEDIA_TYPE_AUDIO | MEDIA_TYPE_TEXT), MEDIA_DIRECTION_SEND_RECEIVE,
            MEDIA_DIRECTION_INACTIVE, MEDIA_DIRECTION_SEND_RECEIVE);

    EXPECT_CALL(*m_pMockMediaNego1, NegotiateSdp(&m_objMockSession))
            .WillOnce(Return(expectedResult));

    SdpNegotiationResult objResult = m_pHandler->NegotiateSdp(nNegoId1, &m_objMockSession);

    EXPECT_EQ(objResult.eNegotiatedType, expectedResult.eNegotiatedType);
    EXPECT_EQ(objResult.eAudioDirection, expectedResult.eAudioDirection);
    EXPECT_EQ(objResult.eVideoDirection, expectedResult.eVideoDirection);
    EXPECT_EQ(objResult.eTextDirection, expectedResult.eTextDirection);
    EXPECT_EQ(objResult.eResult, expectedResult.eResult);
}

TEST_F(MediaNegoHandlerTest, NegotiateSdpFailureFromMediaNego)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    SdpNegotiationResult expectedResult(MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);

    EXPECT_CALL(*m_pMockMediaNego1, NegotiateSdp(&m_objMockSession))
            .WillOnce(Return(expectedResult));

    SdpNegotiationResult objResult = m_pHandler->NegotiateSdp(nNegoId1, &m_objMockSession);

    EXPECT_EQ(objResult.eNegotiatedType, MEDIA_TYPE_INVALID);
    EXPECT_EQ(objResult.eResult, MEDIA_NEGO_ERROR_NO_CODEC_MATCHED);
    EXPECT_EQ(objResult.eAudioDirection, MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(objResult.eVideoDirection, MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(objResult.eTextDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaNegoHandlerTest, NegotiateSdpNegoNotFound)
{
    SdpNegotiationResult objResult = m_pHandler->NegotiateSdp(9999, &m_objMockSession);
    EXPECT_EQ(objResult.eResult, MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    EXPECT_EQ(objResult.eNegotiatedType, MEDIA_TYPE_INVALID);
}

TEST_F(MediaNegoHandlerTest, FinalizeSdpSuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, FinalizeSdp(&m_objMockSession));
    m_pHandler->FinalizeSdp(nNegoId1, &m_objMockSession);  // void return
}

TEST_F(MediaNegoHandlerTest, FinalizeSdpNegoNotFound)
{
    // No EXPECT_CALL needed, just verify it doesn't crash and returns
    m_pHandler->FinalizeSdp(9999, &m_objMockSession);
}

// == Getters for Negotiated Information (using Mock Injection) ==

TEST_F(MediaNegoHandlerTest, GetNegoStateSuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    NEGO_STATE eExpectedState = STATE_IDLE;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegoState()).WillOnce(Return(eExpectedState));
    EXPECT_EQ(eExpectedState, m_pHandler->GetNegoState(nNegoId1));
}

TEST_F(MediaNegoHandlerTest, GetNegoStateNegoNotFound)
{
    EXPECT_EQ(STATE_NOTUSED, m_pHandler->GetNegoState(9999));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedMediaTypeAudioVideo)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    // Simulate successful negotiation for Audio and Video
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedAudioQuality())
            .WillOnce(Return(AUDIO_CODEC::AUDIO_CODEC_AMRWB));
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedVideoQuality())
            .WillOnce(Return(VIDEO_RESOLUTION_VGA_PR));
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedTextQuality())
            .WillOnce(Return(TEXT_CODEC_NOT_USED));

    MEDIA_CONTENT_TYPE eExpectedType =
            static_cast<MEDIA_CONTENT_TYPE>(MEDIA_TYPE_AUDIO | MEDIA_TYPE_VIDEO);
    EXPECT_EQ(eExpectedType, m_pHandler->GetNegotiatedMediaType(nNegoId1));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedMediaTypeOnlyText)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedAudioQuality())
            .WillOnce(Return(AUDIO_CODEC_NOT_USED));
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedVideoQuality())
            .WillOnce(Return(VIDEO_RESOLUTION_NOT_USED));
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedTextQuality()).WillOnce(Return(TEXT_CODEC_T140));

    EXPECT_EQ(MEDIA_TYPE_TEXT, m_pHandler->GetNegotiatedMediaType(nNegoId1));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedMediaTypeNone)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedAudioQuality())
            .WillOnce(Return(AUDIO_CODEC_NOT_USED));
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedVideoQuality())
            .WillOnce(Return(VIDEO_RESOLUTION_NOT_USED));
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedTextQuality())
            .WillOnce(Return(TEXT_CODEC_NOT_USED));

    EXPECT_EQ(MEDIA_TYPE_INVALID, m_pHandler->GetNegotiatedMediaType(nNegoId1));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedMediaTypeNegoNotFound)
{
    EXPECT_EQ(MEDIA_TYPE_INVALID, m_pHandler->GetNegotiatedMediaType(9999));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedQualityAudio)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    AUDIO_CODEC eExpectedQuality = AUDIO_CODEC_AMR;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedAudioQuality()).WillOnce(Return(eExpectedQuality));
    EXPECT_EQ(static_cast<IMS_SINT32>(eExpectedQuality),
            m_pHandler->GetNegotiatedQuality(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedQualityVideo)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    VIDEO_RESOLUTION eExpectedQuality = VIDEO_RESOLUTION_VGA_PR;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedVideoQuality()).WillOnce(Return(eExpectedQuality));
    EXPECT_EQ(static_cast<IMS_SINT32>(eExpectedQuality),
            m_pHandler->GetNegotiatedQuality(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedQualityText)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    TEXT_CODEC eExpectedQuality = TEXT_CODEC_T140_RED;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedTextQuality()).WillOnce(Return(eExpectedQuality));
    EXPECT_EQ(static_cast<IMS_SINT32>(eExpectedQuality),
            m_pHandler->GetNegotiatedQuality(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedQualityInvalidType)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    // No calls eExpected on m_pMockMediaNego1
    EXPECT_EQ(0, m_pHandler->GetNegotiatedQuality(nNegoId1, MEDIA_TYPE_INVALID));
    EXPECT_EQ(0,
            m_pHandler->GetNegotiatedQuality(nNegoId1,
                    MEDIA_TYPE_AUDIOVIDEO));  // Combined eType not handled
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedQualityNegoNotFound)
{
    EXPECT_EQ(0, m_pHandler->GetNegotiatedQuality(9999, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateAudio)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    AUDIO_CODEC_BITRATE eExpectedRate = AUDIO_CODEC_BITRATE_AMR_WB_2385;

    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego())
            .WillOnce(Return(m_objMockAudioNego));  // Return shared_ptr
    EXPECT_CALL(*m_objMockAudioNego, GetNegotiatedAudioCodecRate()).WillOnce(Return(eExpectedRate));

    EXPECT_EQ(static_cast<IMS_SINT32>(eExpectedRate),
            m_pHandler->GetNegotiatedCodecBitrate(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateAudioNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego())
            .WillOnce(Return(IMS_NULL));  // AudioNego doesn't exist
    // Should return default value based on implementation
    EXPECT_EQ(static_cast<IMS_SINT32>(AUDIO_CODEC_BITRATE_MAX),
            m_pHandler->GetNegotiatedCodecBitrate(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateVideo)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    VIDEO_RESOLUTION eExpectedRes =
            VIDEO_RESOLUTION_VGA_PR;  // Implementation uses resolution for video bitrate

    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(m_objMockVideoNego));
    EXPECT_CALL(*m_objMockVideoNego, GetNegotiatedResolution()).WillOnce(Return(eExpectedRes));

    EXPECT_EQ(static_cast<IMS_SINT32>(eExpectedRes),
            m_pHandler->GetNegotiatedCodecBitrate(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateVideoNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(IMS_NULL));
    // Should return default value based on implementation
    EXPECT_EQ(static_cast<IMS_SINT32>(VIDEO_RESOLUTION_INVALID),
            m_pHandler->GetNegotiatedCodecBitrate(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateText)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    // Implementation returns a fixed value for text, doesn't use TextNego
    EXPECT_EQ(1000, m_pHandler->GetNegotiatedCodecBitrate(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateInvalidType)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_EQ(0, m_pHandler->GetNegotiatedCodecBitrate(nNegoId1, MEDIA_TYPE_INVALID));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedCodecBitrateNegoNotFound)
{
    EXPECT_EQ(0, m_pHandler->GetNegotiatedCodecBitrate(9999, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortAudio)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_SINT32 nExpectedPort = 1234;
    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(m_objMockAudioNego));
    EXPECT_CALL(*m_objMockAudioNego, GetRemotePort()).WillOnce(Return(nExpectedPort));
    EXPECT_EQ(nExpectedPort, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortVideo)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_SINT32 nExpectedPort = 5678;
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(m_objMockVideoNego));
    EXPECT_CALL(*m_objMockVideoNego, GetRemotePort()).WillOnce(Return(nExpectedPort));
    EXPECT_EQ(nExpectedPort, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortText)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_SINT32 nExpectedPort = 9101;
    EXPECT_CALL(*m_pMockMediaNego1, GetTextNego()).WillOnce(Return(m_objMockTextNego));
    EXPECT_CALL(*m_objMockTextNego, GetRemotePort()).WillOnce(Return(nExpectedPort));
    EXPECT_EQ(nExpectedPort, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortAudioNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(MEDIA_PORT_INVALID, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortVideoNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(MEDIA_PORT_INVALID, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortTextNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetTextNego()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(MEDIA_PORT_INVALID, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetRemotePortInvalidType)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_EQ(MEDIA_PORT_INVALID, m_pHandler->GetRemotePort(nNegoId1, MEDIA_TYPE_INVALID));
    EXPECT_EQ(MEDIA_PORT_INVALID,
            m_pHandler->GetRemotePort(nNegoId1,
                    MEDIA_TYPE_AUDIOVIDEO));  // Combined eType not handled
}

TEST_F(MediaNegoHandlerTest, GetRemotePortNegoNotFound)
{
    EXPECT_EQ(MEDIA_PORT_INVALID, m_pHandler->GetRemotePort(9999, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedDirectionAudio)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    MEDIA_DIRECTION eExpectedDir = MEDIA_DIRECTION_SEND_RECEIVE;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedAudioDirection()).WillOnce(Return(eExpectedDir));
    EXPECT_EQ(eExpectedDir, m_pHandler->GetNegotiatedDirection(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedDirectionVideo)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    MEDIA_DIRECTION eExpectedDir = MEDIA_DIRECTION_SEND;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedVideoDirection()).WillOnce(Return(eExpectedDir));
    EXPECT_EQ(eExpectedDir, m_pHandler->GetNegotiatedDirection(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedDirectionText)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    MEDIA_DIRECTION eExpectedDir = MEDIA_DIRECTION_RECEIVE;
    EXPECT_CALL(*m_pMockMediaNego1, GetNegotiatedTextDirection()).WillOnce(Return(eExpectedDir));
    EXPECT_EQ(eExpectedDir, m_pHandler->GetNegotiatedDirection(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedDirectionInvalidType)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    // No calls eExpected on m_pMockMediaNego1
    EXPECT_EQ(MEDIA_DIRECTION_INVALID,
            m_pHandler->GetNegotiatedDirection(nNegoId1, MEDIA_TYPE_INVALID));
    EXPECT_EQ(MEDIA_DIRECTION_INVALID,
            m_pHandler->GetNegotiatedDirection(nNegoId1,
                    MEDIA_TYPE_AUDIOVIDEO));  // Combined eType not handled
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedDirectionNegoNotFound)
{
    EXPECT_EQ(MEDIA_DIRECTION_INVALID, m_pHandler->GetNegotiatedDirection(9999, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressAudio)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IpAddress objExpectedAddr("1.2.3.4");
    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(m_objMockAudioNego));
    EXPECT_CALL(*m_objMockAudioNego, GetNegotiatedRemoteAddress())
            .WillOnce(ReturnRef(objExpectedAddr));  // Return by const ref
    EXPECT_EQ(objExpectedAddr, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressVideo)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IpAddress objExpectedAddr("5.6.7.8");
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(m_objMockVideoNego));
    EXPECT_CALL(*m_objMockVideoNego, GetNegotiatedRemoteAddress())
            .WillOnce(ReturnRef(objExpectedAddr));
    EXPECT_EQ(objExpectedAddr, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressText)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IpAddress objExpectedAddr("9.10.11.12");
    EXPECT_CALL(*m_pMockMediaNego1, GetTextNego()).WillOnce(Return(m_objMockTextNego));
    EXPECT_CALL(*m_objMockTextNego, GetNegotiatedRemoteAddress())
            .WillOnce(ReturnRef(objExpectedAddr));
    EXPECT_EQ(objExpectedAddr, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressAudioNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(IpAddress::NONE, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_AUDIO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressVideoNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(IpAddress::NONE, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_VIDEO));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressTextNegoNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_CALL(*m_pMockMediaNego1, GetTextNego()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(IpAddress::NONE, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_TEXT));
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressInvalidType)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    EXPECT_EQ(
            IpAddress::NONE, m_pHandler->GetNegotiatedRemoteAddress(nNegoId1, MEDIA_TYPE_INVALID));
    EXPECT_EQ(IpAddress::NONE,
            m_pHandler->GetNegotiatedRemoteAddress(nNegoId1,
                    MEDIA_TYPE_AUDIOVIDEO));  // Combined eType not handled
}

TEST_F(MediaNegoHandlerTest, GetNegotiatedRemoteAddressNegoNotFound)
{
    EXPECT_EQ(IpAddress::NONE, m_pHandler->GetNegotiatedRemoteAddress(9999, MEDIA_TYPE_AUDIO));
}

// == Options ==

TEST_F(MediaNegoHandlerTest, SetRtpPortAudioVideoSuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_UINT32 nPort = 5004;
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIOVIDEO;  // Set for both

    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(m_objMockAudioNego));
    EXPECT_CALL(*m_objMockAudioNego, SetLocalPort(nPort)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(m_objMockVideoNego));
    EXPECT_CALL(*m_objMockVideoNego, SetLocalPort(nPort)).WillOnce(Return(IMS_TRUE));
    // TextNego should not be called
    EXPECT_CALL(*m_pMockMediaNego1, GetTextNego()).Times(0);

    // Testing the *actual* implementation logic:
    EXPECT_EQ(IMS_TRUE, m_pHandler->SetRtpPort(nNegoId1, eType, nPort));
}

TEST_F(MediaNegoHandlerTest, SetRtpPortAudioOnlySuccess)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_UINT32 nPort = 5006;
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIO;  // Set only for Audio

    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(m_objMockAudioNego));
    EXPECT_CALL(*m_objMockAudioNego, SetLocalPort(nPort)).WillOnce(Return(IMS_TRUE));
    // Video/Text Nego should not be called
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).Times(0);
    EXPECT_CALL(*m_pMockMediaNego1, GetTextNego()).Times(0);

    // Testing the *actual* implementation logic:
    EXPECT_EQ(IMS_TRUE, m_pHandler->SetRtpPort(nNegoId1, eType, nPort));
}

TEST_F(MediaNegoHandlerTest, SetRtpPortOneFails)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_UINT32 nPort = 5008;
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIOVIDEO;

    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(m_objMockAudioNego));
    EXPECT_CALL(*m_objMockAudioNego, SetLocalPort(nPort))
            .WillOnce(Return(IMS_FALSE));  // Audio SetLocalPort fails
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego()).WillOnce(Return(m_objMockVideoNego));
    EXPECT_CALL(*m_objMockVideoNego, SetLocalPort(nPort))
            .WillOnce(Return(IMS_TRUE));  // Video SetLocalPort succeeds

    // Testing the *actual* implementation logic:
    EXPECT_EQ(IMS_FALSE, m_pHandler->SetRtpPort(nNegoId1, eType, nPort));
}

TEST_F(MediaNegoHandlerTest, SetRtpPortOneIsNull)
{
    IMS_UINTP nNegoId1 = SetupMockNegoForTest(m_pMockMediaNego1);
    IMS_UINT32 nPort = 5010;
    MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_AUDIOVIDEO;  // Try to set for both

    EXPECT_CALL(*m_pMockMediaNego1, GetAudioNego()).WillOnce(Return(m_objMockAudioNego));
    EXPECT_CALL(*m_objMockAudioNego, SetLocalPort(nPort))
            .WillOnce(Return(IMS_TRUE));  // Audio succeeds
    EXPECT_CALL(*m_pMockMediaNego1, GetVideoNego())
            .WillOnce(Return(IMS_NULL));  // VideoNego is null

    // Testing the *actual* implementation logic:
    EXPECT_EQ(IMS_FALSE, m_pHandler->SetRtpPort(nNegoId1, eType, nPort));
}

TEST_F(MediaNegoHandlerTest, SetRtpPortNegoNotFound)
{
    EXPECT_EQ(IMS_FALSE, m_pHandler->SetRtpPort(9999, MEDIA_TYPE_AUDIO, 5004));
}
