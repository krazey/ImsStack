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

#include "IJniMedia.h"
#include "MediaManager.h"
#include "MediaSession.h"

#include "MockICoreService.h"
#include "MockIMediaSessionClientListener.h"
#include "MockMediaMsgHandler.h"
#include "video/MockVideoController.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const IMS_SINT32 CALL_KEY_BROADCAST = 0;
const IMS_SINT32 CALL_KEY_1 = 1;
const IMS_SINT32 CALL_KEY_2 = 2;
const IMS_SINT32 CALL_KEY_3 = 3;
const IMS_SINT32 CALL_KEY_4 = 4;
const IMS_CHAR DEFAULT_THREAD_NAME[] = "ET00.MediaManager";
const AString LOCAL_IP = "127.0.0.1";

class TestMediaManager : public MediaManager
{
public:
    TestMediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId) :
            MediaManager(strName, nSlotId)
    {
    }

protected:
    virtual std::shared_ptr<MediaMsgHandler> CreateMessageHandler(IN IMS_SINTP nCallKey) override
    {
        return std::make_shared<MockMediaMsgHandler>(m_nSlotId, nCallKey);
    }
};

class MediaManagerTest : public ::testing::Test
{
public:
    std::unique_ptr<TestMediaManager> m_pMediaManager;
    MockICoreService m_objCoreService;
    IpAddress m_objIpAddr;
    std::unique_ptr<MockIMediaSessionClientListener> m_pMockClient;
    std::shared_ptr<MockVideoController> m_pMockVideoController;

protected:
    virtual void SetUp() override
    {
        m_pMediaManager = std::make_unique<TestMediaManager>("MediaManager", DEFAULT_SLOT_ID);
        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(m_objCoreService, GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));
        m_pMockClient = std::make_unique<MockIMediaSessionClientListener>();
        m_pMockVideoController = std::make_shared<MockVideoController>();
    }

    virtual void TearDown() override {}
};

TEST_F(MediaManagerTest, TestMediaSessionCreateWithNullArgument)
{
    EXPECT_EQ(m_pMediaManager->CreateSession(
                      MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, IMS_NULL, CALL_KEY_1),
            IMS_NULL);
}

TEST_F(MediaManagerTest, TestMediaSessionCreateAndDestroy)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession->SetMtcListener(m_pMockClient.get());

    ASSERT_NE(pIMediaSession, IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), static_cast<MediaSession*>(pIMediaSession));
    EXPECT_NE(m_pMediaManager->GetHandler(CALL_KEY_1), IMS_NULL);

    m_pMediaManager->DestroySession(pIMediaSession);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetHandler(CALL_KEY_1), IMS_NULL);
}

TEST_F(MediaManagerTest, TestGetThreadName)
{
    AString strThreadName = DEFAULT_THREAD_NAME;
    EXPECT_EQ(m_pMediaManager->GetThreadName(DEFAULT_SLOT_ID), strThreadName);
}

TEST_F(MediaManagerTest, TestDestroySession)
{
    IMediaSession* pIMediaSession1 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    IMediaSession* pIMediaSession2 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_2);
    IMediaSession* pIMediaSession3 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_3);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), static_cast<MediaSession*>(pIMediaSession1));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), static_cast<MediaSession*>(pIMediaSession2));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), static_cast<MediaSession*>(pIMediaSession3));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_4), IMS_NULL);

    m_pMediaManager->DestroySession(pIMediaSession1);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), static_cast<MediaSession*>(pIMediaSession2));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), static_cast<MediaSession*>(pIMediaSession3));

    m_pMediaManager->DestroySession(pIMediaSession2);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), static_cast<MediaSession*>(pIMediaSession3));

    m_pMediaManager->DestroySession(pIMediaSession3);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), IMS_NULL);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), IMS_NULL);
}

TEST_F(MediaManagerTest, TestSendMessageBroadcast)
{
    // Create multiple sessions
    IMediaSession* pIMediaSession1 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession1->SetMtcListener(m_pMockClient.get());

    IMediaSession* pIMediaSession2 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_2);
    pIMediaSession2->SetMtcListener(m_pMockClient.get());

    EXPECT_CALL(*m_pMockClient, MediaSession_Notify(REPORT_MEDIA_DETACH, _, _)).Times(2);

    auto pParam = std::make_unique<ImsMediaMsgParamBase>();
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_MEDIA_DETACH, CALL_KEY_BROADCAST,
            reinterpret_cast<IMS_UINTP>(pParam.release())));
}

