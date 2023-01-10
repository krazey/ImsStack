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
#include <gmock/gmock.h>
#include <MediaSession.h>
#include <MediaEnvironment.h>
#include <IMMedia.h>
#include <MockICoreService.h>
#include <MockIMediaSessionClientListener.h>
#include <MockISession.h>

using ::testing::ReturnRef;

const AString LOCAL_IP = "127.0.0.1";
const AString REMOTE_IP = "127.0.0.1";
const IMS_UINT32 REMOTE_PORT = 40000;

class FakeMediaSession : public MediaSession
{
public:
    FakeMediaSession(IN MEDIA_SERVICE_TYPE nService, IN IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId) :
            MediaSession(nService, nCallKey, nSlotId)
    {
    }
    virtual ~FakeMediaSession() {}

    IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam) override
    {
        if (pParam == NULL)
        {
            return IMS_FALSE;
        }

        switch (eEvent)
        {
            case IMMedia::REQUEST_OPEN_SESSION:
                delete static_cast<ImsMediaMsgOpenConfigParam*>(pParam);
                break;
            case IMMedia::REQUEST_MODIFY_SESSION:
                delete static_cast<ImsMediaMsgConfigParam*>(pParam);
                break;
            case IMMedia::REQUEST_CLOSE_SESSION:
                delete pParam;
                break;
            case IMMedia::REQUEST_ADD_CONFIG:
            case IMMedia::REQUEST_DELETE_CONFIG:
            case IMMedia::REQUEST_CONFIRM_CONFIG:
                delete static_cast<ImsMediaMsgConfigParam*>(pParam);
                break;
            case IMMedia::REQUEST_SEND_DTMF:
                delete static_cast<ImsMediaMsgDtmfParam*>(pParam);
                break;
            case IMMedia::REQUEST_SET_MEDIA_QUALITY:
                delete static_cast<ImsMediaMsgSetMediaQualityParam*>(pParam);
                break;
            case IMMedia::REQUEST_QOS:
                delete static_cast<ImsMediaMsgQosParam*>(pParam);
                break;
            default:
                break;
        }

        return IMS_TRUE;
    }
};

class MediaSessionTest : public ::testing::Test
{
public:
    FakeMediaSession* m_pSession;
    MediaEnvironment* m_pEnvironment;
    MockISession* m_pIsession;
    MockICoreService m_objMockICoreService;
    MockIMediaSessionClientListener m_objClientListener;
    IPAddress m_objLocalIpAddress;
    IPAddress m_objRemoteIpAddress;

    IMS_UINTP createAudioSession()
    {
        IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
        m_pSession->FormSDP(negoId, m_pIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND_RECEIVE,
                MEDIA_DIRECTION_INVALID, MEDIA_DIRECTION_INVALID);

        return negoId;
    }

    void destroyAudioSession() { EXPECT_EQ(m_pSession->Terminate(), IMS_TRUE); }

protected:
    virtual void SetUp() override
    {
        m_objLocalIpAddress = IpAddress(LOCAL_IP);
        m_objRemoteIpAddress = IpAddress(REMOTE_IP);
        ON_CALL(m_objMockICoreService, GetIpAddress())
                .WillByDefault(ReturnRef(m_objLocalIpAddress));

        m_pEnvironment = new MediaEnvironment();
        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->pIService = &m_objMockICoreService;

        m_pSession = new FakeMediaSession(MEDIA_SERVICE_DEFAULT, 1, 0);
        m_pSession->SetEnvironment(m_pEnvironment);
        m_pSession->SetMtcListener(&m_objClientListener);

        m_pIsession = new MockISession();
    }

    virtual void TearDown() override
    {
        delete m_pSession;
        delete m_pIsession;
    }
};

TEST_F(MediaSessionTest, testQosRequest)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIOVIDEOTEXT);
    EXPECT_NE(negoId, 0);

    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_AUDIO), IMS_TRUE);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_VIDEO), IMS_TRUE);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_TEXT), IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosCallback)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(negoId, 0);

    ImsMediaMsgQosParam* pParam =
            new ImsMediaMsgQosParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, REMOTE_PORT);
    pParam->m_bResult = IMS_TRUE;

    EXPECT_EQ(
            m_pSession->SendMessage(IMMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testNotifySrvccSuccess)
{
    createAudioSession();
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_TRUE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_SUCCEED), IMS_TRUE);
    destroyAudioSession();
}

TEST_F(MediaSessionTest, testNotifySrvccFailed)
{
    createAudioSession();
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_TRUE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_FAILED), IMS_TRUE);
    destroyAudioSession();
}