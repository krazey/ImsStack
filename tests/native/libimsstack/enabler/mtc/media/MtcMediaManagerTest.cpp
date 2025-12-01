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

#include "CallReasonInfo.h"
#include "MediaManager.h"
#include "IJniMedia.h"
#include "ISipHeader.h"
#include "MediaDef.h"
#include "MockICoreService.h"
#include "MockIMediaSession.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockMediaManager.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMediaQosEventListener.h"
#include "media/MockIMediaReportEventListener.h"
#include "media/MockMediaProfileManager.h"
#include "media/MtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

LOCAL CallKey DEFAULT_CALL_KEY = 1;
LOCAL IMS_UINTP NEGO_ID = 100;

class TestMtcMediaManager : public MtcMediaManager
{
public:
    inline explicit TestMtcMediaManager(
            IN IMtcCallContext& objContext, IN MediaManager& objMediaManager) :
            MtcMediaManager(objContext, objMediaManager)
    {
    }

    inline void ReplaceMediaSession(IN IMediaSession* piMediaSession)
    {
        m_piMediaSession = piMediaSession;
    }

    inline void ReplaceMediaProfileManager(IN MtcMediaProfileManager* pMediaProfileManager)
    {
        delete m_pProfileManager;
        m_pProfileManager = pMediaProfileManager;
    }
};

class MtcMediaManagerTest : public ::testing::Test
{
public:
    inline MtcMediaManagerTest() :
            objContext(),
            pMediaManager(IMS_NULL),
            pListener(new MockIMediaReportEventListener()),
            objISession(),
            objIMessage(),
            pConfigurationProxy(new MockMtcConfigurationProxy()),
            objMessageUtils(),
            pMediaProfileManager(IMS_NULL)
    {
    }

public:
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockIMtcSession objMtcSession;
    MockMediaManager objMediaManager;
    MtcMediaManager* pMediaManager;
    MockIMediaReportEventListener* pListener;
    MockIMediaSession objMediaSession;
    MockISession objISession;
    MockISession objISession1;
    MockIMessage objIMessage;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMessageUtils objMessageUtils;
    MockMtcMediaProfileManager* pMediaProfileManager;
    MockIMtcPreconditionManager objPreconditionManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(DEFAULT_CALL_KEY));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetPreconditionManager())
                .WillByDefault(ReturnRef(objPreconditionManager));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));

        pMediaManager = CreateMtcMediaManager();

        pMediaManager->SetMediaReportEventListener(pListener);
        pMediaManager->SetMediaInfo(objISession, MediaInfo());
        pMediaManager->SetMediaInfo(objISession1,
                MediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_SEND, DIRECTION_RECEIVE,
                        AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_HD_PR, GTT_MODE_FULL));
    }

    virtual void TearDown() override
    {
        delete pMediaManager;
        delete pListener;
        delete pConfigurationProxy;
    }

    MtcMediaManager* CreateMtcMediaManager()
    {
        TestMtcMediaManager* pTestMediaManager =
                new TestMtcMediaManager(objContext, objMediaManager);

        pTestMediaManager->ReplaceMediaSession(&objMediaSession);

        pMediaProfileManager = new MockMtcMediaProfileManager();
        pTestMediaManager->ReplaceMediaProfileManager(pMediaProfileManager);

        return pTestMediaManager;
    }
};

TEST_F(MtcMediaManagerTest, CreateMediaSessionCreatesMediaSession)
{
    MockICoreService objCoreService;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetServiceType).WillByDefault(Return(ServiceType::NORMAL));
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));

    EXPECT_CALL(objMediaManager,
            CreateSession(
                    MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &objCoreService, DEFAULT_CALL_KEY))
            .WillOnce(Return(&objMediaSession));
    EXPECT_CALL(objMediaSession, SetMtcListener(pMediaManager));

    pMediaManager->CreateMediaSession();
}

TEST_F(MtcMediaManagerTest, CreateMediaSessionNotCrashesWhenInternalError)
{
    MockICoreService objCoreService;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetServiceType).WillByDefault(Return(ServiceType::NORMAL));
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    ON_CALL(objMediaManager, CreateSession(_, _, _, _)).WillByDefault(Return(nullptr));

    pMediaManager->CreateMediaSession();
}

TEST_F(MtcMediaManagerTest, DestroyMediaSessionDestroysMediaSession)
{
    EXPECT_CALL(objMediaManager, DestroySession(&objMediaSession));

    pMediaManager->DestroyMediaSession();
}

TEST_F(MtcMediaManagerTest, CreateMediaProfileCreatesMediaProfile)
{
    const IMS_BOOL bForked = IMS_TRUE;
    const IMS_BOOL bOrigin = IMS_TRUE;
    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(*pMediaProfileManager,
            CreateMediaProfile(&objISession, bForked, bOrigin, MEDIA_TYPE_AUDIO, &objMediaSession));

    pMediaManager->CreateMediaProfile(&objISession, bForked, bOrigin);
}

TEST_F(MtcMediaManagerTest, DestroyMediaProfileDestroysMediaProfile)
{
    EXPECT_CALL(*pMediaProfileManager, DestroyMediaProfile(&objISession, &objMediaSession));

    pMediaManager->DestroyMediaForSession(&objISession);
}

TEST_F(MtcMediaManagerTest, DestroyMediaProfileDestroysSessionMedia)
{
    MediaInfo objMediaInfo(DIRECTION_SEND, DIRECTION_SEND, DIRECTION_SEND, AUDIO_QUALITY_AMR_NB,
            VIDEO_QUALITY_HD_PR, GTT_MODE_FULL);
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession), objMediaInfo);

    EXPECT_CALL(*pMediaProfileManager, DestroyMediaProfile(&objISession, &objMediaSession));
    pMediaManager->DestroyMediaForSession(&objISession);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession), MediaInfo());
}

