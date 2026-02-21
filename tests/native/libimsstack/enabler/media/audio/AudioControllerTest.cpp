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
#include <MediaNetworkConnectionWatcher.h>
#include <ServiceConfig.h>
#include <audio/AudioSession.h>
#include <audio/AudioController.h>
#include <config/AudioConfiguration.h>

#include <config/MockAudioConfiguration.h>
#include <MockIMediaSessionListener.h>
#include <audio/MockAudioNego.h>

using namespace android::telephony::imsmedia;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_UINT32 LOCAL_PORT = 20000;
const IMS_UINTP NEGO_ID = 1000;
const IMS_UINT32 ACCESS_NETWORK = MediaNetworkConnectionWatcher::EUTRAN;

class FakeAudioController : public AudioController
{
public:
    FakeAudioController() :
            AudioController()
    {
    }

    virtual ~FakeAudioController() override {}

    IMS_BOOL IsAudioConfigChanged(IN AudioConfig* pAudioConfig) override
    {
        return AudioController::IsAudioConfigChanged(pAudioConfig);
    }

    AudioSession* FindAudioSession(IN IMS_UINTP nNegoId = UNDEFINED_NEGO_ID)
    {
        return AudioController::FindAudioSession(nNegoId);
    }
};

class MockAudioSession : public AudioSession
{
public:
    MOCK_METHOD(void, SetMediaPemType, (MEDIA_PEM_TYPE ePemType), (override));
    MOCK_METHOD(MEDIA_PEM_TYPE, GetPemType, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdateDirectionToInactiveByPem, (), (override));
};

class MockAudioController : public FakeAudioController
{
public:
    MOCK_METHOD(IMS_BOOL, ModifySession, (IN IMS_UINTP nNegoId), (override));
};

class AudioControllerTest : public ::testing::Test
{
public:
    std::unique_ptr<FakeAudioController> m_pController;
    std::unique_ptr<NiceMock<MockAudioConfiguration>> m_pConfig;
    FakeIMediaSessionListener m_objFakeListener;
    MockIMediaSessionListener m_objListener;
    std::shared_ptr<MockAudioNego> m_pAudioNego;

    std::unique_ptr<AudioProfile> m_pLocalProfile;
    std::unique_ptr<AudioProfile> m_pPeerProfile;
    std::unique_ptr<AudioProfile> m_pNegoProfile;
    IpAddress m_objIpAddr;

protected:
    AudioConfiguration m_objAudioConfig;
    virtual void SetUp() override
    {
        m_pController = std::make_unique<FakeAudioController>();
        m_pConfig = std::make_unique<NiceMock<MockAudioConfiguration>>(MEDIA_TYPE_AUDIO);
        m_pConfig->Create(ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID));
        m_pAudioNego = std::make_shared<MockAudioNego>(DEFAULT_SLOT_ID);

        m_objListener.SetDelegate(&m_objFakeListener);
        m_objListener.DelegateToFake();

        m_pLocalProfile = std::make_unique<AudioProfile>();
        AudioProfile::Payload* pSrcAmrPayload = new AudioProfile::Payload();
        pSrcAmrPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
        pSrcAmrPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
        m_pLocalProfile->AddPayload(pSrcAmrPayload);
        m_pLocalProfile->SetDataPort(LOCAL_PORT);

        m_pPeerProfile = std::make_unique<AudioProfile>(*m_pLocalProfile);
        m_pNegoProfile = std::make_unique<AudioProfile>(*m_pLocalProfile);

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pAudioNego, GetLocalAddress()).WillByDefault(ReturnRef(m_objIpAddr));
        ON_CALL(*m_pAudioNego, GetLocalPort()).WillByDefault(Return(LOCAL_PORT));
        ON_CALL(*m_pAudioNego, GetNegotiatedLocalProfile())
                .WillByDefault(Return(m_pLocalProfile.get()));
        ON_CALL(*m_pAudioNego, GetNegotiatedPeerProfile())
                .WillByDefault(Return(m_pPeerProfile.get()));
        ON_CALL(*m_pAudioNego, GetNegotiatedNegoProfile())
                .WillByDefault(Return(m_pNegoProfile.get()));
    }

    virtual void TearDown() override {}
};

