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
#include <MediaDef.h>
#include <video/VideoSession.h>
#include <video/VideoProfile.h>
#include <config/VideoConfiguration.h>

#include <MockIMediaSessionListener.h>

using namespace android::telephony::imsmedia;
using ::testing::_;
using ::testing::Return;

class VideoSessionTest : public ::testing::Test
{
public:
    std::unique_ptr<VideoSession> m_pSession;
    std::unique_ptr<VideoConfiguration> m_pVideoConfig;
    MockIMediaSessionListener m_objMockListener;
    VideoProfile m_objLocalProfile;
    VideoProfile m_objPeerProfile;
    VideoProfile m_objNegoProfile;

protected:
    virtual void SetUp() override
    {
        m_pSession = std::unique_ptr<VideoSession>(new VideoSession());
        m_pVideoConfig = std::unique_ptr<VideoConfiguration>(new VideoConfiguration());
        m_pSession->SetConfiguration(m_pVideoConfig.get());
        m_pSession->SetMediaSessionListener(&m_objMockListener);

        VideoProfile::Payload* pLocalPayload = new VideoProfile::Payload();
        VideoProfile::AvcFmtp* pLocalFmtp = new VideoProfile::AvcFmtp();
        pLocalPayload->SetRtpMap(100, "H264", 90000, 1);
        pLocalPayload->SetFmtp(pLocalFmtp);

        VideoProfile::Payload* pPeerPayload = new VideoProfile::Payload();
        VideoProfile::AvcFmtp* pPeerFmtp = new VideoProfile::AvcFmtp();
        pPeerPayload->SetRtpMap(100, "H264", 90000, 1);
        pPeerPayload->SetFmtp(pPeerFmtp);

        VideoProfile::Payload* pNegoPayload = new VideoProfile::Payload();
        VideoProfile::AvcFmtp* pNegoFmtp = new VideoProfile::AvcFmtp();
        pNegoPayload->SetRtpMap(100, "H264", 90000, 1);
        pNegoPayload->SetFmtp(pNegoFmtp);

        m_objLocalProfile.GetPayloadList().Append(pLocalPayload);
        m_objNegoProfile.GetPayloadList().Append(pNegoPayload);
        m_objPeerProfile.GetPayloadList().Append(pPeerPayload);
        m_objLocalProfile.SetDataPort(10000);
        m_objPeerProfile.SetDataPort(10000);
        m_objNegoProfile.SetDataPort(10000);
    }

    virtual void TearDown() override {}
};

TEST_F(VideoSessionTest, testSetDirection)
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

TEST_F(VideoSessionTest, testAccessNetwork)
{
    const IMS_UINTP NETWORK = 0x11;
    EXPECT_TRUE(m_pSession->SetAccessNetwork(NETWORK));
    ASSERT_TRUE(m_pSession->GetRtpConfig() != IMS_NULL);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAccessNetwork(), NETWORK);
}

TEST_F(VideoSessionTest, testSetAnbrMode)
{
    AnbrMode objAnbrMode;
    m_pSession->SetAnbrMode(objAnbrMode);
    EXPECT_EQ(m_pSession->GetRtpConfig()->getAnbrMode(), objAnbrMode);
}

TEST_F(VideoSessionTest, testUpdateMediaQualityThreshold)
{
    EXPECT_TRUE(m_pSession->UpdateMediaQualityThreshold(IMS_TRUE, IMS_TRUE, IMS_TRUE));
    EXPECT_TRUE(m_pSession->UpdateMediaQualityThreshold(IMS_FALSE, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoSessionTest, testUpdateRtpConfigPortZero)
{
    m_objNegoProfile.SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_objNegoProfile.SetDataPort(0);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_TRUE));

    EXPECT_EQ(m_pSession->GetRtpConfig()->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_NO_FLOW);

    m_objNegoProfile.SetDataPort(10000);
    m_objLocalProfile.SetDataPort(0);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_TRUE));

    EXPECT_EQ(m_pSession->GetRtpConfig()->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_NO_FLOW);
}

TEST_F(VideoSessionTest, testUpdateRtpConfigSendRecv)
{
    m_objNegoProfile.SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_TRUE));

    EXPECT_EQ(m_pSession->GetRtpConfig()->getMediaDirection(),
            RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoSessionTest, testUpdateRtpConfigSendOnlyHoldOff)
{
    m_objNegoProfile.SetDirection(MEDIA_DIRECTION_SEND);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_FALSE));

    EXPECT_EQ(
            m_pSession->GetRtpConfig()->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_SEND_ONLY);
}

TEST_F(VideoSessionTest, testUpdateRtpConfigSendOnlyHoldOn)
{
    m_objNegoProfile.SetDirection(MEDIA_DIRECTION_SEND);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_TRUE));

    EXPECT_EQ(m_pSession->GetRtpConfig()->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_INACTIVE);
}