TEST_F(MtcMediaManagerTest, DestroyAllMediaProfileDestroysAllMediaProfile)
{
    EXPECT_CALL(*pMediaProfileManager, DestroyAllMediaProfiles(&objMediaSession));

    pMediaManager->DestroyAllMediaProfiles();
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithDataReceivedStartedReport)
{
    EXPECT_CALL(*pListener, OnReceivingMediaDataStarted(_, _)).Times(2);

    pMediaManager->MediaSession_Notify(REPORT_DATA_RECEIVE_STARTED);
    pMediaManager->MediaSession_Notify(REPORT_DATA_RECEIVE_STARTED, MEDIA_TYPE_AUDIO);
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithDataReceivedFailedReport)
{
    EXPECT_CALL(*pListener, OnReceivingMediaDataFailed(_, _)).Times(2);

    pMediaManager->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED);
    EXPECT_EQ(IMS_FALSE, pMediaManager->IsAudioInactive());

    pMediaManager->MediaSession_Notify(REPORT_DATA_RECEIVE_FAILED, MEDIA_TYPE_AUDIO);
    EXPECT_EQ(IMS_TRUE, pMediaManager->IsAudioInactive());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithVideoLowestBitRateReport)
{
    EXPECT_CALL(*pListener, OnVideoLowestBitRate).Times(1);

    pMediaManager->MediaSession_Notify(REPORT_VIDEO_LOWEST_BITRATE);
}

TEST_F(MtcMediaManagerTest,
        MediaSessionNotifyWithNwToneReceiveStartedDoNothingIfActiveSessionIsNull)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*pListener, OnReceivingNetworkToneStarted()).Times(0);
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_STARTED);
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithNwToneReceiveFailedAndStarted)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(&objISession));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(0));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pListener, OnReceivingNetworkToneFailed()).Times(1);
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_FAILED);
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pListener, OnReceivingNetworkToneStarted()).Times(1);
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_STARTED);
    EXPECT_FALSE(pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest,
        MediaSessionNotifyWithNwToneReceiveFailedAndLocalToneIsNotUpdatedIf180IsNotReceived)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(&objISession));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(0));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pListener, OnReceivingNetworkToneFailed()).Times(1);
    IMS_BOOL bLocalTone = pMediaManager->IsLocalTone();
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_FAILED);
    EXPECT_EQ(bLocalTone, pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest,
        MediaSessionNotifyWithNwToneReceiveFailedAndLocalToneIsNotUpdatedIfNotUsingDynamicNwTone)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(&objISession));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(1));

    EXPECT_CALL(*pListener, OnReceivingNetworkToneFailed()).Times(1);
    IMS_BOOL bLocalTone = pMediaManager->IsLocalTone();
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_FAILED);
    EXPECT_EQ(bLocalTone, pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest,
        MediaSessionNotifyWithNwToneReceiveFailedDoNothingIfAudioDirIsNotRecvOnlyInConfirmedDialog)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(&objISession));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pListener, OnReceivingNetworkToneFailed()).Times(0);
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_FAILED);
}

TEST_F(MtcMediaManagerTest,
        MediaSessionNotifyWithNwToneReceiveFailedIfAudioDirectionIsReceiveOnlyInConfirmedDialog)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(&objISession));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));

    MediaInfo objInfo;
    objInfo.eAudioDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objInfo);

    MtcSupplementaryService objSupplementaryService(objContext, *pConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService())
            .WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(*pListener, OnReceivingNetworkToneFailed()).Times(1);
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_FAILED);
    EXPECT_TRUE(objSupplementaryService.Get(SuppType::ENFORCE_LT)->bValue);
}

TEST_F(MtcMediaManagerTest,
        MediaSessionNotifyWithNwToneReceiveStartedIfAudioDirectionIsReceiveOnlyInConfirmedDialog)
{
    ON_CALL(*pMediaProfileManager, GetActiveSession()).WillByDefault(Return(&objISession));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));

    MediaInfo objInfo;
    objInfo.eAudioDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objInfo);

    MtcSupplementaryService objSupplementaryService(objContext, *pConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService())
            .WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(*pListener, OnReceivingNetworkToneStarted()).Times(1);
    pMediaManager->MediaSession_Notify(REPORT_NW_TONE_RTP_RECEIVE_STARTED);
    EXPECT_EQ(nullptr, objSupplementaryService.Get(SuppType::ENFORCE_LT));
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithMediaDetachReport)
{
    EXPECT_CALL(*pListener, OnMediaFailed(CallReasonInfo(CODE_MEDIA_UNSPECIFIED))).Times(1);

    pMediaManager->MediaSession_Notify(REPORT_MEDIA_DETACH);
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyFailuresNotifiesListenerMediaInitFailed)
{
    IMS_UINT32 nAnyReportType = REPORT_SUCCESS;

    //clang-format off
    std::vector<RtpError> objErrors{RtpError::INVALID_PARAM, RtpError::NOT_READY,
            RtpError::NO_MEMORY, RtpError::NO_RESOURCES, RtpError::PORT_UNAVAILABLE,
            RtpError::REQUEST_NOT_SUPPORTED, RtpError::RESPONSE_WAIT_TIMEOUT};
    //clang-format on

    EXPECT_CALL(*pListener, OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(7);
    for (RtpError eError : objErrors)
    {
        pMediaManager->MediaSession_NotifyFailures(nAnyReportType, eError);
    }
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyDoesNothingForNotUsedReportType)
{
    EXPECT_CALL(*pListener, OnMediaFailed(_)).Times(0);
    pMediaManager->MediaSession_Notify(REPORT_NOTUSED);
    pMediaManager->MediaSession_Notify(REPORT_INVALID);
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyFailuresDoesNothingIfNoError)
{
    IMS_UINT32 nAnyReportType = REPORT_SUCCESS;
    EXPECT_CALL(*pListener, OnMediaFailed(_)).Times(0);
    pMediaManager->MediaSession_NotifyFailures(nAnyReportType, RtpError::NO_ERROR);
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyQosNotifiesListener)
{
    IMS_BOOL bAnyResult = IMS_TRUE;
    MEDIA_CONTENT_TYPE eAnyMediaType = MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO;

    MockIMediaQosEventListener objQosListener;
    {
        // qos listener is null
        EXPECT_CALL(objQosListener, OnQosStatusChanged(_, _, _, _)).Times(0);
        pMediaManager->MediaSession_NotifyQos(NEGO_ID, bAnyResult, eAnyMediaType);
    }

    {
        // valid qos listener
        pMediaManager->SetQosListener(&objQosListener);
        EXPECT_CALL(*pMediaProfileManager, GetSessionWithNegoId(NEGO_ID))
                .Times(2)
                .WillRepeatedly(Return(&objISession));
        EXPECT_CALL(objQosListener,
                OnQosStatusChanged(&objISession, QosStatus::AVAILABLE, MEDIATYPE_AUDIO, IMS_TRUE))
                .Times(1);
        pMediaManager->MediaSession_NotifyQos(NEGO_ID, IMS_TRUE, eAnyMediaType);

        EXPECT_CALL(objQosListener,
                OnQosStatusChanged(&objISession, QosStatus::LOST, MEDIATYPE_AUDIO, IMS_TRUE))
                .Times(1);
        pMediaManager->MediaSession_NotifyQos(NEGO_ID, IMS_FALSE, eAnyMediaType);
    }
}

TEST_F(MtcMediaManagerTest, GetMediaInfoReturnsMediaInfoSetBySetMediaInfo)
{
    MediaInfo objMediaInfo1 = MediaInfo(DIRECTION_SEND, DIRECTION_SEND, DIRECTION_SEND,
            AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_HD_PR, GTT_MODE_FULL);
    MediaInfo objMediaInfo2 = MediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_SEND_RECEIVE,
            DIRECTION_SEND_RECEIVE, AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_HD_PR, GTT_MODE_FULL);

    pMediaManager->SetMediaInfo(objISession, objMediaInfo1);
    pMediaManager->SetMediaInfo(objISession1, objMediaInfo2);

    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession), objMediaInfo1);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession1), objMediaInfo2);
}

