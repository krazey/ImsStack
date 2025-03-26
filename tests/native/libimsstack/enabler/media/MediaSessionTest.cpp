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
#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockIMediaSessionClientListener.h"
#include "MockISession.h"
#include "MockMediaNego.h"
#include "config/MediaSessionConfig.h"

using ::testing::Return;
using ::testing::ReturnRef;

const AString LOCAL_IP = "127.0.0.1";
const AString REMOTE_IP = "127.0.0.1";
const IMS_UINT32 REMOTE_PORT = 40000;
const IMS_UINT32 BRING_MOCKMEDIANEGO = 1234567;

class FakeMediaSessionConfig : public MediaSessionConfig
{
public:
    virtual IMS_BOOL Create(IN IMS_SINT32 /*nSlotId*/)
    {
        MockICarrierConfig* m_pMockICarrierConfig = new MockICarrierConfig();
        MockICarrierConfig* m_pAudioBundle = new MockICarrierConfig();
        MockICarrierConfig* m_pVideoBundle = new MockICarrierConfig();
        MockICarrierConfig* m_pTextBundle = new MockICarrierConfig();

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pAudioBundle));
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pVideoBundle));
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pTextBundle));

        if (m_pMockICarrierConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        CreateAudioConfiguration(m_pMockICarrierConfig);
        CreateVideoConfiguration(m_pMockICarrierConfig);
        CreateTextConfiguration(m_pMockICarrierConfig);

        return IMS_TRUE;
    }
};

class FakeMediaSession : public MediaSession
{
public:
    FakeMediaSessionConfig* m_pMediaSessionConfig;
    MockMediaNego* m_pMediaNego;

    FakeMediaSession(IN MEDIA_NETWORK_TYPE eNetwork, IN MEDIA_SERVICE_TYPE eServiceType,
            IService* pService, IN IMS_SINTP nCallKey, IN IMS_UINT32 nSlotId) :
            MediaSession(eNetwork, eServiceType, pService, nCallKey, nSlotId)
    {
        CreateMediaConfig(eServiceType);
        m_pMediaNego = new MockMediaNego(nSlotId);
    }

    virtual ~FakeMediaSession()
    {
        delete m_pMediaSessionConfig;
        delete m_pMediaNego;
    }

    IMS_BOOL ModifySession(IN IMS_UINTP nNegoId)
    {
        return m_objAudioController.ModifySession(nNegoId);
    }

    virtual QosRequestParam* createQosParam(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType) override
    {
        (void)nNegoId;
        return new QosRequestParam(eMediaType, IpAddress(REMOTE_IP), REMOTE_PORT);
    }

    virtual MediaNego* FindMediaNego(IN IMS_UINTP nNegoId) override
    {
        if (nNegoId == BRING_MOCKMEDIANEGO)
        {
            return m_pMediaNego;
        }

        MediaNego* pMediaNego = IMS_NULL;
        IMS_SLONG nIndex = m_objMapMediaNego.GetIndexOfKey(nNegoId);

        if (nIndex < 0)
        {
            return IMS_NULL;
        }

        pMediaNego = m_objMapMediaNego.GetValueAt(nIndex);

        if (pMediaNego == IMS_NULL)
        {
            return IMS_NULL;
        }

        return pMediaNego;
    }

    virtual IMS_BOOL MediaSession_SendMsgToMediaManager(
            IN IMS_SINT32 eEvent, IN ImsMediaMsgParamBase* pParam) override
    {
        if (pParam == NULL)
        {
            return IMS_FALSE;
        }

        switch (eEvent)
        {
            case IJniMedia::REQUEST_OPEN_SESSION:
                delete static_cast<ImsMediaMsgOpenConfigParam*>(pParam);
                break;
            case IJniMedia::REQUEST_MODIFY_SESSION:
                delete static_cast<ImsMediaMsgConfigParam*>(pParam);
                break;
            case IJniMedia::REQUEST_CLOSE_SESSION:
                delete pParam;
                break;
            case IJniMedia::REQUEST_ADD_CONFIG:
            case IJniMedia::REQUEST_DELETE_CONFIG:
            case IJniMedia::REQUEST_CONFIRM_CONFIG:
                delete static_cast<ImsMediaMsgConfigParam*>(pParam);
                break;
            case IJniMedia::REQUEST_SEND_DTMF:
                delete static_cast<ImsMediaMsgDtmfParam*>(pParam);
                break;
            case IJniMedia::REQUEST_SET_MEDIA_QUALITY:
                delete static_cast<ImsMediaMsgSetMediaQualityParam*>(pParam);
                break;
            case IJniMedia::REQUEST_QOS:
                delete static_cast<ImsMediaMsgQosParam*>(pParam);
                break;
            default:
                break;
        }

        return IMS_TRUE;
    }

