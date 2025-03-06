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
#include <IJniMedia.h>
#include <text/TextSession.h>
#include <text/TextProfile.h>
#include <config/TextConfiguration.h>
#include <MockIMediaSessionListener.h>

using namespace android::telephony::imsmedia;
using ::testing::_;
using ::testing::Return;

class TextSessionTest : public ::testing::Test
{
public:
    std::unique_ptr<TextSession> m_pSession;
    std::unique_ptr<TextConfiguration> m_pTextConfig;
    MockIMediaSessionListener m_objMockListener;

protected:
    virtual void SetUp() override
    {
        m_pSession = std::unique_ptr<TextSession>(new TextSession());
        m_pTextConfig = std::unique_ptr<TextConfiguration>(new TextConfiguration());
        m_pSession->SetConfiguration(m_pTextConfig.get());
        m_pSession->SetMediaSessionListener(&m_objMockListener);
    }

    virtual void TearDown() override {}
};

TEST_F(TextSessionTest, testSetDirection)
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

TEST_F(TextSessionTest, testAccessNetwork)
{
    const IMS_UINTP NETWORK = 0x11;
    EXPECT_TRUE(m_pSession->SetAccessNetwork(NETWORK));
    ASSERT_TRUE(m_pSession->GetRtpConfig() != IMS_NULL);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAccessNetwork(), NETWORK);
}

TEST_F(TextSessionTest, testSetAnbrMode)
{
    AnbrMode objAnbrMode;
    m_pSession->SetAnbrMode(objAnbrMode);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAnbrMode(), objAnbrMode);
}

TEST_F(TextSessionTest, testUpdateMediaQualityThreshold)
{
    EXPECT_TRUE(m_pSession->UpdateMediaQualityThreshold(IMS_TRUE, IMS_TRUE));
    EXPECT_TRUE(m_pSession->UpdateMediaQualityThreshold(IMS_FALSE, IMS_FALSE));
}

TEST_F(TextSessionTest, testUpdateRtpConfig)
{
    TextProfile objLocalProfile;
    TextProfile objPeerProfile;
    TextProfile objNegoProfile;
    TextProfile::Payload* pLocalPayload = new TextProfile::Payload();
    TextProfile::RedFmtp* pLocalFmtp = new TextProfile::RedFmtp(3, 101);
    pLocalPayload->SetRtpMap(100, "RED", 1000, 1);
    pLocalPayload->SetFmtp(pLocalFmtp);

    TextProfile::Payload* pPeerPayload = new TextProfile::Payload();
    TextProfile::RedFmtp* pPeerFmtp = new TextProfile::RedFmtp(3, 101);
    pPeerPayload->SetRtpMap(100, "RED", 1000, 1);
    pPeerPayload->SetFmtp(pPeerFmtp);

    TextProfile::Payload* pNegoPayload = new TextProfile::Payload();
    TextProfile::RedFmtp* pNegoFmtp = new TextProfile::RedFmtp(3, 101);
    pNegoPayload->SetRtpMap(100, "RED", 1000, 1);
    pNegoPayload->SetFmtp(pNegoFmtp);

    objLocalProfile.GetPayloadList().Append(pLocalPayload);
    objNegoProfile.GetPayloadList().Append(pNegoPayload);
    objPeerProfile.GetPayloadList().Append(pPeerPayload);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(&objLocalProfile, &objPeerProfile, &objNegoProfile));
}

TEST_F(TextSessionTest, testOpen)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_OPEN_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Open());
    EXPECT_EQ(m_pSession->GetState(), TextSession::STATE_IDLE);
}

TEST_F(TextSessionTest, testModify)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_MODIFY_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    m_pSession->SetState(TextSession::STATE_IDLE);
    EXPECT_TRUE(m_pSession->Modify());
    EXPECT_EQ(m_pSession->GetState(), TextSession::STATE_LIVE);
}

TEST_F(TextSessionTest, testClose)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_CLOSE_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Close());
    EXPECT_EQ(m_pSession->GetState(), TextSession::STATE_NONE);
}

TEST_F(TextSessionTest, testSetMediaQuality)
{
    m_pSession->SetState(TextSession::STATE_IDLE);
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_SET_MEDIA_QUALITY, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->SetMediaQuality());
}
