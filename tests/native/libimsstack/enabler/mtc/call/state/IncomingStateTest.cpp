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

#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MtcContextRepository.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/IncomingState.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
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
    // cppcheck-suppress unusedStructMember
    MockIMessage objIMessage;
    MockIMtcMediaManager objMediaManager;
    MockIMessageUtils objMessageUtils;
    MockEpsFallbackTrigger* pEpsFbTrigger;
    MockMtcConfigurationProxy* pConfigurationProxy;
    CallInfo objCallInfo;
    MockMtcTimerWrapper objTimer;
    MtcSupplementaryService* pSupplementaryService;
    ParticipantInfo* pParticipantInfo;
    MediaInfo objMediaInfo;
    ImsVector<AString> objActionSets;

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

        pConfigurationProxy = new MockMtcConfigurationProxy();
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

    MtcExtensionSet GetTestExtensionSet(IN const AString& strOptionTag,
            IN const IMS_BOOL& bIsAvailableOnRemote, IN const IMS_BOOL& bIsRequiredOnRemote)
    {
        ImsList<IMtcExtension*> objExtensions;
        MockIMtcExtension* pExtension = new MockIMtcExtension();
        ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));
        ON_CALL(*pExtension, IsAvailableOnRemote).WillByDefault(Return(bIsAvailableOnRemote));
        ON_CALL(*pExtension, IsRequiredOnRemote).WillByDefault(Return(bIsRequiredOnRemote));
        objExtensions.Append(pExtension);
        MtcExtensionSet objMtcExtensionSet(objCallContext, objExtensions);
        return objMtcExtensionSet;
    }

    void SetActionConfigs(IN IMS_SINT32 nStatusCode, std::initializer_list<IMS_SINT32> objActions)
    {
        AString strActionSet;
        strActionSet.SetNumber(nStatusCode);
        strActionSet += ":";

        bool bFirst = true;
        for (IMS_SINT32 nAction : objActions)
        {
            if (!bFirst)
            {
                strActionSet += ",";
            }
            AString strAction;
            strAction.SetNumber(nAction);
            strActionSet += strAction;
            bFirst = false;
        }

        objActionSets.Add(strActionSet);
        ON_CALL(*pConfigurationProxy,
                GetStringArray(
                        ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY))
                .WillByDefault(Return(objActionSets));
    }
};

TEST_F(IncomingStateTest, OnExitStopsTimer)
{
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_RETRY_UPDATE));

    pIncomingState->OnExit();
}

TEST_F(IncomingStateTest, RejectTransitsStateToTerminating)
{
    const CallReasonInfo objAnyReason(CODE_REJECT_ONGOING_CALL_SETUP);

    EXPECT_CALL(objMtcSession, Reject(objAnyReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objAnyReason));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->Reject(objAnyReason));
}

TEST_F(IncomingStateTest, TerminateInvokesTerminate)
{
    const CallReasonInfo objAnyReason(CODE_USER_DECLINE);

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objAnyReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objAnyReason));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->Terminate(objAnyReason));
}

TEST_F(IncomingStateTest, SessionTerminatedTransitsStateToTerminated)
{
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_));
    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionTerminated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdatedInvokesRejectIncomingAndToTerminating)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMtcSession, HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, Ref(objIMessage)))
            .WillByDefault(Return());

    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    const SipMethod objMethod(SipMethod::ACK);
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    const CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE);

    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdatedWithNoCodecMatched)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMtcSession, HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, Ref(objIMessage)))
            .WillByDefault(Return());

    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    const SipMethod objMethod(SipMethod::ACK);
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::ERROR_NO_CODEC_MATCHED));
    const CallReasonInfo objReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE);

    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdatedWithInvalidDescriptor)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMtcSession, HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, Ref(objIMessage)))
            .WillByDefault(Return());

    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    const SipMethod objMethod(SipMethod::ACK);
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::ERROR_INVALID_DESCRIPTOR));
    const CallReasonInfo objReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS);

    EXPECT_CALL(objMtcSession, Reject(objReasonInfo));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReasonInfo));

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdatedInvokesIncomingCallReceived)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMtcSession, HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, Ref(objIMessage)))
            .WillByDefault(Return());

    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));

    const SipMethod objMethod(SipMethod::INVITE);

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
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);
    EXPECT_EQ(CallStateName::ALERTING, pIncomingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateFailedNotifiesStartFailed)
{
    EXPECT_CALL(
            objUiNotifier, SendIncomingCallRejected(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->SessionEarlyMediaUpdateFailed(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateFailedWith491StartsGlareCondition)
{
    ON_CALL(objIMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
    SetActionConfigs(
            SipStatusCode::SC_491, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_GLARE_CONDITION});

    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objISession));
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_RETRY_UPDATE, _));

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
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);
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
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionEarlyMediaUpdateReceivedInvokesRespondToEarlyUpdate488)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
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
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));
    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionPrackReceivedInvokesRespondToPrackAndSendsIncomingCallReceived)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT));
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));

    MockIMtcService objService;
    ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_TRUE));
    SetParamsForIncomingCallReceived();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived());

    EXPECT_EQ(CallStateName::ALERTING, pIncomingState->SessionPrackReceived(&objISession));

    // RespondToPrack fails
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT));
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionPrackReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionPrackReceivedSendsIncomingCallReceivedIf180Exists)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived());

    EXPECT_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser()).Times(0);
    EXPECT_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession)).Times(0);
    SetParamsForIncomingCallReceived();

    EXPECT_EQ(CallStateName::ALERTING, pIncomingState->SessionPrackReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionPrackReceivedInvokesRejectIncomingIfOfferAnswerFails)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::NO_ERROR));
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession)).WillByDefault(Return());

    const SipMethod objMethod = SipMethod::PRACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT));
    const CallReasonInfo objReason(CODE_SIP_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200));
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionPrackReceived(&objISession));
}

