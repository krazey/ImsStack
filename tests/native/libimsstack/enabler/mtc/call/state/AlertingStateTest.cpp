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

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipKeepAliveHelper.h"
#include "MockISipMessage.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/AlertingState.h"
#include "call/state/IMtcCallState.h"
#include "call/state/MtcCallState.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MockUdpKeepAliveSender.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "ussi/MockUssiController.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiData.h"
#include "ussi/UssiDef.h"
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
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcService objService;
    MockIMtcMediaManager objMediaManager;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcSession objMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockIMessageUtils objMessageUtils;
    MockMtcTimerWrapper objTimerWrapper;
    MockUssiController* pUssiController;
    MockISipKeepAliveHelper objKeepAliveHelper;
    CallInfo objCallInfo;
    MediaInfo objMediaInfo;
    MockUdpKeepAliveSender* pUdpKeepAliveSender;
    ImsVector<AString> objActionSets;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objISession, GetPreviousRequest(_)).WillByDefault(Return(&objIMessage));
        ON_CALL(objIMessage, GetMessage).WillByDefault(Return(&objISipMessage));
        ON_CALL(objISession, GetPreviousRequest(_)).WillByDefault(Return(&objIMessage));
        ON_CALL(objIMessage, GetMessage).WillByDefault(Return(&objISipMessage));

        pConfigurationProxy = new MockMtcConfigurationProxy();
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

        pUdpKeepAliveSender = new MockUdpKeepAliveSender(&objKeepAliveHelper, objCallContext);
        ON_CALL(objCallContext, CreateUdpKeepAliveSender)
                .WillByDefault(Return(pUdpKeepAliveSender));

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
        pUssiController = new MockUssiController(objCallContext, new UssiDataParser());
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

TEST_F(AlertingStateTest, OnEnterStartsKeepAlive)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(1));
    EXPECT_CALL(*pUdpKeepAliveSender, Start);

    pAlertingState->OnEnter();
}

TEST_F(AlertingStateTest, OnExitStopsKeepAlive)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(1));
    EXPECT_CALL(*pUdpKeepAliveSender, Stop);

    pAlertingState->OnExit();
}

TEST_F(AlertingStateTest,
        HandleUserAlertSendsProvisonalResponseReliablyAndStartAlertingTimerIf100relIsOnlyInSupportedHeader)
{
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nAnyTime));
    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE, IMS_FALSE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE, IMS_FALSE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_RESTART_RINGING_TIMER_BY_SENDING_180_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_MT_ALERTING, nAnyTime));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->HandleUserAlert());
}

TEST_F(AlertingStateTest, HandleUserAlertRejectCallIfSendsProvisonalResponseFails)
{
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nAnyTime));
    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_FALSE, IMS_FALSE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE, IMS_FALSE))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->HandleUserAlert());
}

TEST_F(AlertingStateTest,
        HandleUserAlertSendsProvisonalResponseReliablyIf100relIsInSupprtedAndRequireHeader)
{
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nAnyTime));
    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE, IMS_TRUE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE, IMS_TRUE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_RESTART_RINGING_TIMER_BY_SENDING_180_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_MT_ALERTING, nAnyTime));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->HandleUserAlert());
}

TEST_F(AlertingStateTest,
        HandleUserAlertSendsProvisonalResponseWithReliableParamAsTrueIfConfigurationOn)
{
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_FALSE));
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_RINGING_TIMER_MILLIS_INT))
            .WillByDefault(Return(nAnyTime));
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_PRACK_SUPPORTED_FOR_18X_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE, IMS_FALSE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    EXPECT_CALL(objMtcSession, SendProvisionalResponse(IMS_TRUE, IMS_TRUE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_RESTART_RINGING_TIMER_BY_SENDING_180_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_MT_ALERTING, nAnyTime));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->HandleUserAlert());
}

TEST_F(AlertingStateTest, HandleUserAlertDoesNotSendProvisonalResponseIf180IsAlreadySent)
{
    ON_CALL(objMessageUtils, IsResponseExist(&objISession, SipStatusCode::SC_180))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendProvisionalResponse(_, _)).Times(0);
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_RESTART_RINGING_TIMER_BY_SENDING_180_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->HandleUserAlert());
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

    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_TRUE, IMS_TRUE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, Accept).Times(0);
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->Accept(eAcceptCallType, objMediaInfo));
}

TEST_F(AlertingStateTest, AcceptStopsKeepAlive)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(1));
    pAlertingState->OnEnter();
    CallType eAcceptCallType = CallType::VOIP;
    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(eAcceptCallType));

    EXPECT_CALL(*pUdpKeepAliveSender, Stop);

    pAlertingState->Accept(eAcceptCallType, objMediaInfo);
}

TEST_F(AlertingStateTest, AcceptDifferentCallTypeWithOutNegoInvokesAccept)
{
    CallType eAcceptCallType = CallType::VOIP;
    CallType eCurrentCallType = CallType::VT;
    ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(eCurrentCallType));

    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_FALSE, IMS_FALSE));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_IDLE));

    EXPECT_CALL(objTimerWrapper, StopAll);
    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->Accept(eAcceptCallType, objMediaInfo));
}

TEST_F(AlertingStateTest, RejectInvokesRejectIncoming)
{
    const CallReasonInfo objReason(CODE_USER_DECLINE);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->Reject(objReason));
}

TEST_F(AlertingStateTest, OnTimerExpiredRejectIncomingCallIfAlertingTimerExpired)
{
    const CallReasonInfo objReason(CODE_TIMEOUT_NO_ANSWER);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING,
            pAlertingState->OnTimerExpired(MtcCallState::TIMER_MT_ALERTING));
}