TEST_F(MtcMediaManagerTest, GetMediaInfoReturnsEmptyMediaInfoWhenNotSet)
{
    MediaInfo objMediaInfo;

    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession), objMediaInfo);
}

TEST_F(MtcMediaManagerTest, UpdateAudioMediaDirectionUpdatesMediaInfo)
{
    std::vector<IMS_SINT32> objDirections{DIRECTION_INVALID, DIRECTION_INACTIVE, DIRECTION_RECEIVE,
            DIRECTION_SEND, DIRECTION_SEND_RECEIVE};
    for (IMS_SINT32 eDirection : objDirections)
    {
        pMediaManager->UpdateMediaDirection(objISession, MEDIATYPE_AUDIO, eDirection);
        EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, eDirection);
    }
}

TEST_F(MtcMediaManagerTest, UpdateVideoMediaDirectionUpdatesMediaInfo)
{
    std::vector<IMS_SINT32> objDirections{DIRECTION_INVALID, DIRECTION_INACTIVE, DIRECTION_RECEIVE,
            DIRECTION_SEND, DIRECTION_SEND_RECEIVE};
    for (IMS_SINT32 eDirection : objDirections)
    {
        pMediaManager->UpdateMediaDirection(objISession, MEDIATYPE_VIDEO, eDirection);
        EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, eDirection);
    }
}

TEST_F(MtcMediaManagerTest, UpdateTextMediaDirectionUpdatesMediaInfo)
{
    std::vector<IMS_SINT32> objDirections{DIRECTION_INVALID, DIRECTION_INACTIVE, DIRECTION_RECEIVE,
            DIRECTION_SEND, DIRECTION_SEND_RECEIVE};
    for (IMS_SINT32 eDirection : objDirections)
    {
        pMediaManager->UpdateMediaDirection(objISession, MEDIATYPE_TEXT, eDirection);
        EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, eDirection);
    }
}

TEST_F(MtcMediaManagerTest, FormSdpWhenNegotiationStateIsOfferSent)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_EQ(IMS_FAILURE, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsSuccessAndNegotiationStateToBeNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    EXPECT_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .Times(2)
            .WillOnce(Return(NegotiationState::STATE_IDLE))
            .WillOnce(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaSession, FormSdp(NEGO_ID, &objISession, MEDIA_TYPE_AUDIO, _, _, _, _))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIO)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsSuccessAndContainsVideo)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    EXPECT_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .Times(2)
            .WillOnce(Return(NegotiationState::STATE_IDLE))
            .WillOnce(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaSession, FormSdp(NEGO_ID, &objISession, MEDIA_TYPE_AUDIOVIDEO, _, _, _, _))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIOVIDEO)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pMediaManager->FormSdp(&objISession, CallType::VT));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsSuccessAndNegotiationStateToBeNotNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    EXPECT_CALL(objMediaSession,
            FormSdp(NEGO_ID, &objISession, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO, _, _, _, _))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO))
            .Times(0);

    EXPECT_EQ(IMS_SUCCESS, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsFailure)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    EXPECT_CALL(objMediaSession,
            FormSdp(NEGO_ID, &objISession, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO, _, _, _, _))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO))
            .Times(0);

    EXPECT_EQ(IMS_FAILURE, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, NegotiateSdpAndNegotiationStateIsNotNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(objMediaSession, NegotiateSdp(NEGO_ID, &objISession))
            .WillOnce(Return(SdpNegotiationResult()));

    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    MEDIA_CONTENT_TYPE eContentType = MEDIA_TYPE_AUDIO;
    ON_CALL(objMediaSession, GetNegotiatedMediaType(NEGO_ID)).WillByDefault(Return(eContentType));
    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, eContentType)).Times(0);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpAndNegotiationStateIsNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(objMediaSession, NegotiateSdp(NEGO_ID, &objISession))
            .WillOnce(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIO)));

    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIO)).Times(1);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpContainsVideo)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(objMediaSession, NegotiateSdp(NEGO_ID, &objISession))
            .WillOnce(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIOVIDEO)));

    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIOVIDEO)).Times(1);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpContainsText)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(objMediaSession, NegotiateSdp(NEGO_ID, &objISession))
            .WillOnce(Return(SdpNegotiationResult(MEDIA_NEGO_NO_ERROR, MEDIA_TYPE_AUDIOTEXT)));

    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIOTEXT)).Times(1);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpReturnsError)
{
    const SdpNegotiationResult objResult(MEDIA_NEGO_ERROR_INVALID_DESCRIPTOR);
    ON_CALL(objMediaSession, NegotiateSdp(_, _)).WillByDefault(Return(objResult));

    EXPECT_EQ(pMediaManager->NegotiateSdp(&objISession).eResult, objResult.eResult);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeDoesNothingIfNoPemAndConfigDoesNotRequire)
{
    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(AString::ConstEmpty()));
    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, PemType::NONE)).Times(0);
    pMediaManager->UpdatePemType(&objISession, &objIMessage);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeUpdatesProfileManagerIfNoPemButConfigRequires)
{
    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(AString::ConstEmpty()));
    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, PemType::NONE));
    pMediaManager->UpdatePemType(&objISession, &objIMessage);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeUpdatesProfileManagerIfPemSendrecv)
{
    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(AString("sendrecv")));

    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, PemType::SENDRECV));
    pMediaManager->UpdatePemType(&objISession, &objIMessage);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeUpdatesProfileManagerIfPemSendonly)
{
    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(AString("sendonly")));

    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, PemType::SENDONLY));
    pMediaManager->UpdatePemType(&objISession, &objIMessage);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeUpdatesProfileManagerIfPemRecvonly)
{
    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(AString("recvonly")));

    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, PemType::RECVONLY));
    pMediaManager->UpdatePemType(&objISession, &objIMessage);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeUpdatesProfileManagerIfPemInactive)
{
    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(AString("inactive")));

    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, PemType::INACTIVE));
    pMediaManager->UpdatePemType(&objISession, &objIMessage);
}