TEST_F(IncomingStateTest, SessionRprDeliveryFailedRejectsIncomingCall)
{
    const CallReasonInfo objReason(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));

    pIncomingState->SessionRprDeliveryFailed(&objISession);
}

TEST_F(IncomingStateTest, SessionStartFailedInvokesIgnoring)
{
    MockIMtcService objService;
    ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::STARTED));
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    ON_CALL(objAosConnector, IsImsConnected).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->SessionStartFailed(&objISession));
}

TEST_F(IncomingStateTest, SessionStartFailedInvokesSendStartFailed)
{
    MockIMtcService objService;
    ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(1);

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->SessionStartFailed(&objISession));
}

TEST_F(IncomingStateTest, OnTimerExpiredDoesNothing)
{
    EXPECT_CALL(objMtcSession, GetOngoingUpdateType()).Times(0);

    EXPECT_EQ(CallStateName::INCOMING,
            pIncomingState->OnTimerExpired(
                    MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON));
}

TEST_F(IncomingStateTest, OnTimerExpiredByPrackWaitRejectsCall)
{
    EXPECT_CALL(objMtcSession,
            Reject(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK)))
            .Times(1);

    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->OnTimerExpired(MtcCallState::TimerType::TIMER_MT_PRACK_WAIT));
}

TEST_F(IncomingStateTest, OnTimerExpiredInvokesSendEarlyUpdate)
{
    EXPECT_CALL(objMtcSession, GetOngoingUpdateType())
            .Times(1)
            .WillOnce(Return(UpdateType::SESSION));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SESSION)).Times(1);

    EXPECT_EQ(CallStateName::INCOMING,
            pIncomingState->OnTimerExpired(MtcCallState::TimerType::TIMER_RETRY_UPDATE));
}

TEST_F(IncomingStateTest, OnTimerExpiredRejectIncomingCallIfAlertingTimerExpired)
{
    const CallReasonInfo objReason(CODE_TIMEOUT_NO_ANSWER);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(objReason));

    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->OnTimerExpired(MtcCallState::TIMER_MT_ALERTING));
}

TEST_F(IncomingStateTest, QosReservedDoesNothingIfPrackIsNull)
{
    MockIMessage* objNullIMessage = reinterpret_cast<MockIMessage*>(0x0);
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(objNullIMessage));
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived()).Times(0);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->QosReserved(&objISession, 0));
}

TEST_F(IncomingStateTest, QosReservedDoesNothingIfIsCheckingResourcesRequiredToAlertUserIsFalse)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived()).Times(0);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->QosReserved(&objISession, 0));
}

TEST_F(IncomingStateTest, QosReservedDoesNothingIfIsAvailableToAlertUserIsFalse)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived()).Times(0);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->QosReserved(&objISession, 0));
}

TEST_F(IncomingStateTest, QosReservedInvokesSendIncomingCallReceived)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived()).Times(1);

    EXPECT_EQ(CallStateName::ALERTING, pIncomingState->QosReserved(&objISession, 0));
}

TEST_F(IncomingStateTest, QosReservedInvokesRejectsIfSendProvisionalResponseFailed)
{
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objPreconditionManager, IsCheckingResourcesRequiredToAlertUser())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToAlertUser(&objISession))
            .WillByDefault(Return(IMS_TRUE));

    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE, IMS_TRUE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE, IMS_TRUE))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objMtcSession, Reject(_)).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_)).Times(1);

    EXPECT_EQ(CallStateName::TERMINATING, pIncomingState->QosReserved(&objISession, 0));
}

TEST_F(IncomingStateTest, QosReserveFailedDoesNothing)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(_, _)).Times(0);
    EXPECT_CALL(objMtcSession, Reject(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_)).Times(0);

    EXPECT_EQ(CallStateName::INCOMING,
            pIncomingState->QosReserveFailed(&objISession, QosLossPolicy::MAINTAIN));
}

TEST_F(IncomingStateTest, QosReserveFailedInvokesSendIncomingCallRejected)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(_, _)).Times(1);
    EXPECT_CALL(objMtcSession, Reject(_)).Times(1);
    EXPECT_CALL(objUiNotifier, SendIncomingCallRejected(_)).Times(1);

    EXPECT_EQ(CallStateName::TERMINATING,
            pIncomingState->QosReserveFailed(&objISession, QosLossPolicy::RELEASE));
}

TEST_F(IncomingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp).Times(0);

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    pIncomingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(IncomingStateTest, OnIpcanChangedPushesPendingOperation)
{
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    const IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(eIpcan));

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
    IMS_SINT32 nAnyDataFailureReason = 1;
    ON_CALL(*pEpsFbTrigger, IsWaitingRegistration).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallStateName::INCOMING,
            pIncomingState->OnAosStateChanged(
                    MtcAosState::CONNECTED, nAnyAosReason, nAnyDataFailureReason));
}

TEST_F(IncomingStateTest, OnAosConnectedReturnsAlertingStateIfWaitingEpsFallback)
{
    IMS_UINT32 nAnyAosReason = 1;
    IMS_SINT32 nAnyDataFailureReason = 1;

    MockIMtcService objService;
    ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingRegistration).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    SetParamsForIncomingCallReceived();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_REQUIRE_PRACK_FOR_ALERT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted);
    EXPECT_CALL(objUiNotifier, SendIncomingCallReceived);
    EXPECT_EQ(CallStateName::ALERTING,
            pIncomingState->OnAosStateChanged(
                    MtcAosState::CONNECTED, nAnyAosReason, nAnyDataFailureReason));
}

}  // namespace android