TEST_F(AudioControllerTest, testCreateSessionFail)
{
    EXPECT_EQ(m_pController->CreateSession(nullptr, 10000, nullptr, MEDIA_SERVICE_DEFAULT),
            IMS_FALSE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 0);
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, 1000, nullptr, MEDIA_SERVICE_DEFAULT),
            IMS_FALSE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 0);
}

TEST_F(AudioControllerTest, testDeleteSessionFail)
{
    EXPECT_FALSE(m_pController->DeleteSession(0));
    EXPECT_FALSE(m_pController->DeleteSession(100));
}

TEST_F(AudioControllerTest, testUpdateLocalAddressFail)
{
    EXPECT_EQ(m_pController->UpdateLocalAddress(nullptr), IMS_FALSE);
}

TEST_F(AudioControllerTest, testOpenSessionFail)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    IpAddress objIpAddr(LOCAL_IP);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(2000), IMS_FALSE);
}

TEST_F(AudioControllerTest, testCloseSessionFail)
{
    EXPECT_EQ(m_pController->CloseSession(), IMS_FALSE);
}

TEST_F(AudioControllerTest, testCloseSessionWithSessionCreated)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    EXPECT_EQ(m_pController->CloseSession(), IMS_FALSE);
}

TEST_F(AudioControllerTest, testDeleteSessionFailWithOneSession)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(NEGO_ID), IMS_TRUE);

    // Attempt to delete the only session
    EXPECT_FALSE(m_pController->DeleteSession(NEGO_ID));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    m_pController->CloseSession();
}

TEST_F(AudioControllerTest, testDeleteSessionSuccess)
{
    IMS_UINTP negoId1 = 1000;
    IMS_UINTP negoId2 = 2000;
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, negoId1, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, negoId2, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 2);

    // Attempt to delete one session
    EXPECT_EQ(m_pController->DeleteSession(negoId1), IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    m_pController->CloseSession();
    m_pController->CloseSession();
}

TEST_F(AudioControllerTest, testModifySessionSendDtmf)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(NEGO_ID), IMS_TRUE);

    m_pController->SetCallSessionState(IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(2000, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);

    EXPECT_EQ(m_pController->SendDtmf('1'), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
}

TEST_F(AudioControllerTest, testAddSession)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(NEGO_ID), IMS_TRUE);

    EXPECT_EQ(m_pController->AddSession(2000, ACCESS_NETWORK, m_pAudioNego), IMS_FALSE);
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, 2000, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 2);

    EXPECT_EQ(m_pController->AddSession(2000, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);
}

TEST_F(AudioControllerTest, testConfirmSession)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(NEGO_ID), IMS_TRUE);

    EXPECT_EQ(m_pController->ConfirmSession(2000), IMS_FALSE);
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, 2000, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 2);

    EXPECT_EQ(m_pController->ConfirmSession(2000), IMS_TRUE);
}

