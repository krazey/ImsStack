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
#include <ServiceConfig.h>
#include <video/VideoController.h>
#include <video/MockVideoNego.h>
#include <MockIMediaSessionListener.h>

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_UINT32 LOCAL_PORT = 20000;

class VideoControllerTest : public ::testing::Test
{
public:
    VideoController* m_pController;
    VideoConfiguration* m_pConfig;
    FakeIMediaSessionListener m_objFakeListener;
    MockIMediaSessionListener m_objListener;
    std::shared_ptr<MockVideoNego> m_pVideoNego;

    VideoProfile* m_pLocalProfile;
    VideoProfile* m_pPeerProfile;
    VideoProfile* m_pNegoProfile;
    IpAddress m_objIpAddress;

protected:
    virtual void SetUp() override
    {
        m_pController = new VideoController();
        m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);
        m_pConfig->Create(ConfigService::GetConfigService()->GetCarrierConfig(DEFAULT_SLOT_ID));
        m_pVideoNego = std::make_shared<MockVideoNego>(DEFAULT_SLOT_ID);

        m_objListener.SetDelegate(&m_objFakeListener);
        m_objListener.DelegateToFake();

        m_pLocalProfile = new VideoProfile();
        m_pLocalProfile->SetTransportType("RTP/AVP");
        auto pAvcFmtp = std::make_shared<VideoProfile::AvcFmtp>();
        VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
        pAvcPayload->GetRtpMap().SetPayloadType("H264");
        pAvcPayload->SetFmtp(pAvcFmtp);
        m_pLocalProfile->GetPayloadList().Append(pAvcPayload);
        m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

        m_pPeerProfile = new VideoProfile(*m_pLocalProfile);
        m_pNegoProfile = new VideoProfile(*m_pLocalProfile);

        m_objIpAddress = IpAddress(LOCAL_IP);
        ON_CALL(*m_pVideoNego, GetLocalAddress()).WillByDefault(ReturnRef(m_objIpAddress));
        ON_CALL(*m_pVideoNego, GetLocalPort()).WillByDefault(Return(LOCAL_PORT));
        ON_CALL(*m_pVideoNego, GetNegotiatedLocalProfile()).WillByDefault(Return(m_pLocalProfile));
        ON_CALL(*m_pVideoNego, GetNegotiatedPeerProfile()).WillByDefault(Return(m_pPeerProfile));
        ON_CALL(*m_pVideoNego, GetNegotiatedNegoProfile()).WillByDefault(Return(m_pNegoProfile));
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

TEST_F(VideoControllerTest, testCreateSessionFail)
{
    EXPECT_EQ(m_pController->CreateSession(nullptr, nullptr), IMS_FALSE);
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, nullptr), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateRtpConfigWithNoSession)
{
    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pVideoNego, IMS_FALSE), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateLocalAddressFail)
{
    EXPECT_EQ(m_pController->UpdateLocalAddress(nullptr), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateQualityThresholdWithNoSession)
{
    EXPECT_EQ(m_pController->ApplyQualityThreshold(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateSessionWithNoSession)
{
    EXPECT_EQ(m_pController->UpdateSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testOpenSessionFail)
{
    EXPECT_EQ(m_pController->OpenSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testOpenSessionMultipleTimes)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateSessionBeforeOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateQualityThresholdBeforeOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->ApplyQualityThreshold(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testCloseSessionFail)
{
    EXPECT_EQ(m_pController->CloseSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testCloseSessionWithSessionCreated)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testCloseSessionAfterOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
}

TEST_F(VideoControllerTest, testUpdateSessionAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateRtpConfigAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pVideoNego, IMS_FALSE), IMS_FALSE);
}

TEST_F(VideoControllerTest, testUpdateQualityThresholdAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->ApplyQualityThreshold(), IMS_FALSE);
}

TEST_F(VideoControllerTest, testModifySession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);

    ImsMediaVideoParam* pSetSurfaceParam = new ImsMediaVideoParam();
    pSetSurfaceParam->nValue = SURFACE_FAR;
    EXPECT_EQ(m_pController->SendMessage(
                      IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(pSetSurfaceParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);

    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = 0;
    EXPECT_EQ(m_pController->SendMessage(
                      IJniMedia::SELECT_CAMERA_CMD, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pVideoNego, IMS_FALSE), IMS_TRUE);
    EXPECT_EQ(m_pController->ApplyQualityThreshold(), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(), IMS_TRUE);
}

TEST_F(VideoControllerTest, testSendMessageWithNoSession)
{
    ImsMediaVideoParam* pSetSurfaceParam = new ImsMediaVideoParam();
    pSetSurfaceParam->nValue = SURFACE_FAR;
    EXPECT_EQ(m_pController->SendMessage(
                      IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(pSetSurfaceParam)),
            IMS_FALSE);
}

TEST_F(VideoControllerTest, testSendMessageWithInvalidCommand)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);

    ImsMediaVideoParam* pSetSurfaceParam = new ImsMediaVideoParam();
    pSetSurfaceParam->nValue = SURFACE_FAR;
    EXPECT_EQ(m_pController->SendMessage(-1, reinterpret_cast<IMS_UINTP>(pSetSurfaceParam)),
            IMS_FALSE);
}

TEST_F(VideoControllerTest, testSendMessageWithInvalidParameter)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);

    EXPECT_EQ(m_pController->SendMessage(
                      IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(nullptr)),
            IMS_FALSE);
}

TEST_F(VideoControllerTest, testSendMessageAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    ImsMediaVideoParam* pSetSurfaceParam = new ImsMediaVideoParam();
    pSetSurfaceParam->nValue = SURFACE_FAR;
    EXPECT_EQ(m_pController->SendMessage(
                      IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(pSetSurfaceParam)),
            IMS_FALSE);
}

TEST_F(VideoControllerTest, testIsSessionOpened)
{
    // Initial state: No session
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Create session
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    // Session created but not opened (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Open session
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pVideoNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    // Session is now open (state is STATE_IDLE or higher)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_TRUE);

    // Close session
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    // Session is closed (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);
}
