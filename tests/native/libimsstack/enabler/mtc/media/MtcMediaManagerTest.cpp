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

#include "../../include/media/MockIMediaSession.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMediaReportEventListener.h"
#include "media/MockMediaProfileManager.h"
#include "media/MtcMediaManager.h"
#include "sipcore/SipStatusCode.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey DEFAULT_CALL_KEY = 1;
LOCAL IMS_UINTP NEGO_ID = 100;

class TestMtcMediaManager : public MtcMediaManager
{
public:
    inline explicit TestMtcMediaManager(IN IMtcCallContext& objContext) :
            MtcMediaManager(objContext)
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
            piMediaSession(IMS_NULL),
            objISession(),
            objIMessage(),
            pConfigurationManager(new MockIMtcConfigurationManager()),
            pConfigurationProxy(new MtcConfigurationProxy(pConfigurationManager)),
            objMessageUtils(),
            pMediaProfileManager(IMS_NULL)
    {
    }

public:
    MockIMtcCallContext objContext;
    MtcMediaManager* pMediaManager;
    MockIMediaReportEventListener* pListener;
    MockIMediaSession* piMediaSession;
    MockISession objISession;
    MockIMessage objIMessage;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMessageUtils objMessageUtils;
    MockMtcMediaProfileManager* pMediaProfileManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(DEFAULT_CALL_KEY));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetMessageUtils()).WillByDefault(ReturnRef(objMessageUtils));

        pMediaManager = CreateMtcMediaManager(objContext);

        pMediaManager->SetMediaReportEventListener(pListener);
    }

    virtual void TearDown() override
    {
        delete pMediaManager;
        delete pListener;
        delete pConfigurationProxy;
    }

    MtcMediaManager* CreateMtcMediaManager(IN IMtcCallContext& objContext)
    {
        TestMtcMediaManager* pTestMediaManager = new TestMtcMediaManager(objContext);

        piMediaSession = new MockIMediaSession();
        pTestMediaManager->ReplaceMediaSession(piMediaSession);

        pMediaProfileManager = new MockMtcMediaProfileManager();
        pTestMediaManager->ReplaceMediaProfileManager(pMediaProfileManager);

        return pTestMediaManager;
    }
};

TEST_F(MtcMediaManagerTest, GetStateReturnsIdleInitially)
{
    EXPECT_EQ(MediaState::IDLE, pMediaManager->GetState());
    EXPECT_EQ(MediaState::IDLE, pMediaManager->GetOldState());
}

TEST_F(MtcMediaManagerTest, TerminateSetsStateToTerminating)
{
    pMediaManager->Terminate();

    EXPECT_EQ(MediaState::TERMINATING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithSuccessReport)
{
    pMediaManager->MediaSession_Notify(REPORT_SUCCESS);

    EXPECT_EQ(MediaState::STARTED, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, MediaSessionNotifyWithCloseSessionReport)
{
    pMediaManager->MediaSession_Notify(REPORT_CLOSE_SESSION);

    EXPECT_EQ(MediaState::TERMINATED, pMediaManager->GetState());
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

    pMediaManager->MediaSession_Notify(REPORT_VIDEO_LOWEST_BIT_RATE);
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
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
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
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
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
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
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
    pMediaManager->SetMediaInfo(objInfo);

    MtcSupplementaryService objSupplementaryService(*pConfigurationProxy);
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
    pMediaManager->SetMediaInfo(objInfo);

    MtcSupplementaryService objSupplementaryService(*pConfigurationProxy);
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

TEST_F(MtcMediaManagerTest, FormSdpWhenNegotiationStateIsOfferSent)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_EQ(IMS_FAILURE, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsSuccessAndNegotiationStateToBeNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    EXPECT_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .Times(2)
            .WillOnce(Return(NegotiationState::STATE_IDLE))
            .WillOnce(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(*piMediaSession, FormSDP(NEGO_ID, &objISession, MEDIA_TYPE_AUDIO, _, _, _, _))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIO)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsSuccessAndContainsVideo)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    EXPECT_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .Times(2)
            .WillOnce(Return(NegotiationState::STATE_IDLE))
            .WillOnce(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(*piMediaSession, FormSDP(NEGO_ID, &objISession, MEDIA_TYPE_AUDIOVIDEO, _, _, _, _))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIO)).Times(1);
    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_VIDEO)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pMediaManager->FormSdp(&objISession, CallType::VT));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsSuccessAndNegotiationStateToBeNotNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    EXPECT_CALL(*piMediaSession,
            FormSDP(NEGO_ID, &objISession, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO, _, _, _, _))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO))
            .Times(0);

    EXPECT_EQ(IMS_SUCCESS, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, FormSdpReturnsFailure)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    EXPECT_CALL(*piMediaSession,
            FormSDP(NEGO_ID, &objISession, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO, _, _, _, _))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_CONTENT_TYPE::MEDIA_TYPE_AUDIO))
            .Times(0);

    EXPECT_EQ(IMS_FAILURE, pMediaManager->FormSdp(&objISession, CallType::VOIP));
}

