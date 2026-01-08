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
#include <audio/AudioSession.h>
#include <config/MockAudioConfiguration.h>
#include <MockIMediaSessionListener.h>

using namespace android::telephony::imsmedia;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class AudioSessionTest : public ::testing::Test
{
public:
    std::unique_ptr<AudioSession> m_pSession;
    NiceMock<MockAudioConfiguration>* m_pAudioConfig;
    MockIMediaSessionListener m_objMockListener;

protected:
    virtual void SetUp() override
    {
        m_pSession = std::unique_ptr<AudioSession>(new AudioSession());
        m_pAudioConfig = new NiceMock<MockAudioConfiguration>();
        m_pSession->SetConfiguration(m_pAudioConfig);
        m_pSession->SetMediaSessionListener(&m_objMockListener);
    }

    virtual void TearDown() override {}
};

TEST_F(AudioSessionTest, testSetGetInactivityTimer)
{
    IMS_UINT32 INACTIVITY_TIME = 12000;

    m_pSession->SetNetworkToneTimer(INACTIVITY_TIME);
    EXPECT_EQ(m_pSession->GetInactivityTimer(NETWORK_TONE_INACTIVITY), INACTIVITY_TIME);
    m_pSession->SetNetworkToneTimer(0);
}

TEST_F(AudioSessionTest, testSetDirection)
{
    m_pSession->SetDirection(MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_INVALID);
    m_pSession->SetDirection(MEDIA_DIRECTION_SEND);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_SEND);
    m_pSession->SetDirection(MEDIA_DIRECTION_RECEIVE);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_RECEIVE);
    m_pSession->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    m_pSession->SetDirection(MEDIA_DIRECTION_INACTIVE);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(AudioSessionTest, testSetNegoId)
{
    const IMS_UINTP NEGO_ID = 0x1234;
    m_pSession->SetNegoId(NEGO_ID);
    EXPECT_TRUE(m_pSession->IsSameNegoId(NEGO_ID));
    EXPECT_FALSE(m_pSession->IsSameNegoId(0x3456));
}

TEST_F(AudioSessionTest, testAccessNetwork)
{
    const IMS_UINTP NETWORK = 0x11;
    EXPECT_TRUE(m_pSession->SetAccessNetwork(NETWORK));
    ASSERT_TRUE(m_pSession->GetRtpConfig() != IMS_NULL);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAccessNetwork(), NETWORK);
}

TEST_F(AudioSessionTest, testSetAnbrMode)
{
    AnbrMode objAnbrMode;
    m_pSession->SetAnbrMode(objAnbrMode);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAnbrMode(), objAnbrMode);
}

TEST_F(AudioSessionTest, testUpdateRtpConfig)
{
    AudioProfile objLocalProfile;
    AudioProfile objPeerProfile;
    AudioProfile objNegoProfile;
    AudioProfile::Payload* pLocalPayload = new AudioProfile::Payload();
    std::shared_ptr<AudioProfile::EvsFmtp> pLocalFmtp = std::make_shared<AudioProfile::EvsFmtp>();
    pLocalPayload->SetRtpMap(100, "EVS", 16000, 1);
    pLocalPayload->SetFmtp(pLocalFmtp);

    AudioProfile::Payload* pPeerPayload = new AudioProfile::Payload();
    std::shared_ptr<AudioProfile::EvsFmtp> pPeerFmtp = std::make_shared<AudioProfile::EvsFmtp>();
    pPeerPayload->SetRtpMap(100, "EVS", 16000, 1);
    pPeerPayload->SetFmtp(pPeerFmtp);

    AudioProfile::Payload* pNegoPayload = new AudioProfile::Payload();
    std::shared_ptr<AudioProfile::EvsFmtp> pNegoFmtp = std::make_shared<AudioProfile::EvsFmtp>();
    pNegoPayload->SetRtpMap(100, "EVS", 16000, 1);
    pNegoPayload->SetFmtp(pNegoFmtp);

    objLocalProfile.AddPayload(pLocalPayload);
    objNegoProfile.AddPayload(pNegoPayload);
    objPeerProfile.AddPayload(pPeerPayload);

    EXPECT_NE(m_pSession->UpdateRtpConfig(
                      0, &objLocalProfile, &objPeerProfile, &objNegoProfile, IMS_TRUE),
            IMS_NULL);
}