TEST_F(AudioControllerTest, testUpdateQualityThreshold)
{
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_FALSE(m_pController->UpdateQualityThreshold(NEGO_ID, nullptr));
    EXPECT_EQ(m_pController->UpdateQualityThreshold(UNDEFINED_NEGO_ID, m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(NEGO_ID, m_pAudioNego), IMS_TRUE);
}

TEST_F(AudioControllerTest, testInactivityTimer)
{
    IMS_UINTP nNegoId1 = 1000;
    IMS_UINTP nNegoId2 = 2000;
    IMS_UINT32 inactivityTime1 = 1111;
    IMS_UINT32 inactivityTime2 = 2222;
    IMS_UINT32 inactivityTime3 = 3333;

    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId1, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId2, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 2);

    m_pController->SetNetworkToneTimer(nNegoId1, inactivityTime1);
    m_pController->SetNetworkToneTimer(nNegoId2, inactivityTime2);

    EXPECT_EQ(
            m_pController->GetInactivityTimer(NETWORK_TONE_INACTIVITY, nNegoId1), inactivityTime1);
    EXPECT_EQ(
            m_pController->GetInactivityTimer(NETWORK_TONE_INACTIVITY, nNegoId2), inactivityTime2);

    m_pController->UpdateSession(nNegoId2, ACCESS_NETWORK, m_pAudioNego);

    m_pController->SetNetworkToneTimer(UNDEFINED_NEGO_ID, inactivityTime3);
    EXPECT_EQ(
            m_pController->GetInactivityTimer(NETWORK_TONE_INACTIVITY, nNegoId1), inactivityTime1);
    EXPECT_EQ(
            m_pController->GetInactivityTimer(NETWORK_TONE_INACTIVITY, nNegoId2), inactivityTime3);
}

TEST_F(AudioControllerTest, testIsAudioConfigChanged)
{
    AudioConfig* pAudioConfig1 = new AudioConfig();
    AudioConfig* pAudioConfig2 = new AudioConfig();
    pAudioConfig2->setAccessNetwork(5);

    EXPECT_EQ(m_pController->IsAudioConfigChanged(IMS_NULL), IMS_FALSE);
    EXPECT_EQ(m_pController->IsAudioConfigChanged(pAudioConfig1), IMS_TRUE);
    EXPECT_EQ(m_pController->IsAudioConfigChanged(pAudioConfig1), IMS_FALSE);
    EXPECT_EQ(m_pController->IsAudioConfigChanged(pAudioConfig2), IMS_TRUE);
    EXPECT_EQ(m_pController->IsAudioConfigChanged(pAudioConfig2), IMS_FALSE);
    EXPECT_EQ(m_pController->IsAudioConfigChanged(pAudioConfig1), IMS_TRUE);

    pAudioConfig1->setAccessNetwork(5);
    EXPECT_EQ(m_pController->IsAudioConfigChanged(pAudioConfig1), IMS_TRUE);

    delete pAudioConfig1;
    delete pAudioConfig2;
}

TEST_F(AudioControllerTest, testIsSessionOpened)
{
    // Initial state: No session
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Create session
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    // Session created but not opened (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Open session
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(NEGO_ID), IMS_TRUE);
    // Session is now open (state is STATE_IDLE or higher)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_TRUE);

    // Close session
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    // Session is closed (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);
}

TEST_F(AudioControllerTest, testUpdateAccessNetworkFailed)
{
    const IMS_UINT32 NEW_ACCESS_NETWORK = ACCESS_NETWORK + 1;

    // case 1: No sessions
    EXPECT_EQ(m_pController->UpdateAccessNetwork(NEW_ACCESS_NETWORK), IMS_FALSE);

    // case 2: One session, created but not opened (STATE_NONE)
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_EQ(m_pController->UpdateAccessNetwork(NEW_ACCESS_NETWORK), IMS_FALSE);
}

TEST_F(AudioControllerTest, testUpdateAccessNetworkSuccess)
{
    const IMS_UINT32 NEW_ACCESS_NETWORK = ACCESS_NETWORK + 1;

    // case 1: One session, created and opened
    IMS_UINTP nNegoId2 = 2000;
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, nNegoId2, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId2), IMS_TRUE);

    // Set up the session state via UpdateSession
    m_pController->SetCallSessionState(IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(nNegoId2, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateAccessNetwork(NEW_ACCESS_NETWORK), IMS_TRUE);

    m_pController->CloseSession();
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 0);

    // case 2: Two sessions. First is STATE_NONE, second is opened.
    IMS_UINTP nNegoId3 = 3000;
    IMS_UINTP nNegoId4 = 4000;
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, nNegoId3, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    EXPECT_TRUE(m_pController->CreateSession(
            &m_objListener, nNegoId4, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId4), IMS_TRUE);

    // Set up the session state via UpdateSession
    m_pController->SetCallSessionState(IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(nNegoId4, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateAccessNetwork(NEW_ACCESS_NETWORK + 1), IMS_TRUE);

    m_pController->CloseSession();
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 0);
}