TEST_F(MtcMediaManagerTest, NegotiateSdpAndNegotiationStateIsNotNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(*piMediaSession, NegotiateSDP(NEGO_ID, &objISession, _, _, _, _)).Times(1);

    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    MEDIA_CONTENT_TYPE eContentType = MEDIA_TYPE_AUDIO;
    ON_CALL(*piMediaSession, GetNegotiatedMediaType(NEGO_ID)).WillByDefault(Return(eContentType));
    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, eContentType)).Times(0);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpAndNegotiationStateIsNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(*piMediaSession, NegotiateSDP(NEGO_ID, &objISession, _, _, _, _)).Times(1);

    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    MEDIA_CONTENT_TYPE eContentType = MEDIA_TYPE_AUDIO;
    ON_CALL(*piMediaSession, GetNegotiatedMediaType(NEGO_ID)).WillByDefault(Return(eContentType));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, eContentType)).Times(1);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpContainsVideo)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(*piMediaSession, NegotiateSDP(NEGO_ID, &objISession, _, _, _, _)).Times(1);

    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    MEDIA_CONTENT_TYPE eContentType = MEDIA_TYPE_AUDIOVIDEO;
    ON_CALL(*piMediaSession, GetNegotiatedMediaType(NEGO_ID)).WillByDefault(Return(eContentType));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIO)).Times(1);
    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_VIDEO)).Times(1);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, NegotiateSdpContainsText)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));

    EXPECT_CALL(*piMediaSession, NegotiateSDP(NEGO_ID, &objISession, _, _, _, _)).Times(1);

    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    MEDIA_CONTENT_TYPE eContentType = MEDIA_TYPE_AUDIOTEXT;
    ON_CALL(*piMediaSession, GetNegotiatedMediaType(NEGO_ID)).WillByDefault(Return(eContentType));

    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_AUDIO)).Times(1);
    EXPECT_CALL(*piMediaSession, RequestQos(NEGO_ID, MEDIA_TYPE_TEXT)).Times(1);

    pMediaManager->NegotiateSdp(&objISession);
}

TEST_F(MtcMediaManagerTest, RunForMtCallInEarlyDialogState)
{
    CallInfo objCallInfo;
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).Times(0);
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_NE(MediaState::STARTING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, RunInvokesDoNothingIfMediaIsNotNegotiated)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).Times(0);
    pMediaManager->Run(&objISession, &objIMessage, IMS_FALSE);
    EXPECT_NE(MediaState::STARTING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, RunForConfirmedDialogIfAudioDirectionIsReceiveOnly)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objMediaInfo);

    EXPECT_CALL(*piMediaSession,
            SetOptions(NEGO_ID, IMediaSession::OptionType::SET_CONFIRMED_SESSION, IMS_TRUE, 0))
            .Times(1);
    EXPECT_CALL(*pMediaProfileManager, SetConfirmed(&objISession, IMS_TRUE)).Times(1);
    EXPECT_CALL(*piMediaSession, FinalizeSDP(NEGO_ID, &objISession)).Times(1);

    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 1000)).Times(1);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(1);
    pMediaManager->Run(&objISession, &objIMessage, IMS_FALSE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, RunForConfirmedDialogIfAudioDirectionIsNotReceiveOnly)
{
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_TRUE));
    MtcSupplementaryService objSupplementaryService(*pConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService())
            .WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(*piMediaSession,
            SetOptions(NEGO_ID, IMediaSession::OptionType::SET_CONFIRMED_SESSION, IMS_TRUE, 0))
            .Times(1);
    EXPECT_CALL(*pMediaProfileManager, SetConfirmed(&objISession, IMS_TRUE)).Times(1);
    EXPECT_CALL(*piMediaSession, FinalizeSDP(NEGO_ID, &objISession)).Times(1);
    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(1);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(1);
    pMediaManager->Run(&objISession, &objIMessage, IMS_FALSE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_EQ(nullptr, objSupplementaryService.Get(SuppType::ENFORCE_LT));
}