TEST_F(MtcMediaManagerTest, RunForMtCallInEarlyDialogState)
{
    CallInfo objCallInfo;
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).Times(0);
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
}

TEST_F(MtcMediaManagerTest, RunInvokesDoNothingIfMediaIsNotNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).Times(0);
    pMediaManager->Run(&objISession, &objIMessage, IMS_FALSE);
}

TEST_F(MtcMediaManagerTest, RunForConfirmedDialogIfAudioDirectionIsReceiveOnly)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    EXPECT_CALL(objMediaSession,
            SetOptions(NEGO_ID, IMediaSession::OptionType::SET_CONFIRMED_SESSION, IMS_TRUE, 0))
            .Times(1);
    EXPECT_CALL(*pMediaProfileManager, SetConfirmed(&objISession, IMS_TRUE)).Times(1);
    EXPECT_CALL(objMediaSession, FinalizeSdp(NEGO_ID, &objISession)).Times(1);

    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 1000)).Times(1);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(1);
    pMediaManager->Run(&objISession, &objIMessage, IMS_FALSE);
}

TEST_F(MtcMediaManagerTest, RunForConfirmedDialogIfAudioDirectionIsNotReceiveOnly)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));
    MtcSupplementaryService objSupplementaryService(objContext, *pConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService())
            .WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(objMediaSession,
            SetOptions(NEGO_ID, IMediaSession::OptionType::SET_CONFIRMED_SESSION, IMS_TRUE, 0))
            .Times(1);
    EXPECT_CALL(*pMediaProfileManager, SetConfirmed(&objISession, IMS_TRUE)).Times(1);
    EXPECT_CALL(objMediaSession, FinalizeSdp(NEGO_ID, &objISession)).Times(1);
    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(1);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(1);
    pMediaManager->Run(&objISession, &objIMessage, IMS_FALSE);
    EXPECT_EQ(nullptr, objSupplementaryService.Get(SuppType::ENFORCE_LT));
}

TEST_F(MtcMediaManagerTest, RunIf180IsNotReceived)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(3)
            .WillRepeatedly(Return(SipStatusCode::SC_183));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_TRUE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);

    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(3);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).Times(3).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(3);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);

    EXPECT_CALL(objIMessage, GetStatusCode()).WillOnce(Return(SipStatusCode::SC_182));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedWithoutNegotiation)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillOnce(Return(NegotiationState::STATE_NEGOTIATED));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_FALSE(pMediaManager->IsLocalTone());

    EXPECT_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillOnce(Return(NegotiationState::STATE_OFFER_SENT));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_TRUE(pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest, RunAndLocalToneStateIsNotChangedIf180IsReceivedInCaseOfInvalidPolicy)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(-1));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    IMS_BOOL bLocalTone = pMediaManager->IsLocalTone();
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(bLocalTone, pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedInCaseOfUsingDynamicNwToneTimer)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(0));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objIMessage, GetStatusCode()).WillOnce(Return(SipStatusCode::SC_183));
    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_TRUE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);

    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(3);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).Times(4).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(4);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, nullptr, IMS_TRUE);

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(2)
            .WillRepeatedly(Return(SipStatusCode::SC_182));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(4)
            .WillRepeatedly(Return(SipStatusCode::SC_183));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 1000)).Times(1);
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedInCaseOfNotUsingDynamicNwToneTimer)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(1));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(3)
            .WillRepeatedly(Return(SipStatusCode::SC_183));
    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(4)
            .WillRepeatedly(Return(PemType::INACTIVE));

    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_TRUE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(4);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).Times(4).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(4);

    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_FALSE(pMediaManager->IsLocalTone());

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(2)
            .WillRepeatedly(Return(SipStatusCode::SC_182));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_FALSE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_TRUE(pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedAndMessageIsNullInCaseOfLocalToneWith180ByForce)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(2));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(1);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(0);
    pMediaManager->Run(&objISession, nullptr, IMS_TRUE);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(1);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(1);
    pMediaManager->Run(&objISession, nullptr, IMS_TRUE);
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedInCaseOfLocalToneWith180ByForce)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_LOCAL_RINGBACK_TONE_WITH_180_RESPONSE_INT))
            .WillByDefault(Return(2));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(3);
    EXPECT_CALL(objMediaSession, Run(NEGO_ID)).Times(3).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(3);

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(4)
            .WillRepeatedly(Return(SipStatusCode::SC_180));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(2)
            .WillRepeatedly(Return(SipStatusCode::SC_182));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_FALSE(pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest, SetRtpPortInvokesMediaSessionSetOptions)
{
    IMS_UINT32 eMediaType = MEDIATYPE_AUDIO;
    MEDIA_CONTENT_TYPE eContentType = MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO;
    IMS_UINT32 nPort = 12345;
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    EXPECT_CALL(objMediaSession,
            SetOptions(NEGO_ID, IMediaSession::OptionType::SET_RTP_PORT, eContentType, nPort));
    pMediaManager->SetRtpPort(&objISession, eMediaType, nPort);
}