    virtual IMS_UINTP CreateProfile(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType) override
    {
        MediaNego* pMediaNego = CreateMediaNego(nNegoId);

        if (pMediaNego == IMS_NULL)
        {
            return UNDEFINED_NEGO_ID;
        }

        IMS_UINTP nMediaNego = reinterpret_cast<IMS_UINTP>(pMediaNego);

        std::shared_ptr<AudioNego> pAudioNego = pMediaNego->GetAudioNego();

        if (pAudioNego != IMS_NULL)
        {
            m_objAudioController.CreateSession(this, nMediaNego,
                    m_pMediaSessionConfig->GetAudioConfiguration(), MEDIA_SERVICE_DEFAULT);
        }

        std::shared_ptr<VideoNego> pVideoNego = pMediaNego->GetVideoNego();

        if (pVideoNego != IMS_NULL && MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_VIDEO))
        {
            m_objVideoController.CreateSession(
                    this, m_pMediaSessionConfig->GetVideoConfiguration());
        }

        std::shared_ptr<TextNego> pTextNego = pMediaNego->GetTextNego();

        if (pTextNego != IMS_NULL && MEDIA_IS_CONTAINED_THIS_TYPE(eMediaType, MEDIA_TYPE_TEXT))
        {
            m_objTextController.CreateSession(this, m_pMediaSessionConfig->GetTextConfiguration());
        }

        return nMediaNego;
    }

    virtual IMS_BOOL CreateMediaConfig(IN MEDIA_SERVICE_TYPE /*eServiceTyp*/) override
    {
        m_pMediaSessionConfig = new FakeMediaSessionConfig();
        if (m_pMediaSessionConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!m_pMediaSessionConfig->Create(m_nSlotId))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }
};

class MediaSessionTest : public ::testing::Test
{
public:
    MockICoreService m_objMockService;
    FakeMediaSession* m_pSession;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    MockISession* m_pIsession;
    MockICoreService m_objMockICoreService;
    MockIMediaSessionClientListener m_objClientListener;
    IpAddress m_objLocalIpAddress;
    IpAddress m_objRemoteIpAddress;

    IMS_UINTP createAudioSession()
    {
        IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
        m_pSession->FormSdp(negoId, m_pIsession, MEDIA_TYPE_AUDIO, MEDIA_DIRECTION_SEND_RECEIVE,
                MEDIA_DIRECTION_INVALID, MEDIA_DIRECTION_INVALID);

        return negoId;
    }

    IMS_UINTP createVideoSession()
    {
        IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIOVIDEO);
        m_pSession->FormSdp(negoId, m_pIsession, MEDIA_TYPE_AUDIOVIDEO,
                MEDIA_DIRECTION_SEND_RECEIVE, MEDIA_DIRECTION_SEND_RECEIVE,
                MEDIA_DIRECTION_INVALID);

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
        m_pSession = new FakeMediaSession(
                MEDIA_NETWORK_LTE, MEDIA_SERVICE_DEFAULT, &m_objMockService, 1, 0);
        m_pSession->SetMtcListener(&m_objClientListener);

        m_pIsession = new MockISession();
    }

    virtual void TearDown() override
    {
        delete m_pSession;
        delete m_pIsession;
    }
};