TEST_F(MediaManagerTest, TestSendMessageSessionNotFound)
{
    // No session created for CALL_KEY_1
    auto pParam = new ImsMediaMsgParamBase();
    // Expect SendMessage to return false because the session doesn't exist
    EXPECT_FALSE(m_pMediaManager->SendMessage(
            IJniMedia::NOTIFY_MEDIA_DETACH, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam)));
}

TEST_F(MediaManagerTest, TestSendMessageBroadcastNoSessions)
{
    // No sessions are created.
    auto pParam = new ImsMediaMsgParamBase();
    // Broadcast should still return true, even if there are no sessions to send to.
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_MEDIA_DETACH, CALL_KEY_BROADCAST,
            reinterpret_cast<IMS_UINTP>(pParam)));
}

TEST_F(MediaManagerTest, TestSendMessageCommonResponse)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession->SetMtcListener(m_pMockClient.get());

    auto pParam1 = std::make_unique<ImsMediaResponseConfigParam>();
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::RESPONSE_OPEN_SESSION, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam1.release())));

    auto pParam2 = std::make_unique<ImsMediaResponseConfigParam>();
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::RESPONSE_MODIFY_SESSION, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam2.release())));

    auto pParam3 = std::make_unique<ImsMediaResponseConfigParam>();
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::RESPONSE_ADD_CONFIG, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam3.release())));

    auto pParam4 = std::make_unique<ImsMediaResponseConfigParam>();
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::RESPONSE_CONFIRM_CONFIG, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam4.release())));
}

TEST_F(MediaManagerTest, TestSendMessageCommonNotification)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession->SetMtcListener(m_pMockClient.get());

    auto pParam1 = std::make_unique<ImsMediaResponseConfigParam>(MEDIA_TYPE_AUDIO);
    EXPECT_CALL(
            *m_pMockClient, MediaSession_Notify(REPORT_DATA_RECEIVE_STARTED, MEDIA_TYPE_AUDIO, _))
            .Times(1);
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_FIRST_PACKET, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam1.release())));

    auto pParam2 = std::make_unique<ImsMediaNotifyInactivityParam>(MEDIA_TYPE_VIDEO);
    EXPECT_CALL(*m_pMockClient,
            MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_VIDEO, MEDIA_PROTOCOL_RTP))
            .Times(1);
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_MEDIA_INACTIVITY, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam2.release())));

    auto pParam3 = std::make_unique<ImsMediaResponseConfigParam>();
    EXPECT_CALL(*m_pMockClient, MediaSession_Notify(REPORT_MEDIA_DETACH, _, _)).Times(1);
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_MEDIA_DETACH, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam3.release())));

    // return false due to the missing request qos operation
    auto pParam4 = std::make_unique<ImsMediaMsgQosParam>();
    EXPECT_FALSE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_QOS_INFO, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam4.release())));

    auto pParam5 = std::make_unique<ImsMediaVideoParam>(100);
    EXPECT_CALL(*m_pMockClient, MediaSession_Notify(REPORT_VIDEO_LOWEST_BITRATE, _, _)).Times(1);
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::NOTIFY_VIDEO_BITRATE, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam5.release())));
}

TEST_F(MediaManagerTest, TestSendMessageVideoIndication)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    static_cast<MediaSession*>(pIMediaSession)->SetVideoController(m_pMockVideoController);

    auto pParam1 = std::make_unique<ImsMediaVideoParam>();
    EXPECT_CALL(*m_pMockVideoController, SendMessage(IJniMedia::SETSURFACE_CMD, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::SETSURFACE_CMD, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam1.release())));

    auto pParam2 = std::make_unique<ImsMediaVideoParam>();
    EXPECT_CALL(*m_pMockVideoController, SendMessage(IJniMedia::SELECT_CAMERA_CMD, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::SELECT_CAMERA_CMD, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam2.release())));

    auto pParam3 = std::make_unique<ImsMediaVideoParam>();
    EXPECT_CALL(*m_pMockVideoController, SendMessage(IJniMedia::CHANGE_CAMERA_ZOOM_CMD, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::CHANGE_CAMERA_ZOOM_CMD, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam3.release())));

    auto pParam4 = std::make_unique<ImsMediaVideoParam>();
    EXPECT_CALL(*m_pMockVideoController, SendMessage(IJniMedia::SET_PAUSE_IMAGE_CMD, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::SET_PAUSE_IMAGE_CMD, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam4.release())));

    auto pParam5 = std::make_unique<ImsMediaVideoParam>();
    EXPECT_CALL(*m_pMockVideoController, SendMessage(IJniMedia::CHANGE_ORIENTATION_CMD, _))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::CHANGE_ORIENTATION_CMD, CALL_KEY_1,
            reinterpret_cast<IMS_UINTP>(pParam5.release())));
}