TEST_F(MtcMediaManagerTest, GetRtpPortInvokesMediaSessionGetRemotePort)
{
    IMS_UINT32 eMediaType = MEDIATYPE_AUDIO;
    MEDIA_CONTENT_TYPE eContentType = MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO;
    IMS_UINT32 nPort = 12345;
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    EXPECT_CALL(objMediaSession, GetRemotePort(NEGO_ID, eContentType)).WillOnce(Return(nPort));
    EXPECT_EQ(pMediaManager->GetRemoteRtpPort(&objISession, eMediaType), nPort);
}

TEST_F(MtcMediaManagerTest, SetConferenceCallInvokesMediaSessionSetOptions)
{
    EXPECT_CALL(objMediaSession,
            SetOptions(UNDEFINED_NEGO_ID, IMediaSession::OptionType::SET_CONFERENCE_ENABLE, 0, 0));
    pMediaManager->SetConferenceCall();
}

TEST_F(MtcMediaManagerTest, GetNegotiationStateReturnsIdleIfMediaSessionIsNull)
{
    MtcMediaManager* pTestMediaManager = new MtcMediaManager(objContext, objMediaManager);
    EXPECT_EQ(pTestMediaManager->GetNegotiationState(&objISession), NegotiationState::STATE_IDLE);
    delete pTestMediaManager;
}

TEST_F(MtcMediaManagerTest, GetNegotiationStateReturnsValueFromMediaSession)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    //clang-format off
    std::vector<NegotiationState> objNeoStates{
            STATE_IDLE, STATE_OFFER_RECEIVED, STATE_OFFER_SENT, STATE_NEGOTIATED, STATE_NOTUSED};
    //clang-format on

    for (NegotiationState eState : objNeoStates)
    {
        EXPECT_CALL(objMediaSession, GetNegoState(NEGO_ID)).WillOnce(Return(eState));
        EXPECT_EQ(pMediaManager->GetNegotiationState(&objISession), eState);
    }
}

TEST_F(MtcMediaManagerTest, GetNegotiatedDirectionReturnsInvalidIfMediaSessionIsNull)
{
    IMS_UINT32 eMediaType = MEDIATYPE_AUDIO;

    MtcMediaManager* pTestMediaManager = new MtcMediaManager(objContext, objMediaManager);
    EXPECT_EQ(pTestMediaManager->GetNegotiatedDirection(&objISession, eMediaType),
            MEDIA_DIRECTION::MEDIA_DIRECTION_INVALID);
    delete pTestMediaManager;
}

TEST_F(MtcMediaManagerTest, GetNegotiatedDirectionReturnsValueFromMediaSession)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    IMS_UINT32 eMediaType = MEDIATYPE_AUDIO;
    MEDIA_CONTENT_TYPE eContentType = MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO;

    //clang-format off
    std::vector<MEDIA_DIRECTION> objDirections{MEDIA_DIRECTION_INVALID, MEDIA_DIRECTION_INACTIVE,
            MEDIA_DIRECTION_RECEIVE, MEDIA_DIRECTION_SEND, MEDIA_DIRECTION_SEND_RECEIVE};
    //clang-format on

    // cppcheck-suppress knownEmptyContainer
    for (MEDIA_DIRECTION eDirection : objDirections)
    {
        EXPECT_CALL(objMediaSession, GetNegotiatedDirection(NEGO_ID, eContentType))
                .WillOnce(Return(eDirection));
        EXPECT_EQ(pMediaManager->GetNegotiatedDirection(&objISession, eMediaType), eDirection);
    }
}

TEST_F(MtcMediaManagerTest, GetNegotiatedCallTypeReturnsUnknownIfMediaSessionIsNull)
{
    MtcMediaManager* pTestMediaManager = new MtcMediaManager(objContext, objMediaManager);
    EXPECT_EQ(pTestMediaManager->GetNegotiatedCallType(&objISession), CallType::UNKNOWN);
    delete pTestMediaManager;
}

TEST_F(MtcMediaManagerTest, GetNegotiatedCallTypeReturnsValueBasedOnMediaTypeFromMediaSession)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    MEDIA_CONTENT_TYPE eContentType = MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO;
    ON_CALL(objMediaSession, GetNegotiatedMediaType(NEGO_ID)).WillByDefault(Return(eContentType));
    EXPECT_EQ(pMediaManager->GetNegotiatedCallType(&objISession), CallType::VOIP);
}

TEST_F(MtcMediaManagerTest, SetSrvccStateDoesNohtingIfMediaSessionIsNull)
{
    MtcMediaManager* pTestMediaManager = new MtcMediaManager(objContext, objMediaManager);
    pTestMediaManager->SetSrvccState(SrvccState::IDLE);
    delete pTestMediaManager;
}

