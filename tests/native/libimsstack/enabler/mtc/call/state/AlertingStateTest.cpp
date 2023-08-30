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
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/state/AlertingState.h"
#include "call/state/IMtcCallState.h"
#include "call/state/MtcCallState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/ISipMessage.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipMethod.h"
#include "sipcore/SipStatusCode.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiController.h"
#include "ussi/UssiData.h"
#include "utility/MessageUtil.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class AlertingStateTest : public ::testing::Test
{
public:
    AlertingState* pAlertingState;
    MockISession objISession;
    MockIMessage objIMessage;
    MockISipMessage objISipMessage;
    MockIMtcCallContext objCallContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcService objService;
    MockIMtcMediaManager objMediaManager;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcSession objMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockIMessageUtils objMessageUtils;
    MockMtcTimerWrapper objTimerWrapper;
    UssiController* pUssiController;
    CallInfo objCallInfo;
    MediaInfo objMediaInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objISession, GetPreviousRequest(_)).WillByDefault(Return(&objIMessage));
        ON_CALL(objIMessage, GetMessage).WillByDefault(Return(&objISipMessage));
        ON_CALL(objISession, GetPreviousRequest(_)).WillByDefault(Return(&objIMessage));
        ON_CALL(objIMessage, GetMessage).WillByDefault(Return(&objISipMessage));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));

        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));

        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objCallContext, GetSession(&objISession)).WillByDefault(Return(&objMtcSession));

        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        ON_CALL(objCallContext, GetTimer).WillByDefault(ReturnRef(objTimerWrapper));

        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));

        pUssiController = IMS_NULL;
        pAlertingState = new AlertingState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pAlertingState;
        delete pConfigurationProxy;
        delete pUssiController;
    }

    void SetUpForUssi()
    {
        // TODO: use MockUssiController
        pUssiController = new UssiController(objCallContext, new UssiDataParser());
        ON_CALL(objCallContext, GetUssiController).WillByDefault(Return(pUssiController));
        ON_CALL(objISession, GetNextRequest()).WillByDefault(Return(&objIMessage));

        AString strHeaderValue(UssiConstants::HEADER_USSD_PACKAGE);
        ON_CALL(objMessageUtils,
                AddValueIfNotExists(
                        &objIMessage, strHeaderValue, ISipHeader::RECV_INFO, AString::ConstNull()))
                .WillByDefault(Return(IMS_SUCCESS));

        AString strEmptyHeaderName;
        ON_CALL(objMessageUtils,
                AddValueIfNotExists(&objIMessage, _, ISipHeader::ACCEPT, strEmptyHeaderName))
                .WillByDefault(Return(IMS_SUCCESS));
    }
};

TEST_F(AlertingStateTest, HandleUserAlertSendsProvisonalResponseAndStartAlertingTimer)
{
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationManager, GetRingingTimer).WillByDefault(Return(nAnyTime));
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_MT_ALERTING, nAnyTime));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->HandleUserAlert());
}

TEST_F(AlertingStateTest, HandleUserAlertRejectCallIfSendsProvisonalResponseFails)
{
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationManager, GetRingingTimer).WillByDefault(Return(nAnyTime));
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->HandleUserAlert());
}

TEST_F(AlertingStateTest, AcceptSameCallTypeInvokesAccept)
{
    CallType eAcceptCallType = CallType::VOIP;
    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(eAcceptCallType));
    EXPECT_CALL(objTimerWrapper, StopAll);
    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->Accept(eAcceptCallType, objMediaInfo));
}

TEST_F(AlertingStateTest, RejectIncomingCallIfAcceptFails)
{
    CallType eAcceptCallType = CallType::VOIP;
    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(eAcceptCallType));

    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_FAILURE));

    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->Accept(eAcceptCallType, objMediaInfo));
}

TEST_F(AlertingStateTest, AcceptDifferentCallTypeInvokesSendEarlyUpdate)
{
    CallType eAcceptCallType = CallType::VOIP;
    CallType eCurrentCallType = CallType::VT;
    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(eCurrentCallType));

    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, Accept).Times(0);
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->Accept(eAcceptCallType, objMediaInfo));
}