TEST_F(AudioControllerTest, testGetMediaDirection)
{
    EXPECT_EQ(m_pController->GetMediaDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(AudioControllerTest, testConcurrentWifiAndLteSession)
{
    const IMS_UINTP nNegoId1 = 1000;
    const IMS_UINTP nNegoId2 = 2000;
    const IMS_UINT32 nNetworkType1 = MediaNetworkConnectionWatcher::IWLAN;
    const IMS_UINT32 nNetworkType2 = MediaNetworkConnectionWatcher::EUTRAN;

    ASSERT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId1, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    ASSERT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId2, m_pConfig.get(), MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    ASSERT_EQ(m_pController->GetAudioSessionSize(), 2);

    m_pController->SetCallSessionState(IMS_TRUE);
    ASSERT_EQ(m_pController->UpdateSession(nNegoId1, nNetworkType1, m_pAudioNego), IMS_TRUE);
    ASSERT_EQ(m_pController->UpdateSession(nNegoId2, nNetworkType2, m_pAudioNego), IMS_TRUE);

    ASSERT_EQ(m_pController->DeleteSession(nNegoId2), IMS_TRUE);
    ASSERT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateSession(nNegoId1, nNetworkType1, m_pAudioNego), IMS_TRUE);
    m_pController->CloseSession();
}

TEST_F(AudioControllerTest, testUpdateAnbrEnabledConfig)
{
    EXPECT_FALSE(m_pController->UpdateAnbrEnabledConfig(NEGO_ID, IMS_TRUE));

    // Create a session
    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);

    // Update ANBR config
    EXPECT_CALL(m_objListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_UPDATE_ANBR_ENABLED_CONFIG, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pController->UpdateAnbrEnabledConfig(NEGO_ID, IMS_TRUE));

    // Call again with same value, should not send message
    EXPECT_TRUE(m_pController->UpdateAnbrEnabledConfig(NEGO_ID, IMS_TRUE));

    // Update ANBR config to disabled
    EXPECT_CALL(m_objListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_UPDATE_ANBR_ENABLED_CONFIG, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pController->UpdateAnbrEnabledConfig(NEGO_ID, IMS_FALSE));
}

TEST_F(AudioControllerTest, testNotifyAnbrReceived)
{
    // No sessions, should return false
    EXPECT_FALSE(m_pController->NotifyAnbrReceived(MEDIA_TYPE_AUDIO, 0, 0));

    // Create a session, but it's not live
    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);
    EXPECT_FALSE(m_pController->NotifyAnbrReceived(MEDIA_TYPE_AUDIO, 0, 0));

    // Make the session live by updating it
    m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego);

    // Now it should try to notify, but the underlying session method will return false
    EXPECT_FALSE(m_pController->NotifyAnbrReceived(MEDIA_TYPE_AUDIO, 0, 0));
}

TEST_F(AudioControllerTest, testHandleForkedSessionUpdate)
{
    const IMS_UINTP NEGO_ID_1 = 1;
    const IMS_UINTP NEGO_ID_2 = 2;

    // Create two sessions to simulate a forked call
    ASSERT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID_1, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));
    ASSERT_TRUE(m_pController->CreateSession(
            &m_objListener, NEGO_ID_2, m_pConfig.get(), MEDIA_SERVICE_DEFAULT));

    // Simulate that the previous session (session 1) is active
    AudioSession* pSession1 = m_pController->FindAudioSession(NEGO_ID_1);
    ASSERT_NE(pSession1, nullptr);
    pSession1->SetMediaPemType(MEDIA_PEM_TYPE::SENDRECV);

    AudioConfig* pConfig = static_cast<AudioConfig*>(pSession1->GetRtpConfig());
    ASSERT_NE(pConfig, nullptr);
    pConfig->setRemotePort(5004);

    EXPECT_CALL(
            m_objListener, MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_MODIFY_SESSION, _))
            .Times(1)
            .WillOnce(Return(true));

    m_pController->HandleForkedSessionUpdate(NEGO_ID_1);
}