TEST_F(MtcMediaManagerTest, SetSrvccStateSetsStateToMediaSession)
{
    EXPECT_CALL(objMediaSession, NotifySrvccStatus(MEDIA_SRVCC_STATUS::MEDIA_SRVCC_IDLE));
    pMediaManager->SetSrvccState(SrvccState::IDLE);

    EXPECT_CALL(objMediaSession, NotifySrvccStatus(MEDIA_SRVCC_STATUS::MEDIA_SRVCC_STARTED));
    pMediaManager->SetSrvccState(SrvccState::STARTED);

    EXPECT_CALL(objMediaSession, NotifySrvccStatus(MEDIA_SRVCC_STATUS::MEDIA_SRVCC_SUCCEED));
    pMediaManager->SetSrvccState(SrvccState::SUCCEEDED);

    EXPECT_CALL(objMediaSession, NotifySrvccStatus(MEDIA_SRVCC_STATUS::MEDIA_SRVCC_FAILED));
    pMediaManager->SetSrvccState(SrvccState::FAILED);

    EXPECT_CALL(objMediaSession, NotifySrvccStatus(MEDIA_SRVCC_STATUS::MEDIA_SRVCC_CANCELED));
    pMediaManager->SetSrvccState(SrvccState::CANCELED);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoOfferSetsDirectionToSendReceiveIfNotHeldByMe)
{
    ON_CALL(objContext, IsHeldByMe).WillByDefault(Return(IMS_FALSE));

    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    objRefMediaInfo.eTextDirection = DIRECTION_INACTIVE;

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::UNKNOWN);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::VOIP);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::VT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::RTT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND_RECEIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::VIDEO_RTT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND_RECEIVE);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoOfferSetsDirectionToSendIfHeldByMe)
{
    ON_CALL(objContext, IsHeldByMe).WillByDefault(Return(IMS_TRUE));

    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    objRefMediaInfo.eTextDirection = DIRECTION_INACTIVE;

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::UNKNOWN);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::VOIP);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::VT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::RTT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND);

    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);
    pMediaManager->AdjustDirectionForAutoOffer(objISession, CallType::VIDEO_RTT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoAnswerDoesNothingIfNotHeldByMe)
{
    ON_CALL(objContext, IsHeldByMe).WillByDefault(Return(IMS_FALSE));

    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;
    objRefMediaInfo.eTextDirection = DIRECTION_SEND_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAnswer(objISession);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND_RECEIVE);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoAnswerSetsDirectionToSendFromSendReceive)
{
    ON_CALL(objContext, IsHeldByMe).WillByDefault(Return(IMS_TRUE));

    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;
    objRefMediaInfo.eTextDirection = DIRECTION_SEND_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAnswer(objISession);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoAnswerSetsDirectionToInactiveFromReceive)
{
    ON_CALL(objContext, IsHeldByMe).WillByDefault(Return(IMS_TRUE));

    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_RECEIVE;
    objRefMediaInfo.eTextDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAnswer(objISession);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INACTIVE);
}

TEST_F(MtcMediaManagerTest,
        AdjustDirectionForLocalResourceConfirmationSetDirectionToActiveFromInactiveOnVoip)
{
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    pMediaManager->AdjustDirectionForLocalResourceConfirmation(objISession, CallType::VOIP);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_INVALID);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INVALID);
}

TEST_F(MtcMediaManagerTest,
        AdjustDirectionForLocalResourceConfirmationSetDirectionToActiveFromInactiveOnVt)
{
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    objMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    pMediaManager->AdjustDirectionForLocalResourceConfirmation(objISession, CallType::VT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_INVALID);
}

TEST_F(MtcMediaManagerTest,
        AdjustDirectionForLocalResourceConfirmationSetDirectionToActiveFromInactiveOnVideoRtt)
{
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    objMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    objMediaInfo.eTextDirection = DIRECTION_INACTIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    pMediaManager->AdjustDirectionForLocalResourceConfirmation(objISession, CallType::VIDEO_RTT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_SEND_RECEIVE);
}

TEST_F(MtcMediaManagerTest,
        AdjustDirectionForLocalResourceConfirmationDoesNothingIfAlreadyActiveDirection)
{
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objMediaInfo.eVideoDirection = DIRECTION_SEND;
    objMediaInfo.eTextDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    pMediaManager->AdjustDirectionForLocalResourceConfirmation(objISession, CallType::VIDEO_RTT);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eVideoDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo(objISession).eTextDirection, DIRECTION_RECEIVE);
}

TEST_F(MtcMediaManagerTest, IsOnHoldReturnsTrueIfAudioDirectionIsNotSendReceiveAndInvalid)
{
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);
    EXPECT_TRUE(pMediaManager->IsOnHold(objISession));

    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);
    EXPECT_TRUE(pMediaManager->IsOnHold(objISession));

    objMediaInfo.eAudioDirection = DIRECTION_SEND;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);
    EXPECT_TRUE(pMediaManager->IsOnHold(objISession));
}

TEST_F(MtcMediaManagerTest, IsOnHoldReturnsFalseIfAudioDirectionIsSendReceiveOrInvalid)
{
    MediaInfo objMediaInfo;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);
    EXPECT_FALSE(pMediaManager->IsOnHold(objISession));

    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);
    EXPECT_FALSE(pMediaManager->IsOnHold(objISession));
}

TEST_F(MtcMediaManagerTest,
        GetSupportedMediaTypesFromSdpInvokesMediaSessionGetSupportedMediaTypesFromSdp)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(objMediaSession, GetSupportedMediaTypesFromSdp(NEGO_ID, &objISession))
            .WillByDefault(Return(MEDIA_TYPE_AUDIO));
    EXPECT_EQ(MEDIATYPE_AUDIO, pMediaManager->GetSupportedMediaTypesFromSdp(&objISession));
}

TEST_F(MtcMediaManagerTest, IsForkedSessionReturnsValueFromProfileManager)
{
    ON_CALL(*pMediaProfileManager, IsForked(&objISession)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(pMediaManager->IsForkedSession(&objISession));

    ON_CALL(*pMediaProfileManager, IsForked(&objISession)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(pMediaManager->IsForkedSession(&objISession));
}

TEST_F(MtcMediaManagerTest, FinalizeSdpInvokesMediaSessionFinalizeSdp)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMediaSession, FinalizeSdp(NEGO_ID, &objISession));
    pMediaManager->FinalizeSdp(&objISession);

    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMediaSession, FinalizeSdp(NEGO_ID, &objISession));
    pMediaManager->FinalizeSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, RestoreSdpInvokesISessionRestoreIfEstablishedState)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMediaSession, FinalizeSdp(NEGO_ID, &objISession));

    ON_CALL(objISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    EXPECT_CALL(objISession, Restore);

    pMediaManager->RestoreSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, RestoreSdpDoesNotInvokeISessionRestoreIfNotEstablishedState)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMediaSession, FinalizeSdp(NEGO_ID, &objISession));

    ON_CALL(objISession, GetState).WillByDefault(Return(ISession::STATE_CREATED));
    EXPECT_CALL(objISession, Restore).Times(0);

    pMediaManager->RestoreSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeInvokesSetMediaPemTypeWithSendRecvHeader)
{
    const IMS_UINTP nNegoId = 123;
    AString strPemHeader = "sendrecv";
    PemType eExpectedProfileManagerPemType = PemType::SENDRECV;
    MEDIA_PEM_TYPE eExpectedMediaSessionPemType = MEDIA_PEM_TYPE::SENDRECV;

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(*pMediaProfileManager,
            CreateMediaProfile(
                    &objISession, IMS_FALSE, IMS_TRUE, MEDIA_TYPE_AUDIO, &objMediaSession));

    pMediaManager->CreateMediaProfile(&objISession, IMS_FALSE, IMS_TRUE);

    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(strPemHeader));

    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(nNegoId));

    // Expect calls made by MtcMediaManager::UpdatePemType
    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, eExpectedProfileManagerPemType));
    EXPECT_CALL(objMediaSession, SetMediaPemType(nNegoId, eExpectedMediaSessionPemType));

    // Act
    pMediaManager->UpdatePemType(&objISession, &objIMessage);

    ON_CALL(objMediaSession, DestroyProfile(nNegoId)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, DestroyMediaProfile(&objISession, &objMediaSession));
    pMediaManager->DestroyMediaForSession(&objISession);
}