TEST_F(AlertingStateTest, RejectInvokesRejectIncoming)
{
    const CallReasonInfo objReason(CODE_USER_DECLINE);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->Reject(objReason));
}

TEST_F(AlertingStateTest, RejectIncomingCallIfAlertingTimerExpired)
{
    const CallReasonInfo objReason(CODE_TIMEOUT_NO_ANSWER);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING,
            pAlertingState->OnTimerExpired(MtcCallState::TIMER_MT_ALERTING));
}

TEST_F(AlertingStateTest, QosReserveFailedInvokesRejectIncomingCallIfNextActionIsRelease)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(&objISession, IMS_TRUE))
            .Times(2);  // TODO: duplicated code.
    const CallReasonInfo objReason(CODE_REJECT_QOS_FAILURE);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING,
            pAlertingState->QosReserveFailed(&objISession, QosLossPolicy::RELEASE));
}

TEST_F(AlertingStateTest, QosReserveFailedDoesNothingIfNextActionIsModify)
{
    // TODO: do modify
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(&objISession, IMS_TRUE)).Times(0);
    EXPECT_CALL(objMtcSession, Reject(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::ALERTING,
            pAlertingState->QosReserveFailed(&objISession, QosLossPolicy::MODIFY));
}

TEST_F(AlertingStateTest, SessionStartedTransitsStateToEstablished)
{
    MtcSupplementaryService objSupplementaryService(objCallContext, *pConfigurationProxy);
    ON_CALL(objCallContext, GetSupplementaryService)
            .WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(objUiNotifier, SendStarted).Times(1);

    EXPECT_EQ(CallStateName::ESTABLISHED, pAlertingState->SessionStarted(&objISession));
}

TEST_F(AlertingStateTest, SessionStartedTerminatesCallIfOfferAnswerFails)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_ACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::NO_ERROR));
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession, &objIMessage))
            .WillByDefault(Return());

    SipMethod objMethod = SipMethod::ACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionStarted(&objISession));
}

TEST_F(AlertingStateTest, SessionTerminatedTransitsStateToTerminated)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_TERMINATE))
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionTerminated(&objISession));

    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_TERMINATE))
            .WillByDefault(Return(&objIMessage));
    SipMethod objMethod = SipMethod::CANCEL;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionTerminated(&objISession));
}

TEST_F(AlertingStateTest, AcceptIsCalledWhenUpdateIsNotSentBySrvcc)
{
    ON_CALL(objISession, GetPreviousResponse(_)).WillByDefault(Return(&objIMessage));
    ON_CALL(objIMessage, GetState).WillByDefault(Return(IMessage::STATE_SENT));

    ON_CALL(objMtcSession, GetOngoingUpdateType).WillByDefault(Return(UpdateType::NORMAL));
    EXPECT_CALL(objMtcSession, Accept).Times(1);
    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(AlertingStateTest, NoAcceptIsCalledWhenUpdateIsSentBySrvcc)
{
    ON_CALL(objISession, GetPreviousResponse(_)).WillByDefault(Return(&objIMessage));
    ON_CALL(objIMessage, GetState).WillByDefault(Return(IMessage::STATE_SENT));

    ON_CALL(objMtcSession, GetOngoingUpdateType)
            .WillByDefault(Return(UpdateType::SRVCC_RECOVERED_CANCEL));
    EXPECT_CALL(objMtcSession, Accept).Times(0);
    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(AlertingStateTest, SessionEarlyMediaUpdateFailedNotifiesStartFailed)
{
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pAlertingState->SessionEarlyMediaUpdateFailed(&objISession));
}

TEST_F(AlertingStateTest, SessionEarlyMediaUpdateFailedWith491StartsGlareCondition)
{
    ON_CALL(objIMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objISession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objISession));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TimerType::TIMER_GLARE_CONDITION, _));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->SessionEarlyMediaUpdateFailed(&objISession));
}