TEST_F(VideoSessionTest, testUpdateRtpConfigRecvOnlyHoldOff)
{
    m_objNegoProfile.SetDirection(MEDIA_DIRECTION_RECEIVE);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_FALSE));

    EXPECT_EQ(m_pSession->GetRtpConfig()->getMediaDirection(),
            RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
}

TEST_F(VideoSessionTest, testUpdateRtpConfigRecvOnlyHoldOn)
{
    m_objNegoProfile.SetDirection(MEDIA_DIRECTION_SEND);

    EXPECT_TRUE(m_pSession->UpdateRtpConfig(
            &m_objLocalProfile, &m_objPeerProfile, &m_objNegoProfile, IMS_TRUE, IMS_TRUE));

    EXPECT_EQ(m_pSession->GetRtpConfig()->getMediaDirection(), RtpConfig::MEDIA_DIRECTION_INACTIVE);
}

TEST_F(VideoSessionTest, testSetMtu)
{
    const IMS_UINT32 MTU = 1400;
    m_pSession->SetMtu(MTU);
    ASSERT_TRUE(m_pSession->GetRtpConfig() != IMS_NULL);
    VideoConfig* pVideoConfig = reinterpret_cast<VideoConfig*>(m_pSession->GetRtpConfig());
    EXPECT_EQ(pVideoConfig->getMaxMtuBytes(), MTU);
}

TEST_F(VideoSessionTest, testOpen)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_OPEN_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Open());
    EXPECT_EQ(m_pSession->GetState(), VideoSession::STATE_IDLE);
}

TEST_F(VideoSessionTest, testModify)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_MODIFY_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    m_pSession->SetState(VideoSession::STATE_IDLE);
    VideoConfig* pVideoConfig = reinterpret_cast<VideoConfig*>(m_pSession->GetRtpConfig());
    pVideoConfig->setVideoMode(VideoConfig::VIDEO_MODE_PREVIEW);
    EXPECT_TRUE(m_pSession->Modify());
    EXPECT_EQ(m_pSession->GetState(), VideoSession::STATE_PREVIEW);
}

TEST_F(VideoSessionTest, testClose)
{
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_CLOSE_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->Close());
    EXPECT_EQ(m_pSession->GetState(), VideoSession::STATE_NONE);
}

TEST_F(VideoSessionTest, testSetMediaQuality)
{
    m_pSession->SetState(VideoSession::STATE_IDLE);
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_SET_MEDIA_QUALITY, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pSession->SetMediaQuality());
}

TEST_F(VideoSessionTest, testOnSetSurfaceCmd)
{
    m_pSession->SetState(VideoSession::STATE_IDLE);
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_SET_DISPLAY_SURFACE, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_SET_PREVIEW_SURFACE, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    ImsMediaVideoParam objDisplayParam(SURFACE_FAR);
    EXPECT_TRUE(m_pSession->OnMessages(
            IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(&objDisplayParam)));
    ImsMediaVideoParam objPreviewParam(SURFACE_NEAR);
    EXPECT_TRUE(m_pSession->OnMessages(
            IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(&objPreviewParam)));
}

TEST_F(VideoSessionTest, testOnSelectCameraCmd)
{
    m_pSession->SetState(VideoSession::STATE_IDLE);
    CONST IMS_UINT32 CAMERA_ID_FRONT = 1;
    EXPECT_CALL(m_objMockListener,
            MediaSession_SendMsgToMediaManager(IJniMedia::REQUEST_MODIFY_SESSION, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    ImsMediaVideoParam objSelectCameraParam(CAMERA_ID_FRONT);
    EXPECT_TRUE(m_pSession->OnMessages(
            IJniMedia::SELECT_CAMERA_CMD, reinterpret_cast<IMS_UINTP>(&objSelectCameraParam)));
    EXPECT_EQ(m_pSession->GetCameraId(), CAMERA_ID_FRONT);
}

TEST_F(VideoSessionTest, testOnChangeCameraZoomCmd)
{
    ImsMediaVideoParam objZoomParam(100);
    EXPECT_TRUE(m_pSession->OnMessages(
            IJniMedia::CHANGE_CAMERA_ZOOM_CMD, reinterpret_cast<IMS_UINTP>(&objZoomParam)));
}

TEST_F(VideoSessionTest, testOnChangeOrientation)
{
    ImsMediaVideoParam objOrientationParam(90);
    EXPECT_TRUE(m_pSession->OnMessages(
            IJniMedia::CHANGE_ORIENTATION_CMD, reinterpret_cast<IMS_UINTP>(&objOrientationParam)));
}