TEST_F(MtcMediaManagerTest, UpdatePemTypeDoesNotInvokeSetMediaPemTypeWhenNoHeader)
{
    const IMS_UINTP nNegoId = 123;
    AString strPemHeader = "";  // No P-Early-Media header
    PemType eExpectedProfileManagerPemType = PemType::NONE;
    MEDIA_PEM_TYPE eExpectedMediaSessionPemType = MEDIA_PEM_TYPE::NONE;

    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(*pMediaProfileManager,
            CreateMediaProfile(
                    &objISession, IMS_FALSE, IMS_TRUE, MEDIA_TYPE_AUDIO, &objMediaSession));

    pMediaManager->CreateMediaProfile(&objISession, IMS_FALSE, IMS_TRUE);

    ON_CALL(objMessageUtils, GetHeader(&objIMessage, ISipHeader::P_EARLY_MEDIA, _))
            .WillByDefault(Return(strPemHeader));

    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(nNegoId));

    EXPECT_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_INITIALIZE_P_EARLY_MEDIA_WHEN_NO_HEADER_BOOL))
            .WillOnce(Return(IMS_TRUE));
    // Expect calls made by MtcMediaManager::UpdatePemType
    EXPECT_CALL(*pMediaProfileManager, SetPemType(&objISession, eExpectedProfileManagerPemType));
    EXPECT_CALL(objMediaSession, SetMediaPemType(nNegoId, eExpectedMediaSessionPemType)).Times(0);

    pMediaManager->UpdatePemType(&objISession, &objIMessage);

    ON_CALL(objMediaSession, DestroyProfile(nNegoId)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, DestroyMediaProfile(&objISession, &objMediaSession));
    pMediaManager->DestroyMediaForSession(&objISession);
}

TEST_F(MtcMediaManagerTest, SessionMediaDefaultConstructorInitializesMediaInfosToDefault)
{
    SessionMedia objSessionMedia;
    MediaInfo objDefaultMediaInfo;

    EXPECT_EQ(objSessionMedia.GetMediaInfo(), objDefaultMediaInfo);
    EXPECT_EQ(objSessionMedia.GetOldMediaInfo(), objDefaultMediaInfo);
}

TEST_F(MtcMediaManagerTest, SessionMediaConstructorWithMediaInfoInitializesMediaInfo)
{
    MediaInfo objInitialMediaInfo(DIRECTION_SEND, DIRECTION_SEND, DIRECTION_INACTIVE,
            AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_VGA_PR, GTT_MODE_FULL);
    SessionMedia objSessionMedia(objInitialMediaInfo);

    MediaInfo objDefaultMediaInfo;
    EXPECT_EQ(objSessionMedia.GetMediaInfo(), objInitialMediaInfo);
    EXPECT_EQ(objSessionMedia.GetOldMediaInfo(), objDefaultMediaInfo);
}

TEST_F(MtcMediaManagerTest, SessionMediaSetMediaInfoBacksUpCurrentAndSetsNew)
{
    SessionMedia objSessionMedia;

    MediaInfo objFirstMediaInfo(DIRECTION_SEND, DIRECTION_SEND, DIRECTION_INACTIVE,
            AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_VGA_PR, GTT_MODE_FULL);
    objSessionMedia.SetMediaInfo(objFirstMediaInfo);

    MediaInfo objDefaultMediaInfo;
    EXPECT_EQ(objSessionMedia.GetMediaInfo(), objFirstMediaInfo);
    EXPECT_EQ(objSessionMedia.GetOldMediaInfo(), objDefaultMediaInfo);

    MediaInfo objSecondMediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_SEND_RECEIVE, DIRECTION_INACTIVE,
            AUDIO_QUALITY_AMR_WB, VIDEO_QUALITY_HD_PR, GTT_MODE_VCO);
    objSessionMedia.SetMediaInfo(objSecondMediaInfo);

    EXPECT_EQ(objSessionMedia.GetMediaInfo(), objSecondMediaInfo);
    EXPECT_EQ(objSessionMedia.GetOldMediaInfo(), objFirstMediaInfo);
}