TEST_F(MediaManagerTest, TestSendMessageEtc)
{
    m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);

    auto pParam1 = std::make_unique<ImsMediaMsgDtmfParam>();
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::SEND_DTMF, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam1.release())));
}

TEST_F(MediaManagerTest, TestHandleMessageAndDispatch)
{
    // This test covers HandleMessage and DispatchMessage
    m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);

    auto pParam = new ImsMediaMsgDtmfParam();
    ImsMessage msg(IJniMedia::NOTIFY_QOS_INFO, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam));

    // DispatchMessage calls HandleMessage, which in turn calls SendMessage.
    EXPECT_TRUE(m_pMediaManager->DispatchMessage(msg));
}

TEST_F(MediaManagerTest, TestHandleRequestMsgCommon)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession->SetMtcListener(m_pMockClient.get());
    auto pMockHandler =
            std::static_pointer_cast<MockMediaMsgHandler>(m_pMediaManager->GetHandler(CALL_KEY_1));
    ASSERT_NE(pMockHandler, nullptr);

    auto pParam1 = std::make_unique<ImsMediaMsgOpenConfigParam>(MEDIA_TYPE_AUDIO);
    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_OPEN_SESSION, pParam1.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_OPEN_SESSION, CALL_KEY_1, pParam1.get()));

    auto pParam2 = std::make_unique<ImsMediaMsgConfigParam>(MEDIA_TYPE_AUDIO);
    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_MODIFY_SESSION, pParam2.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_MODIFY_SESSION, CALL_KEY_1, pParam2.get()));

    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_ADD_CONFIG, pParam2.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_ADD_CONFIG, CALL_KEY_1, pParam2.get()));

    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_DELETE_CONFIG, pParam2.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_DELETE_CONFIG, CALL_KEY_1, pParam2.get()));

    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_CONFIRM_CONFIG, pParam2.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_CONFIRM_CONFIG, CALL_KEY_1, pParam2.get()));

    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_CLOSE_SESSION, pParam2.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_CLOSE_SESSION, CALL_KEY_1, pParam2.get()));

    auto pParam3 = std::make_unique<ImsMediaMsgSetMediaQualityParam>(MEDIA_TYPE_AUDIO);
    EXPECT_CALL(
            *pMockHandler, SendMessageToJava(IJniMedia::REQUEST_SET_MEDIA_QUALITY, pParam3.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SET_MEDIA_QUALITY, CALL_KEY_1, pParam3.get()));
}

TEST_F(MediaManagerTest, TestHandleRequestMsgAudio)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession->SetMtcListener(m_pMockClient.get());
    auto pMockHandler =
            std::static_pointer_cast<MockMediaMsgHandler>(m_pMediaManager->GetHandler(CALL_KEY_1));
    ASSERT_NE(pMockHandler, nullptr);

    auto pParam = std::make_unique<ImsMediaMsgDtmfParam>();
    EXPECT_CALL(*pMockHandler, SendMessageToJava(IJniMedia::REQUEST_SEND_DTMF, pParam.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SEND_DTMF, CALL_KEY_1, pParam.get()));
}

TEST_F(MediaManagerTest, TestHandleRequestMsgVideo)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    pIMediaSession->SetMtcListener(m_pMockClient.get());
    auto pMockHandler =
            std::static_pointer_cast<MockMediaMsgHandler>(m_pMediaManager->GetHandler(CALL_KEY_1));
    ASSERT_NE(pMockHandler, nullptr);

    auto pParam = std::make_unique<ImsMediaMsgParamBase>(MEDIA_TYPE_VIDEO);
    EXPECT_CALL(
            *pMockHandler, SendMessageToJava(IJniMedia::REQUEST_SET_DISPLAY_SURFACE, pParam.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SET_DISPLAY_SURFACE, CALL_KEY_1, pParam.get()));

    EXPECT_CALL(
            *pMockHandler, SendMessageToJava(IJniMedia::REQUEST_SET_PREVIEW_SURFACE, pParam.get()))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SET_PREVIEW_SURFACE, CALL_KEY_1, pParam.get()));
}

TEST_F(MediaManagerTest, TestHandleRequestMsgSessionNotFound)
{
    // No session created for CALL_KEY_1
    auto pParam = std::make_unique<ImsMediaMsgOpenConfigParam>(MEDIA_TYPE_AUDIO);

    // Expect HandleRequestMsg to return false because the session doesn't exist
    EXPECT_FALSE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_OPEN_SESSION, CALL_KEY_1, pParam.get()));
}
