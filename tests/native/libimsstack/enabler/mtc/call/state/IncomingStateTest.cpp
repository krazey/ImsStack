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

#include "MockIMtcService.h"
#include "MtcContextRepository.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/MockMtcPendingOperationHolder.h"
#include "call/ParticipantInfo.h"
#include "call/state/IncomingState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipMethod.h"
#include "sipcore/SipStatusCode.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class IncomingStateTest : public ::testing::Test
{
public:
    IncomingState* pIncomingState;
    MockIMtcCallContext objCallContext;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcUiNotifier objUiNotifier;
    MockIMtcSession objMtcSession;
    MockISession objISession;
    MockIMessage objIMessage;
    MockISipMessage objISipMessage;
    MockIMtcMediaManager objMediaManager;
    MockIMessageUtils objMessageUtils;
    MockEpsFallbackTrigger* pEpsFbTrigger;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    CallInfo objCallInfo;
    MockMtcTimerWrapper objTimer;
    MtcSupplementaryService* pSupplementaryService;
    ParticipantInfo* pParticipantInfo;
    MediaInfo objMediaInfo;

protected:
    virtual void SetUp() override
    {
        pEpsFbTrigger = new MockEpsFallbackTrigger(objCallContext);
        ON_CALL(objCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));

        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));

        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));

        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objCallContext, GetTimer).WillByDefault(ReturnRef(objTimer));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
        pSupplementaryService = IMS_NULL;
        pParticipantInfo = IMS_NULL;

        pIncomingState = new IncomingState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pIncomingState;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pParticipantInfo;
        delete pEpsFbTrigger;
    }

    void MakeIsPreviewOfAnswerReturnsTrue(IN const SipMethod& objMethod)
    {
        ON_CALL(objISession, IsSdpNegotiationAllowedForNonRpr).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
        ON_CALL(objIMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));
        ON_CALL(objIMessage, GetMessage).WillByDefault(Return(&objISipMessage));
        ON_CALL(objISipMessage, IsMessageRpr).WillByDefault(Return(IMS_FALSE));
    }

    void SetParamsForIncomingCallReceived()
    {
        ON_CALL(objCallContext, GetCallKey).WillByDefault(Return(1));

        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        pSupplementaryService = new MtcSupplementaryService(objCallContext, *pConfigurationProxy);
        ON_CALL(objCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        pParticipantInfo = new ParticipantInfo(objCallContext);
        ON_CALL(objCallContext, GetParticipantInfo).WillByDefault(ReturnRef(*pParticipantInfo));
    }
};

TEST_F(IncomingStateTest, RejectTransitsStateToTerminating)
{
    const CallReasonInfo objAnyReason(CODE_REJECT_ONGOING_CALL_SETUP);

    EXPECT_CALL(objMtcSession, Reject(objAnyReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objAnyReason));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->Reject(objAnyReason));
}

TEST_F(IncomingStateTest, TerminateInvokesTerminate)
{
    const CallReasonInfo objAnyReason(CODE_USER_DECLINE);

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objAnyReason));
    EXPECT_CALL(objUiNotifier, SendTerminated(objAnyReason));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->Terminate(objAnyReason));
}

TEST_F(IncomingStateTest, SessionTerminatedTransitsStateToTerminated)
{
    EXPECT_CALL(objUiNotifier, SendStartFailed(_));
    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionTerminated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdatedInvokesIncomingCallReceived)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMtcSession, HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, Ref(objIMessage)))
            .WillByDefault(Return());

    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));

    const SipMethod objMethod(SipMethod::INVITE);
    MakeIsPreviewOfAnswerReturnsTrue(objMethod);
    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->SessionEarlyMediaUpdated(&objISession));

    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_TRUE));
    SetParamsForIncomingCallReceived();
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived(_, _, _, _, _));
    EXPECT_EQ(CallStateName::ALERTING, pIncomingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateFailedNotifiesStartFailed)
{
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->SessionEarlyMediaUpdateFailed(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateFailedWith491StartsGlareCondition)
{
    ON_CALL(objIMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objISession));
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_GLARE_CONDITION, _));

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->SessionEarlyMediaUpdateFailed(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateReceivedInvokesRespondToEarlyUpdate200)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(
            CallStateName::INCOMING, pIncomingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateReceivedSendsIncomingCallReceivedIfQosReserved)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_TRUE));

    SetParamsForIncomingCallReceived();
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived(_, _, _, _, _));
    EXPECT_EQ(
            CallStateName::ALERTING, pIncomingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateReceivedRejectsCallIfSendingResponseFails)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));

    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateReceivedInvokesRespondToEarlyUpdate488)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objISession, IsSdpNegotiationAllowedForNonRpr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    pIncomingState->SessionEarlyMediaUpdateReceived(&objISession);
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateReceivedInvokesRespondToEarlyUpdate488AndFails)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objISession, IsSdpNegotiationAllowedForNonRpr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionPRAckReceivedInvokesRespondToPrackAndSendsIncomingCallReceived)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    MockIMtcService objService;
    ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_TRUE));
    SetParamsForIncomingCallReceived();

    EXPECT_EQ(CallStateName::ALERTING, pIncomingState->SessionPRAckReceived(&objISession));

    // RespondToPrack fails
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionPRAckReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionPRAckReceivedInvokesRejectIncomingIfOfferAnswerFails)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::NO_ERROR));
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession, &objIMessage))
            .WillByDefault(Return());

    const SipMethod objMethod = SipMethod::PRACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    const CallReasonInfo objReason(CODE_SIP_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200));
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionPRAckReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionRPRDeliveryFailedRejectsIncomingCall)
{
    const CallReasonInfo objReason(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    pIncomingState->SessionRPRDeliveryFailed(&objISession);
}

TEST_F(IncomingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp).Times(0);

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    pIncomingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(IncomingStateTest, OnIpcanChangedPushesPendingOperation)
{
    MockMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

    pIncomingState->OnIpcanChanged(eIpcan);
}

TEST_F(IncomingStateTest, SendUpdateBySrvccByCanceled)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ImsList<IMtcSession*> objSessions;
    objSessions.Append(&objMtcSession);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_CANCEL)).Times(1);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(IncomingStateTest, SendUpdateBySrvccByFailed)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ImsList<IMtcSession*> objSessions;
    objSessions.Append(&objMtcSession);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(1);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(IncomingStateTest, OnAosConnectedInvokesPreconditionManagerIpCanChanged)
{
    IMS_UINT32 nAnyAosReason = 1;
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(0));
    EXPECT_CALL(*pEpsFbTrigger, IsWaitingEpsFallbackForNoTrigger).Times(0);
    EXPECT_CALL(objPreconditionManager, HandleQosOnIpcanChanged);

    EXPECT_EQ(CallStateName::INCOMING,
            pIncomingState->OnAosStateChanged(MtcAosState::CONNECTED, nAnyAosReason));
}

TEST_F(IncomingStateTest, OnAosConnectedReturnsAlertingStateIfWaitingEpsFallback)
{
    IMS_UINT32 nAnyAosReason = 1;

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallbackForNoTrigger).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pEpsFbTrigger, IsVoNr).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted);
    SetParamsForIncomingCallReceived();
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived(_, _, _, _, _));
    EXPECT_EQ(CallStateName::ALERTING,
            pIncomingState->OnAosStateChanged(MtcAosState::CONNECTED, nAnyAosReason));
}

}  // namespace android