TEST_F(MtcMediaManagerTest, SessionMediaRestoreMediaInfoRestoresMediaInfo)
{
    SessionMedia objSessionMedia;

    MediaInfo objFirstMediaInfo(DIRECTION_SEND, DIRECTION_SEND, DIRECTION_INACTIVE,
            AUDIO_QUALITY_AMR_NB, VIDEO_QUALITY_VGA_PR, GTT_MODE_FULL);
    objSessionMedia.SetMediaInfo(objFirstMediaInfo);

    MediaInfo objSecondMediaInfo(DIRECTION_SEND_RECEIVE, DIRECTION_SEND_RECEIVE, DIRECTION_INACTIVE,
            AUDIO_QUALITY_AMR_WB, VIDEO_QUALITY_HD_PR, GTT_MODE_VCO);
    objSessionMedia.SetMediaInfo(objSecondMediaInfo);
    EXPECT_EQ(objSessionMedia.GetMediaInfo(), objSecondMediaInfo);

    objSessionMedia.RestoreMediaInfo();
    EXPECT_EQ(objSessionMedia.GetMediaInfo(), objFirstMediaInfo);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpPopulatesAudioCodecAttributes)
{
    const IMS_UINTP nNegoId = 123;
    const float nBitrateKbps = 64.0f;
    const float nBandwidthKhz = 16.0f;
    const float nBitrateStartKbps = 32.0f;
    const float nBitrateEndKbps = 128.0f;
    const float nBandwidthStartKhz = 8.0f;
    const float nBandwidthEndKhz = 24.0f;

    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(nNegoId));

    ON_CALL(objMediaSession, GetNegotiatedCodecBitrateKbps(nNegoId))
            .WillByDefault(Return(nBitrateKbps));
    ON_CALL(objMediaSession, GetNegotiatedCodecBandwidthKhz(nNegoId))
            .WillByDefault(Return(nBandwidthKhz));
    ON_CALL(objMediaSession, GetNegotiatedCodecBitrateRange(nNegoId, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(nBitrateStartKbps),
                    SetArgReferee<2>(nBitrateEndKbps), Return()));
    ON_CALL(objMediaSession, GetNegotiatedCodecBandwidthRange(nNegoId, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(nBandwidthStartKhz),
                    SetArgReferee<2>(nBandwidthEndKhz), Return()));

    const SdpNegotiationResult objResult(MEDIA_NEGO_NO_ERROR);
    ON_CALL(objMediaSession, NegotiateSdp(nNegoId, &objISession)).WillByDefault(Return(objResult));

    ON_CALL(objMediaSession, GetNegoState(nNegoId))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    pMediaManager->NegotiateSdp(&objISession);
    const AudioCodecAttributes& objAudioCodecAttributes =
            pMediaManager->GetMediaInfo(objISession).objAudioCodecAttributes;

    EXPECT_FLOAT_EQ(objAudioCodecAttributes.nBitrateKbps, nBitrateKbps);
    EXPECT_FLOAT_EQ(objAudioCodecAttributes.nBitrateStartKbps, nBitrateStartKbps);
    EXPECT_FLOAT_EQ(objAudioCodecAttributes.nBitrateEndKbps, nBitrateEndKbps);
    EXPECT_FLOAT_EQ(objAudioCodecAttributes.nBandwidthKhz, nBandwidthKhz);
    EXPECT_FLOAT_EQ(objAudioCodecAttributes.nBandwidthStartKhz, nBandwidthStartKhz);
    EXPECT_FLOAT_EQ(objAudioCodecAttributes.nBandwidthEndKhz, nBandwidthEndKhz);
}

TEST_F(MtcMediaManagerTest, SetMediaInfoUpdatesAudioCodecAttributes)
{
    const IMS_UINTP nNegoId = 123;
    const float nBitrateKbps = 64.0f;
    const float nBandwidthKhz = 16.0f;
    const float nBitrateStartKbps = 32.0f;
    const float nBitrateEndKbps = 128.0f;
    const float nBandwidthStartKhz = 8.0f;
    const float nBandwidthEndKhz = 24.0f;
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(nNegoId));
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBitrateKbps(nNegoId))
            .WillOnce(Return(nBitrateKbps));
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBandwidthKhz(nNegoId))
            .WillOnce(Return(nBandwidthKhz));
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBitrateRange(nNegoId, _, _))
            .WillOnce(
                    DoAll(SetArgReferee<1>(nBitrateStartKbps), SetArgReferee<2>(nBitrateEndKbps)));
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBandwidthRange(nNegoId, _, _))
            .WillOnce(DoAll(
                    SetArgReferee<1>(nBandwidthStartKhz), SetArgReferee<2>(nBandwidthEndKhz)));
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    const MediaInfo& objResultMediaInfo = pMediaManager->GetMediaInfo(objISession);
    const AudioCodecAttributes& objResultAttributes = objResultMediaInfo.objAudioCodecAttributes;

    EXPECT_EQ(objResultAttributes.nBitrateKbps, nBitrateKbps);
    EXPECT_EQ(objResultAttributes.nBandwidthKhz, nBandwidthKhz);
    EXPECT_EQ(objResultAttributes.nBitrateStartKbps, nBitrateStartKbps);
    EXPECT_EQ(objResultAttributes.nBitrateEndKbps, nBitrateEndKbps);
    EXPECT_EQ(objResultAttributes.nBandwidthStartKhz, nBandwidthStartKhz);
    EXPECT_EQ(objResultAttributes.nBandwidthEndKhz, nBandwidthEndKhz);
}

TEST_F(MtcMediaManagerTest, SetMediaInfoDoesNotUpdateAudioCodecAttributesWhenNotEmpty)
{
    MediaInfo objMediaInfo;
    AudioCodecAttributes objTempAttributes(24.4f, 7.2f, 24.4f, 16.0f, 8.0f, 16.0f);
    objMediaInfo.objAudioCodecAttributes = objTempAttributes;

    EXPECT_CALL(*pMediaProfileManager, GetNegoId(_)).Times(0);
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBitrateKbps(_)).Times(0);
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBandwidthKhz(_)).Times(0);
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBitrateRange(_, _, _)).Times(0);
    EXPECT_CALL(objMediaSession, GetNegotiatedCodecBandwidthRange(_, _, _)).Times(0);
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    const MediaInfo& objResultMediaInfo = pMediaManager->GetMediaInfo(objISession);
    EXPECT_EQ(objResultMediaInfo.objAudioCodecAttributes, objTempAttributes);
}

TEST_F(MtcMediaManagerTest, SetMediaInfoDoesNotUpdateAudioCodecAttributesWhenNegoIdIsUndefined)
{
    MediaInfo objMediaInfo;

    EXPECT_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillOnce(Return(UNDEFINED_NEGO_ID));
    pMediaManager->SetMediaInfo(objISession, objMediaInfo);

    const MediaInfo& objResultMediaInfo = pMediaManager->GetMediaInfo(objISession);
    EXPECT_EQ(objResultMediaInfo.objAudioCodecAttributes, AudioCodecAttributes());
}