TEST_F(AlertingStateTest, OnTimerExpiredInvokesSendEarlyUpdateIfGlareConditionTimerExpired)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(_));

    EXPECT_EQ(CallStateName::ALERTING,
            pAlertingState->OnTimerExpired(MtcCallState::TIMER_RETRY_UPDATE));
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

TEST_F(AlertingStateTest, SessionStartedStartsTimerForDelayingUpdate)
{
    IMS_SINT32 nAnyTime = 60;
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_DELAY_UPDATE_AFTER_CONNECTED_TIMER_MILLIS_INT))
            .WillByDefault(Return(nAnyTime));

    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED, nAnyTime));
    EXPECT_EQ(CallStateName::ESTABLISHED, pAlertingState->SessionStarted(&objISession));
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
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession)).WillByDefault(Return());

    SipMethod objMethod = SipMethod::ACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionStarted(&objISession));
}

TEST_F(AlertingStateTest, SessionStartedTerminatesCallIfNoCodecMatched)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_ACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::ERROR_NO_CODEC_MATCHED));
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession)).WillByDefault(Return());

    SipMethod objMethod = SipMethod::ACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionStarted(&objISession));
}

TEST_F(AlertingStateTest, SessionStartedTerminatesCallIfInvalidSdpDescriptor)
{
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_ACK))
            .WillByDefault(Return(&objIMessage));
    ON_CALL(objMessageUtils, HasSdp(&objIMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    ON_CALL(objMediaManager, NegotiateSdp(&objISession))
            .WillByDefault(Return(NegotiationResult::ERROR_INVALID_DESCRIPTOR));
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession)).WillByDefault(Return());

    SipMethod objMethod = SipMethod::ACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    EXPECT_CALL(
            objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
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

TEST_F(AlertingStateTest, SessionCanceledOnAcceptedTransitsStateToTerminated)
{
    EXPECT_CALL(objUiNotifier, SendTerminated(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE)));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionCanceledOnAccepted(&objISession));
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
    SetActionConfigs(
            SipStatusCode::SC_491, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_GLARE_CONDITION});

    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objISession));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TimerType::TIMER_RETRY_UPDATE, _));

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
    ON_CALL(objPreconditionManager, OnSdpReceived(&objISession)).WillByDefault(Return());

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
    SetUpForUssi();

    EXPECT_CALL(objTimerWrapper, StopAll);
    EXPECT_CALL(*pUssiController, FormAcceptUssi).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMtcSession, Reject(_)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->AcceptUssi(CallType::VOIP, objMediaInfo));
}

TEST_F(AlertingStateTest, AcceptUssiRejectsIncomingUssiIfAcceptUssiFails)
{
    SetUpForUssi();

    EXPECT_CALL(*pUssiController, FormAcceptUssi).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMtcSession, Accept).Times(1).WillOnce(Return(IMS_FAILURE));
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason));

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->AcceptUssi(CallType::VOIP, objMediaInfo));
}

TEST_F(AlertingStateTest, AcceptUssiRejectsIncomingUssiIfFormAcceptUssiFails)
{
    SetUpForUssi();
    EXPECT_CALL(*pUssiController, FormAcceptUssi).Times(1).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objMtcSession, Accept).Times(0);
    const CallReasonInfo objReason(CODE_REJECT_INTERNAL_ERROR);
    EXPECT_CALL(objMtcSession, Reject(objReason)).Times(1);
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReason)).Times(1);

    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->AcceptUssi(CallType::VOIP, objMediaInfo));
}

TEST_F(AlertingStateTest, UssiStartedTransitsStateToEstablished)
{
    MtcSupplementaryService objSupplementaryService(objCallContext, *pConfigurationProxy);
    ON_CALL(objCallContext, GetSupplementaryService)
            .WillByDefault(ReturnRef(objSupplementaryService));
    SetUpForUssi();
    SipMethod objMethod = SipMethod::ACK;
    ON_CALL(objIMessage, GetMethod).WillByDefault(ReturnRef(objMethod));

    UssiResult objResult(UssiNextAction::SEND_INFO_WITH_NOTIFY_ELEMENT, UssiError::CODE_NONE);
    EXPECT_CALL(*pUssiController, ParseUssiBodyAndCheckResult(_, _))
            .Times(1)
            .WillOnce(Return(objResult));
    EXPECT_CALL(objCallContext, CreateClientConnection(_))
            .Times(1)
            .WillOnce(Return(reinterpret_cast<ISipClientConnection*>(0x0)));
    EXPECT_CALL(objUiNotifier, SendStarted).Times(1);

    EXPECT_EQ(CallStateName::ESTABLISHED, pAlertingState->UssiStarted(&objISession));
}

TEST_F(AlertingStateTest, SessionStartFailedNotifiesStartFailed)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    const IMS_SINT32 objNonEstablishedStates[] = {ISession::STATE_CREATED,
            ISession::STATE_INITIATED, ISession::STATE_NEGOTIATING, ISession::STATE_ESTABLISHING,
            ISession::STATE_RENEGOTIATING, ISession::STATE_REESTABLISHING,
            ISession::STATE_TERMINATING, ISession::STATE_TERMINATED};

    for (const auto& state : objNonEstablishedStates)
    {
        ON_CALL(objISession, GetState).WillByDefault(Return(state));
        EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR)));

        EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionStartFailed(&objISession));
    }
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

TEST_F(AlertingStateTest, SessionStartFailedInvokesTerminateIfEstablishedState)
{
    ON_CALL(objISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    const CallReasonInfo objReason(CODE_SIP_SERVER_ERROR);

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, objReason));
    EXPECT_CALL(objUiNotifier, SendTerminated(objReason));
    EXPECT_EQ(CallStateName::TERMINATING, pAlertingState->SessionStartFailed(&objISession));
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