TEST_F(MediaSessionTest, testGetSupportedMediaTypesFromSdp)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(negoId, 0);

    negoId = BRING_MOCKMEDIANEGO;

    EXPECT_EQ(m_pSession->GetSupportedMediaTypesFromSdp(0, m_pIsession), MEDIA_TYPE_INVALID);

    ON_CALL(*m_pSession->m_pMediaNego, GetSupportedMediaTypesFromSdp(m_pIsession))
            .WillByDefault(Return(MEDIA_TYPE_AUDIO));
    EXPECT_EQ(m_pSession->GetSupportedMediaTypesFromSdp(negoId, m_pIsession), MEDIA_TYPE_AUDIO);

    ON_CALL(*m_pSession->m_pMediaNego, GetSupportedMediaTypesFromSdp(m_pIsession))
            .WillByDefault(Return(MEDIA_TYPE_VIDEO));
    EXPECT_EQ(m_pSession->GetSupportedMediaTypesFromSdp(negoId, m_pIsession), MEDIA_TYPE_VIDEO);

    ON_CALL(*m_pSession->m_pMediaNego, GetSupportedMediaTypesFromSdp(m_pIsession))
            .WillByDefault(Return(MEDIA_TYPE_TEXT));
    EXPECT_EQ(m_pSession->GetSupportedMediaTypesFromSdp(negoId, m_pIsession), MEDIA_TYPE_TEXT);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackUnmatch)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(negoId, 0);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_AUDIO), IMS_TRUE);

    ImsMediaMsgQosParam* pParam =
            new ImsMediaMsgQosParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, 30000);
    pParam->m_bResult = IMS_TRUE;

    EXPECT_CALL(m_objClientListener, MediaSession_NotifyQos(negoId, IMS_TRUE, MEDIA_TYPE_AUDIO))
            .Times(0);

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackAudio)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(negoId, 0);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_AUDIO), IMS_TRUE);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_AUDIO), IMS_TRUE);  // check request twice

    EXPECT_CALL(m_objClientListener, MediaSession_NotifyQos(negoId, IMS_TRUE, MEDIA_TYPE_AUDIO))
            .Times(1);

    ImsMediaMsgQosParam* pParam =
            new ImsMediaMsgQosParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, REMOTE_PORT);
    pParam->m_bResult = IMS_TRUE;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_CALL(m_objClientListener, MediaSession_NotifyQos(negoId, IMS_FALSE, MEDIA_TYPE_AUDIO))
            .Times(1);

    pParam = new ImsMediaMsgQosParam(MEDIA_TYPE_AUDIO, m_objRemoteIpAddress, REMOTE_PORT);
    pParam->m_bResult = IMS_FALSE;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackVideo)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_VIDEO);
    EXPECT_NE(negoId, 0);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_VIDEO), IMS_TRUE);

    ImsMediaMsgQosParam* pParam =
            new ImsMediaMsgQosParam(MEDIA_TYPE_VIDEO, m_objRemoteIpAddress, REMOTE_PORT);
    pParam->m_bResult = IMS_TRUE;

    EXPECT_CALL(m_objClientListener, MediaSession_NotifyQos(negoId, IMS_TRUE, MEDIA_TYPE_VIDEO))
            .Times(1);

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testQosRequestAndCallbackText)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_TEXT);
    EXPECT_NE(negoId, 0);
    EXPECT_EQ(m_pSession->RequestQos(negoId, MEDIA_TYPE_TEXT), IMS_TRUE);

    ImsMediaMsgQosParam* pParam =
            new ImsMediaMsgQosParam(MEDIA_TYPE_TEXT, m_objRemoteIpAddress, REMOTE_PORT);
    pParam->m_bResult = IMS_TRUE;

    EXPECT_CALL(m_objClientListener, MediaSession_NotifyQos(negoId, IMS_TRUE, MEDIA_TYPE_TEXT))
            .Times(1);

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testNotifySrvccSuccess)
{
    createAudioSession();

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_FALSE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_SUCCEED), IMS_TRUE);

    destroyAudioSession();

    m_pSession->ModifySession(createAudioSession());

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_TRUE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_SUCCEED), IMS_TRUE);

    destroyAudioSession();
}

TEST_F(MediaSessionTest, testNotifySrvccFailed)
{
    IMS_UINTP negoId = createAudioSession();

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_FALSE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_FAILED), IMS_FALSE);

    m_pSession->ModifySession(negoId);

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_TRUE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_FAILED), IMS_TRUE);

    destroyAudioSession();
}

TEST_F(MediaSessionTest, testNotifySrvccCanceled)
{
    IMS_UINTP negoId = createAudioSession();

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_CANCELED), IMS_FALSE);

    m_pSession->ModifySession(negoId);

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_CANCELED), IMS_TRUE);

    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_STARTED), IMS_TRUE);
    EXPECT_EQ(m_pSession->NotifySrvccStatus(MEDIA_SRVCC_CANCELED), IMS_TRUE);

    destroyAudioSession();
}

TEST_F(MediaSessionTest, testNotifyFirstPacket)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(negoId, 0);

    ImsMediaResponseConfigParam* pParam = new ImsMediaResponseConfigParam();

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_FIRST_PACKET, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testNotifyMediaInactivity)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIOVIDEO);
    EXPECT_NE(negoId, 0);

    ImsMediaNotifyQualityStatusParam* pParam = new ImsMediaNotifyQualityStatusParam();
    pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    ImsMediaNotifyInactivityParam* pParam1 = new ImsMediaNotifyInactivityParam();
    pParam1->m_eMediaType = MEDIA_TYPE_VIDEO;
    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(pParam1)),
            IMS_TRUE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}

TEST_F(MediaSessionTest, testGetRemotePort)
{
    MediaNego::MediaNegoResult eErrorReason = MediaNego::MediaNegoResult::NO_ERROR;
    IMS_SINT32 eAudioDirection = MEDIA_DIRECTION_INVALID;
    IMS_SINT32 eVideoDirection = MEDIA_DIRECTION_INVALID;
    IMS_SINT32 eTextDirection = MEDIA_DIRECTION_INVALID;

    IMS_UINTP nNegoId = createVideoSession();
    m_pSession->NegotiateSdp(nNegoId, m_pIsession, &eAudioDirection, &eVideoDirection,
            &eTextDirection, eErrorReason);
    EXPECT_EQ(m_pSession->GetRemotePort(nNegoId, MEDIA_TYPE_AUDIO), -1);
    EXPECT_EQ(m_pSession->GetRemotePort(nNegoId, MEDIA_TYPE_VIDEO), -1);
}

TEST_F(MediaSessionTest, testNotifyVideoBitrate)
{
    createAudioSession();

    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = 100000;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_VIDEO_BITRATE, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_TRUE);

    pParam = IMS_NULL;

    EXPECT_EQ(m_pSession->SendMessage(
                      IJniMedia::NOTIFY_VIDEO_BITRATE, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_FALSE);

    destroyAudioSession();
}
