/**
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
#include <ServiceConfig.h>
#include <IJniMedia.h>
#include <audio/AudioController.h>
#include <audio/MockAudioNego.h>
#include <MediaNetworkConnectionWatcher.h>
#include <MockIMediaSessionListener.h>

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_UINT32 LOCAL_PORT = 20000;
const IMS_UINT32 ACCESS_NETWORK = MediaNetworkConnectionWatcher::EUTRAN;

class FakeAudioController : public AudioController
{
public:
    FakeAudioController() :
            AudioController()
    {
    }

    virtual ~FakeAudioController() {}

    IMS_BOOL IsAudioConfigChanged(IN AudioConfig* pAudioConfig) override
    {
        return AudioController::IsAudioConfigChanged(pAudioConfig);
    }
};

class AudioControllerTest : public ::testing::Test
{
public:
    FakeAudioController* m_pController;
    AudioConfiguration* m_pConfig;
    FakeIMediaSessionListener m_objFakeListener;
    MockIMediaSessionListener m_objListener;
    std::shared_ptr<MockAudioNego> m_pAudioNego;

    AudioProfile* m_pLocalProfile;
    AudioProfile* m_pPeerProfile;
    AudioProfile* m_pNegoProfile;
    IpAddress m_objIpAddr;

protected:
    virtual void SetUp() override
    {
        m_pController = new FakeAudioController();
        m_pConfig = new AudioConfiguration(MEDIA_TYPE_AUDIO);
        m_pConfig->Create(ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID));
        m_pAudioNego = std::make_shared<MockAudioNego>(DEFAULT_SLOT_ID);

        m_objListener.SetDelegate(&m_objFakeListener);
        m_objListener.DelegateToFake();

        m_pLocalProfile = new AudioProfile();
        AudioProfile::Payload* pSrcAmrPayload = new AudioProfile::Payload();
        pSrcAmrPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
        pSrcAmrPayload->SetFmtp(new AudioProfile::AmrFmtp());
        m_pLocalProfile->GetPayloadList().Append(pSrcAmrPayload);
        m_pLocalProfile->SetDataPort(LOCAL_PORT);

        m_pPeerProfile = new AudioProfile(*m_pLocalProfile);
        m_pNegoProfile = new AudioProfile(*m_pLocalProfile);

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pAudioNego, GetLocalAddress()).WillByDefault(ReturnRef(m_objIpAddr));
        ON_CALL(*m_pAudioNego, GetLocalPort()).WillByDefault(Return(LOCAL_PORT));
        ON_CALL(*m_pAudioNego, GetNegotiatedLocalProfile()).WillByDefault(Return(m_pLocalProfile));
        ON_CALL(*m_pAudioNego, GetNegotiatedPeerProfile()).WillByDefault(Return(m_pPeerProfile));
        ON_CALL(*m_pAudioNego, GetNegotiatedNegoProfile()).WillByDefault(Return(m_pNegoProfile));
    }

    virtual void TearDown() override
    {
        delete m_pController;
        delete m_pConfig;
        delete m_pLocalProfile;
        delete m_pPeerProfile;
        delete m_pNegoProfile;
    }
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
    EXPECT_EQ(m_pController->DeleteSession(0), IMS_FALSE);
    EXPECT_EQ(m_pController->DeleteSession(100), IMS_FALSE);
}

TEST_F(AudioControllerTest, testUpdateLocalAddressFail)
{
    EXPECT_EQ(m_pController->UpdateLocalAddress(nullptr), IMS_FALSE);
}

TEST_F(AudioControllerTest, testOpenSessionFail)
{
    IMS_UINTP nNegoId = 1000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
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
    IMS_UINTP nNegoId = 1000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    EXPECT_EQ(m_pController->CloseSession(), IMS_FALSE);
}

TEST_F(AudioControllerTest, testDeleteSessionFailWithOneSession)
{
    IMS_UINTP negoId = 1000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, negoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(negoId), IMS_TRUE);

    // Attempt to delete the only session
    EXPECT_EQ(m_pController->DeleteSession(negoId), IMS_FALSE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    m_pController->CloseSession();
}

TEST_F(AudioControllerTest, testDeleteSessionSuccess)
{
    IMS_UINTP negoId1 = 1000;
    IMS_UINTP negoId2 = 2000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, negoId1, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, negoId2, m_pConfig, MEDIA_SERVICE_DEFAULT),
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
    IMS_UINTP nNegoId = 1000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId), IMS_TRUE);

    m_pController->SetCallSessionState(IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(2000, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(nNegoId, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);

    EXPECT_EQ(m_pController->SendDtmf('1'), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
}

TEST_F(AudioControllerTest, testAddSession)
{
    IMS_UINTP nNegoId = 1000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId), IMS_TRUE);

    EXPECT_EQ(m_pController->AddSession(2000, ACCESS_NETWORK, m_pAudioNego), IMS_FALSE);

    EXPECT_EQ(m_pController->CreateSession(&m_objListener, 2000, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 2);

    EXPECT_EQ(m_pController->AddSession(2000, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);
}

TEST_F(AudioControllerTest, testConfirmSession)
{
    IMS_UINTP nNegoId = 1000;
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId), IMS_TRUE);

    EXPECT_EQ(m_pController->ConfirmSession(2000), IMS_FALSE);

    EXPECT_EQ(m_pController->CreateSession(&m_objListener, 2000, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 2);

    EXPECT_EQ(m_pController->ConfirmSession(2000), IMS_TRUE);
}

TEST_F(AudioControllerTest, testUpdateQualityThreshold)
{
    IMS_UINTP nNegoId = 1000;

    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 1);

    EXPECT_EQ(m_pController->UpdateQualityThreshold(nNegoId, nullptr), IMS_FALSE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(UNDEFINED_NEGO_ID, m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(nNegoId, m_pAudioNego), IMS_TRUE);
}

TEST_F(AudioControllerTest, testInactivityTimer)
{
    IMS_UINTP nNegoId1 = 1000;
    IMS_UINTP nNegoId2 = 2000;
    IMS_UINT32 inactivityTime1 = 1111;
    IMS_UINT32 inactivityTime2 = 2222;
    IMS_UINT32 inactivityTime3 = 3333;

    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId1, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId2, m_pConfig, MEDIA_SERVICE_DEFAULT),
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
    IMS_UINTP nNegoId = 1000;
    // Initial state: No session
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Create session
    EXPECT_EQ(
            m_pController->CreateSession(&m_objListener, nNegoId, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    // Session created but not opened (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Open session
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId), IMS_TRUE);
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
    IMS_UINTP nNegoId1 = 1000;
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId1, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateAccessNetwork(NEW_ACCESS_NETWORK), IMS_FALSE);
}

TEST_F(AudioControllerTest, testUpdateAccessNetworkSuccess)
{
    const IMS_UINT32 NEW_ACCESS_NETWORK = ACCESS_NETWORK + 1;

    // case 1: One session, created and opened
    IMS_UINTP nNegoId2 = 2000;
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId2, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);

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
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId3, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);
    EXPECT_EQ(m_pController->CreateSession(
                      &m_objListener, nNegoId4, m_pConfig, MEDIA_SERVICE_DEFAULT),
            IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pAudioNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(nNegoId4), IMS_TRUE);

    // Set up the session state via UpdateSession
    m_pController->SetCallSessionState(IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(nNegoId4, ACCESS_NETWORK, m_pAudioNego), IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateAccessNetwork(NEW_ACCESS_NETWORK + 1), IMS_TRUE);

    m_pController->CloseSession();
    EXPECT_EQ(m_pController->GetAudioSessionSize(), 0);
}