TEST_F(MtcMediaManagerTest, RunIf180IsNotReceived)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
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
    EXPECT_NE(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(3);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).Times(3).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(3);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(objIMessage, GetStatusCode()).WillOnce(Return(SipStatusCode::SC_182));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, RunAndLocalToneStateIsNotChangedIf180IsReceivedInCaseOfInvalidPolicy)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
            .WillByDefault(Return(-1));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

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
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
            .WillByDefault(Return(0));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objIMessage, GetStatusCode()).WillOnce(Return(SipStatusCode::SC_183));
    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_TRUE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_NE(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(3);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).Times(4).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(4);

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, nullptr, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(2)
            .WillRepeatedly(Return(SipStatusCode::SC_182));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(4)
            .WillRepeatedly(Return(SipStatusCode::SC_183));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 1000)).Times(1);
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedInCaseOfNotUsingDynamicNwToneTimer)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
            .WillByDefault(Return(1));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
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
    EXPECT_NE(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(4);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).Times(4).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(4);

    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .Times(2)
            .WillRepeatedly(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_FALSE(pMediaManager->IsLocalTone());

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(2)
            .WillRepeatedly(Return(SipStatusCode::SC_182));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_FALSE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_TRUE(pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedAndMessageIsNullInCaseOfLocalToneWith180ByForce)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
            .WillByDefault(Return(2));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(1);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(0);
    pMediaManager->Run(&objISession, nullptr, IMS_TRUE);
    EXPECT_NE(MediaState::STARTING, pMediaManager->GetState());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(1);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(1);
    pMediaManager->Run(&objISession, nullptr, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
}

TEST_F(MtcMediaManagerTest, RunIf180IsReceivedInCaseOfLocalToneWith180ByForce)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForLocalRingbackToneWith180Response())
            .WillByDefault(Return(2));
    ON_CALL(*pMediaProfileManager, GetNegoId(&objISession)).WillByDefault(Return(NEGO_ID));
    ON_CALL(*piMediaSession, GetNegoState(NEGO_ID))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ON_CALL(*pMediaProfileManager, IsConfirmed(&objISession)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*piMediaSession, SetNetworkToneRtpTimer(NEGO_ID, MEDIA_TYPE_AUDIO, 0)).Times(3);
    EXPECT_CALL(*piMediaSession, Run(NEGO_ID)).Times(3).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*pMediaProfileManager, UpdateProfileForMediaActivation(&objISession)).Times(3);

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(4)
            .WillRepeatedly(Return(SipStatusCode::SC_180));

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::INACTIVE));
    EXPECT_CALL(*pMediaProfileManager, IsPemSendInOtherEarlySession(&objISession))
            .WillOnce(Return(IMS_FALSE));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(*pMediaProfileManager, GetPemType(&objISession))
            .WillOnce(Return(PemType::SENDRECV));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_TRUE(pMediaManager->IsLocalTone());

    EXPECT_CALL(objIMessage, GetStatusCode())
            .Times(2)
            .WillRepeatedly(Return(SipStatusCode::SC_182));
    pMediaManager->Run(&objISession, &objIMessage, IMS_TRUE);
    EXPECT_EQ(MediaState::STARTING, pMediaManager->GetState());
    EXPECT_FALSE(pMediaManager->IsLocalTone());
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoAnswerForNormalCase)
{
    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND;
    objRefMediaInfo.eVideoDirection = DIRECTION_SEND;
    objRefMediaInfo.eTextDirection = DIRECTION_SEND;
    pMediaManager->SetMediaInfo(objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAccept(IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eVideoDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eTextDirection, DIRECTION_SEND);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoOffer)
{
    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND;
    objRefMediaInfo.eVideoDirection = DIRECTION_SEND;
    objRefMediaInfo.eTextDirection = DIRECTION_SEND;
    pMediaManager->SetMediaInfo(objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAccept(IMS_TRUE, IMS_FALSE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eVideoDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eTextDirection, DIRECTION_SEND_RECEIVE);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoAnswerForHeldByMeCase)
{
    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND;
    pMediaManager->SetMediaInfo(objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAccept(IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eVideoDirection, DIRECTION_INVALID);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eTextDirection, DIRECTION_INVALID);

    objRefMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pMediaManager->SetMediaInfo(objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAccept(IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eAudioDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eVideoDirection, DIRECTION_INVALID);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eTextDirection, DIRECTION_INVALID);
}

TEST_F(MtcMediaManagerTest,
        AdjustDirectionForAutoAnswerForHeldByMeIfAudioAndVideoDirectionDifferent)
{
    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    pMediaManager->SetMediaInfo(objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAccept(IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eAudioDirection, DIRECTION_SEND);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eTextDirection, DIRECTION_INVALID);
}

TEST_F(MtcMediaManagerTest, AdjustDirectionForAutoAnswerIfAudioAndVideoDirectionDifferent)
{
    MediaInfo objRefMediaInfo;
    objRefMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objRefMediaInfo.eVideoDirection = DIRECTION_INACTIVE;
    pMediaManager->SetMediaInfo(objRefMediaInfo);

    pMediaManager->AdjustDirectionForAutoAccept(IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eAudioDirection, DIRECTION_SEND_RECEIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eVideoDirection, DIRECTION_INACTIVE);
    EXPECT_EQ(pMediaManager->GetMediaInfo().eTextDirection, DIRECTION_INVALID);
}