TEST_F(AudioSessionTest, testUpdateEarlyMediaDirection)
{
    // Common setup for all test cases in this test
    AudioProfile localProfile, peerProfile, negoProfile;
    auto negoPayload = new AudioProfile::Payload();
    negoPayload->SetRtpMap(100, "EVS", 16000, 1);
    negoPayload->SetFmtp(std::make_shared<AudioProfile::EvsFmtp>());
    negoProfile.AddPayload(negoPayload);
    negoProfile.SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    auto peerPayload = new AudioProfile::Payload();
    peerPayload->SetRtpMap(100, "EVS", 16000, 1);
    peerPayload->SetFmtp(std::make_shared<AudioProfile::EvsFmtp>());
    peerProfile.AddPayload(peerPayload);

    // Case 1: PEM type is SENDRECV -> should be SEND_RECEIVE
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::SENDRECV);
        AudioConfig* pConfig = m_pSession->UpdateRtpConfig(
                0, &localProfile, &peerProfile, &negoProfile, IMS_FALSE);
        ASSERT_NE(pConfig, nullptr);
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
    }

    // Case 2: PEM type is SENDONLY -> should be RECEIVE_ONLY
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::SENDONLY);
        AudioConfig* pConfig = m_pSession->UpdateRtpConfig(
                0, &localProfile, &peerProfile, &negoProfile, IMS_FALSE);
        ASSERT_NE(pConfig, nullptr);
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
    }

    // Case 3: PEM type is RECVONLY -> should be RECEIVE_ONLY
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::RECVONLY);
        AudioConfig* pConfig = m_pSession->UpdateRtpConfig(
                0, &localProfile, &peerProfile, &negoProfile, IMS_FALSE);
        ASSERT_NE(pConfig, nullptr);
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_SEND_ONLY);
    }

    // Case 4: PEM type is INACTIVE -> should be RECEIVE_ONLY
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::INACTIVE);
        AudioConfig* pConfig = m_pSession->UpdateRtpConfig(
                0, &localProfile, &peerProfile, &negoProfile, IMS_FALSE);
        ASSERT_NE(pConfig, nullptr);
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
    }

    // Case 5: PEM type is NONE, and RecvOnlyEarlySession is enabled -> should be RECEIVE_ONLY
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::NONE);
        EXPECT_CALL(*m_pAudioConfig, IsRecvOnlyEarlySessionEnabled()).WillOnce(Return(true));
        AudioConfig* pConfig = m_pSession->UpdateRtpConfig(
                0, &localProfile, &peerProfile, &negoProfile, IMS_FALSE);
        ASSERT_NE(pConfig, nullptr);
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
    }

    // Case 6: PEM type is NONE, and RecvOnlyEarlySession is disabled -> should be SEND_RECEIVE
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::NONE);
        EXPECT_CALL(*m_pAudioConfig, IsRecvOnlyEarlySessionEnabled()).WillOnce(Return(false));
        AudioConfig* pConfig = m_pSession->UpdateRtpConfig(
                0, &localProfile, &peerProfile, &negoProfile, IMS_FALSE);
        ASSERT_NE(pConfig, nullptr);
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
    }

    // Case 7: Confirmed session, should not call UpdateEarlyMediaDirection
    {
        m_pSession->SetMediaPemType(MEDIA_PEM_TYPE::SENDONLY);  // This should be ignored
        AudioConfig* pConfig =
                m_pSession->UpdateRtpConfig(0, &localProfile, &peerProfile, &negoProfile, IMS_TRUE);
        ASSERT_NE(pConfig, nullptr);
        // Direction should remain as negotiated (SEND_RECEIVE)
        EXPECT_EQ(pConfig->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
    }
}

TEST_F(AudioSessionTest, testUpdateMediaQualityThreshold)
{
    EXPECT_TRUE(m_pSession->UpdateMediaQualityThreshold(IMS_TRUE, IMS_TRUE, IMS_TRUE));
    EXPECT_TRUE(m_pSession->UpdateMediaQualityThreshold(IMS_FALSE, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioSessionTest, testGetEnabledRtcp)
{
    EXPECT_FALSE(m_pSession->GetEnabledRtcp());
}

TEST_F(AudioSessionTest, testOpen)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_OPEN_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Open());
}

TEST_F(AudioSessionTest, testModify)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_MODIFY_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Modify());
}

TEST_F(AudioSessionTest, testAdd)
{
    EXPECT_CALL(
            m_objMockListener, MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_ADD_CONFIG, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Add());
}

TEST_F(AudioSessionTest, testDelete)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_DELETE_CONFIG, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Delete());
}

TEST_F(AudioSessionTest, testConfirm)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_CONFIRM_CONFIG, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Confirm());
}

TEST_F(AudioSessionTest, testClose)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_CLOSE_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Close());
}

TEST_F(AudioSessionTest, testSendDtmf)
{
    EXPECT_CALL(
            m_objMockListener, MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_SEND_DTMF, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->SendDtmf('1'));
}

TEST_F(AudioSessionTest, testUpdateAnbrEnabledConfig)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_UPDATE_ANBR_ENABLED_CONFIG, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->UpdateAnbrEnabledConfig(IMS_TRUE));
}

TEST_F(AudioSessionTest, testIsAnbrEnabled)
{
    // Initially, ANBR should be disabled.
    EXPECT_FALSE(m_pSession->IsAnbrEnabled());

    // Enable ANBR
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_UPDATE_ANBR_ENABLED_CONFIG, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->UpdateAnbrEnabledConfig(IMS_TRUE));

    // Check if ANBR is enabled.
    EXPECT_TRUE(m_pSession->IsAnbrEnabled());

    // Disable ANBR
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_UPDATE_ANBR_ENABLED_CONFIG, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pSession->UpdateAnbrEnabledConfig(IMS_FALSE));

    // Check if ANBR is disabled.
    EXPECT_FALSE(m_pSession->IsAnbrEnabled());
}

TEST_F(AudioSessionTest, testNotifyAnbrReceived)
{
    EXPECT_FALSE(m_pSession->NotifyAnbrReceived(0, 0, 0));
}

TEST_F(AudioSessionTest, testSetMediaQuality)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_SET_MEDIA_QUALITY, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->SetMediaQuality(IMS_TRUE));
}

TEST_F(AudioSessionTest, RequestRtpReceptionStats)
{
    // Arrange
    const int32_t intervalMs = 3000;

    // Expect a message to be sent to the media manager
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_RTP_RECEPTION_STATS,
                    testing::Truly(
                            [](ImsMediaMsgParamBase* p)
                            {
                                const auto param =
                                        static_cast<ImsMediaMsgRtpReceptionStatsParam*>(p);
                                return param && param->m_nIntervalMs == intervalMs;
                            })))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // Act & Assert
    EXPECT_TRUE(m_pSession->RequestRtpReceptionStats(intervalMs));
}