TEST_F(AlertingStateTest, SessionEarlyMediaUpdateReceivedInvokesRespondToEarlyUpdate200)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200));
    pAlertingState->SessionEarlyMediaUpdateReceived(&objISession);

    // RespondToEarlyUpdate fails
    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING,
            pAlertingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(AlertingStateTest, SessionEarlyMediaUpdateReceivedInvokesRespondToEarlyUpdate488)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objISession, IsSdpNegotiationAllowedForNonRpr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_RECEIVED));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488));
    EXPECT_EQ(
            CallStateName::ALERTING, pAlertingState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(AlertingStateTest, SessionPrackReceivedInvokesRespondToPrack)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200)).Times(1);
    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->SessionPrackReceived(&objISession));

    // RespondToPrack fails
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionPrackReceived(&objISession));
}

TEST_F(AlertingStateTest, SessionPrackReceivedInvokesRejectIncomingIfOfferAnswerFails)
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

    SipMethod objMethod = SipMethod::PRACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    const CallReasonInfo objReason(CODE_SIP_NOT_ACCEPTABLE);
    EXPECT_CALL(objMtcSession, RespondToPrack(SipStatusCode::SC_200));
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionPrackReceived(&objISession));
}

TEST_F(AlertingStateTest, SessionRprDeliveryFailedRejectsIncomingCall)
{
    const CallReasonInfo objReason(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionRprDeliveryFailed(&objISession));
}

TEST_F(AlertingStateTest, AcceptUssiInvokesAccept)
{
    // TODO: Mocking UssiController may be needed.
    SetUpForUssi();

    EXPECT_CALL(objTimerWrapper, StopAll);
    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->AcceptUssi(CallType::VOIP, objMediaInfo));
}

TEST_F(AlertingStateTest, RejectIncomingUssiIfAcceptUssiFails)
{
    // TODO: Mocking UssiController may be needed.
    SetUpForUssi();

    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->AcceptUssi(CallType::VOIP, objMediaInfo));
}

TEST_F(AlertingStateTest, UssiStartedTransitsStateToEstablished)
{
    MtcSupplementaryService objSupplementaryService(objCallContext, *pConfigurationProxy);
    ON_CALL(objCallContext, GetSupplementaryService)
            .WillByDefault(ReturnRef(objSupplementaryService));
    SetUpForUssi();

    EXPECT_CALL(objUiNotifier, SendStarted).Times(1);

    EXPECT_EQ(CallStateName::ESTABLISHED, pAlertingState->SessionStarted(&objISession));
}

TEST_F(AlertingStateTest, SessionStartFailedNotifiesStartFailed)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionStartFailed(&objISession));
}

TEST_F(AlertingStateTest, SessionStartFailedDoesNotNotifyStartFailedIfSrvccStarted)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::STARTED));
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    ON_CALL(objAosConnector, IsImsConnected).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->SessionStartFailed(&objISession));
}

TEST_F(AlertingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp).Times(0);

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    pAlertingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(AlertingStateTest, OnIpcanChangedPushesPendingOperation)
{
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    const IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(eIpcan));

    pAlertingState->OnIpcanChanged(eIpcan);
}

TEST_F(AlertingStateTest, TerminateInvokesTerminate)
{
    const CallReasonInfo objAnyReason(CODE_USER_DECLINE);

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objAnyReason));
    EXPECT_CALL(objUiNotifier, SendTerminated(objAnyReason));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->Terminate(objAnyReason));
}

TEST_F(AlertingStateTest, SendUpdateBySrvccByCanceled)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ImsList<IMtcSession*> objSessions;
    objSessions.Append(&objMtcSession);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_CANCEL)).Times(1);

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(AlertingStateTest, SendUpdateBySrvccByFailed)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    ImsList<IMtcSession*> objSessions;
    objSessions.Append(&objMtcSession);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(1);

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->OnSrvccStateUpdated(SrvccState::FAILED));
}