TEST_F(AudioControllerTest, testUpdateMediaDirection)
{
    // No sessions, should return false
    EXPECT_FALSE(m_pController->UpdateMediaDirection(MEDIA_DIRECTION_SEND, false));

    // Create a session, but not live
    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);
    EXPECT_FALSE(m_pController->UpdateMediaDirection(MEDIA_DIRECTION_SEND, false));

    // Make the session live by updating it
    m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego);

    EXPECT_TRUE(m_pController->UpdateMediaDirection(MEDIA_DIRECTION_SEND, false));
    EXPECT_TRUE(m_pController->UpdateMediaDirection(MEDIA_DIRECTION_SEND, true));
}

TEST_F(AudioControllerTest, testSetCallSessionState)
{
    m_pController->SetCallSessionState(true);   // CONFIRMED
    m_pController->SetCallSessionState(false);  // EARLY
}

TEST_F(AudioControllerTest, testModifySessionFail)
{
    EXPECT_FALSE(m_pController->ModifySession(NEGO_ID));
}

TEST_F(AudioControllerTest, testSetMediaQualityFail)
{
    EXPECT_FALSE(m_pController->SetMediaQuality(NEGO_ID));
}

TEST_F(AudioControllerTest, testUpdateRtpConfigFail)
{
    EXPECT_FALSE(m_pController->UpdateRtpConfig(NEGO_ID, ACCESS_NETWORK, nullptr));
    EXPECT_FALSE(m_pController->UpdateRtpConfig(999, ACCESS_NETWORK, m_pAudioNego));
}

TEST_F(AudioControllerTest, testUpdateSessionFail)
{
    EXPECT_FALSE(m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, nullptr));

    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);
    ON_CALL(*m_pAudioNego, GetNegotiatedNegoProfile()).WillByDefault(Return(nullptr));
    EXPECT_FALSE(m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego));
}

TEST_F(AudioControllerTest, testUpdateSessionReadyToConfirm)
{
    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);
    m_pController->CreateSession(
            &m_objListener, NEGO_ID + 1, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);

    m_pController->SetCallSessionState(AudioController::READY_TO_CONFIRM);

    EXPECT_TRUE(m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego));
}

TEST_F(AudioControllerTest, testUpdateSessionNoRtpConfigChange)
{
    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);
    m_pController->SetCallSessionState(true);
    // First call to set the initial config
    m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego);
    // Second call, config should not have changed
    EXPECT_TRUE(m_pController->UpdateSession(NEGO_ID, ACCESS_NETWORK, m_pAudioNego));
}

TEST_F(AudioControllerTest, RequestRtpReceptionStats)
{
    // Arrange
    const IMS_UINT32 intervalMs = 3000;

    // Create a session, which will be a MockAudioSession due to the override
    m_pController->CreateSession(&m_objListener, NEGO_ID, m_pConfig.get(), MEDIA_SERVICE_DEFAULT);
    m_pController->SetCallSessionState(true);

    // Act
    EXPECT_TRUE(m_pController->RequestRtpReceptionStats(NEGO_ID, intervalMs));
}

TEST_F(AudioControllerTest, RequestRtpReceptionStats_InvalidNegoId)
{
    // Arrange: No session is added, or a session with a different negoId is added.
    // Act & Assert
    EXPECT_FALSE(m_pController->RequestRtpReceptionStats(999, 3000));
}
